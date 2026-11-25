# [À RÉPONDRE] Q-porte-remplacement-remboursement : la porte rembourse-t-elle les palissades qu'elle remplace ?

Contexte : poser une porte (PalisadeGate) sur une palissade finie du même camp retire maintenant les palissades dessous (`OrderRemoveOwnBuilding`, CELEBORN-1125d) avant la pose. `DestroyBuilding` laisse des gravats 10 s, rembourse la file de production, mais rien pour un bâtiment fini : le joueur perd donc le Bois des palissades remplacées.

- A (défaut codé) : aucun remboursement (le Bois des palissades est perdu, la porte est payée plein tarif). Effet en jeu : remplacer 4 sous-cases coûte 4 x 25 % + le prix de la porte.
- B : remboursement partiel (une part du prix, à fixer) des palissades retirées. Effet : poser une porte sur un mur existant coûte presque le prix de la porte seule.
- C : remboursement total. Effet : la porte ne coûte que son prix, mais on peut poser / reposer pour gagner un peu de Bois si le tarif de la porte est inférieur.
- Autre : à dire.

Recommandation : B (une part, p. ex. la même que la part de sous-case), car A punit le joueur qui améliore son mur et C ouvre un aller-retour.

Valeur par défaut codée : A, aucun remboursement (rien à changer dans le code tant que la réponse n'est pas donnée ; le remboursement s'ajoutera derrière un cvar en une ligne).
