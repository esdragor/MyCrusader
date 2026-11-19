# GALADRIEL-1119x : suppression du C++ mort des panneaux triche et scenario (NON COMPILE)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `galadriel-tab`, commit `bbc11ce` (un seul, C++ + commentaires). Branche prete, NON COMPILEE, aucun build, aucun jeu. merge main non fait. Impact joueur : aucun (code mort retire ; panneaux script inchanges).

## Bloquants : AUCUN
Grep avant suppression, de chaque symbole (`UHOLCheatPanel`, `UHOLScenarioPanel`, `ECheat`/`ESpeed` (imbriques), `GetWindowHeight` statique, `CVarLowFold`, includes `HOLCheatPanel.h` / `HOLScenarioPanel.h`, noms de tests) dans `Source\`, `Script\`, `Tools\`, `Config\` : aucun appelant vivant hors des fichiers supprimes. Seuls restent les 4 `#include "HUD/HOLCheatPanel.h"` (sans usage du type derriere), retires.
Statiques lues par le script, verifiees hors des fichiers supprimes : `CheatsStartOpen` -> `UHOLHUDLayoutReadLibrary` (`HOLHUDLayoutReadLibrary.h:54/.cpp:16`, + `HOLHUDLayoutRules.h:35`), `ShouldFoldCheats` -> `UHOLHUDLayoutReadLibrary` (`.cpp:21`), `GetWindowHeight` -> `UHOLCheatSettingsLibrary` (`HOLCheatSettingsLibrary.h:30/.cpp:42`).

## Fichiers supprimes (git rm), chemins sous `Source\HeroesOfLibertyUI\`
HUD\HOLCheatPanel.cpp (483), HUD\HOLCheatPanel.h (149), HUD\HOLScenarioPanel.cpp (314), HUD\HOLScenarioPanel.h (87), Tests\HOLScenarioPanelTests.cpp (288, ne contenait que les 2 tests des panneaux). Total 1321 lignes.

## Retires / mis a jour
- `#include "HUD/HOLCheatPanel.h"` : Dev\HOLUIHUD.cpp:7, Dev\HOLUIMouse.cpp:16, HUD\HOLGameHUD.cpp:12, HUD\HOLScriptScreen.cpp:13. Aucun include du panneau de scenario hors des fichiers supprimes.
- Commentaires : HUD\HOLGameScreen.h:247, HUD\HOLScriptScreen.cpp:516, Tests\HOLNewGameSitesBoxTests.cpp:37, Tests\HOLScreenTestLibrary.cpp:153 et .h:57, `Source\HeroesOfLiberty\Tests\HOLScenarioNoteTests.cpp:9`.
- Script (commentaires seulement) : HOLScenarioPanelScript.as:2, HOLScenarioPanelScript_Test.as:2 et 8-10 (NOT PORTED -> HOLScenarioPanelLocalizedScript_Test.as, rouge+vert 11:34), en-tetes des deux `*LocalizedScript_Test.as` (NOT COMPILED -> rouge/vert vu).
- Re-grep apres : zero reference C++ dans `Source\`, `Config\`, `Tools\` (fill_po.py garde les cles `HOLCheatPanel,*` / `HOLScenarioPanel,*` : voulu, le script les utilise). Restent des mentions de provenance (« Same panel as HOLCheatPanel.cpp », « HOLCheatPanel.cpp:474-475 ») dans des commentaires de `Script\UI\HOLCheatPanelScript.as` (lignes 5, 12, 41, 253, 269), `HOLCheatPanelScript_Test.as:2`, et dans les en-tetes des deux `*LocalizedScript_Test.as` (lignes de l'ancien test) : historique, non modifie.

## Champs
BUILDS_DEMANDES: 1 (build C++ a lancer par la session principale : suppression de 4 fichiers + 4 includes) ;
LIGNES_CPP_SUPPRIMEES: 1321 lignes de fichiers entiers + 6 lignes d'includes/commentaires C++ ; `git diff --stat HEAD~1 HEAD` : 18 fichiers, 15 insertions, 1340 suppressions (dont ~8 lignes de commentaires script) ;
TESTS_CPP_RETIRES: 2 : `HeroesOfLiberty.UI.ScenarioPanel.LocalizedOneLine` (FHOLScenarioPanelTextTest) -> script `Test_ScenarioPanelTitleAndStatesInEnglish`, `Test_ScenarioPanelTitleAndStatesInJapanese`, `Test_ScenarioPanelTranslationsAreNotTheFrenchSource` (HOLScenarioPanelLocalizedScript_Test.as) + partie francaise `Test_ScenarioPanelFrenchTexts`, `Test_ScenarioPanelStateWrapsOnlyAnError` (HOLScenarioPanelScript_Test.as) ; `HeroesOfLiberty.UI.CheatPanel.Localized` (FHOLCheatPanelTextTest) -> `Test_CheatPanelButtonsAreTranslatedInEnglish`, `...InJapanese`, `Test_CheatPanelSpeedLabelsAreTranslated`, `Test_CheatPanelWoodStoneGoldLabelsInFrench` (HOLCheatPanelLocalizedScript_Test.as) ;
TESTS_SCRIPT_AJOUTES: 0 (aucun neuf) ; LIGNES_MIGREES_PAR_LOT: 0 ; TYPES_A_EXPOSER: aucun ; TEST_ROUGE: vu a 11:34 (lot precedent, en/ja faussees puis vert) ; TESTS_TAUTOLOGIQUES: 0 ; PROCESSUS_DETACHES: 0.
Total quick_check attendu : C++ -2 (les 2 tests), script inchange (+0) : le total baisse de 2 par construction, deja compense par les 7 tests script ajoutes en 1119w.

## Anomalies
1. La cvar `hol.HUD.CheatsLowFold` (defaut true, desactivait le repli en fenetre basse) n'etait definie que dans HOLCheatPanel.cpp et ignoree par le script (HOLCheatPanelScript.as:457-461 replie toujours) : elle disparait avec le fichier ; aucune reference dans `Source`, `Script`, `Config`, `Tools`. A noter si un reglage de test l'utilisait hors depot.
2. Les 4 fichiers qui incluaient `HOLCheatPanel.h` ne recevront plus `CoreMinimal.h` / `CommonUserWidget.h` par lui : ils incluent `HUD/HOLGameScreen.h` / `HOLGameHUD.h` (widgets communs), a confirmer au build, sinon ajouter l'include direct manquant.
3. Un commentaire de `HOLScenarioPanelLocalizedScript_Test.as` ligne 2 garde une phrase coupee (« ...marked NOT PORTED. » puis ligne suivante « HOLScenarioPanel.* and that C++ test are deleted. ») : cosmetique.

## NON VERIFIE
Rien compile : aucune autre dependance transitive de `HOLCheatPanel.h` n'a ete cherchee que par la liste d'includes des 4 fichiers ; `Placeholders.txt` non relu (aucun provisoire touche) ; `EtapeDefini.md` et `Auto_Journey_File.md` non touches (consigne). Commande pour la session principale : `Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree Galadriel, puis `quick_check` (total C++ attendu -2).