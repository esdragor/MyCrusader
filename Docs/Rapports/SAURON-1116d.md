# SAURON-1116d (worktree Sauron, branche visual) : rapport

APPELS_DECLARES: 36 (arrêt à 45 vus ; le seuil de 80 se juge sur tool_uses)
HEURE_DEBUT/HEURE_FIN: 15:56 / 16:07 (Get-Date PowerShell)
PREMIER_BUILD: sans C++   BUILD: sans C++ (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: vu à 16:03 : `fixture : FAIL (raw 'Condition failed' lines printed 20, summary line 0, game lines 2)` et même chose sur le vrai journal scenario_nomad_start.log (check_log_test.ps1 contre le stub de CheckLog.ps1 = filtre d'avant) ; puis un seul commit vert 713eb37 (test + correctif) ; assertit l'effet : oui (lignes réellement imprimées par la fonction que check.ps1 appelle, comptées sur un vrai journal de scénario)
COMMITS: 3 (713eb37 outil + test, 755ba48 merge main, 5b767ff journal), dernier à 16:06, merge_main_fait: oui
TESTS: check_log_test.ps1 PASS (fixture + scenario_deploy.log, 16:04 ; refait après merge main 16:06 PASS) ; check.ps1 nomad_start PASS 46 s de bout en bout (16:04, sortie : 1re ligne `engine smoke tests: 20 'Condition failed' at start (...)`, plus aucune des 20 lignes) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (aucun Retours\E-*.md touché)
QUESTIONS: aucune

## (1) Les 3 ovales blancs (deploy, nomad_start)
Cause : c'est la **crête** de chaque place (D286, J4, D439-D444) : `Tools/Editor/maps/common.py:222-244` spawn_ridges pose 3 sphères aplaties du moteur (RIDGE_BOULDERS :108, 6,5 à 8 m de long, à -650 / 0 / +650 cm), matériau MI_Rock gris clair (ROCK_COLOR 0.33,0.31,0.29 :115), sans ombre (D258), avec NavArea_Null, **30 m devant chaque place vers le centre** (RIDGE_AHEAD = 3000, :107).
Voulue : oui pour la crête (Plain appelle spawn_ridges ; la carte grise de test n'en avait pas). **Non voulu dans deploy** : `deploy.txt:10/:17 hol.Camera.FocusNear 3000 0` (forum + 30 m dans son repère, HOLPlayerController.cpp:1186) puis `:14 hol.Units.SpawnHere` autour du foyer : l'engin naît exactement sur le rocher du milieu, dans le NavArea_Null. nomad_start : même crête vue à côté de la fondation, rien dessus.
Ligne `[ ]` en tête de Sauron (Auto_Journey_File.md l. 90) : correctif proposé `FocusNear 3000 1500` (hors crête ±1050 cm ; Maison à 27 m au lieu de 23 : portée du Mangonneau à vérifier) ; test rouge proposé : vérif python des scénarios mapSize (un SpawnHere après un FocusNear à moins de 1100 cm d'un point de ridge_points dans le repère du forum = FAIL), rouge sur deploy. Non fait (budget d'appels).

## (2) « LogAutomationTest: Error: Condition failed » (20 par lancement)
Cause : smoke tests **du moteur** `[Core][SmokeFilter]` de `C:\UE\UE_5.8_HOL\Engine\Source\Runtime\Core\Tests\Experimental\UnifiedError\UnifiedErrorTests.cpp` (CreateErrorMessage :479-509, CreateErrorMessageWithContext :512-533, etc.), lancés à chaque `-game` non cuit par `LaunchEngineLoop.cpp:4376 RunSmokeTests` ; ils comparent le message à un texte anglais (« Empty error ») alors que `-culture=fr` le traduit (« Erreur vide » dans le journal). Preuve : lancement caché `-culture=en -nullrhi` à 16:0x : 0 ligne, « [Empty error] » ; tous les journaux `-culture=fr` du worktree : 20 lignes.
check.ps1 ne doit PAS les compter (ni le jeu ni le scénario ; aucun test du projet n'a SmokeFilter). Mais il les IMPRIMAIT (20 lignes `Error:` dans sa sortie, d'où les « anomalies » de SAURON-1116c et CELEBORN-1116a). Correctif 713eb37 : `Tools/Scenarios/CheckLog.ps1` (Select-CheckLogLines) remplace les 20 lignes par une ligne qui les nomme, garde toute autre erreur ; check.ps1:83 l'appelle ; commentaire de cause ajouté à preflight.ps1:123-125.
Ligne `[x]` en tête de Sauron (l. 91). Reste : preflight.ps1:126 exclut TOUT `LogAutomationTest: Error` (pourrait se restreindre à `Condition failed`).

FAIT / RESTE:
- FAIT : cause des ovales (crête D286) ; cause des Condition failed (smoke tests moteur sous -culture=fr) + outil corrigé 713eb37 ; 2 lignes en tête de Sauron dans Auto_Journey_File.md ; VisualInProgress.md (5b767ff) ; EtapeDefini.md ligne 416.
- RESTE : correctif de deploy (ligne l. 90, sans C++) ; restreindre l'exclusion de preflight.ps1:126. Branche visual prête (main fusionné).
NON VÉRIFIÉ: portée du Mangonneau pour FocusNear 3000 1500 ; le décompte exact des 20 CHECK anglais par test (vérifié : 0 en, 20 fr, mais pas assertion par assertion) ; preflight.ps1 complet non relancé (seul check.ps1, qu'il appelle, l'a été) ; EtapeDefini.md n'apparaît pas dans `git status` de la principale après l'ajout (edit_line a dit « ligne 416 (ajout) » : fichier peut-être non suivi ou déjà commité ailleurs, non regardé).
PROCESSUS_DETACHES: « aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron »
ANOMALIES_PROCESSUS: un lancement caché hors outil (UnrealEditor -game -nullrhi -culture=en -saveddirsuffix=Probe, journal dans le scratchpad), arrêté par Stop-Process après 61 s ; aucune autre.
GIT_STATUS_PRINCIPALE: ` M Auto_Journey_File.md`, ` M Auto_Journey_Jour.md`, `?? Docs/Priorite_Socle_Factions_2026-11-16.md`, `?? Docs/Rapports/` ; inchangée hors docs voulus : oui (mes écritures : 2 lignes d'Auto_Journey_File.md, 1 ligne d'EtapeDefini.md, ce rapport)
