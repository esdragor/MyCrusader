# [À RÉPONDRE] Signaux SG3 : le geste de la roue et l'icône (HALDIR-1115d, 2026-11-15)

Contexte : le GDD § 15 (D386) dit « roue de signaux (Alt maintenu + clic) » et « un simple Alt + clic pose un signal neutre », sans dire
comment on choisit l'un des 5 types ni à quoi ressemble l'icône (la durée est tranchée : 5 s, D784). Défauts codés derrière des réglages.

## Q1 : comment choisir le type dans la roue ?

- **A (codé par défaut)** : Alt + clic bref = neutre ; Alt + clic **glissé** (plus de 16 px) dans une direction = le type de ce côté,
  dans le sens des aiguilles depuis le haut : Attaque ici (haut), Défends ici, Besoin d'aide, Regroupement, Éclaire ici (haut gauche).
  Conséquence visible : un seul geste, rapide (manière LoL) ; aucune roue affichée pendant le glisser tant qu'on ne la dessine pas.
  Réglage : `hol.Signal.WheelPixels` (16).
- **B** : Alt + clic **maintenu** plus de 0,3 s ouvre une roue à l'écran, on relâche sur le type ; bref = neutre. Plus lisible pour
  un débutant, plus lent ; demande le dessin de la roue.
- **C** : Alt + clic = neutre seulement ; le type se choisit par Alt + 1 à 5 avant le clic. Aucun dessin, mais 5 touches à retenir.
- **Autre** : à préciser.

Recommandation : **A** (le plus rapide en combat, et B peut s'y ajouter plus tard en dessinant la roue pendant le glisser).

## Q2 : l'icône du signal sur la mini-carte (et plus tard au sol)

- **A (codé par défaut)** : un carré arrondi de 10 px d'une **couleur par type** : neutre blanc, Attaque rouge, Défends bleu,
  Besoin d'aide orange, Regroupement vert, Éclaire ici jaune. Lisible d'un coup d'œil, mais le rouge se confond avec l'alerte d'attaque.
- **B** : un **pictogramme** par type (épée, bouclier, « ! », drapeau, œil), même couleur pour tous ; plus clair, demande 5 icônes
  (provisoires d'abord).
- **C** : tout en **blanc** (`hol.Signal.IconStyle 1`), le type n'est donné que par la ligne du fil de messages (D445).
- **Autre** : à préciser.

Recommandation : **B** à terme ; **A** en attendant (déjà codé, `hol.Signal.IconStyle 0`).

Réponse :
