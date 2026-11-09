# [À RÉPONDRE] Q-neutrals-trees-decor : aucune commande de test ne coupe les arbres du bosquet central (réponse C de Q-neutrals-camp-cache impossible sans C++)

Contexte : tu as répondu C (retirer les arbres autour du camp). Les arbres du bosquet de `Plain` au centre ne sont PAS des sources de ressources : `hol.Sources.Nearest 0 0` renvoie des mines à 120 m, pas d'arbre ; `hol.Sources.TakeNear 0 0` x16 a vidé 8 mines et aucun arbre du bosquet. Ce sont des arbres de décor de la carte. Aucune commande de test (`hol.*`) ne les retire ; `console show InstancedStaticMeshes` cacherait TOUS les meshes instanciés (arbres du reste de la carte compris), ce n'est pas une clairière.

Options :
- A. Une petite commande C++ de test `hol.Decor.ClearNear X Y R` (ou la liste des arbres du bosquet en données) qui retire le décor autour du camp : vraie clairière à l'écran, le jeu ne change pas. Demande un lot C++ (3 agents C++ déjà en file ce soir).
- B. Repli appliqué : plan serré sur le camp (`hol.Camera.Focus -300 0` + `hol.Camera.Zoom 6`, capture `neutrals_camp_close`). Vu en essai de balayage : les 3 Lanciers gris se voient entre les troncs (zoom +3 et +6), mais selon la pose de l'animation 1 à 2 sont à moitié cachés par un tronc ou un houppier. Dans la meilleure prise (sonde sans les Hommes d'armes) les 3 sont nets.
- C. Déplacer le camp du scénario hors du bosquet (ancienne option A de `Q-neutrals-camp-cache`, `hol.Camp.Add 0 -2500 ...`, les `waitlog` du camp 1 suivent).
- Autre : ...
Conséquence visible : A donne la clairière demandée ; B garde le décor et un plan serré ; C montre le camp en terrain dégagé sans C++.
Recommandation : C (aucun C++, camp net dans un terrain libre) ; A seulement si tu tiens à voir le camp DANS le bosquet.
Valeur par défaut codée : aucune modification du scénario `neutrals` (état de la branche `visual` inchangé : camp en (0, 0), plan large). Le repli B est décrit ci-dessus (4 lignes dans `neutrals.txt` après `capture neutrals_camp` + une ligne dans `neutrals.look`) mais n'est PAS commité : deux pré-vols consécutifs avec lui ont échoué à l'audit Sanity (une unité de mes Hommes d'armes « spins » en mêlée, le bug D641 connu, unités #8 puis #9), alors que le pré-vol sans lui passait ; la cause (flou de la mêlée ou effet du zoom) n'est pas isolée.

Réponse (utilisateur) :

Reponse (utilisateur, 2026-11-09 17:07) : A (commande de decor en C++ pour retirer les arbres autour du camp). Hors de cette seance (file de build pleine, fusion a 17:41) : ligne [ ] (C++) en tete de file Sauron pour la prochaine seance ; E-neutrals reste LOGIQUE SEULEMENT d'ici la.
