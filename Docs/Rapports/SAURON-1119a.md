# SAURON-1119a: lot C1 (cheat and victory consoles to script), UNCOMPILED

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, branch envelope, commit 4fbd19d (3 files; nothing of SAURON-1119b touched). Merge main not done.
BUILDS_DEMANDES: 0. TESTS_CPP_RETIRES: 0. TESTS_SCRIPT_AJOUTES: 6. LIGNES_CPP_SUPPRIMEES: 197. LIGNES_MIGREES_PAR_LOT: ~115 C++ lines -> 306 script lines (with tests 456). TYPES_A_EXPOSER: none for the 16 ported; for the 4 left see below.

## Commands ported (16 of the 20 in the cheat blocks)
hol.Cheat.Resources, Food, Wood, Stone, Gold, FastBuild, Population, Wound, Technology, Tier, UnitSpeed, GatherSpeed; hol.Victory.Defeat, FallForum, SkipRuin, HoldPresence.
Same names, same arguments and defaults, same "Usage: ..." lines, same RPC (AHOLRpcPlayer.ServerCheat<X>, found by HOLRpcBridge::FindPlayerActor like CallFull did).

## Commands remaining in C++ (and why)
- hol.Cheat.DamageNear, hol.Cheat.SpawnBuilding: need the C++ `GetPointNear` (main centre or enemy centre, frame offset).
- hol.Cheat.DamageSelected: needs `Controller->GetSelection()`.
- hol.Units.SpawnHere: needs `GetRTSCamera()`, unit subsystem (also not an RPC command).
- (hol.Victory.Surrender, line ~1597 before removal, is outside the 22 cheats: untouched.)
Minimal C++ binding to finish them (one lot): a `UFUNCTION(ScriptCallable)` on a library or the controller: `static bool GetPointNear(AHOLPlayerController*, const TArray<FString>& PointArgs, FVector& OutPoint)`, `AHOLBuilding* AHOLPlayerController::GetSelection() const` exposed, and for SpawnHere `GetRTSCamera()->GetFocus()`. Then the 3 cheats become plans like the others (ServerCheatDamageBuilding(Amount, AttackType, Point), ServerCheatSpawnBuilding(Type, Slot, Point)).

## Design
- Script\Player\HOLControllerCommands.as (NOT Script\Dev: skipped in -game/-server). `const FConsoleCommand X("hol...", n"Handler")` pattern, same as Script\Audio\HOLAudioSelectGain.as, so scripts CAN register console commands with arguments: no C++ binding needed for the 16.
- Pure layer `HOLControllerCommands::PlanOf(Command, Args)` -> FHOLCheatPlan (route, ints, real, name, usage) is what the test checks; `Send` calls the RPC; `Run` loops over local controllers (GetAllActorsOfClass + IsLocalController, current world only; the C++ looped over every game world: only matters for PIE with several worlds).
- Guard: the C++ was under `#if !UE_BUILD_SHIPPING`. The script has NO such guard (I did not verify an AS equivalent). The server refuses every cheat unless cheats are active (HOLRpcBridge::CheatsAllowed in each ServerCheat<X>), and the C++ Do<X> bodies stay `#if !UE_BUILD_SHIPPING`: in Shipping the commands exist but do nothing.
- Differences (not byte-identical): console help texts are lost (FConsoleCommand has no help string); "Usage:" lines were UE_LOG Warning, now `Log` (Display) with LogHOLMatch; the "RpcRoute: X not sent: ..." line was Error, now Display and with the RPC function name (e.g. ServerCheatResources) instead of the short name (CheatResources) as a suffix.
- FString parsing (no known AS FCString): own ParseInt / ParseReal / IsNumber (Atoi / Atof / IsNumeric without exponent).
- The C++ RequestCheat<X> methods and the .h are untouched: HOLCheatPanel.cpp (HeroesOfLibertyUI, lines 300-346) and DamageNear/SpawnBuilding/DamageSelected still call them. The block comment above the cheats (line 1877) is now partly stale.

## Tools reading the commands / log lines
| tool (file:line) | read | where script writes it |
|---|---|---|
| Tools\Scenarios\*.txt (~223 files in Tools, 822 occurrences of hol.Cheat.* / hol.Victory.* / hol.Units.SpawnHere; e.g. ai_evolving.txt 18, building_attack.txt 14) | the command names | registered under the same names (const FConsoleCommand, one per name) |
| Script\Net\HOLRpcPlayer.as:537-693 (proof actor `Commands.Add("hol.Cheat...")`) | hol.Cheat.Resources/FastBuild/Population/Technology/Tier/UnitSpeed/GatherSpeed/Wound/DamageNear/SpawnBuilding, hol.Victory.* | same names; the 3 C++ ones stay C++ |
| rpc_cheats_run.ps1 (NOT at Tools\Tests in this worktree; I did not locate it, see NON VERIFIE) | "RpcCheat: ... received by the server" lines | unchanged, written by ServerCheat<X> in HOLRpcPlayer.as |
| any tool grepping "Usage: hol.Cheat/Victory" | none found in Tools (grep empty) | HOLControllerCommands.as PlanOf (Log, Display level) |

## Compilation suspects (script, never compiled)
1. `FString::Mid`, `Len`, `ToLower`, `StartsWith`, `Contains`, `IsEmpty`, `TArray<FString>::Contains` assumed to exist as in C++ (used in tests and the new file).
2. Default arguments of the helper `CheatArgs(const FString& A = "", ...)` in the test file (global function in a test file; no other file calls it, but a global of the same name elsewhere would clash).
3. `struct FHOLCheatPlan` at file scope used by the test file in another file: global structs are shared, global functions are not (hence namespace HOLControllerCommands): check that `FHOLCheatPlan` is visible from the test module.
4. Local `Plan` variables in `PlanOf` and `Run` (function name PlanOf chosen to avoid the shadowing).
5. `Rpc.ServerCheat<X>(...)` called directly from script on a UFUNCTION(Server) of AHOLRpcPlayer; `float(Plan.Real)` for the float parameters; `FName(Plan.Name)`.
6. `GetAllActorsOfClass(Controllers)` in a global function reached from a console command (same pattern as HOLBiomeDecorAudit.as:13, worked there).
7. Constants of the 16 `const FConsoleCommand` have unique names: no clash with other files expected (HOL<Cheat|Victory>...Console).

## Red to place once
In Test_ControllerCommandsResourcesDefaultsAndMasks change `T.AssertEquals(1000, All.Ints[0], ...)` to 999 (or in HOLControllerCommands.as ResourcesPlan the default 1000 to 999): expect 1 failing test of 6 (Test_ControllerCommandsResourcesDefaultsAndMasks); restore, then green. Second option: Population default 165 -> 164 fails Test_ControllerCommandsScalarCheats.

## Validation for the main session
1. Build (the C++ lost 197 lines of registrations: nothing else must change): `<worktree>\Tools\build.ps1 -LiveCoding=0 -Wait`.
2. `quick_check.ps1 -Parts tests,net` (6 new script tests: Test_ControllerCommandsEachNameOnceAndRouted, ...ResourcesDefaultsAndMasks, ...ScalarCheats, ...TechnologyAndTier, ...Victory, ...Parsing).
3. rpc_cheats_run.ps1 (find with `Get-ChildItem Tools -Recurse -Filter rpc_cheats_run.ps1`): all 13 bits; the 3 C++ commands (DamageNear, SpawnBuilding) must still pass since they stay.
4. Check at the console in a -game run that `hol.Cheat.Resources 100` and `hol.Victory.Defeat 1` are listed once (no double registration: the C++ ones are gone).

## Docs
AngelscriptInProgress.md: 1 line appended (line 322). EtapeDefini.md, Auto_Journey_File.md, Auto_Journey_Retours.md, Placeholders.txt, IN_USE.txt not touched. Placeholders: none added or removed.

## NON VERIFIE
Everything (no build, no quick_check). Not opened: rpc_cheats_run.ps1, quick_check.ps1 (path), the plugin's FString binding, any `#if` AS equivalent of UE_BUILD_SHIPPING. PROCESSUS_DETACHES: none started.
