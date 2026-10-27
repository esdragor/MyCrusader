# Map size in progress

Journal du chantier **taille de la carte**, ouvert le 2026-10-27. Demande de l'utilisateur : « définir la taille de la
map ; pouvoir avoir Plaine par exemple mais en version 2, 4, 6, 8, 10-12 joueurs ; elles auront un nom bien sûr
(Petite 2 joueurs…) », précisée aussitôt : « ça reste à 8, c'est juste pour dire que Grande et Immense sont faites pour
8 ». Décisions nouvelles : à la suite de la review du GDD (prochain numéro libre au moment de trancher, voir
`EtapeDefini.md` ; D486 au 2026-10-27).

## Ce qui existe déjà (état au 2026-10-27, `main` 7c48518)

- **Une seule carte de partie**, `Plaine` (`/Game/Maps/Plain`), faite par script (`Tools/Editor/create_match_map.py`) :
  sol de 500 m, **limites de 400 m** (`AHOLMapBounds`), **8 emplacements** sur un cercle de 150 m (`SLOT_ANGLES`, les
  premiers écartés : 0 et 1 face à face, 0 à 3 aux quatre points cardinaux), navmesh cuit dans la carte, crête de
  rochers devant chaque emplacement, bosquet au centre ; ressources de chaque emplacement (bois de 40 arbres, 6
  buissons) et filons pierre / or entre chaque paire de voisins (`AHOLSourceLayout`). Une carte d'essai trop serrée
  pour une vraie partie.
- **Limite des cartes** : case de 4 m, **220 × 220 cases = 880 m** au plus (D344, `AHOLMapBounds::MaxSideTiles`),
  imposée par les positions réseau sur 13 bits (±450 m autour de l'origine, D343 / D344).
- **8 joueurs au plus partout**, et ça ne change pas (TC-Q1) : GDD D01, § 14.4 (formats 2, 4, 6, 8) ;
  `FHOLMatchConfig::MaxSlots = 8` ; salons : `LobbyRules.SlotCount = 8`, `DefaultMaps = [("Plain", 8)]`.
- **Choix de la carte** : liste `+Maps=` de `[/Script/HeroesOfLibertyUI.HOLNewGameScreen]` (`DefaultGame.ini`),
  nom affiché par `UHOLNewGameScreen::GetMapName` (« Plaine ») ; `FHOLMatchConfig::Map` porte le chemin de la carte ;
  le salon annonce ses cartes avec leur nombre de places (`MapInfo("Plain", 8)`).

## But

Une carte (Plaine d'abord, toute carte ensuite) existe en **plusieurs tailles nommées**, chacune faite pour un nombre
de joueurs : le joueur choisit **la carte** puis **la taille** au lancement (menu solo et salon).

## Proposition de départ (à trancher)

| Taille | Faite pour | Côté | Cases | Surface par joueur |
|---|---|---|---|---|
| **Petite** | 2 joueurs | 440 m | 110 × 110 | ~9,7 ha |
| **Moyenne** | 4 joueurs | 616 m | 154 × 154 | ~9,5 ha |
| **Normale** | 6 joueurs | 732 m | 183 × 183 | ~8,9 ha |
| **Grande** | 8 joueurs | 808 m | 202 × 202 | ~8,2 ha |
| **Immense** | 8 joueurs | 880 m | 220 × 220 | ~9,7 ha |

Règle de calcul (TC-Q5 = B) : **proportions d'AoE2** (Minuscule 120, Moyenne 168, Normale 200, Grande 220, Géante 240
cases d'AoE2) ramenées à la limite de 880 m (D344), arrondies à la case de 4 m. Valeurs dans la table des tailles, à
revoir après l'essai (distance entre deux bases voisines, temps de rush). Sol : +50 m de chaque côté des limites. Les noms
sont provisoires (fr / en / ja : Petite / Small / 小, Moyenne / Medium / 中, Normale / Normal / 標準, Grande / Large /
大, Immense / Huge / 特大) ; à l'écran : « Petite (2 joueurs) ».

## Questions à trancher

Une à la fois, options A/B/C et recommandation (méthode de la review du GDD).

| Question | Sujet | Réponse |
|---|---|---|
| **TC-Q1** | Au-delà de 8 joueurs ? | **Non, 8 au plus** (2026-10-27) : Grande et Immense sont faites pour 8 ; rien ne change à `MaxSlots`, aux salons, à la palette, aux équipes ni à la performance (D01, D190 inchangées) |
| ~~TC-Q2~~ | Population à 10-12 joueurs | sans objet (TC-Q1) |
| **TC-Q3** | **Une carte par taille ou une carte qui s'adapte** : A : le script fait **un fichier par taille** (`Plain_2`, `Plain_4`… , navmesh cuit chacun, même recette paramétrée par la taille) ; B : **une seule carte** dont les limites, les emplacements et les ressources sont posés au lancement selon la taille (navmesh à refaire au démarrage, plus long). | **A** (2026-10-27) : un fichier par taille ; **à l'écran, deux listes séparées, « Carte » et « Taille »**, dans le salon comme au menu solo : le jeu en déduit le fichier (Plaine + Grande → `Plain_Large`, par exemple) |
| **TC-Q4** | **Taille et nombre de joueurs** : A : comme AoE, la taille est libre (2 joueurs sur une Grande possible), elle donne le nombre de places, la taille par défaut suit le nombre de joueurs ; B : la taille est fixée par le nombre de joueurs (pas de choix séparé, Immense à part) ; C : la taille fixe un maximum de places, au-delà les emplacements sont fermés. | **A** (2026-10-27) : **la taille fixe le nombre de places** (Petite 2, Moyenne 4, Normale 6, Grande et Immense 8), les autres emplacements sont fermés ; une taille trop petite pour les joueurs déjà assis est **grisée** dans la liste ; chaque fichier n'a que les emplacements de sa taille |
| **TC-Q5** | **Dimensions** : la table ci-dessus ou d'autres valeurs ; surface du sol au-delà des limites (aujourd'hui +50 m de chaque côté). | **B** (2026-10-27) : proportions d'AoE2, 440 / 616 / 732 / 808 / 880 m (110 / 154 / 183 / 202 / 220 cases) ; sol +50 m de chaque côté, comme aujourd'hui |
| **TC-Q6** | **Placement à 6 et 8** : cercle régulier (comme aujourd'hui), ou cercle pour le chacun-pour-soi et **équipes côte à côte** quand les équipes sont choisies (comme AoE « équipes ensemble ») ; ordre des premiers emplacements quand la carte n'est pas pleine. | **C** (2026-10-27) : emplacements en cercle (2 face à face, 4 aux points cardinaux, 6 et 8 réguliers) ; **places tirées au hasard au lancement** selon la graine de la partie (`FHOLMatchConfig::Seed`, rejouable) ; option de salon **« Équipes ensemble »**, cochée par défaut (alliés côte à côte) ; décochée, places au hasard sans regrouper ; scénarios d'essai et `quick_check` gardent des places fixes |
| **TC-Q7** | **Ressources et obstacles par taille** : mêmes ressources par joueur à toutes les tailles (bois, baies, filons entre voisins, D295) ; Immense : plus de ressources neutres au centre ou seulement plus de place. | **B pour Plaine** (2026-10-27) : base de chaque joueur identique à toutes les tailles, ressources neutres ajoutées selon la place libre, symétriques entre les bases et au centre. **Nuance de l'utilisateur : le placement des ressources, sur toute la carte comme dans la base de chaque joueur, est à la discrétion de chaque carte**, pas une règle commune (ex. une carte « El Dorado » : pas d'or dans les bases mais beaucoup de mines au centre, ou aucune ressource sauf de l'or partout). Conséquence : la table des tailles ne donne que côté et places ; **chaque carte a sa recette**, qui reçoit la taille et pose ce qu'elle veut (symétrie entre les places à sa charge, § 8.1) |
| **TC-Q8** | **Noms** : Petite / Moyenne / Normale / Grande / Immense, ou autres. | **A** (2026-10-27) : Petite, Moyenne, Normale, Grande, Immense (en : Small, Medium, Normal, Large, Huge ; ja : 小, 中, 標準, 大, 特大), affichés « Petite (2 joueurs) » |

## Découpage (questions toutes tranchées le 2026-10-27)

| Étape | Contenu | Dépend de |
|---|---|---|
| **TC1** Tailles en données | Table des tailles (clé, nom fr / en / ja, places, côté en cases : 110 / 154 / 183 / 202 / 220) en CSV comme les autres tables (`Tools/Data/MapSizes.csv` → table Unreal) ; règles pures : fichier de (carte, taille), places d'une taille, tailles permises pour N joueurs assis, limite D344 ; tirage des places (graine, « Équipes ensemble ») ; `FHOLMatchConfig` porte la taille et `bTeamsTogether` (nouvelle version du format) ; tests rouges d'abord | — |
| **TC2** Plaine à toutes les tailles | `create_match_map.py` devient la **recette de Plaine**, appelée par taille : limites, sol (+50 m), cercle des emplacements (N de la taille), crêtes, base de chaque joueur identique, ressources neutres selon la place libre (symétriques), bosquet ; un fichier par taille ; tests `Map.StartSlots` par taille (symétrie, places, limite de 880 m) ; ancienne `Plain` remplacée (scénarios et `quick_check` repointés) | TC1 |
| **TC3** Menu et salon | Listes « Carte » et « Taille » (écran Nouvelle partie, salon, `LobbyRules.DefaultMaps` et gestionnaire C#), tailles trop petites grisées, emplacements au-delà fermés, option « Équipes ensemble » cochée par défaut, tirage des places au serveur au lancement, textes fr / en / ja | TC1, TC2 |
| **TC4** Vérification | `quick_check` : partie lancée sur chaque taille (emplacements, caméra bornée, centres) ; scénario d'essai `Tools/Scenarios/map_sizes.txt` ; essai de l'utilisateur | TC1-TC3 |
| **TC5** Clôture | Décisions au GDD (§ 8, § 14.4, § 21), ligne d'`EtapeDefini.md`, fusion dans `main` | tout |

Hors chantier : les autres cartes (forêt, rivière, volcan…) reprendront la table des tailles ; mini-carte (M7).

## Brouillon des décisions pour le GDD (TC5, numéros pris à la clôture)

- **Tailles de carte** (TC-Q1, Q4, Q5, Q8) : cinq tailles nommées, chacune faite pour un nombre de joueurs : Petite
  (2), Moyenne (4), Normale (6), Grande (8), Immense (8, plus de place) ; 8 joueurs au plus (D01 inchangée) ; côtés
  aux proportions d'AoE2 ramenées à la limite de 880 m (D344) : 440 / 616 / 732 / 808 / 880 m ; la taille fixe le
  nombre de places, une taille trop petite pour les joueurs assis est grisée. § 8, § 14.4.
- **Une carte par taille, choisie par deux listes** (TC-Q3, Q9) : un fichier par (carte, taille) fait par la recette de
  la carte ; à l'écran, « Carte » puis « Taille », au menu solo comme au salon ; table des tailles en C++ avec sa copie
  `Tools/Data/MapSizes.csv`. § 15, § 16.6.
- **Places de départ** (TC-Q6) : en cercle ; tirées au hasard au lancement (graine de la partie) ; option « Équipes
  ensemble » cochée par défaut ; scénarios d'essai à places fixes. Précise D244 (« l'emplacement de départ est l'index
  de l'emplacement »). § 14.4.
- **Ressources au choix de chaque carte** (TC-Q7, Q10) : chaque carte place ses ressources, bases comprises (ex. El
  Dorado) ; Plaine : base identique à toutes les tailles (pierre et or à ~60 m), ressources neutres entre voisins selon
  la place libre. Reformule § 14.4 « ressources contestées entre chaque paire de voisins » ; précise D295. § 8.
- **Carte d'essai** (TC-Q11) : l'ancienne `Plain` (400 m, 8 places) reste pour les scénarios et `quick_check`, hors du
  menu et du salon. § 16.6.
- **Équipes en solo** (TC-Q12) : l'équipe se choisit au menu solo (fin du grisé de D240 pour l'équipe), si l'IA les
  respecte. § 15.
- **Idées de cartes** : El Dorado, Les Gués (eau infranchissable sauf passages, qui ne ralentissent pas ; pont large,
  détruit par le siège, refait gratuitement par les villageois ; écart avec D33), La Forêt profonde, La Couronne, La
  Steppe, Les Cols, L'Arrière-pays. § 8.

## Idées de cartes (hors chantier, 2026-10-27)

Chacune sera une recette à elle, appelée par taille (TC-Q7). Retours de l'utilisateur :

- **El Dorado** *(retenue)* : pas d'or dans les bases, un amas de filons d'or au centre, entouré de marais (ralentit,
  empêche la charge de la cavalerie lourde, D33).
- **Les Gués** *(retenue, revue par l'utilisateur)* : une rivière coupe la carte ; **l'eau ne se traverse pas**, sauf
  aux **passages où l'eau est basse**, qui **ne ralentissent pas** ; au milieu de la rivière, **un pont, destructible
  par le siège et reconstructible gratuitement par les villageois**, qui doit **donner un avantage** : **le pont est
  large**, une armée y passe en formation, alors qu'aux passages elle défile en file, plus lente et exposée (choix B de
  l'utilisateur).
  ⚠️ Écart avec D33 (gué = passage étroit, ralentissement, défense réduite dans l'eau) : à reporter au GDD.
- **La Forêt profonde** *(retenue)* : forêt dense entre les bases, clairières et routes ; vue bloquée, couvert, cavalerie
  et siège ralentis ; routes rapides mais exposées.
- **La Couronne** *(retenue)* : bases sur des plateaux autour d'une cuvette centrale riche ; peu de rampes ; hauteur.
- ~~Les Ruines~~ *(écartée : troisième carte « tout au centre », manque de diversité)*, remplacée par les trois
  suivantes, toutes retenues :
- **La Steppe** *(retenue)* : plaine ouverte, ressources éparpillées en petits gisements partout ; expansion tôt,
  raids, aucun point unique à tenir.
- **Les Cols** *(retenue)* : des montagnes découpent la carte en 2 ou 3 couloirs entre les bases, chacun avec ses
  ressources et ses hauteurs ; choisir quel couloir tenir et lequel attaquer.
- **L'Arrière-pays** *(retenue)* : centre vide ; richesses derrière chaque base, en bord de carte, accessibles par des
  chemins détournés qui longent les bords ; défendre ses arrières plutôt que le milieu.

## Suivi

- 2026-10-27 : chantier écrit (état des lieux, proposition de tailles, questions, découpage). TC-Q1 tranchée : 8
  joueurs au plus, Grande et Immense pour 8 ; TC-Q2 sans objet ; ancien TC3 (plus de 8 joueurs) et TC5 (performance)
  retirés. Rien codé.
- 2026-10-27 : TC-Q3 = A, un fichier par taille, choisi par deux listes « Carte » et « Taille » (salon et menu solo).
- 2026-10-27 : TC-Q4 = A, la taille fixe le nombre de places, tailles trop petites grisées.
- 2026-10-27 : TC-Q5 = B, proportions d'AoE2 (440 à 880 m).
- 2026-10-27 : TC-Q6 = C, places au hasard, option « Équipes ensemble » cochée par défaut (à ajouter à TC3 : option
  du salon et du menu, champ de `FHOLMatchConfig`, tirage au serveur).
- 2026-10-27 : TC-Q7 = B pour Plaine ; les ressources (bases comprises) sont à la discrétion de chaque carte (recette
  par carte, paramétrée par la taille ; exemple El Dorado).
- 2026-10-27 : TC-Q8 = A, Petite / Moyenne / Normale / Grande / Immense. Toutes les questions tranchées ; découpage
  TC1 à TC5 mis à jour. Prochain : TC1.
- 2026-10-27 : worktree **`C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`** réservé (IN_USE.txt), branche
  **`map-size`** depuis `main` 7c48518. TC1 en C++ (config du salon, mode de jeu et menu en C++ : exception de D483).
  Sous-agent, boucle 1 : table des tailles, règles pures (fichier de carte, places, tailles permises, tirage des places
  avec « Équipes ensemble »), champs de `FHOLMatchConfig` (fichier sans ces champs → places fixes), tests rouges puis
  verts. En cours.
- 2026-10-27 : **TC1 boucle 1 faite** (sous-agent, non commité, Elrond) : `Map/HOLMapSizeRules.h/.cpp` (`EHOLMapSize`
  None / Small / Medium / Normal / Large / Huge ; table C++ 2/4/6/8/8 places, 110/154/183/202/220 cases ;
  `GetAllowedSizes`, `GetDefaultSize`, `GetMapFile` (`/Game/Maps/Plain` + Large → `/Game/Maps/Plain_Large`, None =
  carte telle quelle), `GetFixedPlaces`, `DrawPlaces` (`FRandomStream(Seed)`, rotation au hasard puis places
  régulières ; Équipes ensemble : groupes puis membres mélangés, équipe 0 = seul), `GetPlaces(config)`) ;
  `FHOLMatchConfig` version 3 : `MapSize` (None), `bDrawPlaces` (false : places fixes pour un fichier sans le champ),
  `bTeamsTogether` (true) ; `Tools/Data/MapSizes.csv` (copie lisible, un test vérifie qu'elle suit le C++) ; tests
  `HeroesOfLiberty.MapSize.*` rouges (fonctions vides) puis **verts 6/6**, `Match.*` 20/20 (`ManagerReference` attend
  maintenant `CurrentVersion`), **`quick_check -Parts tests` 202/202 PASS**.
  Pour TC3 : noms fr / en / ja en `LOCTEXT` côté interface (+ `fill_po.py`) ; le gestionnaire C# écrit `mapSize`,
  `bDrawPlaces`, `bTeamsTogether` (il écrit encore la version 2, lue avec places fixes) ; le mode de jeu refuse des
  places fixes au-delà des places de la taille.
  Question ouverte **TC-Q9** : table en C++ + CSV recopié (vérifié par test) au lieu d'une table de données Unreal.
- 2026-10-27 : **TC-Q9 = A** (table en C++, `MapSizes.csv` copie lisible vérifiée par test, lue par la recette des
  cartes sans éditeur). **TC1 close** (non commitée : commit après validation de l'utilisateur). TC2 lancée.
- 2026-10-27 : **TC2 boucle 1 faite** (sous-agent, non commité, Elrond, ~78 min dont ~28 min d'attente derrière les
  compilations des autres copies) : `create_match_map.py` = recette de Plaine ; sans argument il refait `Plain` à
  l'identique (376 sources comparées au cm) ; avec des tailles (ou `all`) il lit `MapSizes.csv` et fabrique
  `Plain_Small` … `Plain_Huge` (5 cartes en 1 min 49 s, une carte présente n'est jamais écrasée) ; ressources neutres
  dans `plain_neutral_sources` (propre à Plaine). Emplacements numérotés dans l'ordre du cercle (0 à 180°, sens
  trigonométrique), rayon = 0,75 du demi-côté : 165 / 231 / 274 / 303 / 330 m, bases voisines à 330 / 327 / 274 /
  232 / 253 m. Base identique (crête à 30 m, 40 arbres, 6 baies) ; **filons par base** (~60 m, pierre côté bois, or
  de l'autre) au lieu de partagés entre voisins (à trancher, TC-Q10) ; neutres : groupes (bois de 20, or, bois,
  pierre) entre chaque paire de voisins vers le centre, 8 / 16 / 18 / 24 / 32 groupes ; bosquet de 12 à 140 troncs ;
  sources 180 / 360 / 534 / 712 / 720. `MapsToCook` + 5 cartes (pas dans `+Maps=`, TC3). Test
  `HeroesOfLiberty.Map.PlainSizes` rouge (5 cartes absentes) puis vert ; tests lancés à la main **203/203** ;
  `quick_check -Parts tests` **à relancer** (0 test lancé : jeux bloqués derrière `Build.bat -Mode=ValidatePlatforms`
  d'autres copies). Reste TC2 boucle 2 : remplacer `Plain` et repointer scénarios et `quick_check` (ou garder `Plain`
  comme carte d'essai).
- 2026-10-27 : note de mycrusader-17 : depuis AS1, chaque jeu lancé par les scripts ouvre le serveur de débogage
  Angelscript sur le port par défaut ; l'extension VS Code s'y connecte et fige le jeu (« Sending debug database » en
  boucle). Ajouter **`-asdebugport=0`** aux lancements si un `quick_check` d'Elrond reste bloqué (correctif dans
  `quick_check.ps1` sur `net-playtest`, arrivera dans `main` à sa fusion). En attente : TC-Q10.
- 2026-10-27 : **TC-Q10 = A** : chaque base a sa pierre et son or (~60 m) ; la dispute se fait sur les groupes neutres
  entre voisins. À la clôture : GDD § 14.4 « ressources contestées entre chaque paire de voisins » à reformuler
  (neutres entre voisins), D295 précisée.
- 2026-10-27 : `quick_check -Parts tests` relancé dans Elrond : **PASS 203/203 en 56 s**. Question posée : TC-Q11
  (ancienne `Plain` gardée comme carte d'essai, ou supprimée et scénarios recalés).
- 2026-10-27 : **TC-Q11 = A** : `/Game/Maps/Plain` reste la **carte d'essai du développement** (scénarios,
  `quick_check`), **plus accessible dans le menu ni dans le salon** (précision de l'utilisateur) ; le menu et le salon
  n'offrent que Plaine en 5 tailles. **TC2 close** (pas de boucle 2). TC3 lancée (boucle 1 : partie et menu solo ;
  boucle 2 : salon).
- 2026-10-27 : **TC3 boucle 1 faite** (sous-agent, non commité, Elrond) : places au lancement (`AHOLGameState::StartPlaces`,
  calculé au serveur dans `InitVictory`, recalculé chez le client depuis `LaunchConfig` répliquée, même graine) ;
  `AHOLStartSlot::FindForSlot` remplace `Find` (centres, Paysans, héros, Roi, caméra, `hol.Camera.FocusSlot`, ordres
  relatifs au départ, essais de foule et de croisements) ; config qui ne tient pas : « Match config refused on … », rien
  ne plante ; règles `GetNumPlaces(config)`, `CheckPlaces`, `FitSlotsToSize` (ferme les emplacements au-delà de la
  taille, y remonte d'abord les joueurs). Écran Nouvelle partie : ligne « Taille » sous « Carte » + « Équipes
  ensemble » ; `+Maps=` garde `/Game/Maps/Plain.Plain` comme famille Plaine, carte jouée = `GetMapFile` ; taille par
  défaut, tailles trop petites grisées, emplacements fermés grisés ; config sans taille (scénario) → `Plain` d'essai,
  taille « — ». Textes fr / en / ja (`fill_po.py`, 326 textes, aucune traduction changée). Tests `MapSize.*` rouges 3/9
  puis verts 9/9 (`StartPlaces` par la vraie chaîne) ; `quick_check` partie `menu` : cas `takenrow` sur Moyenne, nouveau
  cas `menu sizes` (écrit après le code) ; **`quick_check -Parts tests,menu` PASS 206/206**, partie lancée à la main sur
  `Plain_Small` places tirées conforme. Risque de fusion : `HOLGameState`, `HOLPlayerController`.
  Question ouverte TC-Q12 : « Équipes ensemble » en solo alors que les équipes y sont grisées (D240).
- 2026-10-27 : **TC-Q12** remplacée : **les équipes s'ouvrent en solo** (le grisé est un reste de D240 : combat par
  camps, victoire et ruine par équipe, salon les ont depuis) ; faction toujours grisée ; « Équipes ensemble » active en
  solo. Condition : un test montre qu'une IA alliée n'attaque pas le joueur ; sinon on n'ouvre pas et on revient vers
  l'utilisateur. Confié au sous-agent de TC3 boucle 2 (salon), en cours.
- 2026-10-27 (utilisateur absent, « fais un max ») : brouillon des décisions du GDD écrit ci-dessus ; scénarios d'essai
  écrits dans Elrond (pas encore lancés) : `map_size_small` (1 contre 1 IA), `map_size_large` (4 contre 4, Équipes
  ensemble), `map_size_huge` (5 chacun pour soi sur 8 places), places tirées, 5 Paysans, caméra dézoomée.
- 2026-10-27 : **TC3 boucle 2 faite** (sous-agent, non commité, Elrond). Gestionnaire C# : `LobbyRules.MapSizes`,
  `DefaultMaps` = Plaine en 5 tailles ; nouveau salon en **Grande**, Équipes ensemble cochée ; `PUT settings` prend
  `size` et `teamsTogether` (chef seul, « Prêt » remis à faux) ; taille plus petite → emplacements au-delà fermés,
  joueurs remontés ; trop petite pour les joueurs assis → `slot_taken` ; plus grande → rouvre les emplacements ajoutés ;
  `GET /lobbies` donne `map`, `size`, `maxPlayers` selon la taille ; « Lancer » écrit le match.json **v3** (`mapSize`,
  `bDrawPlaces: true`, `bTeamsTogether`, graine) ; `PROTOCOL.md` à jour. Client : `FHOLLobby` (`Size`,
  `bTeamsTogether`), `UpdateSettings`, écran en salon (listes actives pour le chef), liste des salons « Plaine —
  Petite » (colonne de 240 px, liste 882 → 1130 px). **TC-Q12** : pas d'IA qui choisit ses cibles dans cette branche ;
  le combat automatique passe par les camps ; `quick_check -Parts sides` PASS (allié jamais combattu) → **équipes
  ouvertes en solo**, faction grisée. Tests C# rouges 11/262 puis **262/262** ; Unreal `MapSize.Lobby.*` rouges puis
  verts ; contrôle menu « teams open in solo » rouge puis vert ; **`quick_check -Parts tests,menu` PASS 209/209**.
  `lobby_match.ps1` adapté (Petite, `Plain_Small`) mais **pas lancé** : il faut refaire le serveur précompilé
  (`net_match.ps1 -Stage`, cuisson, verrou d'AutomationTool : machine calme), puis `build_manager.ps1`, puis
  `lobby_match.ps1`. Fichiers à risque de fusion avec `net-playtest` : gestionnaire (Lobby*, MatchConfig, PROTOCOL),
  `HOLManagerClient*`, `HOLNewGameScreen`, `HOLMultiplayerScreen`, scripts de test, .po.
  Choix pris par le sous-agent, à montrer à l'utilisateur : nouveau salon en Grande ; erreur `slot_taken` réutilisée.
- 2026-10-27 : **TC4 faite sauf l'essai de l'utilisateur et `lobby_match`** (sous-agent, non commité, Elrond).
  Scénarios lancés sans fenêtre, conformes sans correction : `map_size_small` (`Plain_Small`, places 1 0),
  `map_size_large` (`Plain_Large`, équipe 1 sur {6,7,0,1}, équipe 2 sur {2,3,4,5}), `map_size_huge` (`Plain_Huge`,
  5 bases, écarts 2 1 2 2 1) ; captures `scen_map_size_*.png` (Elrond, `Saved_Tests\Screenshots\WindowsEditor\HOL`).
  Nouvelle commande de développement **`hol.Match.Open <config>`** (`Dev/HOLScenario.cpp` : ouvre une config comme
  `-MatchConfig=`, sans les contrôles de l'écran). Nouvelle partie **`sizes`** de `quick_check` (6 cas : Small 2,
  Medium 4, Normal 6, Large 4c4 Équipes ensemble, Huge 5, refus Small places fixes) : carte, places distinctes, centres
  à leur place (6 m), caméra sur le slot 0 et bornée aux limites de la taille, équipes voisines, refus sans plantage ;
  rouge (6 FAIL) puis **PASS en 33 s**, **ajoutée à `-Full`**. `quick_check.ps1` reprend de `main`
  `UE_SKIP_UBT_SDK_SETUP=1` et `-asdebugport=0` (texte identique). **`quick_check -Parts tests,menu,sizes` PASS en
  100 s** (209/209). Reste : essai de l'utilisateur, `lobby_match` (après `net_match.ps1 -Stage`, machine calme),
  puis TC5 (GDD, `-Full`, commit et fusion après validation).
- 2026-10-27 : **essai de l'utilisateur réussi** (« ça fonctionne » : scénarios et menu). TC1-TC4 commitées dans
  `map-size` (07a8da6). **TC5 commencée** : décisions au GDD, **D490 à D496** (tailles ; un fichier par taille et
  deux listes ; places et « Équipes ensemble », précise D244 ; ressources au choix de chaque carte, reformule § 14.4
  et précise D295 ; carte d'essai `Plain` ; équipe en solo ; idées de cartes et écart de Les Gués avec D33), nouveau
  § 8.5, § 8.1, § 14.4, § 15 et § 16.6 retouchés ; prochain numéro D497. Fusion de `main` (25 commits) dans
  `map-size` confiée à un sous-agent. Reste : `-Full`, `lobby_match`, fusion dans `main`.
- 2026-10-27 : **chantier clos.** Fusion de `main` (ab6ed0d, 25 commits) dans `map-size` par un sous-agent (ef03494) :
  conflits dans `LobbyRules.cs`, `PROTOCOL.md`, `HOLManagerClientRules.cpp`, `HOLMultiplayerScreen.cpp`,
  `lobby_match.ps1` (étapes « passer en Petite » partagées avec la manche `-Rejoin`) et la traduction (régénérée) ;
  aucun appel à `AHOLStartSlot::Find` venu de `main`. Build PASS, gestionnaire 269/269, `quick_check -Parts
  tests,menu,sizes,net` PASS, **`-Full` PASS en 305 s (232/232)**, `lobby_match -Rejoin` PASS. Le serveur dédié ne
  compilait pas (`ReadBody` ambigu entre deux fichiers de test dans la compilation groupée du serveur) : renommé
  `ReadSizedBody` (6c8e0c4) ; serveur précompilé, **`lobby_match` PASS**. **Dans `main` : 6c8e0c4** (avance rapide).
  GDD : D490 à D496 ; journal archivé dans `Docs/Archive/`.
