# [À RÉPONDRE] Défi du Chevalier : que fait l'effet ? (GLORFINDEL-1124sh)

Contexte : la ligne `KnightChallenge` (D875 = A) est dans la table (Chevalier vertueux, aura de 8 m, 8 s, recharge 45 s) mais n'a aucun effet en jeu ; la description dit seulement « les ennemis à 8 m ou moins sont défiés (ralentissement, provocation : règles de simulation pas encore branchées) ».
Plan C++ : `AubeInProgress.md`, entrée GLORFINDEL-1124sh.

Que subit un ennemi défié pendant 8 s ?

- A. Provocation seule : tout ennemi à 8 m ou moins du Chevalier le prend pour cible (même si un autre est plus proche). Joueur : le Chevalier « tient » la mêlée, les archers et soigneurs voisins sont protégés. Le plus simple (un verrou de cible).
- B. Provocation et ralentissement (comme le Sceau, 0,66) : le Chevalier tient la mêlée ET les ennemis défiés avancent moins vite. Plus fort, double effet à équilibrer, recoupe le Sceau du Moine.
- C. Ralentissement seul : pas de verrou de cible. Le nom « défi » ne se lirait plus à l'écran.
- Autre : à préciser.

Recommandation : A (un effet lisible, cohérent avec le nom « Défi », le ralentissement est déjà le rôle du Sceau).
Valeur par défaut codée : A (provocation seule), un réglage d'une ligne (`UnitAbilities.csv`, colonne `Effect` quand elle existera pour cette ligne ; sinon une constante dans `CastKnightChallenge`).
