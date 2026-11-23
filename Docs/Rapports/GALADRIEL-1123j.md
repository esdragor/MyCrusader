# GALADRIEL-1123j : D889 largeur reelle (worktree Galadriel, specialization-trial)

APPELS_DECLARES: 20 (arret a 22)
HEURE_DEBUT/HEURE_FIN: 10:47 / 10:58
PREMIER_BUILD: 10:50   BUILD: PASS (61 s)   LAUNCHCHECK_DEBUT: binaire plus vieux que HOLGridCellStatusTests.cpp (edite 10:50) : build lance en premier
TEST_ROUGE: NON VU a l'execution (temps : build + quick_check 197 s + preflight ne laissaient pas place a un build bouchon) ; test ajoute dans UI.GridCell.StatusFit (51 px puis 76 px, 2 lignes attendues ; 1000 px = 1 ligne ; JA et mot unique = 1 ligne), assertit l'effet (GetStatusLineCount apres FitStatusToWidth) : oui   TEST_ROUGE_SEPARE: non (un seul commit 5a98bf3, test + correctif ; pas de bouchon)
COMMITS: 1 (5a98bf3 a 10:55), merge_main_fait: non
TESTS: quick_check.ps1 (worktree) -Parts tests : 2118/2118 PASS en 197 s ; TESTS_TAUTOLOGIQUES: 0 (le nombre de lignes est calcule par FitStatusToWidth sur la mesure de la police de la case ; mesure : la meme estimation que le code si pas de renderer en automation, non verifie)
ESSAIS: tech_soon (le seul nom : pas de tech_soon_en / tech_soon_720, les points _en/_ja/_720 sont dans tech_soon) : preflight PASS, 4 PNG generes, _en et _720 ouverts. VU : la case W affiche « Bientôt » (FR) sur sa ligne, grise, entier, MEME dans les PNG _en et _720 (nom EN « Tempered Halberds » sur 3 lignes) ; « Coming soon » n'apparait dans aucune case : le statut n'est pas localise en EN dans ce scenario (-culture=fr). La coupure D889 n'est donc PAS vue a l'ecran : LOGIQUE SEULEMENT. (Le rapport 1123i parlait de « Coming soon » sur une ligne : non retrouve dans ces PNG.) Aussi : « Halberdier » de la case A est rogne a droite en EN (« Halberdie »), non traite.
QUESTIONS: aucune
FAIT / RESTE: FAIT : HOLGridCell : FitStatusToWidth(CellWidth) decide 1 ou 2 lignes sur CellWidth - 2*StatusSidePad avec MeasureTextWidth (police courante), appele par SetStatus (derniere largeur connue, 76 par defaut) et par NativeTick avec LocalSize.X (chaque tick, defait la coupure si la case s'elargit) ; RefreshNameZone extrait de SetStatus. RESTE : jouer un scenario ou le statut est reellement « Coming soon » (culture en) pour VOIR 2 lignes a 1920 et 1280 ; question de design restee ouverte : si la police reelle mesure <= 72 px a 76 la case garde UNE ligne (le test pose 2 lignes aux deux largeurs avec la mesure du harnais).
NON VERIFIE: Coming soon a l'ecran ; red a l'execution ; mesure reelle vs estimation en automation ; RELECTURE_CPP (L20-2) faite en lecture seule du diff, pas d'agent relecteur ; MoraleInProgress.md non touche (diff d'un autre agent).
BRANCHE PRETE: build + tests verts, mais D889 non vu a l'ecran : tip 5a98bf3
PROCESSUS_DETACHES: voir derniere sortie
ANOMALIES_PROCESSUS: quick_check passe en tache de fond apres 180 s (notification recue, pas d'attente par boucle)
GIT_STATUS_PRINCIPALE: docs seulement : oui