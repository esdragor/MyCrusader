# IA du prototype : chantier en cours

Découpage du chantier « IA du prototype » (D421 à D427, GDD § 14.4 et § 17), par système.
Fait sur papier et en lisant le code, sans compiler : le moteur Hazelight 5.8.4 se recompile avec sa cible Server (2026-10-26).
Rien n'est codé. Les questions sont posées à l'utilisateur une à la fois (options A / B / C, recommandation), puis consignées ici et au GDD.

## Ce que le GDD fixe déjà

- **D421** : simple, mais joue chaque système du prototype : économie et paliers, armée avec contre-unités, héros et pouvoirs,
  décision de duel, forum à vies (reconstruire, contester une ruine), volcan (éviter les coulées, prendre les gisements).
  « IA de premier ordre » : une mécanique que l'IA ne sait pas jouer est à revoir.
- **D422** : Facile, Normal, Difficile = réglages des mêmes curseurs (réaction, macro, agressivité et moments d'attaque, micro,
  choix des duels) ; difficulté évolutive (D329) = courbe posée sur ces curseurs, dès le prototype ; aucune triche (§ 16.4 :
  mêmes règles, mêmes informations qu'un joueur).
- **D423** duels (rapport de force pondéré par le moment), **D424** build (profil tiré, adapté à l'éclairage), **D425** héros
  (prudent, rôle selon le profil), **D426** forum à vies (selon ce que vaut la vie), **D427** volcan (opportuniste).
- **D367, D433** : en *Diplomatie*, chaque IA est seule contre tous, refuse toute proposition.
- **D71** : une IA tient la place d'un joueur déconnecté (défense, économie de base) pendant la fenêtre de reconnexion.
- **D483** : le neuf en Angelscript, sauf Mass, simulation, réseau et rendu.

## Ce qui existe dans le code (relevé du 2026-10-26, `main` 8e81e50)

- **Emplacement IA** : `EHOLSlotState::AI` (`Match/HOLMatchConfig.h:12-17`), accepté par le JSON de config et les scénarios ;
  grisé au menu (`HOLNewGameScreen.cpp:252`). Il reçoit un centre principal (forum à vies) et un Roi en Régicide
  (`HeroesOfLibertyGameModeBase.cpp:411-448`), **rien d'autre** : pas de niveau ni de profil dans `FHOLMatchSlot`.
- **Pas de PlayerState pour une IA** (créé dans `PostLogin`, humains seulement, avec le stock de départ) : production
  (`HOLProductionSubsystem.cpp:163`) et construction (`HOLConstructionSubsystem.cpp:278`) refusées, dépôt de récolte perdu
  (`HOLGatherProcessor.cpp:821`). **Prérequis commun** de tout le chantier.
- **Aucun code d'IA.** `AIModule`, `MassAI` et `StateTree` sont au `.uproject` / `Build.cs`, inutilisés.
- **Chemin des ordres** : RPC `ServerXxx` du `AHOLPlayerController` → `UHOLCommandSubsystem::IssueXxx(..., PlayerIndex)`, file
  appliquée au pas suivant, propriétaire revérifié (D288), vaincu refusé (D358). Une IA côté serveur peut appeler `IssueXxx`
  directement, la même file que les humains ; à sortir du contrôleur : `SendNonWorkersTo` et le plafond `MaxOrderedUnits`.
- **Lecture du monde** : `UHOLUnitSubsystem::GetUnitViews` (`FHOLUnitView` : id, propriétaire, type, position, activité,
  charge, PV, cible, Roi), `GetPopulationByOwner`, sources de `UHOLSourceSubsystem`, état des forums dans `AHOLGameState`.
- **Accroche** : pas fixe 20 Hz, `GetOnStepStarted` / `GetOnStepEnded` de `UHOLSimulationSubsystem`. Reproductible
  (`Seed`, `hol.Simulation.StepHashes`). Tests : `hol.Test.Run`, scénarios `.txt` + `.json` (emplacements IA possibles).

| Système (D421) | Dans le code ? |
|---|---|
| Récolte (bois, baies, pierre, or), maisons, camps, moulin, Caserne | oui (pas de ferme) |
| Production Paysan, Lancier, Homme d'armes, Archer | oui |
| Armée, contre-unités (4 armures, bonus par catégorie) | oui |
| Forum à vies : reconstruire, contester la ruine (V2, V3) | oui |
| Roi, Régicide (V5), abandon | oui |
| Paliers, technologies | **non** (aucun chantier numéroté) |
| Héros, pouvoirs, résurrection | **non** (J8, plan ouvert, Q4 modèle technique reportée) |
| Duel | **non** (exclu de J8) |
| Volcan | **non** (option `bCatastrophe` sans effet) |
| Brouillard, vision | **non** (M7) : aujourd'hui un humain voit tout aussi, l'IA qui lit tout ne triche donc pas encore |
| Diplomatie, marché, tribut | **non** |

## Découpage (IA-Q1 = A)

- **IA0 Socle** : économie de l'emplacement IA (PlayerState, stock de départ), cerveau côté serveur au pas de simulation,
  ordres par la file commune, emplacement IA ouvert au menu, partie IA contre IA lancée par scénario.
- **IA1 Économie** : Paysans répartis, maisons avant le plafond, camps et moulin, production continue.
- **IA2 Armée** : composition contre l'armée vue, regroupement, défense de la base, attaques à des moments choisis.
  Arbalétrier (D486-D489, `main` ce7cc76, règle D421) : l'Archer sort du **Champ de tir** (`ArcheryRange`, case S du
  Paysan, 150 bois, 40 s ; A Archer, Z Arbalétrier), la Caserne ne fait plus que Lancier et Homme d'armes → l'IA doit bâtir
  le Champ de tir pour avoir des Archers, et produire des **Arbalétriers** (80 N + 40 or, 22 s, 12 + 10 contre Lourd, portée
  5) contre les Hommes d'armes.
- **IA3 Fin de partie** : forum à vies (D426), ruine adverse, Roi protégé en Régicide.
- **IA4 Niveaux** : curseurs, Facile / Normal / Difficile, évolutive (D422, D329) ; plus tard, parties à 8 IA pour nourrir
  les données (ouverture, besoins) et raffiner chaque niveau (IA1-Q4).
- **IA5 Tournoi automatique et réglage des niveaux** (idée validée par l'utilisateur le 2026-11-04, après le prototype) :
  pas d'apprentissage par renforcement (trop gros, boîte noire, contraire à § 16.4) ; l'IA reste scriptée et ses curseurs
  (D422) sont réglés par des parties IA contre IA en masse (1v1, 2v1, 2v2, ... sur les 7 cartes, sans fenêtre, `Seed`).
  Outil : N parties entre deux jeux de curseurs -> taux de victoire + mesures par partie (Paysans à 5 et 10 min, premier
  combat, ressources gaspillées, âge suivant, pertes) ; recherche simple sur les curseurs (grille, aléatoire ou évolution) ;
  les niveaux deviennent des points d'une échelle de force (cibles à fixer, ex. Normal bat Facile ~70 %, Difficile bat
  Normal ~70 %) ; l'évolutive (D329) vérifiée comme courbe régulière ; analyse des parties perdues par un agent qui
  PROPOSE des changements de règles, validés par l'utilisateur ; gardes-fous : toujours une part de parties contre
  l'utilisateur et les testeurs (une IA réglée contre des IA seulement devient exploitable par un humain). **Parties IA
  contre Joueur analysées aussi** (ajout de l'utilisateur, 2026-11-04) : chaque partie où une IA affronte un humain
  (solo, salon, essais) est journalisée avec les mêmes mesures et analysée comme les parties IA contre IA ; elle pèse
  plus que celles-ci pour juger la force réelle des niveaux et trouver les trous exploitables (à trancher : journal
  automatique ou non, consentement des joueurs en réseau, format des mesures). **Nouveaux paliers possibles** (idée de
  l'utilisateur, 2026-11-04) : un palier = un point de l'échelle de force (cible du type « Très difficile bat Difficile
  ~70 % »), le tournoi cherche les curseurs qui l'atteignent ; réglage par curseurs seulement (aucune triche, § 16.4),
  donc plafond = la meilleure IA scriptée ; bas (Très facile) : IA ralentie et affaiblie, sans plafond ; haut : plus de
  qualité d'IA (micro, défense, arbitrage des vies), 5 ou 6 paliers en tout au plus ; énumération `EHOLAIDifficulty` et
  menu à étendre (petit travail) ; l'évolutive devient une courbe entre deux paliers choisis, voire adaptative (monte si
  le joueur gagne, descend s'il perd). Recommandation : poser l'échelle d'abord avec les 4 niveaux actuels, décider des
  paliers en plus après les mesures. Numéro D et cibles de force à trancher à l'ouverture.
- **Avec chaque système à venir** : héros (J8, D425), paliers et build (D424), duel (D423), volcan (D427),
  brouillard (M7 : la lecture du monde passe par la vision), déconnexion (D71, avec le réseau).

## IA1 Économie : relevé du code (2026-10-26 soir, `main` 4d79f71, lecture seule)

- **Préalable** : IA0.1 (PlayerState de l'emplacement IA) ; sans lui, production, construction et dépôt refusés.
- **Ressources** : 200 de chaque au départ (`PostLogin`, humains seulement), **aucun Paysan de départ** (centre et Roi
  seulement ; les scénarios font naître les Paysans). Arbres 100 bois, buissons 200 nourriture, filons 2 500 (pierre, or).
  **Pas de ferme ni de chasse** : sur `Plain`, 6 buissons par emplacement (1 200 nourriture) + 200 de départ ≈ 28 Paysans,
  puis plus de nourriture du tout (l'Homme d'armes, le Lancier et l'Archer en coûtent aussi). **La pierre ne sert à rien**
  (tout coûte du bois).
- **Récolte** : 0,7 /s, charge 10 ; dépôt le plus proche qui accepte ; source épuisée → la plus proche du même massif
  (8 m, bosquets à 20 m), sinon attente devant une source prise, sinon inactif ; jamais vers une autre ressource ni un
  massif lointain (un filon épuisé = Paysans inactifs).
- **Bâtiments** (`BuildingTypes.csv`) : Maison 2×2, 50 bois, 20 s, +10 pop ; camp de bois, camp de mine (pierre, or),
  moulin (nourriture) 2×2, 100 bois, 25 s ; Caserne 3×3, 150 bois, 40 s ; centre principal 4×4, +10 pop, Paysan 50
  nourriture 20 s, file de 5. Aucun prérequis ni palier. Plafond = somme des bâtiments finis, 175 au plus.
- **Plafond atteint** : le serveur **accepte et fait payer**, la tête de file attend (file bloquée) ; seule l'interface grise.
- **Construction** : `HOLPlayerOrders::Place` (snap à la grille de 4 m, `CheckPlacement` : hors carte, chevauchement,
  source, 40 m d'un centre ennemi, navmesh) ; la fondation n'est pas rendue à l'appelant. Seule recherche de place :
  `FindFreeFootprint` (anneaux jusqu'à 4 cases autour d'un obstacle). N bâtisseurs = N^0,63 ; bâtisseurs qui finissent un
  camp ou le moulin récoltent dans les 25 m ; sinon chantier suivant à 20 m, sinon inactifs.
- **Production** : `IssueProductionOrder` (file, annulation, ralliement sur point / source / chantier) atteinte seulement par
  les RPC du contrôleur, pas par `HOLPlayerOrders`.
- **API de l'IA aujourd'hui** : résumé = Paysans inactifs, stock, 8 sources par ressource (distance au dépôt) ; ordres =
  récolte et déplacement. **Manquent** : population (utilisée, plafond), bâtiments et chantiers, Paysans par ressource,
  position du centre ; ordres Place, Build, production, recherche de place.
- **Carte `Plain`** : 100 × 100 cases jouables ; départs sur un cercle de 150 m ; bois (40 arbres) à gauche à 18-27 m,
  baies à droite à 20-23 m, filons à 120 m du milieu (~60 m de deux départs, disputés), crête de rochers à 30 m devant.

## Questions

- ~~IA-Q1 calendrier~~ → **A** (choix de l'utilisateur, conforme à la recommandation, 2026-10-26) : socle et systèmes
  existants maintenant (IA0 à IA4) ; ensuite **chaque chantier qui ajoute un système livre aussi sa part IA** (règle de D421 :
  une mécanique ne sort que si l'IA sait la jouer). Écartés : B (IA d'un bloc quand tout existe : très tard, règles jamais
  lues par une IA), C (chapitre IA après chaque gros chantier : toujours un chantier de retard). À reporter au GDD (§ 14.4)
  à la clôture d'IA4, et dans le plan de J8 quand il reprend.
- ~~IA-Q2 langage~~ → **B** (choix de l'utilisateur, conforme à la recommandation, 2026-10-26) : **lecture du monde en C++**
  (résumé par emplacement : armées et composition, menaces près de la base, Paysans inactifs, ressources, forums ; porte
  du brouillard en M7), **décisions en Angelscript**, règles maison à priorités, réflexion 2 à 4 fois par seconde, ordres par
  `IssueXxx`. Pas de StateTree (asset d'éditeur, peu lisible en diff). Repli si Angelscript ne tourne pas sur le serveur
  après la recompilation : décisions en C++, le résumé reste. Écartés : A (tout en script, 1 400 unités parcourues par IA),
  C (tout en C++, contre D483). GOAP évoqué par l'utilisateur : coût de calcul faible, mal adapté à la stratégie d'un RTS
  (actions parallèles et longues, état chiffré, replanification constante, choix difficiles à expliquer) ; les prérequis
  d'une production se résolvent par l'arbre des bâtiments, sans planificateur.
- ~~IA-Q3 vérification~~ → **A** (choix de l'utilisateur, conforme à la recommandation, 2026-10-26) : trois étages :
  tests C++ du résumé (règle pure) ; parties IA contre IA lancées par scénario, sans fenêtre, reproductibles (`Seed`), avec
  des critères chiffrés par tranche lus dans les journaux (IA1, indicatif : aucun Paysan inactif plus de 10 s, jamais
  bloquée au plafond de population, pas plus de 400 d'une ressource en stock) ; essai de l'utilisateur contre l'IA à la fin de
  chaque tranche, scénario prêt. À vérifier au premier code : une partie de 10 min peut-elle tourner plus vite que le temps
  réel sans fenêtre ? Sinon, hors de `quick_check` (moins d'une minute), aux clôtures de tranche seulement. Écartés : B (essai
  seul : aucune garde quand un autre chantier change une règle), C (tests seuls : vérifie que ça marche, pas que c'est
  agréable).
- ~~IA-Q4 première tranche~~ → **A** (choix de l'utilisateur, conforme à la recommandation, 2026-10-26) : **IA0 seule, avec
  une décision visible** (Paysans inactifs envoyés récolter) : la plus petite tranche qui traverse toute la chaîne, Angelscript
  sur le serveur dédié compris ; sert d'essai réel à AS-Q1 et AS-Q2. Écartés : B (IA0 + IA1 : longue, un souci d'Angelscript
  sur le serveur vu au milieu), C (socle C++ seul : rien de visible, l'inconnue reste ouverte).
- ~~IA1-Q1 périmètre d'IA1~~ → **A** (choix de l'utilisateur, contre la recommandation B, 2026-10-26) : **la ferme d'abord**
  (D390 codée comme première étape d'IA1, puis jouée par l'IA) : économie complète, testable sur 10 min et plus. Écartés :
  B (IA1 sans ferme, ferme en ligne à part avec sa part IA : IA1 plus courte mais critères limités aux 5 premières minutes),
  C (buissons grossis le temps d'IA1 : réglage provisoire, l'IA n'apprend pas l'épuisement).
- ~~IA1-Q2 valeurs de la ferme~~ → **la taille qu'il faut pour en avoir 8 autour d'un moulin** (réponse de l'utilisateur,
  2026-10-26) : **2 × 2 cases (8 m)**, comme le moulin : 8 fermes font un carré de 6 × 6 cases (24 m) autour de lui, chacune
  le touche (côté ou coin) ; en 3 × 3, 6 au plus le touchent. Reste repris de la recommandation (à régler en test) :
  **75 bois, 15 s, 0,55 /s** (≈ 80 % des baies). Conséquence : la ferme **se traverse** (pas de corps ni de trou dans le
  navmesh, comme AoE4), sinon les fermes serrées enferment le moulin.
- ~~IA1-Q3 Paysans de départ~~ → **5** (réponse de l'utilisateur, 2026-10-26 ; recommandé : 6 comme AoE4 ; écartés : 0
  comme aujourd'hui, 3) : **5 Paysans posés devant le centre** de chaque emplacement (humain ou IA), stock de 200 inchangé ;
  valeur dans les réglages de partie, à régler en test. Tranche le point laissé ouvert par D280. Les scénarios qui font naître
  leurs Paysans sont à relire (comptes attendus).
- ~~IA1-Q4 répartition et choix de construction~~ → **A pour commencer** (choix de l'utilisateur, 2026-10-26) : **ouverture
  écrite en données** (liste d'étapes), puis **règle de besoins** (ce qu'elle veut dépenser dans la minute, Paysans répartis
  selon le manque). Écartés : B (table de pourcentages, aveugle à ce qu'elle veut bâtir), C (besoins dès la première seconde,
  ouverture peu lisible). **Marqué pour plus tard** (demande de l'utilisateur) : **parties à 8 IA** qui nourrissent ces données
  (ouverture, cibles de la règle de besoins) et un **raffinage selon le niveau** (avec IA4).
- ~~IA1-Q5 placement des bâtiments~~ → **A** (choix de l'utilisateur, conforme à la recommandation, 2026-10-26) : **une règle
  par type**, puis recherche de cases libres en anneaux (`CheckPlacement`) : camp au bord du massif visé ; moulin + 8 fermes
  en bloc 6 × 6 près du centre ; maisons en ceinture à 12-25 m du centre, côté opposé aux ressources ; Caserne vers
  l'ennemi (IA2). Recherche en C++ (requête offerte au script), règles en script ; vaut pour toute carte. Écartés : B (carte
  d'influence : plus de calcul et de réglages, mauvais placements difficiles à expliquer), C (gabarits par carte : `Plain`
  seule, cassés dès qu'une case est prise).

**IA1 tranchée (IA1-Q1 à IA1-Q5).**

**Découpage tranché (IA-Q1 à IA-Q4).**

## Plan d'IA0 (rien codé)

**Préalables** : moteur Hazelight 5.8.4 recompilé avec sa cible Server et projet recompilé de zéro (ligne « Moteur »
d'EtapeDefini). AS-Q1 et AS-Q2 tranchées par D484 (f62a53d) : tests du script en script (`Test_*` dans `*_Test.as`,
même `Automation RunTests`), cache de scripts précompilé par cible dans le paquet (`AngelscriptInProgress.md`).
Chaque étape : test rouge d'abord, sur la vraie chaîne.

1. **IA0.1 Économie de l'emplacement IA** : un PlayerState par emplacement IA au démarrage, sans contrôleur (`SetIsABot`),
   stock de départ comme un humain, retrouvé par `FindPlayerState`. Tests rouges : production et construction acceptées pour
   un emplacement IA, dépôt de récolte crédité. À vérifier : tout ce qui parcourt `PlayerArray` (salon, statistiques de partie,
   bandeau de fin, gestionnaire) doit accepter un joueur sans connexion.
2. **IA0.2 Ordres sans contrôleur** : sortir `SendNonWorkersTo` et le plafond `MaxOrderedUnits` du `AHOLPlayerController`
   pour qu'un ordre de l'IA suive les mêmes limites qu'un humain.
3. **IA0.3 Résumé C++** (lecture du monde par emplacement, IA-Q2) : Paysans inactifs, sources proches de ses dépôts, stock.
   Tests d'automatisation C++ (règle pure).
4. **IA0.4 Cerveau en Angelscript** : un cerveau par emplacement IA, réflexion tous les 5 à 10 pas (2 à 4 par seconde) au pas
   de simulation ; décision unique : chaque Paysan inactif part récolter la source utile la plus proche. Tests en script (`*_Test.as`, D484).
5. **IA0.5 Serveur dédié** : le script tourne sur la cible Server (partie à deux clients et un emplacement IA, et paquet
   `-server`). Repli si non : décisions en C++ (IA-Q2).
6. **IA0.6 Menu** : emplacement IA ouvert dans « Nouvelle partie » (niveau grisé jusqu'à IA4).
7. **IA0.7 Vérification** (IA-Q3) : scénario IA contre IA sans fenêtre : aucun Paysan IA inactif plus de 10 s pendant 2 min ;
   mêmes `hol.Simulation.StepHashes` sur deux courses de même `Seed` ; mesurer si la partie peut tourner plus vite que le
   temps réel. Essai de l'utilisateur : scénario `Tools/Scenarios/ai_economy_start.txt` (lui contre une IA).

## Plan d'IA1 (rien codé)

**Préalables** : IA0 close (IA0.1 PlayerState de l'emplacement IA, IA0.4 cerveau en script, IA0.5 serveur dédié), donc le
moteur recompilé. IA1.1 et IA1.2 sont des règles du jeu, sans IA : elles peuvent se faire dès que le moteur revient,
en parallèle d'IA0. Chaque étape : moins de 15 min par boucle, test rouge d'abord sur la vraie chaîne.

| Étape | Contenu | Fin |
|---|---|---|
| IA1.1 Paysans de départ | 5 Paysans posés devant le centre de chaque emplacement (humain et IA), valeur dans les réglages de partie (`HOLMatchSettings`, ini) ; scénarios qui comptent leurs Paysans relus | tests verts, `quick_check -Parts` touchées |
| IA1.2 Ferme (D390) | ligne `Farm` de `BuildingTypes.csv` (2 × 2, 75 bois, 15 s) et source liée (inépuisable, **une** place, 0,55 /s, dépôt au moulin ou au centre) ; **traversable** (ni corps ni trou dans le navmesh) ; forme provisoire (champ plat) ; case de la grille du Paysan ; nom fr / en / ja ; part IA = IA1.7 | tests verts, **essai de l'utilisateur** (8 fermes autour d'un moulin, scénario `farms`) |
| IA1.3 Résumé étendu | population (utilisée, plafond, en file), bâtiments et chantiers à soi (type, état, position, file de production), Paysans par ressource, position du centre | tests C++ verts |
| IA1.4 Ordres et place | `Place`, `Build`, production (file, ralliement) dans `UHOLAIOrderLibrary` (par `HOLPlayerOrders`, comme un humain) ; requête de place en anneaux autour d'un point (`CheckPlacement`), bloc moulin + 8 fermes | tests C++ verts |
| IA1.5 Ouverture | ouverture écrite en données (script) : 5 Paysans de départ répartis, Paysans en continu, première maison, camp de bois, moulin | tests en script verts |
| IA1.6 Besoins | règle de besoins : dépense voulue sur la minute, Paysans répartis selon le manque, réaffectés quand une source s'épuise (un filon vidé laisse ses Paysans inactifs) ; Paysans jusqu'à une cible (indicatif 80, § 5.3 : 70 à 90) | tests en script verts |
| IA1.7 Bâtiments | maisons avant le plafond (marge = production en cours pendant 20 s de chantier) ; camps au bord des massifs ; moulin + fermes quand les baies baissent ; règles de placement d'IA1-Q5 | tests en script verts |
| IA1.8 Vérification | partie IA contre IA de 10 min sans fenêtre (`Seed`) (critère « pas plus de 400 d'une ressource en stock » aboli le 2026-10-29) : aucun Paysan inactif plus de 10 s, jamais une file bloquée au plafond, nourriture jamais à court faute de fermes ; Paysans à 5 et 10 min notés (référence pour IA4) | **essai de l'utilisateur** contre l'IA (scénario `ai_economy.txt`) |
| IA1.9 Clôture | GDD (IA1-Q1 à IA1-Q5 : ferme 2 × 2, 5 Paysans de départ, ouverture puis besoins, placement par type ; numéros D libres), `EtapeDefini.md`, mémoire | commit |

**Plus tard (IA4)** : parties à 8 IA pour nourrir les données (ouverture, cibles des besoins) et raffiner chaque niveau.

## IA1.2 Ferme : relevé du code (2026-10-27, Arwen `ai-ia1` = `main` 60230cd, lecture seule)

Chemins sous `Source/HeroesOfLiberty/` sauf mention.

**1. Données**
- `Tools/Data/BuildingTypes.csv` → table `/Game/Buildings/BuildingTypes` par `Tools/Editor/make_building_types.py`
  (éditeur lancé sans fenêtre : `UnrealEditor.exe <projet> -ExecutePythonScript=<script>`, il remplit la table depuis le
  CSV et la sauve, puis quitte) ; struct `FHOLBuildingTypeRow` (`Buildings/HOLBuildingTypes.h:51-119`) ; chemin dans
  `Config/DefaultGame.ini:73-74`. Une colonne neuve = champ `UPROPERTY` dans la struct, **compiler d'abord**, puis le script.
- Sources : pareil, `Tools/Data/SourceTypes.csv` → `/Game/Resources/SourceTypes` par `Tools/Editor/make_source_types.py`,
  struct `FHOLSourceTypeRow` (`Resources/HOLSourceTypes.h`) : `DisplayName`, `Resource`, `Amount` (min 1), `Radius`,
  `Height`, `Mesh`. Rien d'autre (ni places, ni débit, ni inépuisable).
- Noms : en dur dans `Source/HeroesOfLibertyUI/HUD/HOLGameNames.cpp` (`LOCTEXT("Mill", "Moulin")`, l. 64-66 ; repli sur
  `DisplayName` de la table) ; en / ja dans `Tools/Localization/fill_po.py:240` ; recette du moulin
  (`Docs/Archive/MillInProgress.md:24-25`) : GatherText → `fill_po.py` → GatherText (`Config/Localization/Game.ini`).
  `HOLBuildingTypeNames` (`HOLBuildingTypes.h:99`) : ajouter `Farm`.

**2. Sources** (`Resources/HOLSourceSubsystem.h/.cpp`)
- `FHOLSource` (h:16-31) : type, ressource, position, rayon, `Remaining` ; « debout » = `Remaining > 0`. Créées par
  `AddSource(Type, Location, Amount)` (cpp:194, serveur seul, après le début de partie) : la carte (`AHOLSourceLayout`,
  cpp:150) ou le code (stress test). **Jamais retirées** : épuisées, elles restent dans la liste (`Take`, cpp:219 →
  `OnSourceUsedUp` : forme cachée, chunk réseau). Chaque source a sa forme et son trou de navmesh (`AHOLSourceField`,
  `OnSourceAdded` cpp:239-256) et part aux clients par chunks de 63 (`AHOLSourceChunk`).
- **Pas d'inépuisable** : `Amount` min 1 ; `MakeReport` additionne les `Remaining` (débordement si on met MAX_int32).
- **Débit par type d'unité et ressource**, pas par source : `Constants.GatherRates[Resource]` (colonne `GatherRates` de
  `UnitTypes.csv`, Paysan nourriture 0,7), appliqué `Simulation/HOLGatherProcessor.cpp:521` (× `hol.Gather.SpeedScale`).
- **Places** : aucune limite par source ; places = points sur un cercle de rayon `Radius + corps + 15 cm`, espacés de
  100 cm (`FindSourceSpot` cpp:969-1026, `HOLGather::GetSourceSpots`) : un buisson (rayon 110) en tient ~10.
- **Propriétaire** : aucun ; n'importe quel joueur récolte n'importe quelle source (`ApplyGatherOrders` cpp:346-401).
- Source liée à un bâtiment : rien n'existe. Naissance possible à la fin du chantier (`HOLBuildProcessor.cpp:794`,
  `SetConstructionState(Finished)` → `AHOLBuilding::SetConstructionState`, `HOLBuilding.cpp:186`) ; mort à la destruction
  (`UHOLConstructionSubsystem`, `HOLConstructionSubsystem.cpp:496-497` ; annulation l. 372, sans objet : pas fini).

**3. Récolte** (`Simulation/HOLGatherProcessor.cpp`)
- Place : au bord du corps de la source, côté du dépôt (`WalkToSource` l. 596, `FindSourceSpot` l. 969) ; contact à
  30 cm (`ContactGap`), la place doit être sur le navmesh (projection à 20 cm, rayon ouvert vers l'extérieur).
- Dépôt : le plus proche à soi qui accepte la ressource et **fini** (`RefreshDeposits` l. 306, `FindDeposit` l. 1141) :
  le moulin (`Accepts=(Food)`) et le centre acceptent la nourriture : rien à faire.
- Place prise : autre source du **massif** (même ressource à 8 m centre à centre, `NoRoomSourceDistance`), puis bosquet à
  20 m, sinon debout devant (`FindMassifSpot` l. 1054, `StandBeforeSource`). Des fermes de 8 m serrées (centres à 8 m)
  forment un massif, **avec les baies à 8 m** s'il y en a.
- Après construction : les bâtisseurs d'un « camp » (`Accepts` non vide, `IsCamp`, `HOLBuilding.cpp:113`,
  `SendsBuildersToGather` `HOLConstructionRules.cpp:454`) récoltent la source acceptée la plus proche à 25 m
  (`GatherForCamp`, `HOLBuildProcessor.cpp:625-655`). La ferme n'accepte rien : il faut un cas « bâtiment-source → ses
  bâtisseurs récoltent sa propre source ». Un moulin fini prendrait aussi une ferme à 25 m comme « source de nourriture ».

**4. Traversable**
- Tout passe par `AHOLBuilding::BlocksNavigation()` (`HOLBuilding.cpp:119`, aujourd'hui « pas une fondation ») : obstacle de
  navmesh `NavObstacle` (`FitNavObstacle` l. 665-703), garde des emprises (`HOLSimulationProcessors.cpp:347` et `2018`),
  corps sur le chemin (`HOLUnitOrderProcessor.cpp:222`, `955`), rapport (`Map/HOLNavReport.cpp:137`). **Un drapeau de type
  (`Walkable`) lu dans `BlocksNavigation()` suffit** : une fondation est déjà traversée ainsi (D312).
- Pas de collision physique des unités avec les pièces (requête « Selection » seule, `HOLBuilding.cpp:793-797`).
- `CheckPlacement` (`HOLConstructionRules.h:138`) ne regarde pas les unités : on pose sur des Paysans (déjà vrai) ; un
  autre bâtiment sur une ferme reste refusé (chevauchement). Places des bâtisseurs : autour de l'emprise, comme ailleurs
  (`HOLBuildProcessor.cpp:217`). Attaquants : autour du mur visible (`HOLUnitOrderProcessor.cpp:2865`).
- Clic : `FindSourceAt` (`HOLPlayerController.cpp:1844`) prend **toute** source debout par sa projection : la source d'une
  ferme serait choisie ; à écarter (type « lié »), le clic doit tomber sur le bâtiment.

**5. Interface**
- Case = colonne `BuildGridSlot` (index 0-11 de la grille 3 × 4 du palier), rangée = catégorie (`GetCategoryRow`,
  `Player/HOLCommandGrid.cpp:73`, vérifié par `HOLCommandGridTests.cpp:362`). Palier I, rangée économie **pleine** :
  A Maison, Z Bûcherons, E Mineurs, R Moulin → question IA1.2-Q1. Icône : aucune icône de bâtiment, la case montre le nom
  (`HOLGridCell`, D313). Attentes de `quick_check` à revoir : rapport HUD `grid ages I=open [...]` (`quick_check.ps1:2951`).
- Clic droit (`HOLPlayerController.cpp:2097-2153`) : chantier à soi → bâtir ; ennemi → attaquer ; source → récolter ; sinon
  marcher. **Ajouter** : ferme finie à soi → récolter sa source ; ralliement d'un bâtiment sur une ferme (l. 2162,
  `IsCamp()` faux pour la ferme) → sa source.

**6. Forme** : pas de mesh (colonne `Mesh` vide) ; formes provisoires en cubes dans le code
(`AHOLBuilding::BuildProvisionalShape`, `HOLBuilding.cpp:645-663`, une seule forme pour tous : bloc + tour). Le plus
simple : une branche « champ » (dalle brune basse aux couleurs du joueur sur le bord, quelques sillons verts) pour un type
`Walkable`, hauteur de la table ~20 cm. Pas de script `Tools/Editor` à faire.

**7. Tests et scénarios**
- C++ à imiter : `Construction.Tables` (`Tests/HOLConstructionTests.cpp:290`, lit la vraie table : ligne, case, coût),
  `Construction.CampSource` (l. 592), `Gather.Tables` / `Gather.Spots` / `Gather.Massif` (`Tests/HOLGatherTests.cpp`),
  `Buildings.Types` / `Buildings.Foundation` (`Tests/HOLBuildingTests.cpp`), `CommandGrid.Tables`.
- `quick_check` : partie `build` (`build hud` : attentes de la grille ; `build camp mill`, l. 1241-1248 et 3047-3082 :
  moulin aux baies du slot 7), `gather`, `footprints`.
- Scénarios proches : `Tools/Scenarios/mill.txt` (`hol.Build.PlaceNear Mill`, `hol.Units.SpawnHere`, `waitlog`),
  `berries_return.txt`.

**Choix techniques proposés** (à confirmer en boucle 2) : nouvelles colonnes de `SourceTypes.csv` : `Places` (0 = tout le
tour ; ferme 1), `RateScale` (1 ; ferme 0,55 / 0,7 ≈ 0,786, servira au Sanctuaire), `Endless` (ferme vrai : `Take` ne
décompte pas, rapport sans somme), `Linked` (ferme vrai : pas de forme, pas de trou, jamais choisie par `FindSourceAt`,
jamais placée par une carte) ; ligne `FarmField` (Food, rayon ~100 : le Paysan se tient **sur** le champ, côté du dépôt).
`BuildingTypes.csv` : colonnes `Walkable` et `Source` (nom du type de source liée) ; ligne `Farm` 2 × 2, 75 bois, 15 s,
`Walkable`, `Source=FarmField`. `FHOLSource` gagne un propriétaire (serveur seul : la ferme d'un autre jamais récoltée) ;
le bâtiment garde l'indice de sa source.

### Découpage d'IA1.2 (boucles de 15 min au plus, test rouge d'abord sur la vraie chaîne)

| Boucle | Changement | Test rouge d'abord |
|---|---|---|
| F1 | Colonnes `Places`, `RateScale`, `Endless`, `Linked` de `FHOLSourceTypeRow` + ligne `FarmField` de `SourceTypes.csv` ; compilation, puis `make_source_types.py` | `Gather.Tables` : `FarmField` lue de la vraie table (1 place, 0,55 /s pour le Paysan, inépuisable, liée) |
| F2 | `UHOLSourceSubsystem` : source inépuisable (`Take` ne décompte pas), source liée sans forme ni trou ni chunk visible, `RemoveSource` (fin d'une source liée), propriétaire | test sur le vrai sous-système en monde de test : `Take` sur `FarmField` rend la charge sans baisser ; `RemoveSource` la met à terre ; `MakeReport` sans débordement |
| F3 | Récolte : une seule place (`Places`), débit × `RateScale`, jamais la source liée d'un autre joueur | règle pure : `GetSourceSpots` borné à `Places` ; débit 0,55 sur 20 s de pas = 11 ; un second Paysan va à la ferme voisine (massif) ou attend |
| F4 | Colonnes `Walkable` et `Source` de `FHOLBuildingTypeRow`, ligne `Farm` (2 × 2, 75 bois, 15 s, case Q1) ; `HOLBuildingTypeNames::Farm` ; `make_building_types.py` | `Construction.Tables` + `CommandGrid.Tables` : ligne `Farm` lue de la vraie table |
| F5 | `BlocksNavigation()` faux pour un type `Walkable` (chantier et fini) | `Buildings.Foundation`-like : ferme finie, `NavObstacle` sans effet, un point au milieu de l'emprise est sur le navmesh ; maison inchangée |
| F6 | Ferme finie → sa source naît au centre ; ferme détruite → source à terre ; ses bâtisseurs récoltent sa source (premier), les autres le massif | test en monde : poser, finir, détruire une ferme par la vraie chaîne (`UHOLConstructionSubsystem`) |
| F7 | Clic droit sur sa ferme finie → récolte ; ralliement sur une ferme → récolte ; `FindSourceAt` ignore les sources liées | test du contrôleur (comme les tests de sélection) ou étape de `quick_check` `build farm` |
| F8 | Forme « champ » dans `BuildProvisionalShape` ; noms fr / en / ja (`HOLGameNames.cpp`, GatherText → `fill_po.py` → GatherText) | `Buildings.Appearance` : la ferme a une forme basse (< 30 cm) ; `fill_po` : 0 clé inconnue |
| F9 | `quick_check` : étape `build farm` (8 fermes autour d'un moulin, Paysans qui récoltent et déposent au moulin, un Paysan qui traverse le carré) ; attentes `build hud` | `quick_check -Parts build,gather,footprints` |
| F10 | Scénario `Tools/Scenarios/farms.txt` (+ `.json` si besoin) et sa répétition | `play.ps1 farms` sans fenêtre jusqu'à « prêt » |
| F11 | Essai de l'utilisateur, puis commit après validation | — |

F1-F3 et F4-F5 sont indépendantes ; F6 a besoin des deux.

### Questions de jeu laissées ouvertes par le code

- **IA1.2-Q1 Case de la ferme** : la rangée économie du palier I est pleine (A Z E R). A) palier I, rangée 3 (`A` `W`),
  exception à « une rangée par catégorie » ; B) déplacer un bâtiment (ex. Moulin en W, Ferme en R) ; C) la ferme au
  palier II. **Recommandation : A** (palier 0 au GDD § 912, deux touches gardées, rien ne bouge pour le joueur).
- **IA1.2-Q2 Ferme détruite** : A) la ferme meurt, son Paysan va à la ferme voisine ou devient inactif ; B) elle laisse une
  fondation à rebâtir. **Recommandation : A** (comme les autres bâtiments, décombres 10 s, D345.2a).
- **IA1.2-Q3 Stationner dessus** : toute unité peut marcher et s'arrêter sur une ferme (amie ou ennemie), comme AoE4 ;
  les autres bâtiments y restent refusés. **Recommandation : oui.**
- **IA1.2-Q4 Second Paysan sur une ferme prise** : A) il va à la ferme libre la plus proche du massif (même règle que les
  baies), sinon il attend devant ; B) refus net, inactif. **Recommandation : A.** Nuance : les baies à 8 m d'une ferme sont
  du même massif (un Paysan des baies épuisées passe à une ferme libre : voulu).
- **IA1.2-Q5 Défaite** : la défaite du prototype ne compte pas les bâtiments (forum à vies, Régicide) : rien à décider ;
  la ferme compte dans les bâtiments perdus des statistiques. **Recommandation : la laisser compter.**
- **IA1.2-Q6 Moulin fini près des fermes** : ses bâtisseurs « camp » prennent la source de nourriture la plus proche à 25 m,
  ferme comprise. **Recommandation : baies d'abord, ferme libre ensuite** (sinon un moulin bâti au milieu de 8 fermes
  vole la place d'un fermier).

## IA0.1 Emplacement IA : relevé du code (2026-10-27, Arwen `ai-ia1` = `main` 6cddb93, lecture seule)

Chemins sous `Source/HeroesOfLiberty/` sauf mention.

**1. Emplacements**
- `EHOLSlotState { Closed, Human, AI }` existe déjà (`Match/HOLMatchConfig.h:12-17`) ; `FHOLMatchSlot` (`:40-63`) : état,
  faction, couleur, équipe, `PlayerName` (humains nommés par le gestionnaire seulement). Pas d'« ouvert » ni de niveau d'IA.
- Menu « Nouvelle partie » : `AI` grisé (`HeroesOfLibertyUI/Screens/HOLNewGameScreen.cpp:252-253`, D240) → IA0.6. Salon :
  `EHOLLobbySlotState { Open, Closed, Player }` (`Net/HOLManagerClientRules.h:56`), aucun emplacement IA côté gestionnaire.
- Déjà utilisé : 38 emplacements `"state": "AI"` dans `Tools/Scenarios/*.json` (dont `starting_peasants.json`, IA en 1),
  `quick_check.ps1:409` (victoire), `:490` (Régicide), `:513` (3 emplacements), `building_crowd.ps1:31`. La partie `net` de
  `quick_check` (`:227-235`) n'a que 2 humains et des fermés.
- Un emplacement IA aujourd'hui : **centre, Roi, 5 Paysans, héros nés** (`HeroesOfLibertyGameModeBase.cpp:515-563` saute
  seulement `Closed` ; `FHOLStartRules` `HOLStartRules.cpp:14,44`), **compte dans la victoire** (`HOLVictoryRules.cpp:23`,
  `HOLForumRules.cpp:445` : tout non fermé), **jamais de PlayerState, donc pas de stock**. Nom affiché « Joueur N » (table des
  joueurs et bandeau : `GetLaunchConfig().Slots[...].PlayerName`, sinon « Joueur N » : `Match/HOLForumMessages.cpp:21-23`,
  `HeroesOfLibertyUI/HUD/HOLGameHUD.cpp:186-189`, `HOLForumRules::GetPlayersTable`).

**2. PlayerState**
- Classe : `PlayerStateClass = AHOLPlayerState` (`HeroesOfLibertyGameModeBase.cpp:98`). Seul chemin de naissance : la connexion
  (contrôleur). `PostLogin` (`:205-272`) : emplacements pris = `PlayerArray`, `FindSlotOnLogin` (humains seulement,
  `HOLMatchConfig.cpp:85,114`), `SetMatchSlot(Slot, Color)` (`Match/HOLPlayerState.cpp:44-48`), stock par
  `GiveSlotStock` (`HOLGameState.cpp:616-640`) puis `AssignBuildingOwners` (`:452-475`, propriétaire = son contrôleur).
- Recherche par emplacement : **cinq copies** de la même boucle sur `PlayerArray` (`GetPlayerIndex() == Slot`) :
  `UE::HeroesOfLiberty::Match::FindSlotPlayerState` (`Match/HOLGameState.cpp:531-547`, sert `GetSlotStock`),
  `UHOLConstructionSubsystem::FindPlayerState` (`Buildings/HOLConstructionSubsystem.cpp:244-260`),
  `UHOLProductionSubsystem::FindPlayerState` (`Buildings/HOLProductionSubsystem.cpp:125-141`),
  `UHOLHeroSubsystem::FindPlayerState` (`Simulation/HOLHeroSubsystem.cpp:214-230`, commentaire `.h:37` « nullptr for an AI
  slot »), `HOLAISummary::ReadStock` (`AI/HOLAISummary.cpp:130-147`). Un PlayerState IA dans `PlayerArray` est trouvé par
  toutes, sans retouche.
- Créer sans contrôleur (Unreal 5.8) : `SpawnActor<AHOLPlayerState>(PlayerStateClass)` sur le serveur ;
  `APlayerState::PostInitializeComponents` l'inscrit seul (`GameState->AddPlayerState`) ; `Destroyed` l'en retire. Puis
  `SetIsABot(true)`, `SetPlayerName`, `SetMatchSlot(Slot, Color)`, `SetResources(StartingResources)`. Les tests le font déjà
  (`Tests/HOLDisconnectTests.cpp:162-198` : `SpawnActor<AHOLPlayerState>` puis présent dans `PlayerArray`).
- Réplication : `APlayerState` est répliqué et toujours pertinent : le PlayerState IA arrive chez tous les clients
  (`PlayerIndex`, `ColorIndex` en `DOREPLIFETIME`, `HOLPlayerState.cpp:19-20`, `bIsABot` du moteur) ; ses champs
  `COND_OwnerOnly` (stock, population, récolteurs, chaînes, héros privé, `:21-26`) ne partent vers personne (sans
  connexion propriétaire) : l'économie de l'IA reste cachée (D282), rien à ajouter.
- Où le faire naître : au serveur, **avant** `SpawnMainCentres` dans `StartPlay` (`:489-492`), une fonction statique comme
  `SpawnKing` (`SpawnAIPlayerStates(UWorld&, const FHOLMatchConfig&, const FHOLResources&)`) pour la tester sans GameMode.
  `AssignBuildingOwners` sort tôt (pas de contrôleur, `:464`) : bâtiments IA sans `Owner`, comme aujourd'hui.

**3. Stock**
- Le stock vit dans `AHOLPlayerState::Resources` (`Match/HOLPlayerState.h:145`) ; R7 ajoute le stock gardé d'un absent
  (`AbsentSlots`) ; accès commun `AHOLGameState::GetSlotStock` / `SetSlotStock` / `AddSlotStock` (`HOLGameState.cpp:550-586`) :
  absent d'abord, sinon PlayerState, sinon **faux, rien écrit**.
- Crédit : dépôt de récolte `UHOLGatherProcessor::DepositLoad` (`Simulation/HOLGatherProcessor.cpp:887-897`) : sans
  PlayerState, la charge est **vidée et perdue** (le Paysan décharge, rien n'est crédité, aucun journal) ; remboursements
  de destruction / annulation `AddSlotStock` (`HOLConstructionSubsystem.cpp:422,445`) : perdus pareil.
- Débit : construction `ApplyOrders` (`HOLConstructionSubsystem.cpp:276-282`) : sans PlayerState, **`continue` muet** (ordre
  jeté sans journal) ; production (`HOLProductionSubsystem.cpp:161-167`) : rejet journalisé « Production order of slot N
  ignored: not its building » (message trompeur pour l'IA).
- Tests d'IA0.2 (`Tests/HOLAIOrdersTests.cpp:82-150`, emplacement 1 sans PlayerState) : ils vérifiaient la règle
  `HOLPlayerOrders` (emplacement 0-7, plafond 200, chaînes, `SendNonWorkersTo`) et le départ en récolte, **pas** le dépôt ;
  marchaient déjà sans PlayerState : ordres d'unités (déplacement, attaque, récolte, construire un chantier existant), la
  récolte elle-même. Ne marchaient pas : poser une fondation (`Place` passe par la construction), produire, créditer.
  IA0.3 : `Tests/HOLAISummaryTests.cpp:208-304` attend « slot 1: no PlayerState, no stock » (`bHasStock` faux) : **à
  retourner** en IA0.1 (le test `World` monte un emplacement IA sans PlayerState).

**4. Lecteurs de `PlayerArray` (et du contrôleur d'un PlayerState) avec un PlayerState IA**
- Sans risque (le PlayerState IA y est voulu ou sans effet) : `FindPlayerState` × 5 (ci-dessus) ; population publiée
  (`HOLProductionSubsystem.cpp:340-360`) ; chaînes d'ordres (`Commands/HOLUnitOrderProcessor.cpp:3970-3985`) ; récolteurs
  (`HOLGatherProcessor.cpp:977-985`) ; bulles de dépôt (`:998-1001`, contrôleur absent → sautées) ; effondrement D358
  (`HOLGameState.cpp:721-728`, stock mis à 0 : voulu) ; couleurs (`HOLPlayerState.cpp:29-42`, et
  `Presentation/HOLInterpolatedInstancesProcessor.cpp:93-100`) : la couleur de l'IA vient alors de la configuration au lieu
  de l'indice (mieux) ; `PreLogin` / `PostLogin` (`GameModeBase.cpp:173,216`) : l'emplacement IA pris ne gêne pas (les
  humains n'ont que des emplacements `Human`) ; `SendStartedIfAllHere` (`:351-359`, `Human` seulement) ;
  `SendEmptyIfNobodyLeft` (`:416`, contrôleurs : l'IA ne retient pas le serveur, voulu) ; `AssignBuildingOwners` (sort tôt) ;
  déconnexion R7 (`Logout` passe par un contrôleur ; `FHOLDisconnectRules::KeepsSlot` `Match/HOLDisconnectRules.cpp:7-16`
  = `Human` nommé seulement : l'IA n'est jamais absente) ; spectateur (PlayerState IA jamais `OnlyASpectator`, emplacement
  0-7 : accepté par `HOLPlayerOrders`) ; HUD (`HeroesOfLibertyUI` ne lit que le PlayerState local et `GetLaunchConfig`).
- Change ce qu'on voit (voulu, à vérifier) : statistiques (`Match/HOLMatchStatsSubsystem.cpp:122-145`) : l'IA a enfin ses
  courbes et sa ligne (`HeroesOfLibertyUI/Screens/HOLStatsScreen.cpp:275,393` ; avant : ligne vide) ; rapports
  « Production report … others [slot N …] » (`Player/HOLPlayerController.cpp:1194-1201`) et « Hero private slot N other »
  (`HOLHeroSubsystem.cpp:55-60`) : une entrée de plus en solo avec une IA. Les attentes exactes de `quick_check`
  (`quick_check.ps1:4697,4732,4783,4808`, « others [slot X F 0 …] ») sont dans la partie `net`, sans IA ; à confirmer par
  `-Parts victory,production` (parties solo avec IA en 1).
- **Correction minimale** : `UpdateStartWait` (`GameModeBase.cpp:290-297`) ajoute le nom vide de l'IA à `Present` (sans
  effet, mais sauter `IsABot()`) ; **`waitplayers N`** de `Dev/HOLTestSequence.cpp:293-297` compte tout `PlayerArray` : une
  partie réseau avec IA passerait trop tôt → ne compter que `!IsABot()`. Les cinq `FindPlayerState` : les remplacer par
  `FindSlotPlayerState` (un seul endroit) n'est pas nécessaire à IA0.1 (option).

**5. Tests à imiter**
- Modèle pour IA0.1 : **`HeroesOfLiberty.Match.DisconnectGains`** (`Tests/HOLDisconnectTests.cpp:151-230`) : vrai monde
  (`UWorld::CreateWorld(Game)` + `InitializeActorsForPlay`), `AHOLGameState` + `AHOLPlayerState` par `SpawnActor`, centre et
  fondation, `UHOLProductionSubsystem`, vrai dépôt `UHOLGatherProcessor::DepositLoad`, vraie destruction ; avec le monde
  d'IA0.2 (`HOLAIOrdersTests.cpp:82-150` : processeurs d'ordres et de récolte au pas, emplacement 1 IA) pour faire passer
  les ordres de construction / production par `UHOLCommandSubsystem` → `ApplyOrders` au pas (vraie chaîne). IA0.3
  (`HOLAISummaryTests.cpp:231-304`) pour le stock lu par le résumé.
- Aucun test ne passe par `StartPlay` (relevé d'IA1.1) : la naissance se teste par la fonction statique, la vraie partie par
  un scénario (`starting_peasants` a déjà l'IA en 1).

### Découpage d'IA0.1 (boucles de 15 min au plus, test rouge d'abord sur la vraie chaîne)

| Boucle | Changement | Test rouge d'abord |
|---|---|---|
| E1 | `AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(UWorld&, Config, Starting)` (statique, serveur) : un `AHOLPlayerState` par emplacement `AI`, `SetIsABot`, nom, `SetMatchSlot`, stock de départ ; journal « AI slot N: player state, F … » ; appelée dans `StartPlay` avant `SpawnMainCentres` | `HeroesOfLiberty.AI.Slot.Spawn` (monde de test, `AHOLGameState`) : config humain 0 + IA 1 et 3 + fermé : 2 PlayerStates IA dans `PlayerArray`, `IsABot`, emplacements 1 et 3, couleurs de la config, stock = `StartingResources`, `GetSlotStock` vrai ; rien pour 0 et les fermés ; second appel sans doublon |
| E2 | rien (ou correctifs révélés) | `HeroesOfLiberty.AI.Slot.Economy` (monde d'IA0.2 + PlayerState IA d'E1) : fondation de maison posée par un ordre de l'emplacement IA (`UHOLAIOrderLibrary` / `HOLPlayerOrders` → construction au pas) et payée ; Paysan produit au centre IA (file, coût débité) ; dépôt de 10 bois crédité (`DepositLoad`) ; sans PlayerState (avant E1) : les trois refusés / perdus |
| E3 | IA0.3 : attente « slot 1 no stock » retournée (le monde de `Summary.World` reçoit son PlayerState IA) ; `ReadStock` par `GetSlotStock` (option) | `AI.Summary.World` : emplacement IA avec stock |
| E4 | `waitplayers` ne compte que les humains ; `UpdateStartWait` saute `IsABot` ; rejet de construction sans PlayerState journalisé | test de `HOLTestSequence` si possible, sinon `quick_check -Parts net` inchangé |
| E5 | vérification en vraie partie | `quick_check -Parts tests,victory,production,gather` ; scénario `starting_peasants` hors écran : ligne « AI slot 1: player state », stock de l'IA dans « others [slot 1 F 200 …] » ; `-Parts net` (pas d'IA, rien ne doit bouger) |
| E6 | essai de l'utilisateur ? (rien de visible hors statistiques : écran de fin avec la ligne de l'IA) ; commit après validation | — |

E1 d'abord ; E2-E4 indépendantes ensuite. Pas d'essai visible avant IA0.4 : proposer de commiter IA0.1 sur `quick_check` seul.

### Questions de jeu laissées ouvertes par le code

- **IA0.1-Q1 Nom de l'IA** : aujourd'hui « Joueur N » comme un humain sans nom. A) « Joueur N » inchangé ; B) « IA N » /
  « Ordinateur N » dans la table des joueurs, le bandeau et les statistiques (texte traduit, `PlayerName` vide) ; C) un nom
  tiré d'une liste (seigneurs médiévaux). **Recommandation : B** (on sait contre qui on joue ; C plus tard avec les niveaux).
- **IA0.1-Q2 Stock de départ** : A) le même que l'humain (`StartingResources`, 200, D283), comme le plan ; B) un bonus par
  niveau (IA4). **Recommandation : A** maintenant, B noté pour IA4.
- **IA0.1-Q3 Emplacements IA des scénarios et de `quick_check`** : les 38 `"AI"` existants (cibles passives, souvent
  `startingPeasants` 0) auront un PlayerState et un stock, mais **pas de cerveau** tant qu'IA0.4 n'existe pas. A) tous en
  reçoivent un (rien ne bouge : pas de cerveau) ; B) un champ du `.json` pour couper le futur cerveau (`"aiBrain": false`)
  dès IA0.4. **Recommandation : A maintenant, B à IA0.4** (sinon chaque scénario de combat aura une IA qui joue).
- **IA0.1-Q4 Victoire** : l'IA compte déjà (emplacement non fermé : forum à vies, Régicide, effondrement D358). Rien à
  décider ; **recommandation : la laisser compter** (une IA battue s'effondre comme un humain).
- **IA0.1-Q5 Couleur** : celle de la configuration (comme un humain) ; aujourd'hui les rapports et la pastille de sélection
  prennent l'indice de l'emplacement faute de PlayerState (`FindColorIndex`). **Recommandation : la configuration** (vient
  d'office avec E1).

## IA0.4 Cerveau en script : relevé du code (2026-10-27, Arwen `ai-ia1`, lecture seule)

Chemins sous `Source/HeroesOfLiberty/` sauf mention ; moteur = `C:/UE/UE_5.8_AS_3/Engine/Plugins/Angelscript/Source/AngelscriptCode/` (noté AC).

**1. Script**
- `Script/` d'Arwen : un seul fichier, `Script/Tests/HOLScriptSmoke_Test.as:5-8` (`Test_SeesProjectClasses`, AS1), plus
  `Binds.Cache*` (générés). Conventions : test = `void Test_<Nom>(FUnitTest& T)` dans un module `*_Test` ; le type sert de
  valeur (`AHOLBuilding`, pas `StaticClass()`) ; commentaires en anglais.
- Monde d'un test en script : chaque `Test_` reçoit un monde neuf de type Game (`InitializeStandalone("UnitTestWorld")`,
  `InitializeActorsForPlay`, pas de carte ni de `BeginPlay` : AC `Private/Testing/UnitTest.cpp:275-332`) ; nos sous-systèmes
  y existent, mais **rien d'exposé au script ne fait naître une unité ou une source** : seuls `UHOLAISummaryLibrary` et
  `UHOLAIOrderLibrary` sont `BlueprintCallable` dans tout le projet. Un test en script ne peut donc tester qu'une règle pure
  (résumé → ordres) ; la vraie chaîne se teste en C++ (ci-dessous, 6).
- `quick_check.ps1:216` : `Automation RunTests HeroesOfLiberty+Angelscript.UnitTests` avec `-asdebugport=0
  -as-exit-on-error` ; `:5240-5244` « script tests ran (N) » > 0 ; `:2176-2179` et `:2236` : trois premières
  `Angelscript: Error:` du journal (tests et jeu). `Tools/Scenarios/play.ps1:46` : `-asdebugport=0`, sans `-as-exit-on-error`.
- Naissance d'une classe de script en partie : **une classe de script qui dérive d'un `UWorldSubsystem` C++ est créée
  d'office** pour chaque monde que la classe de base accepte (AC `ClassGenerator/AngelscriptClassGenerator.cpp:2592-2598`,
  sous-système « externe », réactivé au rechargement ; `UScriptWorldSubsystem` = même chose avec `BP_Tick` à l'image,
  `Public/BaseClasses/ScriptWorldSubsystem.h:9-150`). Accès en script : `UMaClasse::Get()` (AC `Binds/Bind_Subsystems.cpp:85-96`).
  Le projet ne dépend pas d'`AngelscriptCode` (aucun `Build.cs`) : le C++ trouve la classe de script par la classe de base
  (`World->GetSubsystemArrayCopy<UBase>()`), sans dépendance. Les mondes des tests C++ (`UWorld::CreateWorld(Game)`)
  reçoivent aussi ce sous-système (les scripts sont compilés au démarrage de l'éditeur des tests).
- Liaisons générées (`Script/Binds.Cache`) : espaces de noms **`HOLAISummary::ReadAISummary(int PlayerIndex)`** et
  **`HOLAIOrder::IssueAIGatherOrder(PlayerIndex, UnitIds, SourceIndex, bQueue)`** (préfixe `U` et suffixe `Library`
  retirés) ; le `WorldContextObject` est rempli par le script appelant (le sous-système lui-même).
- Leçons d'AS1-AS2 (`AngelscriptInProgress.md` « Suivi ») : `-as-exit-on-error` sinon l'éditeur attend devant « corrigez
  les erreurs » ; `-asdebugport=0` sinon l'extension VS Code fige le jeu ; `FUnitTest` existe hors éditeur (tests
  compilés dans le paquet, inertes) ; au rechargement complet une classe devient `<Nom>_REPLACED_N` (ne pas garder de
  `UClass*`) ; une classe `ClientOnly` (module `HeroesOfLibertyUI`) nommée dans un script casserait le serveur.

**2. Pas de simulation**
- 20 pas par seconde (`Config/DefaultGame.ini:12`, `Simulation/HOLSimulationSettings.h:102`) : 5 à 10 pas = 4 à 2
  pensées par seconde. Cadence : `UHOLSimulationSubsystem::OnPrePhysicsPhaseStarted` (`Simulation/HOLSimulationSubsystem.cpp:296-318`,
  rien pendant l'attente des joueurs R3.10), `RunStep` (`:321-400`) : `OnStepStarted(StepIndex)` en tête (« ce qui est
  émis ici est appliqué par ce pas »), `Step hash` si `hol.Simulation.StepHashes` (`:35-36`, `:386`), `++StepIndex`,
  `OnStepEnded` en queue (`:400`). Serveur et solo seulement : `OnWorldBeginPlay` sort sur `NM_Client` (`:203-210`).
- Les deux délégués sont **natifs** (`DECLARE_MULTICAST_DELEGATE`, `HOLSimulationSubsystem.h:47-51`), invisibles du script.
  Modèle à copier : `UHOLProductionSubsystem::OnWorldBeginPlay` (`Buildings/HOLProductionSubsystem.cpp:59-70` : sort sur
  client, `GetOnStepStarted().AddUObject`) ; aussi `HOLConstructionSubsystem.cpp:54`, `Match/HOLGameState.cpp:80-81`.
- Branchement proposé : classe C++ abstraite `UHOLAIBrainSubsystem : UWorldSubsystem` (`AI/HOLAIBrain.h`, la seule part
  C++ : cadence et liste des emplacements) ; `OnWorldBeginPlay` hors client → `GetOnStepEnded().AddUObject` ; à chaque pas,
  pour chaque emplacement IA (PlayerStates `IsABot()` triés par `GetPlayerIndex`) dont `(Step + Slot) % Interval == 0`
  (étalé), appel d'un `UFUNCTION(BlueprintImplementableEvent) Think(int32 PlayerIndex)` redéfini en script ; fonction
  publique `ThinkNow(PlayerIndex)` pour les tests. **`OnStepEnded`** plutôt que `OnStepStarted` : le résumé lit l'état du
  pas fini, l'ordre entre dans la file et s'applique au pas suivant, quel que soit l'ordre des abonnés (la production et
  la construction lisent leurs files dans `OnStepStarted`).
- Même file que l'humain : `HOLAIOrder::IssueAIGatherOrder` → `HOLPlayerOrders::Gather` (`AI/HOLAIOrders.cpp:37-42`) →
  `Commands->IssueGatherOrder` (`Commands/HOLPlayerOrders.cpp:138`), comme les RPC du contrôleur ; refusé sur un client
  (`HOLAIOrders.cpp:13-22`). Déterminisme : le résumé est trié (`IdlePeasants` croissants, sources par distance puis indice,
  `AI/HOLAISummary.cpp:49-53`, `:84-94`) ; le cerveau ne lit ni l'heure ni le hasard (un tirage plus tard : `Seed`,
  `Match/HOLMatchConfig.h:85`) ; pensée au pas, pas à l'image.

**3. Exposition au script**
- `UHOLAISummaryLibrary::ReadAISummary` (`AI/HOLAISummary.h:149-158`) et `UHOLAIOrderLibrary::IssueAIGatherOrder` /
  `IssueAIMoveOrder` (`AI/HOLAIOrders.h:14-30`) : `BlueprintCallable`, liés (vu dans `Binds.Cache`). `FHOLAISummary`,
  `FHOLAIResourceSummary`, `FHOLAISourceNear` : `USTRUCT(BlueprintType)`, champs `UPROPERTY(BlueprintReadOnly)`
  (`HOLAISummary.h:23-97`) : lisibles en script, **pas écrivables** (AC `Binds/Helper_PropertyBind.h:65-72` ; un test en
  script ne peut pas bâtir un résumé ; `meta = (ScriptReadWrite)` l'ouvre au script seul, `:29,51-55`). `Get(EHOLResource)`
  et `GetStock()` ne sont pas des `UFUNCTION` : en script, lire `Food`, `Wood`, `Stone`, `Gold`.
- Manque pour IA0.4 : (a) l'appel au pas (2) ; (b) la liste des emplacements IA (dans la base C++) ; (c) **le résumé garde
  les sources qu'on ne peut pas récolter** : `Make` ne teste pas `FHOLSource::IsGatherableBy` (`HOLAISummary.cpp:58-63` ;
  `Resources/HOLSourceSubsystem.h:31,39`) : un champ de la ferme ennemie près d'un dépôt IA serait choisi, refusé par la
  récolte (`Simulation/HOLGatherProcessor.cpp:384`), le Paysan resterait inactif et réenvoyé à chaque pensée ; (d) un
  interrupteur par emplacement (IA0.1-Q3 B : `"aiBrain": false` des scénarios de combat) ; (e) un journal par décision
  (« AI slot 1 step 40: 3 idle -> gather W #12 ») pour IA0.7. Pas nécessaires à IA0.4 : production et pose dans
  `UHOLAIOrderLibrary` (IA1.4), Paysans par ressource (IA1.3).

**4. Décision « source utile la plus proche »**
- Le résumé donne : Paysans inactifs (comme le compteur du HUD, D307 : un récolteur en aller-retour ou un bâtisseur n'est
  pas inactif, `HOLAISummary.cpp:46-53`), par ressource jusqu'à 8 sources debout reliées à un dépôt fini de l'IA, triées
  par `DepositDistance` (bord du dépôt le plus proche, `:56-94`), le stock (`bHasStock`). Rien sur l'occupation d'une
  source (une ferme n'a qu'une place) ni sur les Paysans déjà par ressource (IA1.3).
- Règle proposée (pure, testable, déterministe) : à chaque pensée, s'il y a des Paysans inactifs : ressource = **la plus
  basse du stock parmi celles qui ont une source** (égalité : nourriture, bois, pierre, or) ; **un seul ordre** : tous les
  inactifs à sa source la plus proche (`Sources[0]`) ; aucune source : rien, journal. Un ordre par pensée : les suivants
  partent aux pensées suivantes, déjà sur la ressource la plus basse après les dépôts. Au départ (200 partout, 5 Paysans) :
  les 5 vont aux baies.

**5. Serveur dédié (IA0.5, constat)**
- `AngelscriptCode` est un module `Runtime` (`C:/UE/UE_5.8_AS_3/Engine/Plugins/Angelscript/Angelscript.uplugin`), chargé par
  la cible Server ; AS2 a vu le serveur empaqueté démarrer sur son cache (« Using fully precompiled scripts »,
  `Tools/Package/precompile_scripts.ps1` lance l'exe `WindowsServer\`) ; sans cache, il compile les `.as` au démarrage
  (AC `AngelscriptManager.cpp:433-460`). La partie `net` de `quick_check` lance `UnrealEditor -server` (`:331`) : le
  script s'y compile depuis les `.as`. Non vérifié : une classe de script qui pense **sur** le serveur (IA0.5).

### Découpage d'IA0.4 (boucles de 15 min au plus, test rouge d'abord sur la vraie chaîne)

| Boucle | Changement | Test rouge d'abord |
|---|---|---|
| B1 | `AI/HOLAIBrain.h/.cpp` : `UHOLAIBrainSubsystem` abstrait (Game/PIE, hors client, `OnStepEnded`, intervalle `hol.AI.ThinkSteps` 5, emplacements `IsABot` triés, `ThinkNow`) ; `Script/AI/HOLEconomyBrain.as` vide qui le dérive | `HeroesOfLiberty.AI.Brain.Exists` (C++) : monde de test → un sous-système de script dérivé trouvé par `GetSubsystemArrayCopy` ; rouge sans le `.as` |
| B2 | `Think` en script : la règle de 4 → `HOLAIOrder::IssueAIGatherOrder` ; journal | `HeroesOfLiberty.AI.Brain.GatherIdle` (C++, monde d'IA0.2 + `SpawnAIPlayerStates` + centre de l'emplacement 1 comme `AI.Summary.World`, arbre et baies) : `ThinkNow(1)` puis `RunStep` → le Paysan IA inactif récolte la source attendue ; Paysan humain intouché ; rien sans Paysan inactif |
| B3 | règle en fonction pure de script (`HOLAIEconomy::ChooseGather(Summary)`) ; champs du résumé `ScriptReadWrite` (si IA0.4-Q2 = A) | `Script/AI/HOLEconomyBrain_Test.as` : stock le plus bas, égalité dans l'ordre F W S G, aucune source, aucun inactif |
| B4 | résumé : sources `IsGatherableBy(PlayerIndex)` seulement | `AI.Summary.*` : champ d'une ferme ennemie exclu, celle de l'IA gardée |
| B5 | cadence au vrai pas (abonnement, étalement par emplacement) ; interrupteur `aiBrain` (IA0.4-Q3) | C++ : N pas simulés par `OnStepEnded` → pensées aux pas attendus, une seule par emplacement et par intervalle, aucune sur un monde client |
| B6 | vraie partie | scénario `starting_peasants` hors écran : 5 Paysans IA récoltent en moins de 1 s, journal de décision ; `quick_check -Parts tests,victory,production,gather` et `-Parts net` (aucune IA : rien ne bouge) ; deux courses de même `Seed` : mêmes `Step hash` ; puis essai de l'utilisateur (scénario prêt) et commit après validation |

B1 d'abord (il ouvre l'inconnue : sous-système de script créé dans un monde de test) ; B3 et B4 indépendantes.
Repli si B1 échoue : délégué `BlueprintAssignable` sur un sous-système C++ auquel un `UScriptWorldSubsystem` s'abonne.

### Questions laissées ouvertes par le code

- **IA0.4-Q1 Ressource visée** : A) la plus basse du stock (égalité F W S G), tous les inactifs d'une pensée à sa source la
  plus proche ; B) la source la plus proche toutes ressources confondues (souvent tout au bois) ; C) répartition fixe
  (moitié nourriture, moitié bois) : demande les Paysans par ressource (IA1.3). **Recommandation : A** (visible, simple,
  remplacée par la règle de besoins d'IA1.6).
- **IA0.4-Q2 Tests du cerveau** : A) un test C++ sur la vraie chaîne (C++ → script → ordre → file → processeurs) plus des tests
  en script de la règle sur des résumés bâtis en script (`ScriptReadWrite`) ; B) C++ seul ; C) test d'intégration en script
  sur une carte (`IntegrationTest_`, PIE serveur + client) : lourd, hors `quick_check`. **Recommandation : A** (D484
  respectée, la règle se teste en script, la chaîne en C++).
- **IA0.4-Q3 Interrupteur des scénarios** (IA0.1-Q3 B) : A) champ `"aiBrain"` par emplacement du `.json` (défaut vrai, faux
  dans les scénarios de combat et `quick_check`) ; B) variable `hol.AI.Brain 0/1` pour toute la partie ; C) aucun : les
  emplacements IA des scénarios jouent. **Recommandation : A** (une IA qui joue et une cible passive dans le même scénario).
- **IA0.4-Q4 Fréquence** : A) 5 pas (4 par seconde), étalée par emplacement ; B) 10 pas (2 par seconde) ; C) réglage par niveau
  dès maintenant. **Recommandation : A**, variable `hol.AI.ThinkSteps`, réglage par niveau à IA4 (D422, « réaction »).

## IA0.5 Serveur dédié : relevé du code (2026-10-28, Arwen `ai-ia1` = `main` 40108a6, lecture seule)

**Constat** : rien ne bloque le cerveau sur un serveur dédié ; jamais vu tourner (tests en monde serveur sans client
seulement ; `NM_DedicatedServer` est global au processus : il faut l'éditeur en `-server` ou l'exe serveur).
- Création : `HOLAIBrain.cpp:36-49` (`ShouldCreateSubsystem`, `DoesSupportWorldType` Game/PIE), `:56` et `:125` ne refusent
  que `NM_Client`. Classe de script dérivée de `UWorldSubsystem` activée par `ActivateExternalSubsystem`
  (`AngelscriptClassGenerator.cpp:2437-2453, 2592-2596`, hors `WITH_EDITOR`, après `OnPostEngineInit`). Plugin Angelscript
  `EnabledByDefault`, modules `Runtime` ; UHT de la cible Server présent.
- Cache précompilé : lu par défaut (`AngelscriptManager.cpp:360-362, 434-460`, identifiant de build seul vérifié,
  `PrecompiledData.cpp:2609`), journal « Using fully precompiled scripts » ; sinon `.as` compilés au démarrage.
  `precompile_scripts.ps1` produit déjà le cache de `WindowsServer`. `Script/` copié dans tout build cuisiné
  (`BaseGame.ini:142`). **Jamais vu** : une classe de script activée en mode précompilé (AS2 n'a vu que le démarrage).
- Le serveur cuisiné d'Arwen (`Saved/StagedBuilds/WindowsServer`, 2026-10-25) date d'avant AS1 : pas de `Script/` ;
  le refaire = `net_match.ps1 -Stage` (verrou d'UAT). Dernier paquet (2026-10-27-e36efe9) d'avant IA0.4 : rouge naturel.
- Le cerveau n'écrit aucune ligne à sa création : rien à lire dans un journal de serveur.
- Partie réseau : `-MatchConfig=<json>` ; `"state": "AI"` accepté, `SpawnAIPlayerStates` (`GameModeBase.cpp:584-628`) et
  Paysans de départ (`:565-569`, nombre global `FHOLStartRules::GetNumStartingPeasants`, pas par emplacement) côté serveur ;
  un client ne prend qu'un `Human` libre (`HOLMatchConfig.cpp:86`) ; partie figée jusqu'à l'arrivée des joueurs nommés
  (`:495-501`). Stock de l'IA `COND_OwnerOnly`. Salon/gestionnaire C# : pas d'état IA (`Lobby.cs:93`, `MatchConfig.cs:84-87`,
  `LobbyStore.cs:232`) → IA0.6.
- Tests : partie `net` de `quick_check` (éditeur `-server`, Alice et Bob, `-as-exit-on-error`, `$NoStartingPeasants`
  `:217` met 0 partout) ; `net_match.ps1:82-85` n'écrit que Human/Closed ; côté client `hol.Units.Where <slot>`
  (`HOLUnitView.cpp:79-110`) montre activité et « gathers X ». `packaged_smoke.ps1:27` écarte `WindowsServer`.

### Découpage d'IA0.5 (proposé)

| Boucle | Changement | Test rouge d'abord |
|---|---|---|
| B1 | ligne de création « AI brain <classe> awake, net mode …, thinking slots [..] » dans `OnWorldBeginPlay` ; partie `aiserver` de `quick_check` : éditeur `-server` sans client, slot 0 Human sans nom, slot 1 AI, `"startingPeasants": 5`, attentes de `starting_peasants` | « awake, net mode DedicatedServer » (absente), puis « AI slot 1 step », « 5 idle -> gather », « Gather order of slot 1: 5 units », `hol.Gather.Report 1` |
| B2 | partie `netai` : serveur de B1 + Alice et Bob, IA dans un 3e emplacement (place de départ à vérifier sur Plain) | sur chaque client, `hol.Units.Where <IA>` : 5 Paysans en route ou en récolte ; serveur : lignes de B1, aucun Human commandé |
| B3 | passage serveur dans `packaged_smoke.ps1` (exe serveur seul, ~20 s) | rouge sur 2026-10-27-e36efe9 ; vert après **un** paquet complet (machine calme, sessions prévenues) : « Using fully precompiled scripts », « awake… DedicatedServer », « -> gather » |
| B3b (facultative) | `net_match.ps1 -AISlot <n>` + nouvelle cuisson du serveur | « AI slot n step » au journal de `net_server` |

Replis : classe de script non activée en précompilé (B3 rouge, B1 vert) → sous-système C++ concret ou délégué
`BlueprintAssignable` (repli d'IA0.4 B1) ; script absent côté Server → `ChooseGather` en C++ (~40 lignes, IA-Q2).

### Questions

- IA0.5-Q1 test à deux clients : A) dans `net` ; B) partie `netai` séparée ; C) `net_match.ps1` à la main. Reco B.
- IA0.5-Q2 Paysans de l'IA : A) `startingPeasants` par emplacement ; B) `"startingPeasants": 5` global dans `aiserver`/`netai` ;
  C) triche. Reco B.
- IA0.5-Q3 paquet : A) passage serveur dans `packaged_smoke` ; B) nouveau script ; C) une fois à la main. Reco A.
- IA0.5-Q4 nouvelle cuisson du serveur de `net_match` : A) maintenant ; B) pas pour IA0.5 (une seule prise du verrou
  d'UAT : le paquet) ; C) jamais. Reco B.
- IA0.5-Q5 emplacement IA dans le salon : A) à IA0.6 ; B) dès IA0.5 ; C) après le prototype. Reco A.
- IA0.5-Q6 essai de l'utilisateur : A) aucun, tout automatique ; B) scénario `net_ai` (client contre l'IA sur serveur
  dédié, demande B3b) ; C) paquet avec un ami. Reco A.

## IA0.6 Menu : relevé du code (2026-10-28, Arwen `ai-ia1` = 0758f87, lecture seule)

NG = `Source/HeroesOfLibertyUI/Screens/HOLNewGameScreen.cpp`, QC = `Tools/Tests/quick_check.ps1`.
- **Solo** : listes `SlotState1..7` (Fermé / Humain / IA, ordre d'`EHOLSlotState`, NG:45-48), emplacement 0 = « Vous » ;
  `IA` grisée par `SetOptionEnabled(AI, false)` (NG:310-311, D240) ; `HandleStateChosen` (NG:938-945) et `Refresh`
  (NG:674, 709-717) traitent déjà une ligne IA comme occupée ; tailles de carte comptent `!= Closed`
  (`HOLMapSizeRules.cpp:223-244`). `Difficulty` par emplacement existe (Normal), nom par `HOLForumMessages::GetAIName`.
  `Launch` (NG:995-1037) passe l'IA telle quelle → `SpawnAIPlayerStates`, cerveau, 5 Paysans. Note « Grisé : pas encore dans
  le jeu (IA, …) » NG:279-280 et `fill_po.py:72`. Tests : partie `menu` (QC:388-426, `hol.UI.Choose`, `hol.UI.Shown`,
  marques QC:2809-2848) ; rouge naturel : `UHOLDropdown::Choose` ignore une option grisée (`HOLDropdown.cpp:166-175`) ;
  `smoke` lance par `hol.UI.NewGame.Launch` ; aucun test C++ de l'écran.
- **Salon** : même écran (`bLobby`). Jeu : `EHOLLobbySlotState { Open, Closed, Player }` (`HOLManagerClientRules.h:60`),
  `ReadSlotState` refuse le reste (`.cpp:218-224`), `MakeSlotBody` open/closed (`.cpp:209-214`), `CanToggleSlot`
  (`:137-141`), `CanStart` ≥ 2 joueurs et tous prêts (`:143-147`) ; écran : `LobbyStateNames` (NG:100-103), `ShowLobby`
  (NG:500-551), ligne de journal « slots o x c3r » (NG:585-600), `HandleStateChosen` salon `Value <= 1` (NG:915-928).
  Gestionnaire C# : `SlotState`/`LobbySlot` (`Lobby.cs:58-94`), `SetSlot` + `PUT /lobbies/{id}/slots/{index}`
  (`LobbyStore.cs:224-251`, `LobbyRoutes.cs:106-113`), `color_taken` (`:156`), changement de taille (`:180-214`, une IA
  au-delà des places serait fermée), vue (`LobbyViews.cs:75-77`), `Start` compte les humains (`LobbyStore.cs:257-279`,
  `LobbyRules.cs:51` = 2), `MatchConfig.FromLobby` Human/Closed seulement (`MatchConfig.cs:46-94`) ; `match.json` annonce
  déjà `"AI"` (`PROTOCOL.md:441, 445`), le serveur le lit (B2). Tests : xUnit `Manager/HOLManager.Tests/`
  (`MatchStartTests.ReferenceLobbyGivesTheReferenceFile`, référence `Data/match_v3_reference.json` relue par le C++
  `HOLMapSizeLobbyTests.cpp`), `build_manager.ps1 -Test` ; `manager_client.ps1 -Case Leader` ; `lobby_match.ps1`
  (exige le serveur cuisiné, verrou d'UAT).

### Découpage d'IA0.6 (proposé)

| Boucle | Changement | Test rouge d'abord |
|---|---|---|
| S1 | IA ouverte en solo (NG:310-311, commentaire NG:938, note NG:279, `fill_po.py:72`) | `menu ai` : `hol.UI.Choose SlotState1 2` → « value 2 chosen », couleur et équipe montrées (rouge : option grisée ignorée) |
| S2 | niveau (selon Q1) | `hol.UI.Shown SlotLevel1` sur une ligne IA, absent sur une ligne fermée |
| S3 | du menu à la partie | IA choisie à la liste, `hol.UI.NewGame.Launch -` → « thinking slots [1] », « idle -> gather » |
| L1 | gestionnaire : état IA (`PUT slots {"state":"ai"}`, couleur libre, places, Prêt remis à zéro, vue, taille, PROTOCOL.md) | xUnit : IA dans la vue, couleur prise, descend à Petite |
| L2 | gestionnaire : lancement (`FromLobby` `"AI"`, `difficulty`, couleurs aléatoires, règle de départ Q3) | référence `match_v3_ai_reference.json` (xUnit + `HOLMapSizeLobbyTests.cpp`) |
| L3 | client C++ : `EHOLLobbySlotState::AI`, lecture, `MakeSlotBody("ai")`, `CanStart` | `HOLManagerClientTests` : salon avec `"ai"` lu, départ chef + IA |
| L4 | écran du salon : choix « IA » du chef, nom « IA Normal N », couleur/équipe (Q4), « a » dans la ligne `slots` | `manager_client.ps1 -Case AI` |
| L5 | bout en bout après recuisson du serveur | `lobby_match.ps1 -AI` : « thinking slots [n] », clients voient l'IA (machine calme) |
| C | clôture : GDD, scénario `Tools/Scenarios/ai_menu.txt`, essai | `-Parts menu,smoke,aiserver` |

### Questions

- IA0.6-Q1 niveau au menu : A) liste « Niveau » grisée sur Normal sur les lignes IA ; B) dans la liste d'état (« IA
  Facile / Normale / Difficile / Évolutive », AoE4) ; C) caché jusqu'à IA4. Reco A (C si trop large).
- IA0.6-Q2 partie solo sans humain : A) non ; B) emplacement 0 en IA (spectateur) ; C) plus tard. Reco A.
- IA0.6-Q3 « au moins 2 joueurs » au salon : A) une IA compte ; B) 2 humains ; C) seulement en salon d'essai. Reco A.
- IA0.6-Q4 qui règle l'IA au salon : A) le chef (couleur, équipe ; niveau grisé) ; B) tout automatique ; C) couleur seule.
  Reco A.
- IA0.6-Q5 nom de l'IA au salon : A) « IA Normal N » ; B) « IA » ; C) « Ordinateur N ». Reco A.
- IA0.6-Q6 ajouter/retirer une IA remet les Prêt à zéro : A) oui ; B) non. Reco A.
- IA0.6-Q7 « Humain » sur les emplacements 1-7 en solo : A) gardé ; B) retiré ; C) grisé. Reco A.

## Suivi

- 2026-10-26 : chantier ouvert (choix de l'utilisateur parmi les tâches faisables sans Unreal). Lecture du GDD faite ;
  relevé du code lancé en lecture seule (emplacements IA, chemin des ordres, systèmes existants, J8 et suite).
- 2026-10-26 : relevé du code revenu (ci-dessus) ; découpage proposé ; IA-Q1 posée.
- 2026-10-26 : IA-Q1 = A ; IA-Q2 posée.
- 2026-10-26 : IA-Q2 = B ; question sur le coût du GOAP (réponse ci-dessus) ; IA-Q3 suivante.
- 2026-10-26 : IA-Q3 = A ; IA-Q4 posée.
- 2026-10-26 : IA-Q4 = A ; découpage tranché ; plan d'IA0 écrit ; prochain pas : IA0.1 quand le moteur est recompilé.
- 2026-10-26 : AS-Q1 / AS-Q2 tranchées par une autre session (D484) ; seul le moteur bloque IA0.
- 2026-10-26 : **IA0.3 lancée sans attendre le moteur** (« go ») : IA0.1 à IA0.3 sont du C++ de jeu testé dans l'éditeur ;
  seuls IA0.4 (script) et IA0.5 (cible Server) attendent la recompilation. Le Hazelight actuel compile le projet (UHT
  supprimé). Ordre : IA0.3 (isolé), IA0.2, puis IA0.1 après la fusion de R7 (`GetSlotStock` / `AddSlotStock` sur
  `net-playtest`, même zone). Arwen, branche `ai-ia0` = `main` fc6ec1c, `IN_USE.txt` posé, UHT supprimé ; sous-agent.

- 2026-10-26 : IA0.3 : 5 tests `HeroesOfLiberty.AI.Summary.*` écrits (IdlePeasants, Sources, NoDeposit, Stock, World sur un vrai monde de test) ; compilés, **rouges** sur le bouchon (résumé vide), le monde de test se monte bien (unités, centres, sources).
- 2026-10-26 : IA0.3 : résumé codé (`AI/HOLAISummary.h/.cpp` : règle pure `HOLAISummary::Make`, couche `Read` / `ReadAISummary`, stock lu à part par `ReadStock`) ; 5/5 verts, deux fois de suite.
- 2026-10-26 : IA0.3 : `quick_check -Parts tests` 181/181 dans Arwen (branche `ai-ia0`, rien commité) ; IA0.3 attend la validation.
- 2026-10-26 : **IA0.3 commitée** (accord de l'utilisateur) : c301e01 sur `ai-ia0`, `main` avancée (ff). **IA0.2 lancée**
  (Arwen, même branche, sous-agent) : ordres sans contrôleur (`SendNonWorkersTo`, plafond `MaxOrderedUnits`).
- 2026-10-26 : IA0.2 : relevé du contrôleur (côté serveur, avant `IssueXxx`) : emplacement lu sur le PlayerState ; plafond
  `MaxOrderedUnits` (200 : ordre entier refusé au-delà ; pose de fondation : posée sans bâtisseurs) ; `FPlayerOrderScope`
  (chaînes, D328) ; `SendNonWorkersTo` (soldats d'un ordre de récolte ou de construction envoyés marcher au point).
  Côté client seulement (restent) : sélection et groupes tronqués à 200, unités à soi, lecture du clic. Le reste (D288,
  D358, cible) est déjà dans la file et les processeurs. Faille vue : un spectateur (PlayerState à `INDEX_NONE`) qui
  enverrait un RPC donnerait un ordre « du serveur » obéi par tous ; la règle commune exige un emplacement 0-7.
  Règle commune : `Commands/HOLPlayerOrders.h/.cpp` ; bibliothèque du script : `AI/HOLAIOrders.h/.cpp`.
- 2026-10-26 : IA0.2 : 4 tests `HeroesOfLiberty.AI.Orders.*` écrits (Rule ; Gather, Cap, NonWorkers sur un vrai monde de test,
  emplacement 1 sans PlayerState, processeurs de récolte et d'ordres lancés au pas) ; compilés, **rouges** sur le bouchon,
  le monde se monte (201 Paysans nés).
- 2026-10-26 : IA0.2 : codé : règle `HOLPlayerOrders` (emplacement 0-7, plafond 200, chaînes, `SendNonWorkersTo` ; Move, Attack,
  AttackBuilding, AttackMove, Stop, Gather, Build, Place), les 9 RPC d'ordres du contrôleur l'appellent (une ligne chacun) ;
  script : `UHOLAIOrderLibrary::IssueAIGatherOrder` / `IssueAIMoveOrder` (refusés sur un client). 4/4 verts, deux fois de suite
  (pièges du test : ordre des processeurs du pas = ordres puis récolte ; messages attendus en regex, parenthèses échappées).
- 2026-10-26 : IA0.2 : `quick_check -Parts tests` 185/185 dans Arwen (181 + 4), rien commité ; IA0.2 attend la validation.
  À faire avant le commit : parties `gather`, `build`, `move` de `quick_check` (chemin humain des RPC, en vraie partie).
- 2026-10-26 : IA0.2 relue par la session principale (diff du contrôleur : 9 RPC d'ordres en une ligne, rien d'autre) ;
  `quick_check -Parts gather,build,move,production` **PASS** en 412 s dans Arwen (chemin humain rejoué en vraie partie).
  Attente : accord de commit et validation du refus des ordres sans emplacement (spectateur).
- 2026-10-26 (soir) : **plan d'IA1 (économie) ouvert sur papier** pendant la recompilation complète du moteur (choix de
  l'utilisateur, « go ») : lecture du GDD (économie) et relevé du code de l'économie lancés ; puis questions IA1-Q*, une à la fois.
- 2026-10-26 (soir) : relevé du code de l'économie revenu (section « IA1 Économie : relevé du code ») ; IA1-Q1 posée.
- 2026-10-26 (soir) : IA1-Q1 = A (ferme d'abord) ; IA1-Q2 posée (valeurs de la ferme).
- 2026-10-26 (soir) : IA1-Q2 = ferme 2 × 2 (8 autour d'un moulin), 75 bois, 15 s, 0,55 /s, traversable.
- 2026-10-26 (soir) : IA1-Q3 = 5 Paysans de départ ; IA1-Q4 posée (répartition des Paysans).
- 2026-10-26 (soir) : IA1-Q4 = A (ouverture puis besoins ; parties à 8 IA notées pour IA4) ; IA1-Q5 posée (placement).
- 2026-10-26 (soir) : IA1-Q5 = A (règles par type) ; **IA1 tranchée** ; plan d'IA1 écrit (IA1.1 à IA1.9). Prochain pas : IA1.1 et IA1.2 dès que le moteur est recompilé (en parallèle d'IA0), le reste après IA0.
- 2026-10-27 : **moteur revenu** (Hazelight 5.8.4 avec cible Server, toutes les copies recompilées). Reprise (« continue ») :
  **IA0.4** dans Arwen (`ai-ia0` avancée à `main` 85508be), boucle 1 = premier fichier Angelscript du projet (test rouge
  exprès compté par `quick_check -Parts tests`) ; **IA1.1** en parallèle dans Elrond (branche `ai-ia1` = `main` 85508be),
  boucle 1 = relevé, test rouge, 5 Paysans de départ. IA0.1 attend toujours R7 dans `main`. Deux sous-agents.
- 2026-10-27 : IA0.4 boucle 1 : premier fichier Angelscript du projet, `Script/AI/AIBrain_Test.as` dans Arwen (un test vert trivial, `Test_RedOnPurpose` rouge exprès) ; filtre de la partie `tests` de `quick_check` = `HeroesOfLiberty+Angelscript.UnitTests`. Lancement de `quick_check -Parts tests`, attendu FAIL qui nomme `Test_RedOnPurpose`.
- 2026-10-27 : IA1.1 boucle 1 : **arrêtée avant le code** : à 10:28, une autre session (Angelscript AS1) a pris Elrond
  (branche `angelscript-first`, IN_USE.txt) ; rien écrit ni compilé dans Elrond. Relevé fait en lecture (sur `main` 85508be) :
  centre et Roi naissent dans `AHeroesOfLibertyGameModeBase::SpawnMainCentres` (au `StartPlay`, seulement si la carte a des
  `AHOLStartSlot`) ; le Roi par `SpawnKing` = `UHOLUnitSubsystem::RequestUnits` à `FHOLStartRules::GetKingLocation` (côté -X,
  bord du corps + 450 cm) : les Paysans de départ suivront le même chemin (valeur dans `UHOLMatchSettings`, position à côté
  de `GetKingLocation`, règle pure testable dans `FHOLStartRules`). **Aucun test C++** ne passe par `StartPlay` (pas de
  GameMode ni de carte dans `Source/.../Tests`) : ils ne cassent pas. **Cassent** : presque tous les scénarios
  (`Tools/Scenarios/*.txt`, 43 sur 44 ont `waitunits N` = « au moins N » qui passerait trop tôt, et des
  `waitlog Selection: 5 units (own): Peasant x5` / `hol.Select.Own` qui prendraient les 5 de départ) et les parties de
  `quick_check` jouées en vraie partie sur Plain (gather, build, production, move, victory, net…, qui comptent les
  Paysans, les oisifs, la population). Question pour la boucle 2 : un interrupteur pour les essais (champ du `.json`
  de scénario ou cvar, 0 Paysan de départ) plutôt que de relire 44 scénarios ; puis test rouge, code, `quick_check -Full`.
- 2026-10-27 : IA0.4 boucle 1, résultat : `quick_check -Parts tests` **FAIL** comme attendu, « tests 186/187 », échec nommé `Angelscript.UnitTests.AI.AIBrain_Test.Test_RedOnPurpose` ; le test vert compté ; Started = Completed sans retouche de la regex ; le script compile au démarrage sans fenêtre (`-nullrhi`), 54 s. Test rouge retiré, relance attendue PASS 186/186.
- 2026-10-27 : **collision** : une autre session (« Angelscript AS1 », `AngelscriptInProgress.md`) a pris Elrond à 10:28
  (branche `angelscript-first`) et y fait le même premier fichier Angelscript (même ligne `+Angelscript.UnitTests` dans
  `quick_check`). Agent IA0.4 arrêté après le rouge vu (relance verte non faite) ; Arwen garde `Script/AI/AIBrain_Test.as`
  (test vert seul) et la même ligne de `quick_check`, non commités : la fusion avec `angelscript-first` sera sans conflit.
  IA1.1 sans copie libre (Elrond pris, Galadriel / Haldir / Glorfindel occupés) ; question de l'interrupteur des essais posée.
- 2026-10-27 : choix de l'utilisateur : **essais à 0 Paysan de départ** (réglage `StartingPeasants` : 5 en vraie partie, menu
  et salon ; 0 par défaut dans les scénarios et `quick_check`, qui font naître leurs Paysans ; un champ du `.json` de scénario
  remet 5 : test de la règle, essais de l'IA ; aucun scénario existant à relire). **IA1.1 dans Arwen** (branche `ai-ia1`,
  restes d'IA0.4 mis de côté dans le stash « IA0.4 loop 1 ») ; IA0.4 boucle 2 (cerveau) après IA1.1 ou la fusion d'AS1.

- 2026-10-27 : IA1.1 boucle 2 : réglage `StartingPeasants` = 5 (`UHOLMatchSettings`, `DefaultGame.ini`) ; champ
  `StartingPeasants` de `FHOLMatchConfig` (-1 = le réglage : menu, salon ; lu dans le `.json`, `"startingPeasants": 5`) ;
  `hol.Scenario` passe par `HOLTestHooks::MakeScenarioConfig` qui met 0 si le `.json` ne le donne pas ; `quick_check` lancera
  son jeu et son serveur avec `-ini:Game:[/Script/HeroesOfLiberty.HOLMatchSettings]:StartingPeasants=0` ; règle pure
  `FHOLStartRules::GetNumStartingPeasants` / `GetStartingPeasantLocations` (rangée devant le centre, -X, entre le corps et
  le Roi) ; naissance dans `SpawnMainCentres` par `RequestUnits`. Tests rouges `HeroesOfLiberty.Match.StartingPeasants.Locations`
  et `.Count` (bouchons) : compilation et rouge en cours.
- 2026-10-27 : IA1.1 boucle 2, résultat : rouge vu (bouchons : 0 place, 0 Paysan), puis **vert deux fois**
  (`HeroesOfLiberty.Match.StartingPeasants.Locations` : 5 places distinctes devant le centre, hors du corps, à 2,5 m du Roi,
  pour 8 centres autour de la carte ; `.Count` : réglage 5, menu et salon 5, les 32 `.json` de scénario et le scénario
  sans `.json` 0, un `.json` avec `"startingPeasants": 5` ou 3 le rend). `quick_check -Parts tests` **PASS 188/188** (37 s).
  Non commité (Arwen, `ai-ia1`). Reste (boucle 3) : un scénario à 5 Paysans pour voir la règle en vraie partie sur Plain
  (places libres d'arbres, sélection), et `quick_check -Full` pour vérifier que l'option `-ini` remet bien 0 dans ses parties.
- 2026-10-27 : IA1.1 boucle 2 arrêtée au minuteur (15 min), travail fini et journal écrit juste avant ; diff relue par la
  session principale. Vu : `building_crowd.ps1`, `crossings.ps1` et `packaged_smoke.ps1` écrivent un `.json` passé par
  `-MatchConfig` (pas par `hol.Scenario`) : ils auraient 5 Paysans → leur mettre `"startingPeasants": 0` (boucle 3) ;
  `net_match.ps1` et `lobby_match.ps1` = vraies parties, 5 voulus. `quick_check -Full` lancé dans Arwen par la session principale.
- 2026-10-27 : IA1.1 : `quick_check -Full` dans Arwen : **toutes les parties solo PASS** (tests 188/188, sequence, victory,
  sides, menu, camera, production, move, sources, gather, build…, 201 s) ; partie `net` FAIL en bloc : son serveur a trouvé
  le port 7777 pris (par une autre session) et écouté sur 7778, les clients sont allés ailleurs. Sans rapport avec IA1.1 ;
  `-Parts net` relancé seul, port libre.
- 2026-10-27 : IA1.1 : `-Parts net` relancé : FAIL encore, cette fois parce que le client 1 a mis plus de 2 min à charger
  ses modules (processeur à 79 % : autres sessions et un autre projet Unreal ouverts) ; à relancer au calme avant le commit.
  `"startingPeasants": 0` ajouté au `.json` de `building_crowd.ps1`, `crossings.ps1`, `packaged_smoke.ps1` (session
  principale). Boucle 3 lancée (sous-agent) : scénario d'essai `starting_peasants` (humain contre une IA, 5 Paysans) et sa
  répétition hors écran.
- 2026-10-27 : IA1.1 boucle 3 : scénario d'essai `starting_peasants` (Plain, humain en 0 contre une IA en 1, Régicide coché pour voir le Roi, `"startingPeasants": 5`) écrit dans Arwen ; il ne fait naître aucun Paysan, vérifie les 5 de chaque camp (journal, nombre d'unités) puis rend la main, caméra sur la base. Répétition hors écran (`-RenderOffScreen`, `hol.Scenario.AutoContinue 1`) en cours.
- 2026-10-27 : IA1.1 boucle 3 : `starting_peasants` répété hors écran, **vert du premier coup** (2 passages, prêt en 29-71 s de démarrage, étapes en 4-6 s) : lignes « Starting Peasants of slot 0/1: 5 », 12 unités (2 Rois + 10 Paysans, tous Idle, rangée à 250 cm du Roi, 180 cm d'écart), double clic = « 5 units (own): Peasant x5 » et « (other): Peasant x5 », rien de sélectionné en rendant la main ; captures : rangée nette devant chaque forum, Roi derrière, population 5/10, Inactifs 5. `placement` (sans le champ) : aucune ligne « Starting Peasants », 4 unités (les siennes). **Casse** : le test `HeroesOfLiberty.Match.StartingPeasants.Count` échoue (« starting_peasants.json: no starting Peasant » attendu 0, vaut 5) : il exige 0 de tout `.json` de `Tools/Scenarios` ; à corriger dans le test (sauter les `.json` qui donnent `startingPeasants`), code non touché par la boucle 3.
- 2026-10-27 : IA1.1 : test `.Count` corrigé par la session principale (un `.json` qui demande `startingPeasants` a son
  nombre, `starting_peasants.json` doit donner 5 ; les autres 0) ; recompilé, `quick_check -Parts tests` **PASS 188/188** ;
  `-Parts net` relancé au calme (processeur à 55 %) : **PASS** (137 s). Toutes les parties de `-Full` donc vertes.
  **Attente : essai de l'utilisateur** (scénario `starting_peasants`), puis accord de commit.
- 2026-10-27 : **essai de l'utilisateur : « works »** (scénario `starting_peasants`). IA1.1 commitée dans Arwen (acd9e1e),
  `main` (806ae43, premier fichier Angelscript d'AS1) fusionnée (f636eaa ; conflit `quick_check` : `-as-exit-on-error` d'AS1
  et `$NoStartingPeasants` gardés tous deux) ; recompilé, `quick_check -Parts tests,smoke,gather` **PASS** (189/189 dont
  le test en script d'AS1). Avance de `main` dans la copie principale : attend l'« ok » de la session Héros (J8a.1, compile
  dans la copie principale) ; Angelscript et NetPlaytest d'accord. Le rouge `net` de 10:51 venait du serveur de NetPlaytest
  sur 7777 (confirmé par elle). Côté de la rangée de Paysans (toujours -X du monde, comme le Roi) : laissé tel quel, à revoir
  si l'essai contre l'IA le demande.
- 2026-10-27 : **IA1.2 ferme lancée** (« go »), Arwen `ai-ia1` = `main` 60230cd. Boucle 1 : relevé du code en lecture seule
  (bâtiments, sources, récolte, navmesh, grille du Paysan, noms) et découpage en boucles de 15 min.
- 2026-10-27 : IA1.2 boucle 1 faite (relevé en lecture seule, section « IA1.2 Ferme : relevé du code ») : pas de source
  inépuisable, ni de places, ni de débit par source, ni de propriétaire ; traversable = un drapeau lu par
  `AHOLBuilding::BlocksNavigation()` ; rangée économie du palier I pleine. Découpage F1-F11 ; questions IA1.2-Q1 à Q6 à
  poser (case de la ferme d'abord).
- 2026-10-27 : choix de l'utilisateur : **IA1.2-Q1 = A** (ferme au palier I, rangée 3, touche `W` ; exception à « une rangée
  par catégorie ») ; **IA1.2-Q2 à Q6 = recommandations** (détruite → décombres, son Paysan va à une ferme voisine ou inactif ;
  toute unité marche et s'arrête dessus, autres bâtiments refusés ; ferme prise → ferme libre la plus proche du massif, sinon
  attente ; comptée dans les bâtiments perdus ; bâtisseurs d'un moulin : baies d'abord, puis ferme libre). Boucle F1 lancée.
- 2026-10-27 : IA1.2 F1 : colonnes `Places`, `RateScale`, `Endless`, `Linked` de `FHOLSourceTypeRow` (défauts 0, 1, faux,
  faux : rien ne change pour les sources existantes), fonction `GatherRate(UnitRate)` de la ligne, ligne `FarmField` de
  `SourceTypes.csv` (Food, rayon 100, 1 place, `RateScale` 0.7857143 : 0,7 × 0.7857143 = 0,55 exactement en float) ;
  test `Gather.Tables` étendu ; compilation, rouge attendu (table pas encore régénérée), puis `make_source_types.py`.
- 2026-10-27 : IA1.2 F1 (résultat partiel, arrêt à 15 min) : colonnes, `GatherRate`, nom `HOLSourceTypeNames::FarmField`,
  CSV et test faits dans Arwen ; compilation PASS (301 s) ; **rouge vu** : `Gather.Tables` échoue sur « FarmField in the
  source type table » (table pas régénérée), les 4 sources existantes passent leurs nouvelles attentes (défauts). Le lancement
  d'un test seul a pris 4 min 45 (premier lancement après compilation). Reste : `make_source_types.py`, vert deux fois,
  `quick_check -Parts tests`.
- 2026-10-27 : IA1.2 F1 finie par la session principale : table régénérée (`make_source_types.py`, « rows: BerryBush,
  FarmField, GoldVein, StoneVein, Tree », `Content/Resources/SourceTypes.uasset` modifié) ; `quick_check -Parts tests`
  **PASS 189/189** deux fois (`Gather.Tables` vert). Non commité. Leçon : premier lancement d'un test après compilation
  ≈ 5 min (chargement) ; une boucle qui compile ne doit pas aussi régénérer une table et tester → F2+ : le sous-agent code,
  compile et constate le rouge ; la session principale lance le vert et `quick_check`.
- 2026-10-27 : IA1.2 F2 (sous-agent, Arwen) : `UHOLSourceSubsystem` : `Take` ne décompte pas une source `Endless` ;
  source `Linked` sans forme ni trou de navmesh (toujours envoyée par chunk : les ids restent les mêmes partout) et jamais
  placée par la carte ; `FHOLSource::OwnerSlot` (255 = à personne) passé à `AddSource` ; `RemoveSource(Id)` = la met à
  terre comme une source épuisée (reste 0, sautée partout, ids inchangés). Test rouge d'abord `Gather.LinkedSource` sur le
  vrai sous-système en monde de test (bouchons), puis code.
- 2026-10-27 : IA1.2 F2 : sous-agent arrêté au minuteur (test `Gather.LinkedSource` et bouchons écrits, compilation restée
  en file) ; logique écrite par la session principale (`Take` d'une source `Endless` rend tout sans décompter ; `OwnerSlot`
  gardé ; `IsLinked` : ni `ShowSource` ni `HideSource` ; carte qui pose un type lié : ignoré avec avertissement ;
  `RemoveSource` = reste 0, chunk mis à 0, comme épuisée ; rapport « FarmField 1: endless »). **Rouge vu** sur les bouchons
  (11 échecs attendus : la carte pose le champ, propriétaire 255, champ épuisé, pas de retrait…), logique remise.
- 2026-10-27 : IA1.2 F2 **verte** : `quick_check -Parts tests,gather,sources` PASS (190/190, sources, gather, footprints),
  `-Parts tests` PASS une deuxième fois. Non commitée. **Nouveau partage** (F1 et F2 arrêtées au minuteur, compilation
  ~5 min + premier test ~5 min) : le sous-agent écrit le test, les bouchons dans l'arbre et la logique dans un patch à part,
  **sans compiler** ; la session principale compile les bouchons (rouge), applique le patch, compile (vert), `quick_check`.
  F3 lancée ainsi.
- 2026-10-27 : IA1.2 F3 : sous-agent arrêté au minuteur, sans journal ni patch : test `Gather.FarmField` écrit (4 cas :
  débit 0,55, une place, ferme d'un autre refusée, baies à 0,7) ; il appliquait la logique d'un coup par script
  (`f3_edit.py`), arrêté à la 2e retouche (indentation). Reprise par la session principale : bouchons `HasFreePlace` /
  `ClaimPlace` ajoutés, compilation + rouge en cours ; reste de la logique prêt (`f3_rest.py`, 7 retouches vérifiées).
  Leçon : le test de F3 (~400 lignes, monde complet) prend à lui seul une boucle → **un sous-agent pour le test, un autre
  pour la logique**, et la logique livrée en patch vérifié par `git apply --check`.
- 2026-10-27 : IA1.2 F3 **verte** : rouge vu sur les bouchons (deux Paysans sur une ferme, débit 1 au lieu de 5,5, envoi
  vers la ferme d'un autre), logique appliquée (`f3_rest.py`) : débit `GatherRate` du type, ferme jamais « épuisée »,
  places tenues (`PlaceHolders`, lâchées quand le Paysan n'a plus cette source), ferme d'un autre refusée à l'ordre et sautée
  dans le massif ; tous les `Gather.*` verts ; `quick_check -Parts tests,gather,sources,build` **PASS** (191/191). Non commitée.
  Prochain : F4 (colonnes `Walkable` et `Source` des bâtiments, ligne `Farm`, case W) puis F5 (traversable).
- 2026-10-27 : IA1.2 F4 (sous-agent, Arwen, écrit sans compiler) : `FHOLBuildingTypeRow` gagne `Walkable` (faux) et `Source`
  (FName, vide), en fin de struct ; `BuildingTypes.csv` : deux colonnes en fin (`False,` partout) et ligne `Farm` (Ferme,
  2 × 2 comme le moulin, 400 cm, 1 000 PV, 75 bois, 15 s, case 8 = `W`, palier 0, Economic, rien d'accepté, population 0,
  `Walkable` vrai, `Source` = `FarmField`) ; `HOLBuildingTypeNames::Farm`. Exception de la case W : aucune règle de code
  (la case est la colonne `BuildGridSlot`) ; seul `CommandGrid.Tables` la nomme (la ferme en rangée 3 au lieu de celle de
  sa catégorie) ; commentaires de `BuildGridSlot` et `GetCategoryRow`. Tests : `Construction.Tables` (ferme lue de la vraie
  table + sa source `FarmField` liée ; 6 anciens bâtiments non traversables, sans source), `CommandGrid.Tables` (6 types,
  W = ferme, rangée 3 colonne 1, `PressKey` bâtit, S D F X C V vides). `quick_check` (`build hud`, `build barracks`) :
  `W=Farm greyed`, `I W: Ferme`, info-bulle `I W: Ferme \| W 75 missing`. Reste : compiler, rouge, `make_building_types.py`.
- 2026-10-27 : IA1.2 F4 **verte** (session principale) : rouge vu (« six buildable types » 5, `W` absent, « No building type
  Farm ») ; table régénérée (`make_building_types.py` par `-ExecutePythonScript`, « rows: Barracks, Farm, House, LumberCamp,
  MainCentre, Mill, MiningCamp ») ; `quick_check -Parts tests,build` **PASS** (191/191, toutes les parties `build`), mais
  859 s dont 857 au lancement du jeu (premier lancement après la table : chargement long, à surveiller). Non commitée. F5 lancée.
- 2026-10-27 : IA1.2 F5 (sous-agent, Arwen, écrit sans compiler) : `HOLConstruction::BlocksNavigation(State, bWalkable)`
  (nouvelle surcharge) lue par `AHOLBuilding::BlocksNavigation()` (`Type->Walkable`) ; tous les lecteurs passent par là
  (obstacle `NavObstacle`, garde des emprises et évitement, corps des chemins, rapport) ; rien d'autre ne pose de trou
  (pièces sans navigation ni collision des unités). Placement inchangé (`CheckPlacement` compte tous les bâtiments). Test
  rouge d'abord `Buildings.Walkable` (vraie table, monde de test : ferme fondation / chantier / finie sans zone interdite,
  moulin fini qui bloque, une seule emprise gardée, seconde ferme / maison sur la ferme et ferme sur le moulin refusées).
  Bouchon dans l'arbre (la surcharge ignore `bWalkable`) ; logique = une ligne, patch
  `scratchpad\f5_logic.patch` (`git apply --check` OK). Rouge attendu : ferme en chantier et finie qui bloquent, 2 emprises.
- 2026-10-27 : IA1.2 F5 **verte** (session principale) : rouge vu (7 échecs attendus de `Buildings.Walkable`), patch appliqué
  (`!bWalkable && BlocksNavigation(State)`), `quick_check -Parts tests,build,gather` **PASS** (192/192, build *, footprints,
  gather). Non commitée. F6 découpée : **F6a** naissance / retrait du champ avec la ferme (scénarios compris), **F6b**
  bâtisseurs qui récoltent (ferme finie ; moulin : baies d'abord, Q6). F6a lancée.
- 2026-10-27 : IA1.2 F6a (sous-agent, Arwen, écrit sans compiler) : `AHOLBuilding::LinkedSource` (serveur, pas une
  `UPROPERTY`), `GetLinkedSource`, `UpdateLinkedSource` (fini + type à `Source` → `AddSource` au centre, propriétaire du
  bâtiment, une fois ; sinon retrait) appelée par `SetConstructionState` (seul point de fin de chantier :
  `HOLBuildProcessor.cpp:794`), `Setup` (bâtiment déjà apparu), `PostInitializeComponents` (apparition différée née finie :
  `hol.Cheat.SpawnBuilding` des scénarios, centres de départ, stress test ; tous passent par `SpawnActorDeferred` + `Setup`
  + `FinishSpawning`) et `BeginPlay` (né avant le départ du registre) ; `ReleaseLinkedSource` (une fois) appelée par
  `UHOLConstructionSubsystem::DestroyBuilding` (mort par dégâts, triche, chute) et `EndPlay(Destroyed)` (autres `Destroy`).
  Test rouge d'abord `Buildings.FarmField` (vraie table, vrai registre en monde de test : chantier sans champ, fin → champ
  au centre du joueur 1, moulin sans source, ferme née finie en différé et après apparition, destruction par
  `DamageBuilding` → champ à terre une fois, rien de plus au second coup). Bouchons vides dans l'arbre ; logique dans
  `scratchpad\f6a_logic.patch` (`git apply --check` OK, CRLF vérifié).
- 2026-10-27 : IA1.2 F6a **verte** (session principale) : rouge vu (13 échecs attendus de `Buildings.FarmField`), patch
  appliqué (`UpdateLinkedSource` / `ReleaseLinkedSource` dans `HOLBuilding.cpp`), `quick_check -Parts tests,build,gather,victory`
  **PASS** (193/193). Non commitée. F6b lancée (bâtisseurs qui récoltent).
- 2026-10-27 : IA1.2 F6b (sous-agent, Arwen, écrit sans compiler) : le choix se fait dans `UHOLBuildProcessor::Advance`
  (l'étape après la fin, la source de la ferme déjà née) : `GatherForFarm` avant `GatherForCamp`. Ferme finie : son champ
  au premier bâtisseur (place libre = aucune unité vivante n'a ce champ pour tâche, les bâtisseurs envoyés à ce pas compris :
  `UHOLBuildProcessor::HasFreePlace`), les autres au champ libre à soi le plus proche à 25 m de la ferme
  (`FindFreeFarmField`), sinon règle d'une maison. Moulin : `FindCampSource` ne prend jamais une source liée (baies
  d'abord ; vaut aussi pour l'ordre sur un moulin fini et le ralliement, `HOLUnitSubsystem.cpp:673`) ; sans baies,
  `GatherForCamp` donne à chaque bâtisseur un champ libre à soi à 25 m (`FCampSend::FieldsSent`, ligne de journal). Test
  rouge d'abord `Construction.FarmBuilders` (`Tests/HOLFarmBuildersTests.cpp`, vrais processeurs ordres → récolte →
  bâtisseurs, `hol.Build.Instant`) : (a) ferme à 2 bâtisseurs, (b) moulin baies + ferme plus proche, (c) moulin + ferme
  seule, (d) camp de bois. Bouchons (déclarations, corps vides, appel) dans l'arbre ; logique dans
  `scratchpad\f6b_logic.patch` (`git apply --check` OK, CRLF gardé).
- 2026-10-27 : IA1.2 F6b : rouge vu (`Construction.FarmBuilders` : bâtisseurs de la ferme inactifs, ceux du moulin au champ
  au lieu des baies), patch appliqué, compilation PASS (230 s). Vérification **arrêtée à la limite des 15 min** (rappel de
  l'utilisateur à 13 min) : `quick_check` à 4 parties sous processeur à 100 % (deux autres sessions compilent, dont
  Glorfindel) ; `-Parts tests` seul : l'éditeur s'est figé au démarrage (« Loading Windows ini files took 19 s »), 0 test,
  FAIL en 423 s, sans rapport avec le code. **Reste** : `-Parts tests` au calme, puis `build,gather,victory` à part.
  Leçon : sous forte charge, ne pas enchaîner compilation + `quick_check` multi-parties ; regarder la charge avant, découper.
- 2026-10-27 : info de NetPlaytest (mycrusader-17) : depuis AS1, chaque jeu / serveur de test ouvre le serveur de débogage
  Angelscript sur le port par défaut, l'extension VS Code s'y accroche et fige le jeu (« Sending debug database ») →
  `-asdebugport=0` (ajouté aux lancements de `quick_check` sur `net-playtest`, arrive à sa fusion) ; et
  `UE_SKIP_UBT_SDK_SETUP=1` évite qu'un jeu attende au démarrage derrière les compilations des autres copies. Relance de la
  vérification de F6b avec cette variable.
- 2026-10-27 : IA1.2 F6b **verte** (relance après 55 min, `UE_SKIP_UBT_SDK_SETUP=1`, processeur encore à 100 %) :
  `quick_check -Parts tests` **PASS 194/194** (56 s, `Construction.FarmBuilders` vert), puis `-Parts build,gather,victory`
  **PASS** (96 s). Non commitée. F1-F6b vertes dans Arwen. Reste : F7 (clic droit et ralliement sur une ferme, `FindSourceAt`
  saute les sources liées), F8 (forme de champ, noms fr / en / ja), F9 (étape `build farm` de `quick_check`), F10 (scénario
  `farms`), F11 (essai de l'utilisateur, commit).
- 2026-10-27 (soir) : séance close à la demande de l'utilisateur ; reprise à **F7** dans une nouvelle session (Arwen garde
  `IN_USE.txt`, branche `ai-ia1`, F1-F6b non commitées ; stash « IA0.4 loop 1 » obsolète).
  **Reprise (« continue AIInProgress »)** : F7 d'abord (clic droit / ralliement sur une ferme à soi finie → récolter son
  champ ; `FindSourceAt` saute les sources liées ; le dépôt par clic droit, D485, est arrivé dans `main` 15b7664 : fusionner
  `main` dans `ai-ia1` avant F7, conflit possible dans `HOLPlayerController`). Méthode qui marche (F4-F6b, 12-13 min) :
  le sous-agent écrit test + bouchons dans l'arbre (rouge) + logique en patch dans le scratchpad (`git apply --check`), sans
  compiler ; la session principale compile (`Tools\build.ps1 -Root <Arwen>`), lance le seul nouveau test (rouge), applique,
  recompile, puis `quick_check -Parts tests` seul, puis les parties en vraie partie dans une boucle à part. Minuteur de 15 min
  avec chaque sous-agent ; regarder la charge avant de vérifier ; `$env:UE_SKIP_UBT_SDK_SETUP='1'` avant `quick_check` sous
  charge ; colonnes de table changées → compiler puis `UnrealEditor.exe <uproject> -ExecutePythonScript=<Tools\Editor\make_*.py>
  -nullrhi -unattended -nosplash`.
- 2026-10-27 : IA1.2 F7 (sous-agent, Arwen, écrit sans compiler) : le chunk réplique type et place (`FHOLNetSource`,
  `FVector3f`) mais pas le propriétaire ; la ferme étant vérifiée à soi d'abord, ça suffit. Recherche valable partout :
  `UHOLSourceSubsystem::FindLinkedSourceAt(Location, Tolerance = 10 cm)` (source debout, type `Linked`, distance au sol) et
  `AHOLBuilding::FindLinkedSourceHere()` (finie, type à `Source`, la source liée à son centre ; sur le serveur =
  `GetLinkedSource()`). Picking : `static AHOLPlayerController::IsPickableSource(const FHOLSourceTypeRow&)` (`!Linked`), lue par
  `FindSourceAt`. `OrderAt` (dans l'arbre, sans danger sur bouchons) : `SourceIndex` / `Source` modifiables ; juste après le
  bloc `NeedsBuilders`, sans source sous le curseur et avec une sélection, le bâtiment sous le curseur (`Site`, sinon
  `FindBuildingAt` pour le ralliement) à soi dont `FindLinkedSourceHere` rend un champ → `SourceIndex` / `Source` = ce champ
  (journal « Order: on farm <bâtiment>: its field, source N »), la suite prend le chemin des sources tel quel (récolte,
  `RallySource`) ; rien ajouté près de `if (Source)` (bloc D485 de `main`). Ferme : `IsCamp()` faux (`Accepts` vide), donc
  pas de `RallySite`. Test rouge d'abord `HeroesOfLiberty.Buildings.FarmClick` (`Tests/HOLBuildingTests.cpp`, vraies tables,
  vrai registre en monde de test). Bouchons dans l'arbre : `FindLinkedSourceAt` et `FindLinkedSourceHere` → `INDEX_NONE`,
  `IsPickableSource` → `true`. Rouge attendu (6 échecs) : FarmField non cliquable ; ferme finie trouvée = `GetLinkedSource`,
  à son centre, à quelques cm ; ferme née finie du joueur 2 trouvée ; son champ encore trouvé après la mort de l'autre.
  Verts dès les bouchons : chantier sans champ, buisson au centre d'un moulin non pris, ferme détruite sans champ, rien à
  5 m, baies / arbre cliquables. Logique dans `scratchpad\f7_logic.patch` (3 corps, `git apply --check` OK, CRLF gardé).
  Le clic réel reste pour F9 (`quick_check` `build farm`).
- 2026-10-27 : IA1.2 F7 **verte** (session principale) : compilation des bouchons PASS (108 s), rouge vu (les 6 échecs
  attendus de `Buildings.FarmClick`, test seul en 31 s), patch appliqué, recompilé (121 s), `FarmClick` vert ;
  `quick_check -Parts tests` **PASS 195/195** (43 s). Placement choisi contre le conflit avec D485 de `main` : la ferme à soi
  finie sous le curseur devient « la source cliquée » juste après le bloc `NeedsBuilders` de `OrderAt` (rien près de
  `if (Source)`). `-Parts build,gather,move` lancé à part. Non commitée.
- 2026-10-27 : IA1.2 F7 : `quick_check -Parts build,gather,move` **PASS** (60 s, toutes les parties `build`, gather,
  footprints, move). F7 close (non commitée). F8 lancée (sous-agent : forme de champ, noms fr / en / ja).
- 2026-10-27 : info de l'Arbalétrier (mycrusader-99) : `main` ce7cc76 (D486-D489) ajoute le Champ de tir et l'Arbalétrier,
  noté au plan d'IA2. **À la fusion de `main` (F11)** : `BuildingTypes.csv` (ligne `ArcheryRange` contre nos colonnes
  `Walkable` / `Source` : lui ajouter `False,`), case S prise par le Champ de tir (`CommandGrid.Tables` d'IA1.2 attend S vide),
  attentes `build hud` de `quick_check`, `HOLGameNames.cpp`, `fill_po.py`, `HOLBuilding*` : relire, régénérer la table.
- 2026-10-27 : IA1.2 F8 (sous-agent, Arwen, écrit sans compiler) : test `HeroesOfLiberty.Buildings.FarmShape`
  (`HOLBuildingTests.cpp`, vraie table, monde de test) : ferme à soi en fondation (fantôme), chantier (1er travail, mi-chemin)
  et finie : pièces visibles < `FieldMaxHeight` (30 cm), au sol, sur 8 × 8 m à 10 % ; finie = terre + 4 bords couleur du
  joueur + `FieldRowCount` (5) rangs ; rayon de sélection au centre trouve la ferme ; moulin > 100 cm ; `Farm->Height` de la
  table < 30. Bouchon : `BuildFieldShape()` (branche `Walkable` sans mesh de `RefreshVisuals`) refait la forme bloc + tour ;
  constantes `Field*` dans `HOLBuilding.h`. Rouge attendu : finie 4 pièces au lieu de 10, 1 bord au lieu de 4, piquets du
  chantier à 90 cm ; et tant que la table n'est pas régénérée, hauteur 400 (fantôme, chantier à mi-chemin, finie, test de
  table). `BuildingTypes.csv` : `Height` de Farm 400 → **20** (barre de vie, marqueur, fantôme) : **régénérer la table**.
  Logique dans `scratchpad8_logic.patch` (corps de `BuildFieldShape` : terre 8 cm, bords 12 cm, rangs 16 cm vert / blé ;
  piquets d'un `Walkable` à 25 cm ; `git apply --check` OK, CRLF gardé). Noms : `HOLGameNames.cpp` Farm « Ferme »,
  FarmField « Champ » ; `fill_po.py` Farm « Farm » / « 農場 », FarmField « Field » / « 畑 » (GatherText → fill_po →
  GatherText à faire). Pas de test de nom (module UI hors de portée des tests du jeu) ; `quick_check` attend déjà « Ferme ».
- 2026-10-27 : IA1.2 F8 **verte au test** (session principale) : bouchons compilés (144 s), table régénérée (`Height` de
  Farm 400 → 20 : barre de vie, marqueur, fantôme), rouge vu (`Buildings.FarmShape` : piquets du chantier à 90 cm, 4 pièces
  au lieu de 10, 1 bord au lieu de 4), patch appliqué, recompilé (228 s), `FarmShape` vert. Traductions : GatherText →
  `fill_po.py` (en / ja : 318 textes, 0 clé inconnue ; Ferme = Farm / 農場, Champ = Field / 畑) → GatherText. Le doute du
  sous-agent sur un dessin du champ par-dessus les rangs est sans objet (source liée sans forme, F2). Reste :
  `quick_check -Parts tests`, puis `build` à part.
- 2026-10-27 : IA1.2 F8 : `quick_check -Parts tests` **PASS 196/196** (39 s), puis `-Parts build` **PASS** (58 s, toutes les
  étapes `build`, `I W: Ferme` en fr). F8 close (non commitée). F9 lancée (sous-agent, étape `build farm`, écrite sans lancer).
- 2026-10-27 : IA1.2 F9 (sous-agent, Arwen, écrit sans lancer) : étape `build farm` à la fin de la partie `build` de
  `quick_check.ps1` (après `build barracks ghost`), 4 marques : `build farm` (ferme B (-6000, 12400) bâtie par 1 Paysan,
  puis envoyé à (-4400, 10800) : champ de B libre), `build farm a` (ferme A (-6000, 13200), collée au nord de B, bâtie par
  2 Paysans, 12 s de récolte), `build farm click` (3e Paysan, `hol.Select.OrderGround` sur A à (-5750, 13350) = vrai
  `OrderAt`), `build farm cross` (`hol.Nav.Path` et un marcheur de (-6000, 15000) à (-6000, 10600), à travers A et B,
  `hol.Units.LogDetours 1`). Fermes à 16-24 m à l'est du moulin Mi (dépôt) ; triche bois 150 (2 × 75) ; `Build.Speed 4`,
  `Gather.SpeedScale 5`, remis à 1 à la fin ; ses Paysans retirés (`hol.Units.Remove`), les fermes restent. Juge :
  « Building Farm of slot 0 finished: its FarmField is source N » pour A et B ; bâtisseurs de A : un « its farm's field
  (source A) », l'autre « a free field (source B) » ; leurs dépôts de nourriture à Mi (2 unités, par leurs #id) ;
  `Order: on farm Farm of slot 0 (own)…: its field, source A` + `Order: 1 units gather source A` + `Gather order … (Food)` ;
  règle F3 lue dans le code : place de A et de B prises, pas de nourriture à 20 m → « found no room in its massif, it
  stands idle in front of it », aucun dépôt du 3e ; chemin du navmesh ratio ≤ 1,02, aucune ligne `Detour:`, marcheur à
  ≤ 4 m du but après 8 s (44 m). Rapport `build farm`. Lignes : constantes `$Farm*` 643-656, séquence 1398-1435,
  analyse 3290-3336. Syntaxe vérifiée par le parseur PowerShell ; ~35 s de jeu. Doutes : terrain libre à l'est de Mi
  (fantôme « valid » jamais vu), `ClickUnit -2` = l'autre Paysan, chemin de B → la 2e ferme dans 25 m ; à lancer :
  `quick_check -Parts build`.
- 2026-10-27 : IA1.2 F9 **verte** (session principale) : étape `build farm` de `quick_check` (fin de la partie `build`) :
  ferme B bâtie par un Paysan (champ né, il le récolte, puis renvoyé), ferme A bâtie par deux (l'un sur son champ, l'autre
  sur le champ libre de B), nourriture déposée au moulin Mi, clic droit d'un troisième sur A (`Order: on farm`, place
  prise : debout devant), marcheur à travers A et B (chemin droit, sans détour, arrivé). Trois retouches de la séquence,
  aucune du jeu : second clic sur un Paysan déjà choisi (pas de nouvelle ligne `Selection`), deux Paysans nés ensemble qui
  se chevauchent (nés un par un à 3 m), cadre trop étroit (remplacé par `ClickUnit -2 add`). `quick_check -Parts build`
  **PASS** (109 s, toutes les étapes `build`). Boucle à ~16 min (trois relances de 2,5 min). Non commitée. Prochain : F10.
- 2026-10-27 : IA1.2 F10 (sous-agent, Arwen) : scénario `farms` (`Tools/Scenarios/farms.txt`, sans `.json` : un joueur,
  Plain) : à l'est du moulin Mi de quick_check, moulin M (-5200, 12800) fini, anneau de 3 × 3 blocs de 8 m autour :
  5 fermes finies récoltées (-6000, 13600 / 12800 / 12000), (-4400, 13600), (-4400, 12000), 1 finie libre (-4400, 12800,
  son bâtisseur renvoyé), 2 cases vides à bâtir (-5200, 13600) et (-5200, 12000) ; caserne finie (-2600, 12600) pour le
  ralliement (laissé à l'utilisateur) ; 5 Paysans libres vers (-5200, 10200) ; triche 5000, population 40. Placement par
  la grille (`hol.Camera.Focus` + `hol.Build.Place` + fantôme « valid » vérifié + `hol.Build.Click`), chantiers finis au
  premier coup (`hol.Cheat.FastBuild 1`, remis à 0) ; vérifie « finished Farm at », « its FarmField is source », un dépôt
  de nourriture au moulin (`Gather.SpeedScale 5` le temps d'un dépôt, remis à 1), rien de sélectionné, caméra sur le
  moulin. Il faut `hol.Camera.TestCursor 0.5 0.5` pendant les placements (sinon le fantôme suit le vrai curseur et la
  caméra défile au bord : 1re répétition en échec, fantôme du moulin balayant y = 6800), rendu (`off`) avant les notes.
  Répétitions hors écran (`-RenderOffScreen -nosound -asdebugport=0`, `hol.Scenario.AutoContinue 1`, éditeur lancé
  directement : play.ps1 ne prend pas d'arguments) : **2 vertes**, « Scenario farms: ready » à 52 s du lancement (séquence
  31 s) les deux fois. Capture : `HeroesOfLiberty-Arwen\Saved_Tests\Screenshots\WindowsEditor\HOL\farms_ring.png` (anneau
  net, Paysans sur les champs, +10 au moulin ; dans la vue de départ les cases vides sont à gauche et à droite du moulin,
  la caserne en haut). Lancement : `& C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 farms`.
- 2026-10-27 : IA1.2 F10 relue par la session principale (capture `farms_ring` : 6 fermes autour du moulin, 5 récoltées,
  1 libre, 2 cases vides, dépôt +10). Liste des cas avant l'essai : bâtir (`build farm`), champ né / retiré
  (`Buildings.FarmField`), une place et débit (`Gather.FarmField`), traversable (`Buildings.Walkable` + marcheur de
  `build farm`), bâtisseurs (`Construction.FarmBuilders` + `build farm`), clic droit (`Buildings.FarmClick` + `build farm`),
  forme (`Buildings.FarmShape`), noms (`build hud`). **Cas sans test en vraie partie** : ralliement d'un bâtiment sur une
  ferme (seule la recherche du champ est testée ; le chemin `RallySource` est celui d'une source) → vu à l'essai.
  **Essai de l'utilisateur demandé (F11)** : `play.ps1 farms` dans Arwen.
- 2026-10-27 : **essai de l'utilisateur : « tout marche parfaitement »** (scénario `farms`, ralliement sur une ferme compris).
  IA1.2 commitée dans Arwen (fb29b46). Fusion de `main` (b40b180) : conflits avec le Champ de tir (AR, D486-D489) et les
  fiches de héros résolus (CSV : colonnes `Walkable`, `Source` et `Shape` gardées, la ferme dessinée en champ par
  `Walkable` ; ferme en W à côté du Champ de tir en S ; `CommandGrid.Tables` à 7 types, D F X C V vides ; attentes
  `build hud` / `build barracks` ; deux formes provisoires ; noms et `fill_po` des deux) ; table et traductions refaites
  (324 textes, 0 clé inconnue) ; recompilé ; `quick_check -Parts tests` **217/217**, `build,gather,move` **PASS** (farm,
  hero, dropoff, shiftwork), `production,sources,victory` **PASS**, `net` **PASS** (2 clients, fiches de héros). `main`
  (c8ec209, doc + play.ps1) refusionnée (3ae16b3). Avance rapide de `main` : accord demandé aux sessions mycrusader-bc,
  -17, -fb, -b8. Leçon : le jeu d'essai resté ouvert verrouillait deux `.uasset` (première fusion ratée, nettoyée) →
  fermer le jeu d'essai avant de fusionner.
- 2026-10-27 : quatre « ok » ; `main` avancée à 3ae16b3 (avance rapide), DOC 6cddb93 (journal + mes deux lignes
  d'`EtapeDefini.md`, celles des autres sessions laissées non commitées) ; `ai-ia1` = `main` 6cddb93 ; copie principale
  recompilée. **Prochain : IA0.1** (PlayerState de l'emplacement IA ; R7 est dans `main`), puis IA0.4, IA0.5 (préalables
  d'IA1.3+ au plan).
- 2026-10-27 : IA0.1 boucle 1 : relevé en lecture seule fait (section « IA0.1 Emplacement IA : relevé du code ») :
  `EHOLSlotState::AI` existe (menu grisé, absent du salon), un emplacement IA a déjà centre, Roi, Paysans, héros et compte
  dans la victoire, mais aucun PlayerState : dépôt perdu sans journal, fondation jetée sans journal, production refusée ;
  un `AHOLPlayerState` né par `SpawnActor` s'inscrit seul dans `PlayerArray`, est trouvé par les cinq `FindPlayerState` et
  `GetSlotStock`, et son économie reste cachée aux clients (`COND_OwnerOnly`). Lecteurs de `PlayerArray` : aucun ne casse ;
  à corriger : `waitplayers` (compter les humains seuls), `UpdateStartWait` (sauter `IsABot`), attente « no stock » du test
  `AI.Summary.World`. Découpage E1-E6 (E1 naissance statique avant `SpawnMainCentres` ; E2 économie sur la vraie chaîne ;
  E3 résumé ; E4 correctifs ; E5 `quick_check` + `starting_peasants` ; E6 commit) ; questions IA0.1-Q1 à Q5 à poser (nom de
  l'IA d'abord : recommandation « IA N »).
- 2026-10-27 : IA0.1 E1 (sous-agent, Arwen, écrit sans compiler) : test rouge `HeroesOfLiberty.AI.Slot.Spawn`
  (`Tests/HOLAISlotTests.cpp`, vrai monde + `AHOLGameState`, config humain 0, IA 1 couleur 5 et IA 3 couleur 2, le reste
  fermé, stock lu de `UHOLMatchSettings::StartingResources`) ; dans l'arbre, **bouchon** `SpawnAIPlayerStates` (rend 0)
  déclaré dans `HeroesOfLibertyGameModeBase.h` et appelé dans `StartPlay` avant `SpawnMainCentres` (cartes à départs
  seulement), et `MakeAIPlayerName(Slot)` déjà réel (= `HOLForumMessages::GetSlotName(Slot, INDEX_NONE)`, « Joueur N » :
  la seule ligne à changer pour IA0.1-Q1). Rouge attendu sur le bouchon : « two spawned », « two player states »,
  « slot 1/3: a player state » (le reste de la boucle sauté), « still two player states » ; verts : stock de départ non
  vide, rien pour 0 et 2, « second call: none spawned ». Vraie logique (sauter client et monde sans GameState ; un
  `AHOLPlayerState` par emplacement `AI` non tenu ; `SetIsABot`, nom, `SetMatchSlot(Slot, couleur)`, `SetResources` ;
  journal « AI slot N: player state, F 200 … (colour C, 'Joueur N') ») dans
  `C:\Users\baudo\AppData\Local\Temp\claude\c--Users-baudo-Documents-MyCrusader\31239e0c-7a0a-4376-8bef-69fe094a906d\scratchpad\e1_logic.patch`
  (`git apply --check` OK, appliqué puis retiré pour vérifier : CRLF gardé). Humain reproduit : emplacement + couleur
  (`PostLogin`) et stock de départ ; population comptée par la production à chaque pas, camp lu de la config
  (`InitVictory`), rien d'autre posé pour un humain. `AI.Summary.World` ne passe pas par `StartPlay` : inchangé (vert).
- 2026-10-27 : IA0.1 E1 **verte** (session principale) : bouchon compilé (57 s), rouge vu (`AI.Slot.Spawn` : 5 échecs
  attendus, aucun PlayerState IA), patch appliqué, recompilé, vert ; `quick_check -Parts tests` **PASS 218/218** (35 s),
  `-Parts victory,production,sides` **PASS** (34 s, emplacements IA de `quick_check` avec leur PlayerState). Nom de l'IA :
  « Joueur N » en attendant IA0.1-Q1 (une ligne, `MakeAIPlayerName`). Non commitée. E2 lancée (sous-agent).
- 2026-10-27 : IA0.1 E2 (sous-agent, Arwen, écrit sans compiler) : test `HeroesOfLiberty.AI.Slot.Economy`
  (`Tests/HOLAISlotTests.cpp`, monde `FEconomyWorld` = celui d'`AI.Orders.*` + construction et production liées au pas par
  `OnWorldBeginPlay`, centre de l'emplacement 1 à l'origine, PlayerStates IA par le vrai `SpawnAIPlayerStates` d'E1).
  Avec PlayerState : fondation de maison par `HOLPlayerOrders::Place` (le chemin du cerveau ; pas encore dans
  `UHOLAIOrderLibrary`) → posée au pas suivant, couleur 5 de la config, stock débité du coût de la table ; Paysan par
  `IssueProductionOrder` (le contrôleur fait pareil : **aucune fonction de production dans `HOLPlayerOrders` ni
  `UHOLAIOrderLibrary`**, à ajouter pour IA0.4) → en file, payé, né après `BuildSteps` pas (`ServeBirthRequests`) ;
  10 bois par `UHOLGatherProcessor::DepositLoad` → crédités (pas de vraie marche : le monde de test n'a pas de
  déplacement). Contre-épreuve sans PlayerState : fondation et file refusées, 10 bois vidés et perdus (`GetSlotStock`
  faux). Lecture du code : **les trois passent déjà avec E1** (ni contrôleur exigé, ni navmesh : `bOnNavMesh` vrai par
  défaut, pas de `AHOLMapBounds`) ; le test est rouge aujourd'hui seulement par les deux journaux attendus de la
  contre-épreuve (E4) : « Construction order of slot 1 ignored: no player state » (le `continue` muet) et « Production
  order of slot 1 ignored: no player state » (au lieu du trompeur « not its building »), tous deux `Warning`, dans
  `C:\Users\baudo\AppData\Local\Temp\claude\c--Users-baudo-Documents-MyCrusader\31239e0c-7a0a-4376-8bef-69fe094a906d\scratchpad\e2_logic.patch`
  (`git apply --check` OK). Doutes : `Warning` aussi pour un ordre de production d'un spectateur (emplacement -1,
  avant « not its building » en `Display`) ; Paysan IA au centre seulement (pas de bâtisseur sur la fondation).
- 2026-10-27 : IA0.1 E2 **verte** : `AI.Slot.Economy` compilé, rouge vu sur la seule raison attendue (les deux avertissements
  « Construction / Production order of slot 1 ignored: no player state » absents ; fondation payée, Paysan produit, dépôt
  crédité déjà verts grâce à E1, contre-épreuve sans PlayerState : tout refusé ou perdu) ; patch appliqué (avertissements à
  la place du `continue` muet et de « not its building ») ; vert ; `quick_check -Parts tests` **219/219**.
  Noté pour IA0.4 : aucune fonction de production dans `HOLPlayerOrders` / `UHOLAIOrderLibrary` (le contrôleur appelle
  `IssueProductionOrder`), ni de pose dans `UHOLAIOrderLibrary` (`HOLPlayerOrders::Place` existe).
- 2026-10-27 : IA0.1 E3 + E4 (session principale, petites retouches) : `AI.Summary.World` lit le stock de l'emplacement IA
  après le vrai `SpawnAIPlayerStates` (et aucun avant) ; `waitplayers` ne compte que les humains, `UpdateStartWait` saute
  `IsABot` (pas de test dédié : sans effet aujourd'hui, vu par `-Parts net`). Les 11 tests `HeroesOfLiberty.AI.*` verts.
- 2026-10-27 : IA0.1 E5 : `quick_check -Parts tests,victory,production,gather,sides` **PASS** (219/219, 37 s), `-Parts net`
  **PASS** (2 clients, fiches de héros, 144 s), scénario `starting_peasants` hors écran prêt en 18 s (« AI slot 1: player
  state, F 200 W 200 S 200 G 200 (colour 1, 'Joueur 2') », 5 Paysans de chaque camp). **IA0.1 finie, non commitée** (rien de
  visible à essayer avant IA0.4) : attend l'accord de commit de l'utilisateur et **IA0.1-Q1** (nom de l'IA : recommandation
  B « IA N » / « Ordinateur N » ; aujourd'hui « Joueur N », une ligne dans `MakeAIPlayerName`). Q2-Q5 : recommandations
  du relevé appliquées d'office (stock de l'humain, emplacements IA des scénarios gardés, l'IA compte dans la victoire,
  couleur de la config).
- 2026-10-27 : IA0.4 boucle 1 : relevé en lecture seule fait (sous-agent, Arwen `ai-ia1`) : section « IA0.4 Cerveau en
  script ». Le résumé et les ordres sont déjà liés au script (`HOLAISummary::ReadAISummary`, `HOLAIOrder::IssueAIGatherOrder`) ;
  manquent l'appel au pas (délégués natifs) → base C++ abstraite `UHOLAIBrainSubsystem` dérivée en script (créée d'office,
  pensée sur `OnStepEnded`), la liste des emplacements IA, le filtre `IsGatherableBy` du résumé, un interrupteur par
  emplacement, un journal de décision. Champs du résumé non écrivables en script (`BlueprintReadOnly`). Découpage B1-B6 ;
  questions IA0.4-Q1 à Q4 (recommandations : A, A, A, A). Rien codé.
- 2026-10-27 : IA0.4 B1 (sous-agent, Arwen, écrit sans compiler) : `AI/HOLAIBrain.h/.cpp` : `UHOLAIBrainSubsystem`
  `UCLASS(Abstract)`, Game/PIE (`DoesSupportWorldType`), `ShouldCreateSubsystem` refuse la classe abstraite et une classe
  remplacée (`CLASS_NewerVersionExists`, `IsUnreachable` comme `UScriptWorldSubsystem`) ; `OnWorldBeginPlay` hors client →
  `GetOnStepEnded().AddUObject` (retiré dans `Deinitialize`) ; à chaque pas, emplacements `GetAISlots()` (PlayerStates
  `IsABot` triés par indice, vide sans GameState) dont `(Step + Slot) % hol.AI.ThinkSteps == 0` (défaut 5, au moins 1) :
  journal `Verbose` « AI brain: slot N thinks (step S) » (`LogHOLMatch`) puis `ThinkNow(Slot)` ; `ThinkNow` (refusé sur
  client) appelle `Think` = **`BlueprintNativeEvent` qui rend un `bool`** (C++ : `false`, rien), compte `GetNumThoughts()`
  quand le script rend vrai ; `Find(World)` = `GetSubsystemArrayCopy<UHOLAIBrainSubsystem>()[0]`. Redéfinition sûre
  (vérifiée dans le moteur) : même forme que `UScriptWorldSubsystem::BP_ShouldCreateSubsystem` (NativeEvent à retour,
  redéfini par `UFUNCTION(BlueprintOverride)`) ; AC `Binds/Bind_BlueprintEvent.cpp:461-552` lie tout événement C++,
  `ClassGenerator/AngelscriptClassGenerator.cpp:457-462` accepte la redéfinition, `:3180-3182` classe de script
  `CLASS_CompiledFromBlueprint` sans `CLASS_Native` ni `CLASS_Abstract` hérités, `:2592-2596` + `:2437-2453` activée
  comme sous-système externe (après `PostEngineInit` au premier chargement) ; moteur `SubsystemCollection.cpp:409`
  saute les classes abstraites. Pas de repli écrit. Test `HeroesOfLiberty.AI.Brain.Exists` (`Tests/HOLAIBrainTests.cpp`,
  monde nu) : brain trouvé, classe ≠ C++ et non native, aucun emplacement sans GameState, `ThinkNow(1)` vrai, 1 pensée.
  **Rouge attendu sans le `.as`** (« A brain subsystem in a game world » nul). Script dans le scratchpad (`b1/HOLAIBrain.as`,
  `class UHOLAIBrain : UHOLAIBrainSubsystem`, `Think` compte et rend vrai) : à copier dans `Script/AI/` après le rouge.
  À faire : compiler, voir le rouge, copier le `.as`, vert, `-Parts tests` (le sous-système naît dans tous les mondes de test).
- 2026-10-27 : IA0.4 B1 **verte** (session principale) : compilé (75 s), rouge vu (`AI.Brain.Exists` : aucun sous-système,
  pas de sous-classe de script), `Script/AI/HOLAIBrain.as` copié, vert sans recompiler (le script se compile au démarrage) :
  la classe de script `UHOLAIBrain` dérive de `UHOLAIBrainSubsystem` et son `Think` (`BlueprintOverride`) est atteint.
  `quick_check -Parts tests` **PASS 220/220**, `-Parts victory,production,sides,gather` **PASS** (cerveau présent, pense sans
  agir). Non commitée (avec IA0.1). **Arrêt ici** : B2 (la règle de `Think`) dépend d'**IA0.4-Q1** (ressource visée) ;
  questions en attente de l'utilisateur : IA0.1-Q1 (nom de l'IA), IA0.4-Q1 à Q4, accord de commit d'IA0.1 + B1.
  `main` avancée par NetPlaytest à b5a2e82 (ferme + R7.10) : à fusionner dans `ai-ia1` au prochain commit.
- 2026-10-27 : IA0.1-Q1 = B avec le niveau (« IA Difficile 3 »), Q1b = A (champ niveau maintenant) ; nom (sous-agent, Arwen,
  écrit sans compiler) : enum `EHOLAIDifficulty { Easy, Normal, Hard, Evolving }` et champ `FHOLMatchSlot::Difficulty`
  (Normal par défaut, `"difficulty": "Hard"` dans le `.json`, valeur inconnue = Normal + avertissement `LogHOLMatch`) ;
  `HOLForumMessages::GetDifficultyName`, `GetAIName` (« IA {0} {1} »), `GetSlotName(Slot, Viewer, Config)` (IA nommée,
  humain inchangé) et `GetPlayerName(Config, Slot, Viewer)` (Vous / nom du salon / IA / Joueur N) ; `MakeAIPlayerName(Config,
  Slot)`. Test **`HeroesOfLiberty.AI.Slot.Name`** (`HOLAISlotTests.cpp`) : PlayerState par le vrai `SpawnAIPlayerStates`,
  fil (`GetMessage`, `SplitNames`), tableau (`GetPlayersTable`), bandeau/stats (`GetPlayerName`), `.json` par
  `MakeScenarioConfig` (Hard, absent, inconnu), aller-retour `ToJson`/`FromJson`. **Rouge attendu** (bouchons : « Joueur N »,
  le `.json` ignore le champ). Vraie logique dans le patch du scratchpad `name_logic.patch` (`git apply --check` OK) : noms,
  lecture du `.json`, tableau, fil coloré (`HOLGameScreen`), bandeau (`HOLGameHUD`), stats (`HOLStatsScreen`).
  Traductions dans `fill_po.py` (AI Easy/Normal/Hard/Evolving ; ja かんたん / ふつう / むずかしい / 成長型). Restent « Joueur N »
  pour une IA : le propriétaire de la sélection (`HOLGameScreen` l. 1130, 1264). À faire : compiler, voir le rouge,
  appliquer le patch, vert, GatherText, `-Parts tests`.
- 2026-10-27 : **IA0.1 + B1 commitées** dans Arwen (e8cf110, accord de l'utilisateur, « A »), `main` b5a2e82 fusionnée
  (d7073d4, 222/222). **IA0.1-Q1 = B avec le niveau** (« IA Difficile 3 ») ; **Q1b = A** (champ niveau maintenant).
  Nom : rouge vu (`AI.Slot.Name`), patch appliqué (+ variable `PlayerName` remise dans `HOLForumRules.cpp`), vert ;
  propriétaire d'une sélection (`HOLGameScreen.cpp`, `GetOwnerNameIn`) par le même nom ; traductions refaites (327 textes,
  0 clé inconnue ; ja かんたん / ふつう / むずかしい / 成長型). `quick_check -Parts tests` : 218/223, 5 tests de victoire qui
  attendaient « Joueur N » pour des emplacements IA → attentes passées à « IA Normal N » (`HOLForumMessagesTests.cpp`,
  `HOLVictoryTests.cpp` l. 1616 et 1673, `SplitNames(Held, 1, Teams)`), **non recompilé** (séance close par l'utilisateur).
  **Reprise** : recompiler Arwen, `quick_check -Parts tests`, puis `-Parts victory,sides,build` (noms dans le HUD), puis
  commit (nom + niveau). Question posée, sans réponse : **IA0.4-Q1** (ressource visée par le cerveau ; recommandation A, la
  plus basse du stock), puis Q2-Q4 (recommandations A), puis B2.
- 2026-10-27 (reprise) : Arwen recompilé (53 s), `quick_check -Parts tests` **PASS 223/223** (attentes « IA Normal N » des 5
  tests de victoire vertes). Désormais une boucle = un sous-agent (15 min / 300k tokens, minuteur dans le même message,
  demande de l'utilisateur) ; la session principale garde le fil, les questions et les commits. Boucle 2 lancée en
  sous-agent : `quick_check -Parts victory,sides,build` (noms de l'IA dans le HUD et le tableau des joueurs).
- 2026-10-27 : boucle 2 (sous-agent, 4 min 17, 74k tokens) : `-Parts victory,sides,build` FAIL sur 3 attentes périmées de
  `quick_check.ps1` (l. 2347, 2363, 2417 : « Joueur 2 » pour un emplacement IA) ; tout le reste PASS (sides, camera 2p/8p,
  16 sous-parties de build). Noms vus dans le journal : « Players table: Vous, IA Normal 2 », 8p « IA Normal 1 … IA Normal 7,
  Vous », « IA Normal 2 tient votre ruine », « Le Roi de IA Normal 2 est mort », stats « row IA Normal 2 » ; « Joueur 2 »
  restant = emplacement humain (camera_2p), voulu. Corrigé : les 3 attentes et les notes de `regicide.txt` /
  `forum_messages.txt` (« IA Normal 2/3/4 »). **IA0.4-Q1 = A** (ressource la plus basse du stock, première passe, remplacée
  par la règle de besoins d'IA1.6 et les niveaux d'IA4). Relance de `-Parts victory` en sous-agent.
- 2026-10-27 : `-Parts victory` relancé (sous-agent, 56 s) **PASS** (victory, forum, regicide). **Nom de l'IA commité** dans
  Arwen (d42b5a8 : niveau par emplacement, « IA <Niveau> <N> », traductions, attentes et notes de scénario). En attente :
  **IA0.4-Q2** (tests du cerveau ; recommandation A), puis Q3-Q4, puis B2.
- 2026-10-27 : **IA0.4-Q2 = A** (test C++ sur la vraie chaîne + tests en script de la règle, champs du résumé
  `ScriptReadWrite`). Question suivante : IA0.4-Q3 (interrupteur des scénarios).
- 2026-10-27 : **IA0.4-Q3 = A** (champ `"aiBrain"` par emplacement du `.json`, défaut vrai, faux dans les scénarios de
  combat et de `quick_check`). Question suivante : IA0.4-Q4 (fréquence).
- 2026-10-27 : **IA0.4-Q4 = A, C à terme** (pensée tous les 5 pas, `hol.AI.ThinkSteps`, étalée par emplacement ; réglage
  par niveau à IA4, D422). Toutes les questions d'IA0.4 tranchées (A, A, A, A). B2 (la règle dans `Think`, test
  `AI.Brain.GatherIdle`) lancée en sous-agent : écrit sans compiler, logique dans un patch du scratchpad.
- 2026-10-27 : IA0.4 B2 (sous-agent, Arwen, écrit sans compiler) : test **`HeroesOfLiberty.AI.Brain.GatherIdle`**
  (`Tests/HOLAIBrainTests.cpp`, monde `FBrainWorld` : `AI.Summary.World` + processeurs d'`AI.Orders.*` ; slot 1 IA par le
  vrai `SpawnAIPlayerStates` au stock de départ, son centre fini en (0,0), baies (0,1800), arbre (2000,0), un Paysan IA et un
  Paysan humain inactifs). Cas 1 (stock égal) : `ThinkNow(1)` → rien dans l'image, `RunStep` → Paysan IA `GatherTrip`, message
  « Gather order of slot 1: 1 units to source <baies> (Food) » une fois ; cas 3 (même monde, plus d'inactif) : `ThinkNow(1)` →
  file de récolte vide (`ConsumeGatherOrders`) ; cas 2 (second monde, nourriture +100 par `SetResources`) : « … source <arbre>
  (Wood) » une fois ; Paysan humain inactif partout ; `GetAISlots()` = {1}. Ajout C++ dans l'arbre (ne rend pas vert) :
  `UHOLAIBrainSubsystem::GetStepIndex()` (`BlueprintPure`, int64, pas de la simulation, 0 sans) pour le journal.
  **Rouge attendu** : `Think` du script inchangé (compte et rend vrai) → messages attendus absents, Paysan IA `Idle` (cas 1 et
  2) ; les cas 3 et « humain inactif » passent déjà. Vraie logique dans le patch
  `C:\Users\baudo\AppData\Local\Temp\claude\c--Users-baudo-Documents-MyCrusader\9feacf6b-c420-452a-a0ea-a74517f4d9df\scratchpad\b2\b2_logic.patch`
  (`git apply --check` OK, `Script/AI/HOLAIBrain.as` seul) : `HOLAISummary::ReadAISummary` → plus bas stock parmi les
  ressources qui ont une source (égalité F W S G) → `HOLAIOrder::IssueAIGatherOrder(Slot, IdlePeasants, Sources[0])` ;
  journal `Log(n"LogHOLMatch", f"AI slot 1 step 40: 3 idle -> gather Food #12")` (« -> no source », « … refused ») ; rien
  sans inactif (pas de ligne : 4 pensées par seconde). Non vérifié par compilation : f-string et `n""` (préprocesseur AS vu),
  passage auto du `WorldContext` depuis le sous-système (relevé).
  **Risques une fois la règle active** (avant l'interrupteur B5) : tests C++ : aucun (le cerveau ne s'abonne qu'à son
  `OnWorldBeginPlay`, que les mondes de test n'appellent pas) ; `quick_check` : partie solo et serveur `net` lancés avec
  `StartingPeasants=0` (pas de Paysans de départ pour l'IA, 8p de `camera` compris) ; seul cas : **`victory` (régicide,
  l. 492)** `hol.Units.Spawn 1 -16600 0 1 Peasant 0` (Paysan de l'IA à 5,5 m du Roi, `wait 4`, puis `hol.Units.Remove -16600 0
  1500 1`) : le cerveau l'envoie récolter près de son centre, il peut sortir des 15 m avant le `Remove` et rester sur la carte
  (rapports `hol.HUD.Report` suivants, coups comptés). Scénarios d'essai (`play.ps1`) avec Paysans IA qui partiront récolter :
  `blocked_bodies`, `combat_off_dying`, `falls`, `peasant_fight` (`SpawnHere … Peasant 1`), `regicide.txt` (3 Paysans de
  l'IA), `hero_look` (`startingPeasants` 5) ; `starting_peasants` voulu (B6). À faire : compiler, voir le rouge, appliquer le
  patch, vert (le script se recompile au démarrage), `-Parts tests`, puis `-Parts victory` (régicide) ; B3-B6 ensuite.
- 2026-10-27 : IA0.4 B2 boucle « compiler, rouge, patch, vert » (sous-agent, Arwen) : compilation **PASS en 90 s**, sans
  correction. **Rouge vu, pour la bonne raison** (`AI.Brain.GatherIdle` seul, 87 s) : Paysan IA resté `Idle` aux cas 1, 3 et 2
  (l. 252, 262, 282 de `HOLAIBrainTests.cpp`), aucune ligne « Gather order of slot 1 … » ; les attentes « pas appliqué dans
  l'image de la pensée » et « Paysan humain inactif » déjà vertes. **Patch appliqué** (`git apply`, `Script/AI/HOLAIBrain.as`
  seul) : **vert** du premier coup (27 s), aucune `Angelscript: Error` (f-string, `n""` et `WorldContext` compilent). Journal :
  « AI slot 1 step 0: 1 idle -> gather Food #0 » puis « Gather order of slot 1: 1 units to source 0 (Food) », et au second
  monde « AI slot 1 step 0: 1 idle -> gather Wood #1 » / « … source 1 (Wood) » (pas 0 : le test pense avant tout pas).
  `quick_check -Parts tests` **PASS 224/224** (31 s). Non commité. Reste : `-Parts victory` (régicide, Paysan de l'IA envoyé
  récolter), puis commit, puis B3-B6.
- 2026-10-27 : B2 en vraie partie (sous-agent) : `-Parts victory,production,sides,gather` **PASS** (43 s), mais **faux vert
  silencieux dans le régicide** : `AI slot 1 step 9: 1 idle -> gather Food #326` (pas 760) → le Paysan IA posé au pas 758
  sort des 15 m avant `hol.Units.Remove` (pas 839 : « Remove: 0 units ») et récolte pendant riposte/refusal/death ; le cas
  « le Roi n'attaque pas un Paysan proche » n'est plus testé. Ailleurs : IA sans Paysan (`StartingPeasants=0`), aucune
  pensée. Aucune `Angelscript: Error`. **Décision** : l'interrupteur `"aiBrain"` (part de B5) avancé en **B2b**, commit
  commun B2 + B2b. B2b lancée en sous-agent (écrit sans compiler) : champ par emplacement, contrôle « Remove: 1 units »
  dans le régicide de `quick_check` (rouge sur la vraie chaîne), `"aiBrain": false` dans les scénarios hors IA.
- 2026-10-27 : IA0.4 B2b (sous-agent, Arwen, écrit sans compiler) : interrupteur `"aiBrain"` (IA0.4-Q3 A). **Dans l'arbre** :
  champ `FHOLMatchSlot::bAIBrain` (vrai par défaut, `Match/HOLMatchConfig.h`) ; `UHOLAIBrainSubsystem::GetThinkingSlots()`
  (`BlueprintPure`, la boucle du pas `OnStepEnded` passe désormais par elle) en **bouchon** (rend `GetAISlots()`) ; test
  **`HeroesOfLiberty.AI.Brain.Switch`** (`Tests/HOLAIBrainTests.cpp`) : `.json` par le vrai `MakeScenarioConfig` (emplacement
  1 `"aiBrain": false`, 2 sans le champ, 3 `"aiBrain": true`), `InitVictory` + vrai `SpawnAIPlayerStates` (3 PlayerStates) →
  `GetAISlots()` = 1,2,3 et `GetThinkingSlots()` = 2,3 ; défaut vrai (`MakeDefault`, emplacement ajouté par `Normalize`) ;
  aller-retour `ToJson`/`FromJson` de la config lue et d'un interrupteur posé en code ; contrôle **`quick_check`** (partie
  `victory`, section « victory regicide pass ») : « Remove: 1 units of slot 1 within 1500 of (-16600, 0) » exactement.
  **Rouges attendus** : C++ : « slot 1 "aiBrain": false, brain off », l'aller-retour de la config lue et « thinking slots: 2
  and 3 » (le convertisseur ne connaît que le nom `bAIBrain`, il ignore `"aiBrain"` ; le filtre est absent) ; l'aller-retour
  d'un interrupteur posé en code passe déjà (le convertisseur écrit et relit `bAIBrain`) ; `quick_check -Parts victory` :
  « the enemy Peasant still there after 4 s, removed (0 units) » (le cerveau l'envoie récolter, faux vert d'avant).
  **Vraie logique** dans
  `C:\Users\baudo\AppData\Local\Temp\claude\c--Users-baudo-Documents-MyCrusader\9feacf6b-c420-452a-a0ea-a74517f4d9df\scratchpad\b2b\b2b_logic.patch`
  (`git apply --check` OK sur Arwen avec les bouchons) : `FromJson` recopie `"aiBrain"` d'un emplacement en `bAIBrain` avant
  le convertisseur (boucle du pré-passage de `"difficulty"` réécrite, même comportement) ; `GetThinkingSlots` retire les
  emplacements dont `AHOLGameState::GetLaunchConfig().Slots[S].bAIBrain` est faux (la config que la victoire lit, posée par
  `InitVictory` côté serveur ; sans `AHOLGameState`, tous pensent) ; `ThinkNow` reste sans filtre (les tests forcent une
  pensée) ; `"aiBrain": false` dans le `.json` du régicide de `quick_check` (`victory_regicide.json`, l. 490, lu par
  `hol.UI.NewGame.Launch` → `FromJson` → écran Nouvelle partie qui garde la config) et dans les scénarios d'essai
  `blocked_bodies`, `combat_off_dying`, `falls`, `peasant_fight`, `regicide` (Paysans de l'IA posés par le scénario) et
  `hero_look` (5 Paysans de départ de l'IA, décor : faux pour garder les captures comme avant ; le seul discutable, une IA
  active y serait sans gêne). `starting_peasants` garde son IA. Les autres `.json` de `quick_check` avec IA (`victory` l. 409,
  `sides` l. 513) et les autres scénarios ne donnent aucun Paysan à l'IA : laissés tels quels. `ToJson` écrit `"bAIBrain"`
  (nom du convertisseur), relu tel quel. À faire : compiler, voir les deux rouges (`AI.Brain.Switch` seul, puis
  `-Parts victory`), appliquer le patch, verts, `-Parts tests`, puis commit commun B2 + B2b après accord ; B3-B6 ensuite.
- 2026-10-27 : IA0.4 B2b compilée et verte (sous-agent, Arwen, ~5 min de 22:02 à 22:07). Compilation de l'arbre aux
  bouchons **PASS** (53 s), aucune correction. **Rouge C++** `HeroesOfLiberty.AI.Brain.Switch` seul (24 s) : exactement les
  trois attendus (`HOLAIBrainTests.cpp` l. 317 « .json: slot 1 "aiBrain": false, brain off », l. 326 « round trip of the
  read configuration: slot 1 off, 2 and 3 on », l. 346 « thinking slots: 2 and 3 » reçu "1,2,3") ; l'aller-retour posé en
  code déjà vert. **Rouge `quick_check -Parts victory`** (29 s) : `victory regicide : FAIL` « the enemy Peasant still there
  after 4 s, removed (0 units) », sequence, victory et forum PASS. **Patch** `b2b_logic.patch` appliqué (`git apply` sans
  heurt), recompilation **PASS** (7 s). **Verts** : `AI.Brain.Switch` **Success** (22 s) ; `-Parts victory` **PASS** (27 s),
  journal : « Remove: 1 units of slot 1 within 1500 of (-16600, 0) », aucune ligne « AI slot 1 step … » (la seule récolte est
  celle du slot -1) ; `quick_check -Parts tests` **PASS 225/225** (33 s). Aucune `Angelscript: Error`, aucune correction.
  Journaux dans `...\scratchpad\b2b\` (red.log, green.log, qc_*.txt). Non commité. Reste : commit commun B2 + B2b après
  accord, puis B3-B6.
- 2026-10-27 : **B2 + B2b commitées** dans Arwen (b988cf0, principe A : commit sur la foi des tests). La lecture du `.json`
  ayant changé (`FromJson`), passe de contrôle en sous-agent : `-Parts sides,production,gather,build`. Ensuite B3 (règle en
  fonction pure + tests en script, champs du résumé `ScriptReadWrite`).
- 2026-10-27 : contrôle après b988cf0 (sous-agent) : `-Parts sides,production,gather,build` (+ camera, footprints) **PASS**
  en 112 s, aucun avertissement de lecture de config, aucune `Angelscript: Error`. Aucune pensée « -> gather » (IA sans
  Paysan, `StartingPeasants=0`) : la branche de récolte se verra en B6 (`starting_peasants`). B3 lancée en sous-agent
  (écrit sans compiler).
- 2026-10-27 : IA0.4 B3 (sous-agent, Arwen, écrit sans compiler) : la règle sortie de `Think` en fonction pure, testée en
  script (IA0.4-Q2 A). **Dans l'arbre** : (1) `AI/HOLAISummary.h` : spécificateur **`ScriptReadWrite`** ajouté à côté de
  `BlueprintReadOnly` sur `FHOLAISourceNear::SourceIndex`, `FHOLAIResourceSummary::Stock` et `Sources`,
  `FHOLAISummary::IdlePeasants`, `Food`, `Wood`, `Stone`, `Gold` (+ une phrase au commentaire d'en-tête) ; syntaxe lue dans
  le moteur : l'UHT Hazelight accepte le spécificateur nu (`UhtPropertyMemberSpecifiers.cs:29-33`, pose la métadonnée
  `ScriptReadWrite`, comme `meta = (ScriptReadWrite)`), exemple `AC/Private/Binds/UObjectInWorld.h:11`, et
  `Helper_PropertyBind.h:53-58` le teste avant `CPF_BlueprintReadOnly` (lecture + écriture en script, Blueprint inchangé) :
  **recompilation C++ nécessaire**. (2) `Script/AI/HOLAIEconomy.as` : struct de script `FHOLAIGatherChoice` (`bGather`,
  `Resource` 0-3 F W S G ou -1, `ResourceName`, `SourceIndex` ou -1, `Peasants`, `Nothing` = « no idle » / « no source »)
  et `HOLAIEconomy::ChooseGather(const FHOLAISummary&)` en **bouchon** (rend `Nothing = "not written"`, compile).
  (3) `Script/AI/HOLAIEconomy_Test.as` (module `*_Test`, aides sans préfixe `Test_`) : `Test_LowestStockChosen` (bois
  puis or le plus bas), `Test_TieInOrderFoodWoodStoneGold` (tout égal → Food ; bois = or → Wood ; pierre = or → Stone),
  `Test_LowestWithoutSourceSkipped` (bois le plus bas sans source → Stone), `Test_NoSourceNothing` (« no source »),
  `Test_NoIdleNothing` (« no idle »), `Test_AllIdleToFirstSource` (3, 7, 9 → source 21 = `Sources[0]` sur 21, 22, 23).
  `HOLAIBrain.as` (`Think`) **inchangé** dans l'arbre : `AI.Brain.GatherIdle` reste vert pendant la phase rouge.
  **Rouges attendus** : les 6 `Angelscript.UnitTests.AI.HOLAIEconomy_Test.Test_*` (bouchon : pas d'ordre ; les deux cas
  « rien » échouent sur la raison, « not written » au lieu de « no source » / « no idle ») ; tests C++ tous verts.
  **Vraie logique** dans
  `C:\Users\baudo\AppData\Local\Temp\claude\c--Users-baudo-Documents-MyCrusader\9feacf6b-c420-452a-a0ea-a74517f4d9df\scratchpad\b3\b3_logic.patch`
  (`git apply --check` OK sur Arwen avec le bouchon) : corps de `ChooseGather` (la règle de B2 telle quelle) et `Think`
  qui l'appelle (mêmes lignes de journal, même ordre `IssueAIGatherOrder(Slot, Choice.Peasants, Choice.SourceIndex)`, rien
  sans inactif). **Non vérifié faute de compilation** : `ScriptReadWrite` combiné à `BlueprintReadOnly` dans l'UHT ;
  écriture en chaîne `Summary.Food.Sources.Add(...)` / `Summary.Food.Stock = …` sur un struct C++ en script ; struct de
  script avec valeurs par défaut et `TArray<int>` ; `namespace` de script + import automatique vu d'un autre module (le
  serveur de débogage force `bAutomaticImports`, `AngelscriptDebugServer.cpp:1375`) ; signatures `FUnitTest`
  (`AssertEquals` int / FString, `AssertTrue/False`, lues dans `AC/Private/Testing/AngelscriptTest.cpp:185-233`) ;
  paramètre `Summary` inutilisé du bouchon (avertissement, pas une erreur). À faire : compiler, `quick_check -Parts tests`
  → 6 rouges script nommés, appliquer le patch, vert (tests script + `AI.Brain.GatherIdle`), puis B4-B6.
- 2026-10-27 : IA0.4 B3 compilée et verte (sous-agent, Arwen, ~10 min de 22:21 à 22:31). Compilation **PASS** (435 s, dont
  l'attente de la file de `build.ps1`) : l'UHT accepte `ScriptReadWrite` nu à côté de `BlueprintReadOnly`, pas besoin de
  `meta = (ScriptReadWrite)`. **Rouge `quick_check -Parts tests`** (34 s) : `tests 225/231 : FAIL`, exactement les 6
  `Angelscript.UnitTests.AI.HOLAIEconomy_Test.Test_*` rouges, pour la raison du bouchon (assertions « Expected true, got
  false » sur `bGather` ; `Test_NoIdleNothing` « Expected `no idle` Actual `not written` », `Test_NoSourceNothing`
  « Expected `no source` Actual `not written` ») ; les 225 autres verts. Le script compile du premier coup (struct de
  script, `TArray<int>`, `namespace`, écriture `Summary.Food.Sources.Add`, signatures `FUnitTest` : tout bon). À noter :
  le contrôle `scripts compiled` passe aussi KO pendant un rouge de script, car les assertions ratées s'écrivent
  `Angelscript: Error: Assertion failed …` (faux KO sans gravité, propre au rouge). **Patch** `b3_logic.patch` appliqué
  (`git apply` sans heurt, aucune correction, script seul : pas de recompilation). **Verts** : `-Parts tests` **PASS
  231/231** (34 s), les 6 tests en script **Success**, `AI.Brain.GatherIdle` **Success**, aucune `Angelscript: Error` ;
  `-Parts victory` **PASS** (29 s : victory, forum, regicide). Journaux dans `...\scratchpad\b3\` (qc_red.txt,
  red_tests.log, qc_green.txt, green_tests.log, qc_victory.txt). Non commité (Arwen : `HOLAIBrain.as`, `HOLAISummary.h`
  modifiés ; `HOLAIEconomy.as`, `HOLAIEconomy_Test.as` nouveaux). Reste : commit B3 après accord, puis B4-B6.
- 2026-10-27 : **B3 commitée** dans Arwen (5168def). Amélioration à noter : pendant
  un rouge de script, `scripts compiled` de `quick_check` tombe aussi (les `Angelscript: Error: Assertion failed` sont lues
  comme des erreurs de compilation) : faux KO limité au rouge, à corriger un jour dans `Get-ScriptErrors`. B4 (sources
  récoltables seulement) lancée en sous-agent (écrit sans compiler).
- 2026-10-27 : IA0.4 B4 (sous-agent, Arwen, écrit sans compiler). **Tests rouges écrits** (Arwen, non commités) :
  `HeroesOfLiberty.AI.Summary.GatherableOnly` (`Tests/HOLAISummaryTests.cpp`) : monde comme `AI.Summary.World`, centres
  finis des emplacements 0 (humain, x −10000) et 1 (IA, origine), baies (0, 2600), ferme de l'emplacement 0 à (1800, 0) et
  ferme de l'IA à (0, −3000), posées **par le vrai chemin** (`AHOLBuilding::Setup(Farm, …)` fini après
  `OnWorldBeginPlay` → champ ajouté au registre, propriétaire = la ferme, comme `Buildings.FarmField`) ; attendu :
  `ReadAISummary(1)` Food = « baies, champ de l'IA », champ ennemi absent ; `Read(0)` Food = « baies, champ de
  l'emplacement 0 », champ de l'IA absent (décalages sur les axes : l'ordre tient quelle que soit la taille du centre).
  `HeroesOfLiberty.AI.Brain.GatherableOnly` (`Tests/HOLAIBrainTests.cpp`, + include `AI/HOLAISummary.h`) : monde de
  `AI.Brain.GatherIdle` + ferme finie de l'emplacement 0 à (−1400, 0), plus près que les baies (0, 1800) ; résumé : baies
  en tête, champ ennemi absent ; `ThinkNow(1)` + `RunStep` → message « Gather order of slot 1: 1 units to source <baies>
  (Food) », Paysan IA en `GatherTrip`, Paysan humain inactif. **Rouge attendu** (arbre inchangé, `Make` garde toute source
  debout) : Summary.GatherableOnly échoue sur « champ ennemi laissé de côté » et les deux listes (IA : « ennemi baies
  IA » ; emplacement 0 : « baies IA ennemi ») ; Brain.GatherableOnly : le cerveau vise le champ ennemi (nourriture la plus
  proche), la récolte le refuse (« 0 units … 1 refused »), le Paysan reste inactif, message attendu absent. **Vraie
  logique** : `C:\Users\baudo\AppData\Local\Temp\claude\c--Users-baudo-Documents-MyCrusader\9feacf6b-c420-452a-a0ea-a74517f4d9df\scratchpad\b4\b4_logic.patch`
  (`git apply --check` OK sur Arwen) : dans `Make`, `continue` si `PlayerIndex` hors 0-255 ou
  `!Source.IsGatherableBy(uint8(PlayerIndex))`, commentaire mis à jour. Tests existants : aucun `AI.Summary.*` ni
  `AI.Brain.*` ne pose de source à propriétaire (`MakeSource` laisse `OwnerSlot = NoOwner`, sources de carte par
  `AddSource` sans propriétaire) → resteront verts ; les tests en script bâtissent leurs résumés sans `Make`. **Non
  vérifié faute de compilation** : compilation des deux tests ; le départ du Paysan IA vers son propre champ n'est pas
  testé (il va aux baies, plus proches) ; la ferme posée sans contrôle d'emprise près du centre (aucun contrôle dans `Setup`).
  À faire : compiler, `quick_check -Parts tests` → 2 rouges nommés, appliquer le patch, vert, puis commit B4 après accord,
  puis B5-B6.
- 2026-10-27 : IA0.4 B4 **rouge puis vert** (sous-agent, Arwen, ~8 min). Compilation des tests : PASS du premier coup
  (96 s), aucune correction. **Rouge** `quick_check -Parts tests` : FAIL 231/233, exactement les deux tests
  `GatherableOnly`, pour la bonne raison : `AI.Summary.GatherableOnly` → « champ ennemi laissé de côté » faux des deux
  côtés, listes IA « 1 0 2 » (attendu « 0 2 ») et emplacement 0 « 0 2 1 » (attendu « 0 1 ») ; `AI.Brain.GatherableOnly`
  → nourriture la plus proche = champ ennemi, « Gather order of slot 1: 0 units to source 2 (Food), 1 refused », Paysan IA
  pas en route. Patch `b4_logic.patch` appliqué tel quel (`HOLAISummary.cpp`, +8/−3), recompilé (61 s). **Vert** :
  `-Parts tests` **PASS 233/233** (34 s ; « Gather order of slot 1: 1 units to source 0 (Food) ») ; `-Parts
  victory,gather` **PASS** (43 s : victory, forum, regicide, camera 2p/8p, gather, footprints). Journaux dans
  `...\scratchpad\b4\` (build1.txt, qc_red.txt, red_tests.log, build2.txt, qc_green.txt, green_tests.log,
  qc_victory_gather.txt). Non commité (Arwen : `AI/HOLAISummary.cpp`, `Tests/HOLAIBrainTests.cpp`,
  `Tests/HOLAISummaryTests.cpp` modifiés). Reste : commit B4 après accord, puis B5-B6.
- 2026-10-27 : **B4 commitée** dans Arwen (4b5a3eb). B5 (cadence au vrai pas : pensées aux pas attendus, une
  par emplacement et par intervalle, étalées, aucune sur un client) lancée en sous-agent (écrit sans compiler).
- 2026-10-27 : IA0.4 B5 (sous-agent, Arwen, écrit sans compiler) : **arrêté au minuteur de 15 min** alors qu'il retouchait
  un commentaire, sans journal (entrée écrite par la session principale). Travail complet : test
  **`HeroesOfLiberty.AI.Brain.Cadence`** (`HOLAIBrainTests.cpp`, +205 lignes) : monde serveur prêt (GameState, simulation,
  Mass, cerveau), 3 emplacements IA par le vrai `.json` dont le 2 `"aiBrain": false`, 20 vrais pas
  (`UHOLSimulationSubsystem::RunStepsForTests`, nouveau, tests seulement, `RunStep` avec ses deux délégués, 0 sur un client)
  → emplacement 1 aux pas 4, 9, 14, 19, emplacement 3 aux pas 2, 7, 12, 17, 2 et humain jamais, aucun pas partagé, 8
  pensées ; `hol.AI.ThinkSteps` 10 → 20 pas de plus, moitié moins (29, 39 et 27, 37) ; monde client : aucun pas, aucune
  pensée, `ThinkNow` refusé. Côté C++ : `FHOLAIStepThought` et `GetStepThoughts()` (pensées faites au pas, bornées à 256)
  dans `HOLAIBrain.h`, commentaire de `ThinkSteps` (Q4 = A). **Bouchon** : la liste n'est jamais remplie → rouge sur les
  pas attendus. **Patch** `...\scratchpad\b5\b5_logic.patch` (remplit la liste dans `OnStepEnded` quand `ThinkNow` rend
  vrai ; `apply --check` OK). Aucun défaut de cadence signalé (rapport perdu à l'arrêt). Bilan du dépassement : la tâche
  « chercher des défauts » s'ajoutait à l'écriture du test ; la prochaine fois, une recherche de défauts = boucle à part.
  Vérification (compiler, rouge, patch, vert) lancée en sous-agent.
- 2026-10-27 : IA0.4 B5 **rouge puis vert** (sous-agent, Arwen, ~5 min). Compilation des tests : PASS du premier coup
  (39 s), aucune correction. **Rouge** `quick_check -Parts tests` : FAIL 233/234, seul `AI.Brain.Cadence`, pour la bonne
  raison : les quatre listes de pas vides (emplacement 1 attendu « 4,9,14,19 », emplacement 3 « 2,7,12,17 », puis avec
  `hol.AI.ThinkSteps` 10 « 4,9,14,19,29,39 » et « 2,7,12,17,27,37 ») ; tout le reste du test déjà vert (8 pensées
  comptées, monde client sans pas ni pensée, `ThinkNow` refusé). Patch `b5_logic.patch` appliqué tel quel
  (`HOLAIBrain.cpp`), recompilé (12 s). **Vert** : `-Parts tests` **PASS 234/234** (32 s) ; `-Parts victory,gather`
  **PASS** (33 s : victory, forum, regicide, camera 2p/8p, gather, footprints). Journaux dans `...\scratchpad\b5\`
  (build1.txt, qc_red.txt, red_tests.log, build2.txt, qc_green.txt, green_tests.log, qc_victory_gather.txt). Non commité
  (Arwen : `AI/HOLAIBrain.cpp`, `AI/HOLAIBrain.h`, `Simulation/HOLSimulationSubsystem.h`, `Tests/HOLAIBrainTests.cpp`
  modifiés). Reste : commit B5 après accord, puis B6.
- 2026-10-27 : **B5 commitée** dans Arwen (998cce4). Séance close par l'utilisateur (« la suite se fera dans une autre
  session »). **Reprise** (Arwen `ai-ia1`, `IN_USE.txt` posé, rien en cours, arbre propre) :
  - Commits d'Arwen pas encore dans `main` : e8cf110 (IA0.1 + B1), d7073d4 (fusion de `main` b5a2e82), d42b5a8 (nom
    « IA <Niveau> <N> »), b988cf0 (B2 + B2b), 5168def (B3), 4b5a3eb (B4), 998cce4 (B5). `main` a avancé depuis (NetPlaytest
    R8a dd1e708, peut-être d'autres) : fusionner `main` dans `ai-ia1` avant B6 (fermer tout jeu d'Arwen avant), recompiler
    Arwen, `quick_check -Parts tests` (234 attendus + ceux de `main`).
  - **B6** (vraie partie), en boucles séparées : (a) sous-agent : scénario `starting_peasants` hors écran (5 Paysans IA
    récoltent en moins de 1 s, lignes « AI slot 1 step S: 5 idle -> gather Food #N ») ; (b) sous-agent :
    `quick_check -Parts tests,victory,production,gather` puis `-Parts net` (aucune IA : rien ne bouge) ; (c) deux courses
    de même `Seed` : mêmes `Step hash` ; (d) essai de l'utilisateur, scénario prêt (`Tools/Scenarios/starting_peasants.txt`
    lancé par `play.ps1`), commit après validation.
  - Puis avancer `main` (demander d'abord aux sessions actives, `ListAgents` / `SendMessage`) et commit DOC dans la copie
    principale : `AIInProgress.md` + seulement les lignes IA d'`EtapeDefini.md` (repérées par leur texte, pas par numéro).
  - Ensuite IA0.5 (serveur dédié : le cerveau pense-t-il sur le serveur ?), IA0.7 (journal de décision), puis IA1.3-IA1.9.
  - Améliorations de processus notées : chercher des défauts = boucle à part ; le sous-agent écrit son journal dès le test
    écrit ; faux KO `scripts compiled` de `quick_check` pendant un rouge de script (`Get-ScriptErrors`, à corriger un jour).
- 2026-10-28 : reprise. **`main` (20e8f8a, 20 commits) fusionnée dans `ai-ia1`** (Arwen, f4b06dc). Conflits :
  `HOLSimulationSubsystem.h` (les deux aides de test gardées : `RunStepsForTests` de B5, `AdvanceStepIndexForTests` de
  `main`) et les fichiers de traduction (pris de `main`, puis collecte → `fill_po.py` (en/ja : 338 textes, 0 clé
  inconnue) → collecte ; clé `HOLGameScreen,AIName` présente). Compilation **PASS** (133 s), `quick_check -Parts tests`
  **PASS 268/268** (37 s). `main` a depuis avancé à 9364046 (deux scripts de test réseau seulement : pas besoin de
  refusionner avant B6). Suivant : B6 (a) scénario `starting_peasants` hors écran.
- 2026-10-28 : IA0.4 **B6 (a) rouge puis vert** (sous-agent, Arwen, ~8 min). `Tools/Scenarios/starting_peasants.txt` mis à
  jour (+17/−5, `.json` inchangé) : en-tête, point 5 et notes (les Paysans de l'IA récoltent d'eux-mêmes) ; vérifications
  ajoutées avant toute commande : `waitlog AI slot 1 step`, `5 idle -> gather`, `Gather order of slot 1: 5 units to
  source`, `hol.Gather.Report 0` → « not gathering 6 » (aucun Paysan humain commandé ; le Roi compte dans le rapport),
  `hol.Gather.Report 1` → « not gathering 1 » (le Roi seul), `GatherTrip` dans `hol.Units.Where 1`. Lancement hors écran
  (`-RenderOffScreen -nosound -unattended -saveddirsuffix=Tests`, `hol.Scenario.AutoContinue 1, hol.Scenario
  starting_peasants`) par `...\scratchpad\b6a\run_offscreen.ps1` (40c29ea2). **Rouge** (`"aiBrain": false` provisoire) :
  « Test sequence: timeout at step 9: waitlog AI slot 1 step ». **Vert** : « AI slot 1 step 4: 5 idle -> gather Food #326 »,
  « Gather order of slot 1: 5 units to source 326 (Food) » : **0,2 s de jeu** après la naissance ; double clic sur les
  Paysans IA en marche = « 5 units (other): Peasant x5 ». Non commité. Remarques : le Roi compte « not gathering » dans
  `hol.Gather.Report` (trompeur, sans effet) ; bruit `LogAutomationTest: Error` des tests du moteur au démarrage. Suivant :
  B6 (b) `quick_check -Parts tests,victory,production,gather` puis `-Parts net`.
- 2026-10-28 : IA0.4 **B6 (b) PASS** (session principale, Arwen + scénario de B6 (a) non commité) : `quick_check -Parts
  tests,victory,production,gather` (+ camera) **PASS** en 55 s (268/268, victory/forum/regicide, production, gather,
  footprints) ; `-Parts net` **PASS** en 191 s (net 2 clients, hero sheets, hero return). Suivant : B6 (c) deux courses de
  même `Seed`, mêmes `Step hash`.
- 2026-10-28 : IA0.4 **B6 (c) vert** (sous-agent, Arwen, sans C++). Graine fixée par `hol.Match.Open <json>` (copie de
  `starting_peasants.json` + `"version": 3, "seed": 4242` ; `hol.Scenario` remplace toujours la graine par l'horloge,
  `Dev/HOLScenario.cpp:101`), empreintes par `hol.Simulation.StepHashes 1` (position, orientation, vitesse, force, cible,
  action de chaque unité ; pas l'état de récolte lui-même, vu par le mouvement), `hol.Gather.LogDeposits 1`. Deux courses :
  **mêmes empreintes sur les 1015 pas communs (0-1014)**, même pensée « AI slot 1 step 4: 5 idle -> gather Food #326 »,
  10 dépôts du slot 1 chacune (premier au pas 355, même empreinte). Contre-épreuve `"aiBrain": false` (copie dans le
  scratchpad) : identiques aux pas 0-4, **divergence au pas 5** (juste après la pensée), 612/617 pas différents.
  Script et journaux : `...\scratchpad\b6c\` (40c29ea2 : run_det.ps1, sp_ai.json, sp_noai.json, det_a/b/noai.log). Idée
  notée : garder le `seed` du `.json` dans `hol.Scenario` quand il est présent (C++, pas fait). Suivant : B6 (d) essai de
  l'utilisateur (`play.ps1 starting_peasants -Root Arwen`), commit après validation.
- 2026-10-28 : **essai de l'utilisateur validé** (« tout est bon ») ; **B6 commitée** dans Arwen (154ac02) : **IA0.4 close**.
  `main` (6fe859f : R8c, scripts de test réseau et doc seulement) refusionnée dans `ai-ia1` (40108a6, sans conflit, pas de
  recompilation). Sessions actives (mycrusader-8a, -8b, -c1) prévenues avant l'avance rapide de `main` vers `ai-ia1`.
  Ensuite : IA0.5 (serveur dédié : le cerveau pense-t-il sur le serveur ?), IA0.7 (journal de décision), puis IA1.3-IA1.9.
- 2026-10-28 : **relevé d'IA0.5** (sous-agent, lecture seule, ~13 min) écrit plus haut (« IA0.5 Serveur dédié ») : rien
  ne bloque le cerveau sur serveur dédié, jamais vu tourner ; découpage B1-B3 (+B3b) ; questions IA0.5-Q1 à Q6 posées.
- 2026-10-28 : **IA0.5 tranchée** : Q6 = A (aucun essai de l'utilisateur, tout automatique), Q3 = A (passage serveur dans
  `packaged_smoke`, vert après un paquet complet dans IA0.5) par l'utilisateur ; Q1 = B (`netai`), Q2 = B
  (`"startingPeasants": 5` global), Q4 = B (pas de nouvelle cuisson de `net_match`), Q5 = A (salon à IA0.6) pris sur
  recommandation (questions techniques). `main` e4884fc (pause refusée en réseau, D361) amenée dans `ai-ia1` (avance
  rapide). B1 suivante.
- 2026-10-28 : IA0.5 **B1 rouge puis vert, commitée** dans Arwen (97b12da). Sous-agent (~12 min, écrit sans compiler) :
  partie **`aiserver`** de `quick_check` (`-Parts aiserver`, et `-Full`) : éditeur en `-server` sans client, Plain,
  `quick_aiserver.json` (slot 0 Human sans nom, slot 1 AI, `"startingPeasants": 5`), séquence `hol.Gather.Report 1`,
  `hol.Units.Where 1`, `quit` ; lignes de naissance, du cerveau et du premier ordre lues dans `quick_aiserver.log` après
  l'arrêt (le `waitlog` ne voit que les lignes écrites après son départ). Patch : ligne « AI brain <classe> awake, net mode
  …, thinking slots [..] » au **premier `OnStepEnded`** (à `OnWorldBeginPlay`, les PlayerStates IA de `StartPlay`
  n'existent pas encore), une par monde (`bAwakeLogged`). Session principale : compilation (fusion de `main`) **PASS**
  (499 s, file d'attente comprise). **Rouge** : `aiserver : FAIL`, seul « no awake line » ; **tout le reste déjà vert** :
  « AI slot 1 step 4: 5 idle -> gather Food #326 », « Gather order of slot 1: 5 units to source 326 (Food) », rapport
  « gathering 5 » : **le cerveau en script pense déjà sur un serveur dédié** (éditeur `-server`). Patch appliqué,
  recompilé (122 s) ; **vert** : `-Parts aiserver,tests` **PASS** (69 s, 269/269), « AI brain HOLAIBrain awake, net mode
  DedicatedServer, thinking slots [1] ». Suivant : B2 (`netai` : serveur + Alice et Bob, IA dans un 3e emplacement).
- 2026-10-28 : IA0.5 **B2 rouge puis vert, commitée** dans Arwen (0758f87). Sous-agent (~5 min) : partie **`netai`** de
  `quick_check` (`-Parts netai`, et `-Full`) : serveur `-server` sur Plain + Alice (slot 0) et Bob (slot 1), IA au slot 2
  (place à 270°), `"startingPeasants": 5` ; fonctions communes avec `aiserver` (`Get-AITestPort`, `Write-AIMatchConfig`) ;
  clients : `hol.Units.Where 2` après 8 s ; serveur : lignes de B1 pour le slot 2 et aucun « Gather order of slot 0/1 ».
  **Rouge** (slot 2 `Closed`, 45 s) : seules les lignes de l'IA KO (serveur et deux clients). **Vert** (slot 2 `AI`) :
  premier passage FAIL par le test lui-même : 6 unités du slot 2 vues par chaque client, 5 Paysans « Gathering » + **le
  héros de départ** inactif (`ShouldSpawnHero`, le cerveau ne commande que les Paysans) ; attente corrigée (les 5 Paysans
  au travail, `$AIBusy -eq 5`, au lieu d'exactement 5 unités). `-Parts netai,tests` **PASS** (79 s, 269/269),
  `aiserver` PASS. Suivant : B3 (passage serveur dans `packaged_smoke.ps1`, rouge sur le paquet 2026-10-27-e36efe9, vert
  après un paquet complet, machine calme, sessions prévenues).
- 2026-10-28 : IA0.5 **B3 rouge** (sous-agent, ~12 min, non commité) : passage serveur dans `Tools/Tests/packaged_smoke.ps1`
  (+92/−10) : `HeroesOfLibertyServer.exe` du paquet, `-MatchConfig` comme `aiserver`, lecture du journal jusqu'à « Gather
  order of slot 1 » (30 s au plus, `-ServerTimeoutSeconds`), arrêt par PID ; vérifications : écoute, « Using fully
  precompiled scripts », « awake … DedicatedServer, thinking slots [1] », « AI slot 1 step », « 5 idle -> gather »,
  « Gather order of slot 1: 5 units », aucune erreur de fichier de script (les 60 `Angelscript: Error` de liaisons du moteur
  au démarrage sont connues, `script_errors.ps1`), aucune ligne fatale ; `-SkipServer`, `-ServerOnly`. Aucun paquet dans
  `Saved\Packages` : le serveur publié R8 (`MyCrusader\Saved\Manager\Servers\2026-10-27-r8`, d'avant IA0.4) copié dans
  `Arwen\Saved\Packages\2026-10-27-r8\WindowsServer\` (588 Mo, à supprimer après le vrai paquet). **Rouge**
  (`-ServerOnly`, 36 s) : seules les quatre lignes du cerveau manquent ; « Version: 2026-10-27-r8 », « Using fully
  precompiled scripts », « listening on port 7820 », 5 Paysans de départ pour les slots 0 et 1 : verts. Suivant : paquet
  complet depuis Arwen (machine calme, sessions prévenues), puis `packaged_smoke.ps1` sans paramètre.
- 2026-10-28 : paquet complet de B3 en attente : 8b (Glorfindel, recuisson + passes lobby, ~15 min) préviendra ; 8a (Héros)
  et c1 d'accord. Info de 8a : dans Celeborn (pas encore dans `main`), `HeroesOfLibertyUI` passe de `ClientOnly` à
  `Runtime` dans le `.uproject` (scripts d'interface sur le serveur dédié) : à sa fusion, la cible Server liera l'interface
  et il faudra recuire le serveur ; refaire alors `packaged_smoke.ps1` (passage serveur de B3).
- 2026-10-28 : c1 (étape O, Haldir) demande aussi d'attendre (O.5b : tests et building_crowd, 30-60 min, puis essai de
  l'utilisateur) et préviendra ; paquet de B3 lancé quand 8b et c1 auront fini. En attendant : relevé d'IA0.6 (menu)
  lancé en sous-agent, lecture seule.
- 2026-10-28 : **relevé d'IA0.6** (sous-agent, lecture seule) écrit plus haut (« IA0.6 Menu ») : solo presque prêt (IA grisée
  au menu seulement), salon à construire (gestionnaire C#, client, écran) ; découpage S1-S3, L1-L5, C ; questions Q1-Q7.
- 2026-10-28 : **IA0.6 tranchée** par l'utilisateur : **Q1** = A revue : colonnes « Joueur » et « État » **fusionnées** en
  une liste qui montre le nom du joueur en partie (« Vous », « IA Normal 1 », « Joueur 2 », « Fermé » ; déroulé Fermé /
  Humain / IA), plus une liste « Niveau » grisée sur Normal visible sur les lignes IA ; **Q3** = A (une IA compte dans
  « au moins 2 joueurs ») ; **Q4** = A (le chef règle couleur et équipe de l'IA, niveau grisé). Pris sur recommandation :
  Q2 = A (pas de partie solo sans humain), Q5 = A (« IA Normal N » au salon), Q6 = A (ajout/retrait d'IA remet les Prêt à
  zéro), Q7 = A (« Humain » gardé). Découpage : S1 IA ouverte, S2 colonne fusionnée + Niveau, S3 menu → partie, puis
  L1-L5, C. 8b a fini ; paquet de B3 attend la fin des tests de c1 (30-45 min). S1 lancée en sous-agent.
- 2026-10-28 : IA0.6 **S1 rouge puis vert, commitée** dans Arwen (95b3009). Sous-agent (~2 min, écrit sans compiler) :
  marque `menu ai` à la fin du bloc fr de la partie `menu` (`hol.UI.Choose SlotState1 2`, `hol.UI.Shown SlotColor1
  SlotTeam1`, capture `qc_fr_ai`, emplacement refermé). **Rouge** : `menu fr : FAIL`, seul « AI open in solo » (option
  grisée ignorée). Patch : `SetOptionEnabled(AI, false)` retiré (le salon remplace ses valeurs par `LobbyStateNames`,
  aucune IA n'y arrive), commentaire NG:938, note « Grisé : pas encore dans le jeu (factions, mode et options). » (+ en/ja
  dans `fill_po.py`). Compilation PASS (266 s, file d'attente), collecte → `fill_po.py` (0 clé inconnue) → collecte ;
  **vert** : `-Parts menu,tests` PASS (fr/en/ja, 269/269, 37 s). **Paquet complet de B3 lancé** depuis Arwen (c1, 8b, 8a
  prévenus). Suivant : `packaged_smoke.ps1` (client + serveur) sur ce paquet ; S2 (colonne fusionnée + Niveau).
- 2026-10-28 : **paquet de B3 : FAIL sans avoir commencé** (« still waiting for the build of another project after 60
  min ») : ProjectShiva (autre projet de l'utilisateur, même moteur AS_3, sessions « playable-* ») compile en continu
  depuis 11:41 (plusieurs UnrealBuildTool + cl.exe) et tient la file d'UBT. Rien tué (pas notre projet). Paquet relancé
  (attend la file). Pour ne pas mêler des sources à moitié écrites au paquet, S2 est écrite **en patchs hors de l'arbre**
  (test + logique), appliqués après le paquet.
- 2026-10-28 : IA0.6 **S2 écrite en patchs** (sous-agent, ~7 min, Arwen intact) dans `...\scratchpad\ia06s2\` (40c29ea2) :
  `s2_test.patch` (commande de dev `hol.UI.Text <Widget>` = texte affiché d'une liste ou étiquette,
  `UHOLDropdown::GetShownText`, `HOLUIOpen.cpp` ; marque `menu ai` étendue : `SlotState1` montre « IA Normal 2 »,
  `SlotLevel1` « Normal » montrée et grisée, `SlotName1` absente, `SlotLevel2` cachée sur une ligne fermée) ;
  `s2_logic.patch` (`UHOLDropdown::SetShownText` ; listes `SlotLevel<N>` 140 px grisées sur Normal, libellés de
  `GetDifficultyName` ; en solo la liste d'état montre `GetAIName` / « Joueur N » / « Fermé », colonne `SlotName` et
  `HeadName` repliées, en-tête « Joueur » ; salon inchangé (noms et « Emplacement », niveau replié) ; clé
  `HOLNewGameScreen,Level` en/ja). **Nom** : `GetAIName` numérote par emplacement (« IA {niveau} {Slot+1} », IA0.1-Q1) :
  l'IA de `SlotState1` = « IA Normal 2 », même nom qu'en partie. Ordre : test → compiler → rouge → logique → `fill_po.py`
  + collecte → compiler → `-Parts menu,lobby` (+ `manager_client`). En attente : le paquet relancé attend toujours la file
  d'UBT (ProjectShiva, 14:06) ; les patchs s'appliqueront après lui.
- 2026-10-28 : 8a a avancé `main` à 7bd73ab (J8a.5) : **`HeroesOfLibertyUI` passe en `Runtime`** (la cible Server compile
  des scripts d'interface `Script/UI/*.as`) : le paquet serveur doit en tenir compte. Paquet en file arrêté (pas commencé),
  `packaged_smoke.ps1` (B3, non commité) mis de côté, **`main` fusionnée dans `ai-ia1`** (ccd5187 ; seuls les fichiers de
  traduction en conflit : pris de `main`, collecte à refaire après le paquet, l'éditeur devant être recompilé), B3 remis,
  **paquet relancé** (file d'UBT toujours tenue par ProjectShiva). Relance automatique à 15:08 puis toutes les 45 min
  (demande de l'utilisateur). Après le paquet : `packaged_smoke`, compilation de l'éditeur, collecte, `quick_check -Parts
  tests,menu,aiserver,netai`, puis S2.
- 2026-10-28 15:08 : relance automatique : le paquet **a démarré** (file d'UBT libérée ; `RunUAT BuildCookRun`, version
  `2026-10-28-ccd5187`). Rien relancé ; passage de secours posé à 15:53.
- 2026-10-28 : **paquet PASS** (`2026-10-28-ccd5187`, Arwen avec `main` J8a.5, 18,6 min dont 746 s de file d'UBT ;
  `precompile_scripts` PASS, 2 cibles). **IA0.5 B3 vert, commitée** (951b02d) : `packaged_smoke.ps1` **PASS** (client 20 s ;
  passage serveur ok en 9 s) ; journal du serveur empaqueté : « Version: 2026-10-28-ccd5187 », « Using fully precompiled
  scripts », « AI brain HOLAIBrain awake, net mode DedicatedServer, thinking slots [1] », « AI slot 1 step 4: 5 idle ->
  gather Food #326 », « Gather order of slot 1: 5 units to source 326 (Food) » : **la classe de script s'active en mode
  précompilé, le cerveau pense dans le serveur du paquet** (aucun repli C++ nécessaire). Copie temporaire du serveur R8
  supprimée. Relance de 15:53 annulée. **IA0.5 : B1-B3 faites** ; reste : contrôles de la fusion ccd5187 (compilation de
  l'éditeur, collecte, `-Parts tests,menu,aiserver,netai`, en cours), puis avance de `main` et S2 d'IA0.6.
- 2026-10-28 : contrôles de la fusion ccd5187 : éditeur PASS, collecte → `fill_po.py` (345 textes, 0 clé inconnue) →
  collecte, `-Parts tests,menu,aiserver,netai` **PASS** (97 s, 285/285) ; traductions commitées. **IA0.5 close dans Arwen**
  (aucun essai : Q6 = A). `main` **pas encore avancée** : Arwen contient aussi IA0.6 S1 (option IA ouverte au menu), pas
  encore essayée par l'utilisateur (absent) : lui demander à son retour (avancer `main` avec S1 ou attendre la clôture
  d'IA0.6). S2 : patch de test appliqué, compilation et rouge en cours.
- 2026-10-28 : IA0.6 S2 **rouge vu** (compilation du patch de test PASS 66 s ; `menu fr : FAIL`, seul « AI line merged » :
  nom « IA » au lieu de « IA Normal 2 », niveau absent, colonne de nom encore là). Patch de logique appliqué, puis **pause
  demandée par l'utilisateur** : chaîne de vérification arrêtée avant la fin de la compilation (aucun processus d'Arwen
  restant). **Reprise** : Arwen = d3d845d + S2 (test + logique) non commités ; faire : `build.ps1 -Root Arwen`, collecte →
  `fill_po.py` → collecte, `quick_check -Parts menu,tests`, `manager_client.ps1 -Case Leader`, regarder la capture
  `qc_fr_ai` (largeur, « Évolutive » dans 140 px), commit S2. Puis demander à l'utilisateur : avancer `main` maintenant
  (IA0.5 close + IA0.6 S1-S2) ou à la clôture d'IA0.6 ; puis S3, L1-L5.
- 2026-10-28 : reprise après la pause. IA0.6 **S2 verte, commitée** (fb134e0) : compilation PASS (89 s), collecte (346
  textes, 0 clé inconnue) ; premier passage `menu fr` FAIL par le test seul (ligne fermée « Shown SlotLevel2: no, greyed » :
  caché et grisé, le test attendait « no » exactement ; motif corrigé en `no(, greyed)?$`) ; puis `-Parts menu` PASS
  (fr/en/ja), `tests` 285/285, `manager_client.ps1 -Case Leader` PASS (salon inchangé : « slots c0r c1r x o o o o o »).
  Capture `qc_fr_ai` relue : colonne « Joueur » fusionnée (Vous, « IA Normal 2 », Fermé…), « Niveau » grisé sur Normal sur
  la seule ligne IA, largeur tenue. Utilisateur : **avancer `main` après un essai rapide** (IA0.5 + IA0.6 S1-S2).
  Scénario d'essai **`ai_menu`** (`.txt` + `.json`, non commités) : partie contre une IA lancée par l'écran Nouvelle partie,
  vérifie « awake », « AI slot 1 step », « Gather order of slot 1: 5 units », notes vers le menu ; hors écran : prêt en
  34 s. Remarque : le cerveau écrit aussi « awake, net mode Standalone, thinking slots [] » sur la carte du menu principal
  (sans effet ; à filtrer un jour : ne rien écrire sans emplacement pensant ou hors carte de jeu).
- 2026-10-28 : **essai `ai_menu` validé** par l'utilisateur (« tout est bon ») ; scénario commité (d30129f) ; **`main`
  avancée** en avance rapide à d30129f (IA0.5 close : B1-B3 ; IA0.6 S1-S2 ; fusion J8a.5). Séance arrêtée par
  l'utilisateur (« commit tout et arrête-toi »). **Reprise** (Arwen `ai-ia1` = `main` d30129f, arbre propre) : IA0.6 S3
  (menu → partie : IA choisie à la liste, `hol.UI.NewGame.Launch -` → « thinking slots [1] », « idle -> gather »), puis
  salon L1-L5 (gestionnaire C#, client, écran, bout en bout après recuisson du serveur), clôture C (GDD, essai) ; puis
  IA0.7. Remarque en attente : ligne « awake … thinking slots [] » sur la carte du menu principal (à filtrer).
- 2026-10-29 : reprise. Arwen `ai-ia1` avancée à `main` e6f1467 (moteur « UE 5.8 HOL », `.uproject` remis avant la
  fusion). IA0.6 **S3 test écrit** (`quick_check.ps1`, fin de la partie `menu`) : marque `menu ailaunch` (Nouvelle partie,
  `hol.UI.Choose SlotState1 2`, `hol.UI.NewGame.Launch - 0` sans fichier, `waitlog AI slot 1 step`, retour au menu
  principal) ; contrôles : « value 2 chosen », « Match on …: given configuration, 2 occupied slots », « thinking slots
  [1] », « AI slot 1 step N: 5 idle -> gather ». Attendu : déjà vert (S1 laisse passer l'IA, `Launch` la donne telle
  quelle), donc S3 sans code si c'est le cas. Compilation de l'éditeur sur le nouveau moteur en cours.
- 2026-10-29 : IA0.6 **S3 verte, commitée** (c2b8f3f, test seul, aucun code de jeu). Premier passage : le jeu n'a pas
  démarré à temps (premier lancement sur « UE 5.8 HOL » : 97 s d'attente du serveur Zen, cache neuf) ; deuxième : **rouge**
  « 5 Peasants sent to gather (0) » (le cerveau pense pour l'emplacement 1, mais population 0/0) : quick_check lance tout le
  jeu avec `StartingPeasants=0` (`$NoStartingPeasants`), la règle du jeu (5) n'est pas en cause. Le test fait ses 5 Paysans
  IA (`waitlog Main centre of slot 1`, `hol.Units.Spawn 5 0 0 1 Peasant`) → `-Parts menu` **PASS** (29 s, 4 rapports dont
  `menu ailaunch`).
- 2026-10-29 : IA0.6 **L1 verte, commitée** (47263bf ; sous-agent ~6 min + reprise). Gestionnaire :
  `SlotState.AI`, `LobbyAI` (couleur, équipe, faction, niveau Normal), `SetSlotAI` (`PUT slots/{i}` `{"state":"ai",
  "colorIndex"?, "team"?}`, chef seul ; nouvelle IA = première couleur libre, équipe 0 ; même requête = changer couleur ou
  équipe) ; vue `{"state":"ai","level":"Normal","colorIndex","team","faction"}` ; **nom fait par le client** (« IA Normal
  N », texte traduit) ; couleurs des IA prises (`color_taken`, joueur qui entre) ; ajout/retrait/changement remet les Prêt
  (Q6) ; `PROTOCOL.md`. **Taille réduite** : le sous-agent retirait une IA au-delà des places alors que la taille
  l'acceptait (joueurs + IA ≤ places) ; corrigé comme l'écran solo (`FitSlotsToSize`) : l'IA **remonte** comme un joueur,
  avec couleur et équipe (test `ASmallerSizeMovesAnAIPastItsPlacesBelowAsAPlayer` rouge puis vert). Un joueur qui rejoint
  ne prend jamais un emplacement IA (état Open seulement). Tests : HOLManager 291/291, Launcher 141, Update 27, Publish 33.
  Notes pour la suite : `GET /lobbies` compte `players` en humains, `maxPlayers` avec les IA (à voir à l'affichage, L4) ;
  tant que L2 n'est pas faite, une IA part en `"Closed"` dans `match.json` ; après une partie, les IA restent au salon.
  Suivant : L2 (lancement : `FromLobby` `"AI"`, niveau, couleurs, départ avec une IA comptée, Q3 = A).
- 2026-10-29 : L2 (lancement du salon avec IA : `FromLobby` `"AI"`, couleurs, départ Q3 = A, référence
  `match_v3_ai_reference.json` + test C++) et L3 (client C++ : `EHOLLobbySlotState::AI`, lecture, `MakeSlotBody`,
  `CanStart`) lancées en parallèle en sous-agents (fichiers séparés ; C++ écrit sans compiler, compilation commune ensuite).
- 2026-10-29 : **L2 revenue** (sous-agent ~5 min, non commitée, attend la compilation commune avec L3). `FromLobby` :
  emplacement IA → `{"state":"AI","faction","colorIndex","team","playerName":"","difficulty":"Normal"}` (`difficulty`
  écrit seulement pour une IA : un salon sans IA écrit le même fichier, `match_v3_reference.json` inchangé, `version` 3) ;
  « Aléatoire » d'une IA tiré de la graine comme un joueur ; `Start` : joueurs + IA ≥ 2 (Q3 = A), Prêt des humains
  seulement ; IA gardées au salon après la partie. Rouge vu (3 sur 6 : `not_enough_players`, IA écrites `Closed`,
  référence) puis vert : HOLManager 297/297 (+ 141, 33, 27). Référence `Data/match_v3_ai_reference.json` (Bob 0, Alice 2,
  équipe 1 ; IA 1 couleur 6, IA 3 couleur 4, équipe 2) ; test C++ `HeroesOfLiberty.MapSize.Lobby.ManagerAIReference`
  (lit la référence, IA, couleurs, équipe, niveau Normal, `bAIBrain` ; même fichier en « Hard » lu Hard), non compilé.
- 2026-10-29 : **L3 revenue** (sous-agent ~4 min) : `EHOLLobbySlotState::AI` (à la fin), `FHOLLobbySlot::Level`, lecture
  `"ai"` (niveau par nom, Normal par défaut ; couleur et équipe exigées comme pour un joueur), `GetNumAIs`,
  `CanEditAISlot` (chef, emplacement IA), `MakeAISlotBody(Couleur?, Équipe?)`, `UHOLManagerClient::SetSlotAI`, `CanStart`
  = joueurs + IA ≥ 2 et humains prêts ; test `HeroesOfLiberty.Net.ManagerClient.AI`. Écran inchangé (L4 : conversion du
  salon en config où une IA devient Closed, NG ≈535-547 ; lignes « Joueur N »/« Fermé » pour une IA, ≈559-568 ; journal
  `slots` où une IA sort en `c<couleur>`, ≈623-629). **L2 + L3 vertes, commitées** (b61fe35, fed5aad) : compilation
  commune PASS du premier coup (64 s ; le C++ des deux, écrit sans compiler, sert de rouge : appels à des fonctions
  absentes) ; `-Parts tests` **287/287** (dont `MapSize.Lobby.ManagerAIReference` et `Net.ManagerClient.AI`) ;
  `manager_client.ps1 -Case Leader` PASS (salon inchangé). Suivant : L4 (écran du salon : « IA » du chef, nom « IA Normal
  N », couleur/équipe, « a » dans la ligne `slots`, `manager_client.ps1 -Case AI`).
- 2026-10-29 : IA0.6 **L4 verte, commitée** (f6657d4 ; sous-agent ~12 min). Écran du salon : 5e choix « IA » (valeur 4) de
  la liste d'état, conversion salon → config avec l'IA (état, couleur, équipe, faction, niveau), « IA Normal N » dans la
  liste d'état, colonne Joueur vide sur une ligne IA, couleur/équipe actives pour le chef seul (`CanEditAISlot` →
  `SetSlotAI`), colonne Niveau (code du solo) montrée au salon seulement s'il y a une IA, `a<couleur>` dans la ligne
  `slots`. **Rouge** : « no 'Lobby: slot 2 ai sent' » ; second rouge hors code : le `Saved\Manager\HOLManager.exe` d'Arwen
  datait d'avant L1 (republié par `build_manager.ps1`). **Vert** : `manager_client -Case AI` PASS (IA Normal 3, niveau
  grisé, Prêt remis à zéro, couleur 5 et équipe 2 au gestionnaire, retrait par Fermé, Lancer → 500 sans serveur),
  `-Case Leader` PASS (« slots c0r c1r x o o o o o » inchangé), `-Parts menu,tests` PASS (287/287). Capture
  `manager_client_ai.png` relue : lisible. Aucun texte nouveau. Points ouverts (choix du sous-agent, gardés) : colonne
  Joueur vide sur une ligne IA (les lignes libres disent « Joueur N ») ; colonne Niveau qui apparaît avec la première IA ;
  pas de cas « non-chef voit une IA ». Suivant : L5 (bout en bout, serveur recuit, machine calme), puis clôture C.
- 2026-10-29 10:29 : **L5 en attente d'une décision** (IA0.6-Q8) : aucun serveur cuit dans Arwen, et « UE 5.8 HOL » n'a
  jamais cuit le serveur (cache neuf : long) ; `net_match.ps1 -Stage` tient le verrou d'UAT (les lancements des autres
  sessions attendent) ; Celeborn lance le jeu en ce moment. Options : A) cuire maintenant ; B) plus tard, machine calme ;
  C) sauter L5 (tout le reste est testé) et clore IA0.6 par l'essai. Reco B, C acceptable. Rappel à 10:44.
- 2026-10-29 : **IA0.6-Q8 tranchée** par l'utilisateur : L5 **sans serveur cuit** : cas `lobby_match.ps1 -AI` sur le
  modèle de `-Rejoin` (l'éditeur d'Arwen comme serveur de partie, `.cmd` lancé par le gestionnaire, pas de verrou d'UAT) ;
  la cuisson « pour tout le monde » se fera dans une autre session (un serveur cuit depuis `main` ≥ e6f1467 suffirait : le
  code serveur d'Arwen = `main`). L5 lancée en sous-agent.
- 2026-10-29 ~11:30 : la session d'Elrond (mycrusader-7a) signale que son `lobby_match` a croisé celui de L5 entre 11:28
  et 11:31 (port 7790 pris par le gestionnaire d'Arwen, course à la sonde de port : ses Alice/Bob sont entrés dans le salon
  d'Arwen) ; elle ne lance plus de test de salon pendant L5. Sous-agent prévenu (relancer un échec de cette fenêtre).
  Un serveur dédié cuit depuis `main` 8ea1b8e (UE 5.8 HOL, Version.txt = dev) est désormais dans
  `Saved\StagedBuilds\WindowsServer` d'Arwen (et des autres copies). Point ouvert : sonde de port de `lobby_match` en
  course entre copies.
- 2026-10-29 : IA0.6 **L5 verte, commitée** (d1844e7 ; sous-agent ~10 min ; test seul, aucun code). `lobby_match.ps1 -AI` :
  l'éditeur d'Arwen comme serveur de partie (même `.cmd` que `-Rejoin`) ; Alice seule + une IA sur Petite (2 places, Q3 =
  A), couleur 2, IA par `hol.UI.Choose SlotState1 4` (« IA Normal 2 »), Prête, Lancer, Plain_Small ; `hol.Units.Where 1`
  côté client : 5 Paysans de l'IA en Gathering ; abandon, bannière Défaite, serveur arrêté seul. **Vert direct** (L1-L4
  suffisaient) ; 1er passage : motif de mon test faux (`slots c2r a0 x …`) ; 2e : FAIL « 1 server(s) of this copy still
  running » par la collision avec Elrond (une « Carol » d'une autre copie entrée sur le port UDP 7900) ; 3e : **PASS 59 s**.
  Serveur : « Starting Peasants of slot 1: 5 », « thinking slots [1] », « AI slot 1 step 4: 5 idle -> gather Food #86 »,
  « Gather order of slot 1: 5 units », « Victory: match over, winners 1 », « Server quits: empty after the match » ;
  `match.json` : emplacement 1 `"AI"`, `"difficulty": "Normal"`. Client : « Players table: Vous, IA Normal 2 ». Captures
  `Saved_Tests_Alice\...\lobby_match_ai_*.png` (partie relue : forum, arbres, Paysans, tableau ; le « champ vide » signalé
  par le sous-agent n'y est pas). `-Rejoin` PASS (90 s). **Points ouverts** : ports entre copies (sonde 7790 du
  gestionnaire en course ; serveurs de toutes les copies depuis UDP 7900) → plages par copie comme `Get-AITestPort`.
  **IA0.6 : S1-S3, L1-L5 faites** ; reste la clôture C (GDD, scénario d'essai, essai de l'utilisateur, `main`).
- 2026-10-29 : clôture C : **essai de salon prêt** (9f7530f) : `Tools\Scenarios\ai_lobby.ps1` (pas de scénario
  `play.ps1` pour un salon) : gestionnaire d'Arwen sur 7850+ (hors des ports des scripts de test), serveur de partie =
  éditeur d'Arwen (`.cmd` comme `lobby_match -AI`, UDP 7960+), jeu en fenêtre « Chef » branché dessus ; tout arrêté à la
  fermeture du jeu. Gestionnaire vérifié hors écran (`/status` : gameVersion dev). Essai demandé à l'utilisateur ; après
  validation : décisions IA0.6 au GDD (D515+), avance de `main`.
- 2026-10-29 : **essai `ai_lobby` de l'utilisateur : bug** : salon, IA, lancement : l'IA joue, mais l'utilisateur (renommé
  « miaou » sur l'écran Multijoueur, jeu lancé avec `-PlayerName=Chef` par le script) reste « en attente de miaou » puis
  « connexion perdue », sans pouvoir jouer. Serveur : « Match waits for: miaou », « Login request: ?Name=Chef », « Chef
  joined as a spectator ». **Cause** (bug réseau latent, pas seulement du script) : `UHOLLocalPlayer::GetNickname` rend
  `-PlayerName=`, qui remplace le `?Name=` de l'adresse de la partie ; le salon, lui, prend le nom de la case. Les
  `Joueur_<Nom>.bat` du paquet d'essai réseau passent aussi `-PlayerName=` : un ami renommé à l'écran serait spectateur.
  **Rouge** : `lobby_match -AI` où Alice (`-PlayerName=Alice`) tape « Alicia » (nouvelle commande de dev `hol.UI.Type
  <Case> <Texte>`) → FAIL (tableau « Alicia, IA Normal 2 », spectatrice). **Correction** : `UHOLLocalPlayer::SetChosenName`,
  appelé par `TakePlayerName` de l'écran Multijoueur ; `GetNickname` le rend avant `-PlayerName=`. **Vert** : `-AI` PASS
  (« Alicia holds slot 0 », 59 s), `-Rejoin` PASS, `lobby_match` par défaut (serveur cuit de main 8ea1b8e) PASS,
  `manager_client` Leader et AI PASS, `quick_check -Parts tests,menu,net,aiserver,netai` PASS (287/287). Commit d4ad92b.
  Essai à refaire par l'utilisateur.
- 2026-10-29 : **2e essai `ai_lobby` validé** par l'utilisateur (« c'est bon »). Copie égarée dans la copie principale :
  `Tools/Tests/lobby_match.ps1` avec une partie des lignes de L5 (le sous-agent avait écrit une fois en chemin relatif),
  identique au commit d1844e7 : retirée (`git checkout --`). `main` (8ea1b8e, partie de test « hero ») fusionnée dans
  `ai-ia1` (1fcf4d3, sans conflit) ; `-Parts menu,hero` PASS ; **`main` avancée** en avance rapide à 1fcf4d3. GDD : **D515**
  (l'IA dans « Nouvelle partie » : Q1, Q2, Q7) et **D516** (l'IA au salon : Q3-Q6, taille réduite, essai), renvoi dans le
  paragraphe IA du § 14.4. **IA0.6 close.** Prochain : IA0.7 (vérification : IA contre IA sans fenêtre, aucun Paysan IA
  inactif plus de 10 s pendant 2 min, mêmes `StepHashes` à même graine, vitesse ; essai `ai_economy_start`). Points ouverts
  hors IA : ports des tests de salon entre copies ; ligne « awake … thinking slots [] » sur la carte du menu principal.
- 2026-10-29 : **IA0.7 en cours** (Arwen, `ai-ia1`, commit 53f3323, pas dans `main`). Partie `quick_check -Parts aiduel`
  (~85 s pour deux courses) : serveur éditeur `-nullrhi -UseFixedTimeStep -FPS=20`, deux IA (Plain), 120 s de jeu,
  `hol.Gather.Report` toutes les 2 s, `StepHashes 1`, deux courses. Résultats : **aucun Paysan IA inactif** (au-delà du
  héros, 1 « not gathering » constant ; seulement 2 décisions du cerveau dans toute la partie, au pas 4-5) ; **vitesse
  x19 à x29** (2412 pas = 120 s de jeu en 4,2 à 6,2 s de temps mur, hors démarrage ~35 s). **Empreintes : DIFFÉRENTES entre
  deux courses** (identiques jusqu'au pas ~18, puis écart dans `HOLParallelSteerToMoveTargetProcessor` au pas 21 ; même
  avec `hol.Simulation.ParallelMass 0` : 181 lignes sur 200 diffèrent) : indéterminisme du mouvement/pilotage hors IA,
  à traiter dans le chantier déplacement (la graine n'existe pas dans le JSON de config, places fixes sur Plain en v2).
  La partie `aiduel` est donc rouge sur ce seul contrôle. Scénario d'essai prêt : `Tools/Scenarios/ai_economy_start.txt`
  (+ .json), à demander à l'utilisateur.
- 2026-10-29 : **essai `ai_economy_start` validé** par l'utilisateur (« ça fonctionne très bien »). Empreintes différentes entre
  deux courses : **renvoyées au chantier déplacement** (déterminisme du pilotage, `HOLParallelSteerToMoveTargetProcessor` au
  pas 21) ; la partie `aiduel` reste rouge sur ce seul contrôle d'ici là (hors `-Full`). **IA0.7 close**, `ai-ia1` fusionnée
  avec `main` puis `main` avancée en avance rapide (b032f0f). **IA0 close** (IA0.1-IA0.7). Prochain : IA1.3 (résumé étendu) ;
  Arwen libre.
- 2026-10-29 : **IA1.3 faite** (Arwen, `ai-ia1`, commit fe719aa, pas dans `main`). Résumé C++ étendu (`AI/HOLAISummary.h/.cpp`) : population
  (`PopulationUsed`, `PopulationCap`, `PopulationQueued` = unités en file des bâtiments, `bHasPopulation`, lus au PlayerState), `Buildings`
  (`FHOLAIBuilding` : type, état, avancement, position, file, en production ; fondations comprises ; triés par type puis position),
  `Peasants` par ressource (compteurs de récolte du PlayerState, ceux du HUD), `bHasCentre` / `CentreLocation`. Nouvelles
  `GatherBuildings` / `ReadFigures`, `Make` à arguments ajoutés en fin ; champs `ScriptReadWrite` donc visibles du cerveau
  Angelscript comme ceux d'IA0.3 (`EHOLConstructionState` devenu `BlueprintType`). Tests `AI.Summary.ExtendedRule` et
  `ExtendedWorld` rouges d'abord (build OK, 287/289), puis verts : `quick_check -Parts tests` 289/289. Limite : le test de monde
  pose population et récolteurs par les setters que les pas appellent (pas de partie jouée) ; la vraie chaîne en partie est
  couverte par la partie `aiduel`. Prochain : IA1.4.
- 2026-10-29 : **IA1.4 faite** (Arwen, `ai-ia1`, commit 882373e, pas dans `main`). `HOLPlayerOrders` : `QueueProduction`, `SetRally`
  (`Place` et `Build` y étaient) ; `UHOLConstructionSubsystem::FindRingFootprint` (anneaux de cases autour d'un point, toutes les
  règles de `CheckPlacement`, navmesh compris, bâtiments prévus comptés) et `FindFarmBlock` (moulin + 8 fermes en 3 x 3 cases) ;
  `UHOLAIOrderLibrary` : `CheckAIPlacement`, `FindAIPlacement`, `FindAIFarmBlock`, `IssueAIPlaceOrder` (placement vérifié avant la
  file, coût pris au pas suivant par la construction), `IssueAIBuildOrder`, `IssueAIProduceOrder`, `IssueAISetRallyOrder`
  (bâtiment nommé par son point, le plus proche du slot à 100 cm : le résumé n'a pas d'identifiant). Exposées à Angelscript comme
  IA0.4 (UFUNCTION de bibliothèque, `HOLAIOrder::...`). Tests `AI.Build.Placement`, `.Place`, `.Production` (nouveau
  `Tests/HOLAIBuildOrdersTests.cpp`, vraies sous-systèmes de construction et de production, PlayerState IA réel) rouges d'abord
  (stubs : 290/293), puis verts : `quick_check -Parts tests` 293/293. Limites : pas de test de partie jouée (une IA qui pose une
  maison) ni de test Angelscript des nouveaux appels ; `IssueAIPlaceOrder` ne vérifie pas le stock (le serveur refuse et journalise) ;
  `FindType` journalise une erreur pour un nom inconnu. Prochain : IA1.5.
- 2026-10-29 : **IA1.5 faite** (Arwen, `ai-ia1`, commit 490ed3e, pas dans `main`). Ouverture écrite en données dans `Script/AI/HOLAIOpening.as`
  (`FHOLAIOpeningData` : poids 3 nourriture / 2 bois, file de 2 Paysans au centre, cible 80, marge de maison 3, suite de bâtiments maison
  puis camp de bois près du bois puis moulin près des baies) et règles pures `ChooseBuild`, `ChooseProduction`, `SplitGather` (le cerveau
  `HOLAIBrain.as` lit le résumé, appelle les règles, envoie `HOLAIOrder::FindAIPlacement`, `IssueAIPlaceOrder`, `IssueAIProduceOrder`,
  `IssueAISetRallyOrder`, `IssueAIGatherOrder` : premiers appels Angelscript de ces ordres). Les 5 Paysans de départ se répartissent
  3 / 2 (un ordre par ressource) ; un Paysan est mis en file au centre tant qu'il y a file (2), place et nourriture, ralliement sur
  la source de la ressource qui manque de Paysans ; première maison quand il reste 3 places, du côté du centre opposé aux sources ;
  le bâtisseur est un Paysan inactif, sinon le dernier né. Résumé : nouveau `PeasantIds` (tous les Paysans), `FHOLAISourceNear.Location`
  lisible/écrivable par script. Tests : 12 tests en script (`HOLAIOpening_Test.as`) et `AI.Opening.FirstThought` / `.Buildings`
  (`Tests/HOLAIOpeningTests.cpp`, vraie chaîne script -> ordres -> file -> construction et production) rouges d'abord (stubs : 291/308),
  puis verts : `quick_check -Parts tests` 308/308. `AI.Brain.GatherIdle` et `.GatherableOnly` adaptés (population 5/100 pour que la
  maison ne vole pas le Paysan ; cas 2 = 3 Paysans déjà sur la nourriture -> l'arbre : l'ancienne règle « plus bas stock » ne sert plus
  qu'au repli sans source pondérée). Partie jouée (`aiduel`, 2 min sans fenêtre) : par IA, ordres au pas 4-5 (3 nourriture, 2 bois),
  Paysan en file au pas 9, **maison posée au pas 14-15** (~0,7 s), camp de bois au pas 19-20, moulin au pas ~1130 (attend 100 bois),
  un Paysan de plus toutes les 20 s ; **aucun Paysan inactif plus de 10 s** une fois les bâtisseurs comptés : `hol.Gather.Report` ajoute
  `building N` et le contrôle de `quick_check` retire les Paysans sur chantier. Parties `aiserver`, `netai`, `menu` (ailaunch)
  vertes après lecture des ordres 3 + 2 et du Paysan bâtisseur. `aiduel` reste rouge sur les empreintes (indéterminisme du
  pilotage, renvoyé au chantier déplacement). Limites : une seule maison (IA1.7 : maisons avant le plafond, bloc moulin + fermes) ;
  coûts des bâtiments recopiés dans les données (50 / 100 / 100 bois, pas de lecture de `BuildingTypes.csv` côté script) ; le
  bâtisseur choisi parmi les non inactifs est le dernier né (pas de position dans le résumé), il peut porter une charge ; pas de
  réaffectation quand une source s'épuise (IA1.6). Prochain : IA1.6 (besoins).
- 2026-10-29 : **IA1.6 faite** (Arwen, `ai-ia1`, commit 4ce6df2, pas dans `main`). Règle de besoins en script `Script/AI/HOLAINeeds.as` (fonctions pures
  sur le résumé et les données de l'ouverture, `FHOLAINeeds`) : dépense voulue sur 60 s = Paysans du centre (1 / 20 s, 50 nourriture) tant
  que Paysans vivants + en file < cible (80, GDD § 5.3 : 70 à 90) avec les maisons qu'ils demandent (3 / 10 de maison = 15 bois), le prochain
  bâtiment de la suite (hors maison) et une réserve (30 bois, en attendant IA1.7/IA2) ; demande aux sources = dépense moins la moitié du
  stock, jamais sous le quart de la dépense ; Paysans demandés par ressource = demande / taux net d'un Paysan (taux de récolte, aller-retour
  jusqu'au dépôt par `DepositDistance`, charge, vitesse) ; part de chaque ressource = Paysans demandés / total (sans source : 0 ; rien à
  demander : repli sur les poids 3 / 2). Les Paysans inactifs (`SplitGather`) et le ralliement du centre (`ChooseProduction`) suivent le
  manque contre cette part au lieu des poids fixes. Un filon vidé sort du résumé, ses Paysans restent inactifs et repartent à la pensée
  suivante vers ce qui reste (test `AI.Needs.UsedUpSource` : les 5 Paysans vont à l'arbre). Coûts lus des tables : nouveau
  `FHOLAITypeFigures` et `UHOLAIOrderLibrary::ReadAIUnitFigures` / `ReadAIBuildingFigures` (coût, temps, places, vitesse, charge, taux),
  appelés par `UHOLAIBrain.ReadFigures` ; `DefaultData` ne garde les valeurs des tables que comme repli. `Remaining` et `DepositDistance`
  du résumé lisibles/écrivables par script. Tests : 13 tests en script (`HOLAINeeds_Test.as`) et `AI.Needs.Figures` / `.Spread` /
  `.UsedUpSource` (vraie chaîne) ; rouges d'abord (règle forcée sur les poids : 317/322, `Spread` et 3 tests de script rouges), puis verts :
  `quick_check -Parts tests` 322/322. Partie `aiduel` : seul le contrôle d'empreintes (connu) est rouge, aucun Paysan IA inactif plus de
  10 s, ouverture 3 + 2 inchangée, ralliement alternant selon le manque ; `aiserver` verte. Limites : le Paysan à reprendre d'une
  ressource trop servie n'est pas choisi (le résumé n'a pas la ressource de chaque Paysan : seuls les inactifs et les nouveaux
  bougent) ; bâtisseur toujours « inactif sinon dernier né » (pas de position ni de charge dans le résumé) ; réserve et part du stock
  provisoires (IA4 les règle sur des parties IA seules) ; pierre et or sans demande tant qu'IA2 ne dépense pas ; `FindType` journalise
  une erreur pour un nom inconnu (test avec erreur attendue). Prochain : IA1.7 (maisons avant le plafond, camps, moulin + fermes).
- 2026-10-29 : **IA1.7 faite** (Arwen, `ai-ia1`, commit 943fcd0, pas dans `main`) ; **la vérification en partie a trouvé deux limites, à traiter (IA1.7b)**. Script (`HOLAIOpening.as`, `HOLAINeeds.as`, `HOLAIBrain.as`) : `ChooseBuild` à quatre rangs : 1. maison quand les places restantes (plafond - prises - en file) + places des maisons en chantier <= marge, la marge = producteurs x ceil((20 s de chantier + 5 s de marche) / 20 s par Paysan) = 2 pour le centre seul, pas au-delà de 175, jamais retenue par les chantiers ; 2. les étapes de la liste (camp de bois, moulin : la maison n'en fait plus partie ; une étape sans source est sautée) ; 3. un camp de plus (bois, pierre, or si les besoins le demandent) à côté de la source la plus proche quand elle est à plus de 1500 cm de tout dépôt (bord du massif entamé), un camp à la fois ; 4. le bloc de fermes quand les baies durent moins de 240 s au rythme des Paysans à la nourriture : `bPlanFarms` -> le cerveau appelle `FindAIFarmBlock` autour du centre (10 anneaux), garde le plan (`FarmPlans`), pose le moulin puis les 8 fermes aux places prévues ; plan refait si une place est refusée. 3 chantiers à la fois (`MaxSites`), bâtisseurs déjà envoyés exclus (`SentBuilders`, vidé sans chantier), personne ne bâtit sous 2 Paysans (`MinPeasantsToBuild`, ce qui épargne le Paysan seul des tests du cerveau). Dépense : les fermes non bâties (75 bois) comptent tant que les baies baissent. Conséquence d'ouverture : le camp est posé dès la première pensée par le premier Paysan inactif (plus de maison d'abord : elle vient vers 40 s), les 4 autres se répartissent 2 / 2 (`aiserver` et `netai` relus en 2 + 2). Le cerveau écrit une ligne `status` toutes les 5 s (population, file, stocks, Paysans, maisons, camps, moulins, fermes). Tests : 15 tests en script nouveaux ou refaits (rouges d'abord : 317/330 avec 13 en échec, sur des signatures vides), puis verts ; `AI.Buildings.Houses` et `.FarmBlock` (vraie chaîne, écrits après le code : pas rouges d'abord), `AI.Opening.FirstThought` et `AI.Needs.UsedUpSource` relus (le camp prend un Paysan) : `quick_check -Parts tests` 333/333, `aiserver` et `netai` verts. `quick_check -Parts aiduel -DuelSeconds 480` (nouveau paramètre, nouvelles vérifications : population au-delà de 20, file bloquée au plafond, stock > 400) : **population 29/40 à 8 min, 3 maisons, 8 fermes et 2 moulins par IA, jamais de file bloquée au plafond** ; mais deux contrôles rouges (plus l'empreinte, connue). **Limite 1 : Paysans inactifs jusqu'à 122 s** après les fermes : les Paysans inactifs et le ralliement du centre vont tous à la source la plus proche (`Sources[0]`), une ferme n'a qu'**une** place (le reste attend devant, inactif : 6 inactifs sur `Food #378` pendant des minutes) ; il faut répartir les inactifs sur plusieurs fermes (le résumé n'a ni `Places` ni les occupants d'une source : à ajouter en C++, `PlaceHolders` du processeur de récolte) ou faire chercher une ferme libre au processeur (comme `FindMassifSpot` pour les arbres). **Limite 2 : stocks > 400** (nourriture 970-1000, bois 1170-1270 à 8 min) : le centre ne produit qu'un Paysan par 20 s (50 nourriture) et l'IA ne dépense rien d'autre avant IA2 (armée) ; les Paysans déjà à une ressource n'en sortent pas (pas de reprise, limite d'IA1.6) ; ce contrôle ne peut passer qu'avec la dépense militaire (IA2) ou un plafond de stock qui coupe la part d'une ressource. Autres limites : le bloc de fermes est unique (un second pour 80 Paysans : IA2/IA4) ; un chantier dont le bâtisseur meurt n'est pas repris (pas de minuterie) ; les tests de chaîne `Houses` et `FarmBlock` n'achèvent pas les chantiers (pas de processeur de construction). Prochain : IA1.7b (Paysans inactifs sur fermes pleines, puis stock), IA1.8.
- 2026-10-29 : **IA1.7b faite** (Arwen, `ai-ia1`, commit 587a1d7, pas dans `main`). **Décision de l'utilisateur : le critère « pas plus de 400 d'une ressource en stock » est ABOLI** (l'IA n'a rien à acheter avant IA2) : retiré du contrôle `aiduel` de `quick_check`, de la ligne IA1.8 du plan ; aucun plafond de part de ressource. **Limite 1 corrigée (Paysans inactifs sur fermes pleines), option (a)** : le résumé C++ porte `Places` et `Occupants` pour chaque source (`FHOLAISourceNear` ; `FHOLAISourceUse` donné à `Make`) ; le processeur de récolte publie les places tenues à chaque pas dans le registre des sources (`PublishOccupants`, `UHOLSourceSubsystem::SetOccupants`, `PruneHolders` tiré de `HasFreePlace`) ; en script (`HOLAIOpening.as`) `WithoutHeldSources` écarte les sources dont les places sont toutes tenues (une ferme n'en a qu'une), `SplitGather` répartit les Paysans libres source par source (`FreeSourceOf`, une place = un Paysan dans la pensée, `WithRoom` met à zéro la part d'une ressource sans place, sinon le stock le plus bas avec place, sinon pas d'ordre) et `ChooseProduction` vise le ralliement sur une ferme libre. Même information qu'un joueur (un ouvrier sur la ferme). Tests rouges d'abord : 3 tests en script (`Test_IdlePeasantsSpreadOverFreeFields`, `Test_FullFieldsSendThePeasantsToTheWood`, `Test_RallySkipsHeldFields`) en échec (334/337) avec l'ancien script, puis verts ; `AI.Summary.Uses` et `GatherableOnly` (places d'une ferme) ajoutés ; `quick_check -Parts tests` 337/337, `aiserver`, `netai`, `gather` verts. `quick_check -Parts aiduel -DuelSeconds 480` : **le contrôle d'inactivité passe** (au plus 2 non-récoltants au-dessus du héros, contre 7 avant ; ancien rouge : 122 s ; population 29/50 et 29/40, 8 fermes et 2 moulins par IA, jamais de file bloquée) ; seul reste rouge l'empreinte des deux courses (connu, hors IA). Limites : un Paysan rallié en même temps qu'un autre sur la même ferme libre peut attendre une pensée devant elle (le suivant la voit tenue) ; le résumé garde 8 sources par ressource (`DefaultMaxSourcesPerResource`) : avec plus de 8 champs et buissons proches, les plus lointains ne sont pas vus ; les stocks restent élevés (nourriture ~1200, bois ~850 à 8 min), sans conséquence avant IA2. Prochain : IA1.8 (vérification, essai de l'utilisateur), IA1.9.
- 2026-10-29 (note de J8c, `JourneyInProgress.md`, J8c-Q5) : **à faire quand le cerveau commandera une armée et son héros (D425)** : lancer *Serrez les rangs* (A, −25 % de dégâts reçus 15 s à 12 m, recharge 60 s) et planter la *Bannière de l'Aube* (Z, niveau 4, point à 4 cases au plus, aura 45 s, recharge 120 s) ; l'aura passive (−15 % à 12 m) lui profite déjà. Rien à coder en J8c côté IA.
- 2026-10-29 : **IA1.8 faite, IA1.9 faite : IA1 close** (session de clôture ; code dans `main` 1911c5a). **Vérification (IA1.8)** : scénario `Tools/Scenarios/ai_economy.txt` (+ .json, l'utilisateur contre une IA qui bâtit toute son économie), `quick_check -Parts aiduel -DuelSeconds 480` : aucun Paysan inactif plus de 10 s (au plus 2 non-récoltants au-dessus du héros), jamais de file bloquée au plafond, 8 fermes et 2 moulins par IA, population **29/50 et 29/40 à 8 min** ; **essai de l'utilisateur validé** (« très bien », 2026-10-29). **GDD : D527 à D531** (ferme 2 × 2 et ses règles, 5 Paysans de départ, ordres d'IA par la file commune et résumé avec places et occupants, ouverture puis besoins avec cible de 80 Paysans et critère « 400 en stock » aboli, placement par type) ; renvois au § 5.1, § 5.3 et au paragraphe IA du § 14.4 ; prochain numéro D532. **Limites connues d'IA1** (honnêtes, à lire avant IA2) : plusieurs maisons sont bien posées (3 par IA à 8 min : l'ancienne limite « une seule maison » est levée par IA1.7) ; un seul bloc de fermes ; un chantier dont le bâtisseur meurt n'est pas repris ; le bâtisseur = Paysan inactif, sinon le dernier né (le résumé n'a ni position ni charge) ; le Paysan d'une ressource trop servie n'est jamais repris (seuls les inactifs et les nouveaux bougent) ; le résumé garde 8 sources par ressource ; un Paysan rallié en même temps qu'un autre sur la même ferme libre peut attendre une pensée devant elle ; réserve, parts et cibles provisoires (IA4, parties à 8 IA) ; stocks élevés (nourriture ~1200, bois ~850 à 8 min) sans conséquence avant IA2 ; pierre et or sans demande tant qu'IA2 ne dépense pas ; **le contrôle d'empreintes identiques de `aiduel` reste rouge** (indéterminisme du pilotage, `HOLParallelSteerToMoveTargetProcessor` au pas 21, hors IA, renvoyé au chantier déplacement et non traité : `aiduel` hors `-Full`). **Paysans à 5 et 10 min : non relevés** (seul le point de 8 min est connu : 29) ; à noter à la première partie IA longue d'IA2 (référence pour IA4). Arwen libre. **Prochain : IA2 (armée : Caserne vers l'ennemi, Champ de tir, contre-unités, héros D425, Serrez les rangs / Bannière de l'Aube, note de J8c).**
- 2026-10-29 : **IA2 (armée) ouverte** (reprise de séance, rien codé). Relevé : le résumé n'a **aucune information sur l'ennemi** (ni unités ni bâtiments adverses) ni sur l'armée propre ;
  ordres existants côté script : `IssueAIMoveOrder`, `IssueAIGatherOrder`, production, ralliement ; **pas d'ordre d'attaque** (`HOLPlayerOrders::AttackMove` et `UHOLCommandSubsystem::IssueAttackBuildingOrder` existent
  côté humain, à exposer comme IA0.4). Unités du prototype : Lancier (Spearman) et Homme d'armes (ManAtArms) à la Caserne, Archer et Arbalétrier au Champ de tir (150 bois, 40 s) ; pas de cavalerie ni de siège ;
  héros J8 en cours (hero-j8c) donc hors de ce premier passage (note J8c-Q5 plus haut). Brouillard : chantier Faramir (D518-D526), l'IA y lira la vue de son camp.
  **IA2-Q1 = A** (choix de l'utilisateur, conforme à la recommandation) : la connaissance de l'armée ennemie vient d'**un seul point d'entrée** C++ (liste d'unités ennemies dans `HOLAISummary` : type, position, PV) ;
  aujourd'hui il lit tout (rien n'est caché), le brouillard n'aura qu'à y brancher la vue du camp, sans toucher aux règles en script. Prochain : IA2-Q2 (composition : règle de contres et part de l'armée dans la dépense).
- 2026-10-29 : **IA2-Q2 = A** (besoins + contres, conforme à la recommandation) : l'armée entre dans la règle de besoins d'IA1.6 (dépense voulue sur 60 s) ; part de l'armée qui monte avec le temps et la menace vue
  (armée ennemie > la sienne) ; composition = unités qui contrent le plus fort des types ennemis vus (Arbalétrier contre Homme d'armes, Archer contre Lancier, Lancier contre la cavalerie quand elle existera), repli sur un
  mélange fixe sans ennemi vu ; Caserne puis Champ de tir bâtis quand la composition les demande. **IA2-Q3 = A** (rapport de force, conforme à la recommandation) : regroupement au ralliement devant le centre ; défense dès qu'un
  ennemi entre dans un rayon autour de ses bâtiments (tout le groupe au contact) ; vague en attack-move sur le centre ennemi quand la valeur de son armée (coût pondéré par les contres) dépasse celle vue chez l'ennemi d'une marge,
  après un délai minimum ; retraite si le rapport s'inverse ; marge et délai = curseurs d'IA4 (valeurs de Normal d'ici là).
  **Découpage IA2** (boucles de 15 min, test rouge d'abord, un sous-agent par boucle) : **IA2.1** résumé : armée propre (soldats par type, identifiants, positions, inactifs) et `Enemies` (unités ennemies : type, position, PV,
  propriétaire ; bâtiments ennemis connus, centre) lus en un seul point C++ (IA2-Q1) ; **IA2.2** ordres exposés au script : `IssueAIAttackMoveOrder`, `IssueAIAttackBuildingOrder` (`HOLPlayerOrders::AttackMove`,
  `IssueAttackBuildingOrder` existent) ; **IA2.3** règle de contres et valeur d'armée en script (fonctions pures, `HOLAIArmy.as`) ; **IA2.4** besoins : armée dans la dépense, Caserne et Champ de tir dans la suite des
  bâtiments, production par contres, ralliement des bâtiments militaires ; **IA2.5** regroupement et défense de la base ; **IA2.6** vague d'attaque et retraite (rapport de force) ; **IA2.7** vérification (`aiduel` IA contre IA
  longue, scénario d'essai `ai_army` : l'utilisateur contre une IA qui attaque ; relever les Paysans à 5 et 10 min) ; **IA2.8** clôture (GDD D532+, `main`). Héros (D425, Serrez les rangs, Bannière de l'Aube) : après
  J8c, hors de ce passage. Prochain : IA2.1 dans Arwen (`ai-ia1`, `main` à fusionner d'abord).
- 2026-10-29 : **IA2.1 faite** (Arwen, `ai-ia1`, commit 054c160, pas dans `main` ; sous-agent ~11 min). `FHOLAISummary` : `Army` (soldats de l'emplacement : unités qui ne récoltent pas, ni mourantes, ni Roi ni héros ; par identifiant),
  `Enemies` (unités des autres emplacements non vaincus, Paysans compris, par identifiant), `bHasEnemyCentre` / `EnemyCentreLocation` (centre principal ennemi le plus proche du sien, égalité : emplacement le plus bas) ; struct
  `FHOLAIUnit` (Id, OwnerSlot, Type, Location, Health, bIdle, TargetId) lisible/écrivable par script. **Point d'entrée unique** `HOLAISummary::GatherEnemies` (commentaire : le brouillard y coupera à la vue du camp ; `GatherBuildings` aussi
  à couper, pour le centre ennemi). `Make` : deux arguments ajoutés en fin. `FHOLUnitView` gagne `bHero` (chemin simulé et chemin client). Tests rouges d'abord (stubs : 352/356), puis verts : `quick_check -Parts tests` 356/356 (44 s) ;
  4 tests C++ (`HOLAIArmyTests.cpp` : `AI.Summary.ArmyRule`, `.EnemiesRule`, `.Army`, `.Enemies`) et 2 en script (`HOLAIArmy_Test.as`, visibilité des champs seulement). Limites : unité mourante couverte seulement par les tests de règle
  (le monde de test n'a pas de simulation) ; le résumé ne liste ni le Roi ni le héros (à ajouter si le cerveau en a besoin). Prochain : IA2.2 (ordres d'attaque) et IA2.3 (règles de contres, script) en parallèle.
- 2026-10-29 : **IA2.2 écrite, pas vérifiée** (Arwen, `ai-ia1`, rien de commité, sous-agent ~40 min). `HOLAIOrders.h/.cpp` : `IssueAIAttackMoveOrder`, `IssueAIAttackBuildingOrder` (bâtiment de n'importe quel propriétaire, à 100 cm près ;
  `FindBuildingAt`), `FHOLAITypeFigures` étendu (Health, Damage, AttackInterval, AttackRange, MeleeArmor, RangedArmor, bRanged, ArmorCategories, BonusCategories / BonusDamage = dégâts plats ajoutés par coup, comme
  `HOLCombat::GetDamage`) lus de la même ligne `FHOLUnitTypeRow` que le combat ; tests `AI.Orders.AttackMove`, `.AttackBuilding`, `.CombatFigures` et `HOLAIFigures_Test.as`. **Rouge fait** (stubs : 356/361). **Vert non fait** : la
  compilation de l'implémentation est bloquée par un `cl.exe` figé (PID 38316, lancé à 18:27, ~5 s de CPU en 28 min ; UBT PID 34756 l'attend) ; l'arrêt a été refusé par le classificateur de permissions (au sous-agent comme à la session
  principale) : à tuer par l'utilisateur puis relancer `Tools\build.ps1` et `quick_check -Parts tests` (attendu 361/361). Chiffres des tables : Lancier 80 PV, 8 dégâts, 1,875 s, portée 130, bonus Cavalerie +17 ; Homme d'armes 155 PV, 12, 1,375 s,
  70, armures 4/4, catégories HeavyInfantry + Heavy ; Archer 70 PV, 5, 1,625 s, 2000, bonus LightInfantry +5 et GunpowderInfantry +5 ; Arbalétrier 80 PV, 12, 2,125 s, 2000, bonus Heavy +10. Fin de ligne : `HOLAIOrders.cpp` passe en CRLF
  pur (`git diff --ignore-cr-at-eol` pour le vrai diff, ~68 lignes).
- 2026-10-29 : **IA2.2 faite** (Arwen, `ai-ia1`, commit suivant 054c160, pas dans `main`). Après l'arrêt par l'utilisateur de `cl.exe`, d'UBT et du `build.ps1` figés du sous-agent (le processus de `build.ps1` tenait aussi `build.log`) :
  compilation PASS, `quick_check -Parts tests` **360/361** puis **361/361**. Seul échec : `AI.Orders.AttackBuilding` attendait les Lanciers en `Moving` après un pas ; le monde de test applique l'ordre (`OrderedBuilding` posé,
  vérifié) mais l'approche relève du processeur de combat : assertion retirée, l'approche sera vue en partie jouée (IA2.7). Les deux ordres et les chiffres de combat (voir l'entrée précédente) sont donc prêts pour le script.
  Prochain : IA2.3 (règles de contres et valeur d'armée, `HOLAIArmy.as`, script pur).
- 2026-10-29 : **IA2.3 faite** (Arwen, `ai-ia1`, commit après 525178c, pas dans `main` ; sous-agent ~7 min, script pur). `Script/AI/HOLAIArmy.as` (`FHOLAIArmyData`, `FHOLAIComposition`, namespace `HOLAIArmy`) et `HOLAIArmyRules_Test.as`
  (15 tests). Rouge d'abord (stubs : 361/376), puis vert : `quick_check -Parts tests` **376/376** (revérifié par la session principale). **Formules** : coup = dégâts + bonus plat par catégorie du défenseur − armure (mêlée ou distance
  selon `bRanged`), jamais sous 1 (comme `HOLCombat::GetDamage`, sans le ×0,3 du héros) ; `DamagePerSecond` = coup / intervalle ; `Matchup(A,D)` = DPS(A) x PV(A) / (DPS(D) x PV(D)) avec deux termes tirés des chiffres réels : tirs
  gratuits d'un tireur qui dépasse en portée une unité de mêlée pendant qu'elle approche, et `ShooterExposure` 0,5 quand la mêlée frappe un tireur (valeur ajustée, à régler à IA4) ; `Matchup(A,D) x Matchup(D,A)` = 1 ;
  `UnitValue` = nourriture + bois + pierre + 1,5 x or (80 / 130 / 80 / 140 pour Lancier / Homme d'armes / Archer / Arbalétrier) ; `ArmyValue` = somme des valeurs x fraction de PV ; `EnemyValue` = même somme sans les Paysans,
  chaque type ennemi divisé par la force moyenne de nos types contre lui (bornée à [1/5, 5]) ; `PowerRatio` = notre valeur / valeur ennemie (0 sans armée contre une armée, 1000 sans armée ennemie) ; `ChooseComposition` : sans
  armée ennemie vue, mélange fixe 2 Lanciers : 1 Homme d'armes : 2 Archers : 1 Arbalétrier (le plus en dessous de sa part, égalité au premier) ; sinon le type au meilleur `Matchup` borné à 5 contre l'armée ennemie vue (pondéré par
  la valeur de chaque type) par coût ; renvoie aussi le bâtiment (Caserne pour Lancier et Homme d'armes, Champ de tir pour Archer et Arbalétrier : noms à vérifier contre `BuildingTypes.csv` en IA2.4). **Résultats** : 5 Hommes d'armes
  -> Arbalétrier ; Archers ennemis -> Homme d'armes ; **Lanciers ennemis -> Archer d'extrême justesse (0,6 %) avec le plafond 5, Homme d'armes au-dessus de 5,1** : le GDD § 7.2 se contredit (tableau « masse d'infanterie légère ->
  archers », ligne « lanciers et archers -> hommes d'armes »). Armure = soustraction plate : Archer contre Homme d'armes = 1 dégât par coup. Limites : seuls les 4 types militaires du prototype ont des chiffres (cavalerie et
  siège à ajouter dans `Data.Types` avec leur chantier) ; l'armure n'a pas les niveaux ni les technologies. Prochain : IA2.4 (besoins : armée dans la dépense, Caserne et Champ de tir dans la suite des bâtiments, production par contres,
  ralliement des bâtiments militaires). À trancher peut-être avec l'utilisateur : Lanciers ennemis -> Archer ou Homme d'armes ?
- 2026-10-29 : **Lanciers ennemis -> Archer gardé** (proposition acceptée par l'utilisateur : « très bien ») ; plafond du rapport de force (5) réglé à IA4. Prochain : IA2.4.
- 2026-10-29 : **IA2.4 faite** (Arwen, `ai-ia1`, commit après 5d13714, pas dans `main` ; sous-agent ~21 min). Rouge d'abord (stubs : 379/395, 15 des 18 nouveaux tests en script + `AI.Army.Production` en échec), puis vert :
  `quick_check -Parts tests` **395/395** (revérifié). **Part de l'armée** (`FHOLAIOpeningData`, provisoire pour IA4) : 0 tant que Paysans vivants + en file < 20 (`ArmyMinPeasants`), puis 0,15 montant linéairement avec le temps de jeu
  jusqu'à 0,45 à 600 s, +0,15 tant que `PowerRatio` < 1 avec un ennemi vu (plafond 0,6) ; c'est une part du **revenu** (Paysans x taux net moyen x 60 s), pas de la dépense de l'économie ; elle se partage à égalité entre Caserne et Champ de tir,
  chacun dépensant pour l'unité que `ChooseComposition` (nouvelle surcharge à filtre de bâtiment) choisit pour lui, au plus 60 s / temps de construction ; les parts de Paysans suivent : l'or et la pierre ne reçoivent de demande que si les unités en coûtent
  (mélange fixe : pas d'or ; Archers ennemis -> Homme d'armes + Arbalétrier : or demandé). **Bâtiments** : Caserne ou Champ de tir (celui de la composition, s'il manque) à partir de 16 Paysans (`ArmyBuildPeasants`), maison + camp + moulin faits,
  150 bois, ancrés à centre + 1500 cm vers le centre ennemi ; devant les camps et le bloc de fermes ; la marge de maison compte les bâtiments militaires finis comme producteurs. **Production** : par bâtiment fini, file de moins de 2 unités, stock
  suffisant après les unités déjà choisies dans la même pensée, place restante, bois laissé pour la maison voulue. **Ralliement** des bâtiments militaires = centre + 1500 cm vers l'ennemi (`IssueAISetRallyOrder`, source -1), renvoyé si le point
  bouge de plus de 100 cm. Ligne `status` : ` army S M A C barracks q range q`. `quick_check` : le contrôle des Paysans inactifs soustrait les soldats (lus des lignes `status`) car `hol.Gather.Report` les compte « non récoltants » (contrôle un peu moins strict).
  **`aiduel` 480 s** : seule l'empreinte reste rouge (connue) ; par IA : population 36-37/50, 28 Paysans, 4 maisons, 2 moulins, 8 fermes, **1 Caserne (posée à ~355 s) + 1 Champ de tir**, soldats vivants 5-6 (1-2 Lanciers, 2 Hommes d'armes, 2 Archers,
  0 Arbalétrier), files 2 + 2 ; aucun Paysan inactif. **Limites** : le **bois** est le goulot (15-50 pendant 6 min : le partage 3:2 nourriture/bois et les fermes à 75 bois le mangent) donc l'armée arrive tard (5-6 soldats à 8 min) -> à régler à
  IA4 (poids ou réserve de bois) ; un seul bâtiment de chaque sorte ; pas d'or récolté dans cette partie (l'Homme d'armes en use 20) ; le point de ralliement peut coïncider avec la recherche de place des bâtiments militaires (non vérifié en partie
  réelle) ; dans le monde de test la production remet la population à 0/0 (le test la repose). Prochain : IA2.5 (regroupement et défense de la base).
- 2026-10-29 : **IA2.5 faite** (Arwen, `ai-ia1`, commit après 0055f2e, pas dans `main` ; sous-agent ~14 min). Regroupement et défense de la base en script (`HOLAIArmy.as` : `Threat`, `ShouldDefend`, `UpdateDefence`, `DefenceOrders`, `RegroupOrders`,
  état `FHOLAIDefence` passé en référence ; `HOLAIBrain.as` : `ThinkArmy` après `ThinkMilitary`, point de rassemblement = `RallyPointOf`). Rouge d'abord (stubs : 404/420), puis vert : `quick_check -Parts tests` **420/420** (revérifié) ; 22 tests en
  script (`HOLAIDefence_Test.as`) + `AI.Army.Defence` et `.DefenceWeak` (vraie chaîne : ordres lus dans la file de commandes). **Règles** : menace = unité ennemie non-Paysan vivante à moins de `DefenceRadius` (3000 cm) d'un bâtiment (fondations comprises)
  ou du centre ; lieu = l'ennemi le plus proche du centre, ou le barycentre des menaçants à moins de 1000 cm de lui ; défendre si `Ratio` (rapport de force contre les seuls menaçants) >= 0,3 ou si un menaçant est à moins de 1000 cm du point de rassemblement ;
  tous les soldats en attack-move vers la menace (premier ordre à tous, renvoi toutes les 3 s à ceux qui ne frappent pas) ; fin de l'état 5 s après la dernière demande ; au repos : les soldats inactifs, sans cible, à plus de 1000 cm du point repartent
  en attack-move vers lui (au plus un ordre par 10 s et par soldat). Journal du cerveau : `defence begins` / `defence ends, home`, `defend with K soldiers`, `regroup K soldiers`, et fin de la ligne `status` : `home|defend regroups N defences M`.
  **Limites / à voir** : en partie `aiduel` de 8 min les armées ne se rencontrent jamais (0 ordre de défense ni de regroupement, tous `home`) : **défense et regroupement jamais vus en vraie partie**, il faut une incursion (IA2.6 ou l'essai `ai_army`) ;
  l'heure de jeu du monde de test est toujours 0 (les délais ne sont testés qu'en script) ; Paysans non ordonnés (pas de fuite), pas de déclencheur « unité près d'un bâtiment attaqué », ni défense contre bâtiments ou siège, ni Roi/héros ; valeurs provisoires pour IA4.
  **Nouveau rouge dans `aiduel`** (pas dû à IA2.5, identique avec `ThinkArmy` coupé) : « aucun Paysan IA inactif plus de 10 s (12 s et 10 s) » : approximation du contrôle (il soustrait les soldats lus des lignes `status` toutes les 5 s ; vers 404-412 s,
  2 unités de plus que le héros sont « non récoltantes » sans soldat au `status`, probablement nées entre deux lignes). À corriger au prochain passage sur `quick_check`. Prochain : IA2.6 (vague d'attaque et retraite).
- 2026-10-29 : **IA2.6 faite** (Arwen, `ai-ia1`, commit après 1fc54ed, pas dans `main` ; sous-agent ~21 min). Vague d'attaque et retraite en script (`HOLAIArmy.as` : `EHOLAIWavePhase` Home / Out / Back, `FHOLAIWave`, `CanLaunch`, `WaveRatio`,
  `WaveTarget`, `UpdateWave` ; `HOLAIBrain.as` : `ThinkArmy` refait autour de la vague, une `FHOLAIWave` par emplacement). Rouge d'abord (stubs : 427/443), puis vert : `quick_check -Parts tests` **443/443** (revérifié) ; 21 tests en
  script (`HOLAIAttack_Test.as`) + `AI.Army.Attack` (vraie chaîne : 9 Lanciers au point, 455 s forcées par `AdvanceStepIndexForTests`, centre ennemi connu, un ennemi faible -> 9 ordres d'attack-move vers le centre ; 12 Hommes d'armes vus
  -> aucun ordre). **Règles** : lancement si phase Home, pas de défense en cours, délai écoulé, centre ennemi connu, >= 8 soldats, >= 420 s (300 s sans ennemi vu), `PowerRatio` >= 1,5 ; attack-move de tous les soldats vers le centre ennemi ;
  pendant la vague : soldats inactifs réordonnés toutes les 5 s (centre, sinon ennemi le plus proche) ; retraite (ordre de marche, pas d'attack-move) au point de rassemblement si le rapport près du barycentre < 0,6, ou pertes > 60 % de la taille
  lancée, ou rappel par la défense (au moins 3 ennemis menaçants) ; retour « Home » quand la moitié des survivants est à moins de 1000 cm du point ou après 40 s ; délai 90 s (180 s après une retraite) ; fin de vague sans centre ni ennemi vu. Données
  provisoires pour IA4 (voir `FHOLAIArmyData`). Ligne `status` : ` wave off|out|back size N` ; journaux `attack launched with K soldiers, ratio R`, `retreat: ratio R, K left`, `wave over, home`.
  **`aiduel` 900 s** (marge 1,5) : **aucune vague** (deux IA symétriques, ~12 soldats chacune à 540 s, rapport ~1) ; avec marge 1,0 (temporaire, remise à 1,5) : vagues de 8 à 22 soldats dès 494 s, retraites sur rapport ou pertes, défenses déclenchées
  (555 s, 590 s), séquence `out -> back -> off` correcte ; **jusqu'à 1200 s : aucun bâtiment détruit, aucun vainqueur** (seulement des soldats morts). **À voir** : on ne sait pas si l'attack-move frappe les bâtiments : les vagues
  atteignent le centre ennemi et s'y usent sans le détruire -> peut-être un ordre `IssueAIAttackBuildingOrder` des soldats inactifs près du centre. **Bug d'économie trouvé** (IA1/IA2.4, pas corrigé) : dans la partie de 900 s, ordre
  `idle -> gather Gold #…` répété des centaines de fois et `Gather: … stopped idle with no room` ~2345 fois : des Paysans sont renvoyés sans cesse à une mine d'or sans place ; 5 à 7 unités non récoltantes au-dessus des soldats + héros en fin de
  partie, séries d'inactivité jusqu'à 242-292 s à 1200 s -> le contrôle d'inactivité de `quick_check` (assoupli : `non récoltantes - soldats > héros + 2`) reste rouge, et ce n'est pas un faux rouge. Aussi : bois qui s'accumule (3000-4800) pendant que la
  nourriture reste sous 60 dès ~600 s, donc production militaire bloquée à 24-25 soldats (réglage d'IA4). Limites : Paysans non ordonnés par la vague, pas de renfort d'une vague en cours, retraite sous le feu sans esquive, retraite/rappel non
  couverts par le test C++. Prochain : IA2.6b (corriger l'or sans place ; attaque des bâtiments au centre ennemi), puis IA2.7.
- 2026-10-29 : **IA2.6b faite** (Arwen, `ai-ia1`, commit après 0b9ebc6, pas dans `main` ; sous-agent ~15 min). **(A) Bug de l'or** : cause = un filon d'or ou de pierre a `Places = 0` (« places tout autour », `SourceTypes.csv`), donc `WithoutHeldSources`
  ne le masquait jamais ; le processeur de récolte ne garde que quelques points autour du filon (`FindSourceSpot`) et les Paysans en trop restent inactifs devant (`StandBeforeSource`) ; le cerveau les renvoyait au même filon à chaque pensée
  (2110 ordres `idle -> gather Gold`, ~15 000 « stopped idle with no room ») ; ils s'y entassaient parce que la nourriture n'avait plus de place (8 fermes tenues) et que `LowestStockWithRoom` retombait sur le bois ou l'or. Ce n'était ni un camp manquant
  ni un dépôt hors de portée. **Correction** : `HOLGatherProcessor` note l'heure où un récolteur est resté sans place à chaque source (`LastNoRoom`) et publie les sources « pleines » (20 s, `Gather::FullShownSeconds`) ; `UHOLSourceSubsystem::SetFullSources` /
  `IsFull` ; le résumé porte `bFull` (`FHOLAISourceNear`, `FHOLAISourceUse`) ; `OpenPartOf` en script écarte les sources `bFull` (`SplitGather` et `ChooseProduction`). Rouge d'abord (443/445 : `Test_FullVeinGetsNoPeasant`, `Test_FullVeinSendsThemToTheWood`),
  puis vert : `quick_check -Parts tests,gather,aiserver,netai` **445/445**, gather, aiserver, netai PASS (revérifié). `aiduel` 900 s : ordres `idle -> gather Gold` 2110 -> 42 / 34, « stopped idle with no room » ~15 000 -> 34. **Reste rouge** : contrôle d'inactivité
  (plus longue série 20 s et 36 s au lieu de 242-292 s) : 3 à 5 unités non récoltantes par séries de 24-46 s, cause = **l'IA fait plus de Paysans qu'elle n'a de places** (nourriture : un seul bloc de 8 fermes tenues ; filon petit ; bois hors de portée des 8 sources
  les plus proches ou plein) ; stocks qui s'entassent (bois 2400-2950, or ~2000, nourriture ~30). **Décision de règles à prendre** : un second bloc de fermes quand toutes les fermes sont tenues et la nourriture manque ; ou plafonner les Paysans aux places ;
  ou plus de sources dans le résumé (8 aujourd'hui) ; ou assouplir le contrôle (marge 2 -> ~5). Non testé en C++ : `bFull` du résumé. **(B) Attaque des bâtiments** : l'attack-move **attaque déjà les bâtiments** (`HOLCombatProcessor.cpp` ~1568-1595 : pour une
  unité à `BuildingAttack > 0`, avec `bMaySeek` vrai en attack-move et sans cible unité, tout bâtiment ennemi dans `GetBuildingSearchRadius` = 1600 cm ; `BuildingAttack` : Lancier 20, Homme d'armes 20, Arbalétrier 12, Archer 5, héros 10 ; Archer fait peu) : rien ajouté.
  Les vagues sans bâtiment détruit viennent probablement de la retraite (rapport < 0,6 ou pertes > 60 %), des unités ennemies (même Paysans) prises avant les bâtiments, et de dégâts faibles contre PV et armure des bâtiments (non vérifié : journaliser les PV
  du bâtiment pendant une vague en IA2.7). Prochain : IA2.7 (essai `ai_army`).
- 2026-10-29 (fin de séance, reprise dans une autre session, demande de l'utilisateur) : **état** : IA2.1 à IA2.6b faites et commitées dans Arwen (`ai-ia1`, dernier commit 9dcb5d3, tests 445/445), **rien dans `main`** (main = 7c8cc9c + docs). **À trancher / faire au retour** :
  (1) Paysans en trop pour les places : option A recommandée (second bloc de fermes quand toutes les fermes sont tenues et que la nourriture manque), B plafonner les Paysans aux places, C assouplir le contrôle ; l'utilisateur n'a pas encore répondu ;
  (2) IA2.7 : scénario d'essai `Tools/Scenarios/ai_army.txt` (+ .json) lancé par `play.ps1` (l'utilisateur contre une IA qui attaque ; journaliser les PV du bâtiment attaqué pendant une vague ; abaisser `AttackMinSeconds` / marge dans le scénario si besoin pour qu'une vague vienne), relever les Paysans à 5 et 10 min, essai de l'utilisateur ;
  (3) IA2.8 : clôture (GDD D532+ : IA2-Q1 à Q3, formules d'IA2.3, plafond 5, Lanciers -> Archer, part de l'armée, vague et retraite ; `main` avancée après fusion de `main` dans `ai-ia1`, prévenir les sessions actives). Aucun processus à tuer : les builds figés ont été arrêtés par l'utilisateur.
  Reprise = « continue AIInProgress ».
- 2026-10-30 : **IA2.6c faite** (Arwen, `ai-ia1`, commit d467d36, pas dans `main`). Décision de l'utilisateur : option A (second bloc de fermes). `HOLAIOpening.as` : `SecondBlockWanted` (premier bloc tout bâti, aucune
  source de nourriture avec place libre, stock de nourriture < `SecondBlockFood` = 150, au plus `MaxFarmBlocks` = 2 blocs), `FarmTarget` (8, ou 16 dès qu'une neuvième ferme existe ou que le second bloc est voulu), `PlanBuilt` ; `ChooseBuild`
  replanifie un bloc (`bPlanFarms`) quand le plan courant est tout bâti ; `FarmsWanted` vrai aussi quand la cible est 16 ; la dépense (`HOLAINeeds`) compte les fermes jusqu'à la cible. Rouge d'abord (stubs : 446/449), puis vert : `quick_check -Parts tests`
  **449/449** (4 tests en script). **`aiduel` 900 s** : chaque IA planifie son second bloc vers 540 s, 16 fermes + 3 moulins, population 77-82/90, 49 Paysans, **plus d'échec du contrôle d'inactivité** (seule l'empreinte reste rouge, connue) ; vagues
  (12 soldats, rapport 1,55), retraite (rapport 0,59) et défenses vues dans la même partie. Nourriture toujours basse (0-90), bois 1400-1900. Prochain : IA2.7 (essai `ai_army`).
- 2026-10-30 : **IA2.7 : scénario prêt, essai à faire** (Arwen, `ai-ia1`, commit 561dbc5). `Tools/Scenarios/ai_army.txt` + `.json` (Plain, toi contre « IA Normal 2 », 5 Paysans, 1000 de chaque ressource, notes de ce qu'il faut regarder : Caserne
  et Champ de tir, défense, vague et retraite, bâtiments attaqués). Lancement : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 ai_army -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`.
  Non lancé par moi (essai de l'utilisateur). Rien de plus codé pour les PV du bâtiment attaqué pendant une vague (journal à ajouter seulement si l'essai montre des vagues sans bâtiment détruit). Prochain : retour de l'essai, puis IA2.8 (clôture).
- 2026-10-30 : **Premier essai `ai_army` (utilisateur)** : « il ne fait rien » pendant ~7 min de partie : il fait des fermes, tuer un Paysan ou casser un bâtiment ne change rien. Journal lu : (1) **bug trouvé et corrigé** (Arwen f4eeeab pour le scénario : `waitlog` en tête ; commit suivant) :
  `WoodCostOf` ne connaissait pas `MiningCamp` (0 bois) donc le cerveau ordonnait un camp minier toutes les 0,25 s sans le bois (53 refus du serveur, un Paysan différent à chaque fois, retirés du bois alors que le bois est le goulot) ; coût lu de la
  table (`Data.MiningCamp`), test `Test_MiningCampNeedsItsWood`, 450/450, `aiduel` 480 s : 0 refus. (2) **Pas un bug mais le réglage d'IA2.4/IA2.5** : Caserne posée à 368 s, premiers soldats en file à 410 s ; avant, aucun soldat donc rien pour
  défendre ; les Paysans ne sont ni menace ni déclencheur (`Threat` = non-Paysan près d'un bâtiment) ; pas de réaction à un bâtiment attaqué ni à un Paysan tué ; bois 20-170 pendant que la nourriture s'entasse à 500-700 (même goulot que l'aiduel).
  À trancher avec l'utilisateur : armée plus tôt (Caserne dès le premier bloc de fermes, plus de bois), et réaction à l'attaque d'un bâtiment ou d'un Paysan.
- 2026-10-30 : **IA2.6d faite : armée plus tôt** (demande de l'utilisateur après l'essai, Arwen `ai-ia1`, commit après 64f22d5, pas dans `main`). Il a demandé les trois : armée plus tôt, réaction à un bâtiment attaqué, Paysans qui fuient / se défendent.
  `ArmyBuildPeasants` 16 -> 14, `ArmyMinPeasants` 20 -> 16, et **bois gardé pour le bâtiment militaire** (`SaveForArmyFood` = 200 : tant que la Caserne est voulue, que le bois manque et que le stock de nourriture est >= 200, ni moulin ni ferme du bloc
  n'est ordonné ; « saving wood ») : les fermes dépensaient le bois par 75 et la Caserne (150) ne venait jamais. `aiduel` 480 s : Caserne à ~207 s (était ~358 s), 2 soldats à 5 min, 10 à 8 min (étaient 0 et 5), 8 fermes vers 400 s ; essai en 12 Paysans :
  Caserne à 126 s mais fermes retardées (nourriture à 1) donc 14. Tests 451/451 (deux anciens tests ne codent plus 19/20 Paysans en dur ; test de la règle écrit avec le code, pas rouge d'abord). Prochain : IA2.6e (réaction à un bâtiment ou un Paysan attaqué), IA2.6f (Paysans qui fuient / se défendent).
- 2026-10-30 : **IA2.6e et IA2.6f faites** (Arwen, `ai-ia1`, commit après e3748a8, pas dans `main`). **E, bâtiment attaqué** : `FHOLAIThreat.bAttackingBuilding` (un soldat ennemi à moins de `AttackedRadius` = 800 cm d'un bâtiment ou du centre) ; `ShouldDefend` accepte alors
  un rapport de force d'au moins `DefenceMinRatioAttacked` = 0,05 au lieu de 0,3 (l'armée faible va quand même au contact quand on frappe la base ; sans aucun soldat le rapport est 0 : rien à envoyer). **F, Paysans qui fuient** : `FHOLAISummary.Peasants` (C++ : place et
  vie des Paysans, test `AI.Summary.ArmyRule` étendu), `HOLAIArmy::FleeOrders` (Paysan à moins de `FleeRadius` = 1500 cm d'un soldat ennemi, les Paysans ennemis ne font peur à personne, renvoyé au plus toutes les `FleeEvery` = 4 s) ; le cerveau (`ThinkArmy`) les envoie au
  centre, log `N Peasants run to the centre`. Rouge d'abord (stubs : 451/453 en script, `ArmyRule` rouge avant le remplissage), puis `quick_check -Parts tests,aiserver` **453/453** + aiserver PASS. **`aiduel` 900 s** : les deux armées (jusqu'à ~40 soldats) ne se rencontrent jamais
  (rapport ~1, pas de vague) donc ni fuite ni défense vues en partie : à voir à l'essai. **Limites** : les Paysans arrivés au centre sont inactifs et le cerveau les renvoie au travail, possiblement là où est l'ennemi (va-et-vient toutes les ~4 s tant qu'il reste) ;
  pas de « se défendent » (les Paysans ne frappent pas) ; fuite sans mémoire de la zone dangereuse. Prochain : nouvel essai `ai_army` de l'utilisateur, puis IA2.8.
- 2026-10-30 : **IA2.8 : clôture préparée, `main` pas avancée** (Arwen, `ai-ia1`, fusion de `main` ab7ffc4 : build PASS, `quick_check -Parts tests,aiserver,netai` **461/461** + aiserver + netai PASS). Essai `ai_army` relancé par l'utilisateur après IA2.6c-f : « c'est vachement mieux ». GDD : **D552 à D556** écrits dans `GDD.md`
  (armée et contres, rapport de force, part/défense/vague/retraite, second bloc de fermes, réaction à l'attaque), prochain numéro D557 (la table était à D552 : l'ancien « D532+ » du plan est devenu D552+ après le brouillard et le héros). **Reste** : avancer `main` à `ai-ia1`. Bloqué : la copie principale a des modifications non commitées d'une
  autre session (durées de construction : `BuildingTypes.uasset`, `HOLAIOpening.as`, `HOLAIOpening_Test.as`, `HOLAIOpeningTests.cpp`, `HOLBuildingTests.cpp`, `HOLConstructionTests.cpp`) dont des fichiers que la fusion touche aussi ; à faire quand elles sont commitées (fusion de `main` dans `ai-ia1`, nouveau test, puis `git merge ai-ia1` dans `main`), et prévenir les sessions actives.
  Limites restantes d'IA2 pour IA4 : réglage des valeurs, va-et-vient des Paysans renvoyés au travail près de l'ennemi, Paysans qui ne se défendent pas, jamais de vague en IA contre IA symétrique, cavalerie et siège sans chiffres.
- **IA2.8 close, `main` avancée (2026-10-30)** : la session des durées de construction avait commité (429b807) ; `main` refusionnée dans `ai-ia1` (un conflit dans `DefaultData`, `Farm.BuildTime = 7.5f` de main gardé avec `MiningCamp.CostWood = 100` d'IA2), build PASS, tests 461/461, aiserver et netai PASS ; `git merge ai-ia1` dans `main` en avance rapide : main = `7d27507`. GDD : D552-D556 écrits dans `GDD.md` de la copie principale mais **non commités** (le fichier porte aussi des lignes d'autres sessions : brouillard, héros, réseau) ; prochain numéro D558. Arwen libre. **IA2 est close ; prochain : IA3.**

## IA3 Fin de partie (ouvert le 2026-10-30, worktree Arwen, branche ai-ia1)

- **IA3-Q1 = A** (choix de l'utilisateur, conforme à la recommandation) : défense + attaque de la ruine + Roi protégé en Régicide. Reporté à IA4 : arbitrage fin des vies (D426 : céder une vie pour garder l'économie), aide aux coéquipiers, unité qui gèle le compte.
- Constat de départ : le résumé de l'IA ne lit ni l'état des forums (vies, phase, ruine, reconstruction, compte tenu), ni le Roi, ni l'option Régicide.
- Découpage : **IA3.1** résumé (forums propres et ennemis : vies, phase, secondes de ruine, avancement, compte ; Roi propre ; Régicide) · **IA3.2** défense (forum à terre : Paysans le reconstruisent ; dernière vie : armée rappelée sur la ruine) · **IA3.3** attaque (forum ennemi à terre : l'armée va tenir la ruine si elle domine sur place, se retire sinon ou si sa base est menacée) · **IA3.4** Régicide (Roi à l'abri, cible = Roi adverse) · **IA3.5** scénario d'essai · **IA3.6** clôture (GDD, main).
- **IA3.1 faite (2026-10-30, Arwen 7c9a0de)** : le résumé lit les forums (`FHOLAIForum` : lieu, vies, tombé/ruine/reconstruction, secondes de ruine, avancement, compte tenu, contesté ; `GatherForums` lit l'état public D353, `Make` ajoute le lieu du centre), le Roi (`bHasKing`, `King`, hors armée) et `bRegicide`. Tests C++ `AI.Summary.ForumRule` (règle) et `ForumRead` (monde de test : un forum tombe, la ruine puis la reconstruction se lisent). Tests 463/463.
- **IA3.2 faite (Arwen fcb65f6)** : `Script/AI/HOLAIForum.as` (+ `HOLAIForum_Test.as`, 7 tests) : forum tombé → Paysans envoyés le reconstruire (clic droit = `IssueAIBuildOrder` sur le forum), les plus proches d'abord ; **tous à la dernière vie** (`Lives <= 1`, ce qui est le cas dès la première chute au prototype : 2 vies au départ, pas de palier), `RebuildersSpare = 5` seulement quand il y aura des vies d'avance ; mémoire des envoyés (pas renvoyés, morts oubliés, vidée forum debout) ; ils sont retirés de la liste des Paysans libres et ne fuient pas à la dernière vie (`Calm`). `ShouldDefend` : forum tombé à la dernière vie → l'armée se bat quel que soit le rapport ; la vague est rappelée (`bRecall`). Chaîne réelle vérifiée sans fenêtre (-RenderOffScreen, scénario temporaire) : chute à 60 s → « 7 Peasants rebuild the forum (1 lives left) » → forum rebâti → les Paysans reprennent la récolte. Scénario d'essai `ai_forum` (txt + json). Tests 470/470. Limite : la défense à la ruine n'est jouée qu'en test de règle, pas encore avec des soldats ennemis réels.
- **IA3.3 faite (Arwen b0c9070)** : `HOLAIForum::EnemyRuin` / `RuinTarget` ; `CanLaunch` ne compte plus le temps minimum quand un forum ennemi est tombé (rapport de force et nombre de soldats toujours exigés) ; la cible de la vague (`WaveTarget`, lancement compris) est la ruine la plus proche, sinon le centre ennemi ; le journal de lancement dit la cible. 3 tests. **Joué en vrai** (partie sans fenêtre, forum humain abattu à ~340 s) : la vague part, « contested » puis « held by slot 1 », « forum razed, lives 0 », défaite du forum, victoire de l'IA. Limite : dans cette partie l'armée n'était prête (8 soldats) qu'à ~359 s, après le temps minimum, donc le lancement anticipé n'est vérifié qu'en test de règle.
- **IA3.4 faite (Arwen, tests 479/479)** : Régicide. `FHOLAIKingData` (poste à 2500 cm derrière le forum, à l'opposé du centre ennemi ; danger 2500 cm ; fuite 3000 cm ; ordre toutes les 3 s), `HOLAIForum::KingOrder` (fuit le soldat ennemi le plus proche s'il est dans le rayon, sinon va à son poste s'il est au repos et loin) branché dans le cerveau (`Kings`, ligne « King goes to ») ; `KingTarget` : en Régicide la vague vise le Roi ennemi connu (après une ruine, avant le centre). 5 tests. **Joué en vrai** : Roi de l'IA à son poste dès le pas 4, vague lancée à 357 s vers le Roi humain, « king of slot 0 died », défaite (King), victoire de l'IA.
- **IA3.5 scénarios** : `ai_forum` (forum de l'IA abattu par la triche, reconstruction, défense) et `ai_regicide` (Roi à l'abri, fuite, vague vers le Roi) dans `Tools/Scenarios`, avec leur .json.
- Reste IA3.6 : décisions au GDD (D558 et suivants), fusion de `main` dans `ai-ia1`, tests, avancer `main`.
- **IA3.6 préparée (2026-10-30)** : GDD D558-D561 écrits dans `GDD.md` de la copie principale (non commités, fichier partagé), prochain numéro D562 ; ai-ia1 = d55ba4d, tests 479/479, aiserver et netai PASS, aiduel : seul le rouge connu des empreintes. **Attend l'essai de l'utilisateur** (`ai_forum` puis `ai_regicide`) avant de fusionner `main` et d'avancer `main`.
- **IA3 essai 1 (2026-10-30), retours de l'utilisateur et correctifs (Arwen 2109e29, tests 482/482)** : (1) `ai_forum` : les Paysans attendaient devant la ruine (rien ne se construit en ruine, D352) -> `RebuildOrders` n'appelle un Paysan que s'il arrive à la fin de la ruine (distance / `PeasantSpeed` 800 + `ArriveMargin` 5 s >= secondes de ruine restantes), les autres continuent à travailler ; (2) `ai_regicide` : le Roi fuyait sans fin (sa vitesse 777 = Lancier, jamais rattrapé) -> il se replie seulement vers son poste (rayon de danger 1800 cm, si l'ennemi est plus loin du poste que lui), sinon il reste ; en Régicide la place du Roi est gardée (`Threat`), donc l'armée vient défendre. Question ouverte pour l'utilisateur : vitesse du Roi (777 comme le Lancier, 697 pour le Homme d'armes et le Paladin) ; stat non touchée. Attend le deuxième essai.
- **IA3 essai 1, suite (2026-10-30)** : l'utilisateur a validé « 697 » pour le Roi : vitesse 697 (animation 4,15) dans `UnitTypes.csv`, table et `UnitStats.md` régénérés, test du Roi suivi (Arwen 6958216, tests 482/482, aiserver et netai PASS), GDD D563 (révise D470) et D561 corrigé ; D562 pris par le brouillard, prochain D564. Attend l'essai 2 (`ai_forum`, `ai_regicide`) avant la fusion de `main`.
- **IA3 essai 2 (2026-10-30)** : `ai_regicide` « a l'air bon » ; `ai_forum` : les Paysans arrivaient 6-7 s avant la fin de la ruine (marge 5 s + choix toutes les 4 s) -> `ArriveMargin` 1,5 s et `OrderEvery` 1 s (Arwen, tests 482/482, aiserver PASS). Fusion de `main` suspendue (interrompue par l'utilisateur) jusqu'à la validation de `ai_forum`.
- **IA3 close (2026-10-30)** : essais `ai_forum` et `ai_regicide` validés (« bon ») ; `main` = e6fbaae (fast-forward de ai-ia1) ; tests 482/482, aiserver et netai PASS ; GDD D558-D561 et D563 écrits (non commités, GDD.md partagé), prochain D564. Roi à 697 cm/s : la table des unités a changé, les autres worktrees fusionnent `main` puis recompilent. Limites pour IA4 : Paysans sans défense, arbitrage fin des vies, aide aux alliés, unité qui gèle le compte ; défense de la ruine avec de vrais soldats et départ anticipé sur une ruine vus seulement en tests de règle. Prochain : IA4. Arwen libre.

- **IA4, premier reste : un coéquipier n'est plus un ennemi (2026-10-30, Arwen `ai-ia1`, commit 3f5f58a, pas dans `main`)**. Constat : `HOLAISummary::Make` prenait pour ennemi toute unité, tout forum et tout centre d'un autre emplacement, **coéquipiers compris** (le jeu a des équipes : `FHOLCampTable`, `Team` 1..4 de `FHOLMatchConfig`) ; en 2v2 l'IA aurait visé le centre de son allié (ordres refusés), compté ses soldats dans les menaces, et voulu « tenir » sa ruine. Correctif : `Make` reçoit une table des camps (`const FHOLCampTable& Camps`, par défaut chacun seul, donc les anciens tests ne changent pas) et en retire `Enemies`, `EnemyForums` et le centre ennemi pour l'équipe de l'emplacement ; `Read` passe `FHOLCampTable::FromWorld`. Rouge d'abord sur la vraie chaîne (monde de test avec `InitVictory` et équipes : `AI.Summary.TeammatesRead`, et la règle `TeammatesRule` ; 482/484), puis vert `quick_check -Parts tests,aiserver` **484/484** + aiserver PASS. Ce n'est que le préalable de l'« aide aux coéquipiers » (D426 : aider à contester la ruine d'un allié) : le résumé n'expose pas encore les forums et unités alliés, et le comportement d'aide reste à écrire (décision : à partir de quel rapport l'IA vient-elle aider ? reco : même règle que sa propre ruine, rapport >= 1 sur place). Restent d'IA4 : arbitrage des vies (D426, utile seulement avec des vies d'avance, donc des paliers), Paysans sans défense (décision de design : les Paysans frappent-ils ?), unité qui gèle le compte (déjà couverte à la dernière vie, qui est le cas du prototype).

- **IA4, réponses de l'utilisateur (2026-10-31, Auto_Journey_Retours.md § 2 ; GDD D571-D572)**. (1) **Aide aux coéquipiers = validé** : même règle que pour sa propre ruine (rapport de force >= 1 sur place) -> D571. (2) **Paysans sans défense** : « s'ils gagnent, de base non, mais si c'est contre un Lancier à 5 PV, ça devrait aller » -> D572, **interprétation de l'assistant, à confirmer** : un Paysan ne frappe que s'il est sûr de gagner le duel (rapport de force du duel gagnant avec marge, PV restants compris), sinon il fuit ou continue de travailler. (3) **Arbitrage des vies (D426) : A TRANCHER**, l'utilisateur veut en discuter ; rien n'est écrit comme décidé, la question reste ouverte (Auto_Journey_Retours.md § 2). Ordre du code : aide aux coéquipiers d'abord (le résumé expose forums et unités alliés), Paysans (D572) après confirmation de l'interprétation.
- **IA4 aide aux coéquipiers faite (2026-10-31, Arwen `ai-ia1`, 2665240, pas dans `main`)**. Rouge d'abord (C++ : `AI.Summary.TeammatesRule` et `TeammatesRead` étendus, 484/486 rouge), puis vert. `FHOLAISummary::Allies` (unités des coéquipiers non vaincus, hors mourants) et `AllyForums` (forums principaux des coéquipiers non vaincus, avec lieu). Script `HOLAIArmy` : `RatioAt` (rapport de force de l'armée sur les soldats ennemis à moins de `AllyHelpRadius` 3000 cm d'un lieu), `AllyHelpTarget` (forum allié tombé, avec lieu, le plus proche, où le rapport sur place >= `AllyHelpMinRatio` 1), `CanLaunch` (une ruine alliée suffit comme but, le temps minimum ne compte pas, mais soldats minimum et pas pendant la défense), `WaveTarget(Summary, Data, From, bFound)` (ruine alliée avant ruine ennemie ; **signature changée**, `Data` ajouté). Retraite de la vague inchangée (`WaveRatio` < 0,6). 4 tests de script (`HOLAIForum_Test.as`). `quick_check -Parts tests,aiserver` : 490/490 + aiserver PASS. **Limites** : testé en règle seulement (aucune partie 2v2 jouée) ; le lanceur ne vérifie pas la distance du but (comme pour la ruine ennemie) ; pas de défense d'un allié dont le forum tient encore mais est attaqué (hors périmètre de D571).
- **IA4 Paysans, D572 CONFIRMÉE (2026-10-30, réponse de l'utilisateur dans Auto_Journey_Retours.md, option A ; rien de codé)** : un Paysan de l'IA frappe seulement si le duel (rapport de force du Paysan contre cette unité, PV restants compris, D553) est gagné avec une marge >= 1,5 : il achève un Lancier à 5 PV, jamais un soldat sain ; sinon il fuit ou continue de travailler. Écartées : B (aussi en surnombre devant un voisin ou un bâtiment attaqué) et C (jamais). GDD D572 corrigée (« à confirmer » retiré). Tâche dans la file d'Arwen (`Auto_Journey_File.md`), test rouge d'abord ; valeurs (marge 1,5) réglables aux parties IA contre IA.

- 2026-10-30 19:10 (réponse de l'utilisateur, D426 -> D579) : l'arbitrage des vies dépend de la difficulté de l'IA ; plus elle est dure, plus elle pense long terme. Facile : sacrifie toujours l'armée, même perdue d'avance. Normal : feintes (reculer, contourner), sacrifie l'armée mais pas l'économie, refait économie et armée plus loin dès qu'elle sait perdre. Difficile : snipe les faibles, évalue sa perte, peut laisser sa seconde vie pour garder assez d'armée et écraser l'adversaire avant la capture, puis punit chez l'attaquant. Niveaux : Facile, Normal, Difficile, évolutive (D422/D329). Tâche en file pour Arwen (après les Paysans), réglages à régler en test ; journaliser chaque situation « forum attaqué » (vies, rapport de force, décision).

- **IA4 Paysans faite (2026-10-30 20:35, Arwen ai-ia1, commit 5cf3d39 puis fusions de main ; D572 confirmee, option A)**. Rouge d'abord sur la vraie chaine : C++ AI.Orders.AttackUnit (nouvel ordre UHOLAIOrderLibrary::IssueAIAttackUnitOrder : stub faux, 531/532 rouge) puis script HOLAIPeasantStrike_Test.as sur les tables reelles (LoadFigures lit aussi les chiffres du Paysan ; stubs : 3 tests rouges sur 4). Regle (HOLAIArmy.as) : DuelRatio = Matchup (brique du rapport de force D553) du Paysan contre le soldat avec les PV RESTANTS des deux ; StrikeTargetFor = soldat ennemi le plus proche dans FleeRadius dont le duel >= StrikeMargin 1,5 ; StrikeOrders (au plus toutes les StrikeEvery 4 s par Paysan) ; FleeOrders ne fait plus fuir un Paysan qui frappe ; le cerveau (HOLAIBrain.as) envoie l'ordre IssueAIAttackUnitOrder et journalise Peasant N finishes soldier M off. Avec la table : Paysan 50 PV, 3/3,7 s ; Lancier 80 PV, 8/1,875 s : un Lancier a 5 PV donne ~1,9 (frappe), a 10 PV ~0,95 (non), sain ~0,12 (non). Tests 549/549 + aiserver + netai (netai lent : attend d'autres sessions). **Limites** : jamais vu en vraie partie (aucun scenario) ; le duel ne regarde que le soldat vise, pas les autres soldats proches ; pas de poursuite au-dela de FleeRadius. Valeurs (marge 1,5, portee) a regler en parties IA contre IA. Reste IA4 : arbitrage des vies D579.
- 2026-10-30 20:52 : **IA4 arbitrage des vies D579, sous-etape 1 faite** (Arwen ai-ia1, commit 9d69bd1, dans main). Regle PURE dans HOLAIForum.as : EHOLAILifeCall (Rules / Fight), FHOLAIForumSituation (attaque, vies, rapport, nombre d'ennemis, decision), Judge (forum attaque = soldat ennemi a moins de AttackedRadius 800 cm du forum, ou forum tombe avec menace ; Facile = Fight, Normal/Difficile/Evolutive = Rules), DefendAnyway, Describe, ShouldLog (journal a chaque changement de vies/decision, et a la fin). Cerveau (HOLAIBrain.ThinkArmy) : journalise la ligne forum attacked: lives N, X enemies, ratio R, level L, decision D et ajoute DefendAnyway a ShouldDefend. Rouge d'abord sur les tables reelles (HOLAIForumLives_Test.as, LoadFigures, rapport via HOLAIArmy::Threat : 3 tests rouges sur 4, 561/564), puis vert : quick_check -Parts tests,aiserver 564/564 + aiserver PASS. **Limites** : le niveau de l'emplacement n'est PAS lu par le cerveau (LevelOf renvoie Normal : Facile n'est joue qu'en test ; il faut Level dans FHOLAISummary cote C++ depuis FHOLMatchConfig.Slots[i].Difficulty, test rouge d'abord) ; rien vu en vraie partie ; la vague en route n'est pas rappelee par Facile (question A/B dans Retours section 2). Normal, Difficile, Evolutive : lignes [ ] dans Auto_Journey_File.md.

- 2026-10-30 21:4x (réponses de l'utilisateur, enregistrées par Auto_Journey, **rien de codé** ; GDD D584 et D585) : (1) **Paysans qui achèvent un soldat (D572) : B** : un Paysan n'achève le blessé que si **aucun autre soldat ennemi sain n'est à moins de ~5 m du blessé** (il n'achève que les blessés isolés : traînards, fuyards). A (seul le blessé compte) et C (B sauf si un soldat de l'IA est déjà au contact) écartées. Correction en file d'Arwen : `StrikeTargetFor` de `Script/AI/HOLAIArmy.as`, test rouge d'abord sur les tables réelles (blessé isolé : frappe ; blessé au milieu de trois sains : ne frappe pas). (2) **Arbitrage des vies D579, Facile : B** : Facile rappelle aussi la vague d'attaque dès que le forum est attaqué (plus fidèle à « sacrifie toujours l'armée pour défendre »), plus lourd à tester (retour d'une vague en route). Et **le niveau de chaque IA est lu dans le cerveau, sans autre décision** (`difficulty` de la config, `FHOLMatchConfig.Slots[i].Difficulty` -> `Level` dans `FHOLAISummary`, test rouge d'abord) ; l'essai Facile par un scénario `.json` (`"difficulty": "Easy"`) reste possible. Lignes `[ ]` en tête de la file d'Arwen : lecture du niveau d'abord, puis le rappel de la vague, puis les Paysans.

- **aiduel rouge CORRIGE (2026-10-31, Arwen ai-ia1, 25bbd81, pas dans main)** : la cause n'etait ni le script de test ni le pilotage (l'ancienne piste du 2026-10-29, `HOLParallelSteerToMoveTargetProcessor`, etait fausse ou depassee : J7c.4 a rendu le pilotage deterministe) mais la **reconstruction des tuiles du navmesh sur des fils de travail** : les empreintes des deux courses sont identiques jusqu'au pas 30, puis la ligne `Buildings (D310): footprint MainCentre ...` dit `tiles rebuilt after 31 steps` dans une course et `after 33 steps` dans l'autre (et `1 walkers given a new path` contre `0`), donc le pas ou `UpdateNewFootprints` voit les tuiles reconstruites depend de l'horloge. Rouge d'abord : `quick_check -Parts aiduel -DuelSeconds 30` rouge (premiere difference au pas 31). Correctif : cvar `hol.Simulation.SyncNavBuild` (defaut 0 = le jeu reste asynchrone) ; a 1, `UHOLUnitOrderProcessor::Execute` appelle `ANavigationData::EnsureBuildCompletion()` a chaque pas avant `UpdateNewFootprints` : une tuile soumise par le tick de navigation d'un pas est toujours vue reconstruite au pas d'ordre suivant (journal : `tiles rebuilt after 0 steps` partout). `quick_check.ps1` (`Invoke-AIDuelRun`) met `hol.Simulation.SyncNavBuild 1` en tete de sa sequence. Vert : `aiduel` 30 s, 120 s (defaut) et 480 s (armees, combat) : empreintes identiques sur toutes les courses. **Limite** : le jeu reel (cvar a 0) n'est pas reproductible d'une course a l'autre par ce biais ; sans consequence tant qu'on ne veut ni rejeu ni lockstep.

- **IA4 D579 lecture du niveau FAITE (2026-10-31, Arwen ai-ia1, d2f4f42)** : `FHOLAISummary::Level` (EHOLAIDifficulty) lu par `HOLAISummary::Read` depuis `GetLaunchConfig().Slots[i].Difficulty` (Normal hors table) ; `HOLAIBrain.LevelOf` supprime, `ThinkArmy` lit `Summary.Level`. Rouge d'abord sur la vraie chaine (monde de test + InitVictory, `AI.Summary.LevelRead` : 576/577), puis 577/577 + aiserver PASS. Facile est donc joue en vraie partie des qu'un `.json` porte `"difficulty": "Easy"` (menu et salon : IA Facile). Limite : pas encore vu dans une partie reelle (scenario Easy a faire avec D585).
- **IA4 Paysans D584 FAITE (2026-10-31, Arwen ai-ia1)** : option B de l'utilisateur. `FHOLAIArmyData.StrikeIsolation` = 500 cm (reglage) ; `HOLAIArmy::IsIsolated` : le blesse n'est achevé que si aucun AUTRE soldat ennemi sain (duel du Paysan < `StrikeMargin`) n'est a moins de 500 cm de lui ; un autre blesse a cote n'est pas une escorte, un Paysan ennemi non plus. Rouge d'abord sur les tables reelles (`Test_PeasantStrikesOnlyAnIsolatedWounded` : blesse seul = frappe, entoure de 3 sains = non, 1 sain a 450 cm = non, a 550 cm = oui, deux blesses = oui, rayon 700 = non ; 577/578), puis 578/578 + aiserver PASS. Deux anciens cas du test `PeasantStrikesOnlyAWonDuel` deplaces (le blesse etait a 400 cm du sain). Limite : jamais vu en vraie partie.
- **IA4 D585 FAITE (2026-10-31, Arwen ai-ia1)** : Facile rappelle aussi la vague (option B). `HOLAIForum::WaveRecalled(Summary, Data, Whole, Situation)` = ancienne regle (menace a la maison >= `AttackRecallCount` 3, ou urgence du forum tombe) OU `DefendAnyway` (forum attaque et decision Fight, donc Facile) ; `ThinkArmy` l'appelle. Rouge d'abord (stub faux : 578/579, trois assertions), puis `Test_LivesEasyRecallsTheWave` sur les tables reelles et le vrai `UpdateWave` : vague de 10 Lanciers dehors, 2 Hommes d'armes au forum ; Facile -> recule (`Back`), Normal -> reste dehors, sans menace -> rien, 3 ennemis -> Normal rappelle encore. Chaine reelle : `quick_check` aiserver met l'IA du slot 1 en `"difficulty": "Easy"` (`Write-AIMatchConfig -Levels`) et exige la ligne `AI slot 1 step N: level Easy` (nouvelle ligne du cerveau, une fois par emplacement) ; `-Parts tests,aiserver,aiduel` : 579/579, aiserver PASS (nouveau controle de niveau compris), aiduel PASS. **NON VERIFIE** : le comportement Facile (defense + rappel de la vague) n'a jamais ete vu dans une vraie partie IA contre joueur ; le controle du niveau dans aiserver n'a pas ete vu rouge avant le code (ajoute avec lui).
- **IA4 : BUG TROUVE EN VRAIE PARTIE et corrige (2026-10-31, Arwen ai-ia1, aa9c14a)** : une sonde sur le scenario `ai_easy` (IA Facile, 4 Lanciers du joueur a 300 cm de son centre, qui frappent le Centre principal) ne produisait AUCUNE ligne « forum attacked » ni de defense : le Centre principal fait 1600 cm de large (`BuildingTypes.csv`), donc un soldat qui frappe son MUR est a ~900 cm du point du centre, au-dela de `AttackedRadius` 800. Meme angle mort pour `bAttackingBuilding` (IA2.6e) sur le centre. Correctif : `FHOLAIArmyData.ForumHalfSize` = 800 ajoute a `AttackedRadius` pour le centre seulement (`Threat` : portee par lieu gardé ; `Judge`). Rouge d'abord (`Test_LivesTheForumWallIsAttacked`, valeur a 0 : 580/581), vert 581/581 + aiserver PASS. **Revu en vraie partie** (sonde sans fenetre, `hol.Units.Spawn 4 15200 0 0 Spearman 300`) : `forum attacked: lives 2, 4 enemies, ratio 0.0, level Easy, decision fight` puis `defence begins (4 enemies, ratio 0.0)` ; les Paysans fuient (« N Peasants run to the centre » repete toutes les ~1 s, connu). Piege de test : un lancement manuel d'Unreal doit poser `UE_SKIP_UBT_SDK_SETUP=1` (play.ps1 le fait), sinon il attend les compilations des autres sessions. Scenario d'essai `Tools/Scenarios/ai_easy` (txt + json) : demarre hors ecran sans erreur, `level Easy` lu. **NON VERIFIE** : le rappel de la vague par Facile (D585) en vraie partie (la sonde n'a pas lance de vague), les Paysans D584 en vraie partie.
- **IA4 D579 Normal, Difficile, Evolutive FAITES (2026-10-31, Arwen ai-ia1, commit 26a14ee)** : nouveau `Script/AI/HOLAILives.as` (namespace `HOLAILives`, reglages `FHOLAILivesData` dans `FHOLAIArmyData.Lives`, tous provisoires, a regler en test) + tests `HOLAILives_Test.as` sur les tables reelles (`LvData` = `LoadFigures`). Decisions de `HOLAILives::Weigh` (appelee par `HOLAIForum::Judge`, `EHOLAILifeCall` gagne `Feint` et `Let`) : **Facile** = `Fight` (inchange) ; **Normal** : avec une vie d'avance (`Lives >= 2`) et un rapport de force sous `NormalLoseRatio` 0,3 = `Feint` : l'armee ne charge pas, `FeintStep` la fait reculer au refuge (`RefugePoint` : `RefugeDistance` 4000 cm derriere le centre, a l'oppose des attaquants) pendant `FeintBackSeconds` 6 s, puis contourner (`FlankPoint`, a `FlankDistance` 2500 cm des attaquants, a angle droit) pendant `FeintAroundSeconds` 10 s, puis frappe (les regles defendent) ; ses Paysans qui fuient vont au refuge et non au centre attaque (`FleeTarget`, reglage `LossPlan` Withdraw/StayAndFlee, question IA4-Q1) ; **Difficile** : `ExpectedLoss(rapport)` (carre de Lanchester : perte = 1 - sqrt(1 - 1/R^2), 1 pour R <= 1) ; avec une vie d'avance, si la perte depasse `HardMaxLoss` 0,5 ET que son armee garde `HardCrushRatio` 1,5 sur les ennemis qui ne sont PAS a sa base (`HomeRatio`, garde du camp de l'attaquant) ET qu'un centre ennemi est connu = `Let` : elle ne defend pas (le brain annule `ShouldDefend`), `SnipeOrders` envoie ses soldats oisifs tuer l'ennemi isole dont le duel (PV restants) est gagne avec `SnipeMargin` 1,5 a moins de `SnipeRange` 3000 cm (ordre `IssueAIAttackUnitOrder`), et `FHOLAIWave.bForce` lance la vague tout de suite vers le centre ennemi (`CanLaunch` : sans minimum de temps, de soldats ni de rapport) ; la vague `bPunishing` n'est pas rappelee par les ennemis a la base ; reglage `bHardLetsLifeGo` (question IA4-Q2) ; **derniere vie : regle commune, aucune feinte, aucun Let** ; **Evolutive** : `HOLAILives::Effective(niveau, temps)` = Facile jusqu'a `EvolveNormalAt` 600 s, Normal jusqu'a `EvolveHardAt` 1500 s, Difficile ensuite (seule la gestion du forum attaque suit la courbe pour l'instant ; macro, agression, micro et pouvoirs de D329 restent a faire) ; ligne du journal `evolving, plays X now` aux changements. Journal : `forum attacked: ... decision feint / let the life go, the army is kept (loss L, over the home R)`, `feint, phase N`, `snipes enemy`, `punishes the attacker's home`, `Peasants run to the refuge`. **Rouge d'abord** (reglages neutres : 8 tests rouges sur 589, puis reglages reels : 1 reste rouge par un test mal regle, 40 Lanciers contre 1 Homme d'armes pour un combat bon marche, puis 589/589). `quick_check -Parts tests,aiserver,aiduel` : 589/589, aiserver PASS, aiduel PASS (111 s). Sonde sur le vrai jeu (hors ecran, scenario `ai_normal` + 4 Lanciers du joueur au forum, IA sans armee) : `level Normal`, `forum attacked: lives 2, 4 enemies, ratio 0.0 ... decision feint`, `feint, phase 1/2/3` a 0 s / 6 s / 16 s, `Peasants run to the refuge` repete. Scenarios d'essai : `ai_normal`, `ai_hard`, `ai_evolving` (txt + json). **NON VERIFIE** : le mouvement reel de l'armee (recul, contournement, frappe : la sonde n'avait aucun soldat), le snipe et la vague de punition en vraie partie, la courbe Evolutive en vraie partie (600 s, 1500 s), le GDD ne decrit pas les valeurs (tout est reglage) ; les Paysans qui ont fui au refuge redeviennent oisifs et le cerveau les renvoie travailler pres des attaquants (comportement deja connu pour le centre). **Questions posees** (Retours section 2) : IA4-Q1 (que veut dire « plus loin » : pas de centre secondaire dans la table des batiments) et IA4-Q2 (Difficile laisse-t-il tomber un forum volontairement).
- **IA4 D579 : 3 DEFAUTS TROUVES EN VRAIE PARTIE (sondes hors ecran sur `zz_probe_hard` / `zz_probe_normal` = ai_hard / ai_normal + 4 Lanciers de l'IA + 6 Hommes d'armes du joueur au forum) et corriges (2026-10-31, Arwen ai-ia1)** : (1) la vague de punition de Difficile partait, voyait les attaquants a cote d'elle (rapport 0,08), reculait et repartait 5 s apres, en boucle (journal : `attack launched` / `retreat` / `wave over, home` toutes les 5 s) -> rouge d'abord `Test_LivesHardPunishingWaveLeaves` (3 assertions), puis `CanLaunch` : le lancement force respecte `NextLaunch` (cooldown), et `UpdateWave` : une vague `bPunishing` encore a `DefenceRadius` du centre ne recule pas sur le rapport (elle laisse les attaquants derriere elle) ; apres : une seule vague, `wave out size 4` ; (2) la feinte de Normal etait defaite dans la meme pensee par `RegroupOrders` (`regroup 4 soldiers to (13700, 0)` = le point de ralliement, au milieu des attaquants) -> le brain ne regroupe plus tant que l'armee est retenue (`bHeld`) ; apres : pas de `regroup`, les soldats marchent au refuge (`Block march ... 3469 cm`), contournent (7201 cm), `feint, phase 3` a 16 s puis `defence begins` : l'armee de 4 Lanciers meurt face a 6 Hommes d'armes pendant que les 5 Paysans survivent (7 a la fin) : « sacrifie l'armee mais pas l'economie » vu ; (3) pas de defaut pour `ai_evolving` (`level Evolving`, `evolving, plays Easy now`, scenario pret). `quick_check -Parts tests,aiserver,aiduel` : 590/590, aiserver PASS, aiduel PASS. **Toujours NON VERIFIE** : le snipe (`snipes enemy`) en vraie partie (aucun blesse isole dans les sondes), la courbe Evolutive 600 s / 1500 s, la punition jusqu'au bout (la vague de 4 etait encore en route a 50 s).

- **IA4 D579 : test unitaire du correctif Held / regroupement (2026-10-31, Arwen ai-ia1)** : le correctif (pas de regroupement tant que l'armee est retenue par la feinte ou la vie cedee) n'avait que la sonde en vraie partie. La decision est extraite en regles pures dans Script/AI/HOLAILives.as (ArmyHeld(Situation, Feint) et RegroupUnlessHeld(State, Summary, Data, Assembly, bHeld, Now), appelees par le cerveau a la place du test en ligne) et testee par Test_LivesNoRegroupWhileTheArmyIsHeld (HOLAILives_Test.as) sur la vraie chaine Judge -> FeintStep -> ArmyHeld avec les tables reelles : feinte phase Back et phase Around = aucun ordre et personne de marque ; phase Strike = regroupement ; vie cedee (Difficile, Let) = aucun ordre ; Facile ou forum non attaque = regroupement. **Rouge par mutation** : fonction sans la garde -> 590/591 (le test seul), avec la garde -> 591/591. quick_check -Parts tests,aiserver,aiduel : 591/591, aiserver PASS, aiduel PASS. **NON VERIFIE** : que le cerveau passe bien Held (la ligne d'appel n'est couverte que par la sonde en vraie partie deja faite avant l'extraction ; un alse en dur dans HOLAIBrain.as ne ferait rouge aucun test).

- **IA4 D579 : le correctif Held revu en vraie partie APRES l'extraction (2026-10-31, Arwen ai-ia1)** : sonde sans fenetre (scenario temporaire ai_normal + 4 Lanciers de l'IA au refuge + 6 Hommes d'armes du joueur au forum, supprime ensuite) : orum attacked ... decision feint, eint, phase 1 (pas 9), phase 2 (pas 129), phase 3 puis defence begins (pas 329), les Paysans au refuge, et AUCUNE ligne egroup N soldiers pendant la feinte (le compteur home regroups reste a 0). Donc le cerveau passe bien Held a RegroupUnlessHeld ; le trou signale plus haut (un alse en dur ne ferait rouge aucun test unitaire) est couvert par cette sonde, pas par un test.

- **IA : pas de Caserne en 10 min, Paysans qui oscillent aux baies (2026-11-01, Arwen ai-ia1, commit a99bb7a)** : retour d'essai `ai_easy` (aucune Caserne ni Archerie, Paysans qui refuient aux baies en boucle). Reproduit en partie sans fenetre (serveur, humain sans client + IA Facile, 1000 de chaque ressource, 6 Hommes d'armes ennemis poses pres des baies a 20 s ; sonde jetable hors depot) : a 10 min l'IA avait 10 Paysans, 1 Maison, 0 Caserne, pop 10/10, 1640 nourriture en stock. **Cause 1** : le bâtisseur de la Maison avait fui les soldats ; la fondation restait, comptee par `PlacesComing` comme « maison a venir » pour toujours : plus de Maison demandee, population bloquee a 10, jamais les 14 Paysans (`ArmyBuildPeasants`) d'une Caserne. **Cause 2** : les Paysans qui fuient (`FleeOrders`) arrivent inactifs, `SplitGather` les renvoie a la meme source (les soldats y sont toujours) : va-et-vient. **Corrections** : (1) `HOLAIOpening::ResumeSite` + `FHOLAISiteMemory` (le cerveau retient la progression de chaque site) : un site dont la progression n'a pas bouge depuis `StalledSeconds` (20 s) recoit un Paysan (`IssueAIBuildOrder`, pas deux fois de suite, ligne du journal `builds the abandoned site ... again`) ; (2) `OpenSummary` / `SafePartOf` : une source a moins de `FleeRadius + 300` cm d'un soldat ennemi n'est ni donnee aux Paysans libres ni choisie pour le rassemblement des nouveaux. **Tests rouges d'abord** (`Test_NoGatheringAtTheSourceAnEnemyStandsBy`, `Test_AbandonedSiteIsBuiltAgain`, dans HOLAIOpening_Test.as : 758/760 puis 760/760). **Meme partie rejouee apres correction** : Maison reprise, Caserne a 5 min 40 (pas 10 min sans), 34 Paysans, armee, Archerie a 10 min ; `quick_check -Parts tests,aiduel,aiserver` PASS. **NON VERIFIE** : les essais `ai_easy`/`ai_normal`/`ai_hard` en 3 min a l'ecran (non refaits) ; Normal (feinte, Paysans au refuge) rejoue ensuite avec les memes soldats aux baies : Caserne a 4 min, Archerie a 5 min 30, 34 Paysans, armee (le rendu de la feinte a l'ecran n'est pas revu) ; des Paysans sans source sure restent inactifs au centre tant que l'ennemi est la (journal `idle -> no source`) ; le cas d'un bâtisseur lent (> 20 s sans progres) envoie un second Paysan, inoffensif.

- **IA : toutes les sources sous l'ennemi (2026-11-01 13:0x, Arwen ai-ia1)** : le cas non teste du correctif des Paysans (soldats ennemis aux baies ET au bois : toutes les sources a moins de FleeRadius + 300 cm) : OpenSummary ne laisse aucune source, SplitGather ne rend aucun groupe, les Paysans restent inactifs au centre (journal `idle -> no source`) sans limite de temps. Le GDD et ce journal ne tranchent pas : **comportement NON change**, test `Test_AllSourcesUnderTheEnemyLeavesThePeasantsIdle` (HOLAIOpening_Test.as, vert d'emblee puisqu'il fige l'existant, ce n'est pas un test rouge) ; question IAQ-S posee dans Auto_Journey_Retours.md section 2 (A attendre / B reprendre apres 30 s a la source la plus loin / C comme B sous conditions ; recommandation B). quick_check -Parts tests,aiduel : 761/761, aiduel PASS. **NON VERIFIE** : le comportement en vraie partie avec deux soldats immobiles (non rejoue) ; que A bloque vraiment une IA Facile des minutes (affirme dans la question, non mesure) ; la mutation du test (supprimer SafePartOf le fait-il rouge ? non essaye).
- **Retours d'essais d'IA (2026-11-01, Auto_Journey, agent de docs A)** : réponses de l'utilisateur aux essais refaits du 2026-11-01, tous vus par lui : `ai_easy` (IA Facile : son forum est attaqué, toute son armée sort) = « Très bien » ; `ai_normal` (IA Normale : elle feinte au lieu de charger) = « Très bien ». Les deux sont validés, rien à changer. `ai_hard` (IA Difficile : elle devait laisser tomber la vie du forum pour garder son armée, essai VU À L'ÉCRAN) = « pas du tout, il a fight mes troupes » : l'IA a combattu les troupes de l'utilisateur au lieu de laisser tomber la vie du forum et de garder son armée (D610). **Rattachement CORRIGÉ le 2026-11-01** : la première version de cette entrée attribuait par erreur cette réponse à `ai_evolving` (essai LOGIQUE SEULEMENT, sans réponse). **À instruire comme un BUG** sur `ai_hard` (ligne `[ ]` en tête de la section Arwen de `Auto_Journey_File.md`) : relire les lignes `forum attacked ... level X, decision Y` du journal, trouver pourquoi `fight` sort au niveau Difficile (rapport de force D553, armée ramenée avant l'arrivée de l'attaquant ?). Rouge d'abord sur les tables réelles. La question EV-Q1 (comment avancer le temps d'un essai) est tranchée plus bas (B, D620).
- 2026-11-01 (Auto_Journey, agent de docs B) : **IA4-Q1 = A et IA4-Q2 = A, réponses de l'utilisateur** (enregistrées au GDD, D609 et D610). **IA4-Q1 = A** : Normal = `Withdraw` (les Paysans menacés courent à un refuge à 40 m derrière le Centre, rien n'est bâti ailleurs avant D384) ; la note « à corriger : les Paysans oscillent entre baies et forum » est faite (a99bb7a, 2026-11-01). **IA4-Q2 = A** : Difficile peut laisser tomber un forum (`bHardLetsLifeGo` vrai, seuils 0,5 et 1,5 à régler en essai). Rien à coder.
- 2026-11-01 (Auto_Journey, agent de docs B) : **IAQ-S : réponse PARTIELLE de l'utilisateur, la question reste OUVERTE** (aucune décision GDD, aucun D). Texte fidèle : « les paysans vont vers une source potentiellement non exploité, les 8 paysans des fermes iront reconstruire leur moulin et ferme plus loin, a l'abri. Attention quand même car avec les garnisons, ca va changer ». Ce n'est exactement ni A, ni B, ni C. Lu ainsi : (1) l'attente au centre (A) est refusée : les Paysans sans source sûre repartent vers **une source potentiellement non exploitée** ; (2) les Paysans des fermes (8 dans l'exemple) **reconstruisent leur moulin et leur ferme plus loin, à l'abri** (une construction qui n'existe pas aujourd'hui dans le cerveau) ; (3) le tout sera à revoir quand les garnisons arriveront. **Précision à demander à l'utilisateur** : quand cela se déclenche et après combien de temps ; ce que « potentiellement non exploitée » désigne (la source la plus loin de tout soldat ennemi, comme B, ou n'importe quelle source non utilisée même sous 1800 cm d'un soldat) et ce qui se passe si elle est elle aussi menacée (le rayon de fuite de 1500 cm) ; si la reconstruction du moulin et des fermes ne vaut que lorsque les fermes sont détruites ou menacées, où est « à l'abri », et à quel coût ; si les Paysans des fermes comptent parmi les « 8 » des fermes ou parmi tous les Paysans libres.
- 2026-11-01 (Auto_Journey, agent de docs C) : **trois réponses de l'utilisateur enregistrées**. (1) **Rattachement corrigé** : la réponse « pas du tout, il a fight mes troupes » concernait l'essai `ai_hard` (l'IA Difficile devait laisser tomber la vie du forum pour garder son armée, D610, et elle a combattu), pas `ai_evolving` ; l'entrée « Retours d'essais d'IA » plus haut, la ligne `[ ]` d'Arwen (`Auto_Journey_File.md`) et `EtapeDefini.md` sont corrigées. À instruire comme un BUG sur `ai_hard` : journal `forum attacked ... level X, decision Y` (niveau effectif, rapport de force D553, armée ramenée avant l'attaquant) ; piste à vérifier d'abord : des soldats de l'IA à moins de 16 m de ceux du joueur se battent d'eux-mêmes par l'agressivité des unités (voir `NetPlaytestInProgress.md`), donc la mêlée peut venir de la position de départ et non de la décision. (2) **IAQ-S, précision 1/4 = B d'origine** (GDD D619) : quand toutes les sources sont à moins de 1800 cm d'un soldat ennemi, les Paysans libres de l'IA repartent **après 30 s d'inactivité vers la source la plus éloignée de tout soldat ennemi** (plus d'attente sans limite au centre) ; premier test rouge : adapter `Test_AllSourcesUnderTheEnemyLeavesThePeasantsIdle` (inactifs avant 30 s, envoyés à la source la plus loin après). **Restent ouvertes, sans décision** (Retours § 2, 3 précisions) : la reconstruction du moulin et des fermes « plus loin, à l'abri » (quand, où, aux frais de qui), les 8 Paysans des fermes (compris ou non dans les Paysans libres), les garnisons ; **rien n'est codé de la reconstruction**. (3) **EV-Q1 = B** (GDD D620) : commande `hol.AI.SetEvolveTimes <normal> <hard>` qui change le stade de l'IA Évolutive à la demande (Entrée = stade suivant dans l'essai) et ligne HUD « niveau de l'IA » pour que l'essai se voie à l'écran ; code dans le cerveau de l'IA (ligne `[ ]` d'Arwen) ; `ai_evolving` est ensuite à refaire par Glorfindel (ligne `[B]` de sa file).
- 2026-11-01 (Auto_Journey, agent de docs) : **IAQ-S, les trois précisions restantes TRANCHÉES (réponses de l'utilisateur, conformes aux recommandations A, B, A ; GDD D621)**. **IAQ-S-2 = A** : l'IA rebâtit son moulin et ses fermes **seulement quand l'un d'eux est détruit** ; « à l'abri » = près du refuge de 40 m derrière le Centre (D609) ; aux frais normaux de l'IA (elle attend d'avoir les ressources, pas de réserve). **IAQ-S-3 = B** : les 8 Paysans des fermes sont **compris parmi les Paysans libres** : sans ferme debout, après 30 s ils repartent vers la source la plus loin de l'ennemi (D619), et reviennent bâtir quand IAQ-S-2 le permet. **IAQ-S-4 = A** : coder IAQ-S-2 et IAQ-S-3 **maintenant**, à revoir quand les garnisons arriveront (le script est facile à changer). Effet : la reconstruction est à coder (ligne `[ ]` d'Arwen dans `Auto_Journey_File.md`, `Script/AI/HOLAIBrain.as` et `Script/AI/HOLAIOpening.as`), test rouge d'abord sur les tables réelles ; la ligne IAQ-S précision 1/4 (D619, règle des 30 s) reste valable et s'étend aux Paysans des fermes. La section « IAQ-S » est supprimée de `Auto_Journey_Retours.md` § 2 (plus aucune question ouverte). Rien n'est encore codé de la reconstruction.

- **IA : outil d'essai d'IA, EV-Q1 = B (D620) FAIT (2026-11-01 17:12, Arwen ai-ia1, 7dadefd, séance interactive)** : commande `hol.AI.SetEvolveTimes <normal> <hard>` (temps de jeu en secondes ; sans valeur ou `reset` = temps de `FHOLAILivesData`), C++ `UHOLAIToolsLibrary` (`Source/HeroesOfLiberty/AI/HOLAITools.h/.cpp`, namespace `HOLAITools` en script) ; règle `HOLAILives::Played` (Effective sur les temps posés), appelée par le cerveau à la place d'`Effective` (HOLAIBrain.as, 2 endroits) ; ligne HUD `HOLAILives::LevelLine` « IA 3 : Évolutive, joue Normal » dans `HOLGameScreenScript.as` (`RefreshAILevel`, en haut à gauche sous la barre du haut, **affichée seulement tant que les temps sont posés** : provisoire, pas tranché si elle doit toujours se voir ; hol.HUD.Report ligne `ai level:`). Tests rouges d'abord (stub, 774/776) puis verts : `Test_LivesEvolveTimesMoveTheStageWithoutTheClock` (horloge à 50 s, temps 100/200 = Easy donne l'armée, 10/200 = Normal feinte, 10/20 = Difficile cède, sur les tables réelles) et `Test_LivesLevelLineShowsTheStagePlayed` ; build PASS, `quick_check -Parts tests,aiserver` 776/776 + PASS. **NON VÉRIFIÉ** : la ligne HUD à l'écran (position, lisibilité, chevauchement avec la barre du haut), la commande tapée dans la console d'une partie réelle, la mutation du test.

- **Essai `neutral_color` validé par l'utilisateur (2026-11-01, relayé par le coordinateur)** : réponse « works » : la couleur grise des neutres (propriétaire 8, `GetOwnerLinearColor`, a99bb7a/9f3b-série du 2026-11-01) est validée À L'ÉCRAN ; l'item est supprimé de `Auto_Journey_Retours.md`. Rien à changer.

- **BUG `ai_hard` « il a fight mes troupes » INSTRUIT (2026-11-01 17:3x, Arwen ai-ia1, 2a2fd83, séance interactive)** : le défaut est dans le SCÉNARIO, pas dans la décision. Journal : `forum attacked: lives 2, 8 enemies, ratio 0.092, level Hard, decision let the life go` (juste, niveau effectif Difficile, rapport de force D553 = 0,09, perte 1,0, ratio sur le foyer ennemi 1000) puis `punishes the attacker's home`, `attack launched with 6 soldiers`. Mais le scénario posait les 6 Lanciers près de l'Archerie (~12000, 2500, à 3800 cm du Centre) : l'IA les regroupe d'abord au point de ralliement (13700, 0) et les 8 Hommes d'armes du joueur étaient posés ((13812, -112)) PILE sur ce point. La vague punitive, partie de plus de DefenceRadius (3000) du Centre, voit les attaquants à moins de 3000 cm, retombe sous le ratio de retraite (0,12 < 0,6), `retreat` : le repli va au point de ralliement, donc sur les attaquants : mêlée (Lanciers tués, mesuré avec `hol.Units.Where` chaque seconde). Hypothèse « soldats de départ à moins de 16 m » : confirmée en partie (point de ralliement), la décision n'est pas en cause : le test rouge demandé (« décision let ») est vert d'emblée. **Correction** : scénario `ai_hard` (Lanciers rassemblés devant le forum au point de ralliement, attaquants posés DERRIÈRE le forum, captures recadrées, `.look` mis à jour) : sondes jetables avant/après, 0 combat, les 6 Lanciers partent en colonne vers le Centre du joueur, PNG ouverts et vus. **Laissé tel quel (pas décidé)** : le repli d'une vague punitive déjà sortie va toujours au point de ralliement (les attaquants peuvent s'y trouver) ; une tentative de repli vers le refuge a été essayée puis ANNULÉE (elle traverse les attaquants dans cette disposition, et le test `Test_LivesHardPunishingWaveLeaves` du 2026-10-31 fixe explicitement « loin de la base, la règle habituelle s'applique »). Retours : essai `ai_hard` refait listé (VU À L'ÉCRAN). **NON VERIFIE** : partie naturelle ; scénario joué par un humain.

- **`ai_hard` CLOS (2026-11-01 17:40, réponse de l'utilisateur « ça a l'air bon », relayée par le coordinateur)** : essai refait validé À L'ÉCRAN ; cause = scénario (Lanciers posés sur le point de ralliement où arrivaient les Hommes d'armes), décision `let` juste. Reste la **question AIQ-H** (Retours § 2, A/B/C, provisoire A = rien ne change) : le repli d'une vague punitive déjà sortie va au point de ralliement, donc vers des soldats du joueur qui se tiennent devant le forum.
- **IAQ-S précision 1/4 = B (D619) FAITE (2026-11-01 17:4x, Arwen ai-ia1, bc5f388)** : `HOLAIOpening::StrandedGroups` (+ `FarthestGroups`, `EnemyDistanceOf`, `FHOLAIStranded`, `StrandedSeconds` = 30) : quand toutes les sources sont sous un soldat ennemi (`OpenSummary` vide), les Paysans libres attendent 30 s depuis le premier constat, puis vont à la source la plus loin de tout soldat (places respectées), l'attente recommence ; dès qu'une source est sûre, règle habituelle et attente oubliée ; le cerveau passe par `StrandedGroups` (mémoire `Strandeds`, ligne de journal « every source is under an enemy soldier ... leave after 30 s »). Test `Test_AllSourcesUnderTheEnemySendsThePeasantsToTheFarthestSourceAfter30Seconds` rouge d'abord (stub), vert ; `quick_check -Parts tests,aiserver,aiduel` 776/776 + PASS. **NON VÉRIFIÉ** : le comportement en vraie partie (deux soldats immobiles), le va-et-vient réel après le départ, mutation du test.

- **AIQ-H = A (réponse de l'utilisateur, 2026-11-01 17:50, relayée par le coordinateur)** : ne rien changer, le repli d'une vague punitive déjà sortie va au point de ralliement ; à revoir avec les garnisons. Aucun code. **Numéro de décision GDD à écrire par la session principale** (pas de D attribué ici).

- **IAQ-S-2 / IAQ-S-3 / IAQ-S-4 FAITES (D621, 2026-11-01 17:5x, Arwen ai-ia1, e89c067)** : (IAQ-S-2 = A) le moulin et les fermes détruits sont rebâtis près du refuge de 40 m derrière le Centre, seulement quand l'un d'eux est détruit, aux frais normaux (elle attend le bois) : `HOLAIOpening::BlockLoss` (ce qui a été VU fini puis a disparu, dit une fois), `RelocateLost` (le spot du moulin / de la k-ième ferme perdue passe à un bloc trouvé autour du refuge par `FindAIFarmBlock`), `RefugeCentreOf` (Centre - TowardEnemy x `RefugeDistance`), `Data.bMillLost` (le moulin d'ouverture détruit se rebâtit près du refuge, ou au spot déplacé du bloc planifié) ; cerveau : `RelocateDestroyed` + mémoire `BlockSeens`, ligne de journal « farm block destroyed ... built again near the refuge ». (IAQ-S-3 = B) les Paysans d'une ferme détruite deviennent inactifs : ce sont des Paysans libres, ils passent par la règle des 30 s (D619, `StrandedGroups`) puis reviennent bâtir via la reconstruction ; aucun code de plus. (IAQ-S-4 = A) codé maintenant, à revoir avec les garnisons. Tests rouges d'abord (stubs, 776/778) puis verts : `Test_DestroyedMillAndFarmAreBuiltAgainNearTheRefuge`, `Test_DestroyedOpeningMillIsBuiltAgainNearTheRefuge` ; `quick_check -Parts tests,aiserver,aiduel` 778/778 + PASS. **Sonde en vraie partie** (scénario jetable, supprimé : IA Facile, `FastBuild`, `hol.Cheat.DamageNear` sur son moulin fini) : moulin détruit à 16.54.20, une Maison d'abord (population), puis `build Mill at (19200, 0)` = refuge, dès 100 de bois ; **NON VÉRIFIÉ** : la reconstruction des FERMES et du bloc planifié en vraie partie (testée seulement en pur), le Paysan de ferme qui revient bâtir, le refuge occupé par l'ennemi, la mutation des tests.

- **Ligne HUD du niveau de l'IA sous le panneau Triches FAITE (2026-11-02 10:3x, Arwen ai-ia1, 67d2e69, séance interactive)** : cause = la ligne (padding haut 72) tombait derrière le panneau C++ « Triches » (capture `ai_level_line_a` : seul « ile » dépassait). Outil 12b d'abord : `HOLHUDLayout::Overlaps` (test `UI.Layout.Overlap` rouge sur stub, 794/795, puis vert), ligne `HUD overlap: ...` de `hol.HUD.Report` (`UHOLScriptScreen::DescribeOverlaps`, panneaux du script + panneau Triches, positions dessinées), `preflight.ps1` FAIL si autre chose que `none` (WARN sans `hol.HUD.Report`). Scénario `ai_level_line` (+ .look, .json) : rouge avec l'ancien script (`HUD overlap: cheats x AILevelPanel`, PNG vu), vert après correction : `GetCheatPanelBottom` (C++, hauteur du panneau plié ou déplié) lu à chaque rafraîchissement par le script (`AILevelTop`). PNG des deux états ouverts et vus. `quick_check -Parts tests` 794/794. **NON VÉRIFIÉ** : fenêtres autres que 1280x720 (mise à l'échelle DPI), `ai_evolving` refait par Glorfindel (il doit appeler `hol.HUD.Report`), mutation du test pur. Question TB-Q2 (l'IA Difficile donne-t-elle des tributs ?) posée : `Retours/Q-TB-Q2-ia-tributs.md`.
- **TB-Q2 TRANCHEE (2026-11-02 ~10:40, reponse de l'utilisateur) : A** : l'IA ne donne jamais de tribut (ni Difficile ni autre) ; si un joueur lui en envoie un, elle le prend sans reagir. Rien a coder. Numero D : a ecrire par la session principale.
- **Essai ai_level_line : E1 OK (2026-11-02, reponse de l'utilisateur)** : la ligne HUD du niveau de l'IA sous le panneau Triches est validee a l'ecran. Retours\E-ai-level-line.md supprime.

- 2026-11-09 (Arwen, heal-sleep, sans C++, D751) : test rouge 18ba103 (aiwave : l'ecart avant-arriere de la vague avant contact doit rester <= 2500 cm, mesure 6741 cm). Cause vue au journal : des soldats libres (220 cm/s) repartent seuls apres un arret, les engins (186) restent derriere ; l'ordre de groupe du moteur (formation, vitesse du plus lent) est deja la pour un ordre. Correctif en script : HOLAIArmy WaveSpread=1200 (UpdateWave, tous les AttackReissue : un soldat libre a plus de 1200 cm devant le dernier s'arrete sur place, FHOLAIWaveStep.HoldIds/HoldPoints, HOLAIBrain les execute). Etat : RED, change saved as Docs/wip_D751_wave_spread.patch (not committed), tree reverted. Pas de test unitaire script ajoute ; EtapeDefini.md non mis a jour.
- 2026-11-09 09:5x (Arwen, heal-sleep, sans C++, D751, 2e agent) : **CAUSE REELLE** : la vitesse du Lancier est 777 cm/s (Tools/Data/UnitTypes.csv), pas 220 ; le moteur forme la vague en blocs au rythme du plus lent (180, journal Block march: 21 units ... pace 180), mais un soldat cut off from its slot by an obstacle quitte le bloc et court seul a 777 (straggler), et un sous-ensemble reordonne seul (soldats inactifs) forme son propre groupe a 777. Mesure : sans correctif 6741 cm (escorte + engins) ; patch WaveSpread=1200 evalue toutes les 5 s : 3773 (un soldat a 777 cm/s fait 3885 cm entre deux evaluations). **Correctif** : patch WIP applique (WaveSpread 1200 : un soldat libre a plus de 1200 cm devant le dernier s'arrete sur place, HoldIds/HoldPoints executes par HOLAIBrain) + AttackReissue 5 -> 1 s (evaluation chaque seconde : depassement ~777 + 1200). Essai abandonne : reordonner toute la vague en un ordre de groupe quand un soldat est inactif = 6741 identique (ne traite pas les cut-off). Resultat : aiwave PASS 3 passes de suite (seuil 2500 inchange, non relache). **NON VERIFIE** : l'effet en vraie partie, la cause racine C++ (un cut-off devrait garder le rythme du bloc, hors perimetre sans C++), mutation du test.
