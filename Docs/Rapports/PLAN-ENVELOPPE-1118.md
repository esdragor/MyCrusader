# PLAN-ENVELOPPE-1118 : étape 5 « enveloppe de jeu » (lecture seule, main 84a42b2, 2026-11-18)

Agent PLAN-ENVELOPPE-1118, aucune édition de code. Mesures faites avec grep de signatures et de plages ; les numéros de
ligne sont les numéros physiques du fichier (le contrôleur va jusqu'à ~7 115 lignes physiques, les 6 741 de l'énoncé sont
les lignes non vides : les tailles ci-dessous sont en lignes non vides, les plages en lignes physiques).

Impact joueur : aucun tant que les lots restent « commande console / ordre / règle » ; les vrais risques joueur sont la pause,
le placement de bâtiment (fantôme par image), la file de production et la défaite par forum.

## 1. Verdict par fichier et par bloc

Règle de lecture : RESTE = raison D839 (E = par pas / par entité ou par image, R = réplication, L = liaison à exposer,
M = mesure de performance). MIGRE = vers un `.as`, par un pont de type `HOLUnitTierSubsystem` (sous-système Abstract +
sous-classe script, ~50-80 lignes C++) ou une bibliothèque statique (`UFUNCTION` BlueprintCallable).

### 1.1 `Player\HOLPlayerController.cpp` (6 741 l. ; en-tête 851 l., 18 UFUNCTION, 3 propriétés répliquées)

| Bloc | Plage | Taille | Verdict |
|---|---|---|---|
| Route RPC script (`HOLRpcScriptRoute_Haldir1117n`, CallN/CallCheat/Route*, IsOn, LogNotSent) + statiques | 55-100, 5447-5752 | ~490 | RESTE (L : c'est la liaison vers `HOLRpcPlayer.as`) ; fondable quand les `Request<X>` C++ n'ont plus d'appelant |
| **Commandes console `hol.*`** : **106 `FAutoConsoleCommand`** (191-2430), toutes des lambdas fines de 12-20 lignes qui bouclent sur `GetLocalControllers()` et appellent un `Request<X>` ou une méthode du contrôleur | 191-2440 | ~2 240 | voir 1.1a |
| Initialisation, `SetupInputComponent`, `PlayerTick`, `HandlePan/Zoom/Drag`, `Handle*Key`, `HandleSelect*`/`HandleOrder*` (Enhanced Input) | 2444-2780, 6424-6790 | ~650 | RESTE (E : `PlayerTick` chaque image, liens Enhanced Input ; L) |
| Caméra (`TryPlaceCamera`, `GetCursorFraction`, `FocusCameraOnForum`) | 2679-2780, 5338 | ~120 | RESTE (E : par image, entrée) |
| Sélection et picking (`FindBuildingAt`, `GatherPickCandidates`, `FindSourceAt`, `SelectAt/InBox/SameType`, `SetSelection`, `PurgeSelection`, `GetSelectionBox`, `OrderAt` 3097-3276) | 2780-3710, 3832-3935 | ~930 | RESTE pour l'essentiel (E : le HUD lit la boîte et la sélection à chaque image ; état de sélection lu par le C++) ; ~120 l. (`SelectNextIdleWorker`, `SelectBuildingCategory`) MIGRENT avec 1.1d |
| Sous-groupes, catégories, groupes de contrôle (`GetSubgroups`, `CycleSubgroup`, `RestoreSubgroup`, `GetUnitSubgroups`, `Add/Remove/SelectControlGroup`, `SelectArmy/Hero`) | 3316-3478, 6790-7029 | ~600 | MIGRE ~400 l. de calcul pur (vers `Script\Player\HOLSelectionRules.as`, liaison = bibliothèque de lecture sur l'état de sélection, ~80 l. C++) ; l'état reste en C++ |
| Grille dans le contrôleur (`GetGridCommands`, `GetUnitsContext`, `GetOrderCommands`, `PressGridKey`, `ClickAge`, `ExecuteGridCommand` 4208-4425) | 3937-4425 | ~490 | MIGRE avec `HOLCommandGrid` (1.2) ; `ExecuteGridCommand` = aiguillage vers `Request<X>` / `BeginPlacement` |
| Placement de bâtiment (`BeginPlacement`, `EndPlacement`, `GetPlacementGhost`, `CheckPlacementHere`, `UpdatePlacementGhost`, `ClickPlacement`) | 4749-4930 | ~180 | RESTE (E : `UpdatePlacementGhost` à chaque image, appelle `CheckPlacement` ; état lu par le HUD) |
| **`Request<X>`** (~65 fonctions de 8 lignes, déjà « route vers l'acteur script, sinon LogNotSent », D845) | 4482-4760, 4911-5130, 5269-6124 et 6203-6244 | ~520 | MIGRE par **suppression** (zéro ligne C++ ensuite) quand leurs appelants C++ (HUD, tests, console) sont partis : dépend de l'étape 4 et des lots console |
| **`Do<X>`** (~60 corps serveur, 10-20 l. : `HOLPlayerOrders::Move(...)`, sous-systèmes, cheats `#if !UE_BUILD_SHIPPING`) | 4567-6340 | ~850 | MIGRE vers `Script\Net\HOLRpcPlayer.as` (bas débit, par ordre), **après** l'exposition de `HOLPlayerOrders` / `UHOLCommandSubsystem` (lots M1, P1, P2) ; cheats : liaison supplémentaire ~100 l. C++ (`WoundUnits`, `ForumPresence`...) |
| Réplication : `GetLifetimeReplicatedProps`, `OnRep_WatchedLoad`, `UpdateWatchedUnit/DoWatchUnit/SendWatchedLoad`, `Tick` serveur, `ClientShowDepositPopups`, et les 5 RPC de grosses charges (`ServerRequestMatchStats`, `ClientReceiveMatchStats`, `ClientReceiveFogMemory`, `ClientMatchStatsRefused`...) | 3703-3830, 7063-7115 | ~250 | RESTE (R ; D842) |
| `Describe*`, `ReceiveSignal/Chat`, `GetActiveSignals`, `IsInputFrozen`, pause | 5937-6100, 6351-6424, 7039-7062 | ~260 | MIGRE ~150 (affichage des signaux et du chat), reste ~110 |

**1.1a Les 106 commandes console `hol.*`** (comptées : 106 `FAutoConsoleCommand`, plus 2 cvars lus ; 191-2430, ~21 par centaine de
lignes). Par familles (noms) :
- ~45 « cheat / victoire / héros / duel / diplomatie / signal / chat / spé / marché / tribut / production » (`hol.Cheat.*` 17,
  `hol.Victory.*` 5, `hol.Hero.*` 2, `hol.Duel.*` 3, `hol.Diplomacy.*` 2, `hol.Signal.*`, `hol.Chat.*`, `hol.Spec.*` 3,
  `hol.Market.*`, `hol.Tribute`, `hol.Production.*` 5, `hol.Spawn.Here`) : MIGRENT (`Script\Dev\HOLControllerCommands.as`,
  `FConsoleCommand` existe dans le moteur script, 7 usages dans le projet). Ils appellent déjà des `Request<X>` qui passent
  par l'acteur script : le script appelle l'acteur directement, **zéro C++** ou quelques lignes de lecture du « contrôleur local ».
  ~1 000 lignes C++ retirées, ~600 de script.
- ~50 « pilote de test » (`hol.Select.*` Click/Box/Order/Group/Army/Grid/Subgroup, `hol.Units.*`, `hol.Build.*`, `hol.Camera.*`) :
  chacun appelle une méthode du contrôleur qui n'est PAS une UFUNCTION (18 seulement dans l'en-tête). Migrer = exposer ~45
  méthodes (bibliothèque ~250 l. C++) pour retirer ~1 240 l. : gain net réel mais faible, **à décider (question 1)**.
- ~10 « rapports » (`hol.*.Report`) : lisent l'état (`DescribeSelection`...) ; suivent le sort des pilotes.

### 1.2 `Player\HOLCommandGrid.cpp` (517 l. ; en-tête 436 l.)

Fonctions pures sur des structures de contexte (`GetCommand` 55-136, `GetOrder`, `GetTradeCell/Gain`, `PressKey` 238, `ClickAge`,
`Check*` 313-365, `Describe` 376, `NextInCycle`, `SplitSubgroups` 460, `MergeCommands` 509, `PickEvenly`). Aucune boucle par
entité. Déjà amorcé : bibliothèque de lecture `UHOLCommandGridReadLibrary` + `Script\Tests\HOLCommandGrid_Test.as` (963 l.,
10 tests sur 15 portés, GALADRIEL-1117m). Verdict : **MIGRE ~480 l.** vers `Script\Player\HOLCommandGridRules.as` en 2 lots
(55-240 puis 238-531). Liaison : les structures `FHOLGridContext` / `FHOLGridCommand` doivent être exposées (ce qui est
exposé aujourd'hui n'est pas vérifié, ~60-100 l. C++) et les appelants C++ (contrôleur 3937-4425, HUD) passent par un pont
UnitTier (le script répond). **Dépend de l'étape 4** : tant que l'écran de jeu est en C++, la grille est appelée d'un
code C++ et chaque appel traverse le pont (fréquence d'appel non mesurée, voir « Non vérifié »).

### 1.3 `Commands\HOLPlayerOrders.cpp` (317 l. ; en-tête 86 l.)

`IsAccepted`, `SendNonWorkersTo`, `Move`, `Attack`, `AttackBuilding`, `AttackMove`, `Stop`, `ToggleDeploy`, `Gather` 126-158,
`Build` 158-185, `DropOff` 185-219, `Place` 219, `QueueProduction`, `MarketTrade`, `SetRally`, `TakesGarrisonOrder`, `Garrison`,
`UngarrisonAll`, cvar `hol.Garrison.RightClickEnters`. Règles « de chaque slot » (joueur et IA), appelées par ordre, pas par
pas. **MIGRE ~300 l.** vers `Script\Commands\HOLPlayerOrders.as` en 2 lots, par une bibliothèque statique sur
`UHOLCommandSubsystem::Issue*` (lot M1, ~150 l. C++ neuves) ; un pont mince `UHOLPlayerOrdersSubsystem` (~80 l. C++) remplace
la namespace tant que l'IA C++ et les tests C++ l'appellent (`AIOrders` 16 usages, `GarrisonOrder` 13, `Gather` 11, `DeployButton` 6).

### 1.4 `Commands\HOLCommandSubsystem.cpp` (462 l.)

File des ordres lue par `HOLUnitOrderProcessor` (Mass) à chaque pas : `Issue*` (69-414), `Consume*` (78-468), `IsRefused`,
`DropOrdersOf`, `NotifyNavigationChanged/FootprintBlocked`. **RESTE 462 l.** (E : file consommée par processeur Mass ; L).
Seule évolution : l'exposer (lot M1, ajout dans un fichier neuf, ce fichier n'est pas modifié). Candidat conditionnel
~230 l. (`IssueMoveOrder` 182-240, `IssueCombatOrder`, `IssueGather/DropOff/QueuedWork` : plafonds et chaînes) après mesure (M).

### 1.5 `Match\HOLGameState.cpp` (1 641 l.)

| Bloc | Plage | Taille | Verdict |
|---|---|---|---|
| Réplication et `OnRep_*` (7), `GetLifetimeReplicatedProps` (52 propriétés) | 47-80, 142, 194, 330, 1142, 1332, 1507, 1686 | ~130 | RESTE (R) |
| Accesseurs et écriteurs de l'état répliqué (`SetForumState`, `Get/Set/AddSlotStock`, `SlotTechnologies`, `SetHeroSheet`, `Set/StoreMarketBook/DiplomacyBook`, `SetRelay/VolcanoViews`) | 239-345, 849-934, 1311-1400, 1512-1604 | ~350 | RESTE (R, L : fines, l'état est répliqué) |
| Départ, places, nomades (`InitVictory`, `UpdateStartPlaces`, `ApplyNomadSeats` : déjà pont vers `UHOLSpecializationSubsystem`) | 80-210 | ~130 | RESTE (L) |
| Forum : chute, phases, défaite, fin de manche, effondrement (`HandleForumFall`, `HandleKingDeath`, `ApplyPendingDefeats`, `AdvanceForums` 464-563, `SkipForumPhase`, présence 592-640, `DefeatSlot`, `EvaluateMatchEnd`, `CollapseSlot`, `GetWinners/Reason/Mask`) | 384-690, 1192-1260 | ~650 | MIGRE (`Script\Match\HOLMatchFlowBoard.as`, pont UnitTier `UHOLMatchFlowSubsystem`, ~25 UFUNCTION d'accès à l'état, ~150 l. C++). `AdvanceForums` tourne à chaque pas mais sur 8 slots : acceptable en script (à mesurer). Règles de forum déjà en script d'après `HOLForumRules_Test.as` (327 l.) |
| Absence / reconnexion (`KeepAbsentSlot`, `FindSlotOnLogin`, `RetireHolder`, `DefeatExpiredAbsences`, `SimulateReturn`, `GetAbsenceBannerLines`, `DescribeDisconnections`) | 726-1142, 1253-1310, 1712-1750 | ~600 | MIGRE ~300 (règles d'absence) ; RESTE ~300 (L : `SpawnHeldBot`, recherche de `PlayerState`, `FindLiveHolder` : API du moteur de connexion) |
| Pause (`RequestPause/Resume`, `LiftPauseOf`, `GetPauseTimerLeft`, `NotifyPauseChanged`) | 1402-1510 | ~110 | MIGRE ~60 (la décision) ; RESTE ~50 (R) |
| Diplomatie : `ApplyStance`, `ApplyVision`, `ApplyDiplomacyOrders`, `ReportDiplomacy` | 1606-1712 | ~110 | MIGRE (vers la planche de diplomatie en script, règles déjà testées `HOLDiplomacyRules_Test.as`) |

Total GameState : MIGRE ~750 l. (dont ~60 sur la pause), RESTE ~890 l.

### 1.6 `Buildings\HOLConstructionSubsystem.cpp` (588 l.)

| Bloc | Plage | Taille | Verdict |
|---|---|---|---|
| Coque (`DoesSupportWorldType`, `OnWorldBeginPlay`, `Deinitialize`, `FindPlayerState`, `OnStepStarted`) | 40-72, 368-391 | ~100 | RESTE (L) |
| Géométrie de placement (`SnapFootprint` 72, `CheckPlacement` 168, `FindFreeFootprint` 189, `FindRingFootprint`, `FindFarmBlock`, `DescribeRefusal`, `FindGroundZ`) | 72-368 | ~330 | RESTE (E : `CheckPlacement` appelé à chaque image par le fantôme ; `Find*` recherches en spirale de l'IA : M avant tout) |
| `ApplyOrders`, `Place` 420-505, `Cancel`, `DamageBuilding`, `RefundProductionQueue`, `DestroyBuilding` | 391-588 | ~200 | MIGRE ~150 (`Script\Buildings\HOLConstructionBoard.as`, `UHOLConstructionBoard` Abstract) ; RESTE ~50 : le `SpawnActor` du `AHOLBuilding` répliqué (L, +~80 l. C++) |

Les règles (`HOLConstructionRules.cpp` 695 + `HOLConstructionReadLibrary.cpp` 467) sont HORS de la liste mais conditionnent ce
fichier : elles restent la source des règles lues par le fantôme.

### 1.7 `Buildings\HOLProductionSubsystem.cpp` (608 l.)

| Bloc | Plage | Taille | Verdict |
|---|---|---|---|
| Coque, `GetDisplayStep/BuildSteps/GetCost`, `FindPlayerState`, `AddCheatPopulation` | 57-176, 613-... | ~150 | RESTE (L ; `GetCost` lu par la grille) ; ~60 MIGRENT avec la grille |
| Marché et tribut (`EnsureMarketSettings`, `RecoverMarketBook`, `ApplyMarketTrade` 200, `ApplyTribute` 236) | 176-269 | ~95 | MIGRE (règles déjà `HOLMarketRules.as` 245 l. + `HOLMarketBoardScript.as` : reste le câblage ; ~50 l. C++) |
| `ApplyOrders` (file de production, population, palier déjà script) | 269-488 | ~220 | MIGRE ~180 (bas débit, par ordre) ; touche la file répliquée de `AHOLBuilding` : liaison ~60 l. |
| `AdvanceQueues`, `GetBirthLocation` | 488-613 | ~125 | RESTE (E : chaque pas, chaque bâtiment qui produit ; naissance d'unité = Mass ; M) |

Total : MIGRE ~300, RESTE ~300.

### 1.8 `Dev\HOLTestSequence.cpp` (827 l.)

Déclarations de crochets `TFunction` (77-110, liaison), constantes et `FTestSequence` (135-740) : machine à pas pilotée à
chaque image (`FTickableGameObject`), exécute des commandes, capture des images, crochet de sortie du moteur,
`hol.Test.Run`. `StepVerb`, `FindWaitKeyBefore`, `FindPreviousOptionStart`, `IsGameWait`, `ExpandSequences` (~150-200 l.)
sont de la chaîne pure. Verdict : **RESTE ~650 l.** (E : image ; L : capture, sortie du moteur, crochets C++ des autres
modules) ; **MIGRE ≤ 180 l.** (analyse des lignes et expansion), gain faible, risque élevé (toutes les preuves de scénario
`Tools\Scenarios` passent par là). Le chiffre « 915 l. passent en script » de ANALYSE-1116a est trop optimiste.

## 2. Tests C++ « monde sans Mass » liés (par usage grep dans `Source\HeroesOfLiberty\Tests\`)

Usages (nombre d'occurrences) : `HOLPlayerOrders::` AIOrders 16, GarrisonOrder 13, Gather 11, DeployButton 6,
DiplomacyBranch 4, AISlot 3, StartingHero 2 ; `UHOLCommandSubsystem` dans 25 fichiers ; `HOLCommandGrid::` Regicide 6,
StartingHero 6, Victory 3, DamnedWarrior 2, ProductionPopulationEnd 1 ; `AHOLGameState` dans 46 fichiers (Disconnect 35,
AISlot 12, Victory 10, StartingHero 8) ; Construction : Victory 11, Building 9, AIBuildOrders 5 ; Production : AIBuildOrders 3,
Disconnect 5, ProductionPopulationEnd 3 ; contrôleur : Building 4, 5 autres à 1 ; `HOLTestSequence` : CaptureSeq (35 l.),
ScenarioNote (167 l.).

- **Portables tôt (bas risque, API publique, ~2 430 l. C++)** : Building 959, FarmBuilders 374, ProductionPopulationEnd 158,
  UnitTierProduction 201, TierCheat 48, DuelOrder 176, HeroAbilityOrder 92, HeroTalentOrder 121, BuildingRegistry 98, CaptureSeq 35,
  ScenarioNote 167. Les 10 tests de grille déjà en script (`HOLCommandGrid_Test.as`) montrent le patron.
- **Conditionnels (~7 500 l.)** : GarrisonOrder 918, Gather 689, AIOrders 435, AIBuildOrders 209, DeployButton 297,
  DiplomacyBranch 619, DiplomacyAllies 420, HeroReturn 887, Regicide 453, Victory 1 367, Disconnect 1 060, DisconnectDisplay 136,
  StartingHero 757. Ils créent des unités ou une partie complète : portables **seulement après** les ponts GameState (S2-S4) et
  PlayerOrders (P1-P2), et si le monde de test script peut porter le sous-système d'unités sans processeur (non vérifié). Ceux
  qui passent des unités réelles par Mass restent en C++ (D838).
- Les tests de `HOLCommandSubsystem` (file Consume*) restent en C++ tant que le fichier reste en C++.

## 3. Lots (15 min d'agent, ≤ ~300 lignes écrites, sans build ; un build groupé par vague)

Un lot qui retire du C++ « supprime » sans écrire : seul le script et le C++ de liaison comptent dans les 300 lignes.
Dépendances : M1 avant P1, P2, C3-C5 ; S* indépendants ; G1-G2 avant C6-C7 ; étape 4 avant la suppression des `Request<X>` (C11).

| Lot | Vague | Fichiers | C++ | Test rouge d'abord | Preuve à rejouer | Risque joueur |
|---|---|---|---|---|---|---|
| **C1** consoles cheat + victoire (22 cmds) | 1 | `HOLPlayerController.cpp`, `Script\Dev\HOLControllerCommands.as` | non (suppression) | `HOLControllerCommands_Test.as` : chaque commande enregistrée une fois et appelle l'acteur RPC | quick_check tests net ; `rpc_cheats_run.ps1` | nul (dev) |
| **M1** exposition de `UHOLCommandSubsystem` | 1 | `HOLCommandScriptLibrary.h/.cpp` (neufs) | oui ~150 | `HOLCommandLibrary_Test.as` : émettre un `Move` et le lire via `Consume` | quick_check tests | nul (personne ne l'appelle) |
| **R1** marché + tribut | 1 | `HOLProductionSubsystem.cpp`, `HOLMarketBoardScript.as` | oui ~50 (pont) | extension de `HOLMarketApply_Test.as` : le tribut débite et crédite | tests ; `rpc_orders_p3p4_run.ps1` | moyen (ressources) |
| **S1** pause + diplomatie | 1 | `HOLGameState.cpp`, `Script\Match\HOLMatchFlowBoard.as` | oui ~70 | `HOLMatchFlow_Test.as` : pause, reprise, minuterie, stance, vision | tests, net ; `rpc_diplomacy_run.ps1` | moyen (pause visible) |
| **C2** consoles hero/duel/diplo/signal/chat/spé/marché/prod (~25) | 2 | contrôleur + `.as` | non | idem C1 | tests net, `rpc_probe_run.ps1` | nul |
| **P1** ordres a (Move..Stop, Toggle, IsAccepted, SendNonWorkers) | 2 | `HOLPlayerOrders.cpp`, `Script\Commands\HOLPlayerOrders.as` | oui ~80 (pont) | `HOLPlayerOrders_Test.as` portant les cas de DeployButton | tests, netsig3 ; `rpc_orders_p1p2_run.ps1` | élevé (ordres de tous les joueurs et de l'IA) |
| **K1** construction `Place/Cancel/Damage/Refund/Destroy` | 2 | `HOLConstructionSubsystem.cpp`, `Script\Buildings\HOLConstructionBoard.as` | oui ~80 | `HOLConstruction_Test.as` (1 204 l. déjà là) : étendre à Place/Cancel | tests net ; `rpc_orders_p3p4_run.ps1` | élevé (placement) |
| **S2** forum : chute, défaite, fin de manche | 2 | `HOLGameState.cpp`, `HOLMatchFlowBoard.as` | oui ~100 | cas du Regicide/Victory portés en script | tests, net ; `HOLForumRules_Test.as` | élevé (fin de partie) |
| **P2** ordres b (Gather, Build, DropOff, Place, Garrison, Rally, Market) | 3 | `HOLPlayerOrders.cpp` + `.as` | non | portage de GarrisonOrder / Gather | tests, netsig3 ; `rpc_orders_p3p4_run.ps1` | élevé |
| **C3** `Do<X>` des 16 ordres vers `HOLRpcPlayer.as` | 3 | contrôleur, `HOLRpcPlayer.as` | non | `HOLRpcPlayer_Test.as` : le corps serveur applique l'ordre | net ; `rpc_orders_p1p2_run.ps1`, `p3p4` | élevé |
| **G1** grille a (55-240) | 3 | `HOLCommandGrid.cpp`, `HOLCommandGridRules.as` | oui ~60 (structures + pont) | 5 tests restants de la grille | tests ; `tier_grid_client_run.ps1` | moyen (grille grisée) |
| **R2** `ApplyOrders` production | 3 | `HOLProductionSubsystem.cpp` + `.as` | oui ~60 | `HOLProductionQueue_Test.as` (166 l.) étendu | tests net ; ProductionPopulationEnd porté | élevé (file de production) |
| **S3** forum : phases, présence | 3 | `HOLGameState.cpp` | non | cas AdvanceForums/Skip/Presence | tests, net | élevé |
| **G2** grille b (238-531) | 4 | grille + `.as` | non | idem G1 | tests ; `tier_grid_client_run.ps1` | moyen |
| **C4** `Do` cheats (~20, #if !SHIPPING) | 4 | contrôleur + `HOLRpcPlayer.as` | oui ~100 (WoundUnits, présence) | triches rouges côté serveur | `rpc_cheats_run.ps1` | nul |
| **S4** absence / reconnexion (règles) | 4 | `HOLGameState.cpp` | non | cas Disconnect portés | tests, net | élevé |
| **R3** reste production (~60) | 5 | production | non | - | tests | bas |
| **C5** `Do` économie/héros/duel/spé/signal/chat | 5 | contrôleur | non | - | `rpc_diplomacy_run.ps1`, `rpc_probe_types_run.ps1` | moyen |
| **C6** grille dans le contrôleur a (3937-4210) | 6 | contrôleur | non | `HOLCommandGrid_Test.as` | `tier_grid_client_run.ps1` | moyen |
| **C7** `ExecuteGridCommand` + touches grille | 7 | contrôleur | non | idem | idem | élevé |
| **C8** sous-groupes et groupes de contrôle (~400) | 8 | contrôleur, `HOLSelectionRules.as` | oui ~80 | `HOLSelectionRules_Test.as` | quick_check tests net | moyen (touches 1-9) |
| **C9, C10** pilotes de test (si question 1 = migrer) | 9, 10 | contrôleur + lib | oui ~250 | tous les noms `hol.*` présents | scénarios `Tools\Scenarios` | nul |
| **C11** suppression des `Request<X>` (après l'étape 4) | 9 ou 11 | contrôleur | non | tests rouges des appelants | net, `rpc_routes_run.ps1`, `rpc_routes_seats_run.ps1` | moyen |
| D1 (option) analyse de `HOLTestSequence` | 6 | `HOLTestSequence.cpp` | non | `CaptureSeq`/`ScenarioNote` portés | tous scénarios | élevé pour 180 l. |
| **T1-T8** tests portables tôt (voir 2) | 2-6, un fichier de tests par lot, en remplissage | `Script\Tests\*` | non | le test lui-même est le rouge | tests | nul |
| **T9-T20** tests conditionnels (voir 2) | 6-10 | idem | non | idem | tests, net | nul |

Le contrôleur est le goulot : 11 lots en série (C1..C11), donc **10 à 11 vagues** si « jamais deux lots sur le même fichier
dans la même vague » est tenu à la lettre. Ramené à ~6 vagues si deux lots sur des plages disjointes d'un même fichier
sont permis (fusion git sans conflit, la plupart des lots ne font que SUPPRIMER leur plage ; question 3).

## 4. Total honnête (~11 700 lignes : 6 741 + 517 + 317 + 462 + 1 641 + 588 + 608 + 827 = 11 701)

| Fichier | MIGRE (ferme) | MIGRE (si pilotes de test) | RESTE |
|---|---|---|---|
| `HOLPlayerController.cpp` | ~3 300 | ~4 500 | ~2 240 à 3 440 |
| `HOLCommandGrid.cpp` | 480 | 480 | 37 |
| `HOLPlayerOrders.cpp` | 300 | 300 | 17 |
| `HOLCommandSubsystem.cpp` | 0 | 0 | 462 |
| `HOLGameState.cpp` | 750 | 750 | 890 |
| `HOLConstructionSubsystem.cpp` | 150 | 150 | 438 |
| `HOLProductionSubsystem.cpp` | 300 | 300 | 308 |
| `HOLTestSequence.cpp` | 0 | 180 | 827 à 647 |
| **Total** | **~5 280 (45 %)** | **~6 660 (57 %)** | **~6 420 à 5 040** |

Le C++ de liaison à AJOUTER est de ~600-800 lignes (M1 150, ponts S/K/R/P ~330, grille 60-100, contrôleur 100-330), donc le C++
net retiré est de ~4 600 (ferme) à ~5 900 lignes. Le script écrit est de ~4 500-5 800 lignes, plus ~1 700 de tests portables
et jusqu'à ~5 500 de tests conditionnels.

**Lots : ~45** (25 de code : C1-C11, M1, P1-P2, G1-G2, K1, R1-R3, S1-S4, D1 ; 8 + 12 de tests). **Vagues : 10 à 11** (6 si lots à plages disjointes permis).
Conforme à l'estimation de l'étape 5 de ANALYSE-1116a (40 à 75 lots), mais la part qui migre est ~45-57 % des 11 700, pas ~100 % :
~15 000 « du volume C++ » de ANALYSE-1116a comptait aussi les règles de construction et les en-têtes.

## 5. Première vague (4 lots parallèles, 1 seul build groupé)

Lots : **C1, M1, R1, S1** (aucun fichier commun : `HOLPlayerController.cpp` / `HOLCommandScriptLibrary.*` neufs /
`HOLProductionSubsystem.cpp` / `HOLGameState.cpp`). Les liaisons C++ de M1, R1, S1 sont groupées dans UN build en fin de vague.

Worktrees conseillés (un par lot, les huit branches sont à `d4731a0`, main est à 84a42b2 : fusionner `main` d'abord) :
- C1 -> **Arwen** (d751-root)
- M1 -> **Celeborn** (hero-j8-close)
- R1 -> **Galadriel** (galadriel-tab ; elle a déjà l'historique de la grille et des bibliothèques de lecture)
- S1 -> **Glorfindel** (net-playtest ; adapté à la preuve réseau)
- Sauron en réserve (visual). À ÉVITER : Elrond (tiers), Haldir (maps), Faramir (fog) : occupés par d'autres chantiers.

## 6. Décisions à demander à l'utilisateur

1. Les ~50 commandes console « pilote de test » (`hol.Select.*`, `hol.Units.*`, `hol.Build.*`) restent-elles en C++ ? Les migrer coûte ~250 l. de
   liaison pour ~1 240 l. retirées et 2 lots ; les commandes cheat / victoire / héros / diplomatie (~45) migrent d'office.
2. `HOLTestSequence` (827 l.) reste-t-il en C++ au titre de la liaison / de l'image (recommandé : oui, sauf ~180 l. d'analyse) ?
3. Autorise-t-on deux lots sur des plages disjointes de `HOLPlayerController.cpp` dans la même vague (6 vagues au lieu de 10-11) ?
   Et les tests de GameState/Victory/Disconnect (7 500 l.) : visés par D838 (restent C++) ou à porter ?

## 7. Non vérifié

- Comment les `Do<X>` sont appelés aujourd'hui par l'acteur script (18 UFUNCTION seulement dans l'en-tête du contrôleur) : le chemin
  `CallN`/`CallEx` n'a pas été lu en entier ; les verdicts « MIGRE » des `Do<X>` supposent que le corps serveur peut devenir un
  corps d'acteur script.
- Le code des notes `HALDIR-1117n/o/q`, `ELROND-1117*` est marqué « NON COMPILÉ » dans le contrôleur : les plages mesurées peuvent bouger au prochain build.
- La fréquence d'appel de `GetGridCommands` / `HOLCommandGrid::*` par le HUD (par image ? sur changement de sélection ?) : décide si le pont de
  grille est acceptable avant l'étape 4.
- Le classement des 106 commandes console est fait sur les NOMS, pas sur chaque corps ; deux commandes (lignes 1497, 1516, 1835,
  1851, 1865 : noms non extraits) non classées.
- Les corps des tests C++ n'ont pas été lus : « portable » vient de la taille et des usages grep, pas d'une lecture ; savoir si le monde
  de test script porte `UHOLUnitSubsystem` sans Mass n'est pas vérifié.
- Les règles de forum « déjà en script » reposent sur le nom `HOLForumRules_Test.as`, pas sur une lecture de la règle ; idem la
  diplomatie.
- Aucune mesure de performance : `AdvanceQueues`, `AdvanceForums`, `Find*Footprint` classés sur le raisonnement.
- Les lignes `FHOLGridContext` / `FHOLGridCommand` : exposition actuelle non vérifiée.
- Les noms de preuves `rpc_*.ps1` rattachés à chaque lot sont déduits du nom, pas du contenu des scripts ; `quick_check -Parts` : seuls
  les noms donnés par l'énoncé sont repris.
