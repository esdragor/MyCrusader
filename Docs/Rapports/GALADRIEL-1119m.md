# GALADRIEL-1119m : HUD H-11a, panneau de triche (HOLCheatPanel), vue script + recette (reprise 14, 2026-11-19)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` (branche galadriel-tab, base main 3b52f7a). Fichiers NEUFS seulement, AUCUNE bascule. Impact joueur : aucun (outil de développement, absent du jeu normal).

## Livré
- `Script\UI\HOLCheatPanelScript.as` : règles pures `HOLCheatPanelRules` / `HOLCheatPanelText` + vue `UHOLCheatPanelScript` (Build / Refresh / ClickName / Describe / GetBottom).
- `Script\UI\HOLCheatPanelScript_Test.as` : 8 tests de règle (liste et ordre des boutons, plan réel de chaque bouton par `HOLControllerCommands::PlanOf`, bascule de la construction rapide, libellés, lignes de journal et de rapport, pas des curseurs, états grisé / suivi du serveur, place sous la barre).
- NE COMPILENT PAS SEULS : appellent `HOLCheatSettings::*`, `HOLHUDLayoutRead::*` (existe déjà), `UHOLScriptScreen::SetCheatsReport` et le crochet. Ne pas lancer un build avec ces deux fichiers présents sans le lot C++ ci-dessous (en-tête des fichiers). Aucun test exécuté, rouge non vu (aucun build permis).

## Ce que fait HOLCheatPanel.cpp (483 l.) et ce qui devient quoi
| Lignes | Fonction | Rôle | Destin |
|---|---|---|---|
| 27-103 | espace CheatPanel | constantes (marge 16, gap 6, bouton 190x29, pas 0,5, plage 0,5-5), specs des boutons et curseurs, libellés NSLOCTEXT, cvar `hol.HUD.CheatsLowFold` | script (fonctions de `HOLCheatPanelRules`) ; **la cvar `hol.HUD.CheatsLowFold` (85-87) n'est PAS reprise** : décision à prendre (question 2) |
| 105-110 | constructeur | `SetIsFocusable(false)`, `SetConsumePointerInput(true)` : un clic sur le panneau s'arrête là | disparaît si le panneau entre dans l'arbre de l'écran script (question 1) |
| 112-150 | NativeOnInitialized | construit, branche les clics, lit `bOpen` dans GGameUserSettingsIni, ouvre selon `CheatsStartOpen` | `Build` ; lecture de la config = liaison C++ `HOLCheatSettings::GetSavedOpen` (GConfig) |
| 152-160 | GetWindowHeight | hauteur du viewport | liaison `HOLCheatSettings::GetWindowHeight` (ou existante : grep `GetViewportSize` dans HOLScriptScreen.cpp avant) |
| 162-182 | NativeConstruct | `Controller`, cvars serveur si pas client (`hol.Prod.BuildTime`, `hol.Units.SpeedScale`, `hol.Gather.SpeedScale`) | liaisons `IsServerLocal`, `GetServerBuildTime`, `GetServerSpeed(i)` |
| 184-211 | FollowServerSettings | la construction rapide et les curseurs suivent le serveur (solo / hôte) | `FollowServerSettings` + `FastBuildFollows` / `SpeedFollows` |
| 213-239 | NativeTick | suit le serveur, replie si fenêtre basse et bloc du héros ouvert (une fois), journal `HUD: cheats folded (window N px high, ...)`, re-place sous la barre | `Refresh(bHeroBlockShown)` (règles `ShouldFoldCheats` / `CheatsStartOpen` DÉJÀ liées : `HOLHUDLayoutRead::`, HOLHUDLayoutReadLibrary.h:52-61) |
| 241-262 | GetBottom, GetBox | bas du panneau (pile de la ligne du niveau IA) ; boîte dessinée (rects, chevauchements) | `GetBottom` script ; `GetBox` : voir recette `DescribeRects` plus bas |
| 264-280 | SetOpen | plie / déplie, libellés, garde le choix (GConfig + Flush) si `bKeep` | `SetOpen` + `HOLCheatSettings::SetSavedOpen(bool)` (écrit + Flush) |
| 282-286 | HandleToggleClicked | bascule + journal `HUD: cheats open|folded` | `ClickToggle` |
| 288-312 | HandleCheatClicked | journal `HUD: cheat <Nom> clicked`, puis `controller->RequestCheat<X>` | `ClickButton` : même route, via `HOLControllerCommands::PlanOf` + `Send` (le C++ `RequestCheat*` passe par `HOLRpcScriptRoute_Haldir1117n::CallCheat` = même RPC script, HOLPlayerController.cpp:4856-4915) |
| 314-349 | HandleUnit/Gather/SpeedChanged | arrondit au pas, borne, un envoi par pas, journal `HUD: cheat unit|gather speed xN.N` | `SpeedMoved` + `SnapSpeed` / `SpeedChanged` |
| 351-374 | RefreshLabels | libellés du toggle, de la construction rapide, des curseurs | `RefreshLabels` |
| 376-396 | ClickWidget | clic par nom (hol.HUD.Click), refusé si plié (sauf le toggle) | `ClickName` |
| 398-412 | Describe | ligne `HUD cheats: ...` du rapport | `Describe` + liaison `SetCheatsReport` |
| 414-483 | BuildLayout | arbre : CheatFrame > CheatPanel (Border) > CheatPanelColumn > CheatToggle + CheatColumn (7 boutons, 2 x label + curseur) | `Build` |

Écarts voulus de la vue script (à voir à la capture) : les `USizeBox` des boutons, anonymes en C++, sont nommés `<Nom>Box` ; pas de `CheatFrame` (le panneau s'ancre dans le cadre de l'écran) ; style du panneau = `Screen.MakePanel` (le C++ pose `FSlateRoundedBoxBrush(HUDPanel, 4)` et padding 8 : vérifier que MakePanel donne la même chose).

## Ce qui doit rester C++ et pourquoi (D839)
- Les envois RPC (`RequestCheat*` du contrôleur) : déjà script de bout en bout (`HOLControllerCommands.as`, `ServerCheat*` de `HOLRpcPlayer.as`) ; ils restent compilés tant que d'autres appelants C++ existent (hors panneau : à grep avant de retirer ; aucun autre appelant dans Source\ hors HOLCheatPanel.cpp selon le grep fait).
- `HOLHUDLayoutRules.h` / `HOLHUDLayoutReadLibrary` : règle pure d'une seule source, déjà liée.
- `UHOLQueueItem` : widget de base Slate (bouton), reste C++.
- Les nouvelles liaisons (lecture de GConfig, hauteur de fenêtre, cvars) : C++ obligatoire (liaison à exposer), minuscules, une bibliothèque.
- Les tests C++ des cvars : aucun.

## Liaisons à ajouter (une bibliothèque, un seul lot C++)
Nouvelle `Source\HeroesOfLibertyUI\HUD\HOLCheatSettingsLibrary.h/.cpp` (nom choisi `UHOLCheatSettingsLibrary` pour que le script dise `HOLCheatSettings::`; éviter `HOLCheatPanel` : collision avec la classe, leçon 10) :
```cpp
UFUNCTION(ScriptCallable) static bool GetSavedOpen();                       // GConfig->GetBool("HOL.CheatPanel","bOpen", GGameUserSettingsIni), false si absent
UFUNCTION(ScriptCallable) static void SetSavedOpen(bool bOpen);             // SetBool + Flush(false, GGameUserSettingsIni)
UFUNCTION(ScriptCallable) static float GetWindowHeight();                   // GEngine->GameViewport->GetViewportSize().Y, 0 sans viewport
UFUNCTION(ScriptCallable) static bool IsServerLocal();                      // GetWorld() ... GetNetMode() != NM_Client (prendre un WorldContext si le plugin l'exige : `UObject* WorldContext` en premier paramètre, et l'appeler `HOLCheatSettings::IsServerLocal(Screen)`)
UFUNCTION(ScriptCallable) static float GetServerBuildTime();                // hol.Prod.BuildTime, 0 si la cvar est absente
UFUNCTION(ScriptCallable) static float GetServerSpeed(int32 Index);         // 0: hol.Units.SpeedScale, 1: hol.Gather.SpeedScale ; 1.0 si absente (le C++ garde alors `Speeds[Index]`)
```
Si le plugin Angelscript expose déjà `Console::GetConsoleVariableFloat(FString)` et un test de mode réseau, les trois dernières liaisons tombent (NON VÉRIFIÉ : grep `Console::` dans `Script\` avant le lot). Dans `UHOLScriptScreen` (HOLScriptScreen.h/.cpp, FICHIER PARTAGÉ : à faire seulement dans le lot) :
```cpp
UFUNCTION(ScriptCallable) void SetCheatsReport(const FString& Line);   // garde la ligne ; HOLUIHUD.cpp:101-104 écrit "HUD cheats: <Line>" au lieu de CheatPanel->Describe()
```
(`ScriptCallable` : à remplacer par la macro réellement utilisée par `HOLScriptScreen.h` pour `GetEconomyNow`.)

## Crochet et qui ouvre / ferme le panneau
- Aujourd'hui : `AHOLGameHUD::` crée le panneau (HOLGameHUD.cpp:86-90, `AddToViewport(1)`, ancre `Screen->GetTopBar()`), le détruit (223-226), `FindCheatPanel` (316-319). Il n'y a AUCUNE touche ni commande qui l'ouvre : le clic sur `CheatToggle` (souris ou `hol.HUD.Click CheatToggle`) ; `hol.HUD.Cheats <0|1>` (HOLUIHUD.cpp:169-181) le plie / déplie sans garder le choix ; la config `HOL.CheatPanel/bOpen` garde le choix d'un match à l'autre ; une fenêtre < 700 px le démarre plié et le replie à l'ouverture du bloc du héros.
- Crochet dans `Script\UI\HOLGameScreenScript.as` : membre `UHOLCheatPanelScript CheatPanel;` ; dans `OnInitialized`, APRÈS la construction de la barre du haut : `CheatPanel = NewObject(this, UHOLCheatPanelScript); CheatPanel.Build(this, Frame, TopBarPanel.TopBar);` (Frame ajouté APRÈS les autres panneaux pour rester au-dessus : l'ancien z-order 1) ; dans `Refresh()` : `CheatPanel.Refresh(<le bloc du héros est visible>)` puis `SetCheatsReport(CheatPanel.Describe())` ; dans `ClickWidget(Name)` : `|| CheatPanel.ClickName(Name)` ; les 5 usages de `GetCheatPanelBottom()` (lignes 754, 1006, 1052, 2349) deviennent `CheatPanel.GetBottom()` ; `SetCheatsHeroBlockShown(View.bVisible)` (1939) devient la valeur passée à `Refresh` (ou `CheatPanel.bHeroBlockShown = View.bVisible`).
- `hol.HUD.Cheats` : `HOLUIHUD.cpp:175-178` appelle `CheatPanel->SetOpen(bOpen,false)` : à re-enregistrer en script (commande `hol.HUD.Cheats` qui appelle `CheatPanel.SetOpen(bOpen,false)` puis écrit `HUD cheats set: open|folded`), sinon la commande disparaît.
- À retirer du C++ à la bascule : HOLCheatPanel.cpp/.h, HOLGameHUD.cpp:12,86-90,223-226,316-319 / .h:12,36,42,70, HOLUIHUD.cpp:7,101-104,119-122,175-178, HOLUIMouse.cpp:16,135 (le contexte de rects reçoit `FindCheatPanel`), HOLScriptScreen.cpp:13,481-491,528-531 / .h:258 (`GetCheatPanelBottom`, `SetCheatsHeroBlockShown`, la boîte dans `DescribeRects`), HOLGameScreen.h:189 (commentaire). Leçon 15 : ces fichiers se heurtent aux autres panneaux ; garder les crochets côte à côte.

## Table outil -> texte lu -> où le script l'écrit (grep réel de Tools\)
| Outil | Texte ou widget lu | Où le script l'écrit |
|---|---|---|
| Scenarios\ai_level_line.txt:12, diplomacy_eight.txt:8, diplomacy_fit.txt:8 | `hol.HUD.Click CheatToggle` (log `HUD click CheatToggle: done` produit par HOLUIHUD.cpp:123) | widget `CheatToggle` (Build) ; `ClickName` appelé par le `ClickWidget` du script ; ligne `HUD click ...` reste C++ |
| Scenarios\heros_triches_600.txt:16 | `waitlog HUD cheats: cheats folded` | `Describe()` -> `SetCheatsReport` -> HOLUIHUD.cpp:101-104 (mot pour mot : `cheats folded` / `cheats open, fast build off, unit speed x1.0, gather speed x1.0`) ; le pli de fenêtre basse doit donner `folded` |
| Tools\Scenarios\rpc_cheats_run.ps1:25 et suivantes | `RpcCheat: fast build received` (journal du SERVEUR) | non touché : lit la route serveur, pas le panneau (le script lance des commandes `hol.Cheat.*`, pas de clic) |
| Tools\Tests\no_cpp_rpc_gate.ps1:10 | noms `ServerCheat*` de la liste « plus de RPC C++ » | non touché ; confirme que les routes de la table ci-dessus sont les RPC script |
| Tools\Localization\fill_po.py:720-732 | clés `HOLCheatPanel,<clé>` (ToggleOpen, ToggleFolded, FastBuildOn, FastBuildOff, CheatFood...CheatPopulation, UnitSpeed, GatherSpeed, SpeedLabel) | `NSLOCTEXT("HOLCheatPanel", <même clé>, ...)` dans `HOLCheatPanelText` : identiques, traductions gardées |
| Tools\Scenarios\*.txt (nombreux) | `hol.Cheat.Resources / Population / SpawnBuilding / Tier / UnitSpeed / Wound` | commandes console, déjà script ou C++ à part (non panneau) |
| hud_rects*, HudRects.ps1 (preflight) | grep `Cheat` : aucun résultat dans Tools\ ; la ligne `HUD rects:` vient de `DescribeRects` (HOLScriptScreen.cpp:528-531, ajoute la boîte du panneau) | NON VÉRIFIÉ : lire HOLScriptScreen.cpp:520-540 pour le NOM du rect ; le script doit fournir la boîte (`Panel.GetCachedGeometry()`) sous le même nom |
| `HUD overlap:` (HOLUIHUD.cpp:97, preflight.ps1 : doit rester `none`) | chevauchement du panneau avec les autres | `DescribeOverlaps` doit connaître la boîte du panneau script |
| Journal de clic (aucun outil ne le lit d'après le grep) | `HUD: cheat <Nom> clicked`, `HUD: cheats open|folded`, `HUD: cheat unit|gather speed xN.N`, `HUD: cheats folded (window N px high, ...)` | `UHOLGameMenuScreen::MenuLog(...)` : niveau Display (leçon 1), mêmes mots |

## Tests C++ existants du panneau
- `HeroesOfLiberty.UI.CheatPanel.Localized` (HOLScenarioPanelTests.cpp:164-~260, lu jusqu'à 219 seulement) : crée un `UHOLCheatPanel` réel, lit le texte des boutons et libellés en FR / EN / JA par les clés `HOLCheatPanel,<clé>`. Une fois le panneau script en place : ce test ne peut plus se construire (la classe disparaît) ; à retirer APRÈS un rouge puis vert d'un test script équivalent (les tests de libellé FR de `HOLCheatPanelScript_Test.as` couvrent le FR ; EN / JA restent à couvrir par un test de table ou par la capture : lacune à dire).
- `HOLHUDLayoutRules` : ses tests (CheatsStartOpen, ShouldFoldCheats) restent, la règle reste C++ (déjà liée).
- Aucun test ne lit `GetBottom` ni `Describe` : les couvrir par le scénario.

## Scénario de preuve à créer : `ecran_hud_triche` (modèle `Tools\Scenarios\ecran_hud_joueurs.json` + .txt + .look)
- json : `"mapSize": "Small"` ; txt : `hol.Test.Sanity`, `hol.HUD.Cheats 0` (état fixé), `waitkey` avant chaque étape.
1. Capture `triche_pliee` : toggle `Triches  +` seul sous la barre du haut. OBJET : le bouton « Triches  + » sous la ligne des ressources.
2. `hol.HUD.Click CheatToggle` + `waitlog HUD click CheatToggle: done` + `waitlog HUD: cheats open` : capture `triche_ouverte` : 7 boutons et 2 curseurs « Vitesse des unités : x1.0 », « Vitesse de collecte : x1.0 ». OBJET : la colonne de 7 boutons.
3. `hol.HUD.Click CheatFood` (+ `waitlog HUD: cheat CheatFood clicked`), capture : la nourriture de la barre du haut a pris +1000 (état des ressources fixé avant par `hol.Cheat.Resources`, Paysans immobiles, leçon 14).
4. `hol.HUD.Click CheatFastBuild` : le bouton dit « Construction rapide : oui ».
5. `hol.Cheat.UnitSpeed 2.0` : le curseur et son libellé suivent (« x2.0 », FollowServerSettings) ; capture.
6. `hol.HUD.Report` : `waitlog HUD cheats: cheats open, fast build on, unit speed x2.0, gather speed x1.0`.
- Jouer AVANT (copie C++) et APRÈS (script), captures découpées sur la zone du panneau et comparées à l'œil (leçon 3). Preflight : `Tools\Scenarios\preflight.ps1 ecran_hud_triche` ; `play.ps1 ecran_hud_triche -Hidden -Root <worktree>`.

## Questions pour la session principale (pas tranché, pas de fichier Q- écrit : 2 maximum)
1. Où vit le panneau en script : A) dans le cadre de l'écran script (ce que le code fait : plus de `CreateWidget` C++, une seule arborescence, `GetTopBar()` n'est plus nécessaire ; risque : z-order au-dessus des autres panneaux et clic consommé par le panneau, à régler par l'ordre d'ajout) ; B) un widget script séparé dans le viewport z 1 comme aujourd'hui (même comportement, mais garde `AHOLGameHUD` et `FindCheatPanel`). Recommandation : A. Valeur codée : A.
2. La cvar `hol.HUD.CheatsLowFold` (1 = replié en fenêtre basse, 0 = comme avant, mesure A/B de Q-heros-triches-600) : la reprendre (cvar script) ou l'abandonner (la question est tranchée = A) ? Recommandation : l'abandonner. Valeur codée : abandonnée (le pli se fait toujours).

## Champs de rapport
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 8 (Test_CheatPanelButtonsAreTheSevenInTheCppOrder, ...EveryButtonPlansTheRouteOfTheCppRequest, ...FastBuildClickAsksTheOppositeState, ...LabelsAreTheCppOnes, ...JournalAndReportLinesAreTheCppOnes, ...SpeedsMoveByStepsAndClamp, ...GreyedAndServerFollowedStates, ...StaysUnderTheTopBar) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 (vue prête, 483 l. à retirer à la bascule) ; TYPES_A_EXPOSER: HOLCheatSettings (6 fonctions), UHOLScriptScreen::SetCheatsReport ; TESTS_TAUTOLOGIQUES: 0 (les plans passent par `HOLControllerCommands::PlanOf`, les libellés par NSLOCTEXT) ; TEST_ROUGE: non vu (aucun build) ; PROCESSUS_DETACHES: aucun lancé.

## NON VÉRIFIÉ (rien compilé)
Noms et existence de `Gameplay::GetPlayerController`, `NAME_None` évité, `Math::RoundToInt`, `USlider` (SetMinValue, SetStepSize, OnValueChanged.AddUFunction avec `float32`), `Screen.MakePanel/MakeText/ConstructWidget` pour USlider / UVerticalBox / USizeBox, `HOLHUDLayoutRead::` (nom de namespace déduit de `UHOLHUDLayoutReadLibrary`) et les champs `bFold/bWasLow/bWasShown` de `FHOLHUDFoldState`, f-string `{Tenths / 10}` (division entière), `Math::Clamp` sur double, l'arrondi de `FoldLine` (`%.0f` C++ vs `RoundToInt`), `FName("Cheat" + Index + "Clicked")` (concaténation FString + int), le nom des rects dans `DescribeRects`.
