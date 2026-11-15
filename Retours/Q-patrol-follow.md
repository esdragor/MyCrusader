# [À RÉPONDRE] Ordres Patrouille, Suivre et focus : case de la grille et comportement

Contexte : le GDD (§ 7.3) liste les ordres « patrouille, suivre, focus » mais la grille D340 ne donne que **Q** attaque-déplacement,
**S** arrêt, **V** maintien (D = Monter/Démonter proposé). Cases libres du panneau des ordres : A Z E R, F, W, X, C.
La règle pure de patrouille est codée (aller-retour A/B, attaque-déplacement à chaque trajet, fin sur ordre manuel), sans case ni touche.

## 1. Case de la grille
- **A** : Patrouille **F**, Suivre **C** ; focus = clic droit sur une cible avec la touche MAJ (pas de case).
  Visible : les ordres tiennent sur les lignes du milieu et du bas, A Z E R restent libres pour des capacités.
- **B** : Patrouille **W**, Suivre **X**, focus **C** (trois cases, ligne du bas).
  Visible : ligne du bas = ordres de déplacement, à côté de V maintien ; tout se fait au clavier.
- **C** : Patrouille **F** seule ; Suivre et focus sans case (clic droit sur un allié = suivre, sur un ennemi = attaque = focus).
  Visible : une seule case nouvelle, le reste passe par la souris.
- **Autre** : à préciser (place AoE4 exacte à relever si voulu).

## 2. Ce que fait « Suivre » et « focus »
- **A** : Suivre un **allié** = rester à ~3 m de lui en marchant, combattre seulement si attaqué ; Suivre un **ennemi** = le pister
  sans l'attaquer (escorte / filature) ; focus = toute la sélection attaque UNE cible jusqu'à sa mort, sans changer de cible.
- **B** : Suivre un allié = comme A, mais l'unité attaque les ennemis à portée (garde du corps) ; Suivre un ennemi = l'attaquer
  (identique à Attaque) ; focus = comme A.
- **C** : Suivre seulement un allié (sur un ennemi l'ordre est refusé) ; pas d'ordre focus distinct (l'Attaque ciblée en tient lieu).
- **Autre** : à préciser.

## Recommandation
1 = **C** (une seule case nouvelle, F ; le clic droit couvre déjà Suivre et focus, comme les RTS connus) ;
2 = **B** (Suivre un allié en garde du corps est le cas utile : escorte du héros ou d'un convoi).

Valeur par défaut codée : aucune case (la règle pure `Script\Orders\HOLPatrolRules.as` ne choisit ni case ni touche ;
le branchement C++ attend la réponse). Distance d'arrivée d'une jambe : `HOLPatrolRules::ArriveRadius = 150` (une ligne à changer).
