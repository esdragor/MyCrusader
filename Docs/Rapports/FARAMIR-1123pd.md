# FARAMIR-1123pd (S3-a, tower_wood)
HEURE_DEBUT/FIN: 16:26 / ~16:36 ; APPELS: >25 (depasse : edits multi-lignes refuses sur fichiers CRLF, python3 bloque) ; premier Edit tardif
COMMITS (zone-effect, NON COMPILES) : b0d7ceb (test rouge + bouchons) ; 6c04d1c (code) ; 3e commit = scenario tower_wood.txt/.look (non joue)
LIGNE DE LANCER : "Combat: building shot step %lld: #%u %s looses arrow %d of %d at #%u %s, flight %d ms (%d steps)" (HOLCombatProcessor.cpp, boucle Shot du tir de batiment) ; waitlog "Combat: building shot"
CVARS : hol.Combat.LogBuildingShots (0), hol.Arrow.Scale (1), hol.Arrow.SlowMotion (1) ; statiques UHOLArrowSubsystem::GetArrowScale / GetFlightDurationSeconds ; compteur UHOLCombatProcessor::GetTotalBuildingShotsLogged
TESTS_C++ : HeroesOfLiberty.Combat.BuildingShotLaunchLog ; HeroesOfLiberty.Combat.ArrowScaleCvar (rouge attendu au bouchon : compteur jamais incremente, echelle non appliquee)
BUILDS_DEMANDES: 1 (lot C++) : Tools\build.ps1 -LiveCoding=0 -Wait puis quick_check / tests "HeroesOfLiberty.Combat.BuildingShot" et "ArrowScaleCvar" puis play.ps1 tower_wood -Hidden -Root <worktree>
RELECTURE_CPP: .generated.h inchange, statiques publiques, pas de case, tests sous WITH_DEV_AUTOMATION_TESTS, bouchon retire au commit 2
NON VERIFIE: compilation, rouge a l'execution, tests, scenario (aucune fleche vue), merge main non fait (consigne)
ANOMALIES: python3 en arriere-plan (bash bh1qybudw) a tuer : stray_processes ; Placeholders : rien d'ajoute.
