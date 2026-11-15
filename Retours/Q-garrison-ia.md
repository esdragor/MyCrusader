# [À RÉPONDRE] Q-garrison-ia : quelles unités l'IA met dans une tour attaquée, et quand elle les fait sortir

**Contexte.** Le GDD dit que l'IA Normal et Difficile garnit ses tours (D696 ; Facile jamais) et qui peut entrer (D793 : tout sauf le siège), mais pas QUI l'IA choisit ni QUAND elle fait sortir. Règle pure écrite (`Script\AI\HOLAIGarrisonRules.as`, branche `tiers`), pas encore branchée : aucune tour n'existe en jeu (F8).

Ce qui est déjà codé (valeurs proposées, une ligne chacune) : une tour finie de l'IA, touchée dans les 5 dernières secondes (`hol.AI.GarrisonAttackedSeconds`), appelle les unités à moins de 15 m (`hol.AI.GarrisonRadius 1500`), les plus proches d'abord, jusqu'à remplir ses places ; l'IA Normal réagit 3 s après le premier coup, Difficile 1 s (`hol.AI.GarrisonReactNormal` / `ReactHard`).

## Q1 : quelles unités l'IA envoie dans la tour (`hol.AI.GarrisonRule`)
- **A : les tireurs à pied seulement (Archers, Arquebusiers…)** (= 1, défaut codé). À l'écran : sous l'attaque, les Archers proches disparaissent dans la tour et tirent de là ; Lanciers et cavaliers restent dehors et se battent au pied de la tour.
- **B : les tireurs d'abord, puis n'importe quelle unité admise pour remplir** (= 2). À l'écran : la tour se remplit toujours (cavaliers, Hommes d'armes compris) ; moins de défenseurs dehors, plus d'unités à l'abri.
- **C : n'importe quelle unité admise, la plus proche d'abord** (= 3). À l'écran : ce sont souvent les cavaliers ou les fantassins (au contact) qui entrent, les Archers restent dehors.
- **Autre** : à préciser.

## Q2 : quand les occupants ressortent (`hol.AI.GarrisonExitCalmSeconds`)
- **A : après un calme de 15 s sans coup sur la tour** (= 15, défaut codé). À l'écran : l'assaut repoussé, les unités ressortent et reprennent leur travail.
- **B : jamais d'elles-mêmes, seulement à la chute de la tour (éjection)** (= 0). À l'écran : la tour reste pleine toute la partie ; ces unités ne servent plus ailleurs.
- **C : dès qu'il n'y a plus d'ennemi à portée de la tour** (pas codé : demande la vue des ennemis proches). À l'écran : sortie plus rapide qu'en A, même pendant un siège à distance (Mangonneau hors de portée).
- **Autre** : à préciser.

**Recommandation : Q1 A + Q2 A.** Les tireurs sont ceux qui gagnent à être en hauteur (D693 bonus de hauteur) et une IA qui ressort après le calme ne gèle pas son armée ; c'est aussi le plus lisible pour le joueur (« les Archers de l'IA montent dans la tour quand je l'attaque »).

Réponse : une ligne à changer par question (réglages ci-dessus), test `Script\AI\HOLAIGarrisonRules_Test.as` à mettre à jour.
