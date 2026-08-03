from pathlib import Path
import re
import shutil
import subprocess
from tempfile import TemporaryDirectory
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[1]
ASM_DIR = PROJECT_ROOT / "src/asm"
FIXTURE_DIR = PROJECT_ROOT / "tests/asm"
FIXTURE_NAME = "state_core_fixture.asm"


def read_symbols(path: Path) -> dict[str, int]:
    symbols: dict[str, int] = {}
    for line in path.read_text().splitlines():
        match = re.fullmatch(r"([^ ]+) \$([0-9A-Fa-f]+)", line)
        if match is not None:
            symbols[match.group(1)] = int(match.group(2), 16)
    return symbols


class StateCoreQualificationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.assembler = shutil.which("ez80asm") or str(
            Path.home() / ".local/bin/ez80asm"
        )

    def stage_fixture(self, root: Path, fixture_source: str | None = None) -> Path:
        staged_asm = root / "src/asm"
        staged_fixture = root / "tests/asm"
        shutil.copytree(
            ASM_DIR,
            staged_asm,
            ignore=shutil.ignore_patterns("*.lst", "*.symbols"),
        )
        shutil.copytree(
            FIXTURE_DIR,
            staged_fixture,
            ignore=shutil.ignore_patterns("*.lst", "*.symbols"),
        )
        if fixture_source is not None:
            (staged_fixture / FIXTURE_NAME).write_text(fixture_source)
        return staged_fixture

    def assemble_fixture(
        self,
        staged_fixture: Path,
        output: Path,
    ) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [
                self.assembler,
                "-l",
                "-s",
                FIXTURE_NAME,
                str(output),
            ],
            cwd=staged_fixture,
            capture_output=True,
            check=False,
            text=True,
        )

    def test_fixture_assembles_with_exact_immutable_spans(self) -> None:
        with TemporaryDirectory(prefix="nurples-state-core-host-") as temp_dir:
            root = Path(temp_dir)
            staged_fixture = self.stage_fixture(root)
            output = root / "state_core_fixture.bin"
            result = self.assemble_fixture(staged_fixture, output)

            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertGreater(output.stat().st_size, 0)
            symbols = read_symbols(staged_fixture / "state_core_fixture.symbols")
            expected_spans = {
                ("state_new_descriptors", "state_new_descriptors_end"): 14 * 12,
                ("state_new_policy_table", "state_new_policy_table_end"): 14,
                ("fixture_actor", "fixture_actor_end"): 16,
                (
                    "fixture_actor_descriptors",
                    "fixture_actor_descriptors_end",
                ): 3 * 12,
                (
                    "fixture_poison_descriptors",
                    "fixture_poison_descriptors_end",
                ): 12,
                ("state_new_actor_classes", "state_new_actor_classes_end"): 3 * 7,
                ("fixture_request_masks", "fixture_request_masks_end"): 14 * 3,
                ("fixture_result", "fixture_result_end"): 17,
            }
            for (start, end), expected in expected_spans.items():
                with self.subTest(start=start, end=end):
                    self.assertEqual(symbols[end] - symbols[start], expected)

    def test_fail_build_guards_reject_malformed_immutable_spans(self) -> None:
        source = (FIXTURE_DIR / FIXTURE_NAME).read_text()
        descriptor = (
            "    dl fixture_global_enter,fixture_global_update,"
            "fixture_global_render,fixture_global_exit\n"
        )
        actor_descriptor = (
            "    dl fixture_actor_enter,fixture_actor_update,"
            "fixture_actor_render,fixture_actor_exit\n"
        )
        poison_descriptor = (
            "    dl fixture_poison_callback,fixture_poison_callback,"
            "fixture_poison_callback,fixture_poison_callback\n"
        )
        mutations = {
            "global descriptors": (
                descriptor,
                "",
                "fixture_global_descriptor_span_mismatch",
            ),
            "global policy": (
                (
                    "state_new_policy_table:\n"
                    "    db 020h,030h,031h,030h,034h,035h,03fh\n"
                ),
                (
                    "state_new_policy_table:\n"
                    "    db 020h,030h,031h,030h,034h,035h\n"
                ),
                "fixture_policy_span_mismatch",
            ),
            "actor record": (
                "    db 0,0\nfixture_actor_end:",
                "    db 0\nfixture_actor_end:",
                "fixture_actor_size_mismatch",
            ),
            "actor descriptors": (
                actor_descriptor,
                "",
                "fixture_actor_descriptor_span_mismatch",
            ),
            "poison actor descriptors": (
                poison_descriptor,
                (
                    "    dl fixture_poison_callback,fixture_poison_callback,"
                    "fixture_poison_callback\n"
                ),
                "fixture_poison_descriptor_span_mismatch",
            ),
            "actor class registry": (
                (
                    "    db 0\n"
                    "    dl fixture_poison_descriptors\n"
                    "    dl fixture_poison_reconcile\n"
                    "state_new_actor_classes_end:"
                ),
                "state_new_actor_classes_end:",
                "fixture_actor_registry_span_mismatch",
            ),
        }

        for name, (before, after, mismatch_symbol) in mutations.items():
            with self.subTest(malformed=name):
                self.assertEqual(
                    source.count(before),
                    1 if name not in {"global descriptors", "actor descriptors"} else (
                        14 if name == "global descriptors" else 3
                    ),
                )
                malformed = source.replace(before, after, 1)
                with TemporaryDirectory(
                    prefix="nurples-state-core-negative-"
                ) as temp_dir:
                    root = Path(temp_dir)
                    staged_fixture = self.stage_fixture(root, malformed)
                    result = self.assemble_fixture(
                        staged_fixture,
                        root / "malformed.bin",
                    )

                diagnostic = result.stdout + result.stderr
                self.assertNotEqual(result.returncode, 0, diagnostic)
                self.assertIn(mismatch_symbol, diagnostic)

    def test_wrapper_core_and_graph_have_separate_ownership(self) -> None:
        wrapper = (ASM_DIR / "state_new.inc").read_text()
        core = (ASM_DIR / "state_core.inc").read_text()
        graph = (ASM_DIR / "state_graph.inc").read_text()

        self.assertIn('include "state_core.inc"', wrapper)
        self.assertIn('include "state_graph.inc"', wrapper)
        self.assertIn("state_new_descriptors:", wrapper)
        self.assertIn("state_new_actor_classes:", wrapper)

        self.assertIn("state_new_tick:", core)
        self.assertIn("state_new_actor_tick:", core)
        self.assertNotIn("state_new_descriptors:", core)
        self.assertNotIn("state_new_actor_classes:", core)
        self.assertNotIn("STATE_NEW_PLAYER_", core)
        self.assertNotIn("state_new_transition_allowed:", core)

        self.assertIn("state_new_transition_allowed:", graph)
        self.assertNotIn("state_new_tick:", graph)
        self.assertNotIn("state_new_actor_tick:", graph)


if __name__ == "__main__":
    unittest.main()
