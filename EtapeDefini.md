# Étapes définies

Les titres seulement : ce qui a déjà été abordé et peut être fait, et les bugs remontés.
Une ligne disparaît dès que c'est fait ou corrigé. Le détail est dans les `*InProgress.md`.

**Chaque session met ce fichier à jour à chaque avancée** (question tranchée, boucle faite, commit, fusion, bug trouvé
ou corrigé), en même temps que son journal, dans `C:\Users\baudo\Documents\MyCrusader\EtapeDefini.md` (chemin absolu,
même depuis un worktree). Relire juste avant d'éditer et ne toucher que ses lignes.

## À faire

### Animations
- Variantes de chutes par type (2 ou 3, choisies par unité) *(après le prototype)*

### Unités
- Deuxième attaque à déclencheur (charge, D345) *(avec la cavalerie, pas avant)*
- Héros Paladin-Commandant (J8a) : apparition, contrôle, mort et retour *(J8a.0 pièces, J8a.1 type `Hero` (×0,3 exact, PV en float), J8a.2 naissance près du forum, `SKM_Hero` et F4, J8a.3 fiche réseau (vie du héros en part de 255, fiches publique et privée écrites par le serveur, PV exacts chez le client, contrôle `net hero sheets`), J8a.4 mort et retour (règles et décision en Angelscript, achat `hol.Hero.Return`, contrôle `net hero return`, alerte « héros prêt », 7263184), J8a.5 bloc du héros (écran de jeu en Angelscript `UHOLScriptScreen`, module d’interface chargé par le serveur, portrait, PV, bouton « Ramener le héros », textes fr/en/ja, aa0f11f), dans `main` ; J8a.6 vérification (partie `hero` de `quick_check`, scénario `hero`, essai validé, 8ea1b8e), **J8a close** ; défaut noté à l'essai : des Hommes d'armes inactifs autour du héros (mêlée contre une cible unique) ; **J8b (XP et niveaux) : questions ouvertes (Q1 à Q6), Q1 tranchée (combat, commandement, développement), Q2 tranchée (XP = coût pondéré : militaire > économie > exploration), Q3 tranchée (+8 % PV et attaque par niveau), Q4 tranchée (barre d'XP, alerte, éclat doré, paliers annoncés), Q5 tranchée (délai 30 s + 8 s × niveau, prix 100 + 50 × niveau), Q6 tranchées (J8b.1 à J8b.6, un essai à la fin) ; J8b.1 (règles pures de l'XP, tests rouges puis verts, 239d10e sur hero-j8a3) faite, J8b.2 (XP dans la partie) en cours : boucle 1 (niveau sur le corps du héros, 585d49c) faite, boucle 2 (attribution des morts et bâtiments) suivante** ; serveur empaqueté à recuire (il lie maintenant l’interface) ; `JourneyInProgress.md`)*

### Fin de partie
- Conditions de victoire à coder : vies par palier (avec les paliers : P-Q1 = C, chantier des paliers juste après J8b, XP et niveaux), Nomade (D354, D207), garnison du Roi (D365, avec la garnison) *(D347 à D360, GDD § 14.1 ; V1 à V5 dans `main` ; chantier `VictoryInProgress.md`)*

### Cartes
- Nouvelles cartes du prototype : El Dorado, Les Gués, La Forêt profonde, La Couronne, La Steppe, Les Cols, L'Arrière-pays *(D496, D498 : dans le prototype, chantier à ouvrir ; GDD § 8.5 ; chacune sa recette, appelée par taille ; journal `Docs/Archive/MapSizeInProgress.md`)*

### Modes de jeu
- Mode *Battlefield* : salon où l'on choisit ses armées et où l'on apparaît avec *(après le prototype, D416)*

### Réseau
- M6 en parallèle : R8 redécoupé (le lanceur remplace R8.1-R8.5) : R8a serveur par version faite et dans `main` (dd1e708, contrôle `server_versions_check`), R8b publication à jour faite (version 2026-10-27-r8 sur le gestionnaire du raccourci, serveur par version), puis R8c mesure à 4 puis 8 clients (faite et dans `main` 9364046 le 2026-10-28 : 2,5 Mbit/s montants au pire par partie à 8, box en fibre : pas de limite), R8d essai E5 avec un ami (reporté) ; partie à X et clôture (R9) *(R1 à R7.10 et R8a dans `main` (dd1e708) : salons, spectateurs, statistiques de partie, fin de partie sans retour d'office, règle du chef, déconnexion et retour (« Revenir »), icône allié / ennemi et nom rayé ; essais E2, E3 et E4 réussis ; `MaxServers` 2 recommandé (R5.5) ; chantier `NetPlaytestInProgress.md`)*
- Lanceur : essai avec un ami *(L0 à L13 dans `main`, décisions D475 à D482 au GDD ; kit et DuckDNS prêts ; serveur par version (D481) fait par le chantier réseau (R8a, dans `main` dd1e708) ; `LauncherInProgress.md`)*

### IA
- IA du prototype : simple mais joue chaque système ; trois niveaux et l'évolutive ; duels, build, héros, forum à vies, volcan *(D421 à D427, GDD § 14.4 ; découpage tranché (IA-Q1 à IA-Q4), chaque nouveau système livre sa part IA ; IA0 (socle) en cours : IA0.2-IA0.3 dans `main` ; IA0.1 (PlayerState par emplacement IA, nom « IA <Niveau> <N> ») et IA0.4 B1 (cerveau en script qui pense sans agir) commitées dans Arwen (e8cf110, d42b5a8) ; IA0.4-Q1 à Q4 tranchées (A, A, A, A puis C à IA4) ; B2 (le cerveau envoie récolter) et B2b (interrupteur aiBrain) commitées dans Arwen (b988cf0) ; B3 (règle en fonction pure, 6 tests en script) commitée (5168def) ; B4 (sources récoltables seulement) commitée (4b5a3eb) ; B5 (cadence au vrai pas) commitée (998cce4) ; `main` (20e8f8a) fusionnée dans Arwen (f4b06dc, tests 268/268) ; B6 (vraie partie : Paysans IA en récolte à 0,2 s, même graine = mêmes pas) validée par l'essai et commitée (154ac02) : **IA0.4 close**, tout dans `main` (40108a6) ; IA0.5 (serveur dédié : le cerveau pense sur serveur dédié, avec deux clients et dans le serveur empaqueté) close ; IA0.6 (IA au menu et au salon) : Q1-Q7 tranchées, S1-S2 (option IA ouverte, colonne Joueur fusionnée, Niveau grisé) validées par l'essai ; tout dans `main` (d30129f) ; S3 (menu → partie), L1 (gestionnaire : IA au salon), L2 (lancement avec IA), L3 (client C++), L4 (écran du salon) et L5 (partie de salon contre une IA, bout en bout) essais `ai_menu` et `ai_lobby` validés ; bug réseau du nom (joueur renommé à l'écran Multijoueur entré en spectateur) corrigé ; **IA0.6 close**, tout dans `main` (1fcf4d3), D515-D516 au GDD ; IA0.7 (vérification) suivante ; IA1 (économie) : IA1.1 (5 Paysans de départ) et IA1.2 (ferme) dans `main`, IA1.3 suivante ; chantier `AIInProgress.md`)*

### Interface
- Barre du bas trop large sous 1500 px (touche la place de la mini-carte) *(à corriger avec la mini-carte)*

### GDD
- Review du GDD : séances closes, reste la courbe d'XP (Q04, à régler en test) ; prochain numéro D499 (§ 22)

### Moteur
- Moteur propre à HeroesOfLiberty avec cible Server : AS_3 appartient désormais à ProjectShiva (réinstallé à la demande de l'utilisateur le 2026-10-28, sans Server) ; moteur fabriqué (2026-10-29 03:43) et installé sous `C:\UE\UE_5.8_HOL` (« UE 5.8 HOL »), `.uproject` repointé, dans `main` (e6f1467) ; 7 copies recompilées, paquet complet et `packaged_smoke` PASS (2026-10-29 04:05) : **le serveur dédié cuit de nouveau** ; serveur de test cuit depuis `main` 8ea1b8e (2026-10-29 11:25) et copié dans MyCrusader, Arwen, Celeborn, Elrond, Galadriel ; partie `lobby` PASS avec lui dans Elrond ; défaut d'outil (ports des tests de salon en course entre copies) corrigé dans Glorfindel le 2026-10-29 (`copy_ports.ps1`, plages par copie, non commité) *(`NetPlaytestInProgress.md`)*
- Angelscript au fil des chantiers : le neuf en script, un fichier migrable passe en script quand un chantier va beaucoup le toucher *(D483 ; tests en script et cache précompilé dans le paquet : D484 ; `AngelscriptInProgress.md`)*

### Questions techniques ouvertes
*(les points de déplacement, ici et dans « Retours d'essai » : chantier `MovementInProgress.md`, après la clôture de J7)*
- Écart inexpliqué des mesures de rendu *(145-170 i/s le 2026-10-11 contre 247-258 le 12, mêmes conditions ; depuis, 218 à 277 i/s à chaque mesure au repos ; `ImplementationInProgress.md`)*

## Bugs remontés

### Retours d'essai

### Revue de code
- `LastAttacker` n'expire jamais (riposte à travers la carte) *(avec le brouillard de guerre, M7 : l'oublier quand il sort de la vue)*

### Aléas et petits restes
- Partie de salon finie : le serveur s'arrête 2 min après la fin même avec des joueurs dedans (`hol.Net.QuitAfterFinished`), contre « rester aussi longtemps qu'on veut » (M6-Q7 revu) *(vu en R9.3 ; corrigé dans Glorfindel le 2026-10-28 : le serveur reste tant qu'il y a quelqu'un, rouge puis vert ×2, non commité ; `NetPlaytestInProgress.md`)*
- `net 2 clients` : « manager: empty once, last » rouge une fois (`-Full`, 2026-10-28), vert à la relance
- `net 2 clients` : maison de l’autre arrivée avant le `waitlog` du client 1 (séquence arrêtée, vu une fois)
- `net 2 clients` : client 2 lent au démarrage, arrivé après le client 1, n'a jamais vu le bandeau d'attente (séquence arrêtée, vu une fois)
- Attaque d'un bâtiment sans ordre direct (attaque-déplacement, recherche automatique, file Maj) : pas de places autour du mur, ancienne règle du point le plus proche *(`EtapeDefiniInProgress.md` O : O.1 rouge montré ; O.2 faite (O.2a places aux nouveaux venus, O.2b l'arc tenu grandit par ses bouts : quatre modes verts, graines 1 et 7) ; O.3 faite le 2026-10-28 (scénario `attack_places`, contrôles verts) ; essai : la règle change (3 rangs face à l'arrivée, l'arc s'élargit avec le nombre, tous les ordres, bâtiments seuls, futur D499) ; O.5a faite (tests rouges montrés) ; O.5b faite (building_crowd 32/32, 4 modes x graines 1 et 7) ; O.5 faite (blocs de 3 rangs, verts) ; 2e essai : trop d'allers-retours, la règle devient une foule à la AoE4 (frappe à portée, ceux de derrière poussent, débordement sur les côtés ; GIF de l'utilisateur) ; O.7a faite (critères de foule rouges sur les blocs : détours, frappe tardive) ; O.7b repris le 2026-10-28 (foule codée, places retirées ; `main` 9e72bf4 fusionnée, compilé ; 2e boucle faite : corps serrés réglés, départs après la frappe 12-17 → 1-5 ; 3e boucle faite : ceux de derrière contournent, plus de 4e rang ; reste : un soldat à portée marche encore le long de la foule ; arrêté le 2026-10-28 à 16 h 30 pendant la 4e boucle, voir « Reprise exacte (O) »), Haldir, branche `attack-places`, non commité)*
- Unité arrêtée près d'un bâtiment : la garde du mur visible a des coins arrondis, jusqu'à ~35 cm dans le coin du socle *(C.1 ; J l'a corrigé pour les seules unités qui frappent)*

## Fichiers de développement

### Références
- `GDD.md` : conception du jeu, journal des décisions (§ 21) et point de reprise de la review (§ 22)
- `UnitStats.md` : tableau des stats des unités, produit par `Tools/Data/make_unit_stats.py` (ne pas modifier à la main)
- `Manager/PROTOCOL.md` : protocole du gestionnaire de salons (M6)
- `Tools/Scenarios/README.md` : scénarios de test, lancés par `play.ps1`

### Chantiers en cours
- `JourneyInProgress.md` : parcours du joueur (D252), J7 close (D451 à D469), plan de J8 (le héros) en cours, Q1 à Q6 tranchées, questions de J8a tranchées, J8a.0 à J8a.2 faites (pièces, type `Hero`, PV en float, naissance, F4), J8a.3 faite (fiche réseau du héros), J8a.4 faite (mort et retour, 7263184), J8a.5 faite (bloc du héros en script, aa0f11f), J8a.6 faite (vérification, 8ea1b8e), **J8a close** ; questions de J8b (XP et niveaux) ouvertes, questions de J8b closes (Q1 à Q6), J8b.1 faite (239d10e, Celeborn), J8b.2 en cours (boucle 1 faite, 585d49c)
- `AIInProgress.md` : IA du prototype (D421 à D427), découpage tranché, IA0.1-IA0.6, IA1.1 et IA1.2 (ferme) dans `main` (1fcf4d3), IA0.6 close (D515-D516), IA0.7 suivante, plan d'IA1 écrit
- `AngelscriptInProgress.md` : Angelscript (D483, D484) ; AS1 (tests en script dans quick_check) et AS2 (cache précompilé au paquet) faits ; en veille (AS-Q3 = A) : le code en script vient d'IA0.4, de J8a.4 et de J8a.5 (écran de jeu en script `UHOLScriptScreen`, `NSLOCTEXT` en script)
- `VictoryInProgress.md` : conditions de victoire, V1 à V5 dans `main` ; paliers après J8b (P-Q1, 2026-10-27) ; noms de salon dans les messages dans `main` (e53b39a)
- `NetPlaytestInProgress.md` : réseau M6 (salons, serveurs dédiés), R8a, R8c dans `main` (9364046), R8b publiée, pause refusée en réseau (e4884fc) ; R9.3 au GDD (D499-D514) ; serveur gardé tant qu'il y a quelqu'un dans Glorfindel (f59902d avec `main` a959e0e : build et `tests,net` verts, reste `-StageOnly` et `lobby` avant `main`, **bloqués** : moteur AS_3 réinstallé le 2026-10-28 à 15:40 par l'outil de ProjectShiva sans cible Server ; moteur « UE 5.8 HOL » prêt le 2026-10-29, chantier repris : `main` 8ea1b8e fusionnée dans `net-playtest` (f95c4e8), build vert, `tests,net` puis `-StageOnly` et `lobby` en cours avant l'avance de `main`) ; pause limitée (D513 : 3 par partie, 60 s) faite, essayée par l'utilisateur et commitée (5dd83fd, `net-playtest`) ; ports des tests de salon par copie (`copy_ports.ps1`, non commité) ; puis remesure de `MaxServers` et publication r9 ; R8d reporté
- `EtapeDefiniInProgress.md` : lignes d'EtapeDefini prises hors des grands chantiers ; O (places pour les attaques de bâtiment sans ordre direct, Haldir) ouverte, O.7b arrêtée (4e boucle à reprendre) ; N close (main 3026278) ; P close (main e9cc592) ; Q close (main 0adb67a)

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
- `CrossbowInProgress.md` : l'Arbalétrier et le Champ de tir (D486 à D489)
- `MapSizeInProgress.md` : tailles de carte, Plaine de Petite à Immense (D490 à D496)
- `TestSpeedInProgress.md` : quick_check en pas fixe et sans dessin pendant les attentes (`-Full` ~10 min -> ~4 min 20)
