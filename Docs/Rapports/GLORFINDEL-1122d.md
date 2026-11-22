# GLORFINDEL-1122d : follow_short vise juste, ROUGE : FindTargetAt ne voit que les unites hostiles (le pont n'est jamais appele)

pont prouve actif : non
suivi vu : non
branche prete : non (ability-columns, tip 7f50e59 ; pas de C++ ; merge main non fait ; scenario rouge, a ne pas lister [A ESSAYER])

Impact joueur : le clic droit sur un allie (ou un Paysan a soi) ne declenche NI Suivre NI Soigner (Moine) : l'unite marche au sol. Le correctif est du C++ (lot groupe).

APPELS_DECLARES: 18 vus (arret a 25 ; premier Edit/Write au 11e appel : consigne « avant le 8e » non tenue, 10 appels de lecture)
HEURE_DEBUT/HEURE_FIN: 16:57 / 17:02 (Get-Date PowerShell)
PREMIER_BUILD: sans C++   BUILD: sans build   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: scenario follow_short, commit 7f50e59 : rouge vu a 16.00.17 UTC (journal : « Test sequence: timeout at step 22: waitlog FollowBridge: follow started ») ; assertit l'effet : oui   TEST_ROUGE_SEPARE: non (aucun correctif Angelscript possible : la cause est dans le C++)
COMMITS: 1 (7f50e59, 17:02), merge_main_fait: non
TESTS: 1 passage de `play.ps1 follow_short -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -TimeoutSec 150` (150 s, le jeu a ete ferme par play.ps1 : echec de sequence a 60 s) ; TESTS_TAUTOLOGIQUES: 0 ; TESTS_SCRIPT_AJOUTES: 0
ESSAIS: follow_short : preflight NON lance, PNG follow_short_start OUVERT et VU (4 Hommes d'armes groupes a gauche du centre, le Paysan seul a droite ; walk, walk2, cancelled non produits) : LOGIQUE SEULEMENT, ROUGE
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ de la seance : voir ci-dessous)   TYPES_A_EXPOSER: aucun

## (1) Visee : stable, prouvee a 14 cm
Verbe retenu : `hol.Units.Spawn <n> <x> <y> <slot> <Type> <rayon>` (HOLUnitSubsystem.cpp:88-107, point ABSOLU en cm) a la place de SpawnHere (qui disperse autour du focus camera, cause du rate de 1122c). Hommes : `hol.Units.Spawn 4 -14800 2500 0 ManAtArms 100` ; Paysan : `hol.Units.Spawn 1 -14800 4500 0 Peasant 20`. Journal : « Spawn: 1 Peasant of slot 0 around (-14800, 4500) ». Releve : `#5 slot 0 at (-14810, 4490) Idle`, hommes #1-#4 vers (-14719..-14795, 2470..2538). Le clic `hol.Select.OrderNear 0 4500` a vise (-14800, 4500) (« Right click near the centre at (-14800, 4500) ») = a 14 cm du Paysan, marge de FindTargetAt non en cause.
Cible mobile (3) : scenario ecrit pour que le Paysan marche apres le debut du suivi (`hol.Select.ClickUnit 5` puis `hol.Select.OrderNear 3000 5500`, 2 releves 4 s et 9 s) ; non rejoue, la sequence n'a pas passe l'etape 22.

## (2) Resultat du passage (journal, UTC ; 1 seul passage, cause lue, les suivants n'auraient rien change)
Lignes « FollowBridge: » : AUCUNE (seul l'echo du timeout). Chemin pris : « Order: 4 units to (-14792, 4496) » puis « RpcOrder: move received by the server » = chemin SOL, pas chemin unite. Distances suiveur-cible : NON MESURABLES (un seul releve). Erreurs : « Angelscript.*rror » = 0 ; « requires WorldContext » = 0.
Captures : follow_short_start (16:59:17, ce passage) ouverte et vue ; les autres non produites.

## CAUSE (ROUGE, lue dans le code, aucun C++ corrige)
HOLPlayerController.cpp:1177-1187 : `FindTargetAt` appelle `GatherPickCandidates(false, Candidates, UnitIds, /*bHostileOnly*/ true)` (:1184) ; le filtre de `GatherPickCandidates` (:1100) saute toute unite `!Camps.AreHostile(...)`. `FindTargetAt` ne peut donc renvoyer QUE l'id d'un hostile : pour un allie ou une unite a soi, `UnderId == 0`. Or les deux branches suivantes exigent un id NON hostile :
- Soigner (:1440-1445) : `UnderId != 0 && ... && !Camps.AreHostile(...)`,
- Suivre (:1459-1461) : `UnderId != 0 && ... && !Camps.AreHostile(...)` puis `NativeRightClickOnUnit` (:1465).
Condition jamais vraie : le pont n'est jamais appele, le clic part en ordre de deplacement au sol. L'ajout `bHostileOnly` vient du commit 8fcb0f4 (« Allies no longer fight each other ... right-click go by camp »), et les branches Soigner / Suivre ont ete ecrites apres sans voir ce filtre. La marge (`TargetMargin`, GetScaledMargin) n'est pas en cause.
Le clic par `hol.Select.ClickUnit ... order` passe, lui, par le livre script (HOLControllerCommandsC10a.as:248-260, UnitScreenPoint) et ne prouve PAS le pont C++.

## RESTE / lot C++ (une ligne [ ] ajoutee a Auto_Journey_File.md, worktree Glorfindel)
Dans le lot C++ : un `FindAllyAt` (gather avec `bHostileOnly=false` mais en excluant les ennemis, hors unites de la selection) utilise par les branches Soigner (:1440) et Suivre (:1459), ou `FindTargetAt` avec un parametre ; puis rejouer follow_short (le scenario est pret : positions absolues, cible mobile, attente des deux lignes de pont). A noter : le Soigner de Q-chapelle-moine = A est donc lui aussi mort en clic reel.
Hors tache : play.ps1 a ferme le jeu a 150 s (TimeoutSec) alors que l'echec de sequence etait a 60 s : le jeu masque ne quitte pas sur echec de sequence (deja note en 1122c).

NON VERIFIE : le pont s'execute-t-il (classe script concrete, HOLFollowBridge.cpp), l'unite suit-elle, l'ordre manuel annule-t-il ; la cible mobile (ClickUnit 5 pris pour le Paysan : non verifie) ; preflight.
DIAGNOSTICS_AS: non relus dans VS Code ; journal du jeu : 0 erreur Angelscript.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: aucune (play.ps1 a ferme 3 processus du jeu masque a 150 s)
GIT_STATUS_PRINCIPALE: modifs de docs seulement (EtapeDefini.md, Auto_Journey_File.md, ce rapport, autres sessions) : oui
