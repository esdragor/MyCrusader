# ELROND-1119u : reste du lot 2, lots 3 et 4 de la bibliotheque de monde de test (liaison C++ seule)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche `tiers`. Commit 8376a8b (4 fichiers, 819 lignes ajoutees, aucune supprimee). NON COMPILE, NON EXECUTE. Aucun fichier `Script\`, `Match\HOLGameState.h` ni en-tete de sous-systeme touche. Pas de push, pas de merge main.

Impact joueur : aucun (outillage de test). Plan : liaison statique minimale pour porter `HOLDisconnectTests.cpp` puis `HOLAIBuildOrdersTests.cpp` en script.

## Fonctions exposees (nom script probable)

Le plugin retire le U et Library : `UHOLGameStateTestLibrary` -> `HOLGameStateTest::`, `UHOLTestWorldLibrary` -> `HOLTestWorld::`. Les `UHOLTestWorldHandle*` sont ceux de `CreateTestWorld`. Toute heure `Now` et fenetre sont en double ; fenetre <= 0 = `FHOLDisconnectRules::DefaultWindowSeconds`.

### `HOLGameStateTest::` (Tests\HOLGameStateTestLibrary.h/.cpp), ajouts
- `FHOLTestForumState` (USTRUCT neuf, script `HOLTestForumState` : Falls, Phase, RuinStepsLeft, HoldSteps, Hold, HolderSlot, bDown) ; `GetForumState(State, Slot)`
- `GetPlayerStateOfSlot(State, Slot)` (PlayerArray, PlayerIndex)
- `GetAbsentNotices(State)` -> `TArray<FString>`, une ligne par avis, champs separes par tab : Slot, Name, EndsAt, BackAt, WindowSeconds (%.3f) ; `CountAbsentNotices(State)` ; `HasAbsentNotice(State, Slot, bBack)` (BackAt <= 0 si bBack faux, > 0 si vrai)
- `GetAbsenceBannerLines(State, Now)` -> `TArray<FString>` ; `GetWindowClock(State, Now)` ; `GetSlotsInUse(State, Except)` (Except peut etre null) ; `DefeatExpiredAbsences(State, Now, Window)` -> `TArray<int32>` ; `DescribeDisconnections(State, Now, Window)` -> `TArray<FString>`
- `SimulateReturn(State, Slot, Starting, Now)`
- `FindSlotOnLogin(State, Json, Name, TakenSlots)` ; `FindSlotForLogin(State, Json, Name, Logging)` + `FindReplacedForLogin(...)` (le OutReplaced du meme appel, deux fonctions car un `AHOLPlayerState*&` sortant est risque en script) ; `RetireHolder(State, Old, Now, Window)`
- Pauses : `RequestPause(State, Requester, Now)`, `RequestResume(...)`, `LiftPauseOf(State, Slot, Now)`, `IsMatchPaused(State)`, `GetPausesLeft(State)` (un seul compteur pour la partie, pas de slot : le plan se trompait)
- `GetResources(Player)`, `SetResources(Player, Resources)` (sur `AHOLPlayerState`, ou ces deux methodes vivent), `AddTechnology(Player, Name)`, `GetPlayerHeroXP(Player)`

### `HOLTestWorld::` (Tests\HOLTestWorldLibrary.h/.cpp), ajouts
- `GetWorldObject(Handle)` -> `UObject*` : le contexte de monde que veulent les statiques a `const UObject*` (`UHOLAIOrderLibrary::IssueAIProduceOrder`, `CheckAIPlacement`...)
- `RunConsoleCommand(Handle, Command)` (`ProcessUserConsoleInput(.., *GLog, World)`)
- `GetWorldSubsystem(Handle, TSubclassOf<UWorldSubsystem>)` -> `UWorldSubsystem*` (a caster en script)
- `InitProductionSubsystem`, `InitSourceSubsystem`, `InitConstructionSubsystem` (appellent `OnWorldBeginPlay`, une seule fois par monde : sinon double abonnement au pas)
- `GetProductionCostResources(Handle, Type, PlayerIndex)`, `GetProductionCostSteps(...)`, `EnqueueProduction(Handle, Building, Technology, Slot, Payer, FallbackStock, MaxQueue)` (la lambda `Queue` des tests : GetCost, TryEnqueue sur `GetMutableProductionQueue`, Payer recoit le stock)
- Heros : `SetHeroXP(Handle, Slot, XP)`, `GetHeroSheetXP(Handle, Slot)`, `HeroSheetEqualsPlayer(Handle, Slot, Player)`
- `FindHeldBot(Handle, Slot)`, `AddSlotStock(Handle, Slot, Gain)` (bool ; le stock resultant se lit par `GetSlotResources`), `SpawnAIPlayerStates(Handle, Json)` (stock de depart = `UHOLMatchSettings::StartingResources`)
- Pas : `AdvanceSteps(Handle, N)` diffuse `GetOnStepStarted()` (c'est ce que font les tests, pas `OnStepEnded` comme disait le plan), compteur 1.. par monde ; `AdvanceStepsWithEnd` ajoute `GetOnStepEnded()` apres chaque pas
- Ressources par slot : `GetSlotResources(Handle, Slot)`, `SetSlotResources(Handle, Slot, Resources)` (joueur de ce PlayerIndex dans le GameState du monde)
- `CountBuildings(Handle, Type, Owner)`

## Les 15 tests de HOLDisconnectTests.cpp : portables maintenant ?
Lecture par ensemble des appels de chaque test (non compile : a confirmer au premier chargement).
1. DisconnectRules : NON. Statiques de `FHOLDisconnectRules` sur `FHOLAbsentSlot` (FindAbsent, GetExpired, GetSecondsLeft, GetStockOnTaking, IsWindowOver, KeepsSlot, MakeDefault) : enveloppe de regles absente.
2. DisconnectGameState : OUI (config JSON, KeepAbsentSlot, DefeatExpiredAbsences, FindSlotOnLogin, SetResources). Manque seulement `GetReasonName` (comparer l'int).
3. DisconnectGains : NON. Methodes de `AHOLBuilding` (Setup 3 args, GetConstructionState/Work, GetTotalWork, DepositLoad, `GetCancelRefund`, TryEnqueue sur `FHOLProductionItem`) non exposees.
4. DisconnectBanner : PARTIEL. Il manque `FormatAbsentLine` / `FormatBackLine` (statiques de regles) ; reste OK (bannieres, avis, `IsPropertyReplicated`).
5. DisconnectHeroSheet : OUI (SetHeroXP, GetHeroSheetXP, HeroSheetEqualsPlayer, GetPlayerStateOfSlot a la place de FindPlayerState).
6. DisconnectTechnologies : NON. Capture de journal (`FOutputDevice`), cvar (`FindConsoleVariable`), `GetTechnologyBonusPercent`, `GetGatherStat`, `FindTechnology`, lecture des files du batiment.
7. DisconnectHeld : NON. Cerveau IA (`ThinkNow`, `GetThinkingSlots`, `IsHeld`, `UHOLAIToolsLibrary::GetSlotLevel`).
8. DisconnectHeldStock : NON pour l'instant. Manque `AHOLGameState::GetSlotStock` (statique, `OutStock`), lecture de `GetProductionQueue` et `GetTechnologies`, `IsABot` ; `IssueAIProduceOrder` est faisable (GetWorldObject).
9. DisconnectReplace : PARTIEL. FindSlotForLogin / FindReplacedForLogin / RetireHolder OK ; manquent `FindLiveHolder` (statique), `IsOnlyASpectator`, lecture de la file de production.
10. DisconnectPause : OUI.
11. DisconnectBannerClock : OUI (`IsPropertyReplicated` existe deja).
12. DisconnectFakeBack : PARTIEL, presque. Manquent `GetSlotStock` (statique) et `IsABot` / `GetPlayerName` (a verifier : `GetPlayerName` est probablement deja visible, `IsABot` non).
13. DisconnectReportMode : OUI.
14. DisconnectLateReturn : NON. Cerveau IA comme le 7.
15. DisconnectWindowEnd : NON. Regles pures (GetWindowEnd, MayReturn, KeepsSlot).
Bilan : OUI 5 (2, 5, 10, 11, 13), PARTIEL 3 (4, 9, 12), NON 7.

## Reste
- `AHOLGameState::GetSlotStock(World, Slot, OutStock)` et `FindLiveHolder(World, Slot, Except)` (statiques, 2 enveloppes de 6 lignes dans `HOLTestWorldLibrary`), `IsOnlyASpectator`, `IsABot`.
- Enveloppe de regles `FHOLDisconnectRules` (tests 1, 4, 15) : un petit `USTRUCT` ou des fonctions plates, comme `FormatSeatReport`.
- Lectures de file de production d'un batiment, de technologies d'un joueur, constructions de `AHOLBuilding` (tests 3, 6, 8, 9).
- Cerveau IA (`UHOLAIBrain::ThinkNow`, `GetThinkingSlots`, `IsHeld`) pour les tests 7 et 14 : probablement un `UFUNCTION` direct sur `UHOLAIBrain`, hors bibliotheque de test.
- `HOLAIBuildOrdersTests.cpp` : le monde se monte desormais avec CreateTestWorld + SpawnGameState(1 humain, 1 IA) ou JSON + `SpawnAIPlayerStates` + SpawnBuilding(MainCentre) + les trois `Init*Subsystem` + `AdvanceSteps` ; les appels `UHOLAIOrderLibrary::*` passent par `GetWorldObject`. Non verifie : `SnapFootprint`, `HOLConstruction::TryPay`, `Overlaps` (statiques de regles non exposes), `GetSlotResources`/`SetSlotResources` couvrent `AIState->GetResources/SetResources`. Au moins Placement et Place sont portables sous reserve de ces trois statiques.

## Pour compiler et verifier (session principale)
Un build groupe (UHT regenere) : `<worktree>\Tools\build.ps1 -LiveCoding=0 -Wait`, puis charger le script. Les 4 fichiers ne se compilent qu'avec WITH_DEV_AUTOMATION_TESTS pour les corps.

## Champs
BUILDS_DEMANDES: 1 (lot C++ groupe : 4 fichiers modifies, a coupler avec le build du lot 2 qui a compile)
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 0
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: 0 (liaison seule, 819 lignes C++ ajoutees)
TYPES_A_EXPOSER: FHOLTestForumState (fait), FHOLDisconnectRules / FHOLAbsentSlot (rules wrapper), UHOLAIBrain (ThinkNow, GetThinkingSlots, IsHeld), AHOLBuilding (construction et file), FHOLProductionItem
NON VERIFIE : compilation C++ et UHT ; `UWorld::GetSubsystemBase(TSubclassOf)` ; `FHOLHeroPrivateSheet::operator==` (deduit du test C++ ligne 380) ; `AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates` et `AHOLGameState::FindHeldBot` / `AddSlotStock` publics (utilises par les tests) ; `OnWorldBeginPlay` public des quatre sous-systemes (utilise par les tests, Hero n'est pas appele) ; chargement script des nouveaux noms ; fenetre par defaut identique a celle des tests (ils passent 180).
Anomalies : budget d'appels atteint ; fin de ligne : les .h/.cpp de `HOLTestWorldLibrary` sont en CRLF et le sont restes (diff : insertions seules) ; les deux fichiers `HOLGameStateTestLibrary` sont en LF (git avertit, sans effet). Un premier essai de script python en heredoc a echoue sans rien ecrire.
merge main non fait. PROCESSUS_DETACHES: aucun lance.
