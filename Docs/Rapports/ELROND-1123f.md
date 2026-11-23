# ELROND-1123f : tests de liaisons + S4-b branche (integ-25)

APPELS_DECLARES: ~22 (arret 25)
HEURE_DEBUT/HEURE_FIN: 10:06 / ~10:20
PREMIER_BUILD: sans C++   BUILD: sans objet (build du lot : PASS 705 s, 1123e)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 64972bb (stub PlanFromSummary + test Test_AIGarrisonPlanFromSummary), vu a 10:14 : tests 2103/2104 ; assertit l effet : oui (appels calcules depuis un resume)   TEST_ROUGE_SEPARE: oui
COMMITS: 2 (64972bb rouge, 1c9571f vert), dernier 10:15, merge_main_fait: non
TESTS: quick_check -Parts tests (worktree) : 2103/2103 PASS au depart (les 3 tests HOLBindingsLotTests verts : 2100 + 3), 2104/2104 PASS apres S4-b ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
FAIT: (1) tests de liaisons verts. (2) HOLAIGarrisonCall.as : PlanFromSummary (tours = batiments finis du slot avec places, candidats = Army avec GarrisonKind lu) et Issue (appelle HOLAIGarrisonOrder::IssueAIGarrisonOrder). HOLAIBrain.as : ThinkGarrison, une ligne apres ThinkRage.
RESTE: aucun lecteur des coups sur AHOLBuilding : le cerveau donne des durees inconnues (-1) donc toutes les tours sont calmes et aucune garnison reelle n a lieu ; rien n est invente. Un lecteur (C++ ou memoire de PV) est a decider. Les liaisons OwnerType ne prouvent que le chemin sans monde.
NON VERIFIE: appel reel IssueAIGarrisonOrder en match (aiserver/aiduel non lances ; quick_check.ps1 les contient aux L7615 / L7694, aucun ne nomme garnison) ; Binds.Cache regenere ; DIAGNOSTICS_AS non consulte (0 erreur de script au quick_check) ; HOLAIPowerCall.as et HOLFactionPowerServer.as non touches.
