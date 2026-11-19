# SAURON-1119p: C1 step 5 follow-up, 4 console commands to script (UNCOMPILED)

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, branch envelope. Resumes SAURON-1119n (library commit dd98d3c).
BUILDS_DEMANDES: 0 (one C++ lot pending: dd98d3c library + the cpp edit below). Nothing built, nothing run.

## Steps
- Step 2+3, commit d52fcf6 (Script\Player\HOLControllerCommands.as, HOLControllerCommands_Test.as): hol.Cheat.DamageNear, hol.Cheat.SpawnBuilding, hol.Cheat.DamageSelected, hol.Units.SpawnHere. FHOLCheatPlan gets PointSource ("near" / "selection" / "camera"), Dx, Dy, bEnemy; Send takes a `const FVector& Point`; Run resolves the point per local controller with HOLControllerRead::PointNearMainCentre / SelectedBuildingLocation / CameraFocus (skip silently when none, as the C++). SpawnHere = Route "SpawnHere" handled in Send through HOLControllerRead::SpawnUnitsAround (no RPC). Names() now 20.
- Step 4, second commit (see git log, on top of d52fcf6): HOLPlayerController.cpp: the `#if !UE_BUILD_SHIPPING` block (ParseAttackType + the 4 FAutoConsoleCommand, plus its trailing blank line) removed = 101 lines; FindMainCentre body (10 lines) replaced by `UHOLControllerReadLibrary::FindMainCentre(...)` (-9 net) + 1 include. diff: 2 insertions, 110 deletions. RequestCheatDamageBuilding / RequestCheatSpawnBuilding kept: still called by Source\HeroesOfLiberty\Dev\HOLBuildingCrowdTest.cpp:682,685,795. GetPointNear (line ~1207) kept: still used by hol.Camera.FocusNear / Build.PlaceNear etc. ParseAttackType had no other caller.

## Tests added (HOLControllerCommands_Test.as)
Test_ControllerCommandsDamageNear, Test_ControllerCommandsSpawnBuilding, Test_ControllerCommandsDamageSelected, Test_ControllerCommandsSpawnHere. Existing Test_ControllerCommandsEachNameOnceAndRouted updated: count 16 -> 20, and SpawnHere exempted from the "StartsWith ServerCheat" route assertion (it has no RPC). CheatArgs got a 5th parameter E (SpawnBuilding's `enemy`).

## Journal deviations
- `SpawnHere: N <Type> of slot S around (x, y)` kept (prefix and N/Type/slot format identical); x, y by Math::RoundToInt (C++ %.0f: possible 1-unit rounding difference on .5, not read by any tool).
- Verbosity: C++ logged SpawnHere Display and the failure/usage lines Warning; script uses Log(n"LogHOLMatch", ...) for all (as the 16 already ported). The log category and text are unchanged.
- Usage lines and the SpawnHere warning text identical to the C++. Without active cheats the refusal is the server's (ServerCheatDamageBuilding / ServerCheatSpawnBuilding); SpawnHere never had one.
- Parsing: Dx/Dy by ParseReal (Atod), amount/slot by ParseInt (Atoi), `enemy` compared case-sensitive, attack type lowercase: as C++. SpawnHere slot cast to uint8 in the C++ (Slot > 255 wraps there; script passes int32 to the library).

## NON VERIFIE
Everything (no build, no script compile, no test). Risks to look at first: the library (dd98d3c) includes and `GetHOLPlayerState` access; the f-string with `Math::RoundToInt(...)` calls inside braces; `FVector Point;` out param binding to `FVector&` of HOLControllerRead::; HOLControllerReadLibrary.h FindMainCentre being plain C++ (no UFUNCTION) in a UCLASS header; script namespace name HOLControllerRead.
Commands after the build: `<worktree>\Tools\build.ps1 -LiveCoding=0 -Wait` then `quick_check.ps1 -Parts tests,net`, then a scenario with SpawnHere (e.g. arquebusier) to see `SpawnHere: N Type of slot S`; a DamageNear/SpawnBuilding scenario (ecran / building crowd) for the RpcCheat lines.
Placeholders.txt: not touched (nothing provisional added or removed). EtapeDefini.md, Auto_Journey_*, IN_USE.txt not edited.
