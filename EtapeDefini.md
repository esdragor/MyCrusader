# Étapes définies

Les titres seulement : ce qui a déjà été abordé et peut être fait, et les bugs remontés.
Une ligne disparaît dès que c'est fait ou corrigé. Le détail est dans les `*InProgress.md`.

## À faire

### Parcours J7c — le tir
- J7c.4 S5 au-dessus du seuil de bande passante (32 073 o/s pour 32 000, erreur 24,2 cm PASS) *(S3 / S4 repassés sous 6 ms par `perf-combat` et `perf-target` ; à trancher : alléger les bits des marcheurs ou revoir D218)*
- J7c.5 Clôture de J7 au GDD (D327, D328, J7-Q5 révisée, J7-Q1 à Q6, décisions de J7c.4) *(après J7c.4)*

### Animations
- Variantes de chutes par type (2 ou 3, choisies par unité) *(après le prototype)*

### Unités
- Deuxième attaque à déclencheur (charge, D345) *(avec la cavalerie, pas avant)*
- Bonus de l'Arbalétrier contre Lourd *(D428 ; la catégorie existe et l'Homme d'armes la porte, reste l'unité)*

### Fin de partie
- Conditions de victoire à coder : messages (V4, en cours dans Arwen, branche `victory-v4`), vies par palier, Nomade, Régicide *(D347 à D360, GDD § 14.1 ; V1 à V3 dans `main` ; chantier `VictoryInProgress.md`)*

### Modes de jeu
- Mode *Battlefield* : salon où l'on choisit ses armées et où l'on apparaît avec *(après le prototype, D416)*

### Réseau
- M6 en parallèle : déconnexion (R7), mise en place chez les amis et essai à deux PC (R8), partie à X et clôture (R9) *(R1 à R3 et R6 dans `main` ; R4 écrans et R5 bout à bout faits dans Glorfindel, non commités, finitions en cours ; chantier `NetPlaytestInProgress.md`)*
- Lanceur : télécharge, met à jour et lance le jeu *(questions L-Q1 à L-Q6 à trancher ; chantier `LauncherInProgress.md`)*
- Un client peut mettre le serveur en pause (`ServerPause` du moteur, `bPauseable`) *(vu en R3.4 ; `NetPlaytestInProgress.md`)*

### IA
- IA du prototype : simple mais joue chaque système ; trois niveaux et l'évolutive ; duels, build, héros, forum à vies, volcan *(D421 à D427, GDD § 14.4 ; chantier à découper par système)*

### Interface
- Barre du bas trop large sous 1500 px (touche la place de la mini-carte) *(à corriger avec la mini-carte)*

### GDD
- Review du GDD : séances closes, reste la courbe d'XP (Q04, à régler en test) ; prochain numéro D445 (§ 22)

### Questions techniques ouvertes
*(les points de déplacement, ici et dans « Retours d'essai » : chantier `MovementInProgress.md`, après la clôture de J7)*
- Écart inexpliqué des mesures de rendu *(145-170 i/s le 2026-10-11 contre 247-258 le 12, mêmes conditions ; depuis, 218 à 277 i/s à chaque mesure au repos ; `ImplementationInProgress.md`)*

## Bugs remontés

### Retours d'essai

### Revue de code
- `LastAttacker` n'expire jamais (riposte à travers la carte) *(avec le brouillard de guerre, M7 : l'oublier quand il sort de la vue)*

### Aléas et petits restes
- `quick_check -Parts move,bodies,build` : « build site » rouge (plafond de population +20, laissé par les parties d'avant ; `-Parts build` seul passe)
- `net 2 clients` : barre de vie sur une unité intacte non sélectionnée (vu une fois)
- `net 2 clients` : maison de l’autre arrivée avant le `waitlog` du client 1 (séquence arrêtée, vu une fois)
