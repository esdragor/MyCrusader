# ARWEN-1123pc (integ-37, script seul, 2026-11-23 15:58)
FAIT : D895 suivi mutuel, le dernier ordre donne gagne. HOLFollowRules::CancelsFollowOf (regle pure + cvar hol.Follow.MutualCancels defaut 1) appelee par HOLFollowOrders::Begin (le registre "qui suit qui" est le livre FHOLFollowBook, en script) : "B suit A" annule le suivi de A sur B.
COMMITS : rouge 031c71e (test + bouchon) ; correctif ba38fe4. Agent: ARWEN-1123pc sur les deux.
TEST_ROUGE: vu a 15:51 : tests 2145/2147 (LastOrderWins, RuleIsPure rouges ; DoesNotCancelThirdParty vert = temoin). TEST_ROUGE_SEPARE: oui
VERT : quick_check -Parts tests 2147/2147 PASS (15:57), 312 s. TESTS_SCRIPT_AJOUTES: 3 (HOLFollowRules_Test.as : Test_FollowMutualLastOrderWins, Test_FollowMutualDoesNotCancelThirdParty, Test_FollowMutualRuleIsPure). TESTS_TAUTOLOGIQUES: 0
BUILDS_DEMANDES: 0 ; TYPES_A_EXPOSER: aucun ; DIAGNOSTICS_AS: pas relu au serveur de langage, compilation des scripts OK via quick_check.
NON TRANCHE : cycle A->B->C->A (D895 muet) : non gere, non teste. Cvar a 0 = ancien comportement.
NON VERIFIE : jeu reel (clic droit sur deux unites), seulement la chaine script du livre ; merge main non fait (main a avance en C++).
integ-37 PRETE a ba38fe4.
