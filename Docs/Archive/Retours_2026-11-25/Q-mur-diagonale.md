# [À RÉPONDRE] Mur en diagonale sur la grille fine : pièce de remplissage ou rotation à 45 degrés ?

Contexte : sur la grille fine de 1 m (D898, D910), une ligne en diagonale pose des pièces carrées de 1 m qui ne se touchent que par un coin : une fente de largeur nulle que le navmesh peut lire comme un passage (escalier franchissable). Branche `walls-g7-1124s` (CELEBORN-1124si), règle `HOLWallLineRules.as`, `FillDiagonalGaps`.

Options :
- A (défaut codé, `hol.Wall.DiagonalFill 1`) : une pièce de remplissage de 1 m à chaque coin de la diagonale. En jeu : le mur diagonal est étanche, en escalier, 2 pièces de 1 m par mètre de diagonale (donc le coût de la ligne est presque doublé au prix actuel par sous-case, D904 B) ; la ligne compte des pièces au-delà du plafond de tracé (jusqu'à 2 fois).
- B : pièces tournées de 45 degrés (rectangles de 1 m x 1,41 m). En jeu : un mur diagonal lisse, même coût qu'une ligne droite de même longueur ; demande une orientation par pièce, une empreinte non alignée sur la grille et un obstacle navmesh tourné (C++, collision, aperçu).
- C : ne rien faire (`hol.Wall.DiagonalFill 0`) ; ligne en pointillé de coins, franchissable si le navmesh lit la fente.
- Autre.

Recommandation : A (étanche tout de suite, sans C++), B plus tard si l'escalier gêne l'œil ou le coût.

Valeur par défaut codée : A, derrière `hol.Wall.DiagonalFill` (1 = remplissage, 0 = diagonale nue).
Reste à mesurer (lot C++, G7 scénario `wall_fine_diagonal`) : la fente de coin est-elle réellement franchie par le Paysan ?

Reponse: B (2026-11-25 13:0x, DOCS-1125me, GDD D917)
