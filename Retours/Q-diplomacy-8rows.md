# [À RÉPONDRE] Q-diplomacy-8rows : jusqu'où allonger le cadre du panneau Diplomatie pour les 8 joueurs

Contexte : un joueur tient maintenant sur 2 lignes (nom, faction, vies, posture, vision, message ; puis les 4 boutons de tribut), et le cadre s'allonge pour que les 8 joueurs tiennent sans défilement à 1920x1080 (présentations 0 et 1, test `diplomacy_eight`). En présentation 1 (à droite) il faut pour cela rentrer dans la place gardée pour les 5 lignes du fil de messages (elles s'effacent) ; en présentation 2 (compact, en bas à gauche) le bloc du héros limite le cadre : le 8e joueur reste à faire défiler.

- A (valeur codée) : le cadre ne mord sur la place du fil que si c'est nécessaire pour faire tenir les 8 joueurs (présentation 1, jamais sur le tableau, la minicarte ni le bloc du héros) ; la présentation 2 défile quand il manque de la place.
- B : comme A, et en présentation 2 retirer le nom de la faction (une colonne de moins, mais pas de ligne en moins : ne gagne pas de hauteur) ou réduire encore l'espacement : à essayer, gain faible.
- C : ne jamais mordre sur le fil de messages : la présentation 1 défile à partir de 6 joueurs environ.
- Autre : ...

Conséquence visible : A = en présentation 1 les 8 joueurs sont là d'un coup, un message récent du fil peut passer sous le cadre ouvert ; C = le fil est toujours lisible, la molette sert dès 6 joueurs.
Recommandation : A (le cadre n'est ouvert que le temps de la diplomatie).
Valeur par défaut codée : A, dans `PlaceDipPanel` (HOLGameScreenScript.as, les deux blocs `if (Needed > MaxHeight)` ; les supprimer redonne C).
Reponse: A (defaut code). 2026-11-05 20:38, utilisateur.
