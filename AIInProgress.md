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
  -17, -fb, -b8. Trial game left open had locked two .uasset (first merge attempt failed, cleaned): close the trial game
  before merging. **Prochain : IA1.3** (plan d'IA1).
