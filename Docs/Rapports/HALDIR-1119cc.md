# HALDIR-1119cc : C8-0, par quel crochet le contrôleur demande-t-il au script ?

Code lu : `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` (`Source\`, `Script\`). Lecture seule, 17:02 -> 17:05.
Abréviations : CGB = `Source\HeroesOfLiberty\Player\HOLCommandGridBoard.cpp`, PC = `...\Player\HOLPlayerController.cpp`.

## Conclusion
Crochet existant = `UHOLCommandGridBoard` (sous-système de monde, sous-classe script, `FindAny`, `Ask*`). Le lot C8-2 du plan
(crochet B `ScriptOnGroupKey`) n'est PAS nécessaire : il se réduit à ~6 fonctions `Ask*` sur `UHOLCommandGridBoard`
(GroupKeyAction, IsDoublePress, MergeUnits, RemoveUnits, KeepKnown, NextSubgroup), toujours un lot C++ (un build).

## 1. Mécanismes « C++ demande au script » existants
- MESURÉ : la planche de la grille est un sous-système de monde. `ShouldCreateSubsystem` refuse la classe abstraite C++ et
  les classes script remplacées (CGB:272-280). `DoesSupportWorldType` = `Game` ou `PIE` (CGB:282-285), aucun filtre
  serveur/client. Le script en fournit la sous-classe : `class UHOLCommandGridBoardScript : UHOLCommandGridBoard`
  (`Script\Player\HOLCommandGridBoard.as:10`), méthodes `UFUNCTION(BlueprintOverride) ...Answer`.
- MESURÉ : le C++ la trouve par `FindAny()` (CGB:32-53) : thread de jeu seulement, parcourt `GEngine->GetWorldContexts()`,
  prend `World->GetSubsystemArrayCopy<UHOLCommandGridBoard>()[0]`.
- MESURÉ : `AskX(...)` fait `FindAny()`, rend `false` sans planche (CGB:95-102, 117-121), sinon appelle `XAnswer` et convertit
  la vue en règle. L'appelant écrit `Ask(...) ? Answer : XNative(...)` (`Player\HOLCommandGrid.cpp:60, 563, 527`). Sans planche = corps C++ « Native ».
- MESURÉ : sans script, les `*Answer_Implementation` rendent des valeurs vides (CGB:287-302, 367).
- MESURÉ : mêmes patrons, même schéma, autres planches : `UHOLMatchFlowSubsystem::Find` + `Ask*` (`Match\HOLMatchFlowSubsystem.cpp:7-75`,
  appelés par `HOLGameState.cpp:407, 450, 690, 770, 1513...`) ; `UHOLPlayerOrdersBoard::FindAny` (`Commands\HOLPlayerOrdersBoard.cpp:19`),
  doc `.h:16` : « sans planche, `HOLPlayerOrders.cpp` REFUSE l'ordre et le journalise, le corps C++ n'existe plus » ; `HOLConstructionBoard.cpp:74`,
  `HOLProductionBoard.cpp:64` idem. Le contrôleur appelle déjà `HOLPlayerOrders::Move/Attack/...` (PC:2866, 4451-4522) donc ces planches.
- MESURÉ : `PC` appelle déjà la planche de la grille indirectement : `GetSubgroups` -> `HOLCommandGrid::SplitSubgroups` (PC:1597),
  `GetUnitSubgroups` -> idem (PC:1707) ; `SplitSubgroups` fait `AskSplitSubgroups` (`HOLCommandGrid.cpp:563`).
- ESTIMÉ : le monde client a sa planche (sous-système Game/PIE, pas de filtre réseau ; l'en-tête script dit « a client runs it », `.as:5`).

## 2. Touches 1-9 et Tab : décision locale ?
- MESURÉ : liaison dans `SetupInputComponent` : `NextSubgroup` -> `HandleSubgroupKey` (PC:858), `IA_Group_%d` -> `HandleGroupKey` (PC:869-871).
- MESURÉ : `HandleGroupKey` (PC:5014-5039) lit `IsCtrlHeld/IsShiftHeld` (PC:5041-5049, `IsInputKeyDown`), puis `Remove/Add/SelectControlGroup`
  ; état = `ControlGroups`, `LastGroupPressed`, `LastGroupPressTime` (PC:5104-5181) ; double appui `<= ControlGroupDoublePress` (PC:5178).
  `HandleSubgroupKey` (PC:4987) -> `CycleSubgroup` (PC:1612-1637) : `(Active+1) % Num` (PC:1625, 1633).
- MESURÉ : aucun RPC dans ces fonctions (lues en entier : PC:1612-1637, 4987-5039, 5104-5181 jusqu'à 5213) : sélection locale au client.
- ESTIMÉ : le C++ peut donc interroger la planche locale telle quelle (même chemin que `SplitSubgroups` déjà appelé dans `GetSubgroups`).
- MESURÉ : la logique est déjà découpée en décisions pures (modificateurs -> action, fenêtre de double appui, fusion plafonnée, retrait,
  filtre des morts, modulo) et effets (écrire `ControlGroups`, `OnSelectionChanged.Broadcast()` PC:1636, caméra, `SetUnitSelection`).

## 3. Fonctions appelées à chaque image
- MESURÉ : `IsInActiveSubgroup` (PC:1661) : définie, aucun appelant dans `Source\` vu par ma recherche (liste coupée à 50 lignes, voir NON VÉRIFIÉ).
  Elle appelle `GetActiveSubgroup` -> `GetSubgroups` -> `SplitSubgroups` (reconstruit tout à chaque appel, PC:1586-1609).
- MESURÉ : appelants de `GetSubgroups/GetActiveSubgroup/GetUnitSubgroups` : lecture de la sélection par le HUD (`HOLSelectionChipsReadLibrary.cpp:53-60, 118, 146-149`),
  marqueurs de sélection (`HOLSelectionMarkers.cpp:1313`), minimap (`HOLMinimapView.cpp:646, 656` via `GetCommandUnits`) et le contrôleur (PC:1378, 2109, 2238, 2292, 2348, 2643...).
- ESTIMÉ : fréquence « par image » pour HUD et marqueurs (rendu), « par ordre » pour `GetCommandUnits` ; D839 : elles restent C++ (coût d'un appel script
  par image + par unité non justifié). Note : `GetUnitSubgroups` termine déjà par `SplitSubgroups`, donc par un `Ask` script par appel (déjà le cas aujourd'hui).

## 4. Réponse
- Crochet existant = `UHOLCommandGridBoard`. C8-1 (règles script) : à faire. C8-2 : NON comme écrit (pas de crochet B) ; il devient « ~6 `Ask*`
  + 6 `BlueprintImplementableEvent` + `_Implementation` par défaut + repli Native » dans CGB et `HOLCommandGridBoard.h` (même famille que `AskNextInCycle`, CGB:190).
  C8-3 : branche les 6 appels dans `HandleGroupKey`, `AddToControlGroup`, `RemoveFromControlGroup`, `SelectControlGroup`, `CycleSubgroup`. Test d'accord obligatoire
  (D839 : une source) ; `IsCtrlHeld/IsShiftHeld`, `GetRealTimeSeconds`, `ControlGroups` restent C++.

## NON VÉRIFIÉ
1. `HOLCommandGridBoard.h` : base exacte (`UWorldSubsystem` ?), non lu ; je déduis du `.cpp` (`GetSubsystemArrayCopy`).
2. `.as:5` dit « NOT COMPILED at the time of writing » : état actuel de la compilation de la planche et présence des méthodes `SplitSubgroupsAnswer`/`NextInCycleAnswer` côté script non lus.
3. Que la planche existe bien sur un client en réseau réel (sous-système Game/PIE : lu ; vérifié par un lancement : non).
4. Liste des appelants de `IsInActiveSubgroup` : recherche limitée à 50 lignes, le `.h:199` seul vu en plus de la définition ; appelants Blueprint/UMG non cherchés.
5. Fréquence réelle par image du HUD, des marqueurs, de la minimap : non lue (Tick/Paint), c'est une déduction.
6. `SetUnitSelection` / `SelectControlGroup` après PC:5213 : RPC éventuel non lu.
7. Pas de test C++ sur ces fonctions : non cherché.
