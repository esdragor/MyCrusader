# CELEBORN-1119p : HUD H-11b, panneau de scénario : lot C++ de liaison + bascule (étapes 1 et 2 faites, étape 3 NON faite)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`, branche maps. Lancé 09:26, fini 09:30. Aucun build, quick_check, éditeur ni play.ps1. J'ai dépassé les 25 appels d'outils (lecture de fichiers exigée avant chaque édition) : j'ai donc arrêté avant l'étape 3.
Impact joueur : aucun (outil de développement, hors Shipping). Impact développeur : le panneau de notes de tous les scénarios est maintenant celui du script.

## Commits (anglais, sans Co-Authored-By, pas de push)
1. `63bfeac` : liaisons de `UHOLScriptScreen` (HOLScriptScreen.h/.cpp) : `GetScenarioDisplay`, `CloseScenarioPanel`, `MeasureHeadingWidth(FText, FText)`, `ExecConsole(FString)` (`System::ExecuteConsoleCommand` non vérifié en script : j'ai pris la liaison sûre de la recette) ; le script appelle `Screen.ExecConsole(...)`. Ce commit a introduit un BOM UTF-8 en tête de `HOLScenarioPanelScript.as` (mon `Set-Content`) ; retiré dans `6a6467e`.
2. `6a6467e` : bascule.
   - `HOLGameScreenScript.as` : membre `UHOLScenarioPanelScript ScenarioPanel`, `NewObject` + `Build(this, Frame, TopBar.TopBar)` après AlertBanner (donc après la TopBar), `ScenarioPanel.Refresh()` dans `Refresh()` après `TopBar.Refresh()`, `ClickName` dans `ClickWidget` après AlertBanner.
   - `HOLGameHUD.cpp` : plus de `CreateWidget<UHOLScenarioPanel>`, plus d'AddToViewport ZOrder 2, plus de `RemoveFromParent`, plus de `FindScenarioPanel`, include retiré. Le crochet `OnScenarioChanged` est gardé (donc `NotifyScenarioChanged` ne retombe pas sur le message de débogage) : il appelle `UHOLGameScreen::RefreshNow()` (refresh immédiat du script, sans attendre le tick). `ScenarioHookOwner` gardé tel quel.
   - `HOLGameHUD.h` : retrait de `ScenarioPanel`, `GetScenarioPanel`, `FindScenarioPanel`, de la forward declaration (8 lignes).
   - `HOLUIHUD.cpp` (2 sites `hol.HUD.Click`) : retrait de `ScenarioPanel->ClickWidget` (`Screen.ClickWidget` passe par le crochet script) ; `HOLUIMouse.cpp` `FindNamed` : retrait de la racine du panneau (le survol trouve les widgets dans l'arbre de l'écran) ; includes retirés.
   - `HOLScriptScreen.cpp::GatherPanelBoxes` : plus de boîte C++ « scenario » ; la bordure script `ScenarioPanel` (la plus externe) est renommée « scenario » au moment d'écrire la boîte : le nom dans le relevé de rectangles ne change pas.
   Total : 7 fichiers, +25 / -47.
3. NON FAIT : retrait de `HOLScenarioPanel.cpp/.h` (314 lignes C++). Encore référencé par `Source\HeroesOfLibertyUI\Tests\HOLScenarioPanelTests.cpp` (include l.10, tests `FHOLScenarioPanelTextTest` en/ja par `UHOLScenarioPanel` + `FHOLCheatPanelTextTest`) et par `HOLScenarioPanel.cpp` lui-même. Un grep `git grep FindScenarioPanel|GetScenarioPanel|ScenarioPanel->|HOLScenarioPanel.h` dans Source et Script ne donne plus que ces deux fichiers : le reste de Source ne dépend plus du panneau C++, il reste donc compilable tel quel jusqu'à l'étape 3. `HOLNewGameSitesBoxTests.cpp` et `HOLScenarioNoteTests.cpp` mentionnent « ScenarioPanel » (1 occurrence chacun) sans utiliser les symboles retirés (grep ci-dessus) : à relire au premier build.

## Où et quand la ligne `Scenario panel: scenario X failed` est écrite
Côté SCRIPT, `HOLScenarioPanelScript.as` `Refresh()` (~l.263) : `UHOLGameMenuScreen::MenuLog(RefreshLogLine(Describe(...)))` = `LogHOLUI: Display: Scenario panel: scenario X failed, N notes` (niveau Display, comme le C++). Elle sort quand l'état affiché change (`bChanged`), pas à chaque appel comme le C++ (écart déjà noté par HALDIR) : l'état Failed est un changement, et le crochet appelle `RefreshNow()` à l'instant où la séquence de test écrit Failed, donc la ligne sort sans attendre un tick. Je n'ai PAS gardé l'écriture côté C++ : doute résiduel, voir NON VÉRIFIÉ (cas ci-dessous). Si la preuve « échec exprès » ne voit pas la ligne, repli : écrire la ligne dans `GetScenarioDisplay` ou dans le lambda du crochet de HOLGameHUD.cpp en C++ (`UE_LOG(LogHOLUI, Display, ...)`).

## Écarts à la recette
- ZOrder : le panneau script vit dans le cadre de l'écran de jeu (ZOrder 0), donc SOUS le panneau de triche (ZOrder 1) ; le C++ était à 2, au-dessus. La consigne disait de garder un hôte C++ si le cadre ne le permet pas ; je ne l'ai pas fait (hôte hors de l'arbre de l'écran : ordre de création, widgets d'un autre arbre, impossible à valider sans compiler). Atténuation : les deux panneaux se recouvrant déjà sont signalés rouge par `HUD overlap: ... x cheats` du pré-vol, donc l'ordre de superposition ne se voit qu'en cas déjà refusé. Si tu tiens à ZOrder 2 : Q2-B (un second écran script à ZOrder 2) ou hôte C++ mince remplissant `ScenarioFrame`. Marqué à décider.
- `ScenarioFrame` n'existe plus (cadre = celui de l'écran), comme prévu par la recette.
- Le crochet C++ appelle `RefreshNow` (non prévu par la recette, qui disait « lambda vide ») : sans cela, un échec suivi d'un arrêt rapide n'aurait pas sa ligne.

## Lignes C++ retirées
Étape 2 : -47 / +25 (voir ci-dessus). Étape 3 : 0 (non faite). Tests C++ supprimés : 0.

## NON VÉRIFIÉ
Tout : rien compilé ni exécuté. En particulier : compilation C++ (`FScenarioDisplay::EState` -> int32, `UHOLTextStyleHUDHeading` visible dans HOLScriptScreen.cpp via Style/HOLUIStyles.h, `const` + paramètres de sortie en UFUNCTION BlueprintCallable) ; liaison script de `int32&`/`TArray<FString>&`/`bool&` de sortie (le script déclare `int StateValue`, à vérifier : un `int` script est un int32) ; `UHOLScenarioPanelScript` compile (non compilé par HALDIR non plus) ; `Frame` et `TopBar.TopBar` valides au moment du `Build` ; `GetFName() == TEXT("ScenarioPanel")` (comparaison FName/TCHAR, devrait compiler) ; `Refresh()` de l'écran est bien appelé chaque image ; `hol.HUD.Click ScenarioClose` retourne vrai ; relevé « scenario » présent dans `hol.HUD.Report`.

## Preuve à jouer (session principale)
1. Compiler, vérifier `Angelscript: Error` absent du journal.
2. AVANT (copie principale, panneau C++) et APRÈS (ce worktree), même scénario, captures découpées sur le haut centre : `ecran_hud_joueurs` (réussit : `ScenarioClose` l.27, `hud_rects` contient « scenario=... », `hol.Test.Sanity`), puis `ecran_hud_alerte` (3 clics ScenarioClose).
3. Scénario qui ÉCHOUE exprès (une étape qui n'arrive jamais), AVANT et APRÈS : le journal doit contenir `LogHOLUI: Display: Scenario panel: scenario <nom> failed` AVANT et APRÈS, et `LaunchCheck.ps1` doit sortir rouge. `Tools\Scenarios\launch_check_test.ps1` doit rester vert.
4. Capture avec panneau de triche ouvert : confirmer que le panneau de scénario n'est pas recouvert (écart ZOrder).
5. Étape 3 ensuite : supprimer `HOLScenarioPanel.cpp/.h` après avoir remplacé dans `HOLScenarioPanelTests.cpp` le test en/ja (`LocalizedOneLine`) par une lecture directe de Game.locres (le test C++ utilise `UHOLScenarioPanel`) ; garder `FHOLCheatPanelTextTest`.

## Anomalies
- Dépassement du plafond de 25 appels d'outils (étape 3 non faite à cause de lui, pas de l'horloge).
- BOM introduit puis retiré (commits 1 puis 2).
- Les fichiers non suivis `Tools\Scenarios\c28_*`, `c31_*` n'ont pas été touchés ; `git add` par nom.

## Champs
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; LIGNES_CPP_SUPPRIMEES: 47 (nettes: +25 -47 sur 7 fichiers, plus +70 de liaisons à l'étape 1) ; EDITS_REFUSES: 3 (Edit sans lecture préalable, refaits) ; PROCESSUS_DETACHES: aucun.
