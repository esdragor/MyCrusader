# Étapes définies

Les titres seulement : ce qui a déjà été abordé et peut être fait, et les bugs remontés.
Une ligne disparaît dès que c'est fait ou corrigé. Le détail est dans les `*InProgress.md`.

## À faire

### Parcours J7c — le tir
- J7c.4 S3 et S4 au-dessus du seuil des batailles (6,18 et 6,02 ms pour 6) *(mesures au repos faites ; cause : valeurs des unités, pas le code ; à trancher : optimiser le combat ou revoir D342)*
- J7c.5 Clôture de J7 au GDD (D327, D328, J7-Q5 révisée, J7-Q1 à Q6)

### Animations
- Variantes de chutes par type (2 ou 3, choisies par unité) *(après le prototype)*

### Unités
- Deuxième attaque à déclencheur (charge, D345) *(avec la cavalerie, pas avant)*

### Fin de partie
- Conditions de victoire à coder : forum à vies, ruine à tenir, vies par palier, Nomade, Régicide, abandon, vaincu effondré *(D347 à D360, GDD § 14.1)*

### Interface
- Barre du bas trop large sous 1500 px (touche la place de la mini-carte) *(à corriger avec la mini-carte)*
- Sélection d'un groupe d'ennemis : ouvre le panneau d'info (pastille avec leur nombre) et montre leurs barres de vie, même pleines ; sélection mêlée : on ne garde que le camp prioritaire, Moi > Allié > Ennemi > Neutre

### GDD
- Review du GDD : prochain numéro D361 (D347 à D360 conditions de victoire ; reprise au § 22)

### Questions techniques ouvertes
*(les points de déplacement, ici et dans « Retours d'essai » : chantier `MovementInProgress.md`, après la clôture de J7)*
- Écart inexpliqué des mesures de rendu

## Bugs remontés

### Retours d'essai
- Marcheurs bloqués par des corps (renforts qui butent contre leur propre ligne en mêlée ; deux foules ennemies qui se croisent à grand-peine)
- Ligne qui perd sa forme dès le premier combat (tenir le rang, R1 à R11 validées)
- Unités d'un groupe remises en formation sur place (ordre de bouger d'un rien) : elles peinent, elles ne poussent presque pas les autres

### Revue de code
- `LastAttacker` n'expire jamais (riposte à travers la carte) *(avec le brouillard de guerre, M7 : l'oublier quand il sort de la vue)*
### Aléas et petits restes
- `quick_check -Parts build` rouge sur main : « build cut » et « build gap » (marcheurs remis dehors le long d'un mur)
- `net 2 clients` : barre de vie sur une unité intacte non sélectionnée (vu une fois)
- `net 2 clients` : maison de l’autre arrivée avant le `waitlog` du client 1 (séquence arrêtée, vu une fois)
