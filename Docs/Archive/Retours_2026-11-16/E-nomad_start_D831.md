# [À ESSAYER] E-nomad_start : partie Nomade ; ton premier Forum (Paysans, Z = page Palier II, puis C, clic au sol) ne sort plus fini d'un coup : il se BÂTIT (150 s pour un Paysan seul, plus vite à plusieurs, D816)

STATUT : VU À L'ÉCRAN (D816 côté joueur : chantier ouvert quand le 2e Paysan arrive, vu sur le PNG nomad_start_site)
GESTE: réel      (4677857 : vraie touche Z puis C par `hol.UI.Key` (événement clavier Slate), vrai clic gauche par `hol.Mouse.Click 960 540` au centre de la vue ; seule la sélection des Paysans reste `hol.Select.Type Peasant`)
SON: non entendu      (`-nosound` ; l'essai ne porte pas sur un son)
branches: hero-j8-close

Changement depuis ta réponse OK du 2026-11-16 : D816 (Q-nomad-forum-duree = B) codé (97f757c, dans main 131b77d, `BuildingTypes.csv` MainCentre BuildTime 0 -> 150) ; le scénario a un 4e point (a349e1f) et joue maintenant de vraies touches Z, C et un vrai clic (4677857, L14-5) : il attend que tes Paysans arrivent sur la fondation et photographie le chantier. Les 3 premiers points sont inchangés (rejoués tels quels 15:30 : mêmes captures). Seule autre différence vue au journal : l'IA (Nomade aussi) pose maintenant son propre forum dès le départ (D817).

Pré-vol (2026-11-16 15:32, agent CELEBORN-1116b, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, branche hero-j8-close = main 131b77d + a349e1f + 4677857) : `preflight.ps1 nomad_start -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn -AllowUncommitted` PASS (44 s, 4 PNG ; -AllowUncommitted à cause de 16 fichiers c28_*/c31_* non suivis d'un autre chantier, sans rapport).
Journal exact (Saved_Tests\Logs\scenario_nomad_start.log) : « Construction: slot 0 placed MainCentre foundation at (-14800, -3600), paid F 0 W 150 S 400 G 0 », 3 s plus tard « Construction: slot 0 first work on MainCentre at (-14800, -3600), 1 builders », 14 s après la pose « Construction: slot 0 MainCentre at (-14800, -3600), 2 builders at work », aucune ligne « finished MainCentre » (avant D816 : achevé au premier coup).
PNG OUVERTS et vus :
- nomad_start_wide : aucun bâtiment à toi, un Paysan bleu, une unité rouge de l'IA ; minicarte : petits points éloignés (inchangé).
- nomad_start_grid : « Paysan (5) », page II : C « Centre principal » en clair, Marché / Atelier de siège / Chapelle / Écurie / Forge gris (inchangé).
- nomad_start_forum : fondation en fil de fer bleu clair à gauche de la forêt, Bois 200, Pierre 200, « 5 en marche » (inchangé).
- nomad_start_site (NOUVEAU) : même cadre ; la fondation est devenue un chantier bleu (dalle, 4 tours d'angle basses), PAS un bâtiment achevé ; 2 Paysans bleus au travail aux coins ; « 2 en construction · 3 en marche ».
NON VÉRIFIÉ :
- La durée totale (150 s pour un Paysan seul) n'est pas chronométrée par le scénario : il s'arrête au 2e Paysan ; à toi de laisser tourner (le scénario continue chez toi après la note) et de voir le forum s'achever.
- Barre de progression du chantier au-dessus du bâtiment : non lue sur la capture.
- Geste réel rejoué 15:37 (preflight PASS 49 s, 4 PNG rouverts, identiques) : journal « Key pressed: Z » -> « Grid: age 1 chosen » -> « Key pressed: C » -> « Mouse click 960,540 x1 at 960,540: on SViewport » -> fondation posée au même point (-14800, -3600). L'infobulle de la case C : non lue.
- Sur la capture nomad_start_site du 2e passage, le cadre de sélection dit « 1 en construction · 4 en marche » alors que le journal dit 2 bâtisseurs et que 2 Paysans sont visibles aux coins (HUD en retard ? non cherché).
- Capture nomad_start_wide du 2e passage : décor gris (« Shaders en préparation (1) », compilation des matériaux au premier lancement), sans rapport avec l'essai.
- Rejeu depuis le build de main par un autre agent (intégrateur) : non fait. `session_gate.ps1 -Phase Essay -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn -Essay nomad_start` 15:3x : aucun FAIL (D816 97f757c déjà dans main 131b77d ; seul a349e1f, le scénario, est en plus sur hero-j8-close).

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle.
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 nomad_start -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`
Durée : environ 3 min (1 min de scénario, puis laisse le forum s'achever). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Historique : fiche précédente (D797, réponse OK du 2026-11-16) archivée dans `Docs\Archive\Retours_2026-11-16\E-nomad_start.md`.
Rejoué après D831 84f2b30 (CELEBORN-1116d 16:43, worktree HeroesOfLiberty-Galadriel, branche galadriel-tab, fusionnée avec main 5c1fc95 ; hol.Nomad.ForumCell=2 dans Config\DefaultEngine.ini, sans C++, D830) : rouge vu 16:41 (touche E avant le réglage : « timeout at step 43: waitlog Construction: slot 0 placed MainCentre foundation at »), puis preflight nomad_start PASS 50 s, 4 PNG ouverts : grille page II, case E « Centre principal » en clair, case C vide ; journal « Key pressed: E » -> « placed MainCentre foundation at (-15200, -3600) » ; chantier « 2 en construction · 3 en marche ». branches: galadriel-tab. Commande -Root Galadriel : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 nomad_start -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`
Reponse: REGLAGE : Le town center sera sur E, pas C (utilisateur, 2026-11-16 16:3x, dans la conversation ; D831 ; ligne [ ] en tête de la section Celeborn de la file)
Reponse: OK (utilisateur, 2026-11-16 16:5x, après D831 : Forum sur E ; rejoué -Root Galadriel)
