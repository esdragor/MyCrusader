# [RÉPONDU : A + B puis C, GDD D871] Q-deploy-visible : comment montrer le montage et le démontage du Mangonneau (D869) ?

Contexte : les durées sont maintenant 1,5 s (montage) et 1 s (démontage) (`HOLDeployRules.h:76-77`). Aujourd'hui rien ne se voit pendant le changement : la case D ne change de libellé qu'à la fin et le modèle de l'engin n'a aucune animation de montage (mannequin du moteur).

Options :
- A. Case D grisée avec le texte « Montage... » / « Démontage... » pendant le changement (aucun asset, 1 ligne dans `HOLGameScreen.cpp:1254`).
- B. Barre de progression au-dessus de l'engin (ElapsedMs / durée, déjà dans le fragment), visible de tous les joueurs.
- C. Animation du modèle (pliage / dépliage) : demande un asset d'animation, pas disponible aujourd'hui.
- Autre.

Conséquences en jeu : A informe seulement le joueur qui a sélectionné l'engin ; B se lit d'un coup d'oeil, pour l'ennemi aussi ; C est le rendu final mais bloqué par l'asset.

Recommandation : A tout de suite, B ensuite ; C quand l'asset existe.

Valeur par défaut codée : aucune des trois (seules les durées sont changées, réglables d'une ligne : `ProposedDeployMs` / `ProposedPackMs`). L'essai `deploy` capture « pendant » le montage (`deploy_deploying`, `deploy_packing`) pour juger de ce qui se voit réellement.

Reponse: « A et B puis C en plus » (utilisateur, 2026-11-20 ~15:5x, interface de choix) : texte « Montage... » / « Démontage... » dans la case D ET barre de progression maintenant ; animation (C) ensuite, quand l'asset existera. Enregistré : GDD D871 ; ligne `[ ]` en tête de « Elrond » dans `Auto_Journey_File.md`.
