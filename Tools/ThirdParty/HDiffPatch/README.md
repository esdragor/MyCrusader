# HDiffPatch 5.1.3

Binary patches of the game files between two published versions (launcher, L-Q2 in `LauncherInProgress.md`).

- Source: official release `v5.1.3` of https://github.com/sisong/HDiffPatch (2026-07-31), Windows 64-bit
  binaries archive, folder `windows64/`, binaries unchanged. License: MIT (`LICENSE`).
- `hdiffz.exe`: makes the patches at publication (`Manager/HOLPublish`, called with
  `-m-6 -SD -c-zstd-21-24 <old> <new> <patch>`); never shipped to players.
- `hpatchz.exe`: applies a patch (`hpatchz <old> <patch> <new>`); HOLPublish checks every patch with it, and it is
  shipped with the launcher (L6), which runs it as a process.

Measured in L3: game exe (332 MB) patch 8.3 MB in 38 s, applied in 0.3 s with ~30 MB of memory.
