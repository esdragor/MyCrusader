# HALDIR-1121si : grille et infobulle du Centre secondaire

Impact joueur : l'infobulle de la case Z (Centre secondaire : 150 bois, 400 pierre, +10 population, dépôt, produit Paysan) s'affiche bien en jeu ; c'était le scénario qui visait la mauvaise case. La grille du centre fini reste A Paysan seul en attendant ta réponse à Q-secondary-centre-grid.

APPELS_DECLARES: 24 vus
HEURE_DEBUT/HEURE_FIN: 20:31 / 20:36
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scénario seul) ; rouge vu a 20:34 : preflight FAIL "timeout at step 61" après le 1er correctif (la pose dépendait de la position du curseur), puis vert à 20:35 ; assertit l'effet : oui (waitlog "Mouse hover AgeCell101", "tooltip shown", "Selection: SecondaryCentre of slot 0 (own)")
COMMITS: 2 sur wall-line (hover AgeCell101 ; curseur garé sur GridCell01 avant la pose), dernier à 20:35, merge_main_fait: non (rien à fusionner)
TESTS: preflight secondary_centre PASS (40 s, sanity ok, aucun warning) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: secondary_centre : preflight PASS, sc_grille_infobulle ouvert et VU (infobulle présente), VU À L'ÉCRAN ; GESTE: réel, SON: non entendu ; `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 secondary_centre -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
QUESTIONS: Retours\Q-secondary-centre-grid.md (valeur codée : A = Paysan seul)
FAIT / RESTE:
- Grille : GDD (D384, D879) ne dit pas D/F/W pour le centre secondaire : question posée, rien changé.
- Infobulle : cause = mauvais widget survolé dans le scénario (GridCell01 au lieu de AgeCell101) ; données et textes corrects. Correctif de scénario, 2 commits.
- RESTE : EtapeDefini.md et SecondaryCentersInProgress.md non touchés ; sc_pendant sans barre de construction.
NON VÉRIFIÉ: son ; barre de construction ; PNG sc_apres du dernier passage non rouvert (vu au passage précédent) ; DIAGNOSTICS_AS non relus (aucun .as touché).
PROCESSUS_DETACHES: aucun (preflight termine)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: seuls écrits : Retours\Q-secondary-centre-grid.md, Retours\E-secondary_centre.md, ce rapport : oui
