# GALADRIEL-1119b : HUD H-8 (panneaux d'ages), commit 1 LIAISON seulement

Branche galadriel-tab (worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel). Arret : 25 appels atteints a 01:47 (le temps n'etait pas le facteur). AUCUN build, aucun quick_check.

## Commits
- 9060759 liaison (6 fichiers, +236 lignes, non compile)
- fixup : `class UHOLGridCell;` ajoute dans HOLGridCell.h (delegate OnCellClicked declare avant la classe)

## Fait (commit 1)
- HOLGridCell.h/.cpp : `UENUM(BlueprintType) EHOLGridCellState`, `FHOLGridCellClickedEvent`, `OnCellClicked` (BlueprintAssignable), UFUNCTION sur SetKey / SetCommand / ClearUnit / GetState ; `BroadcastCellClicked()` lie a `OnClicked()` du bouton dans NativeOnInitialized.
- HOLScriptScreen.h/.cpp : GetAgePanelsHost, GetAgesContext (lit exactement 2245-2250 : GetOwnSelection / HasBuildingGroup / GetUnitsContext, converti par `HOLScriptScreenAgesPrivate::ToView`, l'inverse de ToRule), GetChosenAge, GetTier, ClickAge, FillAgeCell, SetAgesReport, SetRoundedBrush (membre const, pas static : le .as l'appelle `Screen.SetRoundedBrush`), GetStyleColor (static, 0 Accent 1 Text 2 TextMuted 3 HUDPanel).
- HOLGameScreen.h/.cpp : `AgePanelsHost` (UHorizontalBox vide ajoute en fin de boucle d'ages via `AddToBar(AgePanelsHost, 0.f)` : aucun changement visible), `GetAgePanelsHost`, `GetBoundTier`, `FillAgeCell(Cell, TypeName, bGreyed)` (corps de 2342-2350 ; FillAgeCell du script prend `FHOLGridCommandView` et appelle celui-ci), `SetAgesMirror` (recopie dans bAgesShown, ShownChosenAge, ShownAgeTypes, ShownAgeReached, ShownAgeTooltips).
- HOLAgePanels.as : AUCUNE correction necessaire d'apres les signatures ecrites (non verifie par compilation).

## RESTE (non fait)
2. BASCULE : crochet dans Script\UI\HOLGameScreenScript.as (membre ~124-127, Build apres `PlayerTable.Build(this, GetPlayersPanel())` ~262, Refresh apres `PlayerTable.Refresh()`, ClickWidget a cote de `TopBar.ClickName`) ET, meme commit : HOLGameScreen.cpp retire la boucle de construction 3207-3254 (AgePanelsHost prend sa place, a ajouter AVANT la boucle supprimee pour garder l'ordre dans la barre, `AddToBar(Host, 0.f)` ; BarGap du premier panneau a poser cote script), 444-462, RefreshAges + ses 3 appels (565 palier, 624 ou 622 age choisi -> `ScriptScreen->RefreshNow()`, 1967), HandleAgeClicked, bloc ClickWidget 2648-2662 (doit passer par le script) ; Describe 2689-2711 / 2796-2806 / 2823-2840 lisent les miroirs de SetAgesMirror + les cellules trouvees par nom (`FindWidget(AgeCell<age><cell>)`) au lieu de `AgeCells` ; DescribeRects 2570-2571 : AgeMedallion0 / AgePanel0 par nom. Decision (b) : GetChosenAge change = RefreshNow du script a l'image du clic.
3. RETRAIT : EMedallionLook, AgeMedallionName, AgeCellName, MedallionSize, KeyBadgeSize, AgePanels / AgeMedallions / AgeCells / AgeCellBoxes / ShownAgeLooks, apres grep de chaque symbole. (MedallionRise reste pour MedallionRoom.)
4. Scenario ecran_hud_ages (section (g) de GALADRIEL-1119a.md).

## Suspects de compilation (commit 1)
- `FHOLBuildable`, `Rule.Buildable`, `Rule.HeroCooldownSeconds[Ability]` (tableau indexable >= 3), `ReachedAge` / `Researches` etc. : types supposes identiques entre FHOLGridContext et FHOLGridContextView (copie inverse de ToRule).
- `OnClicked().AddUObject(this, &UHOLGridCell::BroadcastCellClicked)` : signature void() du delegate natif de CommonButtonBase (la C++ screen y lie `HandleAgeClicked(Age, Cell)` par AddUObject avec 2 charges, donc void()).
- `UENUM(BlueprintType)` sur `enum class : uint8` place sous un commentaire /** */ ; EHOLGridCellState n'a pas de UMETA : OK pour UHT.
- `PlayerController->GetChosenAge()` const et `ClickAge(int32,int32)` non const : signatures prises de la recette, non lues.
- `GetOwnSelection()` / `HasBuildingGroup()` appeles sur `const AHOLPlayerController*` (comme 2247).
- L'enum de `UHOLGridCell` dans le .as : `int(Cells[i].GetState())`.
- `HOLGameScreen.cpp` : `GetMissing`, `GetBuildingEffect`, `LOCTEXT` (namespace local `UE::HeroesOfLiberty::UI::GameScreen`, `LOCTEXT_NAMESPACE` actif en fin de fichier) utilises dans FillAgeCell.
- Include `Player/HOLCommandGridReadLibrary.h` dans HOLScriptScreen.h (FHOLGridContextView par reference/valeur d'une UFUNCTION).

## Table outils -> lignes
Inchangee par ce commit (aucun texte de journal ni nom de widget deplace). Voir la table (d) de GALADRIEL-1119a.md pour la bascule.

## Commandes de validation (apres build par la session principale)
`powershell -File Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree, puis `Tools\Tests\quick_check.ps1 -Parts ...` (tests HOLAgePanels_Test.as, 8 tests ; rouges (f) de GALADRIEL-1119a.md).

## Champs
BUILDS_DEMANDES: 1 (la session principale) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 ; TYPES_A_EXPOSER: ceux du commit 1 ; TEST_ROUGE: non vu ; EDITS_REFUSES: 0 ; NON VERIFIE : compilation C++ et script, vue ; merge main non fait, DLL perimee.
