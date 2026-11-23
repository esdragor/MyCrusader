# ARWEN-1123h (11:24) : wall_line_long geste reel, NON ABOUTI (limite d'appels)

APPELS_DECLARES: 17 (arret a la limite de 18 de l'enonce)
HEURE_DEBUT/HEURE_FIN: 11:19 / 11:24
PREMIER_BUILD: sans C++   BUILD: sans objet   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenario seulement)   TEST_ROUGE_SEPARE: non (pas de code)
COMMITS: 0, merge_main_fait: non
TESTS: preflight wall_line_long lance UNE fois : FAIL (timeout step 29 waitlog placement, pas de PNG) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: wall_line_long : preflight FAIL, aucun PNG, LOGIQUE SEULEMENT. Retours\E-wall_line_long.md NON modifie (reste LOGIQUE SEULEMENT, GESTE: interne). Pas de commande play.ps1 donnee.
QUESTIONS: aucune
FAIT: lu Controller (PressGridKey :2536, grille D = Military slot 6 age 0 = Palisade, HOLCommandGrid_Test.as:532 ; hol.Select.Grid <0..11> = A..V ; hol.Mouse.Drag HOLUIMouse.cpp:344 deja dans le binaire PASS). Essai du scenario : hol.Select.Own / frames / hol.Select.Subgroup / hol.Select.Grid 0 / hol.Select.Grid 6 / waitlog placement a la place de "waitkey + hol.Build.Place Palisade". Resultat : AUCUNE ligne "Grid:" dans le log (ni "age 0 chosen"), le mode placement n'est jamais entre, timeout. Scenario restaure (git checkout), aucun fichier modifie.
RESTE (piste pour l'agent suivant) : (1) la selection n'atteint pas la grille : hol.Select.Own (HOLRpcPlayer.as:623, commande peut-etre de script RPC et non du jeu local) ne selectionne pas / hol.Select.Subgroup bascule peut-etre sur le heros ; ajouter hol.Select.Report apres chaque etape pour voir la selection ; essayer a la place hol.Select.Group, ou un clic reel hol.Select.LeftClickGround sur un Paysan, ou un spawn dedie hol.Units.SpawnHere 5 Peasant puis selection par hol.Select.OrderNear/Army. (2) La grille d'un Paysan doit afficher "Grid: age 0 chosen" apres Grid 0, puis Grid 6 doit mener a BeginPlacement(Palisade). (3) Puis hol.Mouse.Drag 200 540 1700 540 90 (pixels non verifies), captures during/after, ouvrir les PNG. Ligne S3-b de Auto_Journey_File.md et ligne EtapeDefini.md : NON mises a jour (limite d'appels).
NON VERIFIE: le geste D reel ; la selection des Paysans ; le glisser reel ; l'effet visuel au geste reel.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
ANOMALIES_PROCESSUS: premier Edit au 10e appel (lectures de recherche avant : syntaxe Grid / Select inconnue) ; limite de 18 appels atteinte avant la selection des Paysans ; heure Get-Date 11:19 a 11:24.
GIT_STATUS_PRINCIPALE: inchange hors docs voulus (non relance)
