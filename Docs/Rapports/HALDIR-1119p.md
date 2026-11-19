# HALDIR-1119p : HUD H-9, lot A (squelette du bloc de sélection), 2026-11-19

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, branche `selection`. Aucun build, aucune bascule. Effet joueur attendu à la bascule : aucun changement visible.

## Commits
1. `d8142ee` : liaison C++ (`HOLSelectionReadLibrary.h/.cpp` + `SetSelectionReport` dans `HOLScriptScreen.h/.cpp`).
2. `638d98f` : `Script\UI\HOLSelection.as` + `HOLSelection_Test.as`.

## Signatures livrées
Module `HeroesOfLibertyUI`, `Source\HeroesOfLibertyUI\HUD\` (PAS `Match` : `HOLGameNames` est dans ce module ; la bibliothèque y voit aussi `HOLPlayerController`, `HOLSourceSubsystem`, `HOLForumMessages`).
- `UHOLSelectionReadLibrary` (script : `HOLSelectionRead::`), tous avec WorldContext implicite :
  - `static int32 GetSelectionKind()` : 0 rien, 1 bâtiment seul, 2 unités, 3 groupe de bâtiments, 4 source (ordre de `UHOLGameScreen::Refresh`) ; contrôleur = `UGameplayStatics::GetPlayerController(World, 0)`.
  - `static bool GetBuildingSelectionView(FHOLBuildingSelView& Out)`.
  - `static bool GetSourceSelectionView(FHOLSourceSelView& Out)`.
  - `static FText GetOwnerName(int32 OwnerSlot, bool bOwn)`.
- `FHOLBuildingSelView` (`USTRUCT(BlueprintType)`) : `bValid, TypeKey (FString), Name (FText), bOwn, bFinished, bFoundation, Progress (float), Health, MaxHealth, OwnerSlot, Owner (FText), Colour (FLinearColor), Builders, bForumShown, bForumHideHealth, ForumShown (FString), ForumText (FText), ForumLives`.
- `FHOLSourceSelView` : `bValid, Index, TypeKey, Name (FText), Remaining`.
- `UHOLScriptScreen::SetSelectionReport(const FString& Kind, const FString& Activity, const FString& Health, const FString& Forum)` + getters `GetSelectionReportKind/Activity/Health/Forum()`.
- Script : `class UHOLSelectionPanel : UObject` ; `UVerticalBox ChipsHost, StatsHost` ; `Build(UHOLScriptScreen, UHorizontalBox Bar)`, `Refresh()`, `FrameTick()`, `IsFrameLive()`, `SetName(FText, bool)`, `SetOwner(FText, FLinearColor, bool)`, `SetHealth(FText, bool)`, `SetActivity(FText, bool)`, `SetShownKind/Activity/Health(FString)`, `PushReport()`.
- Règles : `HOLSelectionText::*` (LOCTEXT `HOLGameScreen,<clé>` repris), `HOLSelectionRules::{SelectionWidth,BarHeight,SwatchSize,SiteBarWidth,FormatAmount,HealthText,HealthShown,PercentOf,NameText,ProgressText,BuildingShownKind,SourceShownKind,NoneShownKind,BuildersShown,BuildersText,BuildingDetail,SourceDetail,WorkerActivityShown,ForumReport,LivesChanged}`, struct `FHOLSelectionDetail {Shown, Text, bVisible}`.
- 7 tests dans `HOLSelection_Test.as` (`Test_Selection*`, aucune aide globale).

## Écarts au contrat
- Ajout hors contrat : `SetPanelShown(bool)` public (les lots B et C doivent montrer le panneau pour les chemins 2 et 3, la visibilité de `SelectionPanel` n'est pas dans le contrat). Pour ces chemins `Refresh()` cache seulement `SelectionLivesRow` et `SelectionProgressRow`.
- `Build` prend le `UHorizontalBox` de la barre du bas en paramètre : la bascule doit fournir cet hôte (un `GetSelectionHost()` comme `GetAgePanelsHost`, PAS écrit ici, `HOLGameScreen` interdit).
- Les copies `Shown*` posées par `SetSelectionReport` vivent dans `UHOLScriptScreen` (membres privés + getters), pas dans `UHOLGameScreen` (fichier interdit) : la bascule fait lire `Describe()` / `DescribeForum` par ces getters. `Forum` est posé par `RefreshBuilding` mais PAS remis à vide au chemin « rien » ni source (comportement C++ conservé : `ShownForum` n'est écrit que par `RefreshSelection`).
- `ShownSelection` des groupes de bâtiments (`"%d buildings (own): ..."`) : écrit par le lot C via `SetShownKind` (non vu ici).

## Table outil -> texte (reprise du plan section 4, état du lot A)
`selection King (own), hp ...` / `selection none` : `ShownKind` + `ShownActivity` + `ShownHealth`, pushés par `PushReport`. `HUD forum: selection, lives N` : `ShownForum` et `ShownActivity == ShownForum` (le détail du forum pose `ShownActivity = ForumShown`). Noms de widgets identiques au C++ (`SelectionPlace/Spacer/Panel/Box/Column/Name/OwnerRow/Swatch/Owner/HealthRow/HealthLabel/Health/ProgressRow/ProgressBox/Progress/ProgressText/Activity/LivesRow/LivesLabel/Lives`) ; nouveaux : `SelectionChipsHost`, `SelectionStatsHost`.

## Pas fait / questions pour la session principale
- Le panneau C++ garde les puces et stats que B/C posent dans les deux hôtes ; l'ordre dans la colonne est celui du contrat, MAIS l'espacement exact des hôtes vides repliés n'est pas comparé à une capture (risque 7 du plan).
- Pas de scénario `ecran_hud_selection_batiment` créé (hors tâche).
- Les chaînes lues par les outils sont mot pour mot ; `SetName` reçoit un `FText::FromString` (l'historique LOCTEXT du `FText::Format` C++ est perdu, comme `HOLTopBar.as`).
- Question de design : `SelectionName` était caché par le C++ quand des puces sont affichées (ligne 1294) ; c'est `SetName(..., false)` du lot C qui s'en charge, à confirmer par la bascule.
- Le nom `PercentOf` utilise `float32` : `Math::FloorToInt(Progress * float32(100.0))` reproduit le calcul float du C++ (NON VÉRIFIÉ à la compilation).

## NON VÉRIFIÉ
Rien n'a été compilé ni joué : ni C++ (includes `Resources/HOLSourceTypes.h`, `Building->GetType()` const, `UGameplayStatics::GetPlayerController(World,0)` = contrôleur de l'écran, `LOCTEXT` répété dans un second .cpp avec la même clé), ni script (WorldContext implicite d'un `UObject` script créé par `NewObject(this, ...)`, `UHorizontalBox::AddChildToHorizontalBox`, `float32(100.0)`, `FText::ToString()` sur un champ de struct, `SetAutoWrapText`, `USpacer.SetSize`, struct script `FHOLSelectionDetail` passée par valeur). Le fichier `.as` porte « NOT COMPILED YET » en tête : ne pas compiler le module script sans le lot C++.

## Anomalie de processus
Dépassement du plafond de 25 appels d'outils (environ 30) : le mur d'écriture a été atteint trop tard à cause d'un `cat` heredoc refusé et de trois Edit refusés (fichier non lu).
