# [À RÉPONDRE] Soldat de mêlée dont la seule place libre est de l'autre côté de la cible : changer de cible, attendre, ou faire le tour ?

Contexte (Celeborn-8, `hero-j8-close`, BUG D641 vu par l'utilisateur sur `diplomacy_qground`, 6 contre 6 : des hommes d'armes tournent sur place) : quand sa cible meurt, un soldat réclame la place libre la plus proche de SON côté ; si tout son côté est pris, la place libre est à l'autre bout de l'anneau (plus de 90 degrés) : il traverse la mêlée en tournant, et la cible suivante meurt avant son arrivée. Le lot C++ de Celeborn-8 refuse une place à plus de 1,5 rad (`hol.Combat.FarPlaceGap`) et prend l'ennemi le plus proche ayant une place de son côté (`hol.Combat.FarPlaceRetarget`).

Question : quand AUCUN ennemi proche n'a de place de son côté, que fait le soldat ?
- **A. Attendre au plus 2 s sans progrès, puis faire le tour comme avant** (`hol.Combat.FarPlaceWaitSeconds 2`). Visible : un soldat qui reste planté derrière ses amis 2 s, puis contourne.
- **B. Attendre sans limite tant que la cible vit** (`FarPlaceWaitSeconds 99`). Visible : un soldat immobile derrière la ligne ; l'audit « stall » peut le signaler.
- **C. Ne jamais attendre : faire le tour tout de suite** (`FarPlaceWaitSeconds 0`). Visible : le comportement d'avant pour ce cas seulement (le tour, donc le demi-tour tant décrié).
- **Autre** : ______

Recommandation : **A** (borne l'attente, aucun soldat ne reste figé).

Valeur par défaut codée : **A** (`hol.Combat.FarPlaceWaitSeconds 2`, une ligne dans `HOLCombatProcessor.cpp` ou dans le scénario).

Reponse: A (utilisateur, 2026-11-06 en conversation) : attendre 2 s max puis contourner = la valeur codee; rien a changer.
