# Branch audit

Audit date: 2026-08-02 (America/New_York)

Remote audited:

```text
origin  https://github.com/bgates747/nurples.git
```

The live remote was queried directly with `git ls-remote`; its branch object
IDs matched the existing `origin/*` remote-tracking refs. No branch was checked
out, created, deleted, fetched, pushed, or otherwise modified. The remote's
default branch is `main`.

## Publication result

1. There is exactly one local branch: `main` at `d32dc39`.
2. A branch named `main` is published upstream, so there are **no local-only or
   never-published local branch names**.
3. Local `main` is one commit ahead of `origin/main`. Commit `d32dc39` (`fix
   sound sample duration in ADL mode`, dated 2026-07-31) is therefore not yet
   published, even though its containing branch is published.
4. There is one worktree, at this repository, attached to local `main`.
5. The current uncommitted Python-environment work is working-tree state, not
   another branch, and is outside the branch-topology comparison.

## Upstream branches

### 1. `main`

- Live tip: `2032953` — `delete redundant sprite images` (2025-08-08).
- Relationship: remote default and canonical integration branch.
- Apparent purpose: the main Purple Nurples game line. Its recent history
  contains sprite cleanup, regenerated fonts and buffer-ID fixes, music/SFX
  experiments, and general game development accumulated from the older sprite
  work.

### 2. `sprites`

- Live tip: `04fd357` — `seekers working` (2024-12-18).
- Relationship to `origin/main`: fully contained in `main`; it has no commits
  absent from `main`, while `main` has 88 later commits.
- Apparent purpose: a historical milestone branch for the sprite-system work,
  especially seeker enemies, targeting, collision/explosion behavior,
  fireballs, sprite movement, and related test-level changes. It appears to be
  retained as a named checkpoint rather than an active unmerged line.

### 3. `manysprites`

- Live tip: `fd73a0b` — `demo app progressively spawning 230 seekers`
  (2025-01-02).
- Relationship to `origin/main`: diverged at `4b8e992`; one branch-only commit
  remains outside `main`, and `main` has 13 commits after that base that are not
  on this branch.
- Apparent purpose: a sprite-count stress/demo variant. Its unique commit adds
  a build/archive and `nurplesmany.bin`, changes seeker and collision logic,
  expands the test level, and adds a TMX map so the application can
  progressively spawn roughly 230 seekers.

### 4. `temp`

- Live tip: `c1ae4ff` — `WIP on main: e2a4975 mod dg tiles to have green steel
  bg` (2024-12-10).
- Relationship to `origin/main`: based at `e2a4975`; two stash-style commits
  are reachable only from this branch, while `main` has 157 later commits.
- Apparent purpose: a published Git-stash/WIP snapshot rather than a normal
  development branch. The two commits are named `index on main` and `WIP on
  main`. They preserve in-progress enemy/seeker targeting, sprite, fixed-point,
  player/laser, pad/tile, and green-steel DG artwork changes, along with deleted
  generated listing/binary state.

## Concise topology

```text
e2a4975 --- ... --- 04fd357 (`sprites`) --- ... --- 4b8e992 --- ... --- 2032953 (`origin/main`) --- d32dc39 (`main`)
   \                                                    \
    c1ae4ff (`temp`)                                     fd73a0b (`manysprites`)
```

The diagram is conceptual rather than to scale: `sprites` is an ancestor of
`main`; `manysprites` diverges later from a commit already on `main`; and
`temp` diverges earlier as a stash-shaped snapshot.
