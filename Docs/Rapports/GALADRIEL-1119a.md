# GALADRIEL-1119a : HUD H-8, panneaux d'âges (médaillons I à IV, touches A Z E R) en script

Branche `galadriel-tab` (worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`), partie de main 20a5e2f. Lot SANS C++, aucun build, aucun quick_check.
Fichiers neufs : `Script\UI\HOLAgePanels.as` (règles pures `HOLAgePanels` / `HOLAgePanelsText` + vue `UHOLAgePanelsPanel`), `Script\UI\HOLAgePanels_Test.as` (8 tests).
**Les deux fichiers appellent des liaisons qui n'existent pas encore (liste en (a)) : le module script ne compile pas tant que le lot C++ n'est pas écrit** (même convention que ELROND-1119a).
Le plan parlait de « médaillons I à IV, touches A Z E R » : dans le code ce sont `AgePanel<age>` (4 panneaux côte à côte dans la barre du bas, à droite du panneau de commandes), chacun avec son médaillon (numéro dans un disque + touche dans un carré, sans clic) au-dessus de ses 12 `UHOLGridCell` (`AgeCell<age><cell>`, touches Q S D F / W X C V). Le clic sur un bâtiment va à `AHOLPlayerController::ClickAge`.

## Ce que le C++ fait (HOLGameScreen.cpp de ce worktree)
- Construction : 3204-3254 (lambdas locales `MakePanel`, `MakeText`, `AddToBar` : 2883-2897, 3029) ; constantes 246-248, 263, 267-269, 315 ; noms 317-326 ; recherche des widgets 444-462 ; effet de la mise en page `MedallionRoom` 2996-3002 (reste C++, c'est la place libre au-dessus de la colonne).
- Rafraîchissement : `RefreshAges` 2237-2353, appelé de 565 (palier du héros changé : `ShownTier`), 624 (âge choisi changé : `ShownChosenAge`), 1967.
- Clic : `HandleAgeClicked` 2458-2468 (journal `HUD: age I A clicked`) ; `ClickWidget` 2648-2662.
- Rapport : `Describe` 2686-2711 (ligne `grid ages ...`), 2794-2806 et 2822-2840 (contenu et infobulles des cellules d'âge ; 2830-2840 lu seulement en partie).

## (a) Liaison C++ à écrire (un seul lot, dans `UHOLScriptScreen`, `Source\HeroesOfLibertyUI\HUD\HOLScriptScreen.h/.cpp`, sauf mention)
Tous les appels sont écrits dans HOLAgePanels.as tels qu'ils seront.
1. `UFUNCTION(BlueprintPure) UHorizontalBox* GetAgePanelsHost() const;` : un `UHorizontalBox` nommé `AgePanelsHost` que le C++ ajoute à la barre du bas à la place où `AgePanel0` était ajouté (`AddToBar(Host, 0.f)`, 3207) ; le script y met les 4 panneaux. Le C++ garde `MedallionRoom`.
2. `UFUNCTION(BlueprintCallable) void GetAgesContext(FHOLGridContextView& OutContext) const;` : lit exactement 2245-2250 : si `Controller` valide et `!GetOwnSelection()` et `!HasBuildingGroup()` alors `GetUnitsContext(FHOLGridContext)` converti en `FHOLGridContextView` (la conversion existe déjà côté `HOLCommandGridReadLibrary`), sinon contexte vide.
3. `int32 GetChosenAge() const` (lit `Controller->GetChosenAge()`, -1 sans contrôleur) et `int32 GetTier() const` (lit 2253 : `BoundTableState->GetTier(PlayerIndex)`, -1 sinon).
4. `void ClickAge(int32 Age, int32 Cell)` : `Controller->ClickAge(Age, Cell)` (le journal est écrit par le script).
5. `bool FillAgeCell(UHOLGridCell* Cell, const FHOLGridCommandView& Building)` : le corps de 2342-2350 (FindType, GetBuildCost avec le joueur lié, GetMissing, nom du bâtiment, état Ready ou Greyed, SetCommand, SetTooltip avec `Palier non atteint` ou l'effet). Reste en C++ : la règle de coût, les noms, l'effet.
6. `void SetAgesReport(bool bShown, int32 Chosen, const TArray<FName>& Types, const TArray<bool>& Reached, const TArray<FString>& Tooltips)` : recopie dans `bAgesShown`, `ShownChosenAge`, `ShownAgeTypes`, `ShownAgeReached`, `ShownAgeTooltips` de `UHOLGameScreen` (via `ScriptScreen`) pour que `Describe` et `ClickWidget` (C++ restant) les lisent sans widget.
7. `static void SetRoundedBrush(UBorder*, FLinearColor Fill, float Radius, FLinearColor Outline, float OutlineWidth)` (= `SetBrush(FSlateRoundedBoxBrush(...))`, 2270/2275/3224/3235) et `static FLinearColor GetStyleColor(int32 Which)` (0 `HOLUIStyle::Accent`, 1 `Text`, 2 `TextMuted`, 3 `HUDPanel`) : si l'un existe déjà dans HOLScriptScreen (cherche avant d'écrire), adapte les noms dans HOLAgePanels.as (`PaintMedallion`, `PaintPanel`).
8. `UHOLGridCell` (`Source\HeroesOfLibertyUI\Widgets\HOLGridCell.h`) : `UFUNCTION(BlueprintCallable)` sur `SetKey(const FText&)`, `SetCommand(const FText&, EHOLGridCellState)`, `ClearUnit()`, `GetState()` ; `UENUM(BlueprintType) EHOLGridCellState` ; `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHOLGridCellClickedEvent, UHOLGridCell*, Cell)` et `UPROPERTY(BlueprintAssignable) FHOLGridCellClickedEvent OnCellClicked;` diffusé depuis le `OnClicked` Common du bouton (`HandleClicked`). `SetTooltip` et `SetGain` n'ont pas besoin d'être exposés (FillAgeCell les appelle). La classe doit être construisible par `Screen.ConstructWidget(UHOLGridCell, Nom)` (même mécanisme que `UHOLQueueItem`).
9. Exposer au script si ce n'est pas fait : `HOLCommandGridRead::IsBuildGrid(FHOLGridContextView)` (déclaré 330 de HOLCommandGridReadLibrary.h, donc OK), `FHOLGridContextView` (déjà utilisé par HOLCommandGrid_Test.as).

## (b) Crochet dans `Script\UI\HOLGameScreenScript.as`
- Membre (près de la ligne 124-127) : `UHOLAgePanelsPanel AgePanels;`
- `OnInitialized`, après `PlayerTable.Build(this, GetPlayersPanel());` (ligne 262 ; avant `RefreshNow` éventuel) : `AgePanels = Cast<UHOLAgePanelsPanel>(NewObject(this, UHOLAgePanelsPanel)); AgePanels.Build(this);`
- `Refresh()` (1369-1390), après `PlayerTable.Refresh()` : `if (AgePanels != nullptr) AgePanels.Refresh();`
- `ClickWidget` (2202-2240), à côté de `TopBar.ClickName` (2239) : `if (AgePanels != nullptr && AgePanels.ClickName(WidgetName)) return true;`
- Réactivité : `Refresh` du script tourne 4 fois par seconde ; le C++ changeait l'âge choisi dans la frame. Garder côté C++ le test de 622 mais l'appeler `ScriptScreen->RefreshNow()` au lieu de `RefreshAges` (idem 562 pour le palier), ou accepter 0,25 s de retard (à décider à l'essai).

## (c) C++ à retirer, ce qui reste
Retirer de `HOLGameScreen.cpp/.h` : la boucle de construction 3207-3254 (remplacée par l'hôte `AgePanelsHost`), 444-462 (recherche des widgets d'âge ; `AgeCells`, `AgeCellBoxes`, `AgePanels`, `AgeMedallions`, `ShownAgeLooks`), `RefreshAges` 2237-2353 et ses 3 appels, `HandleAgeClicked` 2458-2468, le bloc de `ClickWidget` 2648-2662, `EMedallionLook`, `AgeMedallionName`, `AgeCellName`, `MedallionSize`, `KeyBadgeSize` (`MedallionRise` reste pour `MedallionRoom`), le test de 2571-2577 (voir (d), la ligne `Medallion` du rapport) à recoder sur le widget `AgeMedallion0` trouvé par nom.
Reste en C++ : `Describe` (2686-2711, 2794-2806, 2822-2840) qui lit les miroirs posés par `SetAgesReport` (et `DescribeContent` / `DescribeTooltip` des cellules, trouvées par nom dans l'arbre : `FindWidget(AgeCell<age><cell>)` à chaque `Describe`, au lieu de `AgeCells`) ; `GridKeys`, `AgeNumerals` tant que d'autres lignes les lisent ; `ShownTier` si le palier est lu ailleurs ; `ClickAge` du contrôleur ; `UHOLGridCell` entier ; la règle `HOLCommandGrid` (IsAgeReached, GetAgeBuilding, GetAgePanelOpacity, ClickAge) lue par le script, une seule source.
Test d'accord : aucun (pas de jumeau ; le test script passe par `HOLCommandGridRead`).

## (d) Table outil -> lu -> où le script l'écrit (grep de `Tools\` de ce worktree)
| Outil (fichier:ligne) | Texte ou widget lu | Où le script l'écrit |
|---|---|---|
| `Tools\Tests\quick_check.ps1:1672`, `:1675` | clic `hol.HUD.Click AgeCell001`, attente `HUD message: NotEnoughWood` | `UHOLAgePanelsPanel.ClickName` (nom du widget `AgeCell001` = `HOLAgePanels::CellNameText(0,1)`) puis `Screen.ClickAge` ; le message vient du contrôleur (H-2) |
| `Tools\Tests\quick_check.ps1:4254` | ligne `HUD report: ... grid ages I=open [A=House greyed Z=LumberCamp greyed E=MiningCamp greyed R=Mi...` | C++ `Describe` à partir des miroirs de `SetAgesReport` ; la forme est aussi produite par `HOLAgePanels::GridLine` (même mots : `open`/`locked`, `enabled`/`greyed`/`disabled`, `(chosen I)`) |
| `Tools\Tests\quick_check.ps1:4259` | `tooltips: ...` (cellules de la grille, pas des âges) | non touché |
| `Tools\Scenarios\tiers.txt` (notes 1 à 4) | médaillons II/III s'ouvrent au niveau 3 / 6 ; aucun mot de journal ; l'oeil lit les captures | `UHOLAgePanelsPanel.Refresh` (Look, opacité) |
| `Tools\Scenarios\tech_grid.txt` | pas de panneau d'âge (Ferme sélectionnée) | non touché |
| journal `HUD: age <I..IV> <touche> clicked` (`HOLGameScreen.cpp:2465`, niveau Display) | aucun outil ne la lit dans `Tools\` (grep `HUD: age` : 0) | `UHOLGameMenuScreen::MenuLog(HOLAgePanels::ClickLine(Age,Cell))` mot pour mot, niveau Display (leçon 1) |
| `DescribeRects` / `hud_rects*` (`HOLGameScreen.cpp:2571-2582`, ligne `Medallion = top N clip N cut N`) | `AgeMedallion0`, `AgePanel0` par leur nom, `BottomScale` | noms identiques ; à recoder par nom après retrait de `AgeMedallions`/`AgePanels` |
`hol.HUD.Click` et `hol.Mouse.Hover` trouvent les widgets de l'arbre du script par leur nom (leçon 5) : les noms sont `AgePanel<a>`, `AgeMedallionRow<a>`, `AgeMedallion<a>`, `AgeNumeral<a>`, `AgeKey<a>`, `AgeCell<a><cc>`.

## (e) Suspects de compilation (non vérifiés, aucun build)
- Concaténations : j'ai utilisé des f-strings (`f"AgeCell{Age}{Digits}"`, `f"{ShownTypes[i]}"` d'un `FName`, `f"{Building.bGreyed}"`) : si `FName` ne se formate pas, passer par `ToString()`.
- `FName(f"...")`, `NAME_None`, `n"AgePanel3"` : comme HOLTopBar.as (`FName(Name + "Marker")`).
- `Screen.ConstructWidget(UHOLGridCell, ...)` : `UHOLGridCell` doit être un type construisible (leçon 4/7).
- `Cell.OnCellClicked.AddUFunction(this, n"OnCellClicked")` : le gestionnaire est `UFUNCTION() void OnCellClicked(UHOLGridCell Cell)`, nom ET signature à garder alignés avec le delegate ; sinon l'erreur dit « function not found ».
- `Box.Slot` (propriété `Slot` d'un `UWidget`) et `Cast<UUniformGridSlot>` ; `GridSlot.SetColumn/SetRow` ; `Panel.GetVisibility()`; `FMargin(a,b,c,d)`; `Medallion.SetToolTipText(FText)`.
- `UVerticalBoxSlot`/`UHorizontalBoxSlot` : `SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center)` comme HOLTopBar.as.
- `Screen.GetEconomyNow(Stock, A, B, G, I)` : appelée avec des `int` locaux (ref `int32&` : à vérifier, la leçon 9 parle de `float32` pour `float&`).
- `UHOLTextStyleHUDHeading`, `UHOLTextStyleHUDSmall` : classes de style utilisées par le C++ ; supposées exposées comme `UHOLTextStyleHUDLabel` / `UHOLTextStyleHUD` le sont dans HOLTopBar.as.
- `int(Cells[i].GetState())` : conversion d'un enum exposé.
- Le dimensionnement : `CellSize` 76, `CellGap` 2, `BarGap` 6 recopiés de HOLGameScreen.cpp:247-263 ; si `MedallionRoom` change la mise en page, c'est dans (c).
- Test `Test_AgePanelsLookThroughTheGridRule` : suppose qu'un `FHOLGridContextView` aux champs par défaut (palier 0) a l'âge I ouvert et II à IV fermés (`IsAgeReached`), vérifié dans HOLCommandGrid_Test.as seulement par ses vecteurs d'âges ; à confirmer au premier run.
- Les deux fichiers sont en UTF-8 sans BOM comme HOLTopBar.as et contiennent « — » et « é » : si le plugin les lit mal, le test d'en-tête rougit (c'est le rouge 1 de (f) à l'envers).

## (f) Deux rouges à poser une fois (dans le worktree, à remettre)
1. `HOLAgePanels::HeaderName` : remplacer `" — "` du modèle ou changer `NSLOCTEXT("HOLGameScreen", "AgeHeaderName", "Palier {0} — {1}")` en `"Palier {0} - {1}"` : `Test_AgePanelsNumeralsKeysAndHeader` doit passer rouge (valeur 1 : `Palier II — Essor`).
2. `HOLAgePanels::UsedColumns` : supprimer la ligne `if (!Used[1] && !Used[2] && !Used[3]) Used[0] = true;` : `Test_AgePanelsColumnsArePackedAndFirstKept` doit passer rouge (la colonne d'un âge vide). Puis remettre.
Un troisième, côté vue (le test ne le voit pas, leçon 3) : supprimer l'appel `Panel.SetVisibility(Wanted)` de `Refresh` : aucun test ne rougit, la capture `tiers.txt` doit montrer l'absence des 4 panneaux : c'est la preuve que la vue est vue.

## (g) Scénario de preuve
`tiers.txt` + `tech_grid.txt` ne suffisent pas à eux seuls : `tiers.txt` joue palier 0, 1, 2 (médaillons qui s'ouvrent), mais sans capture dédiée ni état fixé (ressources, Paysans immobiles), et rien ne clique un bâtiment d'âge ; `tech_grid.txt` n'a pas de panneau d'âge. Jouer `tiers.txt` AVANT (main 20a5e2f, panneau C++) et APRÈS (script), captures découpées sur la barre du bas à droite du panneau de commandes, comparées à l'oeil. Recommandé : un `ecran_hud_ages` (`.txt`, `.look`, `.json` avec `"mapSize": "Small"` ; modèle `ecran_hud_joueurs.json` ; `hol.Test.Sanity`) :
1. `waitlog Starting hero of slot 0 spawned at`, `hol.Cheat.Resources 5000`, Paysans immobiles, `hol.Camera.FocusNear`, sélection d'un Paysan (`hol.Select.Category` Economic) -> capture 1 : 4 médaillons I-IV, I ouvert (3 bâtiments A Z E), II à IV grisés avec cadenas ; objet attendu nommé dans le `.look`.
2. `hol.HUD.Click AgeCell000` + `waitlog HUD: age I A clicked` -> capture 2 : le médaillon I allumé (âge choisi), cadre accentué.
3. `hol.Hero.SetXP 0 600` + `waitlog Hero level up: slot 0 level 1 -> 3` -> capture 3 : II s'ouvre, panneau vide (médaillon seul).
4. `hol.Cheat.Resources 0` + `hol.HUD.Click AgeCell001` + `waitlog HUD message: NotEnoughWood` -> `hol.HUD.Report` : la ligne `grid ages I=open [A=House ... greyed ...]` (voir quick_check.ps1:4254).
5. Survol d'un médaillon (`hol.Mouse.Hover AgeMedallion1`) -> infobulle `Palier II — Essor` puis `Palier non atteint` quand fermé.
Pour la comparaison avant/après, jouer le même scénario sur main (sans le crochet) et sur la branche d'intégration.

## Champs d'agent_rules
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 8 (Test_AgePanelsNumeralsKeysAndHeader, Test_AgePanelsTooltipSaysLockedUnderTheHeader, Test_AgePanelsLookFollowsReachedAndChosen, Test_AgePanelsLookThroughTheGridRule, Test_AgePanelsColumnsArePackedAndFirstKept, Test_AgePanelsCellStateWordsAndReportLine, Test_AgePanelsClickJournalAndGate, Test_AgePanelsWidgetNamesAndReportParts) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: environ 120 de C++ (RefreshAges, HandleAgeClicked, construction) réécrites, non retirées ; TYPES_A_EXPOSER: UHOLGridCell (SetKey, SetCommand, ClearUnit, GetState, OnCellClicked), EHOLGridCellState, UHOLScriptScreen (GetAgePanelsHost, GetAgesContext, GetChosenAge, GetTier, ClickAge, FillAgeCell, SetAgesReport, SetRoundedBrush, GetStyleColor) ; TEST_ROUGE: non vu (aucun build) ; TESTS_TAUTOLOGIQUES: 0 (aucun test ne recopie une constante de mise en page ; un test passe par `HOLCommandGridRead::IsAgeReached`) ; NON VERIFIE : compilation des deux fichiers, vue, panneaux à l'écran.
