# SAURON-1119q : HUD H-10, grille de commandes 3x4 (reprise de 1119m : lot C++ + bascule + scenario)

2026-11-19, worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` (branche envelope). Ni build, ni quick_check, ni editeur, ni play.ps1 : RIEN n'est compile ni joue.
Impact joueur : apres bascule la grille A Z E R / Q S D F / W X C V doit etre identique (touches, libelles, infobulles, clic, journal) ; si le script ne se charge pas, la grille est ABSENTE (comme les autres panneaux H-x).

## Commits (anglais, sans Co-Authored-By, pas de push)
1. e95cbde : lot C++ de liaison ET retrait du C++ de la grille dans HOLGameScreen (memes fichiers : un seul commit pour les etapes 1 et 3 cote cpp/h).
2. a42dcf5 : crochet dans HOLGameScreenScript.as (membre `CommandGrid`, Build apres `QueueView.Build` et avant `AgePanels`, surcharge `RefreshGridCells`, entree dans `ClickWidget`) ; en-tetes des 2 fichiers de la vue.
2b. 3308c4f : retire les lignes d'en-tete « NOT COMPILED » restees par erreur dans a42dcf5 (mon remplacement les avait dupliquees, fichiers en LF).
3. 600a88a : retire les includes `UniformGridPanel.h` / `UniformGridSlot.h` devenus morts de HOLGameScreen.cpp.
4. c07929b : scenario `Tools\Scenarios\ecran_hud_grille.txt` + `.look` + `.json` (mapSize Small, hol.Test.Sanity, waitkey avant chaque etape, ScenarioClose avant chaque capture).

ETAPE ATTEINTE : 4 sur 4 (ecrites ; rien de compile).

## Ce qui a ete fait (C++)
- `HOLScriptScreen.h/.cpp` : `FHOLGridCellInfoView` (USTRUCT BlueprintType : Kind, State, OrderType), `RefreshGridCells` (BlueprintImplementableEvent), `GetGridPanelHost`, `GetGridFrame`, `ApplyGridCell`, `ClickGridCell`, `RegisterGridCells` : signatures de la recette.
- `HOLGameScreen.h` : `struct FHOLGridCellFill` (non USTRUCT : bCommand, Name, State, TooltipName, Cost, Missing, Effect, bGain, Gain, bTrade ; MEMES NOMS de methodes que UHOLGridCell : SetCommand, SetTooltip, SetGain, ClearUnit) ; membre `GridPanelHost` (UVerticalBox, Transient) a la place de `GridPanel` ; `GridFrameFill` ; publics `GetGridPanelHost`, `SetGridCells`, `ReadGridFrame`, `ApplyGridCell`, `ClickGridCell` ; `HandleGridCellClicked` retire (devenu `ClickGridCell`, sans les 2 `UE_LOG`) ; include `Widgets/HOLGridCell.h`.
- `HOLGameScreen.cpp` : le `switch` de `RefreshGrid` est DEPLACE tel quel (decision confirmee) : `GridCell` n'est plus un widget mais un `FHOLGridCellFill*` sur `GridFrameFill[Cell]`, donc le texte du switch n'a pas change ; seul changement dedans : `ShownTradeLabels[Cell] = GridCell->DescribeContent()` devient `GridCell->bTrade = true` et se fait dans `ApplyGridCell` (cellule remplie avant lecture, risque 2 de la recette). Boucle bornee par `NumGridCells` (plus `GridCells.Num()`, vide avant `RegisterGridCells`). Fin de `RefreshGrid` : `ScriptScreen->RefreshGridCells()`. `Shown*` inchanges ; `Describe` / `DescribeCells` / `DescribeTooltips` / `DescribeRects` NON TOUCHES (mot pour mot).
- `GridKeys`, `CellSize`, `CellGap` gardes (utilises par Describe, GridWidth, BarHeight).

## LIGNES C++ retirees de HOLGameScreen.cpp
`git diff --numstat` (3 commits C++ cumules) : 69 supprimees / 62 ajoutees (ajouts = `SetGridCells`, `ReadGridFrame`, `ApplyGridCell`, appel `RefreshGridCells`, commentaires). Vraiment retire hors switch : ~62 lignes (boucle de cles / clics 10, bloc de visibilite 8, test `!GridCell` 6, 2 UE_LOG, boucle de `ClickWidget` 13, construction de GridPanel / Grid / 12 cellules + Hidden ~16, `GridColumns`, `GridCellName`, 2 includes). Le switch (~265 lignes) reste en C++, deplace. HOLGameScreen.h : 10 supprimees / 41 ajoutees.
Ecart a la recette : ~73 estimees, ~62 reelles.

## Ce qui tourne par image
Rien par image. `RefreshGridCells` -> `UHOLCommandGridPanel.Refresh` a chaque `RefreshGrid` (selection, stock, palier ; tant qu'une case heros / rage / deploy est affichee, a chaque tour de la tick ~0,25 s) : 1 `GetGridFrame` (12 petites structs), 12 `ApplyGridCell` ou `ClearUnit`. Le C++ recalculait deja le meme contenu. Non mesure.

## Ecarts a la recette
- `ReadGridFrame` (GameScreen) en plus : la struct de vue vit dans HOLScriptScreen.h, la GameScreen rend 3 tableaux d'int / FName et le ScriptScreen fabrique la struct.
- `RefreshGridCells` n'est PAS ajoute au `Refresh()` complet du script (seulement la surcharge de l'evenement) : le C++ appelle `RefreshGrid` a chaque changement. Au 1er Refresh la grille reste cachee (frame vide), comme avant.
- Clic : le C++ branchait `OnClicked()` du bouton ; le script branche `OnCellClicked` (un gestionnaire pour 12 cases), comme les paliers d'ages.
- Journal par `UHOLGameMenuScreen::MenuLog` (script `ClickLine`), niveau Display.

## QUESTIONS (rien tranche)
1. `hol.HUD.Click ScenarioClose` ferme-t-il le panneau de notes pour de bon, ou la ligne `note` suivante le rouvre-t-elle ? J'ai suppose (comme `ecran_hud_joueurs`) qu'elle le rouvre.
2. Contenu du switch : toujours la question de 1119m (migration avec les lots G1 / G2), defaut = non avant l'etape 5.

## NON VERIFIE (tout)
- Aucune compilation C++ ni script. A surveiller : `FHOLGridCellFill` (FHOLTradeGain complet via Player/HOLCommandGrid.h, EHOLGridCellState via Widgets/HOLGridCell.h inclus avant `HOLGameScreen.generated.h`) ; `class UVerticalBox;` ajoute dans HOLScriptScreen.h ; en script : `Screen.MakePanel`, `Screen.ConstructWidget`, `UVerticalBoxSlot`, `FHOLGridCellInfoView`.
- Rouge des 8 tests de `HOLCommandGridPanel_Test.as` non vu : fausser "AZERQSDFWXCV" d'une lettre dans `Test_GridPanelKeysFollowTheAzertyRows`, voir rouge, rendre.
- Forum sans script (`ScriptScreen` nul) : grille absente (accepte, comme les autres panneaux).
- Scenario non lance, preflight non fait : `hol.Mouse.Hover GridCell00` (forme sans coordonnees) et `hol.Mouse.Click GridCell00` supposes d'apres `specialization_button.txt` ; `hol.Cheat.Resources -100000` (additif borne a 0, lu dans AHOLPlayerController::DoCheatResources:4856) ; ni Marche ni heros dans ce scenario.
- Describe mot pour mot non rejoue : `grid ...`, `HUD cells:`, `HUD tooltips:` viennent du C++ inchange (Shown* + etat des cellules enregistrees).

## ANOMALIES
- a42dcf5 contenait des lignes d'en-tete « NOT COMPILED » en double (fichiers LF traites en CRLF) : corrige par 3308c4f.
- Worktree propre. `HOLControllerCommands.as` / `HOLPlayerController.cpp` (lot C1 de la branche) non touches.

## COMMANDES DE TEST (session principale)
1. `Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree (1 build : HOLScriptScreen, HOLGameScreen + le lot C1 deja la).
2. `quick_check -Parts <HUD, Script>` : lignes `grid A=Peasant enabled`, `grid V=Cancel enabled`, `HUD cells:`, `HUD tooltips:` (quick_check.ps1:1374, 1694, 1705, 3694, 4279, 4849, 5284).
3. Scenarios avant / apres : `peasant_grid`, `tech_grid`, `siege_grid`, `tech_grid_shot`, `ecran_hud_file`, `legions_warrior_compare`, `specialization_button` ; puis `Tools\Scenarios\preflight.ps1 ecran_hud_grille` et `play.ps1 ecran_hud_grille -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`.

## Champs
BUILDS_DEMANDES: 1 (lot C++) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 (les 8 de 1119m) ; TEST_ROUGE: NON VU ; LIGNES_CPP_SUPPRIMEES: ~62 net de HOLGameScreen.cpp (69 supprimees, 62 ajoutees) ; EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: aucun lance.
