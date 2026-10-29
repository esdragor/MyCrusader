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
- Héros Paladin-Commandant (J8a) : apparition, contrôle, mort et retour *(J8a.0 pièces, J8a.1 type `Hero` (×0,3 exact, PV en float), J8a.2 naissance près du forum, `SKM_Hero` et F4, J8a.3 fiche réseau (vie du héros en part de 255, fiches publique et privée écrites par le serveur, PV exacts chez le client, contrôle `net hero sheets`), J8a.4 mort et retour (règles et décision en Angelscript, achat `hol.Hero.Return`, contrôle `net hero return`, alerte « héros prêt », 7263184), J8a.5 bloc du héros (écran de jeu en Angelscript `UHOLScriptScreen`, module d’interface chargé par le serveur, portrait, PV, bouton « Ramener le héros », textes fr/en/ja, aa0f11f), dans `main` ; J8a.6 vérification (partie `hero` de `quick_check`, scénario `hero`, essai validé, 8ea1b8e), **J8a close** ; défaut noté à l'essai : des Hommes d'armes inactifs autour du héros (mêlée contre une cible unique) ; **J8b (XP et niveaux) : questions ouvertes (Q1 à Q6), Q1 tranchée (combat, commandement, développement), Q2 tranchée (XP = coût pondéré : militaire > économie > exploration), Q3 tranchée (+8 % PV et attaque par niveau), Q4 tranchée (barre d'XP, alerte, éclat doré, paliers annoncés), Q5 tranchée (délai 30 s + 8 s × niveau, prix 100 + 50 × niveau), Q6 tranchées (J8b.1 à J8b.6, un essai à la fin) ; J8b.1 (règles pures de l'XP, tests rouges puis verts, 239d10e sur hero-j8a3) faite, J8b.2 (XP dans la partie) faite : niveau sur le corps du héros (585d49c), XP des morts et des bâtiments achevés (dc9b3c5), cas `build hero xp` de `quick_check` (78d5608) ; J8b.3 (réseau et retour) faite : prix 100 + 50 × niveau, PV max du héros au niveau chez le client et dans son bloc (b92195a), contrôle `net hero xp` à 2 clients et retour niveau 2 à 200 or dans `build hero xp` (ef696cd)  ; J8b.4 (présentation) faite : barre d'XP du bloc (75ef3f8), alertes de niveau et de palier avec un son, éclat doré vu de tous, textes fr / en / ja, contrôles et captures (23c0e85) ; J8b.5 (vérification : scénario `hero_xp`, essai de l'utilisateur) faite : essai validé, 0204eb9 ; à reprendre : son de montée de niveau en chœur ou cor de guerre (demande de l'essai) ; J8b.6 (IA, rien à coder) suivante** ; serveur empaqueté à recuire (il lie maintenant l’interface) ; `JourneyInProgress.md`)*

### Fin de partie
- Conditions de victoire à coder : vies par palier (avec les paliers : P-Q1 = C, chantier des paliers juste après J8b, XP et niveaux), Nomade (D354, D207), garnison du Roi (D365, avec la garnison) *(D347 à D360, GDD § 14.1 ; V1 à V5 dans `main` ; chantier `VictoryInProgress.md`)*

### Cartes
- Nouvelles cartes du prototype : El Dorado, Les Gués, La Forêt profonde, La Couronne, La Steppe, Les Cols, L'Arrière-pays *(D496, D498 : dans le prototype, chantier à ouvrir ; GDD § 8.5 ; chacune sa recette, appelée par taille ; journal `Docs/Archive/MapSizeInProgress.md`)*

### Modes de jeu
- Mode *Battlefield* : salon où l'on choisit ses armées et où l'on apparaît avec *(après le prototype, D416)*

### Réseau
- M6 en parallèle : R8 redécoupé (le lanceur remplace R8.1-R8.5) : R8a serveur par version faite et dans `main` (dd1e708, contrôle `server_versions_check`), R8b publication à jour faite (version 2026-10-27-r8 sur le gestionnaire du raccourci, serveur par version), puis R8c mesure à 4 puis 8 clients (faite et dans `main` 9364046 le 2026-10-28 : 2,5 Mbit/s montants au pire par partie à 8, box en fibre : pas de limite), R8d essai E5 avec un ami (reporté) ; partie à X et clôture (R9) *(R1 à R7.10 et R8a dans `main` (dd1e708) : salons, spectateurs, statistiques de partie, fin de partie sans retour d'office, règle du chef, déconnexion et retour (« Revenir »), icône allié / ennemi et nom rayé ; essais E2, E3 et E4 réussis ; `MaxServers` 2 recommandé (R5.5), remesure D502 en cours (outil fait, mesure le soir du 2026-10-29) ; chantier `NetPlaytestInProgress.md`)*
- Lanceur : essai avec un ami *(L0 à L13 dans `main`, décisions D475 à D482 au GDD ; kit et DuckDNS prêts ; serveur par version (D481) fait par le chantier réseau (R8a, dans `main` dd1e708) ; `LauncherInProgress.md`)*

### IA
- IA du prototype : simple mais joue chaque système ; trois niveaux et l'évolutive ; duels, build, héros, forum à vies, volcan *(D421 à D427, GDD § 14.4 ; découpage tranché (IA-Q1 à IA-Q4), chaque nouveau système livre sa part IA ; IA0 (socle) en cours : IA0.2-IA0.3 dans `main` ; IA0.1 (PlayerState par emplacement IA, nom « IA <Niveau> <N> ») et IA0.4 B1 (cerveau en script qui pense sans agir) commitées dans Arwen (e8cf110, d42b5a8) ; IA0.4-Q1 à Q4 tranchées (A, A, A, A puis C à IA4) ; B2 (le cerveau envoie récolter) et B2b (interrupteur aiBrain) commitées dans Arwen (b988cf0) ; B3 (règle en fonction pure, 6 tests en script) commitée (5168def) ; B4 (sources récoltables seulement) commitée (4b5a3eb) ; B5 (cadence au vrai pas) commitée (998cce4) ; `main` (20e8f8a) fusionnée dans Arwen (f4b06dc, tests 268/268) ; B6 (vraie partie : Paysans IA en récolte à 0,2 s, même graine = mêmes pas) validée par l'essai et commitée (154ac02) : **IA0.4 close**, tout dans `main` (40108a6) ; IA0.5 (serveur dédié : le cerveau pense sur serveur dédié, avec deux clients et dans le serveur empaqueté) close ; IA0.6 (IA au menu et au salon) : Q1-Q7 tranchées, S1-S2 (option IA ouverte, colonne Joueur fusionnée, Niveau grisé) validées par l'essai ; tout dans `main` (d30129f) ; S3 (menu → partie), L1 (gestionnaire : IA au salon), L2 (lancement avec IA), L3 (client C++), L4 (écran du salon) et L5 (partie de salon contre une IA, bout en bout) essais `ai_menu` et `ai_lobby` validés ; bug réseau du nom (joueur renommé à l'écran Multijoueur entré en spectateur) corrigé ; **IA0.6 close**, tout dans `main` (1fcf4d3), D515-D516 au GDD ; IA0.7 (vérification) en cours dans Arwen (53f3323) : IA contre IA sans fenêtre, aucun Paysan inactif, x19-x29 ; empreintes de deux courses différentes (pilotage, hors IA), scénario `ai_economy_start` prêt ; IA1 (économie) : IA1.1 (5 Paysans de départ) et IA1.2 (ferme) dans `main` ; IA1.3 (résumé étendu : population, bâtiments, Paysans par ressource, centre) faite et commitée dans Arwen (fe719aa, tests 289/289), pas dans `main` ; IA1.4 (ordres et place : poser, construire, produire, ralliement, anneaux, bloc moulin + 8 fermes) faite et commitée dans Arwen (882373e, tests 293/293) ; chantier `AIInProgress.md`)*

### Interface
- Barre du bas trop large sous 1500 px (touche la place de la mini-carte) *(prise le 2026-10-29 à la demande de l'utilisateur, avant la mini-carte ; **T** en cours, Galadriel, branche `bottom-bar-width`, `EtapeDefiniInProgress.md`)*

### GDD
- Review du GDD : séances closes, reste la courbe d'XP (Q04, à régler en test) ; prochain numéro D517 (§ 22)

### Moteur
- Moteur propre à HeroesOfLiberty avec cible Server : AS_3 appartient désormais à ProjectShiva (réinstallé à la demande de l'utilisateur le 2026-10-28, sans Server) ; moteur fabriqué (2026-10-29 03:43) et installé sous `C:\UE\UE_5.8_HOL` (« UE 5.8 HOL »), `.uproject` repointé, dans `main` (e6f1467) ; 7 copies recompilées, paquet complet et `packaged_smoke` PASS (2026-10-29 04:05) : **le serveur dédié cuit de nouveau** ; serveur de test cuit depuis `main` 8ea1b8e (2026-10-29 11:25) et copié dans MyCrusader, Arwen, Celeborn, Elrond, Galadriel ; partie `lobby` PASS avec lui dans Elrond ; défaut d'outil (ports des tests de salon en course entre copies) corrigé le 2026-10-29 (`copy_ports.ps1`, plages par copie, `main` c3d315f) ; serveur gardé et pause P5 dans `main` (2c02f5c), serveur de test recuit depuis ce code et copié dans MyCrusader seulement *(`NetPlaytestInProgress.md`)*
- Angelscript au fil des chantiers : le neuf en script, un fichier migrable passe en script quand un chantier va beaucoup le toucher *(D483 ; tests en script et cache précompilé dans le paquet : D484 ; `AngelscriptInProgress.md`)*

## Bugs remontés

### Retours d'essai

### Revue de code
- `LastAttacker` n'expire jamais (riposte à travers la carte) *(pris le 2026-10-29 sans attendre le brouillard : oubli après un délai sans nouveau coup ; **S** en cours, Elrond, branche `last-attacker-expiry`, `EtapeDefiniInProgress.md` ; M7 ajoutera l'oubli hors de la vue)*

### Aléas et petits restes
- `net 2 clients` : « manager: empty once, last » rouge une fois (`-Full`, 2026-10-28), vert à la relance *(**U** en cours, les trois aléas `net 2 clients`, 2026-10-29, Galadriel, `EtapeDefiniInProgress.md`)*
- `net 2 clients` : maison de l’autre arrivée avant le `waitlog` du client 1 (séquence arrêtée, vu une fois)
- `net 2 clients` : client 2 lent au démarrage, arrivé après le client 1, n'a jamais vu le bandeau d'attente (séquence arrêtée, vu une fois)

## Fichiers de développement

### Références
- `GDD.md` : conception du jeu, journal des décisions (§ 21) et point de reprise de la review (§ 22)
- `UnitStats.md` : tableau des stats des unités, produit par `Tools/Data/make_unit_stats.py` (ne pas modifier à la main)
- `Manager/PROTOCOL.md` : protocole du gestionnaire de salons (M6)
- `Tools/Scenarios/README.md` : scénarios de test, lancés par `play.ps1`

### Chantiers en cours
- `JourneyInProgress.md` : parcours du joueur (D252), J7 close (D451 à D469), plan de J8 (le héros) en cours, Q1 à Q6 tranchées, questions de J8a tranchées, J8a.0 à J8a.2 faites (pièces, type `Hero`, PV en float, naissance, F4), J8a.3 faite (fiche réseau du héros), J8a.4 faite (mort et retour, 7263184), J8a.5 faite (bloc du héros en script, aa0f11f), J8a.6 faite (vérification, 8ea1b8e), **J8a close** ; questions de J8b (XP et niveaux) ouvertes, questions de J8b closes (Q1 à Q6), J8b.1 faite (239d10e, Celeborn), J8b.2 faite (585d49c, dc9b3c5, 78d5608), J8b.3 faite (b92195a, ef696cd), J8b.4 faite (75ef3f8, 23c0e85), J8b.5 faite (0204eb9), J8b.6 suivante
- `AIInProgress.md` : IA du prototype (D421 à D427), découpage tranché, IA0.1-IA0.6, IA1.1 et IA1.2 (ferme) dans `main` (1fcf4d3), IA0.6 close (D515-D516), IA0.7 close (main b032f0f, empreintes renvoyées au chantier déplacement), IA0 close, IA1.3 faite (Arwen fe719aa), IA1.4 faite (Arwen 882373e, ordres et place, tests 293/293), prochain IA1.5, plan d'IA1 écrit
- `AngelscriptInProgress.md` : Angelscript (D483, D484) ; AS1 (tests en script dans quick_check) et AS2 (cache précompilé au paquet) faits ; en veille (AS-Q3 = A) : le code en script vient d'IA0.4, de J8a.4 et de J8a.5 (écran de jeu en script `UHOLScriptScreen`, `NSLOCTEXT` en script)
- `VictoryInProgress.md` : conditions de victoire, V1 à V5 dans `main` ; paliers après J8b (P-Q1, 2026-10-27) ; noms de salon dans les messages dans `main` (e53b39a)
- `NetPlaytestInProgress.md` : réseau M6 (salons, serveurs dédiés), R8a, R8c dans `main` (9364046), R8b publiée, pause refusée en réseau (e4884fc) ; R9.3 au GDD (D499-D514) ; serveur gardé tant qu'il y a quelqu'un dans Glorfindel (f59902d avec `main` a959e0e : build et `tests,net` verts, reste `-StageOnly` et `lobby` avant `main`, **bloqués** : moteur AS_3 réinstallé le 2026-10-28 à 15:40 par l'outil de ProjectShiva sans cible Server ; moteur « UE 5.8 HOL » prêt le 2026-10-29, chantier repris : `main` 8ea1b8e fusionnée dans `net-playtest` (f95c4e8), `tests,net`, `-StageOnly` et `lobby` verts, puis `main` 4f5c297 et b032f0f refusionnées (eba5dc1, 2c02f5c), build, cuisson, `tests,net` et `lobby` verts : **`main` avancée à 2c02f5c le 2026-10-29**) ; pause limitée (D513 : 3 par partie, 60 s) faite, essayée par l'utilisateur et commitée (5dd83fd, `net-playtest`) ; ports des tests de salon par copie (`copy_ports.ps1`) et attente du verrou de compilation par la cuisson (`net_match.ps1 -ubtargs=-WaitMutex`) dans `main` (c3d315f, validé le 2026-10-29) ; remesure de `MaxServers` (D502) : outil `max_servers_measure.ps1` et `net_match -Headless` faits dans Glorfindel (non commités, chaîne prouvée), mesure complète reportée au soir du 2026-10-29 (machine jamais calme l'après-midi) ; puis publication r9 ; R8d reporté
- `EtapeDefiniInProgress.md` : lignes d'EtapeDefini prises hors des grands chantiers ; O (foule autour d'un bâtiment attaqué, D517) close le 2026-10-29 (main 7640732) ; N close (main 3026278) ; P close (main e9cc592) ; Q close (main 0adb67a) ; R close (coins carrés de la garde du mur visible pour les unités arrêtées, code 09860a9, `main` a9e5636, 2026-10-29) ; S (`LastAttacker` expire, Elrond), T (barre du bas sous 1500 px, Galadriel) et U (aléas `net 2 clients`) ouvertes le 2026-10-29

### En pause
- `UnitStatsInProgress.md` : stats des unités, reste la charge (D345.3) avec la cavalerie
- `AnimationsInProgress.md` : chaîne de fabrication et catalogue des animations, resservira à chaque nouvelle unité

### Presque clos (un reste)
- `InterfaceInProgress.md` : grille du Paysan (D340) ; reste la barre trop large
- `LauncherInProgress.md` : lanceur (D475 à D482), clos ; reste l'essai avec un ami
- `ScenariosInProgress.md` : mécanisme des scénarios, encore cité par les autres journaux (mode d'emploi : `Tools/Scenarios/README.md`)
- `TileSizeInProgress.md` : case de 4 m et cartes de 220 × 220 (D344) ; reste S5 (positions sur 13 bits) à mesurer à la prochaine série de mesures ; la ligne plus marquée tous les 5 cases abandonnée le 2026-10-29

### Archivés (`Docs/Archive/`)
- `FrontEndInProgress.md` : menu (D240 à D251)
- `MillInProgress.md` : moulin (D346)
- `ImplementationInProgress.md` : M0 à M2 (suite dans `JourneyInProgress.md`) ; l'écart des mesures de rendu du 2026-10-11 clos le 2026-10-29, non reproduit (218 à 277 i/s à chaque mesure au repos depuis)
- `MovementInProgress.md` : déplacement (D405 à D415, mesures faites), clos le 2026-10-26
- `RidgeRockInProgress.md` : rochers (D439 à D444)
- `CrossbowInProgress.md` : l'Arbalétrier et le Champ de tir (D486 à D489)
- `MapSizeInProgress.md` : tailles de carte, Plaine de Petite à Immense (D490 à D496)
- `TestSpeedInProgress.md` : quick_check en pas fixe et sans dessin pendant les attentes (`-Full` ~10 min -> ~4 min 20)
