# ELROND-1120pa : D869, montage 1,5 s / demontage 1 s (NON COMPILE : lot C++ de la session principale)

APPELS_DECLARES: ~17 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 15:40:41 / 15:44 (Get-Date)
PREMIER_BUILD: sans build (lot C++ de la session principale)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build demande ; binaire de integ-25 compile sur a831674 selon l'enonce)
TEST_ROUGE: 5e6ff7c (Test_DeployDelaysAreVisible, Script\Tests\HOLDeployRules_Test.as) ; assertit l'effet : oui (mesure le temps que met HOLDeployRead::Advance, pas de 50 ms, a changer de phase : 1000-2000 ms, plus la phase a mi-delai). Rouge ATTENDU sur le binaire actuel (8000 / 5000 : MountMs 8000 et DismountMs 5000 hors [1000, 2000]) ; NON VU a l'execution (pas de lancement). Commit du test seul.
COMMITS: 2 (5e6ff7c test, 0abe6ca correctif), dernier 15:42, merge_main_fait: oui (main a831674, deja a jour ou fusion triviale)
TESTS: aucun lance. A lancer apres compilation : quick_check -Parts tests (script HOLDeployRules_Test : Test_DeployDelaysAreVisible doit passer de rouge a vert) + HeroesOfLiberty.Siege.Deploy* C++ (ProcessorTests, ButtonTests, InMatchTests : leurs pas viennent de DefaultProfile(), aucune valeur 8000/5000 en dur trouvee ; les tests C++ citees par le plan aux lignes :124-125, 297, 415-453, 548, :146 n'ont rien a changer) ; TESTS_TAUTOLOGIQUES: 0 (la duree assertee est CALCULEE par Advance, bornes 1-2 s tirees de D869 et non de la constante)
ESSAIS: deploy : NON rejoue (pas de build) ; preflight non lance ; LOGIQUE SEULEMENT. Scenario modifie : captures deploy_deploying (apres D + 15 frames) et deploy_packing (apres 2e D + 15 frames) PENDANT le changement (L19-5), attentes 9 s / 6 s ramenees a 2 s, notes et .look mis a jour. Les waitlog "(delay over)" du scenario suivent la duree reelle.
QUESTIONS: Retours\Q-deploy-visible.md (A : case D grisee « Montage... / Demontage... », B : barre, C : animation), recommandation A puis B ; defaut code : aucun signe nouveau, seules les durees changent (reglables : HOLDeployRules.h:76-77).
FAIT / RESTE: FAIT : cause etablie par lecture ; durees 1500 / 1000 ; test script rouge ; scenario + .look. RESTE : compiler, lancer les tests, preflight puis rejeu deploy (ouvrir les PNG deploy_deploying / deploy_packing : le changement est-il visible ?), reponse a Q-deploy-visible.
CAUSE (D869) : ce n'est NI une duree a 0 NI une duree ignoree. HOLDeployRules.cpp:34-35 (DefaultProfile) = 8000 / 5000 ; HOLDeployProcessor.cpp Execute lit le vrai pas (HOLCombat::ToMilliseconds) et appelle HOLDeploy::Advance (HOLDeployRules.cpp:78-97) qui compte ElapsedMs ; le test C++ du processeur verifiait deja qu'a mi-delai l'engin est encore Deploying. L'ecart avec « instantane » : AUCUN SIGNE VISIBLE pendant le changement (la case D ne change de libelle qu'a la fin, HOLGameScreen.cpp:1254 ; Command.bPack ; aucune animation, aucune barre : le mannequin n'a pas d'animation de montage ; HOLUnitSubsystem.cpp:1206 ne replique que Phase), et le scenario ne capturait qu'apres la fin. Hypothese NON PROUVEE a l'ecran : l'utilisateur a vu la case D basculer sans rien entre. Hors C++ : HOLDeployRules.h:76-77 est le seul endroit des valeurs.
NON VÉRIFIÉ: tout (rien compile ni lance) ; que le test script compile (T.AssertTrue avec message "..." + int ; AssertTrue/AssertEquals existent dans le meme fichier) ; que 1,5 s / 1 s paraissent visibles sans signe ajoute (probablement pas assez : voir Q) ; que « frames 15 » apres la touche D tombe bien dans les 1,5 s a l'ecran (60 img/s en capture ?) ; que le libelle de la case D pendant Deploying/Packing soit coherent (non lu : Command.bPack). Numeros de ligne du journal Palier3InProgress.md : certains approximatifs.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond

ANOMALIES_PROCESSUS: aucune ; IN_USE.txt etait deja « libre » a l'entree (pas d'ecriture OCCUPE au demarrage, ecrit directement « libre » a la fin, CPP: oui).
GIT_STATUS_PRINCIPALE:  M Auto_Journey_File.md
 M Auto_Journey_Jour.md
 M ChatInProgress.md
 M EtapeDefini.md
 M Palier3InProgress.md
?? Docs/Concept_Faction_Theatre_2026-11-18.md
?? Docs/HSR_Aha_Allegresse_Nihilux.md
?? Docs/Priorite_Socle_Factions_2026-11-16.md
?? Docs/Rapports/ETAPE0-1120pm.md
?? Docs/Rapports/GALADRIEL-1120pa.md
?? Docs/Rapports/GLORFINDEL-1120pa.md
?? Docs/Rapports/HALDIR-1120pa.md
?? Retours/Q-deploy-visible.md
 (hors docs voulus : fichiers de code non touches dans la copie principale : oui ; docs modifies par moi : EtapeDefini.md, Auto_Journey_File.md, Palier3InProgress.md, Retours\Q-deploy-visible.md, ce rapport)