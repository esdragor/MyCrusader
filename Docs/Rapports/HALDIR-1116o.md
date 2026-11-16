# HALDIR-1116o
FAIT: E3-13 part python (Map.Csv) : Tools\Tests\map_table_test.py + branchement quick_check values, commit 32b6154 sur maps (merge main 060e8ea, conflit AngelscriptInProgress.md resolu en gardant les deux cotes). Branche prete.
TEST_ROUGE: vu a 21:46 : copie de Maps.csv faussee (ElDorado centre -> standard) FAIL (1) ; colonne en moins (Crown) FAIL (1) ; vraie table PASS (8 maps).
quick_check -Parts values : PASS (une fois, a la fin).
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 (FHOLMapCsvTest reste, retrait en E3-FIN) ; TESTS_SCRIPT_AJOUTES: 1 (map_table_test.py, python) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~45 (part Map.Csv du test C++) ; TYPES_A_EXPOSER: aucun ; TESTS_TAUTOLOGIQUES: 0 ; EDITS_REFUSES: 0.
RESTE: part Map.Rules (HOLMapRules_Test.as apres S1d) ; git rm du test C++ en E3-FIN.
NON VERIFIE: rien au-dela de quick_check values ; le test lit HOLMapRules.cpp comme texte (regex sur MakeMaps), comme site_count_accord_test.py.
ANOMALIES: quick_check.ps1 est UTF-8 avec BOM (sans BOM, PS 5.1 casse l'analyse) ; Placeholders.txt non concerne.
PROCESSUS_DETACHES: aucun lance.
