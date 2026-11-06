# [À RÉPONDRE] Q-qground-buildings : Q puis clic au sol, les bâtiments d'un joueur Neutre sont-ils engagés aussi ?

Contexte : D733 fait engager les UNITÉS neutres rencontrées par l'attaque-déplacement (Q puis clic au sol) : joueur Neutre et camps de carte. Un bâtiment neutre (Diplomatie, joueur passé Neutre) n'est pas une unité : aujourd'hui l'attaque-déplacement le laisse passer.
Question : une troupe en attaque-déplacement qui croise la base d'un joueur Neutre attaque-t-elle ses bâtiments ?

- A. Non, unités seulement (valeur par défaut codée) : les bâtiments d'un Neutre restent tranquilles sauf clic direct dessus (Q puis clic sur le bâtiment, ou clic droit). Conséquence visible : on traverse une base neutre en ne frappant que ses hommes.
- B. Oui, bâtiments aussi : la troupe s'arrête pour casser les maisons du Neutre rencontrées, comme chez un Ennemi. Conséquence : un Q mal placé près d'une base neutre y fait des dégâts ; la posture reste Neutre.
- C. Bâtiments seulement quand ils sont armés (tours, forts) : ce qui peut nous tirer dessus est engagé, le reste passe.
- Autre : à préciser.

Recommandation : A. Un Neutre n'est pas un ennemi (D369) : rien ne tombe sans geste clair, et un clic direct sur le bâtiment reste possible. Les unités en mouvement sont une menace qui se bat de toute façon (elles ripostent) ; un bâtiment ne gêne personne.
Valeur par défaut codée : A. Le changement est une ligne dans `HOLCombatProcessor.cpp` (la recherche de bâtiment de l'attaque-déplacement, `FiresAtOnce` devient `FiresAtOnce || attaque-déplacement et Neutre`), derrière le même réglage `hol.Combat.AttackMoveEngagesNeutral`.

Reponse: A (unites seulement, bati ments neutres non engages par Q+clic). 2026-11-05 19:50, repondu par l'utilisateur. Defaut code conserve : rien a changer.
