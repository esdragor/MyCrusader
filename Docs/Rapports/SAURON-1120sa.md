# SAURON-1120sa (tool-known-reds, sans C++) 22:56 -> 23:0x
APPELS_DECLARES: ~27 (arret par heure/appels atteint) ; HEURE_DEBUT 22:56 ; PREMIER_BUILD: sans C++ ; BUILD: NON COMPILE (aucun build) ; LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: d4a1527 (regen_tables_test.ps1, script absent : 1 FAIL vu 22:58) ; assertit l'effet : oui (les deux commandes imprimees, ordre, drapeaux, -Root)
COMMITS: 3 (d4a1527 test, 318837f script + scenario, un 3e pour -nullrhi), merge_main_fait: non (branche deja a jour avec main selon l'enonce)
TESTS: regen_tables_test.ps1 13 PASS ; no_window_test : regen_tables.ps1 d'abord FAIL (-nullrhi absent de la ligne d'appel), corrige ; reste 1 FAIL PRE-EXISTANT : ecran_multijoueur.ps1:40 Start-Process sans -WindowStyle Hidden (non touche, hors tache) ; TESTS_TAUTOLOGIQUES: 0
TACHE 1 : cause = hol.Select.ClickUnit 0 (index 0 = Paysan). Correctif : hol.Select.Type VirtuousKnight (Script\Player\HOLControllerCommandsC9b.as, log 'Select type <Type>: <N> units') + waitlog sur cette ligne ; .look : anneau de 8 m et PV du Chevalier. LOGIQUE SEULEMENT, NON REJOUE.
NON VERIFIE : le scenario virtuous_knight n'a pas ete joue (aucun play.ps1) ; regen_tables.ps1 sans -DryRun jamais lance (editeur) ; make_*_types.py existent sous Tools\Editor : non verifie.
