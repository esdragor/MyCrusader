# GALADRIEL-1118v : scénario d'essai du panneau H-1 (bannières d'attente et d'absence) (2026-11-18, reprise 12, étape 4)

Impact joueur : aucun changement visible ; la bannière d'absence du script est identique à celle du C++ (place, taille, fond, texte, journal).
Aucun C++ ni script de jeu modifié, aucun build. 4 pré-vols lancés sur 5 permis, un à la fois, masqués.

## Scénario
`Tools\Scenarios\ecran_hud_bannieres.txt/.look/.json` dans Galadriel, commit `269d572` sur galadriel-tab (json copié d'alert_population : Plain, Small, 1 humain). Étapes : camera sur le forum, capture `hud_banniere_aucune`, waitkey, `hol.HUD.Click ScenarioClose` (le panneau du scénario est exactement à la place de la bannière : sans cela il la cachait, vu au 1er pré-vol), `hol.Net.FakeAbsent Bob 90`, waitlog `Absence banner: shown, Bob`, capture `hud_banniere_absence`, `hol.Net.FakeAbsent Bob -1`, waitlog `de retour`, capture `hud_banniere_retour`, waitlog `Absence banner: hidden`, capture `hud_banniere_fin`, `hol.Net.FakeAbsent` (efface), notes. Durée jeu 12 s plus la touche.
`hol.Net.FakeAbsent [Nom] [Secondes=161]` (HeroesOfLibertyGameModeBase.cpp:82-123) : serveur ou solo en partie, pose une fausse notice d'absence (`SetFakeAbsentNotice`), ni IA ni camp perdu ; Secondes < 0 : « Nom : de retour » 5 s ; sans nom : efface. `Leave/End/Back <Slot>` : la vraie chaîne d'un siège nommé (inutile ici). `hol.Disconnect.Report` : lignes de LogHOLMatch « Disconnect report », non utilisé.
Bannière d'ATTENTE : non provoquable en solo (il faut un lobby avec un joueur nommé attendu, `IsWaitingForPlayers`) ; couverte par quick_check net (capture qcnet_2_wait).

## Pré-vols
1. APRÈS Galadriel 23:04 : check PASS mais FAIL « HUD panels cover each other : scenario x AbsencePanel » (panneau du scénario sur la bannière : défaut du scénario, corrigé par le clic ScenarioClose).
2. APRÈS 23:05 : check PASS, FAIL seul « uncommitted file » attendu.
3. AVANT Haldir 23:06 (fichiers copiés puis supprimés ; `git status` Haldir : seulement `Content/Maps/Crown.umap` non suivi, pas de moi) : check PASS (34 s), 4 PNG, FAIL uncommitted attendu.
4. APRÈS 23:07 sur scénario commité : `preflight ecran_hud_bannieres : PASS`.
PNG des deux séries dans le worktree ; AVANT copiés dans `...\scratchpad\hud_bannieres_avant\`, APRÈS dans `...\hud_bannieres_apres\`.

## Comparaison
Ouverts : absence et retour (AVANT et APRÈS), aucune et fin (APRÈS seulement, appels d'outils épuisés). Bannière identique : même place (centrée sous la barre du haut), même taille, même fond sombre, même police, textes « Bob : connexion perdue — 1:30 » / « Bob : de retour ». Journaux mot pour mot identiques (3 lignes, tiret cadratin intact). Hachages PNG différents (scène 3D, nuages, panneau de triches), attendu.
Écart hors bannière : panneau des Triches replié (« Triches + ») dans AVANT, déplié dans APRÈS ; le diff galadriel-tab contre maps (7 fichiers, bannières seulement) ne l'explique pas : réglage ou fichier de config propre à chaque worktree, à confirmer.
Nit : la `.look` de `hud_banniere_aucune` dit « aucun bandeau » ; on y voit aussi le petit panneau « Scénario ... en préparation » (pas une bannière).

## Fiche
`Retours\E-hud-bannieres.md` écrite, [À ESSAYER].

## NON VÉRIFIÉ
Bannière d'attente en vraie partie réseau ; salon (lobby_match) ; compte à rebours qui baisse à l'écran ; captures AVANT aucune et fin non ouvertes ; clic à travers la bannière.

## Anomalies
Aucune de blocage. Le 1er pré-vol a montré que le panneau du scénario recouvre la bannière à sa place (corrigé dans le scénario). stray_processes -Stop sur les deux worktrees : aucun processus détaché.
