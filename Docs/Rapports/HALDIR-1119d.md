# HALDIR-1119d : HUD H-6, barre des groupes de contrôle (2026-11-19, étape 4)

Impact joueur : aucun changement visible attendu ; la barre des puces « 1 · 9 » (bas gauche, au-dessus du bloc de sélection) passe du C++ au script, mêmes places, mêmes noms, mêmes lignes de journal. Les groupes eux-mêmes (création, rappel, touches) restent dans le contrôleur C++.
NON COMPILÉ, NON LANCÉ, SANS CROCHET : commit `b210416` sur `maps` (worktree `Documents\HeroesOfLiberty-Haldir`), 2 fichiers neufs dans `Script\UI\`. `HOLGroupBar.as` appelle 8 liaisons qui n'existent pas encore : **ne pas compiler le module script avec ce fichier avant le lot C++ (a)** (le test non plus : même module). Écrit en tête des deux fichiers.

## Fait
- `Script\UI\HOLGroupBar.as` : `namespace HOLGroupBarText` (NSLOCTEXT "HOLGameScreen", mêmes clés que le LOCTEXT C++ : `ControlGroupSlot`, `ControlGroupTooltip`, `UnitCount`), `namespace HOLGroupBar` (règles pures : `ShownText`, `ChipChanged`, `GroupAt`, `SlotName`/`BoxName`, `ChipLabel`, `TypeOrder` (tri stable, plus nombreux d'abord), `TooltipText`, `LinePart`, `BarLine`, `BarLogLine`, `ClickLine`, `LitGroup`, `ItemScale`, `ItemOpacity`, `ClickAccepted`, constantes 0.5 s / 1.15 / 0.7 / 0.25 s / 64 / 28.8 / 4), `class UHOLGroupBarChip : UObject` (un par puce : porte le numéro de groupe vers `OnButtonBaseClicked`), `class UHOLGroupBarPanel : UObject` (la vue : `Build(Screen, GroupsPanel)`, `Refresh()`, `ClickGroup`, `ClickName`).
- `Script\UI\HOLGroupBar_Test.as` : 10 tests purs (liste en fin de rapport). Valeurs recopiées du C++ et de ce que lit `quick_check.ps1` (« 1=9 », « 1=3 2=n », « HUD: group 2 clicked », `ControlGroup2`).
- Choix de vue : comme H-4 (`PlayersPanel`), le `GroupsPanel` RESTE créé en C++ dans `BuildLayout` (colonne `BottomLeft`, `MakePanel`, marge 6/4, écart `BarGap` 6 px, réduit au départ, dans la `BottomScale`) ; le script y met `GroupsRow`, les 10 `ControlGroupBox<N>` (SizeBox 64 x 28,8) et les 10 `ControlGroup<N>` (`UHOLQueueItem`, pivot 0,5/0,5), dans l'ordre des touches 1..9 puis 0, `AddToRow` identique (centré, marge droite 4). Le style de panneau n'est donc écrit qu'une fois.

## (a) Liaison C++ à ÉCRIRE (un lot, un build) : membres de `UHOLScriptScreen` (`Source\HeroesOfLibertyUI\HUD\HOLScriptScreen.h/.cpp`, même motif que `GetPlayersPanel` : `GetTypedOuter<UHOLGameScreen>()` puis le contrôleur `GameScreen->Controller` / `GetOwningPlayer()`), `UFUNCTION(BlueprintCallable|BlueprintPure, Category = "HeroesOfLiberty|UI")`
```cpp
/** UHOLGameScreen::GroupsPanel (a getter inline sur UHOLGameScreen, comme GetPlayersPanel) : déjà dans sa colonne, sa marge, réduit. */
UBorder* GetGroupsPanel() const;
/** AHOLPlayerController::NumControlGroups (10). */
UFUNCTION(BlueprintPure) static int32 GetNumControlGroups();
/** Le contrôleur ET le monde existent (le C++ sortait de RefreshControlGroups sans rien faire avant eux). */
bool GetControlGroupsAvailable() const;
/** PlayerController->GetControlGroup(Group, Units, Buildings) : retourne bFull. OutCount = Units.IsEmpty() ? Buildings.Num() : Units.Num() ; bOutBuildings = Units.IsEmpty(). Faux et 0 sans contrôleur / groupe hors bornes. */
bool GetControlGroupNow(int32 Group, int32& OutCount, bool& bOutBuildings) const;
/** Les types du groupe, dans l'ordre où ils sont rencontrés (PAS triés : le tri est une règle du script), noms DÉJÀ localisés : unités = HOLUnitView::Gather + Units.Contains(View.UnitId) puis HOLGameNames::GetUnitName(Type, FindUnitType(Type), LocalFactionKey()) ; bâtiments = HOLGameNames::GetBuildingName(Type, UHOLBuildingSettings::FindType(Type)) ; un compte par type (FName fusionné). Appelée seulement quand la puce a changé (comme le C++). */
void GetControlGroupTypes(int32 Group, TArray<FString>& OutNames, TArray<int32>& OutCounts) const;
/** PlayerController->GetLastSelectedGroup(Time) : le groupe, INDEX_NONE si aucun. */
int32 GetLastSelectedGroup() const;
/** Secondes depuis cette sélection : World->GetRealTimeSeconds() - Time (GROS, 1e9, sans sélection ni contrôleur : jamais allumé). */
float GetLastSelectedGroupAge() const;
/** AHOLPlayerController::SelectControlGroup(Group) (HandleGroupSlotClicked moins sa ligne de journal, que le script écrit). */
void SelectControlGroup(int32 Group);
```
Plus `UHOLGameScreen` : `UBorder* GetGroupsPanel() const { return GroupsPanel; }` et `UHOLGameScreen::GetGroupTypes(Group, Names, Counts)` si l'on veut garder LocalFactionKey / FindUnitType privés (probablement : ils sont dans l'espace anonyme du `.cpp`). `UHOLQueueItem::SetLabel` est déjà une UFUNCTION (corrigé 1b30d3e, H-3). `SetRenderScale/SetRenderOpacity/SetRenderTransformPivot/SetToolTipText` sont des UFUNCTION d'UWidget.
Appelées par le .as : `GetGroupsPanel` (`Build`), `GetNumControlGroups` (`Build`), `GetControlGroupsAvailable`, `GetLastSelectedGroup`, `GetLastSelectedGroupAge`, `GetControlGroupNow`, `GetControlGroupTypes` (`Refresh`), `SelectControlGroup` (`ClickGroup`).

## (b) Crochet dans `Script\UI\HOLGameScreenScript.as` (version d'intégration `Documents\HeroesOfLiberty-Arwen`, numéros à re-vérifier après fusion de H-5)
1. Membre après `UHOLPlayerTablePanel PlayerTable;` (ligne 124) : `UHOLGroupBarPanel GroupBar;`.
2. `OnInitialized()` : après `PlayerTable.Build(this, GetPlayersPanel());` (ligne 262), avant `BuildMinimap(Frame);` : `GroupBar = Cast<UHOLGroupBarPanel>(NewObject(this, UHOLGroupBarPanel)); GroupBar.Build(this, GetGroupsPanel());`.
3. `Refresh()` : après `if (PlayerTable != nullptr) PlayerTable.Refresh();` (ligne 1383) : `if (GroupBar != nullptr) GroupBar.Refresh();`.
4. `ClickWidget` (vers 2239) : avant `return false;` final, après `TopBar.ClickName` : `if (GroupBar != nullptr && GroupBar.ClickName(WidgetName)) return true;`. NB : le `ClickWidget` du script est appelé AVANT celui du C++ ; une puce vide rend false, et le C++ n'a plus de branche (voir (c)) : résultat faux, comme avant.
5. `Describe()` : rien à ajouter (le C++ n'écrivait aucune ligne de groupes dans `hol.HUD.Report`).
Cadence : `Refresh()` du script tourne 4 fois par seconde = `GroupRefreshInterval` (0,25 s) : la liste est donc à la même cadence. DIFFÉRENCE : l'éclairage du groupe choisi (0,5 s) était testé à CHAQUE image par le C++ ; en script il l'est tous les 0,25 s (allumé 0 à 0,25 s plus tard, éteint 0 à 0,25 s plus tard). À regarder à l'essai ; si gênant, appeler `GroupBar.Refresh()` depuis un crochet par image (pas dans ce lot).

## (c) C++ à retirer de `HUD\HOLGameScreen.cpp/.h` (même commit que le crochet)
Retirer : `RefreshControlGroups` (cpp ~1978-2085) et son appel (cpp 652), `HandleGroupSlotClicked` (cpp ~2086-2093) et `GroupSlotName` (cpp 1973), la boucle de liaison des puces en `OnInitialized` (cpp 441-448) et `ShownGroups.Init(...)` (cpp 461), la branche `GroupSlots` de `ClickWidget` (cpp 2866-2877), dans `BuildLayout` le seul bloc `GroupsRow` + boucle des 10 `ControlGroupBox` (cpp 3348-3361 ; GARDER 3342-3347 : `GroupsPanel`, son padding, `Collapsed`, son slot), les constantes `GroupRefreshInterval`, `GroupSlotWidth`, `GroupLightSeconds` (cpp 283, 289, 290 ; `GroupRefreshInterval` sert encore à cpp 562 et 621/637 : vérifier avant de la retirer), et au `.h` : `RefreshControlGroups`, `HandleGroupSlotClicked`, `GroupSlotName`, `GroupSlots`, `ShownGroups`, `ShownGroupsLine`, `SinceGroupsRefresh`, `LitGroup`, `LitGroupTime`.
RESTE en C++ : `GroupsPanel` (membre, son `MakePanel` et son slot dans `BottomLeft`, comme `PlayersPanel`), `AHOLPlayerController` (groupes, touches, `SelectControlGroup`, `GetControlGroup`, `GetLastSelectedGroup`), la décision `ClickWidget` du C++ pour tout le reste, `Describe`, et les noms (`LocalFactionKey`, `FindUnitType`, `HOLGameNames`) derrière `GetControlGroupTypes` (une source, pas de jumeau). Les traductions : les clés `ControlGroupSlot`, `ControlGroupTooltip`, `UnitCount` de `HOLGameScreen` (Tools\Localization\fill_po.py:200-201) sont conservées par le NSLOCTEXT script (mêmes espace de noms et clés) ; seul le motif `{0}` / `{1}` est substitué à la main (`Replace`), donc les langues qui réordonnent les champs sont suivies (le motif, pas l'ordre des arguments).

## (d) Outils : texte lu -> où le script l'écrit (grep de `Tools\`, `Source\`, `Script\`)
| Outil (fichier:ligne) | Texte ou widget lu | Où le script l'écrit |
|---|---|---|
| `Tools\Tests\quick_check.ps1:2478` (`waitlog HUD groups: 1=9`) | `HUD groups: 1=9` | `HOLGroupBar::BarLogLine` via `UHOLGameMenuScreen::MenuLog` dans `UHOLGroupBarPanel.Refresh` |
| `quick_check.ps1:2481` (`waitlog HUD groups: 1=3 2=`) et `5681-5684` (`HUD groups: (.*)$`, valeurs `1=9`, `1=3 2=$Soldiers`, `1=$(3 + $Soldiers) 2=$Soldiers`) | idem, ordre des touches (1, 2, ... 0 en dernier), « n » pour les unités, « bN » pour les bâtiments | `HOLGroupBar::LinePart` + `BarLine` (testés) |
| `quick_check.ps1:2485` (`hol.HUD.Click ControlGroup2`) puis `waitlog Group 2: selected` | widget `ControlGroup2`, ligne `Group 2: selected` | `ClickName` -> `ClickGroup` -> `Screen.SelectControlGroup` (la ligne `Group 2: selected` reste écrite par le contrôleur C++) |
| `quick_check.ps1:5685` | `HUD: group 2 clicked` | `HOLGroupBar::ClickLine` via `MenuLog` dans `ClickGroup` |
| `quick_check.ps1:5686` | capture `groups_bar.png` | inchangé (capture du scénario) |
| `Tools\Localization\fill_po.py:200-201` | clés `HOLGameScreen,ControlGroupSlot`, `ControlGroupTooltip` | NSLOCTEXT de `HOLGroupBarText` (mêmes clés) ; `UnitCount` : clé existante déjà |
| `Tools\Scenarios\*.txt/*.look/*.json`, `hud_rects*` | aucune occurrence de `ControlGroup`, `GroupsPanel`, `HUD groups` | rien à changer |
Les lignes passent par `MenuLog` pour garder le `Display:` (leçon 1 : `Log(n"LogHOLUI", ...)` casse la lecture par `waitlog` / `lobby_match.ps1`).

## (e) Suspects de compilation
- `TArray<FString>&` / `TArray<int>&` en paramètre de sortie des liaisons (`GetControlGroupTypes`) : le plugin accepte les sorties non const par `&out` ; si refus, renvoyer un `FHOLControlGroupTypesRead` (struct avec deux tableaux) comme `FHOLPlayersTableRowRead`. Même doute pour `int32&` et `bool&` de `GetControlGroupNow` (déjà utilisés par `GetPopulationNow`).
- `TArray<int>.Insert(Value, Index)` (dans `TypeOrder`) : si l'ordre des arguments est refusé, écrire `Order.Insert(Index, At)` à l'envers ou remplacer par un tri par échange.
- `Boxes.Add(nullptr)` dans un `TArray<USizeBox>` et `Boxes[Group] = Box` (tableau préremplement indexé par numéro de groupe, pas par position) ; `Handlers` : `TArray<UHOLGroupBarChip>` de références fortes (sinon ramassés : `NewObject(this, ...)` leur donne la vue pour propriétaire).
- `Chip.OnButtonBaseClicked.AddUFunction(Handler, n"Clicked")` : l'objet cible n'est pas `this` (classe `UHOLGroupBarChip`) ; `UFUNCTION() void Clicked(UCommonButtonBase Button)`.
- `FName(f"ControlGroup{Group}")` ; `FText::FromString` ; `NSLOCTEXT` dans des fonctions de namespace (comme H-3, qui compile) ; `T.AssertEquals(FName, FName)` dans le test (sinon comparer `.ToString()`) ; `T.AssertEquals(double, double)` pour 1.15 / 0.7 (si l'égalité flottante est refusée : tolérance).
- Les noms de classes `UHOLGroupBarChip` / `UHOLGroupBarPanel` ne heurtent aucune struct `FHOLGroupBar...` (leçon 10).
- Une fonction globale de test partagée : aucune ici (tout passe par `HOLGroupBar::`).

## (f) Deux rouges à poser UNE fois (après compilation du lot)
1. Dans `HOLGroupBar_Test.as`, `Test_GroupBarJournalLinesAreTheCppOnesWordForWord` : attendre `"HUD groups : 1=9"` (espace de trop) : le test doit sortir ROUGE, puis remettre. Prouve que la ligne lue par `waitlog` est vraiment comparée.
2. Dans `HOLGroupBar.as`, `GroupAt` : remplacer `(Position + 1) % Count` par `Position % Count` : `Test_GroupBarOrderIsTheKeysOneToNineThenZero` ROUGE (la puce 0 viendrait en premier), puis remettre. Bouchon dans le worktree, jamais dans la copie principale.

## (g) Scénario de preuve
`group_march.txt` et `j7_army.txt` ne SUFFISENT PAS : aucun n'appuie une touche de groupe ni ne clique une puce (grep : rien). Le seul parcours des groupes est `quick_check.ps1:2470-2490` (parties « groups », qui vérifie le journal, pas l'œil, et pas la vue : la leçon 3 du bouton Menu sans libellé s'applique, une puce sans « 1 · 9 » passerait). Il faut un `ecran_hud_groupes` (modèle `ecran_hud_joueurs.*`) :
- `.json` : `{ "map": "/Game/Maps/Plain.Plain", "mapSize": "Small", "slots": [ { "state": "Human", "colorIndex": 0 }, { "state": "AI", "colorIndex": 1 } ] }`.
- `.txt` (lignes) : `waitlog Camera above the start of slot 0`, `r.SetRes 1920x1080w`, `hol.Cheat.Resources 5000`, `hol.Cheat.Population 165`, `hol.Test.Sanity`, `hol.Camera.FocusSlot own`, `frames 90`, `hol.HUD.Report 1`, `capture hud_groupes_aucun` (barre ABSENTE : aucun groupe) ; `hol.Select.Army` (ou `hol.Cheat.Spawn` de 3 Hommes d'armes + 2 Archers, Paysans immobiles) ; `waitkey Entree : Maj + 1 crée le groupe 1`, `hol.UI.Key LeftShift 0 0.5`, `hol.UI.Key Ampersand 0.2 0.1`, `waitlog HUD groups: 1=5`, `frames 10`, `capture hud_groupes_un` ; un second groupe `hol.Select.Group Add 2` / `Select 2`, `waitlog HUD groups: 1=5 2=`, `capture hud_groupes_deux` ; `hol.Mouse.Hover` sur `ControlGroup1`, `capture hud_groupes_infobulle` ; `hol.HUD.Click ControlGroup2`, `waitlog HUD: group 2 clicked`, `waitlog Group 2: selected`, `frames 6`, `capture hud_groupes_clic` (puce 2 allumée, grossie à 1,15, l'autre à 70 %) ; `waitlog` du retour de l'éclairage, `capture hud_groupes_eteint`.
- `.look` : une ligne par capture, l'OBJET nommé : « bas gauche, au-dessus du bloc de sélection, un cadre sombre avec une puce « 1 · 5 » ; l'infobulle dit Groupe 1, Homme d'armes × 3, Archer × 2 ; la puce cliquée est plus grande, l'autre plus pâle ».
- Preuve = le même scénario joué AVANT (C++, branche sans le lot) et APRÈS (script), captures découpées sur la zone du panneau (bas gauche) et comparées à l'œil ; `preflight.ps1 ecran_hud_groupes` avant tout `play.ps1 ... -Hidden -Root <worktree>`. Les parties « groups » de `quick_check` restent le garde-fou du journal.

## Tests écrits (10, dans `HOLGroupBar_Test.as`)
`Test_GroupBarShownTextIsEmptyUnitsCountOrBuildingsB`, `Test_GroupBarChipIsTouchedOnlyWhenItsGroupChanged`, `Test_GroupBarOrderIsTheKeysOneToNineThenZero`, `Test_GroupBarChipLabelIsNumberDotCount`, `Test_GroupBarTypesAreSortedMostFirstTiesKeepTheMeetingOrder`, `Test_GroupBarTooltipIsGroupNHeadThenOneLineAType`, `Test_GroupBarReportLineIsTheOneQuickCheckReads`, `Test_GroupBarJournalLinesAreTheCppOnesWordForWord`, `Test_GroupBarLitGroupFadesAfterHalfASecond`, `Test_GroupBarClickIsRefusedOnAnEmptyChip`.

## Doutes notés (aucune question de design)
- Éclairage testé à 0,25 s au lieu de chaque image (voir (b)).
- Le texte du tooltip et de la puce n'est pas passé par `FText::Format` mais par `Replace` sur le motif NSLOCTEXT ; le chiffre n'est plus formaté par la culture (sans effet : 1 à quelques centaines).
- `GetControlGroupsAvailable` ajoutée pour garder le comportement « rien avant le contrôleur » (sinon la ligne `HUD groups: none` sortirait trop tôt).

BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 10 (liste ci-dessus) ; LIGNES_CPP_SUPPRIMEES: 0 (au lot suivant, ~150) ; TYPES_A_EXPOSER: FHOLControlGroup... aucun (8 membres de UHOLScriptScreen) ; TEST_ROUGE: non posé (voir (f)) ; TESTS_TAUTOLOGIQUES: 0.
