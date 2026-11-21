# SAURON-1121b (worktree HeroesOfLiberty-Sauron, branche tool-known-reds, sans C++)

Impact joueur : le scenario `deploy` ne fait plus naitre le Mangonneau SUR la crete de 3 rochers (les 3 ovales blancs), il nait a cote.

APPELS_DECLARES: 21 (arret a 25 vus ; le premier Edit/ecriture est venu au 7e appel pour IN_USE.txt par PowerShell, au 8e pour le test : consigne "premier Edit avant le 8e appel" respectee de justesse via IN_USE)
HEURE_DEBUT/HEURE_FIN: 10:57:59 / 11:01:21 (arret dur 11:17 non atteint)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun C++)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aea9d23 (Tools/Tests/ridge_spawn_test.py, rouge vu a 11:00:12 : exit 1, `FAIL deploy.txt:14 SpawnHere (radius 50) at the focus (3000, 0) is off the ridge`) ; assertit l'effet : oui (la position de naissance calculee depuis le FocusNear du scenario, contre l'empreinte des 3 rochers calculee depuis common.RIDGE_BOULDERS)
TEST_ROUGE_SEPARE: oui (rouge aea9d23 seul, puis correctif 1263954)
COMMITS: 3 (aea9d23 test, 1263954 correctif, 58b7a69 docs), dernier a 11:01, merge_main_fait: non (branche non fusionnee avec main : tache sans C++, DLL non verifiee)
TESTS: `python -I Tools\Tests\ridge_spawn_test.py` : rouge 1 FAIL (deploy.txt:14) puis PASS apres correctif (31 verifications de scenarios de cartes a crete) ; quick_check non lance (pas de build/quick_check dans l'agent) ; TESTS_TAUTOLOGIQUES: 0 (les positions sont calculees du scenario, l'empreinte des rochers des vraies constantes de common.py ; deux gardes : centre de la crete = sur, forum (0,0) = hors)
ESSAIS: aucun liste (pas de nouvel essai ; rejeu cache de `deploy` reste a faire par la session principale)
QUESTIONS: aucune
FAIT: tache 1 (ligne VirtuousKnight du CSV) deja dans main (ligne UnitTypes.csv + Placeholders.txt:40 redirection 30 % / 8 m reelle D864) : sautee ; tache prise = 2e ligne [ ] (deploy crete, SAURON-1116d). Test `ridge_spawn_test.py` (cartes dont le builder appelle common.spawn_ridges : cols, crown, fords, forest, hinterland, plain, steppe ; arguments SpawnHere = Count Type Slot Radius, le rayon elargit l'empreinte) branche dans quick_check (partie values) ; correctif deploy.txt `FocusNear 3000 1500` aux deux endroits (lignes 10 et 17) + commentaire de la ligne 68 (Maison a ~2750 cm, portee 3000 OK).
RESTE: rejeu cache de deploy (play.ps1 deploy -Hidden -Root <worktree>) ; fusion avec integ-25 : ELROND-1121a a deja change les memes lignes FocusNear (5375b7a, absent de main) : conflit trivial (memes valeurs) ; D877 (correction du controleur de deploiement) est la ligne Elrond, pas la mienne ; map_eldorado.txt n'est PAS concerne (ElDorado n'a pas de crete : eldorado.py n'appelle pas spawn_ridges).
NON VERIFIE: le build du jeu et quick_check values/tests complet (non lances) ; le jeu reel : la portee du Mangonneau a (3000,1500) de la Maison (5300,0) = 2746 cm < 3000 calcule, pas vu a l'ecran ; la ligne de commentaire :68 n'a pas ete rejouee ; le modele de repere (Dx vers le milieu) vient de l'analyse SAURON-1116d et du commentaire du scenario, non revalide en jeu.
ANOMALIES_PROCESSUS: premier ecriture (IN_USE.txt) via PowerShell au 7e appel, pas par l'outil Edit ; 3 premiers appels = lecture des regles et de la file ; la ligne 1 (VirtuousKnight) de la file reste [ ] dans Auto_Journey_File.md alors que la ligne CSV et le placeholder sont deja dans main (reste de la ligne = branchement HOLCombatProcessor / anneau 8 m, C++ ou D864/D865 deja faits ? a nettoyer par la session principale).
C++ a signaler pour le lot groupe: aucun (l'UE_LOG de ApplyDamageRedirect n'etait pas dans ma tache).
DIAGNOSTICS_AS: sans objet (aucun .as touche)
PROCESSUS_DETACHES: "aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron"
GIT_STATUS_PRINCIPALE: docs seulement (EtapeDefini.md, Auto_Journey_File.md modifies par moi, ligne d'ajout ; le reste des M/?? vient des autres sessions) : oui

Commande de tests a lancer :
  python -I C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Tests\ridge_spawn_test.py
  puis (apres build habituel) : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Tests\quick_check.ps1 -Parts values
  puis rejeu : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 deploy -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
