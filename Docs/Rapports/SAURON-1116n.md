# SAURON-1116n (visual, sans C++) : quick_check rouges connus par test + deploy_accord_test dans values

APPELS_DECLARES: 22 (arrêt à 45 vus)
HEURE_DEBUT/HEURE_FIN: 21:08 / 21:18
PREMIER_BUILD: sans C++   BUILD: sans C++ (aucun build)   LAUNCHCHECK_DEBUT: sans C++ (Get-StaleBinaryProblem lancé quand même à 21:10 avant quick_check -Parts tests : vide)
TEST_ROUGE: tâche 1 : vu à 21:09 : known_reds_tests_part_test « => 3 FAIL » (« tests 1326/1328 : FAIL (REGRESSION ...) », red summary « 1 regression », appel sans $Failures) ; commit vert test + correctif 8e566ef ; assertit l'effet : oui (ligne imprimée par la vraie fonction Report + red summary de la vraie Get-RedSummary). Tâche 2 : vu à 21:13 : ProposedPackMs 5000 -> 5001 dans Script\Siege\HOLDeployRules.as => « values : FAIL (REGRESSION) [KO] deploy_accord_test.py (deploy_accord_test : FAIL (1)) », remis (git status Script vide), commit 8f3dd6c.
COMMITS: 3 (8e566ef, 8f3dd6c, journal f0ac65c puis merge 75fd8f7), dernier à 21:16 (seuil 30 min), merge_main_fait: oui (75fd8f7)
TESTS: known_reds_tests_part_test 11/11 PASS, known_reds_test tout PASS, quick_check_tests_only_test PASS, quick_check_values_red_test 8/8 PASS (avant et après merge main) ; vrai `quick_check -Parts tests` 21:10-21:13 (170 s) : « tests 1352/1354 : FAIL (rouge connu depuis 2026-11-16 : 2 test(s) de known_reds.txt : NomadFirstForum, NomadReplicated) », « red summary : aucune regression (1 rouge(s) connu(s) : tests 1352/1354) » ; `quick_check -Parts values` PASS 21:14 (exit 0). TESTS_TAUTOLOGIQUES: 0
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 (1 test d'outil : known_reds_tests_part_test.ps1) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 ; TYPES_A_EXPOSER: aucun
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
- FAIT 8e566ef : KnownReds.ps1 `Find-KnownTestRed` (rouge connu si TOUS les tests KO sont dans known_reds.txt, date la plus ancienne) ; quick_check.ps1 `Report` prend un 3e paramètre `$FailedTests`, employé seulement si la seule vérification KO est l'unique (« all green ») ; la partie est alors ajoutée à $KnownReds (Get-RedSummary la compte connue) ; l'appel de la partie tests passe `$Failures` (l. ~7491, seule ligne touchée hors zone Report/values). known_reds.txt : commentaire CELEBORN remplacé.
- FAIT 8f3dd6c : deploy_accord_test.py dans `quick_check -Parts values` (3 lignes après unit_ability_accord_test.py).
- Docs : ScenariosInProgress.md (section SAURON-1116n), 2 lignes [x] d'Auto_Journey_File.md, 1 ligne d'EtapeDefini.md (après « REPRISE 21:03 »), IN_USE.txt libre.
- RESTE : rien des deux tâches. Branche prête.
NON VÉRIFIÉ: un test listé qui repasse n'imprime pas « retirer de known_reds.txt » pour la partie tests (seul un nom de partie le fait) ; la ligne [KO] values ne montre pas le détail « constant ProposedPackMs ... » (filtre l. 207 inchangé) ; conflit éventuel avec GLORFINDEL-1116n (netsig3) non essayé : mes zones sont Report (l. ~149-175), values (l. ~205-208) et l'appel `Report "tests ..."`.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
ANOMALIES_PROCESSUS: aucune (quick_check -Parts tests lancé en arrière-plan puis attendu jusqu'à sa fin, aucun quick_check en parallèle)
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, M EtapeDefini.md (mes lignes), ?? Docs/Priorite_Socle_Factions_2026-11-16.md (préexistant) : inchangée hors docs voulus : oui
