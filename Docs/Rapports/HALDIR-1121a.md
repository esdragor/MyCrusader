# HALDIR-1121a : test MainCentre 150 s

NON COMPILE, NON LANCE (aucun build, aucun quick_check dans l'agent).

APPELS_DECLARES: ~21 (arret a 25 vus ; heure dure 11:05 non atteinte)
HEURE_DEBUT/HEURE_FIN: 10:50:38 / ~10:53
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: bd900a5 (test seul) ; assertit l'effet : oui (fondation en chantier au pas N-1, finie au pas N, vraie chaine table -> GetBuildSteps -> AddWork). Rouge NON vu et non attendu : BuildingTypes.csv MainCentre BuildTime = 150 depuis 97f757c, donc le test doit etre VERT d'emblee ; pas de correctif a livrer.
COMMITS: 2 (bd900a5 test ; journal MapsInProgress.md), dernier ~10:53, merge_main_fait: non (branche a main bce2d69 = deja a jour, aucun merge necessaire)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (N = GetBuildSteps(Row.BuildTime), pas d'un nombre recopie ; seul 3000 est un attendu litteral = 150 / 0,05)
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
- Constat : HOLConstructionTests.cpp n'existe plus (supprime par 1e9d0b0, tests portes en script) ; l. 238 et 351-365 n'existent plus. D839 : donc test ecrit en SCRIPT, pas de C++.
- Fait : Test_Construction_MainCentreOneBuilder dans Script\Tests\HOLConstruction_Test.as (fin du fichier).
- bSitesLaidByMaps non touche (reste false).
- RESTE : lancer le test ; si rouge (BuildTime != 150 dans la table .uasset regeneree, ou Math::Pow / GetHealth signature) corriger ; si vert, cloturer la ligne CELEBORN-1116a de Auto_Journey_File.md.
BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 1 (Test_Construction_MainCentreOneBuilder) ; TYPES_A_EXPOSER: aucun
DIAGNOSTICS_AS: NON RELU (serveur de langage non consulte)
NON VERIFIE: compilation du script (Math::Pow(double,double), FHOLBuildingTypeRow.MaxHealth, GetHealth(int, int64, int64) pris du modele Builders / Tables) ; que la table .uasset porte bien 150 (seul le CSV a ete lu) ; verdict vert/rouge.
ANOMALIES_PROCESSUS: premier Edit au 16e appel et non avant le 8e (le fichier cible a du etre retrouve : supprime de main) ; appel PowerShell Get-Date refuse une fois par le classifieur (relance) ; IN_USE.txt ecrit « CPP: oui » a l'entree par erreur puis corrige en « libre ... CPP: non » (tache finalement sans C++).
GIT_STATUS_PRINCIPALE: 3 fichiers non suivis d'avant (Docs\Concept_Faction_Theatre..., HSR_Aha_Allegresse_Nihilux, Priorite_Socle_Factions) + EtapeDefini.md et Auto_Journey_File.md modifies (docs voulus) : oui

Commande a lancer (session principale, depuis C:\Users\baudo\Documents\HeroesOfLiberty-Haldir ; pas de C++ neuf donc pas de build requis si la DLL est a jour) :
powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Tests\quick_check.ps1 -Parts tests -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
(chercher Test_Construction_MainCentreOneBuilder)
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
