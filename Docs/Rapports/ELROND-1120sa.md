# ELROND-1120sa : D869b, barre de montage a 0 a 0,3 s et 2e cycle sur « Montage… »

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche integ-25. Impact joueur : la barre de montage de la case D avance des l'ordre, et le 2e cycle revient a « Demonter » (cle Pack).

APPELS_DECLARES: ~24 (arret a 25 vus ; 23:12 dur)
HEURE_DEBUT/HEURE_FIN: 22:56 / voir fin de fichier
PREMIER_BUILD: 22:59 (incremental, file vide, 32 Go libres, L20-5)   BUILD: voir ligne BUILD_RESULTAT   LAUNCHCHECK_DEBUT: « the binary ... is older than its C++ source HOLDeployButtonTests.cpp » (binaire du 17:14, build lance ensuite)
TEST_ROUGE: c7affed (stub FWatch = ancien comportement + 2 tests) ; assertit l'effet : oui (permille de la vraie `HOLDeploy::ProgressPermille` a 0,3 s de l'ordre, et au 2e ordre). TEST_ROUGE_SEPARE: oui (c7affed seul, puis 5cbbf40). Rouge VU a l'execution : NON (pas de quick_check dans l'agent ; rouge par trace a la main : stub -> 0 permille a 0,3 s, 733 permille a 1,4 s au lieu de >= 875, 2e cycle 1000 permille au lieu de 125..400).
COMMITS: 3 (c7affed test+stub, 5cbbf40 correctif, docs Palier3InProgress), dernier 23:03, merge_main_fait: non (branche deja a jour avec main au depart)
TESTS: aucun lance (pas de quick_check) ; TESTS_TAUTOLOGIQUES: 0
ROUGES_ATTENDUS (a la 1re execution sur c7affed) : `HeroesOfLiberty.Siege.Deploy.MountBarAdvancesFromServerPhase` et `HeroesOfLiberty.Siege.Deploy.SecondMountReturnsToPack` ; verts apres 5cbbf40.
RELECTURE_CPP: relu a la main : `NoteOrder` / `WatchElapsedMs` (HOLDeployRules.cpp), `GetDeployProgress` (HOLPlayerController.cpp:2306), `RequestToggleDeploy` (:2949), membre `DeployWatch` (HOLPlayerController.h) ; inclusion `Siege/HOLDeployRules.h` ajoutee au .h. Point a verifier a la compilation : `HOLDeploy::PhaseAt(-1)` appele par le test (indice hors plage) doit rendre Packed/valeur sans bris.
ESSAIS: aucun (pas de rejeu deploy dans l'agent).
QUESTIONS: aucune.

## Cause
(a) `GetDeployProgress` prenait `DeployWatchedSince` au PREMIER appel local : au premier dessin de la barre, 0 ms ecoules ; l'horloge ignorait l'instant de l'ordre. 0,3 s apres l'ordre : 0 dièse. (b) Entre deux phases identiques non lues (1 puis 1 sans lecture de la phase 2), `DeployWatchedPhase` n'etait pas remis a -1 : l'ancienne horloge etait reprise (barre pleine d'emblee, ou cle restee sur Mounting<n>).

## Correctif
- `HOLDeploy::FWatch` + `NoteOrder` + `WatchElapsedMs` (`Siege\HOLDeployRules.h/.cpp`) : l'ordre pose `OrderedAt` et remet `Phase` a -1 ; la premiere phase 1 ou 3 observee dans les 2 s part de `OrderedAt` ; un ordre refuse expire.
- `AHOLPlayerController::RequestToggleDeploy` appelle `NoteOrder` ; `GetDeployProgress` utilise `WatchElapsedMs` (membre `DeployWatch` remplace `DeployWatchedPhase` / `DeployWatchedSince`).
- Le serveur (DeployProcessor) n'est pas touche : la phase est seule repliquee ; l'ecart horloge locale / serveur reste de l'ordre de la latence (NON VERIFIE en reseau reel).

## A faire par la session principale
`Tools\build.ps1 -LiveCoding=0 -Wait` si BUILD n'est pas PASS ci-dessous, puis `quick_check -Parts tests` (attendu : les deux tests verts, PlayerButton inchange), puis rejeu cache `deploy` (PNG deploy_deploying / deploy_packing).

NON VERIFIE: rouge a l'execution, verts, rejeu `deploy`, comportement reseau (decalage ordre / phase), `PhaseAt(-1)` du test.
ANOMALIES_PROCESSUS: `HOLDeployRules.cpp` s'est retrouve en fins de ligne mixtes apres mon ecriture (corrige par eol_check -Fix, aucun diff git). 10 appels consommes avant le 1er Edit (L20-8 non tenu : lecture groupee de 4 outils + recherches de lignes).

BUILD_RESULTAT: NON ATTENDU (lance 22:59 en un appel, encore en cours a 23:04, appels epuises) : NON COMPILE par l'agent ; HEURE_FIN 23:05.
