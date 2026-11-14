# [À RÉPONDRE] Forêt : où est « en forêt », et combien elle protège des tirs

Chantier : forêt et gué (`ForestFordInProgress.md`, worktree Haldir, branche `maps` ; chapitre B4 de `Auto_Journey_Jour.md`, GDD § 8.1, D33).
2 questions, regroupées. Rien n'est bloqué : les deux réponses sont préparées derrière un réglage (`hol.Terrain.*`, une ligne à changer).

## Q1. Quand une unité est-elle « en forêt » (et donc à couvert des tirs) ?

Contexte : D33 donne un couvert en forêt, mais rien ne dit ce qu'est la forêt pour le jeu. Les cartes savent déjà poser des zones
rondes (marais, route) ; les arbres sont aussi posés un par un.

- **A. Une zone de forêt peinte sur la carte** (comme le marais). En jeu : la limite est nette et la même pour tous ; une unité au milieu
  d'une clairière dans la zone reste à couvert ; il faut peindre la zone sur chaque carte.
- **B. La proximité d'arbres : la même forêt que celle qui cache déjà la vue** (D383 : une case avec au moins 2 arbres debout ; règle
  pure écrite ici en « au moins 3 arbres à moins de 6 m », à aligner sur la grille de la vision au branchement). En jeu : couvert là où
  l'on voit des arbres serrés, pas dans un bosquet clairsemé ; une forêt abattue par les bûcherons ne protège plus.
- **C. Les deux** : zone peinte OU arbres proches. En jeu : le plus généreux ; deux règles à lire pour le joueur.
- **Autre.**

**Recommandation : B** (la forêt qu'on voit est celle qui protège, et la coupe a un effet stratégique).
Défaut codé : A (`hol.Terrain.ForestByTrees 0`) ; B = `hol.Terrain.ForestByTrees 1` (règle déjà écrite et testée).

## Q2. De combien la forêt réduit-elle les dégâts des tirs reçus (la mêlée ne change pas) ?

Contexte : D33 dit « couvert en forêt », sans chiffre ; la valeur dit « modérée ». Seule la cible compte (le tireur peut être hors forêt).

- **A. −25 % (75 % des dégâts)** : en jeu, un archer en forêt encaisse une volée sur quatre de moins ; attaquer la forêt à l'arc reste possible.
- **B. −15 % (85 %)** : couvert à peine sensible, à voir sur les chiffres de dégâts.
- **C. −50 % (50 %)** : la forêt devient un refuge contre les tireurs ; il faut y entrer en mêlée.
- **Autre.**

**Recommandation : A.**
Défaut codé : A (`hol.Terrain.ForestRangedPercent 75`).

Reponse: Q1 A trancher (aucun souvenir de la règle, à débattre et trancher plus tard) ; Q2 A trancher (idem). Rien n'est décidé ; défauts codés inchangés. (réponse donnée dans la conversation, 2026-11-13 ~17:10)
