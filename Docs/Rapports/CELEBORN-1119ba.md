# CELEBORN-1119ba : tests de deconnexion D853 d, liaison C++ + tests script completes

NON COMPILE : aucun build, aucun quick_check, aucun editeur lance (consigne). Rien n'a ete execute.

APPELS_DECLARES: 15 (arret a 25 vus ; arret dur 15:41)
HEURE_DEBUT/HEURE_FIN: 15:22:52 / ~15:27
PREMIER_BUILD: sans build (lot C++ ecrit, a compiler par la session principale)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: aucun vu (rien execute) ; assertit l'effet : oui (stock rendu, file, niveau, ordre IA lu dans la file)
COMMITS: 1 (ad16f0b sur test-world-6, 15:26), merge_main_fait: non (consigne : aucune fusion)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (Gains : la valeur attendue vient de GetBuildingCancelRefund / GetProductionCostResources, calculees par le code, comparee au stock rendu par la vraie chaine KeepAbsentSlot -> DestroyBuilding -> GiveSlotStock)

## Fait

Liaison C++ (UHOLTestWorldLibrary, namespace script HOLTestWorld, Source\HeroesOfLiberty\Tests\HOLTestWorldLibrary.h/.cpp), 6 fonctions, corps sous WITH_DEV_AUTOMATION_TESTS :
- `DestroyBuildingWithRefund(Building)` -> `UHOLConstructionSubsystem::DestroyBuilding(*Building)` (le chemin du combat, rembourse ; `DestroyBuilding` existant reste un Destroy simple).
- `GetSlotLevel(TestWorld, Slot)` -> `UHOLAIToolsLibrary::GetSlotLevel(World, Slot)`.
- `IssueAIProduceOrder(TestWorld, Slot, Location, Type)` -> `UHOLAIOrderLibrary::IssueAIProduceOrder(World, ...)` (le monde du test en contexte).
- `GetAITechnologiesDone(TestWorld, Slot)` -> `UHOLAITechnologyLibrary::ReadAITechnologiesDone`.
- `CountPlayerStatesOfSlot(TestWorld, Slot)` : compte sur `PlayerArray` du game state (ce que le C++ compte).
- `GetPlayerStateSlot(Player)` -> `GetPlayerIndex()`.
3 includes ajoutes dans le .cpp (AI/HOLAIOrders.h, HOLAITechnologyLibrary.h, HOLAITools.h).

Tests script completes (Script\Tests\HOLDisconnect2_Test.as), valeurs C++ reprises :
- Gains (3) : destruction de la fondation et du centre par DestroyBuildingWithRefund ; attendu = depart + 10 bois + remboursement fondation + cout du Paysan (fin du "non porte").
- Technologies (6) : lecture IA `GetAITechnologiesDone` (PloughI en absence, PloughII apres la fin) en plus de la lecture sur le player state du holder.
- Held (7) : `GetSlotLevel` -1 present, Normal absent, -1 au retour.
- HeldStock (8) : l'ordre passe par `IssueAIProduceOrder` (plus EnqueueProduction) puis 1 pas ; compte de player states du siege = 1.
- Replace (9) : ordre de Bob par `IssueAIProduceOrder` + 1 pas ; `GetPlayerStateSlot(BobOld) != 1`.
- FakeBack (12) : compte de player states du siege = 1 (plus seulement FindLiveHolder).
- LateReturn (14) : `GetSlotLevel` Normal apres la fin de fenetre, -1 apres le retour.
- Commentaires "C++ file stays" : reformule en tete de HOLDisconnect2_Test.as. HOLDisconnect_Test.as:9 et HOLDisconnectDisplay_Test.as:8 disent encore "the C++ file stays" : exact tant que le retrait n'est pas fait, non modifies.

## Reste (non fait, liste precise)

- Test 6 : capture de journal (les 2 comptes "researched PloughI" l.295 et 298 du C++) : OutputDevice a lier (demarrer / arreter / compter lignes contenant A et B). La cvar BuildTime reste remplacee par 3000 pas.
- Test 15 : mode Diplomacy dans `HOLMatchConfigRead` (un setter de mode dans le JSON) ; le C++ garde ces 2 assertions.
- Test 1 : `DefaultWindowSeconds == 180` (constexpr, a lire par un accesseur ou `FHOLDisconnectRules` expose).
- Aucun test C++ retire (consigne). Les tests 2, 4, 5, 10, 11, 13 peuvent partir apres le rouge/vert ; 3, 7, 8, 9, 12, 14 aussi si ce lot est vert ; 6, 15, 1 gardent leurs lignes citees.

## Falsification pour le rouge a voir

Fichier `Source\HeroesOfLiberty\Tests\HOLTestWorldLibrary.cpp`, fonction `DestroyBuildingWithRefund` : remplacer `UHOLConstructionSubsystem::DestroyBuilding(*Building);` par `Building->Destroy();` (rebuild C++). Attendu rouge : `Test_DisconnectGains` seul (assertion "back: his stock plus the 10 wood, the foundation's cost and the queue's"). Rendre ensuite le vert.
Variantes sans rebuild du lot de test : mettre `const int Normal = 2;` (HOLDisconnect2_Test.as, Test_DisconnectHeld) -> Held rouge.

## Champs migration
BUILDS_DEMANDES: 1 (lot C++ de liaison, a lancer par la session principale : `Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree Celeborn, puis `quick_check -Parts tests`)
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 0 neufs ; 7 completes (Gains, Technologies, Held, HeldStock, Replace, FakeBack, LateReturn)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: ~35 lignes script, ~75 lignes C++ de liaison
TYPES_A_EXPOSER: capture de journal ; mode de match (Standard/Diplomacy) dans HOLMatchConfigRead ; DefaultWindowSeconds
EDITS_REFUSES: 0
NON VERIFIE : tout (non compile, non execute). Risques : (1) l'ordre IA `IssueAIProduceOrder` avec le monde en contexte : l'ancienne anomalie "file vide" venait du contexte de script, a confirmer ; (2) `CountPlayerStatesOfSlot` compte sur PlayerArray : suppose que `AddPlayerState` du test remplit PlayerArray comme dans le C++ (le C++ fait pareil) ; (3) GetSlotLevel Normal d'un siege tenu suppose le cerveau du monde de test (Held et LateReturn verts avant, GetAIBrain non nul) ; (4) EOL : avertissement git "LF will be replaced by CRLF" sur le .cpp (fichier a fins mixtes d'origine, aucun diff parasite attendu).
PROCESSUS_DETACHES: aucun processus lance par cet agent (stray_processes non lance).
