# ARWEN-1124d : reste du défaut RECETTE-1123pa (b), la zone de soin rend des PV

Impact joueur : la Lumière sacrée soigne bien, et c'est maintenant prouvé par un scénario rejouable (total soigné 0.00 puis 705.00, PV du héros 180 puis 330 / 450).

APPELS_DECLARES: 19 (arrêt à 25 vus ; 15 min)
HEURE_DEBUT/HEURE_FIN: 11:10:48 / 11:19
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun C++, aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scénario de preuve d'une règle déjà verte, pas de correctif) ; assertit l'effet : oui (total soigné lu au journal par waitlog sur la ligne exacte, PV du héros vus sur PNG)   TEST_ROUGE_SEPARE: non (aucun correctif à séparer : le défaut de recette était l'absence de preuve)
COMMITS: 1 (604a7b2, 11:18), merge_main_fait: non (non demandé ; branche integ-37 prête)
TESTS: preflight.ps1 sacred_light_heal -Root worktree : 1er passage FAIL uniquement pour fichiers non commités (attendu), 2e passage PASS ; total soigné identique aux 2 passages (0.00 puis 705.00). Aucun quick_check (non demandé). TESTS_TAUTOLOGIQUES: 0 (705.00 est calculé par HOLHealProcessor::TotalHealed, ligne 494, pas une constante recopiée)
ESSAIS: sacred_light_heal : preflight PASS, 2 PNG ouverts (1er passage), LOGIQUE SEULEMENT (preuve de règle ; GESTE réel, SON non entendu). Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 sacred_light_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
QUESTIONS: aucune
FAIT / RESTE:
- Constat : `hol.Units.Report` n'imprime pas les PV (DescribeUnits, HOLUnitSubsystem.cpp:1426) ; mais `hol.Heal.Report` imprime `healed total` (HOLHealProcessor::TotalHealed) et la boucle de zone y ajoute chaque PV rendu (HOLHealProcessor.cpp:494). Donc ni cvar ni ligne de journal neuve : le scénario lit l'effet.
- Fait : Tools/Scenarios/sacred_light_heal.{txt,json,look} (5 Hommes d'armes + héros blessés à 40 %, vrai clic case + vrai clic sol, 6 s) : `Heal report: links 0, healed total 0.00` avant, `healed total 705.00` après (waitlog sur les lignes exactes) ; héros 180 -> 330 / 450 ; Honneur 100 -> 75 -> 89.
- Aucune ligne de journal ajoutée : grep de quick_check (L22-3) sans objet.
- Ligne 35 de Auto_Journey_File.md (ligne 37 du fichier) cochée ; EtapeDefini.md ligne ajoutée ; fiche Retours/E-sacred_light_heal.md.
- Reste : PV par soldat non lus (Units.Report sans PV) ; soin ennemi / allié hors zone non testé ; Placeholders.txt non touché (aucun provisoire ajouté ni retiré).
NON VÉRIFIÉ: PV par soldat ; 2e passage : PNG non rouverts (seul le journal 705.00 reconfirmé) ; serveur dédié ; la zone de 5 s est finie au 2e cliché (pas de cercle doré visible sur after).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Arwen
ANOMALIES_PROCESSUS: aucune ; IN_USE.txt remis à « libre ».
GIT_STATUS_PRINCIPALE: docs modifiés/nouveaux seulement (AubeInProgress, Auto_Journey_File, EtapeDefini, Retours/E-sacred_light_heal.md et fichiers d'autres agents) : oui, aucun fichier de code touché.
