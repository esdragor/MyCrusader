# HALDIR-1121sh : essai du centre secondaire (D879), suite de 1121sg

Impact joueur : l'essai E-secondary_centre est listé [À ESSAYER] : on pose le Centre secondaire (Z, Z, clic), les Paysans le bâtissent, il se sélectionne seul ; deux écarts à connaître (infobulle de la case Z absente en capture, grille du centre limitée à A Paysan).

APPELS_DECLARES: 17 vus
HEURE_DEBUT/HEURE_FIN: 20:26:26 / 20:30:28
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scénario seul) ; assertit l'effet : oui (waitlog "Selection: SecondaryCentre of slot 0 (own)", "finished SecondaryCentre")
COMMITS: 3 (0adada2 scénario, e6d4fcc correctif cadrage caméra, 213b3a8 notes exactes), dernier à 20:30, merge_main_fait: non (branche wall-line = main 204f769, rien à fusionner)
TESTS: preflight secondary_centre PASS (43 s puis 46 s, sanity ok, aucun warning) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: secondary_centre : preflight PASS, 5 PNG ouverts, VU À L'ÉCRAN (hors infobulle) ; `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 secondary_centre -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` ; GESTE: réel, SON: non entendu
QUESTIONS: aucune

FAIT :
- Commit des 3 fichiers du scénario (preflight exigeait l'arbre propre).
- Étape 4 corrigée : `hol.Camera.FocusNear 0 -4500` remplacé par `hol.Camera.Focus -16800 -6000` (absolu) avant `hol.Select.Click 0.5 0.5` ; journal : "Selection: SecondaryCentre of slot 0 (own)", "Select report: selected SecondaryCentre of slot 0 (own)".
- PNG vus : sc_grille_age2 (page II, case Z = Centre secondaire), sc_pendant (fondation fil de fer, 5 Paysans en marche, bois -150 / pierre -400), sc_apres (centre fini 2400/2400 PV, seul sélectionné, grille A Paysan, "Bâtiment achevé"), sc_forum (Centre principal : A Paysan + D/F voies de palier + W cloche).
- Retours\E-secondary_centre.md créé ([À ESSAYER], GESTE: réel, SON: non entendu, branches: wall-line, commande complète).
- Notes du .txt et du .look corrigées (elles prétendaient "même grille que le forum", faux).

RESTE :
- Infobulle de la case Z : sc_grille_infobulle ne la montre pas (souris sur GridCell01, rien affiché) : à voir à la main ou ajouter un `wait` plus long / vérifier que hol.Mouse.Hover déclenche l'infobulle des cases de grille.
- Grille du centre secondaire = seulement A Paysan (le forum a en plus D/F voies de palier, W cloche) : à trancher par le GDD si voulu (non tranché ici).
- sc_pendant : pas de barre de construction visible (Paysans encore en marche à la capture) ; un `wait` avant la capture le montrerait.
- EtapeDefini.md et SecondaryCentersInProgress.md non mis à jour (hors périmètre de fichiers donné).

NON VÉRIFIÉ : infobulle de la case Z ; barre de construction en cours ; son ; modèle SM_Forum provisoire ; équivalence de grille avec le forum (infirmée) ; EtapeDefini / InProgress.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
ANOMALIES_PROCESSUS: aucune (preflight 43 à 49 s par passage)
GIT_STATUS_PRINCIPALE: hors docs voulus (Docs\Rapports, *.md, Placeholders.txt) aucune ligne de code modifiée par moi ; seuls fichiers écrits en copie principale : Retours\E-secondary_centre.md et ce rapport : oui
