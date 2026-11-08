# [À RÉPONDRE] Q-unit-stats-building : le coup contre les bâtiments dans le bloc de stats d'une unité

Contexte : le Bélier (Attaque 0, contre les bâtiments 60) n'avait AUCUNE ligne d'attaque dans son bloc de stats (à moi comme ennemi, même cause : la ligne « Attaque » n'était ajoutée que pour une attaque ordinaire > 0) ; le mot « Attaque » vu seul sous un Bélier ennemi est la ligne d'ACTIVITÉ (« Attaque » = il frappe en ce moment, sans chiffre, voir le Lancier au combat).

Options :
- 0 : rien (ancien comportement : le Bélier n'affiche aucun chiffre d'attaque).
- 1 (CODÉE, défaut) : « Attaque bâtiments 60 » seulement pour une unité SANS attaque ordinaire (le Bélier). Les autres blocs ne changent pas.
- 2 : en plus, « Attaque bâtiments N » à côté de l'attaque ordinaire quand les deux diffèrent (ex. Lancier 8 / bâtiments 20) ; bloc plus long.
- Autre : (par ex. renommer la ligne d'activité « Attaque » en « Combat » pour qu'elle ne ressemble plus à une stat sans valeur).

Conséquences visibles : 0 = Bélier sans chiffre ; 1 = une ligne de plus sous le Bélier seul ; 2 = une ligne de plus sous toute unité dont le coup contre les bâtiments diffère.

Recommandation : 1.

Réglage : `hol.HUD.UnitStatsBuilding` (0, 1, 2), défaut 1, dans `Source\HeroesOfLibertyUI\HUD\HOLGameScreen.cpp` (CVarUnitStatsBuilding). Le libellé de l'activité reste « Attaque » (LOCTEXT ActivityAttacking), non changé.
