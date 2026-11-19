# CELEBORN-1119cn : classement de HOLVictoryTests.cpp + retrait de Victory.GameState

NON COMPILE, NON TESTE. Aucun build, aucun quick_check. Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`, branche `test-world-10` (partie de integ-21). Commit `c8c953e` (1 fichier, -39 lignes). Pas de merge, pas de push. Lance 18:48 (Get-Date), fini ~18:52, ~13 appels vus.

## 1. Classement (12 tests, lignes = de IMPLEMENT_ a l'accolade fermante, relues dans integ-21 apres le retrait de grille du lot L2)

| Test | Lignes | Ce qu'il appelle | Classe |
|---|---|---|---|
| Victory.GameState | 38 | AHOLGameState seul (InitVictory, DefeatSlot...), aucun Mass | (a) porte, RETIRE ici |
| Victory.Collapse | 148 | RequestUnits x4, ServeBirthRequests, GetUnitView, Units->, 6 AHOLBuilding | (d) Mass |
| Victory.Forum.Fall | 162 | monde + AHOLGameState + AHOLBuilding (Setup, SetPrimaryForum), SetMatchSlot, SetResources, DamageBuilding, GetForumState, diffusion de pas, TActorIterator<AHOLRubble>, UHOLCommandSubsystem. AUCUN RequestUnits. | (c) |
| Victory.Forum.Hold.Chain | 111 | meme monde forum, SetForumPresenceOverride / Clear / SetForumPresenceProvider (lambda), GetForumState, pas, IsSlotDefeated. AUCUN Mass. | (c) |
| Victory.Forum.Hold.Presence | 116 | PlaceUnit (= RequestUnits + ServeBirthRequests) x3, MoveUnit (FTransformFragment) | (d) Mass |
| Victory.Forum.Hold.PresenceTeams | 69 | PlaceUnit x4, processeur de presence | (d) Mass |
| Victory.Forum.Builders | 177 | RequestUnits x3, ServeBirthRequests, Entity x11, GetUnitView x2, Mass x5, Processor x2 | (d) Mass |
| Victory.Forum.Block | 64 | pur (HOLForumRules::GetSelectionBlock, FHOLForumState) | (b') jumeau script existe : `Test_ForumBlock` (HOLForumRules_Test.as l.211) ; restent en C++ 3 assertions de `Block.Text` (tirets cadratins / accents) |
| Victory.Forum.Hold.Display | 125 | pur (GetRing, DescribeHold, GetSelectionBlock) | (b') jumeau script : `Test_ForumHoldDisplay` (l.272, anneaux et lignes en ASCII) ; restent les lignes accentuees de Block.Text |
| Victory.Forum.Look | 40 | pur (GetLook, BuildersWait) + HOLUnitAnimation::Choose (11 dernieres lignes) | (b') jumeau script `Test_ForumLook` (l.336) pour GetLook ; HOLUnitAnimation::Choose (3 assertions) NON porte : (c), ~12 lignes de liaison |
| Victory.Forum.Appearance | 115 | monde forum comme Fall + ApplyPendingAppearance, GetShownLook, UStaticMeshComponent (GetComponents), OnWorldBeginPlay | (c) |
| Victory.Forum.PlayersTable | 144 | pur mais GetPlayersTable(FHOLMatchConfig, TArray<FHOLForumState>, ...) : config et FHOLForumState non exposes comme entree | (c) |

Note de definition : aucun test n'est (b) au sens strict (enveloppes de UHOLTestWorldLibrary suffisantes). Les trois tests purs (Block, Display, Look) ont DEJA leur jumeau script, dans `UHOLForumRulesReadLibrary` (pas dans UHOLTestWorldLibrary), ecrit par une autre session ; leur retrait C++ est a decider par la session principale apres rouge puis vert de HOLForumRules_Test.as.

### Totaux en lignes de C++ (tests seuls, 1309 lignes ; le fichier fait 1425 lignes apres retrait, le reste = en-tete, aides, commentaires)
- (a) porte : 38 lignes (retire, commit c8c953e).
- (b) enveloppes existantes seules : 0 ligne.
- (b') jumeau script deja ecrit, retrait C++ possible apres rouge/vert (avec 3 assertions accentuees + 3 de HOLUnitAnimation a garder ou lier) : Block 64 + Display 125 + Look 40 = 229 lignes.
- (c) enveloppe a ajouter : Fall 162 + Chain 111 + Appearance 115 + PlayersTable 144 = 532 lignes.
- (d) Mass, reste en C++ (D854) : Collapse 148 + Presence 116 + PresenceTeams 69 + Builders 177 = 510 lignes.
(38 + 229 + 532 + 510 = 1309.)

### Enveloppes (c), liaison estimee (a lire dans HOLTestWorldLibrary / Match, jamais compile)
Deja la : SpawnGameState (InitVictory), SpawnPlayerState(Slot) = SetMatchSlot, SpawnBuilding(MainCentre, Owner, Owner, State) = SpawnActor + Setup(Type, Owner, Owner) (equivaut a Setup(MainCentre, 0, 0) / (.., 1, 1)), SetSlotResources, InitProductionSubsystem, AdvanceSteps / AdvanceStepsWithEnd (diffusent OnStepStarted / OnStepEnded), DamageBuildingByAttack, ApplyBuildingAppearance, CountBuildingMeshPieces, TraceSelectionRayDown, FHOLForumStateRead / FHOLForumTuningRead + GetGameTuning.
A ajouter (UFUNCTION sur UHOLTestWorldLibrary) :
1. Commun a Fall, Chain, Appearance (~35 lignes) : `SetBuildingPrimaryForum(Building, bool)` (3), `GetForumState(State, Slot) -> FHOLForumStateRead` (~12, convertit FHOLForumState), `GetForumLives(State, Slot)` (3), `IsRuinOver...` non necessaire. 
2. Chain (~28 lignes) : `SetForumPresenceOverride(State, Slot, bAttackers, bDefenders, AttackerSlot)`, `ClearForumPresenceOverride(State, Slot)`, `SetForumPresenceProviderContestSlot0(State)` (le lambda de test est fixe : bAttackers = Slot==0, bDefenders = Slot==0, AttackerSlot = 1).
3. Fall (~18 lignes) : `CountRubble(TestWorld)` (TActorIterator<AHOLRubble>) + lecture de la taille / emplacement du gravat ; UHOLCommandSubsystem a verifier (pose de point de ralliement sur un forum en ruine) : non verifie s'il est deja expose.
4. Appearance (~10 lignes) : `GetBuildingShownLook(Building)` (ApplyPendingAppearance + GetShownLook).
5. PlayersTable (~40 lignes) : `GetPlayersTableRowsFor(States[], Teams[], Forums as FHOLForumStateRead[], DeadMask, Local, Tuning)` -> TArray<FString> (Row.Shown joints), comme HOLGameStateTest::GetPlayersTableRows deja utilise par HOLDisconnectDisplay_Test.as (a verifier : il prend deja une config de 4 humains, peut peut-etre se reutiliser tel quel).
6. Look (HOLUnitAnimation::Choose) : ~12 lignes.
Total liaison estime : ~140 a 170 lignes de C++ neuf, pour retirer 532 lignes de tests C++ (+ ~11 de Look). Rapport 3 pour 1 en lignes. Les messages `AddExpectedMessage("Victory: forum of slot 0 fell...")` de Fall / Chain sont des lignes Log (pas Error) : le jeu les ecrit au journal, un test script ne peut pas les declarer attendues ; verifier qu'elles ne sont pas de niveau Error / Warning.

## 2. Retrait du test C++ Victory.GameState
Verifie assertion par assertion contre `Script\Tests\HOLVictoryWorld_Test.as` (Test_VictoryGameState, rouge vu puis vert a 18:13 par la session principale) :
- TestNotNull game state spawned -> AssertTrue(State != nullptr) : couvert.
- slot ferme 4 ne perd pas (DefeatSlot Cheat) -> couvert (AssertFalse).
- pas fini au lancement -> couvert. - slot 1 abandonne -> couvert. - slot 1 defait -> couvert. - raison Surrender -> couvert. - slot 0 non defait -> couvert. - fini -> couvert.
- gagnants == {0} -> couvert (Num == 1 puis [0] == 0, plus strict ou egal). - IsWinner(0) && !IsWinner(1) -> couvert.
- plus de defaite apres la fin (slot 0) -> couvert. - jamais deux fois (slot 1) -> couvert.
- 1 defaite annoncee, 1 fin annoncee (lambdas sur OnSlotDefeated / OnMatchOver) -> couvert par UHOLVictoryEventCounter (meme delegues).
- Config : C++ MakeConfig({Human 0, AI 0}) = slots 0 humain, 1 IA, equipe 0, reste ferme ; script SpawnGameState(World, 1, 1) -> meme (verifie dans le rapport cf). Seule difference : le monde est cree par CreateTestWorld au lieu de UWorld::CreateWorld a la main.
Retire en entier : lignes 111-149 de HOLVictoryTests.cpp (le test, 38 lignes + 1 ligne vide), `MakeConfig` reste (utilise par les autres). Aucune autre ligne touchee, CRLF et absence de BOM conserves.

## 3. Test porte
Aucun : il n'existe pas de test de classe (b) stricte (voir le tableau). Pas de falsification a jouer.

## Total de tests attendu
Aujourd'hui 1788. Apres ce retrait (-1 C++ Victory.GameState, Test_VictoryGameState script reste) : 1787 attendu (si le quick_check compte les tests script dans le total : le compte est inchange par construction du port, a verifier par la session principale ; aucun quick_check lance ici).

## Commande pour la session principale
Test C++ retire seulement, aucun C++ de liaison change : `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait` (le fichier de test change), puis `quick_check -Parts tests`.

## Champs migration
BUILDS_DEMANDES: 1 (le .cpp de test change). TESTS_CPP_RETIRES: 1 (HeroesOfLiberty.Victory.GameState). TESTS_SCRIPT_AJOUTES: 0. LIGNES_CPP_SUPPRIMEES: 39. LIGNES_MIGREES_PAR_LOT: 0 de plus (38 deja portees par cf). TYPES_A_EXPOSER: FHOLForumState (lecture deja la), forum presence override / provider, rubble, shown look, PlayersTable en entree, HOLUnitAnimation::Choose. TESTS_TAUTOLOGIQUES: 0. EDITS_REFUSES: 1 (mon garde-fou, compte de FHOLVictoryGameStateTest = 2 et non 3, refait). PROCESSUS_DETACHES: aucun.

## NON VERIFIE
Tout ce qui est compile : le .cpp de test sans le test (un include devenu inutile ne casse pas) ; le total 1787 ; les estimations de lignes de liaison (lues dans les signatures, pas ecrites) ; que UHOLCommandSubsystem soit expose au script ; que les lignes de journal de Fall / Chain ne soient pas Warning / Error ; l'equivalence exacte de HOLTestWorld::SpawnGameState(1,1) (vue dans le rapport cf, pas relue ici). Les jumeaux script de Block / Display / Look n'ont pas ete relus assertion par assertion (seulement leurs noms et quelques chaines).
## Anomalies
Fichiers non suivis `Tools\Scenarios\c28_*`, `c31_*` non touches. `IN_USE.txt` mis a jour (non commite).
