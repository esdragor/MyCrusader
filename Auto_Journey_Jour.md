# Auto_Journey : l'état du jour

Procédure : Auto_Journey.md. Ce fichier ne contient que l'état de la journée (ou du test en cours). Ce qui attend l'utilisateur : Auto_Journey_Retours.md. File de travail : Auto_Journey_File.md. Historique du test du 2026-10-30 : Docs/Archive/Auto_Journey_Test1_2026-10-30.md.

## Décisions prises le 2026-10-30 (séance de questions)

| Question | Réponse | Où c'est noté |
|---|---|---|
| F-Q3 rendu du noir du brouillard | **A, plan dédié** (texture de cases du client, réutilisée par la mini-carte) | `FogInProgress.md` |
| M-Q2 effets de terrain du prototype | **A, seulement le visible** (marais, forêt, route, eau) ; couvert et hauteur après le prototype | `MapsInProgress.md` |
| M-Q9 petits tas variés | **A, types « petit filon »** d'or et de pierre (500) à budget égal, pas de gibier | `MapsInProgress.md` |
| Hommes d'armes inactifs autour du héros | **A, à corriger** : attaquants en trop d'une cible encerclée réattribués à un ennemi proche | `EtapeDefini.md`, `JourneyInProgress.md` |
| Lanciers ennemis -> Archer | déjà tranché (Archer gardé) ; reste le texte du GDD § 7.2 à harmoniser | `AIInProgress.md` |
| Paliers (chantier `tiers`, Elrond) | P-Q2 = A déjà tranchée (socle seulement : palier = f(niveau du héros), panneaux de bâtiments, vies du forum) ; P1 faite (a041984). **Les 3 valeurs par défaut confirmées (2026-10-30)** : le palier ne baisse jamais, même héros mort ; sans héros palier 0 ; un forum déjà tombé gagne la vie du palier dès qu'il est atteint | `VictoryInProgress.md` |
| M-Q3 le pont (Les Gués) | **A, chantier comme une ruine** (mécanique V3 réutilisée) | `MapsInProgress.md` |
| Mini-carte du prototype | **A, façon AoE4 complète** : camps en couleurs, sources explorées, cadre de caméra, clic gauche = caméra, clic droit = ordre de déplacement, alertes qui clignotent ; suit le brouillard | `InterfaceInProgress.md` |
| Fusion dans `main` demain | **A : oui, dès qu'un chantier fini a son `quick_check` des parties touchées vert** (libère le worktree ; rien n'est poussé) ; les essais restent à faire au retour | ici |

## Tableau des agents (une ligne par agent lancé ; remplace l'ancien journal)

Chaque agent ajoute/complète SA ligne ; la session principale ne recopie pas les rapports. Le bilan est généré depuis ce tableau.

| Heure | Worktree / branche | Tâche (ligne de la file) | État | Commits | Non vérifié |
|---|---|---|---|---|---|
| | | | | | |

## Programme de la journée (ordre proposé)

Créneau 1, machine calme (matin) : **mesures au repos** de la clôture de J8 (C2, `Tools/Tests/rest_measures.ps1`), seules.
Créneau 2, en parallèle (un sous-agent par chantier, compilations en file) :

| Worktree | Branche | Chantier | Travail prévu |
|---|---|---|---|
| Celeborn | `hero-j8-close` | J8 clôture | commit de C1 et C3 (déjà faits) ; correctif des Hommes d'armes inactifs autour du héros (test rouge d'abord) |
| Faramir | `fog` | Brouillard | F7 : plan dédié (F-Q3 = A), unités cachées non dessinées, fantômes teintés, ligne « Fog » de `hol.HUD.Report` ; puis texture et lecteur de cases pour la mini-carte |
| Arwen | `ai-ia1` | IA3 | IA3.3 (attaque de la ruine), IA3.4 (Régicide), IA3.5 (scénario `ai_forum`), IA3.6 (clôture) |
| Haldir | `maps` | Cartes | types « petit filon » (M-Q9), reprise de C2/M-Q8 selon l'essai, puis C3 (El Dorado) avec M-Q2 = A ; Plaine remise au budget |
| Elrond | `tiers` | Paliers | selon les réponses aux questions des paliers (à poser) |
| Galadriel | libre | Divers | texte du GDD § 7.2 (Lanciers), recalage de la numérotation des décisions, reste d'`EtapeDefini` |
| Glorfindel | `net-playtest` | Réseau | rien d'ouvert avant l'essai E5 avec un ami ; seulement si un reste apparaît |

Créneau 3, machine calme (fin de journée) : **paquet et serveur à recuire** (C4 de J8), seul ; puis `quick_check` des parties touchées.

## Chapitres du GDD non commencés (relevé Arwen, 2026-10-30 16:45, par recherche dans `Source/` et `Script/`)

Relevé rapide, pas un audit : « aucun code » = aucun fichier ne porte le nom ou la règle ; les lignes marquées (?) sont à vérifier avant d'ouvrir le chantier.

- **§ 12 Volcan + option Catastrophe** (D205, D208, D377-D380, D398-D400, D427) : tout est tranché (coulées qui avancent, site à gisements riches, refroidissement puis basalte, autel 60 s, réarmement, IA opportuniste). Code : seulement le drapeau `bCatastrophe` (config, menu, salon). Manque : tout le moteur d'événement (état du site, lave, dégâts, autel, alertes, IA). **Choisi : `VolcanoInProgress.md`.**
- **§ 16.2 World State Manager** (D29) : états `Warning` etc. prévus, aucun code. Se construit avec le volcan (V1), pas à part.
- **§ 11 Technologies** (D27, D180, grilles 11.1 et 11.2) : trois conditions (niveau du héros/palier, bâtiment, coût), 70-80 % communes ; grilles de palier 0-3 écrites. Code : types de bâtiments et grille de commandes seulement (?), pas de recherche. Manque : table CSV des technologies, file de recherche, effets sur les statistiques.
- **§ 9.6 Arbre de talents** (D08, D103, D194 ; familles des six factions) : rangées 2-9, 1 talent sur 3, sceau à 3, talent clé à 5. Code : aucun. Manque : le contenu des talents de la faction du prototype (Ordre de l'Aube : Serments), points de talent, écran ; dépend du palier/niveau du héros (J8).
- **§ 10 Duel de héros** (D14, D17) : déclenchement, arène, kit semi-automatique, durée, issue largement tranchés. Code : quelques tests de héros/victoire et une mention dans l'IA. Manque : déclenchement, arène, kit de duel, écran (?).
- **§ 5.5 Marché** (D70) : échange de ressources décidé. Code : aucun (?). Manque : bâtiment, taux, interface.
- **§ 5.2 Ressources spéciales, § 6.1 Siège (D23), § 6.2 Bonus de hauteur** : décidés ; la hauteur est exclue du prototype pour les effets de terrain (M-Q2), le siège est partiel (?) ; à relire avant d'ouvrir.
- **§ 8.4 Unités neutres et monstres** (D32, D401 « Neutre ») : camps neutres décidés. Code : trace de « Neutral/Camp » dans la simulation (?) ; manque : camps, récompenses (gisement ou passage, D102), IA des neutres.
- **D384/D388 Centres secondaires et relais neutres** (XP de territoire) : règle de la ruine (D348) réutilisable. Code : une mention dans `HOLHeroXPRules.h` seulement (?) ; manque : lieux posés par la carte et prise par présence.
- **§ 13 Cinq autres factions** (Légions Noires, Enfants du Dragon, Cercle de l'Ombre, Héritiers du Feu, Templiers) : contenu écrit (D87, D106-D173) ; le prototype ne joue que l'Ordre de l'Aube, donc **après le prototype**, ne se prend pas.
- **§ 12.3 Événements futurs** (D209 : inondation, invasion de monstres, faille, tempête) : décidés, **après le prototype** ; dépendent du volcan (V1).
- **§ 14.3 Modes de jeu : Battlefield, Reliques** (D416, D102) : **après le prototype**, ne se prend pas.
- **§ 9.9 Progression hors partie** : décidée dans les grandes lignes ; **après le prototype** (comptes, serveur persistant).

- 16:42 Elrond/Paliers P4 PRÉPARÉ (20 min) : nouveau cas `quick_check -Parts build:tiers` (héros niveau 1, 3, 6), rouge d'abord, **2 vrais défauts d'écran trouvés** (panneaux d'âge verrouillés aux niveaux 3 et 6 ; bloc du forum debout avec vies périmées) corrigés dans `HOLGameScreen.cpp/.h` ; `build:tiers` PASS, `tests` 485/485 ; commit 48e3244 sur `tiers` (main e6fbaae fusionné dedans, pas dans main). Scénario `tiers` écrit, non lancé. Reste : essai, `net` et `victory` à rejouer quand `ai-ia1` sera dans main, P5. Note de processus : ne pas mélanger `victory` et `build:*` dans un même passage de `quick_check`.

- 16:47 Faramir/Fog F7.3 en partie (20 min) : ligne `HUD fog` de `hol.HUD.Report`, unités cachées non dessinées, anneaux/barres/lignes filtrés, scénario `fog` ; commits a94304b, 648da21 sur `fog` ; 383/383. Pas de test pour le dessin (preuve par captures hors écran). Reste : double clic « même type », fantômes teintés, arbres coupés -> agent neuf lancé (12 min, a d'abord). Anomalie : l'horloge de Git Bash retarde de 5 min sur PowerShell (l'arrêt dur se lit à PowerShell).

- 16:50 Haldir/Cartes (arrêt 16:50) : le travail non commité (58 fichiers) vérifié et commité en 3 lots (c3579ba, 3594b4f, 533f480 : tests 329/329, rocks 17/17, sizes OK) + 89ce28f (arbre un tiers plus bas, M-Q10). **M-Q10 (décor retiré, sol uniforme, cartes régénérées) NON commité et rouge** : `Map.Table` contrôle 7 (cercle vide <= 28 m) échoue sans décor ; arbres x3 impossible avec le solveur actuel (budget remis à 150/225). Question M-Q10b (A refaire le solveur, B affleurements bloquants, C relâcher la règle) -> « Questions sans réponse ». Attention : `git checkout` des .umap de Haldir ramènerait les cartes avec décor.

- 16:51 Galadriel/`build` rouge CLOSE (commit 97bbe4b sur `galadriel-tab`, seul `quick_check.ps1` change) : pas de régression du jeu, deux défauts du script de test (héros qui dérive de 1 m : bannière demandée à 17 m plantée, à 16 m refusée ; `TestCursor off` laissait le vrai curseur faire défiler la caméra). `-Parts build` entier PASS en 138 s.

- 16:52 Arwen/Volcan V1 (chapitre GDD, ~10 min) : 12 chapitres non commencés listés dans la section « Chapitres du GDD non commencés » (recherche rapide, les (?) sont à vérifier ; 4 sont « après le prototype ») ; chantier Volcan/Catastrophe ouvert (`VolcanoInProgress.md`, plan V0-V12, C++/AS selon D483, ligne dans `EtapeDefini.md` vérifiée) ; V1 = cycle pur d'un site (Dormant sans Catastrophe, alerte, éruption, refroidissement, repos), test rouge puis vert, `tests` 486/486, commit b8bfb9e sur `ai-ia1`. Questions V-Q1 à V-Q4 (fenêtres de Catastrophe, tirage équilibré, tracés sous le brouillard, vitesse de la lave) -> « Questions sans réponse ». V2 bloquée par V-Q4, V3 par V-Q2.

- 16:53 Faramir (agent n°3, F7.3 b/c) : rien codé (10 min restantes, trop peu pour test rouge + compilation + quick_check + commit) ; constat et plan écrits dans `FogInProgress.md` : `AHOLBuilding::RebuildPieces` ne lit pas la vue, `GetGhosts(Slot)` n'est lu que par le HUD ; règle pure `HOLVisionRules::BuildingDrawsFromMemory` (test rouge), `ELook::Memory`, puis (c) arbres via fantômes de sources. Worktree Faramir libre.

- 16:57 Celeborn/J8 : C1 commité (b297add) ; correctif des Hommes d'armes en surnombre (dd16203, `hol.Combat.SurplusReassign`, test `Hero.SurplusAttackers`, 359/359, `-Parts tests,combat` vert, `-Parts hero` non lancé) ; Paysan sans agression ne se rabat pas ; pas d'essai joueur. Tous les agents ont rendu leur rapport avant 17:00.

## Bilan

(à générer à la fin depuis le tableau des agents : fait, bloqué, commits par worktree, état de main, anomalies du processus, améliorations proposées)
