# SAURON-1119m : HUD H-10, la grille de commandes 3x4 (vague 1 : fichiers neufs + recette, AUCUNE bascule)

2026-11-19, worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` (envelope = main 3b52f7a, binaire a jour). Ni build, ni quick_check, ni editeur, ni play.ps1.
Impact joueur : aucun a cette vague (rien n'est branche) ; apres bascule la grille A Z E R / Q S D F / W X C V doit etre identique (touches, libelles, infobulles, clic, journal).

## Livre
- `Script\UI\HOLCommandGridPanel.as` : namespace `HOLCommandGridPanelRules` (regles pures) + classe `UHOLCommandGridPanel` (la vue). Nom de classe sans heurt avec une struct `F...View` (leçon 10).
- `Script\UI\HOLCommandGridPanel_Test.as` : 8 tests (cles / noms de cellules, 12 cles par ligne, etats et case vide sombre, visibilite du panneau, ligne de journal mot pour mot, clic ordre ou grille, clic permis, texte de contenu du rapport).
- NE COMPILENT PAS SEULS (marques « NOT COMPILED YET » en tete) : `UHOLCommandGridPanel` appelle des liaisons C++ qui n'existent pas encore (ci-dessous) ; tant que le lot C++ n'est pas la, le module script entier ne compile pas avec ces deux fichiers : NE PAS builder avec eux sans le lot (meme regle que `HOLAgePanels.as`, GALADRIEL-1119a). A garder hors du worktree de build, ou a poser avec le lot.
- Rouge VU : pas fait (rien ne compile). A faire avec le lot : fausser « AZERQSDFWXCV » d'une lettre dans `Test_GridPanelKeysFollowTheAzertyRows`, voir rouge, rendre.

## Ce que fait la vue (script) et ce qui reste C++ (decisions donnees, pas rediscutees)
- Script : construit `GridPanel`, `Grid`, `GridCell00..11` (memes noms, memes tailles 76 / ecart 2, memes styles), pose les touches (`SetKey` des la construction : `UHOLGridCell` garde la cle donnee avant `NativeOnInitialized`, HOLGridCell.h:138 et le correctif 15e4094 : le piege passe), cache / montre le panneau (Hidden jamais Collapsed), remplit ou efface chaque cellule, relaie le clic, ecrit la ligne de journal par `UHOLGameMenuScreen::MenuLog`.
- C++ garde : les cellules `UHOLGridCell` (peinture, `SetTooltip` avec cout / manque, `SetGain`, `DescribeContent`, `DescribeTooltip`), le CONTENU de chaque case (le `switch` de `RefreshGrid`, HOLGameScreen.cpp:1855-2128 environ : Produce, Research, Specialize, Build, Trade, Cancel, ordres, HeroAbility, NecroticRage : libelle, etat, cout, infobulle, mirrors `Shown*`), une seule source jusqu'a l'etape 5.
- QUESTION POUR LA SESSION PRINCIPALE (pas de design tranche ici) : les ~350 lignes citees sont a ~80 % du CONTENU (calcul des libelles / couts / infobulles), pas de la vue. Selon la decision donnee, ce contenu reste C++ : il est DEPLACE (extrait en `ComputeGridFrame`, ecrit dans un cache au lieu de toucher les cellules), pas supprime. Ce qui sort vraiment de `HOLGameScreen.cpp` : ~60 lignes (liste plus bas). Si l'on veut que les libelles et infobulles passent vraiment en script, c'est une decision a part (les helpers `HOLGameNames::*`, `GetBuildingEffect`, `GetResearchBlockText`, marche `HOLMarket` ne sont pas exposes) : defaut recommande = non avant l'etape 5.

## RECETTE d'integration (a executer avec le lot C++ de la session)

### 1. Liaisons a ajouter dans `UHOLScriptScreen` (HOLScriptScreen.h / .cpp), signatures exactes
```cpp
/** HUD H-10: the host (a vertical box, name GridPanelHost) the script's command grid is put in; it takes the place GridPanel had in CommandColumn. */
UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
UVerticalBox* GetGridPanelHost() const;

/** HUD H-10: the grid redraws from the frame the C++ screen just computed (selection, stock or hero/rage/deploy tick). Like RefreshAges (HOLScriptScreen.h:57). */
UFUNCTION(BlueprintImplementableEvent, Category = "HeroesOfLiberty|UI")
void RefreshGridCells();

/** The last frame: per cell its kind / state / order type (no widget touched); bOutGrid / bOutOrders = the screen's bGridShown / bOrdersShown. */
UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
void GetGridFrame(TArray<FHOLGridCellInfoView>& OutCells, bool& bOutGrid, bool& bOutOrders) const;

/** Applies the cached fill of cell Cell (SetCommand, SetTooltip, SetGain when quoted) to Target, and keeps ShownTradeLabels[Cell] = Target->DescribeContent() as the C++ did. */
UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
bool ApplyGridCell(int32 Cell, UHOLGridCell* Target);

/** The click: UHOLGameScreen::HandleGridCellClicked without its UE_LOG (the script writes the line): order cell (ActivateOrderCell) or grid cell (ActivateGridCell). */
UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
void ClickGridCell(int32 Cell);

/** The screen keeps the cells for its report (GridCells: DescribeCells, DescribeTooltips, Describe's states, ClickWidget's GetIsEnabled). */
UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
void RegisterGridCells(const TArray<UHOLGridCell*>& Cells);
```
Struct (HOLScriptScreen.h or HOLGameScreen.h, USTRUCT(BlueprintType)) : `FHOLGridCellInfoView { int32 Kind = 0 /* 0 none, 1 command */; int32 State = 2 /* EHOLGridCellState as int: 0 Ready 1 Greyed 2 Disabled */; FName OrderType; /* = ShownOrderTypes[Cell] */ }`. Le cache complet (Name, Cost, Missing, Effect, Gain) reste un `TArray<FHOLGridCellFill>` prive de `UHOLGameScreen` (non expose : `FHOLResources` / `FHOLTradeGain` ne sont pas utilises par le script).
`UHOLGameScreen` : `GetGridPanelHost()` (membre `UVerticalBox* GridPanelHost`, comme `AgePanelsHost`, HOLGameScreen.h:127), les 4 UFUNCTION relaient comme `FillAgeCell` (H-8).

### 2. A RETIRER / TRANSFORMER dans HOLGameScreen.cpp (numeros ACTUELS, main 3b52f7a ; a revérifier apres les autres panneaux : les lignes bougent)
| Lignes | Quoi | Action |
|---|---|---|
| 242-245 `GridKeys`, 246-247 `GridColumns` `CellSize` | cles / constantes | GARDER `GridKeys` (Describe, DescribeCells, DescribeTooltips, ordres) ; `GridColumns`, `CellSize`, `CellGap`, `GridCellName` (278-281) a retirer si plus aucun usage (verifier le grep : AgeCellName ne les utilise pas) |
| 381-390 | boucle `FindWidget(GridCellName)` + `SetKey` + `OnClicked().AddUObject(HandleGridCellClicked)` | RETIRER (les cellules viennent de `RegisterGridCells`, cle et clic poses par le script) ; GARDER 402-403 `ShownGridTypes.Init`, `ShownOrderTypes.Init` |
| 1787-1809 debut de `RefreshGrid` (Commands, Orders, bGridShown, bOrdersShown, bHeroCellsShown) | calcul du contenu | GARDER (devient `ComputeGridFrame`) |
| 1810-1816 | `if (GridPanel) ... SetVisibility(Wanted)` | RETIRER (le script cache / montre : `PanelVisible`) |
| 1819-1823 `RefreshAges()` | | GARDER |
| 1825-2128 boucle `switch` sur `GridCells[Cell]` | `GridCell->SetCommand / SetTooltip / SetGain / ClearUnit` | TRANSFORMER : au lieu d'appeler `GridCell->...`, ecrire `GridFrameFill[Cell]` (Name, State, TooltipName, Cost, Missing, Effect, Gain, bGain) ; `ShownGridTypes`, `ShownOrderTypes`, `ShownTradeLabels`, `ShownMarketPrices`, `ShownEngineTier` inchanges (Describe mot pour mot, leçon 6) ; `ShownTradeLabels[Cell] = ...DescribeContent()` passe dans `ApplyGridCell` (la cellule doit etre remplie avant) ; le `continue` sur `!GridCell` disparait. A la fin : `if (ScriptScreen) ScriptScreen->RefreshGridCells();` |
| 2191-2208 `HandleGridCellClicked` | clic + 2 `UE_LOG(... Display ...)` | DEVIENT le corps de `ClickGridCell` (sans les 2 `UE_LOG` : le script ecrit « HUD: order cell X clicked » / « HUD: grid cell X clicked », niveau Display compris par `MenuLog`) ; retirer de HOLGameScreen.h:332 |
| 2345-2355 boucle de `ClickWidget` sur `GridCells` | clic par nom | RETIRER (le `ClickWidget` du script, `ClickName`, passe avant, comme H-7 / H-8) ; GARDER `return false` |
| 2797-2829 (2797-2808 + 2829) | `GridPanel = MakePanel`, `Grid`, 12 `USizeBox` + `UHOLGridCell` | REMPLACER par `GridPanelHost = MakeVerticalBox(TEXT("GridPanelHost")); CommandColumn->AddChildToVerticalBox(GridPanelHost)->SetHorizontalAlignment(HAlign_Left);` (meme place : sous `QueuePanel`, avant `AgePanelsHost`) ; retirer 2829 `GridPanel->SetVisibility(Hidden)` (le script cree le panneau Hidden) |
HOLGameScreen.h : retirer le membre `GridPanel` (:284) et `HandleGridCellClicked` (:332) ; garder `GridCells` (:348, rempli par `RegisterGridCells`), `ShownGridTypes`... (:479-500) ; ajouter `GridPanelHost`, le cache `GridFrameFill`. `RefreshGrid` (:329) reste (point d'entree des 2 appelants : HOLGameScreen.cpp:503 tick heros / rage / deploy, :978 apres selection).
Lignes reellement retirees : ~10 + 7 + 11 + 27 + 18 = ~73 ; lignes deplacees (pas supprimees, `switch`) : ~300.

### 3. Crochet dans `Script\UI\HOLGameScreenScript.as` (fichier partage : PAS touche a cette vague)
- membre `UHOLCommandGridPanel CommandGrid;`
- `OnInitialized` : `CommandGrid = Cast<UHOLCommandGridPanel>(NewObject(this, UHOLCommandGridPanel)); CommandGrid.Build(this);` APRES la construction de la file (`UHOLProductionQueuePanel`, H-7 : GridPanel est sous QueuePanel) et AVANT les paliers d'ages (`UHOLAgePanelsPanel`), comme le C++ les posait.
- surcharge `UFUNCTION(BlueprintOverride) void RefreshGridCells() { if (CommandGrid != nullptr) CommandGrid.Refresh(); }`
- `ClickWidget(FName Name)` : ajouter `if (CommandGrid != nullptr && CommandGrid.ClickName(Name)) return true;` aux cotes de `QueuePanel.ClickName` et `AgePanels.ClickName`.
- Ne pas oublier `RegisterGridCells` AVANT le premier `RefreshGrid` (il est dans `Build`, donc avant `Refresh()` initial de l'ecran). Attention a `UHOLGameScreen::NativeOnInitialized` : `ShownGridTypes.Init` (402) est APRES la creation du script (380) : sans consequence, `Build` ne lit pas `Shown*`.

### 4. Appele a chaque image (cout)
- Aucun appel par image du script pour la grille. `Refresh` ne tourne qu'a chaque `RefreshGrid` : au changement de selection, de stock, de palier (HOLGameScreen.cpp:515-530) et, tant qu'une case heros / rage / deploy est affichee, a chaque tour du minuteur de la tick (HOLGameScreen.cpp:494-505, ~0,25 s). Par `Refresh` : 1 `GetGridFrame`, 12 `Visibility` / `ClearUnit` ou `ApplyGridCell` (C++).
- A mesurer apres bascule : `hol.HUD.Report` ligne « grid ... » identique (C++ inchange) ; pour le cout, ajouter dans le lot un compteur `GridRefreshCount` ecrit par `ClickGridCell`/`ApplyGridCell` dans `Describe`? NON : ne pas changer `Describe` (quick_check lit la ligne). Mesure proposee : `stat` script du plugin sur `UHOLCommandGridPanel.Refresh` pendant un scenario avec heros selectionne (`legions_warrior_compare`) ; NON FAIT, rien d'instrumente par moi.

## Table « outil -> texte ou nom de widget lu -> ou le script l'ecrit » (grep reel de `Tools\` sur le worktree)
| Outil (fichier:ligne) | Lit | Ou c'est ecrit apres bascule |
|---|---|---|
| `Tools\Tests\quick_check.ps1:1374` | `hol.HUD.Click GridCell00` puis `waitlog queued Peasant (1 in` | nom `GridCell00` : `UHOLCommandGridPanel.Build` (`CellNameText`) ; le texte `queued Peasant` vient du controleur (`ActivateGridCell`), inchange |
| `quick_check.ps1:1694` | `hol.HUD.Click GridCell05` puis `waitlog Order: ` | idem (cellule 5 = S arret, ordres) ; `Order: ` vient du controleur |
| `quick_check.ps1:1705` | `hol.HUD.Click GridCell11` puis `Construction: slot 0 cancelled House` | idem (V annuler) |
| `quick_check.ps1:3694, 4279, 4849, 5284` | `HUD report: ... grid A=Peasant enabled`, `grid V=Cancel enabled`, `A=CloseRanks enabled Z=Banner greyed`, `grid A=BuyFood \w+ ... trades` | `Describe()` C++ INCHANGE : lit `ShownGridTypes` + `GridCells[Cell]->GetState()` (cellules du script donnees par `RegisterGridCells`) ; l'etat Ready / Greyed est celui de la cellule remplie par `ApplyGridCell` |
| `quick_check.ps1:4844-4853` | `HUD cells: A: Rangs (59\|60) s`, `HUD tooltips:` | `DescribeCells` / `DescribeTooltips` C++ inchanges (`DescribeContent` / `DescribeTooltip` des cellules enregistrees) |
| `quick_check.ps1:4254, 5239` | `grid ages ...` | H-8 (paliers), non touche |
| `Tools\Scenarios\ecran_hud_file.txt:17` | `hol.HUD.Click GridCell00` | nom `GridCell00` (script) |
| `Tools\Scenarios\legions_warrior_compare.txt:38,51,64,80,93` ; `legions_warrior.txt` (commentaires) | `hol.HUD.Click GridCell02` (case E, rage) | nom `GridCell02` (script) ; `ClickAllowed` exige la cellule activee (`GetIsEnabled`), comme le C++ |
| `Tools\Scenarios\specialization_button.txt:24,35` | `hol.Mouse.Click GridCell06` + `waitlog Mouse click GridCell06 x1 at` | `HOLUIMouse` cherche le widget par nom dans l'arbre (leçon 5) : `GridCell06` construit par `Screen.ConstructWidget` doit etre trouve ; ligne ecrite par HOLUIMouse, pas par la grille |
| `Tools\Scenarios\legions_roster.txt:80-101`, `legions_roster_gesture.txt:28-135`, `alert_population.txt:27`, `legions_roster_gesture_dawn.txt:25` | `waitlog HUD cells: A: ...` / `HUD tooltips:` | `DescribeCells` / `DescribeTooltips` C++ (inchanges) |
| `Source\HeroesOfLibertyUI\Dev\HOLUIHUD.cpp:22-24, 59-60, 216` | texte d'aide de `hol.HUD.Click` ; `HUD cells:` / `HUD tooltips:` | rien a changer ; ligne 22-24 dit GridCell04 / 05 / 11 pour les ordres (le code reel est Q=4, S=5, V=11) |
| `Tools\Tests\lobby_match.ps1`, `Tools\Scenarios\hud_rects_test.ps1`, `hud_rects_game_test.ps1`, `tier_grid_client_run.ps1` | AUCUN `GridCell`, `GridPanel`, `grid cell`, `HUD cells`, `HUD tooltips` (grep reel) ; `tier_grid_client_run.ps1:33` lit `grid [^;]*?(?=, orders\|, queue\|$)` de la ligne `HUD report` | `Describe()` C++ inchange |
| Lignes de journal `HUD: grid cell X clicked` / `HUD: order cell X clicked` | AUCUN outil ne les lit (grep `Tools\` : rien) | `MenuLog` du script (`ClickLine`) : memes mots ; le niveau « Display » est celui de `MenuLog` (leçon 1) |

## Scenarios de preuve
- Existants AVANT / APRES : `peasant_grid` (Paysans : S arret, V maintien, pas de Q ; batisseurs, paliers), `tech_grid` (Ferme : W X C V Charrue / Brouette...), `siege_grid` (Atelier de siege : Belier / Mangonneau grises par palier), `tech_grid_shot` (capture de la grille « Rechercher » + lecture des infobulles). Les 4 prennent des captures : decouper la zone `GridPanel` (rect de `DescribeRects`) avant (C++) et apres (script), comparer a l'oeil (leçon 3). Preflight de chacun a lancer par la session (`Tools\Scenarios\preflight.ps1 <nom>`) : NON FAIT ici.
- A CREER : `ecran_hud_grille` (.json avec `"mapSize": "Small"`, .txt avec `hol.Test.Sanity`, `waitkey` avant chaque etape, .look nommant l'objet : le panneau 3 x 4 en bas a gauche sous la file). Modele `Tools\Scenarios\ecran_hud_joueurs.json` / `ecran_hud_ages.*`. Etapes : (1) rien de selectionne : panneau cache ; (2) Paysans, ressources par triche, immobiles : A Maison... via les paliers, S / V des ordres (`order cell`) ; (3) Forum : A Paysan actif, une case grisee (stock vide) ; (4) Ferme : W X C V, infobulle `hol.Mouse.Hover GridCell08` ; (5) Marche : trades `A=Nourriture +96 [Food]` ; (6) heros : A Rangs / Z Banniere grisee (niveau 1) ; (7) clic `hol.HUD.Click GridCell00` : ligne `HUD: grid cell A clicked` ; (8) clic sur une case grisee : le message, pas d'ordre. Une capture par point.
- Comparaison de ligne : `hol.HUD.Report` (`grid`, `trades`) AVANT / APRES doit etre identique mot pour mot.

## Risques a surveiller a la bascule
1. `RegisterGridCells` avant le premier `RefreshGrid` (sinon `GridCells` vide : `ShownTradeLabels.SetNum(0)`, rapport « empty »).
2. `ApplyGridCell` doit remplir AVANT de lire `DescribeContent` (marche : `ShownTradeLabels`).
3. Le `UHOLGridCell::SetKey` precoce passe (cle gardee avant l'initialisation) ; `SetCommand` / `SetTooltip` ne sont appeles qu'a `Refresh`, apres l'initialisation (leçon 4).
4. `OnCellClicked` du script : un seul gestionnaire pour 12 cellules (ici `UFUNCTION() OnCellClicked(UHOLGridCell)` + recherche de l'index, comme `HOLAgePanels.as`) : OK, pas de `HandleCellN`.
5. Conflit de fusion : `HOLGameScreen.cpp/.h` et `HOLGameScreenScript.as` bougent avec chaque panneau (leçon 15).
6. SAURON-1119n touche `HOLPlayerController.cpp` et `HOLControllerCommands.as` : `ActivateGridCell` / `ActivateOrderCell` sont dans ce controleur, ne pas en changer la signature.

## Champs du rapport
- BUILDS_DEMANDES: 0 (pour cette vague) ; le lot C++ de la bascule : 1 build (UHOLScriptScreen, HOLGameScreen, struct de vue)
- TESTS_CPP_RETIRES: 0
- TESTS_SCRIPT_AJOUTES: 8 (Test_GridPanelKeysAndNamesFollowTheCells, Test_GridPanelKeysFollowTheAzertyRows, Test_GridPanelCellStateWordsAndEmptyCellIsDark, Test_GridPanelShownOnlyWithGridOrOrders, Test_GridPanelClickLineIsTheCppsWordForWord, Test_GridPanelOrderClickOnlyWithoutGridAndWithAnOrder, Test_GridPanelClickAllowedNeedsAShownPanelAndAnEnabledCell, Test_GridPanelContentPartIsTheReportsCellText)
- TEST_ROUGE: NON VU (rien ne compile avant le lot) ; TESTS_TAUTOLOGIQUES: 0 (litteraux copies du C++ et des outils)
- LIGNES_CPP_SUPPRIMEES: 0 (vague 1) ; prevues ~73 supprimees + ~300 deplacees
- LIGNES_MIGREES_PAR_LOT: ~230 lignes de script neuves (vue + regles) ; contenu `switch` NON migre (decision donnee)
- TYPES_A_EXPOSER: FHOLGridCellInfoView (USTRUCT BlueprintType) ; UHOLScriptScreen::GetGridPanelHost, RefreshGridCells, GetGridFrame, ApplyGridCell, ClickGridCell, RegisterGridCells ; UHOLGridCell::OnCellClicked, SetKey, ClearUnit, GetIsEnabled (deja exposes par H-8 : SetKey, ClearUnit, OnCellClicked a verifier)
- EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: aucun lance
- Questions : 1 (contenu du switch, voir plus haut) ; a poser par la session si besoin dans `Retours\Q-grille-contenu.md` (je n'ai pas cree le fichier).
