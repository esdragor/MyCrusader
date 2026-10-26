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
- **IA3 Fin de partie** : forum à vies (D426), ruine adverse, Roi protégé en Régicide.
- **IA4 Niveaux** : curseurs, Facile / Normal / Difficile, évolutive (D422, D329) ; plus tard, parties à 8 IA pour nourrir
  les données (ouverture, besoins) et raffiner chaque niveau (IA1-Q4).
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
| IA1.8 Vérification | partie IA contre IA de 10 min sans fenêtre (`Seed`) : aucun Paysan inactif plus de 10 s, jamais une file bloquée au plafond, pas plus de 400 d'une ressource en stock, nourriture jamais à court faute de fermes ; Paysans à 5 et 10 min notés (référence pour IA4) | **essai de l'utilisateur** contre l'IA (scénario `ai_economy.txt`) |
| IA1.9 Clôture | GDD (IA1-Q1 à IA1-Q5 : ferme 2 × 2, 5 Paysans de départ, ouverture puis besoins, placement par type ; numéros D libres), `EtapeDefini.md`, mémoire | commit |

**Plus tard (IA4)** : parties à 8 IA pour nourrir les données (ouverture, cibles des besoins) et raffiner chaque niveau.

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
