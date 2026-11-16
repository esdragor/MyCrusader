# SAURON-1116o (visual, sans C++) : D836 point 1, suppression des copies script Deploy et ExplorationXP

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `visual`. **Branche prête** (pas de fusion dans main) : 885de4f, merge main e50d06a (docs et brief_commun.md seulement).

```
APPELS_DECLARES: 21 (arrêt à 45 vus)
HEURE_DEBUT/HEURE_FIN: 21:29 / 21:38 (consigne du coordinateur : fini avant 21:50)
PREMIER_BUILD: sans C++   BUILD: sans C++ (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (suppression, consigne de l'énoncé : pas de rouge ; preuve = grep sans appelant + quick_check avant/après + registre) ; assertit l'effet : sans objet
COMMITS: 1 (885de4f) à 21:36, merge_main_fait: oui (e50d06a)
TESTS: avant 21:29-21:32 `quick_check -Parts tests,values` : values PASS, tests 1352/1354 (rouges connus : Match.StartingHero.NomadFirstForum, .NomadReplicated, D831), red summary aucune régression ; après 21:33-21:36 : values PASS, tests 1339/1341 (mêmes 2 rouges connus), red summary aucune régression. Total -13 = exactement les 13 tests script retirés. TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune (sens tranché par D836)
FAIT / RESTE: voir sections ; reste : rien de la tâche. Branche prête.
NON VÉRIFIÉ: voir section
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
ANOMALIES_PROCESSUS: les deux quick_check sortent en code 1 (attendu : partie tests rouge connue) ; aucune autre
GIT_STATUS_PRINCIPALE: M Auto_Journey.md, M Auto_Journey_File.md, M Auto_Journey_Jour.md, M EtapeDefini.md, M Tools/AutoJourney/brief_commun.md, ?? Docs/Priorite_Socle_Factions_2026-11-16.md, ?? Docs/Rapports/ELROND-1116n.md (+ ce rapport) ; inchangée hors docs voulus : oui (mes lignes : Auto_Journey_File.md l. 75-76, EtapeDefini.md l. 7)
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 0 (13 retirés, voir registre)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: 0 ; supprimées : 757 lignes de script (252 + 288 + 126 + 91), 242 lignes de python (117 + 125), 6 lignes de quick_check.ps1
TYPES_A_EXPOSER: aucun
EDITS_REFUSES: 0
```

## Fait (885de4f)
- Supprimés : `Script\Siege\HOLDeployRules.as`, `Script\Tests\HOLDeployRules_Test.as`, `Tools\Tests\deploy_accord_test.py`, `Script\Heroes\HOLExplorationXPRules.as`, `Script\Heroes\HOLExplorationXPRules_Test.as`, `Tools\Tests\exploration_xp_accord_test.py`.
- `Tools\Tests\quick_check.ps1` : seulement les 2 blocs de 3 lignes (commentaire + appel python + `$ValuesChecks`) de la partie values (ex-l. 205-207 et 211-213). Rien d'autre touché (zone netsig3 de GLORFINDEL-1116n intacte).
- Docs : `AngelscriptInProgress.md` (1 ligne, commitée), `Auto_Journey_File.md` 2 lignes `[x] 885de4f D836`, `EtapeDefini.md` 1 ligne, `IN_USE.txt` occupé puis libre.

## Preuve « sans appelant » (Grep avant suppression)
- `Script\` : `HOLDeploy::`, `HOLExplorationXP::`, `FHOLDeployProfile/State/Decision`, `EHOLDeployPhase/Order/Action`, `FHOLExplorationXPSettings/State`, `FHOLExplorationPlace` : présents dans les 4 fichiers supprimés seulement (214 occurrences, 0 ailleurs).
- `Source\`, `Tools\`, `Config\` (noms de fichiers `.as`, `deploy_accord`, `exploration_xp_accord`) : seulement des commentaires C++ (`HOLDeployRules.h:8`, `HOLDeployRules.cpp:5`, `HOLExplorationXPRules.h:8`, `HOLDeployRulesTests.cpp:9`) et l'appel de quick_check.ps1 retiré.

## Registre des tests script retirés (13) et test C++ qui couvre la même règle
| Test script retiré | Test C++ restant |
|---|---|
| Test_DeployWhoCanDoWhat | HeroesOfLiberty.Siege.Deploy.WhoCanDoWhat |
| Test_DeployDelaysAreTheProfiles | HeroesOfLiberty.Siege.Deploy.DelaysAreTheProfiles |
| Test_DeployAttackOrderDecisions | HeroesOfLiberty.Siege.Deploy.AttackOrderDecisions |
| Test_DeployMinimumRange | HeroesOfLiberty.Siege.Deploy.MinimumRange |
| Test_DeployMoveOrderDecisions | HeroesOfLiberty.Siege.Deploy.MoveOrderDecisions |
| Test_DeployButton | HeroesOfLiberty.Siege.Deploy.Button |
| Test_DeployCounterweight | HeroesOfLiberty.Siege.Deploy.Counterweight |
| Test_DeploySequenceFromOrderToShot | HeroesOfLiberty.Siege.Deploy.SequenceFromOrderToShot |
| Test_DeployEveryCombination | HeroesOfLiberty.Siege.Deploy.EveryCombination |
| Test_ExplorationXPNewTranchePaysOnce | HeroesOfLiberty.ExplorationXP.FogPaysTheHero (monde) |
| Test_ExplorationXPPlacePaysOnce | HeroesOfLiberty.ExplorationXP.FogPaysTheHero (monde) |
| Test_ExplorationXPStartAndOutside | HeroesOfLiberty.ExplorationXP.FogPaysTheHero (monde) |
| Test_ExplorationXPPresets | aucun équivalent C++ (préréglages A/B/C, script seulement) |

Deploy : registre 9/9 nom à nom (mêmes vecteurs, SAURON-1116m). Accords python retirés aussi : `deploy_accord_test.py`, `exploration_xp_accord_test.py` (checks de la partie values, hors compte des tests).

## NON VÉRIFIÉ
- ExplorationXP : `HOLExplorationXPTests.cpp` n'a qu'UN test (FogPaysTheHero, monde + Mass) ; je n'ai pas relu son corps pour vérifier qu'il couvre « une tranche paie une fois », « un lieu paie une fois », « départ et hors carte ». Les préréglages A/B/C (Test_ExplorationXPPresets) n'ont plus aucun test : couverture perdue, acceptée par D836.
- Les commentaires C++ qui citent encore la copie script (4 lignes listées ci-dessus) sont périmés ; `Source\` interdit à ce lot : à corriger par le prochain lot C++.
- Aucun jeu lancé.
