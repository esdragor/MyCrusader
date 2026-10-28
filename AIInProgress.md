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
