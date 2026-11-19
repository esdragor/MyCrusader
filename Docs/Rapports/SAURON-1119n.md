# SAURON-1119n: C1 step 5 follow-up (4 console commands to script), PARTIAL, UNCOMPILED

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, branch envelope. Stopped at the tool-call cap (25 exceeded: too much reading of the 4 commands, their tools and the test file), time 09:18.
BUILDS_DEMANDES: 0 (but a C++ lot is now pending: see below). TESTS_CPP_RETIRES: 0. TESTS_SCRIPT_AJOUTES: 0. LIGNES_CPP_SUPPRIMEES: 0 (a removal of 101 lines was made then REVERTED, see below). TYPES_A_EXPOSER: none more.

## Done
- Commit dd98d3c (2 files, +117): `Source\HeroesOfLiberty\Player\HOLControllerReadLibrary.h/.cpp`, new `UHOLControllerReadLibrary` (script: `HOLControllerRead::`). No existing library exposed these (searched Player\*ReadLibrary.h: only CommandGrid and SignalInput). Signatures (all BlueprintCallable, out value by `FVector&`, return false = nothing there):
  - `static bool PointNearMainCentre(AHOLPlayerController* Controller, double Dx, double Dy, bool bEnemy, FVector& OutPoint)` (the C++ GetPointNear: centre location + centre rotation applied to (Dx, Dy, 0), Z = 0)
  - `static bool SelectedBuildingLocation(AHOLPlayerController* Controller, FVector& OutLocation)` (GetSelection()->GetActorLocation())
  - `static bool CameraFocus(AHOLPlayerController* Controller, FVector& OutFocus)` (GetRTSCamera()->GetFocus())
  - `static bool SpawnUnitsAround(AHOLPlayerController* Controller, int32 Count, FName TypeName, int32 Slot, double Radius, const FVector& Center)` (false on a client / no UHOLUnitSubsystem; else RequestUnits with Center Z = 0): the 4th binding is needed because SpawnHere has no script route (no RPC; it spawns through the subsystem on the server or solo).
  - plain C++ `static const AHOLBuilding* FindMainCentre(const AHOLPlayerController&, bool bEnemy)` (single source of the main-centre search).
- NOT compiled (so `FHOLBuildingIterator` / `HOLBuildingTypeNames::MainCentre` includes: HOLBuilding.h, HOLBuildingTypes.h; `GetHOLPlayerState` assumed public as used from the lambdas).

## Not done (state left coherent: HOLPlayerController.cpp is UNCHANGED, the 4 C++ commands still exist and work)
I had edited HOLPlayerController.cpp (removed lines 1876-1976: the `#if !UE_BUILD_SHIPPING` block with ParseAttackType and the 4 FAutoConsoleCommand, 101 lines; FindMainCentre body 10 lines replaced by a call to UHOLControllerReadLibrary::FindMainCentre; one #include) then reverted it with `git checkout` because the script half was not written. ParseAttackType is used only by those commands (safe to remove).

## Plan for the script half (ready to write, not written)
Extend `FHOLCheatPlan` with `PointSource` ("near" / "selection" / "camera"), `Dx`, `Dy`, `bEnemy`; `Send` gets a `const FVector& Point`; `Run` resolves the point per local controller with HOLControllerRead:: before Send (skip silently when false, as the C++ did), SpawnHere logs the C++ warning when CameraFocus or SpawnUnitsAround is false.
| command | refusal (exact C++ line) | route / args |
|---|---|---|
| hol.Cheat.DamageNear | Args < 3: `Usage: hol.Cheat.DamageNear <Amount> <Dx> <Dy> [enemy|own] [melee|ranged|siege|hero]` | ServerCheatDamageBuilding(Amount=Atoi(A0), AttackType(A4), Point near; Dx=A1, Dy=A2, enemy = A3 == "enemy" case-sensitive) |
| hol.Cheat.SpawnBuilding | Args < 4: `Usage: hol.Cheat.SpawnBuilding <Type> <Slot> <Dx> <Dy> [enemy]` | ServerCheatSpawnBuilding(Name=A0, Slot=Atoi(A1), Point near; Dx=A2, Dy=A3, enemy = A4 == "enemy") |
| hol.Cheat.DamageSelected | none | ServerCheatDamageBuilding(Amount = A0 or 100, AttackType(A1), Point = selected building location); skipped when nothing selected |
| hol.Units.SpawnHere | none; warning `hol.Units.SpawnHere: on the server or in solo, once the match runs` when no camera/subsystem | no RPC: SpawnUnitsAround(Count A0 or 5, Type A1 or Spearman, Slot A2 or 1, Radius A3 or 600, Center = camera focus XY); journal `SpawnHere: %d %s of slot %d around (%.0f, %.0f)` (use Math::RoundToInt for the %.0f) |
AttackType: ranged 1, siege 2, hero 3, else melee 0 (EHOLAttackType order, name compared lowercase).
Server refusal without cheats: already in ServerCheatDamageBuilding / ServerCheatSpawnBuilding (HOLRpcPlayer.as:1071-1095); SpawnHere has none (as before: it never went through the cheat check).

## Journal lines read by tools (grep done)
`SpawnHere: N <Type> of slot S` is read by `waitlog` in many Tools\Scenarios\*.txt (arquebusier, cavalry, fords_group, legions_roster..., 309 occurrences of hol.Units.SpawnHere in 111 scenarios): the new script line must keep the prefix `SpawnHere: ` exactly (the `around (x, y)` suffix is not matched by what I saw). No tool matched the Usage lines, nor `RpcCheat: damage building ...` / `spawn building ...` (already script lines, unchanged), nor `RpcRoute: CheatDamageBuilding not sent` (script version will read `RpcRoute: ServerCheatDamageBuilding not sent...`, as for the 16 ported).

## Tests to add (Script\Player\HOLControllerCommands_Test.as, one per command, via PlanOf)
Test_ControllerCommandsDamageNear (refusal with 2 args gives the usage; route + Amount + AttackType index + Dx/Dy/bEnemy), ...SpawnBuilding (usage < 4; Name, Slot, point params), ...DamageSelected (default 100, attack type from A1, source "selection"), ...SpawnHere (defaults 5 / Spearman / 1 / 600, source "camera"), plus the 4 names added to `Names()` (the existing test then counts 20). Red to place: change the SpawnHere default 5 to 4.

## NON VERIFIE
Everything: no build, no quick_check. The library was never compiled. Command to run after the script half: `<worktree>\Tools\build.ps1 -LiveCoding=0 -Wait` then `quick_check.ps1 -Parts tests,net`, then a scenario with SpawnHere (e.g. arquebusier) to see the `SpawnHere:` line.
PROCESSUS_DETACHES: none. EDITS_REFUSES: 0.
