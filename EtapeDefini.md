# Étapes définies

Les titres seulement : ce qui a déjà été abordé et peut être fait, et les bugs remontés.
Une ligne disparaît dès que c'est fait ou corrigé. Le détail est dans les `*InProgress.md`.

## À faire

### Animations
- Variantes de chutes par type (2 ou 3, choisies par unité) *(après le prototype)*

### Unités
- Deuxième attaque à déclencheur (charge, D345) *(avec la cavalerie, pas avant)*
- Héros Paladin-Commandant (J8a) : apparition, contrôle, mort et retour *(J8a.0 pièces et J8a.1 type `Hero`, ×0,3 exact, PV en float, dans `main` ; J8a.2 (naissance, F4, `SKM_Hero`) suivante ; `JourneyInProgress.md`)*
- Bonus de l'Arbalétrier contre Lourd *(D428 ; la catégorie existe et l'Homme d'armes la porte, reste l'unité ; questions tranchées (Champ de tir créé, l'Archer y passe) ; pièces et gestes (AR1 à AR4) validés en aperçu ; AR6 et la suite attendent le moteur recompilé ; chantier `CrossbowInProgress.md`)*

### Fin de partie
- Conditions de victoire à coder : vies par palier (avec les paliers), Nomade (D354, D207), garnison du Roi (D365, avec la garnison) *(D347 à D360, GDD § 14.1 ; V1 à V5 dans `main` ; chantier `VictoryInProgress.md`)*

### Modes de jeu
- Mode *Battlefield* : salon où l'on choisit ses armées et où l'on apparaît avec *(après le prototype, D416)*

### Réseau
- M6 en parallèle : déconnexion (R7), mise en place chez les amis et essai à deux PC (R8), partie à X et clôture (R9) *(R1 à R6 dans `main` : salons, spectateurs, statistiques de partie, fin de partie sans retour d'office, règle du chef (M6-Q10 à M6-Q14) ; reste les essais E2 et E3 de l'utilisateur ; `MaxServers` 2 recommandé (R5.5) ; chantier `NetPlaytestInProgress.md`)*
- Lanceur : essai avec un ami *(L0 à L13 dans `main`, décisions D475 à D482 au GDD ; kit et DuckDNS prêts ; serveur par version confié au chantier réseau (D481) ; `LauncherInProgress.md`)*
- Un client peut mettre le serveur en pause (`ServerPause` du moteur, `bPauseable`) *(vu en R3.4 ; `NetPlaytestInProgress.md`)*

### IA
- IA du prototype : simple mais joue chaque système ; trois niveaux et l'évolutive ; duels, build, héros, forum à vies, volcan *(D421 à D427, GDD § 14.4 ; découpage tranché (IA-Q1 à IA-Q4), chaque nouveau système livre sa part IA ; IA0 (socle) en cours : IA0.2-IA0.3 dans `main`, IA0.4 (cerveau en script) après le premier fichier Angelscript (AS1), IA0.1 attend R7 ; IA1 (économie) : IA1.1 (5 Paysans de départ, 0 dans les scénarios) dans `main`, IA1.2 (ferme) suivante ; chantier `AIInProgress.md`)*

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

### Revue de code
- `LastAttacker` n'expire jamais (riposte à travers la carte) *(avec le brouillard de guerre, M7 : l'oublier quand il sort de la vue)*

### Aléas et petits restes
- `net 2 clients` : barre de vie sur une unité intacte non sélectionnée (vu une fois)
- `net 2 clients` : maison de l’autre arrivée avant le `waitlog` du client 1 (séquence arrêtée, vu une fois)
- `net 2 clients` : client 2 lent au démarrage, arrivé après le client 1, n'a jamais vu le bandeau d'attente (séquence arrêtée, vu une fois)
- Attaque d'un bâtiment : portée de 400 cm, une unité gênée frappe encore du 2e rang *(équilibrage, à trancher ; `EtapeDefiniInProgress.md` J)*
- Attaque d'un bâtiment sans ordre direct (attaque-déplacement, recherche automatique, file Maj) : pas de places autour du mur, ancienne règle du point le plus proche *(J)*
- Unité arrêtée près d'un bâtiment : la garde du mur visible a des coins arrondis, jusqu'à ~35 cm dans le coin du socle *(C.1 ; J l'a corrigé pour les seules unités qui frappent)*

## Fichiers de développement

### Références
- `GDD.md` : conception du jeu, journal des décisions (§ 21) et point de reprise de la review (§ 22)
- `UnitStats.md` : tableau des stats des unités, produit par `Tools/Data/make_unit_stats.py` (ne pas modifier à la main)
- `Manager/PROTOCOL.md` : protocole du gestionnaire de salons (M6)
- `Tools/Scenarios/README.md` : scénarios de test, lancés par `play.ps1`

### Chantiers en cours
- `JourneyInProgress.md` : parcours du joueur (D252), J7 close (D451 à D469), plan de J8 (le héros) en cours, Q1 à Q6 tranchées, questions de J8a tranchées, J8a.0 (pièces du paladin) et J8a.1 (type `Hero`, PV en float) faites, J8a.2 suivante
- `AIInProgress.md` : IA du prototype (D421 à D427), découpage tranché, IA0.2-IA0.3 et IA1.1 dans `main`, plan d'IA1 écrit
- `AngelscriptInProgress.md` : Angelscript (D483, D484) ; AS1 (tests en script dans quick_check) et AS2 (cache précompilé au paquet) faits ; en veille (AS-Q3 = A) : le code en script vient d'IA0.4 et de J8a.4
- `VictoryInProgress.md` : conditions de victoire, V1 à V4 dans `main`
- `NetPlaytestInProgress.md` : réseau M6 (salons, serveurs dédiés)
- `CrossbowInProgress.md` : l'Arbalétrier et le Champ de tir (D428), AR1 à AR4 faites (pièces, gestes), reste la fabrication et le C++ après la recompilation
- `EtapeDefiniInProgress.md` : lignes d'EtapeDefini prises hors des grands chantiers

### En pause
- `UnitStatsInProgress.md` : stats des unités, reste la charge (D345.3) avec la cavalerie
- `AnimationsInProgress.md` : chaîne de fabrication et catalogue des animations, resservira à chaque nouvelle unité

### Presque clos (un reste)
- `ImplementationInProgress.md` : M0 à M2, arrêté le 2026-10-14 (suite dans `JourneyInProgress.md`) ; garde l'écart des mesures de rendu
- `InterfaceInProgress.md` : grille du Paysan (D340) ; reste la barre trop large
- `LauncherInProgress.md` : lanceur (D475 à D482), clos ; reste l'essai avec un ami
- `MovementInProgress.md` : déplacement (D405 à D415, mesures faites) ; reste l'aléa « build site » après `move,bodies`
- `ScenariosInProgress.md` : mécanisme des scénarios, encore cité par les autres journaux (mode d'emploi : `Tools/Scenarios/README.md`)
- `TileSizeInProgress.md` : case de 4 m et cartes de 220 × 220 (D344) ; reste peut-être une ligne marquée tous les 5 cases

### Archivés (`Docs/Archive/`)
- `FrontEndInProgress.md` : menu (D240 à D251)
- `MillInProgress.md` : moulin (D346)
- `RidgeRockInProgress.md` : rochers (D439 à D444)
- `TestSpeedInProgress.md` : quick_check en pas fixe et sans dessin pendant les attentes (`-Full` ~10 min -> ~4 min 20)
