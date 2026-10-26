# Étapes définies

Les titres seulement : ce qui a déjà été abordé et peut être fait, et les bugs remontés.
Une ligne disparaît dès que c'est fait ou corrigé. Le détail est dans les `*InProgress.md`.

## À faire

### Animations
- Variantes de chutes par type (2 ou 3, choisies par unité) *(après le prototype)*

### Unités
- Deuxième attaque à déclencheur (charge, D345) *(avec la cavalerie, pas avant)*
- Bonus de l'Arbalétrier contre Lourd *(D428 ; la catégorie existe et l'Homme d'armes la porte, reste l'unité)*

### Fin de partie
- Conditions de victoire à coder : vies par palier (avec les paliers), Nomade (D354, D207), garnison du Roi (D365, avec la garnison) *(D347 à D360, GDD § 14.1 ; V1 à V5 dans `main` ; chantier `VictoryInProgress.md`)*

### Modes de jeu
- Mode *Battlefield* : salon où l'on choisit ses armées et où l'on apparaît avec *(après le prototype, D416)*

### Réseau
- M6 en parallèle : déconnexion (R7), mise en place chez les amis et essai à deux PC (R8), partie à X et clôture (R9) *(R1 à R6 dans `main` : salons, spectateurs, statistiques de partie, fin de partie sans retour d'office, règle du chef (M6-Q10 à M6-Q14) ; reste les essais E2 et E3 de l'utilisateur ; `MaxServers` 2 recommandé (R5.5) ; chantier `NetPlaytestInProgress.md`)*
- Lanceur : télécharge, met à jour et lance le jeu *(questions tranchées, plan L0 à L10, L0 à L4 faites (publication), serveur par version confié au chantier réseau (L-Q7) ; chantier `LauncherInProgress.md`)*
- Un client peut mettre le serveur en pause (`ServerPause` du moteur, `bPauseable`) *(vu en R3.4 ; `NetPlaytestInProgress.md`)*

### IA
- IA du prototype : simple mais joue chaque système ; trois niveaux et l'évolutive ; duels, build, héros, forum à vies, volcan *(D421 à D427, GDD § 14.4 ; chantier à découper par système)*

### Interface
- Barre du bas trop large sous 1500 px (touche la place de la mini-carte) *(à corriger avec la mini-carte)*

### GDD
- Review du GDD : séances closes, reste la courbe d'XP (Q04, à régler en test) ; prochain numéro D485 (§ 22)

### Moteur
- Moteur Hazelight 5.8.4 avec cible Server : compilation depuis `C:/UE/Src/HZ`, puis remplacer `C:/UE/UE_5.8_AS_3` en entier et recompiler le projet de zéro (serveur compris) *(D483)*
- Angelscript au fil des chantiers : le neuf en script, un fichier migrable passe en script quand un chantier va beaucoup le toucher *(D483 ; tests en script et cache précompilé dans le paquet : D484 ; `AngelscriptInProgress.md`)*

### Questions techniques ouvertes
*(les points de déplacement, ici et dans « Retours d'essai » : chantier `MovementInProgress.md`, après la clôture de J7)*
- Écart inexpliqué des mesures de rendu *(145-170 i/s le 2026-10-11 contre 247-258 le 12, mêmes conditions ; depuis, 218 à 277 i/s à chaque mesure au repos ; `ImplementationInProgress.md`)*

## Bugs remontés

### Retours d'essai
- Unités qui s'empilent au même point en attaquant un bâtiment (groupe d'Hommes d'armes fondu en une masse sur le bord d'une Maison ennemie, essai du 2026-10-25)

### Revue de code
- `LastAttacker` n'expire jamais (riposte à travers la carte) *(avec le brouillard de guerre, M7 : l'oublier quand il sort de la vue)*

### Aléas et petits restes
- `net 2 clients` : barre de vie sur une unité intacte non sélectionnée (vu une fois)
- `net 2 clients` : maison de l’autre arrivée avant le `waitlog` du client 1 (séquence arrêtée, vu une fois)

## Fichiers de développement

### Références
- `GDD.md` : conception du jeu, journal des décisions (§ 21) et point de reprise de la review (§ 22)
- `UnitStats.md` : tableau des stats des unités, produit par `Tools/Data/make_unit_stats.py` (ne pas modifier à la main)
- `Manager/PROTOCOL.md` : protocole du gestionnaire de salons (M6)
- `Tools/Scenarios/README.md` : scénarios de test, lancés par `play.ps1`

### Chantiers en cours
- `JourneyInProgress.md` : parcours du joueur (D252), J7 close (D451 à D469), plan de J8 (le héros) en cours, Q4 (modèle technique) à trancher
- `AngelscriptInProgress.md` : préparation d'Angelscript (D483, D484), rien codé ; servira au premier fichier en script
- `VictoryInProgress.md` : conditions de victoire, V1 à V4 dans `main`
- `NetPlaytestInProgress.md` : réseau M6 (salons, serveurs dédiés)
- `LauncherInProgress.md` : lanceur, plan L0 à L10, rien codé
- `EtapeDefiniInProgress.md` : lignes d'EtapeDefini prises hors des grands chantiers

### En pause
- `UnitStatsInProgress.md` : stats des unités, reste la charge (D345.3) avec la cavalerie
- `AnimationsInProgress.md` : chaîne de fabrication et catalogue des animations, resservira à chaque nouvelle unité

### Presque clos (un reste)
- `ImplementationInProgress.md` : M0 à M2, arrêté le 2026-10-14 (suite dans `JourneyInProgress.md`) ; garde l'écart des mesures de rendu
- `InterfaceInProgress.md` : grille du Paysan (D340) ; reste la barre trop large
- `MovementInProgress.md` : déplacement (D405 à D415, mesures faites) ; reste l'aléa « build site » après `move,bodies`
- `ScenariosInProgress.md` : mécanisme des scénarios, encore cité par les autres journaux (mode d'emploi : `Tools/Scenarios/README.md`)
- `TileSizeInProgress.md` : case de 4 m et cartes de 220 × 220 (D344) ; reste peut-être une ligne marquée tous les 5 cases

### Archivés (`Docs/Archive/`)
- `FrontEndInProgress.md` : menu (D240 à D251)
- `MillInProgress.md` : moulin (D346)
- `RidgeRockInProgress.md` : rochers (D439 à D444)
