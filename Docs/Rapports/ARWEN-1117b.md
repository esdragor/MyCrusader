# ARWEN-1117b : lot L17-10 (HOLDeployRulesTests.cpp -> script)

Branche prete : d751-root, commit 4e5991a (Script\Tests\HOLDeployRules_Test.as, 1 fichier, sans C++, sans build). Merge main non fait (DLL a jour avec S1f, main inchange depuis 612894d + dfe2a87 docs).

## Registre : test C++ -> test(s) script
| Test C++ (HeroesOfLiberty.Siege.Deploy.*) | Test script (Angelscript.UnitTests.Tests.HOLDeployRules_Test.*) | Retirable |
|---|---|---|
| WhoCanDoWhat | Test_DeployWhoCanDoWhat | oui |
| DelaysAreTheProfiles | Test_DeployDelaysAreTheProfiles | oui |
| AttackOrderDecisions | Test_DeployAttackOrderDecisions | oui |
| MinimumRange | Test_DeployMinimumRange | oui |
| MoveOrderDecisions | Test_DeployMoveOrderDecisions | oui |
| Button | Test_DeployButton | oui |
| Counterweight | Test_DeployCounterweight | oui |
| SequenceFromOrderToShot | Test_DeploySequenceFromOrderToShot | oui |
| EveryCombination | Test_DeployEveryCombination | oui |

Les 9 tests C++ (HOLDeployRulesTests.cpp, 296 l.) sont retirables ; NON supprimes par cet agent (consigne). Le smoke test Test_DeployReadLibrarySmoke (HOLS1fLibraries_Test.as) est redondant avec Test_DeployWhoCanDoWhat, retirable aussi si on veut.

## Non porte
Rien : la bibliotheque expose tout ce que lisent les 9 tests (enums, FHOLDeployProfile/State/Decision, GetTooClose, PhaseAt, DefaultProfile, CanFire/CanMove/CanAct/CanAim, DeployDelayMs/PackDelayMs, Advance, Decide, Toggle, Apply). Helpers C++ reecrits en script (StateOf -> DeployStateOf, MsUntilPhaseChange -> DeployMsUntilPhaseChange). Shortened() est exposee mais n'etait lue par aucun test C++ (non teste directement, couvert via DeployDelayMs).

## Rouge / vert
- Vert d'abord (00:42 -> 00:44) : tests 1385/1387, seuls rouges NomadFirstForum et NomadReplicated (D831).
- Rouge VU (00:45) : une valeur attendue faussee dans chacun des 9 tests (fires->AssertFalse, ElapsedMs 1, Fire->Wait, CanAim min->AssertFalse, March->Wait, Deploy->Pack, DeployMs+1, FiredTooEarly 1, 4*2*8) : tests 1376/1387, les 9 Test_Deploy* en KO + les 2 connus. Bouchon pose et retire dans le worktree Arwen uniquement, hash du fichier restaure identique a celui du vert (0342FF31...).
- Vert final = le run vert ci-dessus (fichier byte-identique).

## Champs
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 (9 retirables, voir registre) ; TESTS_SCRIPT_AJOUTES: 9 (Test_DeployWhoCanDoWhat, DelaysAreTheProfiles, AttackOrderDecisions, MinimumRange, MoveOrderDecisions, Button, Counterweight, SequenceFromOrderToShot, EveryCombination) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 296 (tests portes) ; TYPES_A_EXPOSER: aucun ; EDITS_REFUSES: 0 ; TESTS_TAUTOLOGIQUES: 0 ; PROCESSUS_DETACHES: voir stray_processes (rien de reste attendu).
Docs mis a jour : Auto_Journey_File.md (L17-10 [x]), EtapeDefini.md (1 ligne), AngelscriptInProgress.md (1 ligne), IN_USE.txt du worktree (libre, CPP: non, non suivi par git). Placeholders.txt : aucun provisoire ajoute/retire.
