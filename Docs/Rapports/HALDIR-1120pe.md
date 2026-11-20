APPELS_DECLARES: 23 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 16:07 / 16:09
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build, script seul)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: d5cc2c3 (stub + 3 tests) ; assertit l'effet : oui (liste des arbres a retirer) ; NON LANCE, rouge non vu a l'execution
COMMITS: 3 (d5cc2c3 rouge, 74990c6 regle, journal NeutralsInProgress.md), dernier 16:1x, merge_main_fait: oui (already up to date)
TESTS: aucun lance (pas de quick_check) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
FAIT: cause trouvee (ClearNear saute les AHOLSourceField, l'arbre restant est une source de bois) ; regle HOLCampClearing + 3 tests ; ligne C++ dans Auto_Journey_File.md
RESTE: lot C++ (argument Trees de hol.Decor.ClearNear) ou deplacer le camp du scenario ; relancer le preflight neutrals_clairiere
NON VERIFIE: que l'arbre soit bien une source (deduction : log '0 left', apparence differente du bosquet) ; compilation et execution des tests script ; la regle n'a aucun consommateur tant que le C++ ne l'appelle pas
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir

ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non relu (docs EtapeDefini.md et Auto_Journey_File.md modifies volontairement)