# CELEBORN-1118p : lot ST-1 (écran Statistiques, accroches script)

**NON COMPILÉ.** Aucun build, aucun quick_check, aucun éditeur. Branche `hero-j8-close`, commit `2f3ff34` (6 fichiers, +452 / -57), non fusionné, non poussé.
Fin ~20:42 (arrêt dur 20:51 respecté, ~18 appels d'outils). Pas touché : EtapeDefini.md, Auto_Journey_File.md, *InProgress.md.

BUILDS_DEMANDES: 0 (le build du lot est à lancer par la session principale : `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait`)
TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: ~57 (lambda AddCell et tableau Headers, devenus des membres) ; LIGNES_MIGREES_PAR_LOT: 0 (ST-2 et ST-3 suivent).

Comportement actuel inchangé : `Open` pousse toujours `StaticClass()`, la classe reste concrète (`UCLASS(Blueprintable)`, plus de `Abstract`), `BuildLayout` / `Refresh` / `RefreshTimeline` C++ jouent tels quels tant que l'objet est de la classe C++ (`GetClass() == StaticClass()`). Ils sautent vers les `BP_*` seulement pour une classe fille (script).

## Ce qu'exposait déjà UHOLMatchStatsReadLibrary (non redoublé)
Structs `FHOLMatchStatsSampleState`, `FHOLPlayerMatchStatsState` (Samples, MaxWorkers...), `FHOLMatchStatsEventState`, `FHOLMatchStatsState` (Players, Events), `FHOLMatchStatsChunkState` ; fonctions GetMilitaryScore(UnitsKilled), GetEconomyScore(Gathered, UnitsTrained), GetSeriesValue, GetNiceStep, GetAxisEnd, FormatMatchTime, MakeCurve(Player, SeriesIndex), MakeChunk, MergeChunk, GetNextFromSample, CanReceiveStats, Record*/AddSample. Il MANQUAIT : le moyen de lire le sous-système (`GetStats()`), et le nom / couleur / équipe / résultat des lignes (GameState). Ajouté ci-dessous.

## Liste exacte appelable du script

### UHOLMatchStatsReadLibrary (module HeroesOfLiberty ; namespace script `HOLMatchStatsRead::`, comme les autres ; le `WorldContextObject` est rempli par le script depuis `this`)
- struct `FHOLStatsRowState` : `int SlotIndex` ; `FText Name` ; `FLinearColor Color` ; `int Team` (0 = aucune, afficher « — ») ; `int Result` (0 En jeu, 1 Vainqueur, 2 Vaincu). Une ligne par slot non fermé, dans l'ordre des slots ; `SlotIndex` = indice de `FHOLMatchStatsState::Players`.
- `FHOLMatchStatsState HOLMatchStatsRead::GetWorldStats(const UObject WorldContextObject)` : `GetStats()` du sous-système (solo : l'enregistrement ; client : la copie), vide sans sous-système.
- `bool HOLMatchStatsRead::HasWorldStats(const UObject WorldContextObject)` : au moins un joueur avec `PlayerIndex != INDEX_NONE` (règle « en attente du serveur » de Refresh:313).
- `TArray<FHOLStatsRowState> HOLMatchStatsRead::GetWorldStatsRows(const UObject WorldContextObject)` : vide sans GameState.
- Dynamique « statistiques mises à jour » : PAS ajoutée au sous-système. L'écran C++ garde l'abonnement (`GetOnStatsUpdated` / `GetOnStatsRefused`, minuterie de 10 s, `RequestStats`) et le renvoie au script par `BP_RefreshStats(bRefused)` ; le script n'a donc rien à lier. (Si un autre consommateur script veut l'événement, ce sera un lot à part.)

### UHOLStatsGraph (module HeroesOfLibertyUI ; la peinture NativePaint reste C++, inchangée)
- struct `FHOLStatsGraphCurve` : `FText Name` ; `FLinearColor Color` ; `TArray<FVector2D> Points` (X temps en s, Y valeur : la sortie de `MakeCurve`).
- struct `FHOLStatsGraphMark` : `int TimeSeconds` ; `FLinearColor Color` ; `FText Label`.
- `void SetGraphData(const TArray<FHOLStatsGraphCurve>& InCurves, const TArray<FHOLStatsGraphMark>& InMarks)` (convertit puis appelle `SetData`).
- `FString GetAxesDescription() const` (= `DescribeAxes`, pour les journaux).

### UHOLStatsScreen (classe `UHOLStatsScreen`, le script en dérive : `class UHOLStatsScreenScript : UHOLStatsScreen`)
Statique :
- `TSubclassOf<UHOLStatsScreen> FindScriptClass()` : la plus récente classe fille non abstraite (la classe C++ elle-même est exclue, car concrète) ; null sans script.
- `FText TextOf(FName Key)` : Heading, Summary, Military, Economy, Timeline, SeriesWorkers, SeriesMilitary, SeriesPopulation, SeriesBuildings, SeriesGathered, SeriesStock, Back, Refused, Waiting, Winner, Defeated, Playing ; vide sinon.
- `TArray<FText> HeadersOf(int Tab)` : 0 Résumé (6 titres), 1 Militaire (5), 2 Économie (8) ; vide sinon. Le C++ Refresh l'utilise aussi (une seule source).
- `FText MakeMarkLabel(int Kind, const FText& PlayerName)` : Kind 0 « {0} vaincu », 1 « Forum de {0} tombé ».
Instance :
- `void SelectTab(int Index)`, `void SelectSeries(int Index)`, `void Close()` (appelables).
- `int GetActiveTab() const`, `int GetActiveSeries() const`, `bool IsRefused() const`.
- Mise en page : `UVerticalBox MakeStatsColumn()` ; `UCommonTextBlock AddStatsText(UVerticalBox Column, FName Name, const FText& Text, int Style)` (0 titre, 16 dessous ; 1 corps) ; `UHorizontalBox AddStatsRow(UVerticalBox Column)` ; `UHOLButton AddStatsButton(UHorizontalBox Row, FName Name, const FText& Label)` (onglets et Retour) ; `UWidgetSwitcher AddStatsPages(UVerticalBox Column)` (pose `TabPages`, remet `Grids` à zéro) ; `UGridPanel AddStatsGridPage()` (ajoute à `Grids`, 3 appels : Résumé, Militaire, Économie) ; `UVerticalBox AddStatsTimelinePage()` (remet `SeriesButtons` à zéro) ; `UHorizontalBox AddStatsSeriesRow(UVerticalBox TimelinePage)` ; `UHOLButton AddStatsSeriesButton(UHorizontalBox Row, FName Name, const FText& Label)` (ajoute à `SeriesButtons`, ordre EHOLMatchStatsSeries) ; `UHOLStatsGraph AddStatsGraph(UVerticalBox TimelinePage)` (pose `Graph`, 1100 x 440) ; `void AddStatsCell(UGridPanel Grid, int Row, int Column, const FText& Text, bool bHeader, bool bColored, const FLinearColor& Color)` (pastille de couleur avant le nom si `bColored`).
- Propriétés `BlueprintReadWrite` (le script les affecte avec ce que renvoient les aides ; `NativeOnInitialized` câble onglets / séries / Retour sur elles) : `Status` (UCommonTextBlock), `TabPages`, `SummaryTab`, `MilitaryTab`, `EconomyTab`, `TimelineTab`, `BackButton`, `Graph` ; `BlueprintReadOnly` : `Grids`, `SeriesButtons`.
- Événements à surcharger (`UFUNCTION(BlueprintOverride)`, protégés) : `void BP_BuildStats()` (à la place de BuildLayout) ; `void BP_RefreshStats(bool bRefusedNow)` (à la place de Refresh : à chaque OnStatsUpdated / refus ; la fonction de script doit gérer Status, visibilité de TabPages, les 3 grilles : `Grid.ClearChildren()` puis en-têtes + lignes, puis la chronologie) ; `void BP_RefreshTimeline(int Series)` (à la place de RefreshTimeline : à chaque SelectSeries ; ATTENTION : appelé aussi depuis NativeOnInitialized avant toute statistique, le script doit tolérer des listes vides).

## Reste (ST-2, ST-3)
- ST-2 (script) : `HOLStatsScreen.as` (BP_BuildStats = BuildLayout:62-112 avec les aides ; BP_RefreshStats = Refresh:308-428 ; BP_RefreshTimeline = 242-302 : courbes par ligne `GetWorldStatsRows` + `MakeCurve(GetWorldStats().Players[SlotIndex], Series)`, repères par `Events` avec `MakeMarkLabel`) ; `HOLStatsRules.as` + tests (rouge d'abord).
- ST-3 (bascule) : `Open` pousse `FindScriptClass()` (avertissement et null sans classe, comme Settings:156-162), passer la classe en `Abstract`, retirer BuildLayout / Refresh / RefreshTimeline C++ et leurs LOCTEXT dupliqués (Refused, Waiting, Winner, Defeated, Playing, Heading... existent maintenant deux fois, même clé et même texte dans `LOCTEXT_NAMESPACE "HOLStatsScreen"`, sans conflit).
- Non lu : les autres appelants de `Open` (HOLUIOpen.cpp:91-93, HOLGameScreen.cpp:3331 / 2842 / 2850) : inchangés, aucune action ici.

## NON VÉRIFIÉ / suspects de compilation
1. Tout (aucune compilation). Edition par script PowerShell avec comptes exacts (aucun refus) ; fins de ligne et BOM conservés.
2. `UHOLStatsScreen` : `UHOLButton`, `UCommonTextBlock`, `UVerticalBox`, `UHorizontalBox`, `UHOLStatsGraph` sont seulement déclarés en avant dans le .h (UFUNCTION avec pointeurs : normal) ; si UHT demande les types complets pour une propriété `BlueprintReadWrite`, inclure leurs en-têtes (Settings n'en a pas eu besoin).
3. `UCLASS(Blueprintable)` + propriétés déplacées dans un bloc `public:` (le bloc `protected:` ne garde que les surcharges et les 3 `BP_*`) ; `private:` reste pour `IsScriptScreen`, `Refresh`, etc. `Close` est maintenant une UFUNCTION utilisée par `AddUObject(this, &UHOLStatsScreen::Close)` : devrait passer.
4. `AddText(Column, Name, Text, UHOLTextStyleHeading::StaticClass(), 16.f)` : conversion implicite `UClass*` vers `TSubclassOf<UCommonTextStyle>` (déjà faite à l'identique dans BuildLayout).
5. `HOLMatchStatsReadLibrary.cpp` : nouveaux includes `Match/HOLGameState.h`, `HOLForumMessages.h`, `HOLPlayerColors.h`, `HOLMatchStatsSubsystem.h`, `Engine/Engine.h` ; `Row.Team = Slots[i].Team` (int32 vu dans HOLMatchConfig.h:70) ; `HOLPlayerColors::GetColor` converti par `FLinearColor(...)` comme dans l'écran C++. Build unity : aucun nouveau namespace anonyme.
6. `FindScriptClass` : sans `*It != StaticClass()` la classe C++ concrète serait elle-même renvoyée ; c'est géré, mais à vérifier au premier lancement avec une classe script.
7. Angelscript : le nom de paramètre `Row` / `Column` / `Text` n'entre pas en conflit connu ; pas de `BlueprintImplementableEvent` avec struct en `const&` (que des `bool` et `int32`).

## Anomalies
Un appel Bash (python3 sur stdin) a bloqué 120 s (tâche d'arrière-plan `bcfb8sr60`, inoffensive, à ignorer). `IN_USE.txt` du worktree remis à « libre » (non commité). Fichiers `Tools/Scenarios/c28_*`, `c31_*` non suivis dans le worktree : pas les miens.

## Commande à lancer par la session principale
`powershell -NoProfile -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait` (UN build avec les lots C++ des autres agents si fusionnés), puis `quick_check -Parts tests` (rien de testé par ce lot : comportement C++ inchangé).
