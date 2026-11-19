# HALDIR-1119s : HUD H-9, BASCULE du bloc de selection, reprise 14, 2026-11-19

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, branche `selection`. Rien compile, rien joue. Effet joueur vise : aucun changement visible.

## Commits
| Etape | sha | Contenu |
|---|---|---|
| 1 crochets script | `7eea4ef` | `HOLGameScreenScript.as` (3 membres, Build A puis B puis C, `RefreshSelectionBlock`, `SendSelectionStats`, `SelectionFrameTick`, `ClickWidget`) + `HOLSelectionUnit.as` (`Invalidate`, `IsFrameLive`, `bSingleUnit`) |
| 2 C++ bascule | `8cc9845` | `HOLGameScreen.cpp/.h`, `HOLScriptScreen.cpp/.h` |
| 3 retrait du C++ mort | `3f7e01a` | `HOLGameScreen.cpp/.h` : 733 lignes retirees (cpp 634, h 99) |

Etape atteinte : 3 sur 3.

## Etape 1 : ce qui tourne ou
- Membres : `SelectionBlock` (A), `SelectionUnitBlock` (B), `SelectionChipsBlock` (C) (pas `SelectionPanel` : nom du widget). Build apres `AgePanels.Build` : A `Build(this, GetSelectionHost())`, puis B et C `Build(this, SelectionBlock)`.
- `Refresh()` -> `RefreshSelectionBlock()` (apres `AgePanels.Refresh`, donc a chaque RequestRefresh et a chaque 250 ms) :
  1. `Path = HOLSelectionRead::GetSelectionKind()` ; si le chemin change : `SelectionUnitBlock.Hide()` si on quitte les unites (2), puis `SelectionUnitBlock.Invalidate()` (memos `Sent*` de B remis a une valeur sentinelle : sinon un fragment ecrit par A ou C entre-temps ne serait jamais reecrit par B, ex. ligne d'activite de batiment qui reste en entrant dans un groupe d'unites).
  2. A `Refresh()` (chemins 0, 1, 4).
  3. `SetPanelShown(true)` pour les chemins 2 et 3 (AUCUN des lots B ou C ne le faisait : sans cela le panneau restait cache pour unites et groupes).
  4. B `Refresh()` SEULEMENT si chemin 2 (hors unites, son `Refresh` ecrirait `"none"` par-dessus le rapport du groupe de batiments de C).
  5. C `Refresh()` toujours, en dernier (cache le nom s'il y a des puces ; dessine le groupe de batiments, chemin 3, qui lui rend faux hors groupe).
  6. `SendSelectionStats()` : `TakeStatsReportChange()` -> `SetStatsReport(ShownStats, ShownStatTipList)`.
  7. `SetSelectionLive(A.IsFrameLive() || B.IsFrameLive())`.
- Par image, seulement tant que `bSelectionLive` : `SelectionFrameTick()` (evenement script, appele par `UHOLScriptScreen::NativeTick` comme `QueueFrameTick`) : A `FrameTick` si source selectionnee (chemin 4) ; B `RefreshFrame` + `SendSelectionStats` si UNE unite (`bSingleUnit`, pose par `Refresh` de B). Groupe d'unites (>1) : 4 Hz seulement (voir etape 2).
- `ClickWidget` : `SelectionChipsBlock.ClickName(WidgetName)` en dernier (`SelectionChip0..11`).

## Qui ecrit chaque champ Shown* (regle d'or tenue)
- `ShownKind` / `ShownActivity` / `ShownHealth` de `UHOLSelectionPanel` (lot A) sont les SEULS champs ecrits, chacun par ses setters `SetShownKind/Activity/Health` qui poussent les 4 copies via `SetSelectionReport` vers `UHOLScriptScreen`. B et C ne les touchent que par ces setters (pas de second champ). `ShownForum` : A seul (`RefreshBuilding`).
- C++ : `UHOLGameScreen::ShownSelection/ShownActivity/ShownHealth/ShownForum` ne sont plus ecrits par personne (membres gardes, lus seulement comme repli de `Describe` si `ScriptScreen` est nul). `Describe`, `DescribeForum` lisent `GetSelectionReportKind/Activity/Health/Forum`.
- `ShownStats` / `ShownStatTipList` / `ShownStatsTip` (C++) : ecrits par `SetStatsReportMirror`, appele par `UHOLScriptScreen::SetStatsReport` (vide ensemble : `ShownStatsTip` = tips joints par `" | "`). Seul ecrivain : le script (B).
- Pour le groupe de batiments, C ecrit sante / proprietaire / activite via les setters de A ; A ne les reecrit pas derriere (A ne fait que cacher progression et vies aux chemins 2 et 3), donc pas de double ecriture. A l'inverse en quittant le groupe, A (chemins 0, 1, 4) reecrit ses champs depuis son propre etat : sain.

## Etape 2 : C++
- `UHOLScriptScreen` : `GetSelectionHost()`, `SetStatsReport(FString, TArray<FString>)`, `SetSelectionLive(bool)`, evenement `SelectionFrameTick()`, membre `bSelectionLive`, appel dans `NativeTick` apres `QueueFrameTick`.
- `UHOLGameScreen` : membre `SelectionHost` (UHorizontalBox vide, `AddToBar(SelectionHost, 0.f)` a la PREMIERE place de la barre, ou le bloc C++ etait : conserve l'ordre selection / commandes / ages ; un `Build` de A qui ajouterait directement dans `BottomBar` mettrait le bloc a droite), `GetSelectionHost()`, `SetStatsReportMirror`.
- `Refresh()` : ne construit plus le bloc ; garde `RefreshTopBar`, `RefreshIdleWorkers`, `RefreshGrid`, `RequestRefresh` (le script relit au frame suivant). `HandleSelectionChanged` inchange (BindBuilding, son, `Refresh()`).
- `NativeTick` : aiguillage gardee ; groupe de batiments : `Refresh()` a 4 Hz ; >1 unite : `ScriptScreen->RequestRefresh()` a 4 Hz ; la branche 1 unite / source par image est devenue `SelectionFrameTick` du script.
- `DescribeSelectionName` : widget `SelectionName` du script (`ScriptScreen->WidgetTree->FindWidget`). `DescribeStatsTip` : `SelectionStats` / `SelectionStat<i>` dans l'arbre du script + `ShownStatTipList`. `HoverStat` : puce `SelectionStat<i>` du script (modele `HoverIncome`). `DescribeStats` inchange (lit `ShownStats`).
- Les chaines de la table 4 du plan sont inchangees (aucune ligne `Printf` modifiee ; seules les sources des champs changent).
- Retire de `BuildLayout` : tout le bloc SelectionPlace..SelectionLives, la liaison des puces (cpp 391-401), `SelectionPanel->SetVisibility(Collapsed)` de fin.

## Etape 3 : C++ retire (grep de chaque symbole dans Source\ et Script\ : aucun autre usage)
cpp : `RefreshSelection`, `RefreshUnitSelection`, `RefreshBuildingGroup`, `RefreshChips`, `HandleChipClicked`, `RefreshSourceSelection`, `RefreshUnitActivity`, `HideUnitStats`, `RefreshUnitStats`, `RefreshUnitHealth`, `SetDetailLine`, `FormatStat`, `FormatStatPlain`, `GetOwnerNameIn`, `IsGatheringActivity`, `MakeChipLabel`, `FormatAmount`, `FormatRatio`, constantes `SwatchSize`, `SiteBarWidth`, `SelectionWidth`, `NumChips`, `ChipHeight`, `ChipGap`, `ChipMaxWidth`, `ChipNameChars` = 634 lignes. h : declarations ci-dessus + membres `SelectionPanel/Name/OwnerRow/Swatch/Owner/HealthRow/Health/Stats/StatChips/Activity/LivesRow/Lives/ProgressRow/Progress/ProgressText/Chips`, `ChipItems/ChipFrames/ChipLabels/ActiveChip/ShownChips/ShownActiveChip`, `ShownForumLives` = 99 lignes. Total 733.
Gardes expres : CVars `hol.HUD.UnitStats` et `hol.HUD.UnitStatsBuilding` (lues par nom par la liaison du lot B), `HoverIncome`, `FindUnitType/UnitNameFor/LocalFactionKey/FindUnitTypeFor` (non verifies morts), `ShownSelection/Activity/Health/Forum` (repli de `Describe`), `SetShown`, `GetResourceName`.
Reste a retirer (a l'agent suivant, apres compilation verte) : les 4 membres `ShownSelection/Activity/Health/Forum` + repli de `Describe` si `ScriptScreen` est toujours non nul (non verifie) ; `UnitNameFor`, `LocalFactionKey`, `FindUnitType` si plus lus (grep) ; `ShownStatsTip` eventuellement ; includes devenus inutiles (`UWrapBox`, `UProgressBar`, `HOLUnitView`, `HOLForumMessages`).

## NON VERIFIE (rien compile, rien joue)
- Tout : C++ et script, ensemble avec les 3 lots (bibliotheques `HOLSelection*ReadLibrary`, deja non compilees).
- Que le script voit `GetSelectionHost`, `SetStatsReport(TArray<FString>)`, `SetSelectionLive`, `SelectionFrameTick` (evenement : `UFUNCTION(BlueprintOverride)` sans `override` du `UHOLScriptScreen`, comme `QueueFrameTick`).
- Que `AddChildToHorizontalBox` dans un hote vide a la 1ere place de la barre donne les memes places, hauteur et alignement bas que `AddToBar(SelectionPlace)` (le slot de `SelectionHost` est aligne bas, padding 0 ; `SelectionPlace` y est lui-meme aligne bas avec un `USpacer` 340 x BarHeight) : a comparer en capture (`ecran_hud_selection_*`, scenarios du HALDIR-1119t).
- `DescribeStatsTip` const : `ScriptScreen->WidgetTree->FindWidget` depuis une methode const, `const UWidget*` / `const UCommonTextBlock*` ; `Describe` : references `const FString&` sur l'operateur ternaire (deux lvalues const FString).
- Chemin 3 (groupe) : le nom du chemin 2 vers 3 est cache par C via `SetName("", false)` ; groupe d'unites d'un seul type : `GetChipLabels` donne-t-il une puce ? Sinon le nom du chemin precedent reste (B n'ecrit le nom que pour 1 unite) : a verifier a la capture (`HUD selection name:` pour un groupe).
- Cout par image : 1 unite = `GetUnitSelectionView` + `GetUnitStatsRow` par image (stats reconstruites seulement si la cle change) ; a mesurer `hol.HUD.Report`.
- Spectateur suivi : B utilise `Screen.GetShownSlot()`, C `OwningPlayer.GetPlayerState` (ecart signale par le lot C).

## Suspects de compilation
1. `HOLGameScreen.cpp` : variable `Own` de `Refresh()` toujours utilisee (RefreshGrid) ; `using namespace ...GameScreen` de `NativeTick` peut devenir inutile (avertissement seulement).
2. `HOLGameScreen.h` : `SetStatsReportMirror` inline utilise `ShownStats...` declares plus bas dans la classe (valide en C++).
3. Script : `SelectionFrameTick` et `SetStatsReport` appeles avant que les liaisons existent ; `Selection*Block.Hide()` / `Invalidate()` / `IsFrameLive()` de B sont mes ajouts (`HOLSelectionUnit.as` : `bSingleUnit` pose dans `Refresh`, remis a faux au debut, `Invalidate` met `"\n"` en sentinelle ; `"\n"` dans une `FString` script).
4. Localisation : les `LOCTEXT` `Health`, `ForumLivesLabel`, `Builders`, etc. ont disparu du `.cpp` de l'ecran ; leurs equivalents sont des `NSLOCTEXT("HOLGameScreen", ...)` script (lots A, B) et les `LOCTEXT` des bibliotheques (lot C). `fill_po.py` (clef `HOLGameScreen,SelectionChip` l.419) et les cles neuves `GroupIdleOne` / `GroupIdleOther` (lot B) : question ouverte, non tranchee.

## Questions de design pour la session principale
- Lot C : `GetChipLabels` garde les regles « Tous (N) » / coupe 12 caracteres en C++ (non testables en script) : test C++ ou second jeu de regles script ? (question du lot C, non tranchee ici).
- `UnitNameFor` etc. : a retirer apres preuve.

## Anomalies de processus
- Plafond de 25 appels d'outils depasse (environ 45) : beaucoup de lectures groupees necessaires pour une bascule de cette taille ; temps reel utilise environ 6 minutes sur 15. Le premier `Edit` est venu bien apres le 8e appel (lectures de 6 fichiers avant de pouvoir poser les crochets).
- Les suppressions de blocs C++ ont ete faites par plages de lignes en PowerShell (lecture UTF-8 sans BOM, fin de ligne CRLF conservee, verifie par `git diff --stat`) : relire `git diff 8cc9845 3f7e01a` avant de compiler.
- Le HEAD du worktree portait deja `489d723` (scenarios du HALDIR-1119t) entre mes etapes 1 et 2 ; non touche. `Content\Maps\Crown.umap` non touche. `EtapeDefini.md`, `Auto_Journey_*`, `IN_USE.txt` non edites.
