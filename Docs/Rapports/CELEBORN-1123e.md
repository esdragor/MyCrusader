# CELEBORN-1123e (T8 cout du tir de tour)
APPELS_DECLARES: ~27 (arret a l'heure : 10:25)
HEURE_DEBUT/HEURE_FIN: 10:09:58 / 10:22:29
PREMIER_BUILD: 10:15 (en file derriere Sauron depuis 10:12)   BUILD: PASS (stub 145 s, correctif 30 s)   LAUNCHCHECK_DEBUT: non lance (binaire de 10:05 declare a jour par le lancement ; le build a ete exige par mes sources)
TEST_ROUGE: b5bf92a (bouchon FBuildingShotCost + GetBuildingShotCost, rien compte, + test HeroesOfLiberty.Combat.BuildingShotCost dans HOLBuildingShotTests.cpp) ; assertit l'effet : oui (recherches, candidats par pas lus du vrai processeur) ; vu a 10:18 : searches 0, candidates 0, 0 steps (3 assertions rouges, les 11 autres tests verts)   TEST_ROUGE_SEPARE: oui
COMMITS: 3 (b5bf92a rouge, 0d1c150 correctif, 8e47ff6 docs), dernier a 10:22, merge_main_fait: non (consigne : jamais de fusion ; main non fusionne ici)
TESTS: RunTests BuildingShot+Net.BuildingArrow : 12/12 vert a 10:21 (UnrealEditor-Cmd, 12 trouves) ; mesure : 70 recherches, 14000 candidats, 201 pas, moyenne 69, pas le plus cher 2000 ; TESTS_TAUTOLOGIQUES: 0 (les compteurs viennent du processeur reel, le plafond de recherches vient du calendrier ExpectedArrows)
ESSAIS: tower_wood NON FAIT (< 5 min restantes apres la file de build) ; aucun scenario, aucun PNG
QUESTIONS: aucune (budget 200 candidats/pas pose dans le test, constante BudgetAveragePerStep)
FAIT / RESTE: FAIT compteur (struct FBuildingShotCost dans HOLCombatProcessor.h, 6 champs, compte dans ReleaseBuildingArrows : remise a zero en tete de pas, +1 recherche et +Fighters.Num() candidats avant la boucle) et banc (options Crowd et Towers>2 de RunScenario). RESTE : scenario tower_wood + preflight ; valider le budget ; RELECTURE_CPP : relue seulement par moi (diff court : 2 blocs de compteurs, rien ne change le tir) ; le test ne ferme pas le trou "tour sans cible cherche a chaque pas" (Cooldown remis a StepMs) : hors du banc car tous les ennemis sont a portee.
NON VERIFIE: tower_wood ; DIAGNOSTICS_AS sans objet (C++) ; un banc avec ennemis hors de portee (recherche a chaque pas, 2000 candidats par pas par tour-groupe) ; relecture C++ independante.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn

ANOMALIES_PROCESSUS: file de build 3 min derriere Sauron ; le premier appel de test dans le meme script que le build n'a pas tourne (condition -match sur un tableau) ; CELEBORN-1123d_tests.ps1 d'un autre agent lu, non lance.
GIT_STATUS_PRINCIPALE:  M Auto_Journey_File.md
 M Auto_Journey_Jour.md
 M Docs/Rapports/BUILD-CELEBORN-1123d.txt
 M Docs/Rapports/FARAMIR-1123d.md
 M EtapeDefini.md
 M Retours/Q-sacred-light-values.md
?? Docs/Rapports/ELROND-1123f.md
?? Docs/Rapports/FARAMIR-1123b-fix.ps1
?? Docs/Rapports/FARAMIR-1123e.md
?? Docs/Rapports/GLORFINDEL-1123g.md
