# [À RÉPONDRE] Q-diplomacy-panel-room : combien de joueurs le panneau Diplomatie montre-t-il d'un coup ?

Contexte : le panneau ne déborde plus (les lignes défilent dans un cadre borné, corrigé le 2026-11-05), mais le cadre est bas : en présentation 1 (défaut) à 1920x1080 il montre 2 joueurs sur 8 (il s'arrête au-dessus de la mini-carte), en présentation 2 avec Triches déplié 2 joueurs aussi ; chaque joueur occupe 4 lignes (nom, posture + Vision + Message, quatre boutons de tribut).

Options :
- **A (valeur codée, rien ne change)** : on garde le cadre et le défilement à la molette. Coût : il faut défiler pour voir le 3e joueur.
- **B** : une ligne de moins par joueur : les quatre boutons de tribut passent derrière un seul bouton « Tribut » (menu à quatre choix, comme la posture en présentation 2). Le panneau montre ~3 joueurs d'un coup à 1920x1080 ; mais le tribut redevient deux clics (l'utilisateur a demandé quatre boutons directs le 2026-11-05, B les remet en question).
- **C** : en présentation 1, laisser le panneau monter plus haut en le plaçant à gauche de la mini-carte au lieu de dessus (il ne dépend plus de la place sous le tableau des joueurs) : ~5 joueurs d'un coup, mais le panneau est plus large et couvre une partie de la carte.
- Autre : à préciser.

Recommandation : A (aucun défaut visible, le défilement marche ; B contredit une demande explicite). À reprendre seulement si l'essai à 6-8 joueurs gêne.

Valeur par défaut codée : A (rien à changer ; `PlaceDipPanel` dans `Script/UI/HOLGameScreenScript.as`, `MaxHeight` par présentation).

Reponse: Autre (2026-11-05 14:3x) : allonger un peu le cadre ET reduire la taille des elements ; un joueur = 2 lignes MAX ; allonger en hauteur pour faire tenir les 8 joueurs, mais seulement quand les 2 lignes par joueur sont effectives (donc d'abord le compactage a 2 lignes, ensuite la hauteur). Les 4 boutons de tribut restent directs (demande du 2026-11-05) : ils doivent tenir dans les 2 lignes. Pas encore applique : ligne de la file.
