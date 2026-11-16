# [LOGIQUE SEULEMENT] E-nomad_ai : partie Nomade naturelle contre une IA Normale Nomade : elle fonde seule son premier Forum là où sont ses Paysans

STATUT : LOGIQUE SEULEMENT (preflight PASS, mais le PNG ne montre PAS ce que le .look exige : un forum EN CHANTIER ; il est déjà achevé, voir BUG soupçonné)
GESTE: interne (l'IA pose seule ; caméra par hol.Camera.Focus, aucun geste du joueur)
SON: non entendu (-nosound)
branches: maps

Pré-vol (2026-11-15 14:08, agent HALDIR-1115h, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, main 6a77f35, commit 301ee1f) : `preflight.ps1 nomad_ai -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir -AllowUncommitted` PASS (34 s, 1 PNG). Premier jet (1f434da, caméra FocusSlot 1) : FAIL (pas de capture : la hauteur 3000 cm jamais atteinte).
PNG OUVERTS et vus :
- nomad_ai_forum : au centre du cadre, le Centre principal de l'IA (drapeaux rouges) ENTIER, comme un bâtiment achevé ; un Paysan rouge en haut à droite ; minicarte sans brouillard noir. Pas de fondation visible.
Journal de la partie : « AI slot 1 step 4: Nomad forum founded at (-14000.0, -16800.0), group 0, 5 Peasants sent », « Construction: slot 1 placed MainCentre foundation at (-14000, -16800), paid ... W 150 S 400 », puis « Construction: slot 1 finished MainCentre at (-14000, -16800) after 1 steps of work (by builders: 1 x1) », et l'IA produit aussitôt des Paysans et pose Camp de bûcherons et Moulin.
BUG soupçonné (non corrigé, C++ probable) : le forum du Nomade s'achève après 1 pas de travail (temps de construction du MainCentre nul ou presque ?). Non vérifié pour le joueur humain (nomad_start capture la fondation juste posée).
DIAGNOSTIC (HALDIR-1115i, 2026-11-15 14:2x, lecture du code) : CAUSE = la table. `Tools\Data\BuildingTypes.csv` ligne MainCentre : `BuildTime` = 0 (un forum standard est posé fini) ; `AHOLBuilding::GetTotalWork` -> `HOLConstruction::GetBuildSteps(0, pas)` = max(1, 0) = 1 pas ; `HOLBuildProcessor` l'achève donc au premier coup. Aucun cas particulier Nomade ni IA : le forum du JOUEUR Nomade (nomad_start) s'achèverait de même au premier coup de Paysan. Le GDD (D402) écarte « forum achevé à la pose » mais ne donne AUCUNE durée : question posée `Retours\Q-nomad-forum-duree.md` [À RÉPONDRE] (recommandation A : 75 s pour 1 Paysan). Rien corrigé en attendant (une ligne CSV à changer).
NON VÉRIFIÉ : effet joueur vu à l'écran (déduit du code seulement) ; brouillard absent dans ce scénario (la minicarte montre tout).

Commande complète (règle 13, 2026-11-14) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 nomad_ai -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Durée : environ 1 min (l'événement est déclenché d'office par l'IA, aucune touche). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
