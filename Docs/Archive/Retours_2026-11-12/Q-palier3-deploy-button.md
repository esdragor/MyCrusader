# [À RÉPONDRE] Palier 3 G6 d : où est le bouton Monter / Démonter, et que dit-il

**Contexte.** L'ordre du joueur « Monter / Démonter » existe côté simulation (`HOLPlayerOrders::ToggleDeploy`, puis `HOLDeploy::Toggle`). Le GDD (D712) dit qu'« un bouton manuel monte ou démonte » mais ne dit ni la case, ni le libellé, ni l'icône. Le panneau des ordres (à gauche de la grille) porte Q Attaque, S Arrêt, V Tenir.

**Question 1 : quelle case ?**
- **A (codée, défaut)** : D (cellule 6 du panneau des ordres, à côté de S Arrêt), visible seulement si la sélection contient un engin qui se monte. Réglage : `HOLCommandGrid::DeployCell` (une ligne).
- **B** : W (cellule 8), rangée du bas, loin des ordres de combat.
- **C** : la même case que Tenir (V) pour les engins (Tenir n'a pas de sens pour un engin monté).
- **Autre** : à préciser.
**Recommandation : A.**

**Question 2 : quel libellé ?** (propositions, jamais décidées, marquées dans `Tools/Localization/fill_po.py`)
- **A (codé, défaut)** : « Monter » / « Démonter » (FR), « Deploy » / « Pack up » (EN), « 展開 » / « 撤収 » (JA) ; infobulle « Monter l'engin : il tire, il ne marche plus » / « Démonter l'engin : il ne tire plus, il peut marcher ».
- **B** : « Installer » / « Replier ».
- **Autre** : à préciser.
**Recommandation : A** (les mots du journal et du GDD).

**Conséquences visibles.** Sans icône (ligne dans `Placeholders.txt`), la case montre son texte, comme la rage nécrotique. En partie en réseau, la case n'apparaît pas chez un client tant que la réplication (G6 e) ne porte pas la phase : l'ordre existe, pas la case.

**Valeur par défaut codée** : Q1 = A, Q2 = A. Aucun effet tant que `hol.Siege.Deployable` vaut 0 (aucune unité ne se monte).

Reponse: Q1 A (case D), Q2 A (Monter / Demonter) - recommande, utilisateur, 2026-11-11 soir
