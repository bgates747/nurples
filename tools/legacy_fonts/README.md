# Legacy font experiments

These project-specific scripts were preserved from the obsolete embedded
`agonutils/` tree when Purple Nurples migrated to the canonical native
extension. They are not installed by `nurples-tools`, are not part of the
current build, and still contain stale `src/assets/...` paths.

Before reactivating one, replace its hard-coded paths, remove any shell-based
deletion, and add a focused test. Shared image-format behavior belongs in
`/home/smith/Agon/mystuff/agon-utils`, not here.
