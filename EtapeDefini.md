# Étapes définies

Les titres seulement : ce qui a déjà été abordé et peut être fait, et les bugs remontés.
Une ligne disparaît dès que c'est fait ou corrigé. Le détail est dans les `*InProgress.md`.

## À faire

### Parcours J7c — le tir
- J7c.4 Mesures de fin de J7 (S1, S2 au seuil, S3, S4, S5 en combat, S7) *(en cours dans HeroesOfLiberty-Elrond : lignes de mêlée validées et commitées sur `j7c-shot` ; suite : coût fixe du combat, S5 en combat)*
  - S2 au seuil sur `j7c-shot` (~0,2 ms de plus que `main`) : alléger la mêlée hors combat (raccourcis du combat faits)
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
