# HALDIR-1119m : HUD H-11b, panneau de scénario de test (recette d'intégration, fichiers neufs, AUCUNE bascule)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` (branche maps = main 3b52f7a). Lancé 09:17, rapport écrit avant 09:32. Aucun build, quick_check, éditeur ni play.ps1.
Impact joueur : aucun (outil de développement, `!UE_BUILD_SHIPPING`) ; impact développeur : le panneau de notes de TOUS les scénarios et la ligne de journal lue par LaunchCheck.

## Livré
- `Script\UI\HOLScenarioPanelScript.as` : espace `HOLScenarioPanelRules` (règles pures) + classe `UHOLScenarioPanelScript` (vue). NON COMPILÉ, NON BRANCHÉ : ne compile pas seul sans les 3 liaisons ci-dessous (+ à vérifier au 1er compile : `NSLOCTEXT`, `FText::Format(FText, FText)`, `System::ExecuteConsoleCommand`, `UWidget.GetDesiredSize`).
- `Script\UI\HOLScenarioPanelScript_Test.as` : 7 tests, aucun tautologique (chaque test passe par une fonction de règle qui produit le texte ou la décision ; AUCUN rouge vu : rien n'a été exécuté).

## Tests : C++ -> script
| Test C++ (HOLScenarioPanelTests.cpp) | Équivalent script |
|---|---|
| `FHOLScenarioPanelTextTest` partie fr (l.85-90 : « Scénario deploy », « en préparation… ») | `Test_ScenarioPanelFrenchTexts` (+ prêt, erreur, Relancer, ×) |
| même test, partie « une ligne » (l.144-158 : wrap faux préparation/prêt, vrai erreur) | `Test_ScenarioPanelStateWrapsOnlyAnError` |
| même test, partie en/ja (l.91-142, Game.locres par espace+clé) | NON PORTÉ : un test script ne lit pas un .locres. Reste C++ jusqu'à la bascule, puis doit lire le FText du widget script (voir Q1) |
| `FHOLCheatPanelTextTest` (l.164-286) | hors sujet (panneau de triche, autre widget) : reste C++, le fichier `HOLScenarioPanelTests.cpp` ne peut donc PAS être supprimé en entier |
| (aucun test C++) règle « affiché » cpp:96 | `Test_ScenarioPanelShownRule` |
| (aucun) `Describe` cpp:169-186 | `Test_ScenarioPanelDescribeIsTheCppLine` |
| (aucun) lignes de journal cpp:166, 224, 237 | `Test_ScenarioPanelJournalLinesAreTheCppOnes` |
| (aucun) `ClickWidget` cpp:199-218 | `Test_ScenarioPanelClickRouting` |
| (aucun) ancrage cpp:76-83, notes cpp:147, colonnes visibles | `Test_ScenarioPanelAnchorAndNotes` |

Pas de test « chaîne réelle » (FScenarioDisplay réel -> vue) : il demande la liaison `GetScenarioDisplay` ; à ajouter avec le lot C++ (modèle `Test_AlertBannerFollowsTheCppStateMachine`).

## Ce que fait chaque fonction de HOLScenarioPanel.cpp
| Lignes | Fonction | Rôle | Destin |
|---|---|---|---|
| 24-42 | constantes | marge 16, gap 6, largeur max 860, × 29, Relancer min 96, cyan d'attente | script (`HOLScenarioPanelRules::...()`) |
| 44-49 | constructeur | non focalisable, consomme le pointeur | script : `MakePanel` (Visible) |
| 51-68 | `NativeOnInitialized` | construit, SelfHitTestInvisible, branche les clics, Refresh | script `Build` + `OnButtonBaseClicked` |
| 70-84 | `NativeTick` | place le panneau sous l'ancre (TopBar visible, hauteur désirée) | script `PlaceUnderAnchor` (chaque image) |
| 86-167 | `Refresh` | règle « affiché », titre, état (texte, couleur, une ligne sauf erreur, largeur min = plus long de « en préparation… » / « prêt »), notes reconstruites si changées, ligne waitkey, ligne de journal | script `Refresh` ; mesure de police = liaison C++ |
| 169-186 | `Describe` | « no scenario » / « scenario X closed » / « scenario X ready, N notes[, waiting for the key] » | script `Describe` (mot pour mot) |
| 188-197 | `GetBox` | boîte pour `hol.HUD.Report` (« scenario ») | voir R2 |
| 199-218 | `ClickWidget` | ScenarioRestart / ScenarioClose, refusé si replié | script `ClickName` |
| 220-225 | `HandleCloseClicked` | `bClosed = true`, Refresh, journal | script `Close` + liaison `CloseScenarioPanel` |
| 227-239 | `HandleRestartClicked` | `GEngine->Exec("hol.Scenario <Name>")` + journal | script `Restart` |
| 241-314 | `BuildLayout` | arbre : ScenarioFrame > ScenarioWidth > ScenarioPanel > ScenarioColumn > ScenarioHeader (ScenarioTitle, ScenarioState, ScenarioRestartBox > ScenarioRestart, ScenarioCloseBox > ScenarioClose), ScenarioNotes, ScenarioWaitKey | script `Build`, mêmes noms (sauf ScenarioFrame : le cadre est celui de l'écran, voir Q2) |

## Qui appelle quoi
- Écrivain de l'état : `HOLScenario.cpp:139-142` (remise à zéro + Name), `:149-150` (Ready à la fin), `HOLTestSequence.cpp:395-397` (`panel hide|show` -> bHidden, journal `LogHOLTest: Scenario panel: hidden|shown`), `:402` (`note`), `:410-421` (`notereset`), `:428-441` (`waitkey` -> WaitingForKey + Ready), `:545-547` (Failed + Error), `:606` (WaitingForKey vidé). Tous restent C++ (module jeu, que le module UI ne voit pas, D247).
- Lecteurs du panneau : `HOLGameHUD.cpp:94-107` (crée, ancre = `Screen->GetTopBar()`, ZOrder 2, pose `OnScenarioChanged` -> `Refresh`), `:218-221` (retire), `:322` `FindScenarioPanel` ; `HOLUIHUD.cpp:120-122` (`hol.HUD.Click` -> `ClickWidget`) ; `HOLUIMouse.cpp:135` (survol) ; `HOLScriptScreen.cpp:537-544` (`GetBox` -> boîte « scenario » de `GatherPanelBoxes`).

## Ce qui reste C++ (D839)
- `FScenarioDisplay`, `GetScenarioDisplay`, `OnScenarioChanged`, `NotifyScenarioChanged` (HOLTestHooks, module jeu : une source, écrit par la séquence de test).
- Liaisons à exposer (lot C++ de la séance, membres de `UHOLScriptScreen`, modèle HOLTopBar) :
  - `bool GetScenarioDisplay(FString& OutName, int& OutState, FString& OutError, TArray<FString>& OutNotes, FString& OutWaitingForKey, bool& bOutClosed, bool& bOutHidden) const;` (OutState : 0 Preparing, 1 Ready, 2 Failed)
  - `void CloseScenarioPanel();` (écrit `GetScenarioDisplay().bClosed = true`)
  - `float32 MeasureHeadingWidth(FText A, FText B);` (max des largeurs en police `UHOLTextStyleHUDHeading`, 0 sans moteur de rendu ; code de cpp:134-144)
  - Redémarrage : `System::ExecuteConsoleCommand` si exposé (à vérifier), sinon `void ExecConsole(FString Command);` = `GEngine->Exec(GetWorld(), *Command)`.
- Le test `LocalizedOneLine` en/ja tant que Q1 n'est pas tranchée.

## Crochet à poser (à la bascule, PAS dans cette vague)
Dans `HOLGameScreenScript.as` (modèle : AlertBanner) : membre `UHOLScenarioPanelScript ScenarioPanel`, créé et `Build(Screen, Frame, TopBar.TopBar)` dans `OnInitialized` après la TopBar (l'ancre doit exister), `ScenarioPanel.Refresh()` dans le `Refresh()` de l'écran, `ClickName` dans `ClickWidget`. À la bascule : retirer de `HOLGameHUD.cpp` les lignes 92-108 (création, ZOrder, hook ; garder le hook en lambda vide pour que `NotifyScenarioChanged` ne retombe pas sur le message de débogage), `FindScenarioPanel`, et les 3 lecteurs ci-dessus ; supprimer `HOLScenarioPanel.cpp/.h` ; ne garder du test C++ que `FHOLCheatPanelTextTest`.

## Table outil -> texte lu -> où le script l'écrit (grep réel de Tools\ et Source\)
| Outil | Texte / nom lu | Source C++ | Où le script l'écrit |
|---|---|---|---|
| `Tools\Scenarios\LaunchCheck.ps1:86` (et `launch_check_test.ps1:35`) | `LogHOLUI: Display: Scenario panel: scenario <nom> failed, N notes` (regex `Scenario panel: scenario \S+ failed`) | cpp:166, à chaque `Refresh` | `Refresh` : `UHOLGameMenuScreen::MenuLog(RefreshLogLine(Describe(...)))` (niveau Display, leçon 1). Écart : le C++ écrit à chaque appel du hook, le script écrit au changement (`bChanged`) ; l'état Failed est un changement, donc la ligne sort. |
| `Tools\Scenarios\ecran_hud_alerte.txt:13,22,30`, `ecran_hud_bannieres.txt:10`, `ecran_hud_joueurs.txt:27` | `hol.HUD.Click ScenarioClose` (retourne vrai ou faux) | cpp:199-218 | `ClickName(n"ScenarioClose")` via le crochet `ClickWidget` de l'écran |
| `ecran_hud_*.txt` et ~tous les scénarios | `panel hide` / `panel show` (bHidden) | `HOLTestSequence.cpp:392-398` (reste C++) | `GetScenarioDisplay(... bOutHidden)` -> `IsShown` |
| `note`, `notereset`, `waitkey` de tous les scénarios | notes, ligne cyan | `HOLTestSequence.cpp` (reste C++) | `Refresh` : colonne `ScenarioNotes`, ligne `ScenarioWaitKey` |
| `preflight.ps1` (règle « HUD panels cover each other », `HudRects.ps1`) | aucun nom « scenario » lu par les .ps1 (grep : 0 occurrence de `"scenario"` ni `ScenarioPanel` dans Tools\*.ps1 hors LaunchCheck) ; la boîte vient de `hol.HUD.Report` (`HOLScriptScreen.cpp:537-544`, nom « scenario ») | cpp:188-197 | Le panneau `ScenarioPanel` créé par `MakePanel` entre dans `GatherPanelBoxes` comme panneau script le plus externe, nom différent (R2) |
| `play.ps1`, `quick_check.ps1` | aucun texte du panneau (grep sans résultat sur Scenario panel / ScenarioClose / restarted) | | |
| `HUD: scenario panel closed`, `HUD: scenario <nom> restarted` | aucun outil ne les lit (grep : 0) | cpp:224, 237 | `Close` / `Restart`, `MenuLog` (mot pour mot) |
| `Describe()` | pas de ligne `hol.HUD.Report` : seul appelant = le journal cpp:166 (grep) | | `Describe` |

## Risque
Un défaut ici casse tous les pré-vols : (1) la ligne `Scenario panel: ... failed` absente ou sans « Display: » = un scénario en échec n'est plus détecté par LaunchCheck (faux vert, le pire cas) ; (2) `hol.HUD.Click ScenarioClose` faux = les 3 scénarios `ecran_hud_*` captent la bannière de notes ; (3) panneau plus large ou au mauvais endroit = « HUD panels cover each other » rouge sur tous les scénarios ; (4) ZOrder (Q2) : sous le panneau de triche. Mesure de largeur de l'état : un défaut est visible seulement en capture (leçon 3).

## Preuve avant / après proposée
1. Avant bascule : `quick_check` (sans changement : le C++ reste), scripts compilent (`Angelscript: Error` absent du journal) une fois les 3 liaisons posées.
2. Rejouer `ecran_hud_joueurs` (modèle de la leçon 12-14 : `ScenarioClose` l.27, `hud_rects`, `hol.Test.Sanity`) AVANT et APRÈS, captures découpées sur la zone du panneau (haut centre) comparées à l'œil : titre, « prêt » cyan/or, Relancer, ×, notes, ligne waitkey. Ajouter `ecran_hud_alerte` (3 clics ScenarioClose) et un scénario qui échoue exprès (étape jamais venue) pour voir la ligne `failed` dans le journal ET LaunchCheck rouge. `Tools\Scenarios\launch_check_test.ps1` doit rester vert.

## Questions (pas tranchées)
- Q1 [localisation] : le panneau script écrit `NSLOCTEXT("HOLScenarioPanel", clé, ...)` (mêmes clés que le C++, donc Game.locres en/ja valable). A : garder le test `.locres` en C++ en lisant le FText du widget script via une liaison de lecture ; B : l'abandonner (perte de la garantie en/ja pour un outil de développement, non traduit pour le joueur). Reco : A, valeur par défaut codée : NSLOCTEXT.
- Q2 [couche] : le C++ est au ZOrder 2, par-dessus l'écran de jeu et le panneau de triche ; en script le panneau vit dans le cadre de l'écran de jeu (sous le panneau de triche, ZOrder 1). A : accepter (le panneau de triche a sa place sous la barre, rarement les deux ouverts) ; B : un second écran script à ZOrder 2 (plus de C++ de socle). Reco : A, puis vérifier par capture.
- R2 : la boîte du rapport s'appelle « scenario » en C++ ; en script elle prendrait le nom de `ScenarioPanel` dans `GatherPanelBoxes`. Aucun outil ne lit ce nom (grep), mais une ligne `HUD overlap:` change de libellé ; à vérifier par un rejeu.

## NON VÉRIFIÉ
Tout : rien compilé ni exécuté ; pas de rouge vu ; `NSLOCTEXT`, `FText::Format`, `System::ExecuteConsoleCommand`, `GetDesiredSize`, `Screen.MakeText` (utilisé dans HOLTopBar) en script ; absence de lecteur de `Describe` hors journal (grep de Source\ seulement) ; `HudRects.ps1` lu par grep, pas en entier.

## Champs
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 7 (Test_ScenarioPanelFrenchTexts, StateWrapsOnlyAnError, ShownRule, DescribeIsTheCppLine, JournalLinesAreTheCppOnes, ClickRouting, AnchorAndNotes) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~314 (cpp) portées, rien retiré ; TYPES_A_EXPOSER: membres UHOLScriptScreen (GetScenarioDisplay, CloseScenarioPanel, MeasureHeadingWidth) ; EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: aucun lancé.
