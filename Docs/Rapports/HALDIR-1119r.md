# HALDIR-1119r : HUD H-9, lot C (les puces de sélection), reprise 14, 2026-11-19

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` (branche selection). Aucun build, aucun quick_check, aucune bascule. Fichiers neufs seulement.

## Commits
| Étape | sha | Fichiers |
|---|---|---|
| 1 liaison C++ | `9459005` | `Source\HeroesOfLibertyUI\HUD\HOLSelectionChipsReadLibrary.h/.cpp` |
| 2 script + tests | `e7ecd8f` | `Script\UI\HOLSelectionChips.as`, `Script\UI\HOLSelectionChips_Test.as` |

Dossier de la bibliothèque : le module `HeroesOfLibertyUI` (dossier `HUD`, à côté de `HOLGameNamesReadLibrary`). Raison : `HOLGameNames::GetUnitName/GetBuildingName` (HEROESOFLIBERTYUI_API) y sont, et le module voit déjà le contrôleur, `HOLUnitView`, `HOLFactionRules`, `HOLCommandGrid` (HeroesOfLiberty).

## Signatures C++ livrées (script : `HOLSelectionChipsRead::`)
- `static void GetChipLabels(const APlayerController* OwningPlayer, TArray<FText>& OutLabels, int32& OutActive)` : étiquettes déjà localisées (unités : sous-groupes du contrôleur si les unités sont les siennes, sinon « Tous (N) » si plusieurs types puis une par type, soldats d'abord ; groupe de bâtiments : sous-groupes, centre principal seul sans compte), `OutActive` = -1 pour les unités d'un autre joueur ou sans puces.
- `static bool GetBuildingGroupView(const APlayerController*, FString& OutReport, FText& OutOwner, FLinearColor& OutColour, FString& OutActivityShown, FText& OutActivityText)` : `OutReport` = `"%d buildings (own): X xN Y xM, subgroup S"` mot pour mot ; « Vous » ; couleur du premier bâtiment ; `", sites N"` ou vide ; « N en construction ». Faux si unités ou source sélectionnées ou pas de groupe.
- `static bool ClickChip(APlayerController* OwningPlayer, int32 Chip)` : corps de `HandleChipClicked`, `UE_LOG(LogHOLUI, Display, TEXT("HUD: chip %d clicked"), Chip)` inchangé, `CycleSubgroup` autant de fois qu'il faut. Ne rappelle PAS `Refresh` (le script le fait).
- `LOCTEXT` `HOLGameScreen,SelectionChip` = « {0} ({1}) » (et `SubgroupAll`, `You`, `GroupBuilding`) : même espace de noms `HOLGameScreen` dans le `.cpp` de la bibliothèque, donc `fill_po.py:419` continue de lire la même clé ; les mêmes clés existent encore dans `HOLGameScreen.cpp` jusqu'à la bascule (même texte).

## Signatures script livrées
- `namespace HOLSelectionChipsRules` : `NumChips()` 12, `ChipHeight()`, `ChipGap()`, `WrapSize()` 316, `ChipMaxWidth()` 156, `FramePadding()`, `FrameRadius()`, `ChipsBottomPadding()`, `ChipName(int)`, `FrameName(int)`, `ShownLabel(TArray<FString>, int)`, `ChipsShown(TArray<FString>)`, `NameShown(bool bChips)`, `FrameShown(FString)`, `FrameLit(int Active, int Chip)`, `LabelChanged(FString, FString)`, `ClickAccepted(int)`.
- `class UHOLSelectionChipHandler : UObject` (relais du clic, un par puce, même motif que `UHOLGroupBarChip`).
- `class UHOLSelectionChipsPanel : UObject` : `void Build(UHOLScriptScreen Screen, UHOLSelectionPanel Owner)`, `void Refresh()`, `bool ClickChip(int Chip)`, `bool ClickName(FName WidgetName)`. Champs de lecture pour les tests : `Shown`, `ShownActive`, `bChipsShown`.
- Widgets (mêmes noms que le C++) : `SelectionChips` (UWrapBox, repliée, 316 de large, écart 4) dans `Owner.ChipsHost` ; `SelectionChipFrame0..11` (UBorder arrondi 5, padding 2, transparent, replié) > `SelectionChipBox0..11` (USizeBox 30 de haut, largeur max 152, clip) > `SelectionChip0..11` (UHOLQueueItem). Le `SizeBox` portait un nom auto en C++ ; je lui donne `SelectionChipBox<N>` (nom nouveau, aucun outil ne le cherche : NON VÉRIFIÉ par grep des outils).
- Le CROCHET (membre, Build, Refresh, ClickWidget, C++ à retirer) est en tête de `HOLSelectionChips.as`.

## Écarts au contrat et au plan
1. Les règles « Tous (N) en tête si plusieurs types », « Nom (N) / Nom seul », la coupe à 12 caractères et « aucun actif pour un autre joueur » (INDEX_NONE) vivent dans `GetChipLabels` (C++), parce que le brief veut les étiquettes déjà localisées en C++ et « une seule source ». Elles ne sont donc PAS testables en script. Les tests script (9) portent sur la vue : étiquette gardée dans l'ordre reçu (« Tous (8) » reste en tête), « Nom » seul gardé, limite de 12 puces, aucun cadre allumé pour -1, nom caché avec des puces, texte écrit seulement s'il change, noms des widgets, clic accepté, largeurs. QUESTION : veux-tu un test C++ (D838 : tests de règles côté Mass seulement) ou un second jeu de règles script alimenté par noms bruts et comptes (alors la mise en forme « {0} ({1}) » passerait en `NSLOCTEXT` script, et `fill_po.py:419` devrait lire le `.as`) ?
2. `ShownSelection` du groupe de bâtiments : je ne l'écris pas, j'appelle `Owner.SetShownKind(Report)`. J'appelle aussi, pour le groupe seulement, `SetHealth("", false)`, `SetShownHealth("")`, `SetOwner(Vous, couleur, true)`, `SetActivity(texte, ActivityShown != "")`, `SetShownActivity(ActivityShown)` : le contrat du lot A les expose, mais `ShownActivity` / `ShownHealth` avaient « un seul écrivain, le lot A ». Écart à arbitrer : si A écrit déjà ces champs pour le groupe (cas 3 de son aiguillage), supprimer `RefreshBuildingGroup()` de mon fichier ; sinon A doit laisser le cas « groupe de bâtiments » à mon `Refresh`.
3. Pas d'API du contrat pour cacher la rangée de chantier (`SetShown(SelectionProgressRow, false)` du C++ `RefreshBuildingGroup`) ni la rangée de stats : à la charge de l'aiguillage de A. NON couvert par moi.
4. Le nom : `SetName(FText, bool)` écrase le texte. Quand il y a des puces, j'appelle `Owner.SetName("", false)` à chaque `Refresh` (le C++ ne faisait que cacher). Conséquence : mon `Refresh` doit tourner APRÈS celui de A et B (écrit dans le CROCHET), sinon le nom d'une unité seule pourrait réapparaître une image. Si A offre un `SetNameShown(bool)` ou une lecture de `ChipsHost`, ce serait plus propre : QUESTION.
5. Le joueur de référence est `OwningPlayer.GetPlayerState<AHOLPlayerState>()`, le C++ utilisait `BoundPlayerState` de l'écran (spectateur suivi ?). Pour un spectateur qui suit un joueur, le « bOwn » peut différer. NON VÉRIFIÉ ; le plan citait `GetShownSlot()` (HOLGameScreen.h:89).
6. `ClickChip` : le script n'appelle plus `Refresh()` de l'écran C++ mais celui du panneau (`UHOLSelectionChipsPanel.Refresh`), donc le reste du bloc (A, B) se met à jour à la prochaine image au plus tard (le C++ refaisait tout le bloc dans le clic). À vérifier à la capture : la puce active suit-elle au clic suivant sans image de retard.

## Ce qui n'est pas fait
- Aucune bascule ni crochet posé dans `HOLGameScreenScript.as`, rien retiré de `HOLGameScreen.cpp/.h` (consigne).
- Scénario `ecran_hud_selection_puces` (plan section 5) non créé (hors périmètre de ce lot).
- Placeholders.txt : aucun provisoire ajouté ni retiré.

## NON VÉRIFIÉ
- RIEN n'est compilé (ni C++, ni Angelscript) : le .as dépend de `UHOLSelectionPanel` du lot A (inexistante pour moi) et de la liaison C++ à compiler.
- Existence et signature exactes de membres que j'ai appelés d'après le C++ lu : `AHOLPlayerController::GetSelectedUnits/GetSelectedSource/HasBuildingGroup/GetSelectedBuildings/GetSubgroups/GetUnitSubgroups/GetActive*Subgroup/CycleSubgroup/DescribeSubgroup` (vus dans `HOLPlayerController.h`), `UHOLUnitSubsystem::FindTypeRowFor`, `UHOLBuildingSettings::FindType`, `HOLFactionRules::GetSlotFactionKey(const UWorld*, int32)`, `HOLPlayerColors::GetColor` (vus par grep). `HOLUnitView::Gather/Find` et `FHOLUnitView::CarryCapacity/UnitId/OwnerPlayer/TypeName` : copiés du C++ de l'écran.
- Que `APlayerController` passé depuis `Screen.GetOwningPlayer()` soit bien l'`AHOLPlayerController` local (cast) : oui en solo/client, non rejoué.
- Côté script : `FSlateBrush` arrondi sur `UBorder.SetBrush` (modèle `HOLPlayerTable.as:385`, sur une UImage), `UWrapBox.SetInnerSlotPadding/SetExplicitWrapSize/SetWrapSize`, `USizeBox.SetClipping/SetMaxDesiredWidth`, `AddChildToWrapBox`, méthode `private` d'une classe script, `UHOLScriptScreen::GetAccentColor()` statique (vu dans `HOLQueuePanel.as:6`), assertions `AssertEquals` sur des doubles : modèles proches mais aucun n'a compilé pour CE fichier.
- Que `SetDetailLine` du lot A traite `(Shown, Text)` comme mon `SetActivity(texte, bShown)` + `SetShownActivity(chaîne)` : lecture du contrat seulement.
- Le rendu (cadre allumé à la couleur d'accent, retour à la ligne, puce active qui suit le clic) : preuve par capture AVANT/APRÈS à faire par la session principale (leçon 3). Table outil -> texte lu : journal `HUD: chip %d clicked` non lu par un outil (plan section 4) ; `SelectionChip<N>` par `hol.HUD.Click` : aucun outil ne les clique encore.

## Anomalies
- Un premier `Bash` avec heredoc a échoué sur un guillemet (aucun fichier écrit) ; fichiers refaits avec Write. Aucun conflit d'index. `Content\Maps\Crown.umap` non touché. Fichiers des lots A (`HOLSelectionReadLibrary.*` vus non suivis) non touchés.
