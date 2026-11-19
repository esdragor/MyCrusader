# ELROND-1119w : correcteur de script, integ-15 (integration n°3)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche `integ-15`, commit `47a4dbf` (branche prete, rien pousse).

## Resultat
`quick_check -Parts tests` : **1724/1724 PASS** (172 s, 2e passage). 1er passage : 0/0, une seule erreur Angelscript.

## Corrections (1)
- `Script\Commands\HOLPlayerOrdersBoard.as` (56:10) : `No matching signatures to 'HOLCommandScript::GetCommands(UHOLPlayerOrdersBoardScript)'`.
  Cause : `UHOLCommandScriptLibrary::GetCommands` est declaree avec `meta = (WorldContext = "WorldContextObject")` (`HOLCommandScriptLibrary.h:54`) ; en
  Angelscript le parametre de contexte monde est cache et rempli par le script. Changement : `GetCommands(this)` -> `GetCommands()`. Le sous-systeme monde
  (UWorldSubsystem) fournit son propre monde.

## BLOCKED / TYPES_A_EXPOSER
Aucun. Aucune fonction C++ manquante : la planche script P1b peut fonctionner sans C++ neuf (a verifier en jeu : ce rapport ne prouve que la compilation et le test d'accord).

## Echecs restants
Aucun.

## Points surveilles
- P1b : les 5 tests `HOLPlayerOrdersBoard_Test` (AgreesAttackKinds, AgreesIsAccepted, AgreesMove, AgreesRefusals, AgreesStopHold) : Success.
- Garde spectateur C2 : `Test_ControllerCommandsC2SpectatorSendsNothing` (HOLControllerCommandsC2_Test) : Success.
- **MarchPairNoTremble** : C++ `HeroesOfLiberty.Movement.MarchPairNoTremble` Success ; miroir script `Test_AvoidMarchPairNoTremble` (HOLAvoidRules_Test) Success. Valeur par defaut `hol.Units.MarchMemberDetourSteps = 2` (`HOLSimulationProcessors.cpp:172`) : le test est donc vert a 2.
- Aucune erreur de fusion vue (aucun script du reste de l'integration n'a echoue).

## Anomalies
Aucune. `stray_processes.ps1 -Stop` : aucun processus detache.
