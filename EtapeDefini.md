# Étapes définies

Les titres seulement : ce qui a déjà été abordé et peut être fait, et les bugs remontés.
Une ligne disparaît dès que c'est fait ou corrigé. Le détail est dans les `*InProgress.md`.

## À faire

### Parcours J7c — le tir
- J7c.4 Mesures de fin de J7 (S1, S2 au seuil, S3, S4, S5 en combat, S7) *(en cours dans HeroesOfLiberty-Elrond : boucle 16, les trois règles de mêlée, puis essai `melee_formed`)*
  - Mêlée : les deux lignes se traversent au lieu de se heurter (portée d'arme, espacement entre amis, attente derrière un ami qui combat)
  - S5 en combat au-dessus du seuil (70 Ko/s pour 32)
  - S2 au-dessus du seuil (3,3 ms) : coût fixe du combat sans bataille (0,29 ms par pas) ; S5 hors combat juste au-dessus (32,2 Ko/s)
  - S3 et S4 à remesurer sans la sonde « Separation source » (18 ms faussés)
  - S1 à remesurer au repos (plantage corrigé, combat coupé pendant S1)
- J7c.5 Clôture de J7 au GDD (D327, D328, J7-Q5 révisée, J7-Q1 à Q6)

### Animations
- Variantes de chutes par type (2 ou 3, choisies par unité) *(après le prototype)*

### Interface
- Barre du bas trop large sous 1500 px (touche la place de la mini-carte) *(à corriger avec la mini-carte)*
- Q (attaque) : curseur qui change d'icône et passe au rouge pendant le choix du point *(pas dans l'immédiat)*

### GDD
- Review du GDD : reprendre à D342 (D340 prise par la grille du Paysan, D341 par les scénarios d'essai)
- Préciser D327 au GDD : au commit, seulement les parties touchées (`-Parts`) ; `-Full` aux clôtures de chapitre (choix du 2026-10-21)

### Questions techniques ouvertes
- Croisement de deux unités de face (avant M5)
- Unités immobiles qui restent superposées (vitesse d'arrêt nulle)
- Écart inexpliqué des mesures de rendu

## Bugs remontés

### Retours d'essai
- Marcheurs bloqués par des corps (passage étroit, mur d'unités)
- Formation gênée entre les bâtiments et contre les ressources

### Revue de code
- `LastAttacker` n'expire jamais (riposte à travers la carte) *(avec le brouillard de guerre, M7 : l'oublier quand il sort de la vue)*
### Aléas et petits restes
- `net 2 clients` : barre de vie sur une unité intacte non sélectionnée (vu une fois)
- `net 2 clients` : maison de l’autre arrivée avant le `waitlog` du client 1 (séquence arrêtée, vu une fois)
