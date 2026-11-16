# CELEBORN-1117b : retrait des tests C++ portes (L17-01, L17-04, L17-03)

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, branche hero-j8-close, commit 75f375a ("Remove C++ tests ported to script (L17-01, L17-04, L17-03)"). Branche prete, merge main non fait. NON COMPILE, quick_check non lance (consigne).

## Registre : test C++ supprime (fichier::nom) -> equivalent (fichier::nom)

NOMBRE EXACT RETIRE : 9 tests C++ (le total de quick_check doit baisser de exactement 9 ; aucun test script ajoute dans ce lot).

| # | Test C++ supprime | Equivalent |
|---|---|---|
| 1 | HOLDiplomacyRulesTests.cpp::HeroesOfLiberty.Diplomacy.Rules | Script\Diplomacy\HOLDiplomacyRules_Test.as::Test_DiplomacyRulesVectors + ::Test_DiplomacyRulesVectorsGarrisonVictory (rouge vu ELROND-1117a 1362/1366) |
| 2 | HOLGameMenuTests.cpp::HeroesOfLiberty.Victory.GameMenu.Escape | Script\UI\HOLGameMenuRules_Test.as::Test_GameMenuEscape (rouge vu GALADRIEL-1117a 1365/1368) |
| 3 | HOLGameMenuTests.cpp::HeroesOfLiberty.Victory.GameMenu.Surrender | HOLGameMenuRules_Test.as::Test_GameMenuCanSurrender |
| 4 | HOLGameMenuTests.cpp::HeroesOfLiberty.Victory.GameMenu.QuitToMenu | HOLGameMenuRules_Test.as::Test_GameMenuCanQuitToMenu |
| 5 | HOLGameMenuTests.cpp::HeroesOfLiberty.Victory.GameMenu.StatsButton | HOLGameMenuRules_Test.as::Test_GameMenuStatsButton |
| 6 | HOLGameMenuTests.cpp::HeroesOfLiberty.Victory.GameMenu.Input | HOLGameMenuRules_Test.as::Test_GameMenuKeepsInput (rouge vu GALADRIEL-1117a) |
| 7 | HOLGameMenuTests.cpp::HeroesOfLiberty.Victory.EndBanner.Which | Script\UI\HOLEndBannerScript_Test.as::Test_EndBannerWhich (8 GetEndBanner) + HOLGameMenuRules_Test.as::Test_GameMenuReturnToLobbyAndList (7 OffersReturnToLobby + 3 QuitsToList) |
| 8 | HOLGameMenuTests.cpp::HeroesOfLiberty.Victory.EndBanner.Closed | HOLEndBannerScript_Test.as::Test_EndBannerClosed (7 vecteurs) |
| 9 | HOLTechnologyTableTests.cpp::HeroesOfLiberty.Technologies.Table | Tools\Tests\technology_table_test.py (quick_check -Parts values ; rouge vu CELEBORN-1117a : PloughII absent -> FAIL 5, MinerPicksI Percent 0 -> FAIL 1) |

Fichiers supprimes en entier (git rm) : HOLDiplomacyRulesTests.cpp (151 l.), HOLGameMenuTests.cpp (124 l.). HOLTechnologyTableTests.cpp : retrait du test Table et de ses aides propres (Grid, FGridLine, DawnOwn, PlannedBuildings, CodedStats, KnownStats, template Contains) ; Flags, Import, Playable et Rules intacts. Lignes C++ supprimees : 462 (3 insertions : le commentaire d'en-tete).

## Verification des 5 equivalences de Galadriel (lecture, par vecteur)
Les 5 (Which, Closed, Surrender, QuitToMenu, StatsButton) ont ete relues ligne a ligne contre le script : memes arguments, memes attendus.
- Which : les 8 GetEndBanner avec masques 0b10/0b01/0b101/0b11 = 2/1/5/3, INDEX_NONE = -1 ; les 7 OffersReturnToLobby et 3 QuitsToList identiques (enums EHOLEndBanner/EHOLMatchOrigin -> EHOLGameMenuBanner/EHOLGameMenuOrigin).
- Closed : 7 ShouldOpenEndBanner identiques. Surrender, QuitToMenu, StatsButton : 4 vecteurs chacun identiques.
- Diplomacy : les masques de DiploVecFlags (12, 18, 8, 4, 0, 2, 3, 1, 6, 14) correspondent aux tableaux de booleens C++ ; Winners "0,1,2", "0", "1", "0,1" identiques.
Limite : ces 5 reposent sur la lecture, pas sur un rouge separe (les rouges des lots 1116m/1116r les couvrent). Rien n'a ete garde par doute.

## Gardes (non supprimes)
- HeroesOfLiberty.Technologies.Playable (IsPlayable recopie en python, 2 criteres : pas de test d'accord, recommandation CELEBORN-1117a) ; Technologies.Rules (UDataTable / Mass / file de production).
- HOLSiegeTests.cpp (ARWEN-1117a : Combat ChooseTarget, Health GetMaxHealth/GetDamage, Table restent C++) : hors de ce lot.

## A lancer par la session principale
1. Build : `Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree Celeborn (des fichiers de test C++ disparaissent).
2. `Tools\Tests\quick_check.ps1 -Parts tests,values` : le total C++ doit baisser de exactement 9 (comparer au total avant, ex. 1367/1369 -> 1358/1360 si seuls ces 9 tests passaient).

## Champs
BUILDS_DEMANDES: 0 (a lancer par la session principale)
TESTS_CPP_RETIRES: 9 (Diplomacy.Rules, Victory.GameMenu.Escape/Surrender/QuitToMenu/StatsButton/Input, Victory.EndBanner.Which/Closed, Technologies.Table)
TESTS_SCRIPT_AJOUTES: 0
LIGNES_CPP_SUPPRIMEES: 462
LIGNES_MIGREES_PAR_LOT: 0
TYPES_A_EXPOSER: aucun
TEST_ROUGE: non refait ici (rouges des lots 1116m/r, 1116t/u, 1117a ; Table : python 1117a)
TESTS_TAUTOLOGIQUES: 0
EDITS_REFUSES: 0
PROCESSUS_DETACHES: voir stray_processes (aucun lance)
NON VERIFIE : compilation (aucune), quick_check ; le total exact apres retrait.
Placeholders.txt : aucun provisoire ajoute ni retire.
