# ELROND-1117a : lot L17-01 (Diplomacy.Rules)

FAIT : le port existait deja (CELEBORN-1116t/u) dans Script\Diplomacy\HOLDiplomacyRules_Test.as (pas Script\Tests : un seul fichier, pas de doublon). Seul changement : commentaire perime corrige (CanGarrison/DoorsOpenFor/CanLeaveRampart/AlliedVictory SONT exposes). Commit tiers 912c1e7. Branche prete, merge main non fait (DLL non rebuilt).
TEST_ROUGE: vu a 00:16 : 1362/1366 (Test_DiplomacyRulesVectors : Slots 4->5 ; Test_DiplomacyRulesVectorsGarrisonVictory : "0,1,2"->"0,1,3"), remis ; vert 00:19 : 1364/1366 (seuls rouges : Nomad D831).
Nommage : Test_DiplomacyRulesVectors / ...VectorsGarrisonVictory (pas "Matrix" : la matrice A x B est dans Vectors).
REGISTRE test C++ Diplomacy.Rules -> script : defaut/ordre/soi-meme, allie si les deux, matrice 3x3 du plus hostile, vision + rupture + re-alliance, Neutre adversaire, refus (ordre), vision refus, IA (D367), equipes verrouillees, garnison/remparts, victoire alliee (AIs, equipes) -> Test_DiplomacyRulesVectors + Test_DiplomacyRulesVectorsGarrisonVictory (+ 14 tests detailles du meme fichier). Non porte / non expose : rien (tout est dans HOLDiplomacyRead).
Test C++ NON supprime (HOLDiplomacyRulesTests.cpp, 163 l.) : git rm au lot MIG-C++-FINAL.
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 (deja 2 Vectors) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 (deja migrees) ; TYPES_A_EXPOSER: aucun ; EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: voir stray_processes.
NON VERIFIE : quick_check lance 2 fois (rouge puis vert), pas -Full.
