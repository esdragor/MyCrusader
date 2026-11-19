# HALDIR-1119q : H-9 lot B (unité et stats), reprise 14, 2026-11-19

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, branche `selection` (base main 3b52f7a). Aucun build, aucune bascule, aucun fichier des lots A et C ni `HOLGameScreen*` / `HOLScriptScreen*` touché. Effet joueur : aucun (fichiers neufs).

## Commits
| Étape | sha | Fichiers |
|---|---|---|
| 1 liaison C++ | `0dbd85b` | `Source\HeroesOfLibertyUI\HUD\HOLSelectionUnitReadLibrary.h/.cpp` |
| 2 script + tests | `e640255` | `Script\UI\HOLSelectionUnit.as`, `Script\UI\HOLSelectionUnit_Test.as` |

Module : `HeroesOfLibertyUI` (dossier HUD, comme `HOLGameNamesReadLibrary`) : il voit `HOLGameNames` (qui n'est pas dans le module `HeroesOfLiberty`) et `HOLUnitView`, `HOLUnitSubsystem`, `HOLFactionRules` par sa dépendance sur `HeroesOfLiberty`. Macro `HEROESOFLIBERTYUI_API`.

## Signatures livrées (C++, `HOLSelectionUnitRead::` en script)
- `static bool GetUnitSelectionView(const UObject* WorldContextObject, int32 ViewerSlot, FHOLUnitSelView& Out)` (WorldContext) : lit `GetFirstPlayerController()->GetSelectedUnits()` (Gather, ou Find pour 1 unité), même filtre que `RefreshUnitSelection` 1090-1104.
- `static bool GetUnitStatsRow(const UObject*, FName Type, int32 OwnerSlot, int32 ViewerSlot, FHOLUnitStatsRow& Out)`.
- `static int32 GetUnitStatsMode()`, `static int32 GetUnitStatsBuildingMode()` : lisent par NOM (`IConsoleManager::FindTConsoleVariableDataInt`) les CVars `hol.HUD.UnitStats` et `hol.HUD.UnitStatsBuilding`, qui restent enregistrées dans `HOLGameScreen.cpp` (pas de second enregistrement : il aurait doublonné). 1 si non trouvée.
- `static FText GetUnitName(const UObject*, FName Type, int32 OwnerSlot)` (= `UnitNameFor` 2522).
- `static FText FormatStat(float Value)` (AJOUT hors liste : `FText::AsNumber`, 1 décimale max, culture du jeu : garde « 13,2 » en français sur les puces).
- `FHOLUnitSelView` (USTRUCT BlueprintType, champs simples + tableaux parallèles, pas de conteneur imbriqué) : `UnitCount, OwnerSlot, bOwn, OwnerColour, OwnerName(FText), TypeNames[], TypeCounts[], TypeWorkers[], WorkerActivities[] (int), bHero, Health, MaxHealth, bHealthExact, ActivityKind(int), ActivityName(FString = UEnum::GetValueAsString), LoadResourceName, LoadResource(int), LoadAmount, bLoadAmountKnown, CarryCapacity`.
- `FHOLUnitStatsRow` : `Attack, BuildingAttack, MeleeArmor, RangedArmor, HeroArmor, SiegeArmor, Speed (cm/s), BonusPercent`.
- Codes : activité 0 Idle, 1 Moving, 2 Gathering (+ GatherTrip), 3 Building, 4 Attacking ; ressource 0 Food, 1 Wood, 2 Stone, 3 Gold (`EHOLUnitActivity` n'est pas BlueprintType, d'où les entiers).

## Signatures livrées (script)
- `class UHOLSelectionUnitPanel : UObject` : `Build(UHOLScriptScreen Screen, UHOLSelectionPanel Owner)`, `Refresh()`, `RefreshFrame()`, `Hide()`, `HideStats()`, `bool TakeStatsReportChange()` ; membres lus : `ShownStats`, `ShownStatsTip`, `ShownStatTipList`.
- `namespace HOLSelectionUnitRules` : `SortedTypeOrder`, `TypeBefore`, `TypeLabel`, `SelectionKind`, `FormatPlain`, `Number`, `AttackWithBonus`, `StatsShown`, `BuildingOnly`, `BuildingExtra`, `BuildStatLines(Attack, BuildingAttack, Melee, Ranged, Hero, Siege, Speed, Percent, BuildingMode, bLocal)` -> `FHOLSelectionStatLines {Lines, Plain, Tips, TipPlain}`, `ChipText`, `JoinWith`, `HealthFragment`, `ActivityShown`, `LoadShown`, `ActivityFragment`, `ActivityText`, `CountWorkers` -> `FHOLSelectionWorkerCounts`, `GroupFragment`, `GroupText`, `StatsKey`. Textes dans `namespace HOLSelectionUnitText` (NSLOCTEXT `HOLGameScreen,<clé>` : mêmes clés que le C++).
- 10 tests `Test_SelectionUnit*` (tri, étiquettes, 1 décimale, attaque avec bonus, mots `attack 13.2 (+1.2)` / `melee armor 4` / tips, jointures `HUD stats:` et `HUD stats tip:`, 2 modes bâtiment + Bélier, 3 modes de stats + séparateur, santé et activité, compte d'états).

## Relevés ShownStats / ShownStatTipList (lus par `HUD stats:` / `HUD stats tip:`, HOLUIHUD.cpp:63-64)
Le script les pose dans le panneau (`ShownStats`, `ShownStatTipList`, `ShownStatsTip`, vides quand les stats sont cachées) et lève `TakeStatsReportChange()`. Proposition pour la bascule (non ajoutée) dans `UHOLScriptScreen` : `UFUNCTION() void SetStatsReport(const FString& Stats, const TArray<FString>& TipList);` qui copie dans `UHOLGameScreen` `ShownStats = Stats; ShownStatTipList = TipList; ShownStatsTip = join(TipList, " | ")` (même motif que `SetQueueReport`). Crochet script : `if (SelectionUnit.TakeStatsReportChange()) Screen.SetStatsReport(SelectionUnit.ShownStats, SelectionUnit.ShownStatTipList);`. `DescribeStatsTip` (1434) lit des puces C++ : à réécrire pour chercher `SelectionStats` / `SelectionStat<i>` dans l'arbre du script (ou lire `ShownStatTipList` seul quand `ShownStats` non vide). `HoverStat` : puce = widget nommé `SelectionStat<i>` (détail dans le `// CROCHET :` en tête du `.as`).

## Écarts au contrat
1. `GetUnitStatsRow` a un paramètre `ViewerSlot` en plus (le bonus des technologies est celui du seul joueur propriétaire, calculé côté C++ dans `BonusPercent`).
2. `FormatStat` ajouté (culture) ; `OwnerName` / `OwnerColour` dans la vue (copie locale de `GetOwnerNameIn`, mêmes clés `HOLGameScreen,You` / `PlayerN` ; doublon possible avec `HOLSelection::GetOwnerName` du lot A : à dédupliquer par la bascule si souhaité).
3. Je n'écris pas les champs `Shown*` ; je passe `SetShownKind`, `SetShownHealth`, `SetShownActivity` avec mémos propres (je ne lis pas les champs du lot A).
4. J'appelle aussi `Owner.StatsHost.SetVisibility(...)` (SelfHitTestInvisible quand des stats, Collapsed sinon) : le contrat dit que l'hôte est replié par défaut, non précisé qui le montre.
5. `SelectionStats` est un `UWrapBox` (comme `HOLGameScreen.cpp:2741`), 6 puces `SelectionStat0..5` `UHOLTextStyleHUD`, masquées, tooltip posé.
6. Le panneau doit être créé par le lot A avec un outer qui donne un monde : `NewObject(this, UHOLSelectionUnitPanel)` (les liaisons `WorldContext` s'appuient dessus).

## Nouvelle localisation à traiter (question ouverte)
Le motif C++ `GroupIdle` `"{0} {0}|plural(one=inactif,other=inactifs)"` ne se formate pas en script : deux clés NEUVES `HOLGameScreen,GroupIdleOne` (« {0} inactif ») et `GroupIdleOther` (« {0} inactifs »). Les traductions EN / JA de `GroupIdle` ne les couvrent pas : `fill_po.py` à compléter, ou autre choix de ta part (design non tranché).

## Non fait
Bascule (crochet dans `HOLGameScreenScript.as`, retrait du C++ de `HOLGameScreen.cpp/.h`, `SetStatsReport`, adaptation de `HoverStat` et `DescribeStatsTip`) ; scénario `ecran_hud_selection_unite` ; mise à jour de `Placeholders.txt` (rien de provisoire ajouté). Le journal `EtapeDefini.md` n'est pas édité (consigne).

## NON VÉRIFIÉ (rien n'a été compilé ni joué)
- Tout le C++ : champs `FHOLUnitTypeRow::Attack`, `Speed`, `AttackType`, `HOLTechnology::GetAttackStat(EHOLAttackType)` (espace de noms supposé d'après `HOLTechnologyRules.h:105`), `AHOLGameState::GetLaunchConfig()`, chemins d'include, `GetFirstPlayerController()` comme contrôleur local, `UEnum::GetValueAsString` sur `EHOLResource` (UENUM(BlueprintType) vu).
- Tout le script : comparaison `FString < FString` (opCmp) dans `TypeBefore` (sinon remplacer par une liaison `LexicalLess` en C++), paramètre local `Separator` de `JoinWith` qui masque la fonction `Separator()` (renommer en cas d'erreur), `UWrapBox` / `AddChildToWrapBox` / `Screen.ConstructWidget(UWrapBox, ...)`, `Array.Insert(Index, Place)`, `for (int Kind : Activities)`, `f"..."` avec `{Sign}`, `T.AssertTrue/AssertFalse`, accents UTF-8 dans les `NSLOCTEXT` et les tests (`Récolte — Bois 4/10`, `·`).
- La chaîne `HUD stats:` n'a pas été rejouée (`unit_stats_attack.txt`, `forge.txt`) ; le cadre de la barre (`SelectionStats`) n'a pas été comparé en capture.
- Coût par image : `RefreshFrame` appelle `GetUnitSelectionView` + `GetUnitStatsRow` chaque image pour 1 unité, ne reconstruit les 6 textes que si la clé (type|propriétaire|bonus|modes) change ; à mesurer avec `hol.HUD.Report`.
- Le fichier `Content\Maps\Crown.umap` (non suivi) n'a pas été touché.
