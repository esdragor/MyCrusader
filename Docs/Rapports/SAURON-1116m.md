# SAURON-1116m : jumeaux ExplorationXP, Deploy, Market (migration D830/D835, sans C++)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `visual`. **Branche prête** (pas de fusion dans main) : a235183, 62cfcda, merge main 3e951c5.

```
APPELS_DECLARES: 41 (arrêt à 45 vus)
HEURE_DEBUT/HEURE_FIN: 20:11 / 20:36
PREMIER_BUILD: sans C++   BUILD: sans C++ (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun commit de rouge seul (L15-5 : un commit vert test + correctif) ; rouges VUS à l'exécution :
  ExplorationXP vu a 20:12:47 : exploration_xp_accord_test.py « 2 XPPerPlace: script 41, C++ 40 <-- FAIL », exit 1 (préréglage B faussé dans le .as, remis)
  Deploy vu a 20:19:13 : deploy_accord_test.py « constant ProposedPackMs: script 5001, C++ 5000 <-- FAIL » ; vu a 20:23:20 : quick_check tests 1325/1328, Test_DeployEveryCombination KO (attendu 4*2*8, remis)
  Market vu a 20:28:57 : quick_check tests 1327/1330, Test_MarketTributeVectors KO (Received(100,30) attendu 71, remis à 70)
  assertit l'effet : oui (valeurs calculées par la règle : or payé, prix du livre, reçu du tribut, décisions)
COMMITS: 2 (a235183 Deploy, 62cfcda Market), dernier à 20:34, merge_main_fait: oui (3e951c5, docs seulement)
TESTS: référence 20:17 tests 1326/1328 + values PASS ; vert final 20:33 tests 1328/1330 + values PASS ; seuls rouges = Match.StartingHero.NomadFirstForum, .NomadReplicated (D831, déjà sur main) ; total +2, jamais en baisse. TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (lots de tests)
QUESTIONS: aucune (le sens des jumeaux est écrit en lignes de bascule, R9)
FAIT / RESTE: voir sections ci-dessous ; reste = décision du sens (utilisateur), branchement de deploy_accord_test.py dans quick_check (agent de fusion, quick_check.ps1 verrouillé à Glorfindel)
NON VÉRIFIÉ: voir section
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
ANOMALIES_PROCESSUS: quick_check passé en arrière-plan par le harnais après 180 s (3 fois), résultat lu dans le fichier de sortie ; WriteAllText sans BOM a retiré le BOM de HOLMarketRules_Test.as, remis avant commit ; 1 Edit refusé « not read »
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, M EtapeDefini.md, M ExplorationXPInProgress.md (mes lignes), ?? rapports d'autres agents + ETAPE0 + Priorite_Socle (déjà là) ; inchangée hors docs voulus : oui
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0 (aucun : la suppression fait partie de la bascule)
TESTS_SCRIPT_AJOUTES: 2 (Test_MarketRulesVectors, Test_MarketTributeVectors) + 1 accord python (Tools\Tests\deploy_accord_test.py)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: ExplorationXP 0 ; Deploy 0 lignes de test (déjà 9/9) + ~120 lignes d'accord python ; Market 86 lignes de script (Market.Rules 92 lignes C++ + 5 assertions de Market.Tribute)
TYPES_A_EXPOSER: ExplorationXP aucun ; Deploy aucun ; Market FHOLMarketRow (ligne de Market.csv, si le script devient la source et lit la table)
EDITS_REFUSES: 1
```

## Durée de chaque lot (Get-Date)
| Lot | Début | Fin | Durée | Dont quick_check |
|---|---|---|---|---|
| MIG-J-ExplorationXP | 20:11 | 20:17 | 6 min | référence tests,values 256 s |
| MIG-J-Deploy | 20:17 | 20:24 | 7 min | rouge 223 s |
| MIG-J-Market | 20:24 | 20:34 | 10 min | rouge 208 s, vert 242 s |

## Registre des tests (C++ / script)
**ExplorationXP** : `HeroesOfLiberty.ExplorationXP.FogPaysTheHero` (monde + Mass, `HOLExplorationXPTests.cpp`) reste en C++, sans équivalent script possible. Script : `Test_ExplorationXPNewTranchePaysOnce`, `PlacePaysOnce`, `StartAndOutside`, `Presets` (règle seule, aucun équivalent C++ pur). Accord : `exploration_xp_accord_test.py` (défauts + 3 préréglages), dans quick_check partie `values` (l. 200-201). MarkCell relu des deux côtés : identique (le C++ garde `IsValidIndex` en plus).

**Deploy** (`HOLDeployRulesTests.cpp` / `Script\Tests\HOLDeployRules_Test.as`), nom à nom, nombre d'assertions C++ / script :
WhoCanDoWhat 12/12 ; DelaysAreTheProfiles 9/9 ; AttackOrderDecisions 8/8 ; MinimumRange 10/10 (mêmes messages, format %d / {Index}) ; MoveOrderDecisions 5/5 ; Button 5/5 ; Counterweight 10/10 ; SequenceFromOrderToShot 6/6 ; EveryCombination 2/2 (mêmes 7 distances, 56 cas). Aucun vecteur manquant. Comme chaque cas est calculé depuis `DefaultProfile()`, l'accord neuf `deploy_accord_test.py` compare les constantes (`Proposed*`, `TooClose`), les champs de `DefaultProfile()` et l'ordre des 3 énumérations (auto-test : 3 mutations refusées).

**Market** (`HOLMarketRulesTests.cpp`, 5 tests) :
| C++ | Script | Sort |
|---|---|---|
| Market.Rules (56 assertions, pur sauf lecture de Market.csv) | Test_MarketRulesVectors (mêmes vecteurs ; les lignes « settings: » tiennent le préréglage C du script aux chiffres que le C++ lit dans la table) | portable, porté |
| Market.Tribute, tête pure (5 assertions) | Test_MarketTributeVectors | porté |
| Market.Tribute, partie monde | — | reste C++ (file d'ordres, pas de production, stocks) |
| Market.Apply (18) | — | reste C++ (lit l'asset de la table ; à reclasser) |
| Market.BookState (9) | — | reste C++ (monde, GameState répliqué) |
| Market.Chain (24) | — | reste C++ (monde, chaîne serveur + IA) |
Les 16 tests script existants restent (préréglages A/B/C, aucun équivalent C++).

## Appelants et fréquence
- ExplorationXP : `UHOLVisionSubsystem::PayExplorationXP` (appelée à la fin de `Update()` serveur et de `UpdateClient()`, qui ne paie pas) ; boucle par camp et par case nouvellement explorée à chaque mise à jour du brouillard. Aucun appelant script hors tests.
- Deploy : Mass, par entité et par pas : `HOLDeployProcessor.cpp` (12), `HOLCombatProcessor.cpp` (3), `HOLUnitOrderProcessor.cpp` (3) ; plus `HOLUnitReplicationRules.h`, `HOLUnitView.h`, `HOLPlayerOrders.h`, `HOLUnitCommand.h`. Aucun appelant script hors tests.
- Market : hors Mass, basse fréquence : `HOLProductionSubsystem.cpp` (l. 195 `Recover` toutes les 10 s, l. 253 `HOLTribute::Received` par ordre, ordres d'échange), `HOLGameState.cpp:1538` (`NewBook` au départ), `HOLGameScreen.cpp:2381` (devis à l'affichage), `HOLDiplomacyReadLibrary.cpp:163`, `HOLPlayerController.cpp`, `HOLAIOrders.cpp`. Côté script : `HOLAIMarket.as` (6), `HOLAIBrain.as` (1) emploient la copie script.

## Lignes de bascule écrites (Auto_Journey_File.md, « Lot C++ de fin de séance »), sens NON tranché (R9)
- MIG-CPP-ExplorationXP : proposé C++ reste, copie script supprimée (sans build) ; accord vert.
- MIG-CPP-Deploy : proposé C++ reste (Mass), copie script supprimée (sans build) ; accord vert + registre 9/9.
- MIG-CPP-Market : proposé script source APRÈS le socle de l'étape 1 (pont serveur) ; autre voie : C++ source + bibliothèque pour l'IA ; tests d'accord verts.

## NON VÉRIFIÉ
- L'appelant serveur de `UHOLVisionSubsystem::Update()` n'a pas été trouvé par grep (hors tests) : la cadence « une fois par mise à jour du brouillard » est déduite, non mesurée.
- `deploy_accord_test.py` n'est pas dans quick_check (ligne pour l'agent de fusion) ; il compare constantes, profil et énumérations, pas les corps de fonction (l'accord de comportement repose sur les 9 tests aux mêmes vecteurs des deux côtés).
- `Market.Apply` classé « reste C++ » sur son commentaire (lit l'asset), corps non lu ; visibilité de `FHOLMarketRow` au script non vérifiée.
- Aucun jeu lancé, aucun essai.
```
