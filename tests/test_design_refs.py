from pathlib import Path
import unittest
from nurples_tools.design_refs import validate


class DesignReferences(unittest.TestCase):
    def test_checkout(self):
        validate(Path(__file__).resolve().parents[1] / "level-design")
