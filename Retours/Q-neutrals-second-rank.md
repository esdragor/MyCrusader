# [À RÉPONDRE] Soldat de 2e rang bloqué derrière ses amis : attendre, faire demi-tour, ou changer de cible ?

Contexte (Celeborn-4, `hero-j8-close`, trace `neutrals_trace` du 2026-11-05) : un soldat dont la cible vient de mourir réclame une place libre de l'autre côté de la mêlée, glisse le long de ses amis, se retrouve emmuré entre deux amis qui frappent (1 s), puis tourne le dos et refait le tour par l'autre bout (800 cm de chemin pour 350 cm nets : l'audit crie « spin »). C'est le « retour sur ses pas au milieu de la mêlée » que Q-sanity-circling = C déclare FAUTE du jeu.

- **A. Garder le demi-tour après attente** (`hol.Combat.RearSlideShutTime` 1 s, comportement actuel) : le soldat fait le tour, l'audit tolère un demi-tour qui suit une attente emmurée. Visible : un soldat qui repart dans l'autre sens après une seconde d'arrêt.
- **B. Ne jamais faire demi-tour tant que la cible vit** (`RearSlideShutTime` 99) : il attend derrière ses amis jusqu'à ce qu'une place s'ouvre. Visible : un soldat immobile derrière la ligne, parfois plusieurs secondes.
- **C. Changer de cible** : un soldat emmuré plus de 1 s prend l'ennemi le plus proche dont une place est atteignable (pas de contournement). Visible : le soldat frappe un voisin au lieu de marcher ; nécessite du code (un lot C++).
- **Autre** : ______

Recommandation : **C** si la mesure `nvs99` (B) n'est pas verte (la mesure est dans `DuelInProgress.md`, dernière entrée) ; sinon **B**, qui n'exige aucun code.

Valeur par défaut codée : **A** (rien ne change) ; B = une ligne dans le scénario ou le CSV, `hol.Combat.RearSlideShutTime 99`.

Reponse: C (changer de cible) - sauf si la mesure nvs99 (B) est verte : alors B. 2026-11-05 19:50, repondu par l'utilisateur. Lot C++ pour Celeborn-4 ou agent neuf.
