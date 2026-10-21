# Étapes définies

Les titres seulement : ce qui a déjà été abordé et peut être fait, et les bugs remontés.
Une ligne disparaît dès que c'est fait ou corrigé. Le détail est dans les `*InProgress.md`.

## À faire

### Parcours J7c — le tir
- J7c.4 Mesures de fin de J7 (S1, S2 au seuil, S3, S4, S5 en combat, S7) *(en cours dans MyCrusader-Elrond : S3 et S4 écrits, combat optimisé à l'identique)*
- J7c.5 Clôture de J7 au GDD (D327, D328, J7-Q5 révisée, J7-Q1 à Q6)

### Animations
- Variantes de chutes par type (2 ou 3, choisies par unité)

### Interface
- Grille du Paysan façon AoE4 : ordres à gauche, construction à droite, onglets d'âge A Z E R

### GDD
- Review du GDD : reprendre à D340
- Consigner au GDD la décision des scénarios de test (`hol.Scenario`, `play.ps1`)

### Questions techniques ouvertes
- Croisement de deux unités de face (avant M5)
- Unités immobiles qui restent superposées (vitesse d'arrêt nulle)
- Écart inexpliqué des mesures de rendu

## Bugs remontés

### Retours d'essai
- Marcheurs bloqués par des corps (passage étroit, mur d'unités)
- Formation gênée entre les bâtiments et contre les ressources
- Hit and run impossible avec l'Archer : le geste de tir dure toute la cadence
- Q (attaque) : curseur de souris et couleur qui changent pendant le choix du point

### Revue de code
- `LastAttacker` n'expire jamais (riposte à travers la carte)
- Mort hors mêlée incomplète (avec J7c.2)
- Place 4095 = « cible invalide », mais vraie place au-delà de 4 096 entrées
- `AttackerCount` trop haut d'un après la mort d'un attaquant
- Scénario fermé en cours de préparation : nettoyage trop tard (plantage possible à la fermeture)

### Aléas et petits restes
- `net 2 clients` : barre de vie sur une unité intacte non sélectionnée (vu une fois)
- `net 2 clients` : maison de l’autre arrivée avant le `waitlog` du client 1 (séquence arrêtée, vu une fois)
- Commentaire du test `Gather.Spots` qui parle encore de 70 cm
