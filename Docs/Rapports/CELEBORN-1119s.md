# CELEBORN-1119s : lot 5 de la bibliotheque de monde de test (C++, liaison pour les tests script, D839)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`, branche `test-world-5` (depuis `integ-15`), commit `7cc5478`, 4 fichiers (+466 lignes), NON COMPILE, aucun test lance. Pas de push, pas de merge. En-tetes du jeu intacts (`HOLGameState.h` etc.). Impact joueur : aucun (outillage de test).

Fichiers : `Source\HeroesOfLiberty\Tests\HOLGameStateTestLibrary.h/.cpp`, `HOLTestWorldLibrary.h/.cpp` (corps sous `#if WITH_DEV_AUTOMATION_TESTS`, meme patron). Chaque signature reelle a ete lue par Grep avant l'enveloppe.

## Fonctions exposees (nom SCRIPT probable)

`HOLGameStateTest::` (UHOLGameStateTestLibrary, sans monde) :
- `GetReasonName(int Reason) -> FString` (AHOLGameState::GetReasonName ; 5 = "Disconnected")
- `IsABot(Player)`, `IsOnlyASpectator(Player)` (methodes du moteur APlayerState, pas des membres du jeu)
- `HasTechnology(Player, FName)`, `GetTechnologyCount(Player)` (GetTechnologies().Has / .Done.Num())
- `GetGatherBonusPercent(Player, int Resource)` (GetTechnologyBonusPercent(GetGatherStat(EHOLResource)))
- `IsTechnologyInTable(FName)` (UHOLTechnologySettings::FindTechnology != null)
- `FormatAbsentLine(Name, SecondsLeft, WindowSeconds) -> FString`, `FormatBackLine(Name)`, `FormatSeatReport(Slot, Name, State, SecondsLeft, AIMode)` (State 0 Present, 1 AbsentAIHolds, 2 AIHolds, 3 Defeated ; AIMode 0 None, 1 Hold, 2 Full)
- Regles : `RulesKeepsSlot(Json, Slot, bWaiting, bOver, bDefeated)`, `RulesIsWindowOver(LeftAt, Now, Window)`, `RulesGetSecondsLeft`, `RulesGetWindowEnd(Json, Slot) -> int` (0 Defeat, 1 AIHolds), `RulesMayReturn(State, bOver)`, `RulesFindAbsent(TArray<int> Slots, Slot)`, `RulesGetExpired(TArray<int> Slots, TArray<double> LeftAts, Now, Window)`, `RulesGetStockOnTaking(bHasAbsent, AbsentStock, Starting)` (la liste FHOLAbsentSlot est reconstruite depuis des tableaux paralleles : seuls Slot, LeftAt et Resources sont portes)
- `SetPlayerMatchSlot(Player, Slot, Color)` (absent de la bibliotheque avant : ajoute)

`HOLTestWorld::` (UHOLTestWorldLibrary, avec le handle du monde) :
- `SpawnPlayerStateNoSlot(TestWorld, Name)` : variante qui ne pose PAS le slot (l'existante `SpawnPlayerState` est inchangee). Anomalie confirmee a la lecture : `HOLTestWorldLibrary.cpp:150-164` appelle `SetMatchSlot(Slot, Color)` des le spawn, les tests C++ le font plus tard ; l'ordre `SpawnPlayerStateNoSlot` puis `SetPlayerMatchSlot` rend le test fidele (plus besoin de spawner BobAgain apres la destruction de Bob).
- `FindLiveHolder(TestWorld, Slot, Except)`, `HasSlotStock(TestWorld, Slot) -> bool` + `GetSlotStock(TestWorld, Slot) -> FHOLResources` (la statique C++ prend un `FHOLResources&` : coupee en deux)
- `GetAIBrain(TestWorld) -> UHOLAIBrainSubsystem` (UHOLAIBrainSubsystem::Find). `ThinkNow`, `IsHeld`, `GetThinkingSlots`, `SetHeld`, `GetAISlots` sont DEJA des UFUNCTION de la base (pas d'enveloppe necessaire).
- `GetProductionQueueCount(Building)`, `GetProductionQueueName(Building, Index)`, `IsProductionQueueItemStarted(Building, Index)`
- `GetBuildingConstructionState(Building) -> int`, `GetBuildingConstructionWork`, `GetBuildingTotalWork` (int64), `GetBuildingCancelRefund(Building) -> FHOLResources` (HOLConstruction::GetCancelRefund sur la ligne de type)
- `DepositLoad(TestWorld, Owner, int Resource, int Amount) -> int` (deposee ; la charge restante = Amount moins ce nombre ; le stock de sortie n'est pas expose)

## Test par test (ELROND-1119v) : portable maintenant ?

| # | Test | Portable | Ce qui manque / remarque |
|---|---|---|---|
| 1 | Rules | oui | `FHOLAbsentSlot::MakeDefault` cite dans ELROND-1119v : non lu (je n'ai trouve aucune statique de ce nom dans `HOLDisconnectRules.h`, c'est sans doute un utilitaire du test) |
| 2 | GameState | oui | `GetReasonName(5)` |
| 3 | Gains | oui, a verifier | SpawnBuilding (etat Foundation), EnqueueProduction (Peasant par la file du centre), GetProductionCostResources, nouvelles lectures de construction, `GetBuildingCancelRefund`, `DepositLoad`, `DestroyBuilding` existant. Non verifie : `DepositLoad` suppose un centre du proprietaire dans le monde |
| 4 | Banner | oui | `FormatAbsentLine/FormatBackLine` : les 7 assertions directes |
| 5 | HeroSheet | oui | `IsABot` |
| 6 | Technologies | en partie | manque : capture de journal (`FCaptureLog`, 2 assertions "researched PloughI" comptees) ; `UHOLAITechnologyLibrary::ReadAITechnologiesDone` (parametre de sortie, visibilite script non verifiee ; a defaut `HasTechnology` sur le holder). Le reste (cvar, `GetGatherBonusPercent`, `IsTechnologyInTable`, files, `HasTechnology`) est la |
| 7 | Held | oui sous reserve | `GetAIBrain` ; NON VERIFIE que le cerveau (sous-systeme script, monde de jeu) existe dans le monde de test ; `GetSlotLevel` (UHOLAIToolsLibrary) non regarde |
| 8 | HeldStock | oui | `HasSlotStock/GetSlotStock`, `IsABot`, queue, technologies |
| 9 | Replace | oui | `FindLiveHolder`, `IsOnlyASpectator`, queue, `HasTechnology` |
| 10 | Pause | deja porte | |
| 11 | BannerClock | deja porte | |
| 12 | FakeBack | oui | `GetSlotStock`, `IsABot` |
| 13 | ReportMode | deja porte | `FormatSeatReport` permet aussi des assertions directes |
| 14 | LateReturn | oui sous reserve | meme reserve que 7 (cerveau) |
| 15 | WindowEnd | oui | `RulesGetWindowEnd`, `RulesMayReturn`, `RulesKeepsSlot` |

## Reste (non fait)

- `HOLAIBuildOrdersTests.cpp` : NON LU (limite d'appels atteinte), rien d'expose pour lui hors `GetAIBrain`. A faire par le lot suivant.
- Capture de journal (FCaptureLog sur GLog) : a exposer (enveloppe : demarrer / arreter / compter les lignes contenant X et Y).
- Aucun membre prive ou protege rencontre : tout ce qui a ete enveloppe est public.

## Commande pour la session principale

Build du lot (un seul) : `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait`, puis `quick_check` sur les parties script et Tests ; ensuite le portage des tests Disconnect par l'agent suivant.

## Champs

BUILDS_DEMANDES: 1 (lot C++ de la bibliotheque, non lance)
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 0
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: 0 (liaison pure, +466 lignes C++)
TYPES_A_EXPOSER: capture de journal ; ReadAITechnologiesDone (a verifier) ; ce que demande HOLAIBuildOrdersTests.cpp (non lu)
TESTS_TAUTOLOGIQUES: 0
EDITS_REFUSES: 0
NON VERIFIE : rien compile ; `EHOLResource`, `FHOLGatherLoad::Amount` et `GetTestWorld` utilises comme dans les tests C++ existants ; les noms de classe `UHOLTechnologySettings` et l'include `Technologies/HOLTechnologyTypes.h` pris des lignes du test et de la grep, pas d'un build ; `APlayerState::IsABot` et `IsOnlyASpectator` supposes publics (moteur).
Anomalies : SpawnPlayerState pose le slot au spawn (confirme, variante ajoutee). Fichiers CRLF/LF : avertissement git sans effet. Fichiers non suivis `Tools\Scenarios\c28_*`, `c31_*` laisses. merge main non fait.
PROCESSUS_DETACHES: aucun (aucun processus lance)
