# Production and test pacing separation

Author requests normal single-vblank production Nurples at `/mystuff/nurples`,
and a separate two-vblank30fps review build at `/test/nurples`. Restored the
production source and assembled binary/symbol pair. Current dirty repair game/UI
assets remain authoritative for this deployment and are preserved uncommitted.
No asset regeneration or emulator changes. The test build is the exact executable
from dev2f0f095, with independent copies of those same current runtime assets.
Cross-project deployment evidence: agon-extender BENCH-002 follow-up, and bounded
cadence investigation BENCH-003. Production Rally remains untouched; its test
installation belongs in `/test/arcade/rally`.
