NON COMPILÉ
APPELS_DECLARES: 12 (arrêt à 45 vus par toi)
HEURE_DEBUT/HEURE_FIN: 17:06 / 17:08
PREMIER_BUILD: sans build (NON COMPILÉ, lot à lancer par la session principale)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (aucun build dans l'agent)
TEST_ROUGE: 824693a (tests seuls) assertit l'effet : oui (phase Packed pendant 10 s après Stop ; Deploying/Deployed après fin normale)   TEST_ROUGE_SEPARE: oui (un seul commit : pas de correctif, donc pas de rouge à séparer)
COMMITS: 1, dernier à 17:08, merge_main_fait: non (aucun build, DLL non touchée)
TESTS: aucun lancé (NON COMPILÉ). Attendus : HeroesOfLiberty.Siege.Deploy.StopDuringMarchStaysPackedAndDoesNotMountByItself (vert d'emblée attendu), HeroesOfLiberty.Siege.Deploy.NormalMarchEndWhilePackedMountsTheEngine (vert attendu, D877) ; TESTS_TAUTOLOGIQUES: 0 (chaîne réelle Move / Stop -> file -> UHOLUnitOrderProcessor -> UHOLDeployProcessor -> UHOLCombatProcessor)
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE: FAIT : lecture : Stop -> ApplyCombatOrder (HOLUnitOrderProcessor.cpp:1567-1571 dispatch, :3200 fonction) boucle sur toutes les entités et pose CutDeploy->bMarching = 0 (:3224-3227) sans filtre de Kind ; donc aucun correctif écrit. Tests ajoutés en fin de HOLDeployButtonTests.cpp : aide HOLDeployStopMarch::Run(Test, bStop) + FHOLDeployStopCutsMarchStaysPackedTest + FHOLDeployNormalMarchEndMountsTest. Cas Stop : Move 1000 cm, 5 pas de marche simulée, Stop, assertions bMarching 0 + Phase None, puis 10 s : jamais hors Packed, pas de marche après Stop. Contre-cas : la marche se ferme à l'arrivée -> Deploying/Deployed. RESTE : build (Tools\build.ps1 -LiveCoding=0 -Wait dans C:\Users\baudo\Documents\HeroesOfLiberty-Elrond) puis quick_check -Parts tests ; si les deux sont verts, la ligne de file est périmée (marquée [x] avec cette réserve) ; si le test Stop est rouge, le défaut est ailleurs que ApplyCombatOrder (ex. Stop via UHOLPlayerOrdersBoard::AskStop non routé vers EHOLOrderKind::Stop).
RELECTURE_CPP: relu après écriture (L20-2) : includes déjà présents (Test, Mass, HOLPlayerOrders) car recopiés du modèle FHOLDeployAttackCutsOpenMarchTest ; namespace HOLDeployStopMarch (nom unique), FAutomationTestBase& public TestXxx ; signature HOLPlayerOrders::Stop(World, int, TConstArrayView<uint32>, bool) vérifiée dans HOLPlayerOrders.h:50 ; TArray<uint32> converti en TConstArrayView comme dans le modèle. Doute : HOLDeploy::EPhase enum cast uint8 comme dans le modèle ; fichier en LF (avertissement git CRLF, sans effet). Non compilé.
TEST_ROUGE_SEPARE: oui (voir plus haut)
NON VÉRIFIÉ: compilation, exécution des deux tests ; hypothèse que le harnais sans système de mouvement garde MoveOrder.Phase != None après Move (le modèle existant le suppose aussi) ; que Stop passe par la file jusqu'à EHOLOrderKind::Stop (AskStop non relu).
ANOMALIES_PROCESSUS: aucune
PROCESSUS_DETACHES: voir ci-dessous
GIT_STATUS_PRINCIPALE: voir ci-dessous