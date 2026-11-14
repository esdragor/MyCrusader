# [À RÉPONDRE] Nomade : « le centre doit apparaître dans le palier 2 » (D797), quel sens ?

Contexte : ton réglage sur `nomad_start` (2026-11-14). Le centre secondaire est débloqué au palier « 1 — Essor » du GDD (§ 11.1, D129, D384), page affichée « Palier II — Essor » dans la grille des Paysans (les pages s'affichent I, II, III, IV). Avant ce réglage, le forum du Nomade était page I, case C.

- **A (codé par défaut, recommandé)** : le forum du Nomade est **rangé page « Palier II », case C** (là où le centre secondaire a sa place), mais **toujours constructible** par le Nomade dès le départ (touche Z puis C, ou clic sur la case). Les autres bâtiments de la page II restent grisés tant que le palier n'est pas atteint. Conséquence visible : le Nomade ouvre la page II pour son seul forum ; il peut fonder dès la première minute (150 bois + 400 pierre).
- **B** : même case (page II, C), mais le forum **n'est constructible qu'une fois le palier II atteint** (le Nomade erre sans base jusqu'au niveau de héros du palier 1). Conséquence : début de partie bien plus long sans forum ni population (plafond 0, D794).
- **C** : page « Palier III » (palier 2 au sens du GDD, Puissance), toujours constructible ou seulement au palier atteint.
- **Autre** : à préciser.

Réglages (une ligne à changer) : `hol.Nomad.ForumAge` (page : 0 = I, 1 = II défaut, 2 = III), `hol.Nomad.ForumCell` (case, 10 = C). B demanderait un petit code en plus (retirer l'exception « toujours ouverte »).

Recommandation : **A** (le forum garde sa place de « centre » dans la grille, sans bloquer le départ du Nomade, qui sinon n'a ni population ni production).

Agent : HALDIR-1114e (branche maps).
