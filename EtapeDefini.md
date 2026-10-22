# Étapes définies

Les titres seulement : ce qui a déjà été abordé et peut être fait, et les bugs remontés.
Une ligne disparaît dès que c'est fait ou corrigé. Le détail est dans les `*InProgress.md`.

## À faire

### Parcours J7c — le tir
- J7c.4 Mesures de fin de J7 (S1, S2 au seuil, S3, S4, S5 en combat, S7) *(mêlée dans `main` 48ffa00 ; reste : mesures au repos)*
  - S2 avec la mêlée : allégée hors combat (raccourcis du combat, front seulement en combat), à mesurer au repos
- J7c.5 Clôture de J7 au GDD (D327, D328, J7-Q5 révisée, J7-Q1 à Q6)

### Animations
- Variantes de chutes par type (2 ou 3, choisies par unité) *(après le prototype)*

### Unités
- Tableau complet des stats des unités (vitesse, armure à distance et au corps à corps, cadence d'attaque et de tir, etc.)

### Interface
- Barre du bas trop large sous 1500 px (touche la place de la mini-carte) *(à corriger avec la mini-carte)*
- Q (attaque) : curseur qui change d'icône et passe au rouge pendant le choix du point *(pas dans l'immédiat)*

### GDD
- Review du GDD : reprendre à D344 (D341 scénarios d'essai, D342 seuil de S3 / S4, D343 état d'unité sur 59 bits)

### Questions techniques ouvertes
*(les points de déplacement, ici et dans « Retours d'essai » : chantier `MovementInProgress.md`, après la clôture de J7)*
- Unités qui tremblent (droite-gauche) en s'évitant quand elles se croisent
- Écart inexpliqué des mesures de rendu

## Bugs remontés

### Retours d'essai
- Marcheurs bloqués par des corps (passage étroit : personne ne passe ; mur d'unités à nous : très ralentis ; renforts qui butent contre leur propre ligne en mêlée ; deux foules ennemies qui se croisent à grand-peine)

### Revue de code
- `LastAttacker` n'expire jamais (riposte à travers la carte) *(avec le brouillard de guerre, M7 : l'oublier quand il sort de la vue)*
### Aléas et petits restes
- `net 2 clients` : barre de vie sur une unité intacte non sélectionnée (vu une fois)
- `net 2 clients` : maison de l’autre arrivée avant le `waitlog` du client 1 (séquence arrêtée, vu une fois)
