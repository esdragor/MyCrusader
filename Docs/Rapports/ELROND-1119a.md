# ELROND-1119a : HUD H-3, barre du haut et boutons Menu / Statistiques (2026-11-19, étape 4)

Impact joueur : aucun changement visible attendu ; la barre du haut (ressources, population, Inactifs) et les boutons Menu / Statistiques passent du C++ au script, mêmes places, mêmes noms, mêmes lignes de rapport.
NON COMPILÉ, NON LANCÉ, SANS CRÔCHET : 2 fichiers neufs dans `Documents\HeroesOfLiberty-Elrond\Script\UI\` ; `HOLTopBar.as` appelle des liaisons qui n'existent pas encore : **ne pas compiler le module script avec ce fichier avant le lot C++ ci-dessous** (le test non plus : même module). `HOLGameScreenScript.as`, `HOLGameScreen.cpp/.h` intacts.

## Fait
- `Script\UI\HOLTopBar.as` : `namespace HOLTopBarText` (NSLOCTEXT "HOLGameScreen" mêmes clés que le LOCTEXT C++ : Food, Wood, Stone, Gold, Population, GathererCount, IdleWorkers, Menu, Stats, IncomeTip, TributeFood/Wood/Stone/Gold), `namespace HOLTopBar` (règles pures : `FormatAmount`, `FormatRatio`, `GatherersText`, `IncomeTip`, `ResourceWord`, `ResourceLabel`, `ResourceName`, `AmountOf`, `ResourcesLine`, `BarShown`, `EconomyLine`, `WorkersLine`, `IdleLabel`, `IdleEnabled`, `IdleChanged`, `IdleClickLine`, `IncomeChanged`, constantes en FONCTIONS), `class UHOLTopBarPanel : UObject` (`Build(Screen, Frame)`, `Refresh()`, `ClickName(FName)`, `ClickIdleWorkers/ClickMenu/ClickStats`, `DescribeEconomy()`, `DescribeWorkers()`).
- `Script\UI\HOLTopBar_Test.as` : 5 tests purs (`Test_TopBarAmountsHaveNoGroupingAndRatioIsSpaced`, `Test_TopBarIncomeTipNamesTheResource`, `Test_TopBarReportLinesAreTheCppOnes`, `Test_TopBarIdleCounterLabelEnabledAndChange`, `Test_TopBarIncomeIsSetAgainOnlyWhenItChanges`). Valeurs tirées du C++ (FormatAmount sans groupement, "{0} / {1}", "({0})", "+{0} {1}/min", format `F 0 W 0 S 0 G 0` de FHOLResources::ToString vu dans les regex de quick_check:6426/6429).
- Règles reprises du C++ (HOLGameScreen.cpp : RefreshTopBar 1272-1338, RefreshIdleWorkers 2059-2072, HandleIdleWorkersClicked 2074-2081, HandleMenu/Stats 2837-2859, ClickWidget 2938-2975, Describe 3033-3108, BuildLayout 3271-3341) : barre visible ssi l'état du joueur est arrivé ; infobulle de revenu remise seulement quand le revenu change ; compteur d'inactifs appliqué seulement quand il change (-1 = jamais montré, la ligne de rapport écrit `idle max(.,0)`) ; bouton désactivé à 0 (le clic est refusé) ; « Statistiques » et sa boîte repliées sauf spectateur en match ou après le match ; journal du clic : `HUD: idle workers clicked`.

## (a) Liaison C++ à ÉCRIRE (un lot, un build) : membres de `UHOLScriptScreen` (même motif que `GetPopulationNow`, qui lit déjà `GameScreen->GetBoundPlayerState()` : l'état du joueur MONTRÉ, y compris un joueur suivi par un spectateur ; une bibliothèque de monde ne peut pas le lire)
`Source\HeroesOfLibertyUI\HUD\HOLScriptScreen.h/.cpp`, `UFUNCTION(BlueprintCallable|BlueprintPure, Category = "HeroesOfLiberty|UI")` :
```cpp
/** The shown player's economy (UHOLGameScreen::GetBoundPlayerState): stock, places taken / cap, gatherers by resource, income a minute (GetIncomePerMinute). False (all zero) before the player state arrives. */
bool GetEconomyNow(FHOLResources& OutStock, int32& OutPopulationUsed, int32& OutPopulationCap, FHOLResources& OutGatherers, FHOLResources& OutIncome) const;
/** GetOwningPlayer controller's GetIdleWorkers().Num(), 0 without a controller. */
int32 GetIdleWorkerCount() const;
/** True: the « Statistiques » button is offered (UHOLGameMenuRulesBridge::NativeShowsStatsButton(controller->IsSpectator(), game state IsMatchOver()); false without a controller). */
bool GetStatsButtonShown() const;
/** AHOLPlayerController::SelectNextIdleWorker() (HandleIdleWorkersClicked minus its journal line, which the script writes). */
void SelectNextIdleWorker();
/** UHOLGameMenuScreen::Open(GetOwningPlayer()). */
void OpenGameMenu();
/** UHOLStatsScreen::Open(GetOwningPlayer()). */
void OpenStats();
/** HOLUIStyle::Food / Wood / Stone / Gold for 0..3. */
UFUNCTION(BlueprintPure) static FLinearColor GetResourceColor(int32 Resource);
```
Plus `Source\HeroesOfLibertyUI\Widgets\HOLQueueItem.h` : `UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI") void SetLabel(const FText& InLabel);` (aujourd'hui SANS UFUNCTION : ligne 22). `OnButtonBaseClicked` (UCommonButtonBase) est déjà utilisé en script (HOLGameMenuScreen.as:41). Contrôler : `UHOLScriptScreen::ConstructWidget` public et appelable de l'extérieur de la classe (le script du HUD l'appelle sur lui-même ; mon fichier l'appelle `Screen.ConstructWidget(...)`), `FHOLResources` visible (oui : HOLSocleLibraries_Test.as:217), `UHOLQueueItem` construisible (UCLASS, ok).
Appelées par le .as : `GetEconomyNow`, `GetIdleWorkerCount`, `GetStatsButtonShown` (dans `Refresh`), `SelectNextIdleWorker` (`ClickIdleWorkers`), `OpenGameMenu` (`MenuButtonClicked`, `ClickMenu`), `OpenStats` (`ClickStats`), `GetResourceColor` (`MakeSquare` des marqueurs), `SetLabel` (boutons).

## (b) Crochet dans `Script\UI\HOLGameScreenScript.as` (mêmes lignes que dans le worktree Galadriel : à re-vérifier après la fusion du lot H-1)
1. Membre après `UCommonTextBlock VolcanoText;` (ligne 121) : `UHOLTopBarPanel TopBar;` (et après H-1 : `UHOLWaitBannerPanel WaitBanner;` déjà là).
2. `OnInitialized()` (236) : après `BuildVolcanoBanner(Frame);` (245), avant `BuildMinimap(Frame);` (246) : `TopBar = Cast<UHOLTopBarPanel>(NewObject(this, UHOLTopBarPanel)); TopBar.Build(this, Frame);`. Ordre : après la construction du cadre (239), les bannières H-1 se placent sous la barre (même overlay, elles ne se chevauchent pas : 88 px de marge haute).
3. `Refresh()` (1352) : après `RefreshVolcano();` (1360) : `if (TopBar != nullptr) TopBar.Refresh();`.
4. `ClickWidget` (2154) : avant `return false;` final (après `PauseResumeButton`, vers 2186) : `if (TopBar != nullptr && TopBar.ClickName(WidgetName)) return true;`. NB : `ClickWidget` du script est appelé AVANT celui du C++ (HOLGameScreen.cpp:2940) : le C++ garde ses `MenuButton` / `StatsButton` / `IdleWorkers` jusqu'à leur retrait.
5. `Describe()` (2280) : ne rien ajouter ; la ligne économie du C++ (voir (c)).

## (c) C++ à retirer de `HUD\HOLGameScreen.cpp/.h` (même commit que le crochet)
Retirer : `.cpp` 1272-1338 (`RefreshTopBar` : voir plus bas ce qui RESTE), 2059-2081 (`RefreshIdleWorkers`, `HandleIdleWorkersClicked`), 2837-2859 (`HandleMenuClicked`, `HandleStatsClicked`, `RefreshStatsButton`) et leurs appels (1235, 1264, 569, 652), 2953-2975 (les trois branches de `ClickWidget`), 3271-3341 (`BuildLayout` : TopBar, Resources, IdleWorkersBox, MenuPanel, MenuRow...), 424-440 (liaison des clics Idle / Menu / Stats dans `NativeOnInitialized`), constante `IdleWorkersWidth` (254) si orpheline, `IdleWorkersName/MenuButtonName/StatsButtonName` (284-286) ; `.h` : membres `TopBar`, `FoodAmount..GoldAmount`, `*Gatherers`, `PopulationAmount`, `IdleWorkers`, `MenuButton`, `StatsButton`, `ShownIncome`, `ShownIdleWorkers` (609 selon la suite), déclarations 338, 369-374.
**RESTE en C++ (à décider par le lot suivant, trois pièges)** :
1. **`Describe()` (3033-3108) lit `bEconomyShown`, `ShownResources`, `ShownPopulationUsed/Cap`, `ShownGatherers`, `ShownIdleWorkers`** pour les lignes `F.. W.. S.. G.., population U/C` et `, gatherers .., idle N` (lues par quick_check:6377, 6425-6429). Ou bien on garde un `RefreshTopBar` réduit à la copie de ces champs (5 lignes, sans widget) et un `ShownIdleWorkers` copié de `Controller->GetIdleWorkers().Num()` ; ou bien `Describe()` appelle le script (`DescribeEconomy()` / `DescribeWorkers()` du .as, identiques mot pour mot, testées). Recommandation : la première (moins de surface), la seconde au lot qui migre `Describe`.
2. **`GetTopBar()` (HOLGameScreen.h:147)** sert d'ANCRE aux panneaux de triche et de scénario : `HOLGameHUD.cpp:89` et `:97` (`SetAnchorWidget(Screen->GetTopBar())`). Le `TopBar` n'existera plus dans l'arbre C++ : il faut une porte (`UHOLScriptScreen::GetTopBarWidget()` ou `FindWidget("TopBar")`) et faire retourner `GetTopBar()` par ce widget, sinon les deux panneaux perdent leur ancre (NON VÉRIFIÉ : lecture de `SetAnchorWidget`).
3. **`MenuPanel` est lu par nom dans l'arbre C++** : `HOLScriptScreen.cpp:222` (`GameNames[] = { "PlayersPanel", "MenuPanel" }`) et `:298`, et `GetGameScreenBox(n"MenuPanel", ...)` (`HOLGameScreenScript.as:597`, bloc du héros qui évite le menu). Après le déplacement le C++ ne le trouve plus : passer par `GetWidgetBox(MenuPanel, ...)` du script (le champ est public dans `UHOLTopBarPanel`) et sortir `MenuPanel` de `GameNames`/`DescribeOverlaps` côté C++ pour le compter côté script (`MakePanel` l'y met déjà).
RESTENT aussi : `NativeTick` (cadence), `BindPlayerState` / `BoundPlayerState` (lu par `GetEconomyNow`), `UHOLGameMenuScreen::Open`, `UHOLStatsScreen::Open`, `UHOLGameMenuRulesBridge::NativeShowsStatsButton` (une source), `SetShown`/`MakePanel`/`MakeText` du C++ (autres panneaux), `FormatAmount`/`FormatRatio` de l'espace anonyme tant qu'un autre panneau les appelle (grep avant retrait).

## (d) Outils -> ce qu'ils lisent -> où le script l'écrit
| Outil (fichier:ligne) | Lit | Où le script l'écrit |
|---|---|---|
| `Tools\Tests\quick_check.ps1:1560` | `hol.HUD.Click IdleWorkers`, puis journal `Next idle worker:` | `ClickWidget` -> `TopBar.ClickName("IdleWorkers")` -> `ClickIdleWorkers` : `Log("HUD: idle workers clicked")` puis `SelectNextIdleWorker` (le contrôleur écrit `Next idle worker:`) ; refus si désactivé |
| `Tools\Tests\quick_check.ps1:6425-6429` (+ 6377 autres joueurs) | ligne `HUD ...` du `hol.HUD.Report` : `, gatherers F 0 W 0 S 0 G 0, idle 1$`, `gatherers F 0 W 1 S 0 G 0, idle 0` | `DescribeWorkers()` (ou C++ gardé, voir (c).1) ; format testé par `Test_TopBarReportLinesAreTheCppOnes` |
| `Tools\Tests\quick_check.ps1:768` | `hol.UI.Focus StatsButton` (victoire) | nom `StatsButton` gardé (MakeQueueButton) ; trouvé par l'outil de focus : vérifier qu'il cherche aussi dans l'arbre du script (NON VÉRIFIÉ : `hol.UI.Focus`) |
| `Tools\Tests\lobby_match.ps1:164,170,586` | `hol.HUD.Click StatsButton` -> `Stats screen: opened` ; `& $Press StatsButton` | `ClickStats` -> `OpenStats` (le journal `Stats screen: opened` est écrit par `UHOLStatsScreen::Open`, inchangé) ; refus si replié |
| `Tools\Tests\lobby_match.ps1:165` | `hol.HUD.Click MenuButton` -> `Game menu: opened` | `ClickMenu` -> `OpenGameMenu` (`UHOLGameMenuScreen::Open` écrit la ligne) |
| `Tools\Scenarios\income_tooltip.txt:19,25,36,45` | `hol.Mouse.Hover WoodAmount` / `GoldAmount`, `waitlog Mouse hover WoodAmount` | noms `FoodAmount..GoldAmount` gardés ; Visible (D788) + `SetToolTipText` : l'outil de survol doit trouver ces widgets dans l'arbre du script (NON VÉRIFIÉ) |
| `Tools\Scenarios\peasant_fight.txt:17` | note texte « Inactifs » | rien à porter |
| `HOLGameHUD.cpp:89,97` | `GetTopBar()` (ancre des panneaux) | voir (c).2 |
| `HOLScriptScreen.cpp:222,298`, `HOLGameScreenScript.as:597` | `MenuPanel` par nom dans l'arbre C++ | voir (c).3 |
| `hud_rects_game_test.ps1`, `hud_rects_test.ps1`, `HudRects.ps1` | rectangles (`DescribeOverlaps`/`DescribeRects`) : `TopBar` et `MenuPanel` deviennent des panneaux du script (`MakePanel`), donc comparés entre eux : doivent rester sans chevauchement (ils le sont : gauche et droite) | (non relu : à lancer après le build) |
Grep fait sur `Tools\` pour TopBar / IdleWorkers / MenuButton / StatsButton / Inactifs / *Amount : les lignes ci-dessus sont tout ce qui sort ; aucun outil ne lit le texte « Inactifs : N » ni `Statistiques`. `hol.HUD.Report` : voir `Describe` (c).1.

## (e) Suspects de compilation
- `Screen.ConstructWidget(UImage, Name)` appelé depuis un autre objet (le script du HUD l'appelle sur lui-même) ; `Cast<UHOLQueueItem>(...)` ; `Button.OnButtonBaseClicked.AddUFunction(this, FName(...))` avec un nom construit à l'exécution (le `UFUNCTION()` doit exister sous le nom exact : `IdleWorkersClicked`, `MenuButtonClicked`, `StatsButtonClicked` : ils existent) ; `UFUNCTION()` sans argument dans une classe `UObject`.
- `FHOLResources` passé par valeur dans un namespace (comme HOLCommandGrid_Test) ; `const FString`-retour `DescribeEconomy() const` appelle une fonction de namespace avec des champs membres : peut demander de retirer le `const`.
- `FText::GetEmpty()`, `SetToolTipText`, `SetDesiredSizeOverride`, `FMargin(Margin)` (1 argument), `UHorizontalBoxSlot`, `USizeBox.SetWidthOverride/HeightOverride`, `UBorder.SetPadding(FMargin)`, `Math::Max(int,int)`, `f"..."` avec `{Stock.Food}` (accès champ dans une f-string : à vérifier, sinon variable locale).
- Nom de membre `AmountOf` dans la classe ET dans le namespace `HOLTopBar` : portées distinctes, mais si l'avertissement apparaît, renommer le membre `AmountBlock`.
- Plusieurs lignes `UHOLTextStyleHUD` / `UHOLTextStyleHUDLabel` : vues dans le plan comme déjà utilisées en script.

## (f) Deux rouges à poser UNE fois (dans le worktree, puis remettre)
1. `HOLTopBar.as`, `FormatAmount` : `return f"{Value}";` -> `return f"{Value + 1}";` : rouge `Test_TopBarAmountsHaveNoGroupingAndRatioIsSpaced` ("1200" devient "1201").
2. `HOLTopBar.as`, `WorkersLine` : `Math::Max(ShownIdleWorkers, 0)` -> `ShownIdleWorkers` : rouge `Test_TopBarReportLinesAreTheCppOnes` (le cas -1 écrit "idle -1" au lieu de "idle 0").
(Un troisième si on veut : `IdleEnabled` `Count > 0` -> `Count >= 0` : rouge `Test_TopBarIdleCounterLabelEnabledAndChange`.)
Tests de la vue (widgets, boutons) : non écrits, la vue ne se teste qu'en jeu (scénario (g)). Aucun test tautologique : chaque assertion passe par une règle.

## (g) Scénario d'essai de 3 min : `ecran_hud_hautdroite` (solo, toi contre une IA ; modèle `resources_compare.txt` : en-tête de 2 lignes de commentaire, `frames`, `capture`, `notereset`, `waitkey`)
`Tools\Scenarios\ecran_hud_hautdroite.txt` :
```
# Barre du haut et boutons Menu / Statistiques du HUD, passés du C++ au script (H-3). Tu verras la barre en haut a gauche, le bouton Menu en haut a droite ; Entree = etape suivante. 3 minutes au plus.
# Rien n'est sauvegarde. Plain, toi contre une IA.
frames 90
hol.Cheat.AddResources 500 300 100 50
frames 30
notereset ETAPE 1/3 : en haut a gauche, une barre sombre : carre colore + nom + montant pour Nourriture, Bois, Pierre, Or (chacun suivi de son nombre de recolteurs entre parentheses), puis Population "n / m", puis un bouton "Inactifs : n". En haut a droite un bouton "Menu" seul. Entree = suivant.
capture hud_hautdroite_barre
waitkey Entree : survol et clic
hol.Mouse.Hover WoodAmount 30
waitlog Mouse hover WoodAmount
frames 20
notereset ETAPE 2/3 : la souris est sur le montant de Bois : une bulle "+N bois/min" doit apparaitre. Entree = suivant.
capture hud_hautdroite_bulle
waitkey Entree : bouton Menu
hol.HUD.Click MenuButton
waitlog Game menu: opened
waitui
frames 5
notereset ETAPE 3/3 : le menu du jeu est ouvert (cliquer sur Menu a agi comme Echap). Entree = fin.
capture hud_hautdroite_menu
waitkey Entree : fin
```
`Tools\Scenarios\ecran_hud_hautdroite.look` : une ligne par capture : `hud_hautdroite_barre` : « barre du haut a gauche : 4 carres (nourriture, bois, pierre, or) avec montants 500 / 300 / 100 / 50 + "(0)" , Population "n / m", bouton "Inactifs : 0" grise ; en haut a droite le bouton "Menu" seul (pas de "Statistiques") » ; `hud_hautdroite_bulle` : « bulle "+N bois/min" au-dessus du montant de Bois » ; `hud_hautdroite_menu` : « le menu du jeu par-dessus le HUD ». Pour la variante spectateur (bouton Statistiques) : essai séparé, hors des 3 min (spectateur = partie réseau). NON VÉRIFIÉ : le nom exact de la commande de ressources (`hol.Cheat.AddResources` est supposé : relire `Tools\Scenarios\idle_wall.txt` / `alert_population.txt` avant d'écrire le .txt) ; `waitui` et `notereset` copiés de l'usage vu dans `resources_compare.txt` et `lobby_match.ps1:165`.

## NON VÉRIFIÉ
- Rien compilé, rien lancé. Tous les noms Angelscript sont écrits d'après le C++ et le motif de HOLWaitBanner.as (non compilé non plus) ; voir (e).
- Look : `UHOLQueueItem` est construit pour garder le style C++ des boutons (`MakeButton` donnerait un `UButton` au style HUD, différent). Si `SetLabel`/`ConstructWidget` posent problème, repli : `MakeButton` + `MakeText`, avec un écart de look à voir sur la capture.
- Le texte « Statistiques » et les clés NSLOCTEXT : namespace « HOLGameScreen » supposé celui de `LOCTEXT_NAMESPACE` du C++ (même hypothèse que H-1, qui l'a vu) ; les traductions (`fill_po.py`) non regardées.
- Ordre d'affichage : le cadre du script est au-dessus du cadre C++ ; la barre passe donc au-dessus de l'alerte majeure C++ (comme les bannières H-1).
- `GetBoundPlayerState` via `UHOLScriptScreen::GetTypedOuter<UHOLGameScreen>()` (motif de `GetPopulationNow`, vu).
- Doute noté sans question : « Statistiques » règle `NativeShowsStatsButton` laissée en C++ (pas de jumeau script).

BUILDS_DEMANDES: 0 (ce lot) ; 1 pour le lot C++ suivant. TESTS_SCRIPT_AJOUTES: 5. TESTS_CPP_RETIRES: 0. TYPES_A_EXPOSER: membres UHOLScriptScreen ci-dessus + `UHOLQueueItem::SetLabel`. TESTS_TAUTOLOGIQUES: 0.
