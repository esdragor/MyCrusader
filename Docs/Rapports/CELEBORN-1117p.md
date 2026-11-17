# CELEBORN-1117p : MIG-C-OrderChain.BuildingQueue (aucun code ecrit, rien a compiler)

APPELS_DECLARES: 22 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 18:48:39 / 18:50:15
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun C++ touche)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun commit ; assertit l'effet : non applicable (aucun test neuf)
COMMITS: 0, merge_main_fait: non (main 77f22a7 deja fusionne a l'entree)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 0   TESTS_CPP_RETIRES: 0   TESTS_SCRIPT_AJOUTES: 0   LIGNES_CPP_SUPPRIMEES: 0   LIGNES_MIGREES_PAR_LOT: 0   TYPES_A_EXPOSER: aucun

FAIT / RESTE:
- Le brief est perime : le test C++ `Orders.BuildingQueue` (HOLOrderChainTests.cpp, 171 l.) ne contient PAS de partie EndBuildingTarget ni de partie "apres End" (ses 4 blocs : AfterUnitAttack, Drop/MaySeek, Step/MakeChains, Deposit).
- Ces 4 blocs sont deja portes aux memes vecteurs dans Script\Tests\HOLOrderChain_Test.as (Test_OrderBuildingQueue_AfterUnitAttack, _Drop, _Step, _Deposit, plus AfterMove, TwoBuildings, ThenMove et les 4 Test_OrderBuildingQueue_End* qui utilisent HOLOrderChainRead::EndBuildingTarget(int ref, int ref)).
- Liaison deja presente et complete : UHOLOrderChainReadLibrary (IsCurrentOrderOver, MaySeek, EndBuildingTarget(int32& , int32&) avec FBuildingIdRef en namespace anonyme, MakeChains, Make*, KindName, GetMaxSteps) ; aucune exposition a ajouter.
- RESTE (session principale) : build groupe (la liaison EndBuildingTarget vient de S1g suite, 4f22df8 / 099531f), rouge VU (fausser une valeur des tests Test_OrderBuildingQueue_*), puis vert, puis suppression de HOLOrderChainTests.cpp (le namespace Flags `OrderChains::Flags` n'est utilise que dans ce fichier : a verifier par grep avant de supprimer).
- Tache de repli "Capacites d'unite" : deja faite aussi. HOLUnitAbilityRules_Test.as (4 tests) + Test_UnitAbilityReadLibraryAgreesWithTable (HOLSocleLibraries_Test.as) portent DamnedWarrior.RageConstants (CSV -> table script generee vs constantes C++ par HOLUnitAbilityRead). Le C++ HOLDamnedWarriorTests.cpp garde Table (a besoin d'une ligne de la DataTable UnitTypes : aucune source script, il faudrait exposer ~25 l. de liaison, FindRow -> Faction/MinTier/ArmorCategories/MaxHealth/Attack/AttackInterval/Cost) et Rage (Mass, reste en C++ D838) ; RageConstants C++ peut partir apres le rouge/vert du script. Pas commence faute d'appels.

NON VERIFIE: tout (rien compile, rien lance) ; l'exactitude de la parite vecteur par vecteur est une relecture, pas une execution.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune ; brief perime (partie End inexistante dans le C++ restant).
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, M EtapeDefini.md, ?? Docs/Priorite_Socle_Factions_2026-11-16.md (seuls docs voulus + fichier deja present : oui)
