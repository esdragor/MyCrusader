# GALADRIEL-1117a : lot L17-04 (couverture de HOLGameMenuTests.cpp)

BUILDS_DEMANDES: 0
TESTS_SCRIPT_AJOUTES: 2 (HOLGameMenuRules_Test.Test_GameMenuEscape, HOLGameMenuRules_Test.Test_GameMenuKeepsInput)
TESTS_CPP_RETIRES: 0 (non supprimes : suppression groupee au lot C++ suivant)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: ~35 (2 regles + enum dans HOLGameMenuRules.as, 2 tests)
TYPES_A_EXPOSER: EHOLEscapeAction (jumeau script EHOLGameMenuEscape, comme les autres enums du menu)
TEST_ROUGE: vu a 00:23 : 1365/1368 (Test_GameMenuEscape en rouge, valeur attendue faussee OpenMenu au lieu de CloseMenu), puis vert 00:25 : 1366/1368 (seuls rouges : NomadFirstForum, NomadReplicated, D831)
EDITS_REFUSES: 0
Branche galadriel-tab, commit eb5652e, branche prete. Merge main non fait (pas de C++ touche). IN_USE.txt libre, CPP: non.

## Couverture test C++ -> test script (HOLGameMenuTests.cpp, 7 tests, tous de pure regle, aucun n'ouvre d'ecran reel)

| Test C++ | Test script | Vecteurs |
|---|---|---|
| Victory.GameMenu.Escape | UnitTests.UI.HOLGameMenuRules_Test.Test_GameMenuEscape (AJOUTE) | 7 identiques |
| Victory.GameMenu.Surrender | HOLGameMenuRules_Test.Test_GameMenuCanSurrender | 4 identiques |
| Victory.GameMenu.QuitToMenu | HOLGameMenuRules_Test.Test_GameMenuCanQuitToMenu | 4 identiques |
| Victory.GameMenu.StatsButton | HOLGameMenuRules_Test.Test_GameMenuStatsButton | 4 identiques |
| Victory.GameMenu.Input | HOLGameMenuRules_Test.Test_GameMenuKeepsInput (AJOUTE) | 4 identiques |
| Victory.EndBanner.Which | HOLEndBannerScript_Test.Test_EndBannerWhich (8 vecteurs GetEndBanner, masques en entiers) + HOLGameMenuRules_Test.Test_GameMenuReturnToLobbyAndList (7 OffersReturnToLobby + 3 QuitsToList) | 18 identiques |
| Victory.EndBanner.Closed | HOLEndBannerScript_Test.Test_EndBannerClosed | 7 identiques |

## Tests C++ retirables (liste EXACTE, nom C++ -> nom script)

Les 7, avec leur fichier HOLGameMenuTests.cpp entier (143 l.) :
- HeroesOfLiberty.Victory.GameMenu.Escape -> Angelscript.UnitTests.UI.HOLGameMenuRules_Test.Test_GameMenuEscape
- HeroesOfLiberty.Victory.GameMenu.Surrender -> ...HOLGameMenuRules_Test.Test_GameMenuCanSurrender
- HeroesOfLiberty.Victory.GameMenu.QuitToMenu -> ...HOLGameMenuRules_Test.Test_GameMenuCanQuitToMenu
- HeroesOfLiberty.Victory.GameMenu.StatsButton -> ...HOLGameMenuRules_Test.Test_GameMenuStatsButton
- HeroesOfLiberty.Victory.GameMenu.Input -> ...HOLGameMenuRules_Test.Test_GameMenuKeepsInput
- HeroesOfLiberty.Victory.EndBanner.Which -> ...HOLEndBannerScript_Test.Test_EndBannerWhich + ...HOLGameMenuRules_Test.Test_GameMenuReturnToLobbyAndList
- HeroesOfLiberty.Victory.EndBanner.Closed -> ...HOLEndBannerScript_Test.Test_EndBannerClosed

Au retrait groupe, quick_check descendra de 7 tests C++ (le total net : +2 script ajoutes ici, -7 C++ ; les 5 autres sont des doublons que le brief autorise a retirer apres rouge puis vert, ici fait pour Escape et Input ; les 5 autres vecteurs avaient deja leur rouge/vert par GALADRIEL-1116m/1116r). HOLGameMenuRules.cpp reste (appelants : HOLPlayerController, HOLGameMenuScreen.cpp, etc.) ; le jumeau .as se resorbe a l'etape 4. Commande de verification apres retrait : Tools\Tests\quick_check.ps1 -Parts tests.
NON VERIFIE : l'equivalence des vecteurs 'Which' et 'Closed' repose sur la lecture (pas de rouge nouveau pour ces cinq, rouge vu par les lots precedents).