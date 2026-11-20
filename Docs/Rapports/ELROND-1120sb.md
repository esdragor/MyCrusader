# ELROND-1120sb

ASSERTION (message exact du log d'integration non conserve : integ-34.log donne seulement `all green (failed: HeroesOfLiberty.Siege.Deploy.SecondMountReturnsToPack)`) : deduit par lecture du test (HOLDeployButtonTests.cpp, ~l.710) : l'assertion `deployed: no bar, the face goes back to Pack` attendait -1, lit 0.
CAUSE : bug du TEST. L'aide `PermilleAt` faisait `HOLDeploy::PhaseAt(ObservedPhase)` avec ObservedPhase = -1 ; PhaseAt tombe dans `default` = Packing, donc ProgressPermille(Packing, elapsed 0) = 0 et non -1. Le vrai flux (AHOLPlayerController::GetDeployProgress, HOLPlayerController.cpp:2306-2336) rend false apres WatchElapsedMs quand Phase < 0, sans jamais appeler PhaseAt(-1). Le correctif de 5cbbf40 (FWatch/NoteOrder) est correct ; le 2e cycle se relance bien (NoteOrder remet Phase=-1).
CORRECTIF : 77a229e (Fix: ...), dans le test seul : l'aide mappe -1 sur Deployed (phase 2, pas de barre), comme le controleur. Aucune assertion modifiee ni affaiblie.
BUILD : PASS (HeroesOfLibertyEditor Development, Elrond, 17 s, 23:32) ; file vide (Docs\BuildQueue.state de longueur 0, aucun ticket .json).
RELECTURE_CPP : relu, 2 lignes du test (les deux aides PermilleAt), une pure ligne de test, aucun code de production touche.
NON VERIFIE : les tests n'ont pas ete lances (pas de quick_check demande) : a lancer `quick_check -Parts tests` ; le 1976e test doit passer. Non rejoue : scenario deploy.
PROCESSUS_DETACHES : aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
Commits : 77a229e (fix), puis commit docs journal. merge_main_fait : non. Pas de push.
