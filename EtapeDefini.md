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
- Conditions de victoire à coder : forum à vies, ruine à tenir, vies par palier, Nomade, Régicide *(D347 à D360, GDD § 14.1 ; chantier `VictoryInProgress.md`)*

### Interface
- Barre du bas trop large sous 1500 px (touche la place de la mini-carte) *(à corriger avec la mini-carte)*

### GDD
- Review du GDD : prochain numéro D361 (D347 à D360 conditions de victoire ; reprise au § 22)

### Questions techniques ouvertes
*(les points de déplacement, ici et dans « Retours d'essai » : chantier `MovementInProgress.md`, après la clôture de J7)*
- Écart inexpliqué des mesures de rendu

## Bugs remontés

### Retours d'essai
- Unité apparue ou poussée sur un rocher : y reste coincée (ne suit plus aucun ordre)
- Unités d'un groupe remises en formation sur place (ordre de bouger d'un rien) : elles peinent, elles ne poussent presque pas les autres

### Revue de code
- Le combat ignore les équipes : des alliés se battent entre eux *(`OwnerPlayer` comparé dans le combat et l'évitement ; vu en EtapeDefini D ; le camp Neutre de la sélection se branchera sur les postures de D369)*
- `LastAttacker` n'expire jamais (riposte à travers la carte) *(avec le brouillard de guerre, M7 : l'oublier quand il sort de la vue)*
### Aléas et petits restes
- `quick_check -Parts move,bodies,build` : « build site » rouge (plafond de population +20, laissé par les parties d'avant ; `-Parts build` seul passe)
- `net 2 clients` : barre de vie sur une unité intacte non sélectionnée (vu une fois)
- `net 2 clients` : maison de l’autre arrivée avant le `waitlog` du client 1 (séquence arrêtée, vu une fois)
