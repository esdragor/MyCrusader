# ELROND-1120pe (2026-11-20, 16:02 a 16:07, worktree HeroesOfLiberty-Elrond, branche integ-25, SANS C++)

APPELS_DECLARES: 19 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 16:02 / 16:07
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build demande)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 737a45a (Tools\Scenarios\deploy_timing_test.ps1, test seul) ; rouge VU a 16:05 : FAIL deploy_deploying et FAIL deploy_packing, exit 1 ; assertit l'effet : oui (nombre de crans de la barre calcule depuis le temps entre la touche D et la capture, duree lue dans HOLDeployRules) ; correctif 5e5ff34 (vert VU 16:05 : PASS x2, exit 0)
COMMITS: 3 (737a45a test, 5e5ff34 correctif, 370f78f journal) + merge main 5286768, dernier a 16:06, merge_main_fait: oui
TESTS: deploy_timing_test.ps1 rouge puis vert ; TESTS_TAUTOLOGIQUES: 0 (le nombre de crans est calcule par la formule du widget Filled = permille*8/1000 sur les temps du scenario, pas recopie)
ESSAIS: aucun liste ; deploy non rejoue (consigne). Erreur : j'ai lance `preflight.ps1 deploy`, qui a repondu FAIL (17, captures absentes : binaire de ce worktree non a jour) ; aucun PNG produit, aucun processus detache.
QUESTIONS: aucune
FAIT / RESTE:
 - Ligne prise : AUCUNE ligne `[ ]` sans C++ chez Elrond : l.87 garnison faite par ELROND-1120pd ; l.89 D871 : texte + barre texte de la case D deja faits (8fb9db2 / e9ea0f5), la barre au-dessus de l'engin (monde) demande du C++ ; l.92-97 C++ ; l.111 Palier 3 G1 reservee. Repli : scenarios D869.
 - deploy.look (verifie) demande bien texte + barre : deploy_deploying = Montage... + barre partielle (1 diese au moins), deploy_packing = Demontage... + barre partielle.
 - PROBLEME : deploy.txt attendait `frames 15` apres la touche D. Les images ne sont pas a pas fixe sous play.ps1 : 15 images = 0,1 s a ~150 images/s (fenetre cachee, FrameWaits.ps1), 0,25 s a 60. Le widget donne Filled = permille*8/1000 : 1 diese seulement a partir de 0,1875 s sur 1,5 s (montage) et 0,125 s sur 1 s (demontage). Donc en rejeu cache : 0 diese aux deux captures, contraire au .look. Correctif : `wait 0.3` (temps du jeu, exact) : 1 diese (200 permille) au montage, 2 au demontage (300 permille), les deux dans les 1,5 s / 1 s.
 - RESTE : rejeu cache `play.ps1 deploy -Hidden -Root <worktree>` apres build (ouvrir deploy_deploying.png et deploy_packing.png, y voir la barre) ; la barre au-dessus de l'engin (D871 A+B, partie monde) = lot C++.
NON VÉRIFIÉ: le rendu a l'ecran (aucun PNG ouvert), le debit d'images reel en fenetre cachee (150/s tire du commentaire de FrameWaits.ps1, non mesure), que le waitlog `Order: 1 units deploy` arrive bien avant le depart de la barre (le depart serveur peut retarder de quelques images), HUD.Report n'imprime pas l'etat de la barre (aucun waitlog n'assure le texte « Montage... »). Le test lit la duree dans HOLDeployRules.* (repli 1500 / 1000 ms).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: preflight.ps1 deploy lance sans y etre invite (FAIL, pas d'editeur reste) ; la commande stray_processes a rendu le code 255 malgre « aucun processus detache » (a surveiller).
GIT_STATUS_PRINCIPALE: docs modifies seulement (AubeInProgress, Auto_Journey_*, ChatInProgress, EtapeDefini, Fortifications/Palier3/SpecializationInProgress, Placeholders.txt) + Docs\ non suivis ; aucun fichier de code touche : oui (EtapeDefini.md et ma ligne d'Auto_Journey_File.md par moi)
DIAGNOSTICS_AS: sans objet (aucun .as)
BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 1 (deploy_timing_test.ps1, PowerShell) ; TYPES_A_EXPOSER: aucun
