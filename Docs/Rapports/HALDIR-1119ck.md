# HALDIR-1119ck
- FAIT : ca204bd sur grid-l3b. Test_CGSWTableBuildings garde tout sauf les 2 verifications de capacite de la rangee de recherche (retirees : 'researches N technologies...' et 'N technology cells for M technologies'). Test_CGSWResearchRowFits neuf (meme fichier, meme balayage 8 passes, lignes CGSW: au nom du nouveau test, commentaire en tete en anglais).
- known_reds.txt : ligne ajoutee pour Angelscript.UnitTests.Tests.HOLCommandGridSweeps_Test.Test_CGSWResearchRowFits (2026-11-19, ASCII).
- KnownReds.ps1 : Find-KnownRed compare par egalite exacte (-eq, insensible a la casse) du nom complet imprime devant ': FAIL' ; pas de prefixe. Find-KnownTestRed : rouge connu si TOUS les tests KO sont listes. Le nom doit donc etre exactement celui que quick_check imprime pour un test script (non verifie en execution).
- known_reds_test.ps1 : tout PASS (dont la ligne de ce test : date + cause).
- BUILDS_DEMANDES: 0. TESTS_SCRIPT_AJOUTES: 1 (Test_CGSWResearchRowFits). TESTS_CPP_RETIRES: 0. Total attendu 1787 (1786 verts + 1 rouge connu).
- NON VERIFIE : compilation du script, rejeu des tests (la session principale rejoue : attendu Test_CGSWTableBuildings vert, Test_CGSWResearchRowFits rouge connu) ; que le nom complet imprime par quick_check pour un test script soit bien Angelscript.UnitTests.Tests.HOLCommandGridSweeps_Test.Test_CGSWResearchRowFits ; ResearchCount reste utilise dans Test_CGSWTableBuildings (verification site/marche).
- PROCESSUS_DETACHES: aucun (rien lance).
