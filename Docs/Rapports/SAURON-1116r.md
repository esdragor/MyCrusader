# SAURON-1116r - E3-FIN : suppression des tests C++ portes en script

Branche visual, commit 16613ce ("E3-FIN: remove C++ tests ported to script"), worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron. Aucun build, aucun quick_check (la session principale compile ensuite).

## Registre : test C++ supprime -> test script equivalent (16 tests C++ retires)

| # | Test C++ supprime (fichier::nom) | Test script equivalent (fichier::nom) |
|---|---|---|
| 1 | HOLStartRulesTests.cpp::HeroesOfLiberty.Match.StartRules | Script\Tests\HOLStartRules_Test.as::Test_StartRulesVectors |
| 2 | HOLVersionTests.cpp::HeroesOfLiberty.Version | Script\Tests\HOLVersion_Test.as::Test_Version |
| 3 | HOLVersionTests.cpp::HeroesOfLiberty.VersionDisplay | HOLVersion_Test.as::Test_VersionDisplay |
| 4 | HOLVersionTests.cpp::HeroesOfLiberty.VersionCheck | HOLVersion_Test.as::Test_VersionCheck |
| 5 | HOLVersionTests.cpp::HeroesOfLiberty.AccessKey | HOLVersion_Test.as::Test_AccessKey |
| 6 | HOLHUDLayoutTests.cpp::HeroesOfLiberty.UI.Layout.Overlap | Script\UI\HOLHUDLayout_Test.as::Test_HUDLayoutOverlap |
| 7 | HOLHUDLayoutTests.cpp::HeroesOfLiberty.UI.Layout.CheatsLowWindow | HOLHUDLayout_Test.as::Test_HUDLayoutCheatsLowWindow |
| 8 | HOLCampTableTests.cpp::HeroesOfLiberty.Neutrals.CampTable | Tools\Tests\camp_table_test.py (dans quick_check values) |
| 9 | HOLTalentTableTests.cpp::HeroesOfLiberty.Talents.Csv | Tools\Tests\talent_table_test.py (dans quick_check values) |
| 10 | HOLStartingPeasantsTests.cpp::HeroesOfLiberty.Match.StartingPeasants.Locations | Script\Tests\HOLStartRules_Test.as::Test_StartingPeasantsLocations (rouge non vu separement, voir NON VERIFIE) |
| 11 | HOLFactionChoiceTests.cpp::HeroesOfLiberty.Match.FactionChoice.List | Script\Tests\HOLFactionChoice_Test.as::Test_FactionChoiceList |
| 12 | HOLFactionChoiceTests.cpp::HeroesOfLiberty.Match.FactionChoice.Names | HOLFactionChoice_Test.as::Test_FactionChoiceNames |
| 13 | HOLFactionChoiceTests.cpp::HeroesOfLiberty.Match.FactionChoice.TripleClick | HOLFactionChoice_Test.as::Test_FactionChoiceTripleClick |
| 14 | HOLManagerErrorTextTests.cpp::HeroesOfLiberty.UI.ManagerErrorText.Codes | Script\Tests\HOLManagerErrorText_Test.as::Test_ManagerErrorTextCodes |
| 15 | HOLManagerErrorTextTests.cpp::HeroesOfLiberty.UI.ManagerErrorText.ReturnToLobby | HOLManagerErrorText_Test.as::Test_ManagerErrorTextReturnToLobby |
| 16 | HOLMapRulesTests.cpp::HeroesOfLiberty.Map.Csv | Tools\Tests\map_table_test.py (HALDIR-1116o : rouge vu, PASS, 32b6154) |

TESTS_CPP_RETIRES: 16. Le total des tests C++ de quick_check doit baisser exactement de 16 ; Test_MatchLossText ne remplace rien ici (son test C++ est GARDE). Les tests script restent comptes (aucun retire).

## Fichiers supprimes (git rm) : 7
HOLStartRulesTests.cpp, HOLVersionTests.cpp, HOLCampTableTests.cpp, HOLTalentTableTests.cpp (Source\HeroesOfLiberty\Tests), HOLHUDLayoutTests.cpp (Source\HeroesOfLibertyUI\Tests), Simulation\HOLCampTable.h et HOLCampTable.cpp (espace de noms HOLCampTable::Parse/ReadTypeNames/UnknownMembers ; Grep des appelants avec \bHOLCampTable:: et le nom des fichiers : seul HOLCampTableTests.cpp les appelait ; ne pas confondre avec la struct FHOLCampTable, autre fichier, conservee). Lignes C++ supprimees : environ 1 040 (git diff --stat du commit).

## Fichiers partiels (lignes de test retirees, reste garde)
- HOLStartingPeasantsTests.cpp : retire Locations (43 l.), garde .Count (part python a faire, HOLTestHooks.h).
- HOLFactionChoiceTests.cpp : retire List, Names, TripleClick (88 l.), garde .Net.FactionChoice.Bodies (MakeMeBody/MakeAISlotBody/ParseLobby, hors bibliotheque).
- HOLManagerErrorTextTests.cpp : retire Codes et ReturnToLobby (81 l.), garde MatchLossText ENTIER (il contient ReadFailure 401, non expose ; ses 10 cas ported restent en doublon, non decoupes pour ne pas toucher un test sans build).
- HOLMapRulesTests.cpp : retire Map.Csv (43 l.). Map.Rules GARDE entier : non porte FTextInspector (cle/namespace), IdenticalTo, IsCultureInvariant, forme FSoftObjectPath ; ses cas portes (Test_MapRulesByKey/ByAsset/DisplayName) restent en doublon.
- HOLGameNamesTests.cpp : AUCUN test retire (les 3 tests gardent du non porte). Une seule assertion retiree : « the Forge's name » de ForgeKey (portee par Test_GameNamesForgeName) ; la cle et le namespace restent. LegionsLanguages, FactionNames garde entiers (langue, .locres en/ja, accents, Overrides E3-06 non portes).

## Gardes en entier, pourquoi
- HOLMarketTableTests.cpp (E3-14) : controle l'import UE du CSV (UDataTable::CreateTableFromCSVString, RowStruct), non couvert par le script (SAURON-1116q) ; Test_MarketTableRows ne prouve que les valeurs du fichier. NON supprime.
- E3-04 Count, E3-06, E3-10, E3-11 : pas portes (lignes [ ] de la file).

## Commentaires perimes corriges (4)
HOLDeployRules.h:8, HOLDeployRules.cpp:5, HOLExplorationXPRules.h:8, HOLDeployRulesTests.cpp:9 : ne citent plus les copies script supprimees.

## Commande pour la session principale
quick_check complet apres build : <worktree>\Tools\build.ps1 -LiveCoding=0 -Wait puis quick_check -Parts tests,values ; attendu : total tests C++ N-16, camp/talent/map python PASS, aucun script retire.

## Champs
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 16 (voir registre) ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: ~1 040 ; LIGNES_MIGREES_PAR_LOT: 0 ; TYPES_A_EXPOSER: aucun de nouveau (voir E3-09/E3-12/E3-02 : manager, langue, FTextInspector) ; TESTS_TAUTOLOGIQUES: 0 ; TEST_ROUGE: non applicable (suppression de doublons, rouges vus dans les rapports sources) ; EDITS_REFUSES: 2 (chaines multilignes CRLF, refaites ligne a ligne).
NON VERIFIE : compilation (aucun build : les 4 fichiers partiels coupes par marqueurs, includes laisses) ; rouge separe de Test_StartingPeasantsLocations (CELEBORN-1116p) ; l'equivalence de map_table_test.py avec Map.Csv repose sur la lecture du code comme texte, pas sur la vraie table ; HOLCampTable : aucun appelant de l'espace de noms mais les tests UE n'ont pas ete relances.
PROCESSUS_DETACHES: aucun lance.