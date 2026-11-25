# [À RÉPONDRE] Prix d'un mur sur la grille fine : prix plein ou fraction par sous-case ?

Contexte : D904 (réponse B de Q-murs-grille-fine) fixe « prix actuel par sous-case » ; D910 a ensuite remplacé la sous-case de 2 m par 1 m, donc une case de 4 m = 16 sous-cases (et non 4). Au prix actuel pris à la lettre (palissade 5 Bois, mur de pierre 10 Pierre), une ligne de même longueur coûte 4 fois plus qu'avant D898, et la surface d'une case entière 16 fois.
Branche `walls-g7-1124s`, règle `HOLWallLine::StockAfterLine` (Script/Buildings/HOLWallLineRules.as:79, test rouge/vert CELEBORN-1125ma : N sous-cases posées x prix d'une).

Options :
- A (défaut codé) : prix plein par sous-case (palissade 5 Bois par sous-case de 1 m). En jeu : une ligne de 20 m coûte 100 Bois (aujourd'hui 5 cases de 4 m = 25 Bois) ; tracer un mur long vide vite le stock, les trous (D901) ne sont pas payés.
- B : fraction du prix, le prix d'une case divisé par 4 (le prix reste « par mètre de ligne » : 20 m = 25 Bois comme avant D898). En jeu : même coût qu'aujourd'hui sur une ligne, et la finesse de la grille ne renchérit rien ; contredit la lettre de D904 B (« une ligne de même longueur coûte 4 fois plus »).
- C : fraction du prix, le prix d'une case divisé par 16 (le prix reste « par surface »). En jeu : une ligne de 20 m coûte environ 6 Bois ; murs presque gratuits, la défense perd son coût.
- Autre.

Recommandation : A pour l'instant (c'est la réponse écrite D904 B, ne pas la réinterpréter sans toi) ; B si l'essai `wall_line_long` montre des murs trop chers pour un tracé de 20 à 40 m.

Valeur par défaut codée : A (prix plein). Réglage : le prix par sous-case passé à `StockAfterLine` (CostPerCell) ; à brancher derrière un cvar `hol.Wall.SubCellCostPercent` (100 = prix plein, 25 = B, 6 = C) lors du branchement de la déduction dans `ServerPlaceWallLine` (HOLRpcPlayer.as:1540), une ligne à changer.

Reponse: B (2026-11-25 13:0x, DOCS-1125me, GDD D915)
