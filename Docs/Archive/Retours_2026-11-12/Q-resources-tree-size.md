# [RÉPONDU 2026-11-12] Q-resources-tree-size : l'arbre C (peuplier) garde-t-il la taille vue dans resources_compare ?

Contexte : tu as choisi l'arbre C (peuplier) dans resources_compare ; il y était montré à 7,5 m de haut, la couronne de 1,1 m à 7,5 m et 2,4 m de large. L'ancien arbre du jeu (M-Q10, 2026-10-30 : « un tiers plus bas ») faisait 5 m, couronne à partir de 2,7 m (au-dessus de la tête d'un Paysan) et 1,2 m de large.

- **A (codé par défaut)** : taille vue dans la comparaison (7,5 m, couronne dès 1,1 m, 2,4 m de large). Conséquence : forêts plus hautes et plus pleines ; un Paysan qui coupe au pied est en partie caché par la couronne vue d'en haut.
- **B** : la silhouette C rentrée dans l'ancien gabarit (5 m, couronne dès 2,7 m, 1,2 m de large) : un « crayon » très mince, le Paysan visible sous la couronne.
- **C** : les proportions C gardées, réduites à 5 m de haut (couronne dès 0,7 m, 1,6 m de large).
- **Autre** : dis la hauteur / largeur voulue.

Recommandation : A (c'est ce que tu as vu et choisi) ; B si le Paysan caché sous la couronne gêne en partie.
Réglage : `Source/HeroesOfLiberty/Resources/HOLSourceField.cpp`, constante `TreeSeen` et les deux lignes `AddSeen` de l'arbre (une ligne à changer, C++ : recompiler). Branche visual (SAURON-1112a, 2026-11-12).

Reponse: A (taille vue, 7,5 m ; déjà codé) - 2026-11-12
