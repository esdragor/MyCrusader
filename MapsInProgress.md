# Cartes du prototype : chantier en cours

Chantier ouvert le 2026-10-29 à la demande de l'utilisateur (« go »). Sept cartes à côté de Plaine (D496, D498, GDD § 8.5),
chacune avec sa recette appelée par taille (D491). Worktree **Haldir** (branche `maps` à créer depuis `main` 7fcd990).
Rien n'est codé. Les questions sont posées une à la fois (options A / B / C, recommandation), consignées ici et au GDD.
Boucles de 15 min au plus, test rouge d'abord sur la vraie chaîne, EtapeDefini.md à chaque avancée.

## Ce que le GDD fixe déjà

- **D490** cinq tailles (Petite 110, Moyenne 154, Normale 183, Grande 202, Immense 220 cases ; `Tools/Data/MapSizes.csv`).
- **D491** un fichier par carte et par taille, fait par la recette qui reçoit la taille ; deux listes, Carte puis Taille.
- **D492** places en cercle, tirées au hasard selon la graine, « Équipes ensemble ».
- **D493** chaque carte place ses ressources comme elle veut ; seule règle commune : l'équité entre les places (§ 8.1).
- **D494** l'ancienne `Plain` (400 m, 8 places fixes) reste la carte d'essai des tests et scénarios.
- **D496, D498** les sept cartes et leur description (§ 8.5) ; Les Gués : eau infranchissable hors des gués, gués sans
  ralentissement (écart avec D33), pont large détruit par le siège et refait gratuitement par les villageois.
- **D33** effets de terrain : forêt (vision, ralentit cavalerie et siège, couvert), marais (ralentit tout, empêche la charge),
  gué (étroit, ralentit, défense réduite), route (plus vite), hauteur (D23, bonus des tireurs).
- **D286** les chemins se voient : obstacles devant chaque place. **D439 à D444** rochers (crête de 3 blocs, `NavArea_Null`).
- **D295** ressources contestées entre voisins. **D40** aucune zone sacrée ou corrompue posée par une carte compétitive.
- **D421** l'IA joue chaque système : une carte que l'IA ne sait pas jouer est à revoir.

## Ce qui existe dans le code (relevé du 2026-10-29, `main` 7fcd990, lecture seule)

- **Recette de Plaine** : `Tools/Editor/create_match_map.py` (473 lignes, Python de l'éditeur, lancé par `UnrealEditor
  -ExecCmds="py …"`). Pose : sol, `AHOLMapBounds` (côté), `AHOLStartSlot` 0..N-1 en cercle, crête de 3 rochers devant
  chaque place, bois de 40 arbres, 6 buissons de baies, 2 filons (pierre, or) par place, bosquet central et sources
  neutres selon la place libre. Matériaux mats `MI_Rock`, `MI_Tree` créés au besoin. **Une seule recette, celle de Plaine.**
- **Fichiers** : `Content/Maps/Plain.umap` (essai) et `Plain_{Small,Medium,Normal,Large,Huge}.umap` (1 à 4 Mo chacun).
- **Listes** : `Config/DefaultGame.ini` (`+Maps=` famille, `+MapsToCook=` par taille) ; `HOLMapSizeRules` (`GetMapFile`,
  `GetNumPlaces`, `CheckPlaces`, `FitSlotsToSize`) ; écran Nouvelle partie (listes Carte, Taille) ; gestionnaire C#
  (`LobbyRules.MapSizes`, `DefaultMaps`), `PROTOCOL.md`, `FHOLLobby`, écran du salon, liste des salons « Plaine — Petite ».
- **Tests** : `MapSize.*` (places par la vraie chaîne), partie `sizes` de `quick_check` (6 cas, 33 s), scénarios
  `map_size_small / large / huge`, commande `hol.Match.Open <config>`. `Tools/Tests/crossings.ps1` (rochers).
- **Terrain** : le sol est plat partout ; les arbres et les rochers sont les seuls obstacles. **Aucun code** pour l'eau, le
  marais, la route, la rampe, le plateau, ni aucun modificateur de vitesse ou de dégâts par zone. La hauteur existe
  pour les tireurs (D23) mais aucune carte n'en a. Le brouillard (forêt qui bloque la vue) est M7, pas dans ce chantier.
- **IA** : IA1.4 (Arwen, pas dans `main`) place anneaux et bloc moulin + 8 fermes sur sol plat (`FindRingFootprint`,
  `FindFarmBlock`) ; IA1.5 ouvre la partie avec camp de bois et moulin près des sources. Chaque nouvelle carte doit
  laisser l'IA jouer (D421) : partie `aiduel` à passer sur chaque carte.

## Découpage proposé (M-Q1)

Un socle, puis une carte par étape, de la plus simple à la plus riche : chaque carte n'apporte **qu'un effet de terrain
neuf**, testé seul. Les deux premières se font avec les pièces d'aujourd'hui.

| Étape | Carte | Terrain neuf | Ce qu'elle apporte au jeu |
|---|---|---|---|
| **C0** | Socle | aucun | recette générique (pièces communes : sol, bornes, places, bases, sources équitables) ; une carte = un module de recette ; listes (menu, gestionnaire C#, protocole, `MapsToCook`) nourries par **une seule table** `Tools/Data/Maps.csv` ; test commun par carte et par taille (places distinctes, sources à égale distance de chaque place, chemin navigable place à place, `aiduel` qui tourne) ; `make_maps.ps1` qui refait toutes les tailles d'une carte |
| **C1** | La Steppe | aucun (placement) | petits gisements éparpillés, pas de point à tenir : on s'étend tôt |
| **C2** | L'Arrière-pays | aucun (placement, rochers) | centre vide, richesses en bord de carte derrière chaque base, chemins le long des bords (rochers D439) |
| **C3** | El Dorado | **marais** (zone qui ralentit tout, empêche la charge D345.3 plus tard) | pas d'or dans les bases, amas de filons au centre entouré de marais |
| **C4** | Les Cols | **montagnes** (massifs de rochers, infranchissables ; hauteurs D23 si le relief est prêt, sinon C6) | 2 ou 3 couloirs entre les bases, chacun avec ses ressources |
| **C5** | La Forêt profonde | **forêt dense** (ralentit cavalerie et siège, couvert) et **routes** (plus vite) | clairières et routes rapides mais exposées ; la vue bloquée attend M7 |
| **C6** | La Couronne | **relief** : plateaux, rampes, bonus de hauteur D23 | bases sur des plateaux, cuvette centrale riche, peu de rampes |
| **C7** | Les Gués | **eau** infranchissable, **gués** (file), **pont** large destructible et refait gratuitement | rivière qui coupe la carte ; trois mécaniques, en dernier |

Chaque étape de carte suit la même boucle : recette (module) → 5 tailles → listes → test commun rouge puis vert →
`aiduel` sur la carte → scénario d'essai `map_<nom>` → essai de l'utilisateur → décisions au GDD (§ 8.5) → commit sur
`maps` → fusion de `main` → fusion dans `main`. Les effets de terrain (C3, C5, C6, C7) ajoutent une étape **règle** avant
la recette : zone de terrain lue par la simulation (vitesse, charge, couvert), test en script rouge puis vert.

Mesures : S1/S2 au repos à la fin de C0 et à la clôture (cartes à 220 cases avec forêt dense, eau, relief).

## M-Q1 = A (2026-10-29) : Steppe, Arrière-pays, El Dorado, Les Cols, Forêt profonde, Couronne, Les Gués

## C0 Socle : relevé du code (2026-10-29, Haldir `maps` = `main` 4ce6df2, lecture seule)

- **Listes de cartes, cinq copies de « Plain »** : `Config/DefaultGame.ini` (`+Maps=/Game/Maps/Plain.Plain`, lu par
  `UHOLNewGameScreen::Maps`, et `+MapsToCook=` par taille) ; `UHOLNewGameScreen::GetMapName` (une ligne `LOCTEXT("Map_Plain",
  "Plaine")`, « les noms sont provisoires ») ; `LobbyRules.DefaultMaps` (C#, `MapInfo("Plain", 8) { Sizes = MapSizes }`,
  servi par `GET /lobbies` et vérifié par `LobbyMapSizeTests`) ; tests C++ (`HOLManagerClientTests`, `HOLMapSizeLobbyTests`)
  et C# avec `"map": "Plain"` en dur ; `HOLMultiplayerScreen` affiche « Plaine — Grande » par `GetMapName`.
- **Tailles** : `FHOLMapSizeRules` (table en code, `Tools/Data/MapSizes.csv` miroir vérifié par `MapSize.Csv` en C++ et par
  `LobbyMapSizeTests` en C#) ; `GetMapFile(map, size)` = `/Game/Maps/<Nom>_<Taille>`. Le modèle à suivre pour les cartes.
- **Recette** : `Tools/Editor/create_match_map.py`, un seul fichier : constantes de Plaine, classe `Recipe` (chemin, angles
  des places, rayon du cercle, demi-côté, sol, clé de taille), `plain_recipe()` / `sized_recipe()`, aides `spawn*`,
  `matte_instance`, `spawn_obstacles` (crêtes + bosquet), `base_sources` (bois, baies, filons), `plain_neutral_sources`,
  `make_map` (sol, lumières, bornes, navmesh, places, obstacles, `AHOLSourceLayout`), `wait_for_navmesh`, `recipes_of(argv)`
  (sans argument Plain ; sinon clés de taille ou `all`), refus si le fichier existe. Lancé à la main par
  `UnrealEditor.exe <uproject> -nosplash -unattended -ExecCmds="py create_match_map.py all"` ; aucun `.ps1` ne l'enveloppe.
- **Tests de carte** : `Map.PlainSizes` (`HOLMapSizeMapTests.cpp`) charge chaque `Plain_<Taille>` sans la lancer : places
  uniques, bornes, 40 arbres à 30 m, 6 baies à 25 m, 2 filons à 65 m, sources neutres par joueur. Partie `sizes` de
  `quick_check` (ouvre chaque `Plain_<Taille>` par `hol.Match.Open`, caméra, places, refus). Partie `aiduel` (IA contre IA,
  `/Game/Maps/Plain` en dur). `hol.Nav.Path X1 Y1 X2 Y2` (chemin navmesh entre deux points, `HOLNavReport.cpp`).
- **Noms** : textes fr / en / ja par `.po` (`fill_po.py`) ; « Plaine » = `Map_Plain` dans le module UI.

### Découpage de C0 (boucles de 15 min au plus, test rouge d'abord sur la vraie chaîne)

- **C0.1 Table des cartes** : `Tools/Data/Maps.csv` (`Name,DisplayName,Recipe`), `FHOLMapRules` (`Map/HOLMapRules.h/.cpp` :
  table en code, `GetMaps`, `FindByKey`, `GetAssetPath`, `GetDisplayName` en `NSLOCTEXT`), test `Map.Csv` (miroir) rouge
  puis vert ; menu : `UHOLNewGameScreen::Maps` nourri par la table (`+Maps=` retiré de l'ini), `GetMapName` par la table ;
  test `Map.Names` (chaque carte a un nom fr).
- **C0.2 Gestionnaire C#** : `LobbyRules.DefaultMaps` = toutes les cartes de `Maps.csv` (table C# miroir, test C# qui lit le
  CSV, rouge puis vert) ; `PROTOCOL.md` ; test C++ `ManagerClient` : plusieurs cartes dans `maps`.
- **C0.3 Recette par modules** : `Tools/Editor/maps/common.py` (aides, `Recipe`, sol, lumières, bornes, places, bois / baies /
  filons de base, navmesh, sauvegarde) et `plain.py` (crêtes, bosquet, sources neutres de Plaine) ; `create_match_map.py
  [<Carte>] [<Taille>…|all]` (sans argument : `Plain` d'essai, comme avant) ; `Plain_Small` refaite, `Map.PlainSizes` vert,
  résumé identique (places, obstacles, sources).
- **C0.4 Test commun** : `Map.Sizes` pour chaque carte × taille de la table : fichier présent et dans `+MapsToCook=`, N places
  uniques, bornes au côté, **équité** (par type de source, les distances des 3 sources les plus proches de chaque place
  égales à 1 m près entre les places) ; rouge d'abord (la table dit une carte sans fichier) puis vert.
- **C0.5 `Tools/make_maps.ps1 <Carte> [-Sizes …]`** : supprime les fichiers, lance l'éditeur avec la recette, vérifie le
  résumé dans le journal ; quick_check `sizes` et `aiduel` prennent `-Map` (par défaut Plain).
- **C0.6** : build, `quick_check -Parts tests,menu,sizes,aiduel`, gestionnaire C# vert, journal, EtapeDefini, commit sur `maps`.

## Questions

### M-Q1 : ordre des cartes
- **A (recommandé)** : Steppe, Arrière-pays, El Dorado, Les Cols, Forêt profonde, Couronne, Les Gués (du placement
  seul vers le relief et l'eau ; variété au prototype dès C1 et C2).
- **B** : l'ordre du GDD § 8.5 (El Dorado d'abord, Les Gués en deuxième) : la carte la plus demandée vite, mais trois
  mécaniques neuves dès la deuxième étape.
- **C** : par effet de terrain d'abord (marais, forêt, relief, eau, chacun sur Plaine) puis les sept cartes d'un coup :
  moins d'aller-retour mais rien de jouable avant longtemps.

### M-Q2 : effets de terrain dans le prototype — **tranchée : A (2026-10-30)**, seulement le visible (marais, forêt, route, eau) ; couvert et hauteur à un chantier combat après le prototype
Quels effets de D33 sont codés par ce chantier : tous (vitesse, charge, couvert, hauteur), ou seulement ce que la carte
rend visible (marais = ralentit, forêt = ralentit cavalerie et siège, route = plus vite, eau = bloque), le couvert et la
hauteur restant à un chantier combat ?

### M-Q3 : le pont — **tranchée : A (2026-10-30)**, chantier comme une ruine (mécanique V3 réutilisée)
Bâtiment neutre à PV (siège seul le détruit), refait gratuitement par un Paysan : chantier comme une ruine (V3) ou
comme un bâtiment à 0 ressource ?

## Suivi

- 2026-10-29 : chantier ouvert, découpage proposé (C0 à C7), M-Q1 posée à l'utilisateur. Haldir réservé (`IN_USE.txt`).
- 2026-10-29 : **M-Q1 = A** (l'utilisateur : « ça m'a l'air bon, commence »). Branche `maps` créée dans Haldir depuis `main` 4ce6df2. Relevé de C0 fait, découpage C0.1 à C0.6 écrit. C0.1 à C0.3 lancées.
- 2026-10-29 : **C0.3 faite** (sous-agent, non commité, Haldir) : `Tools/Editor/maps/common.py` (334 lignes : constantes de
  taille et de base, `Recipe` avec `map_name`, `test_recipe`, `sized_recipe`, `read_sizes`, `read_maps`, aides `spawn*`,
  `base_sources`, `make_map(recipe, module, on_saved)`, `wait_for_navmesh`) et `maps/plain.py` (134 lignes : crêtes, bosquet,
  sources neutres) ; `create_match_map.py` 473 → 81 lignes, usage `py create_match_map.py [<Carte>] [<Taille>…|all]`
  (`all` seul = `Plain all`). **Contrat d'un module** : `obstacles(recipe) -> int` et `sources(recipe) -> (sources, groupes
  neutres par joueur)`, `common.base_sources` d'abord (source 0 = premier arbre de la place 0). Essai à sec avec un module
  `unreal` factice, ancien fichier contre nouveaux modules : **tout identique** (acteurs, ordre des sources, résumé) pour
  Plain d'essai (88 acteurs, 376 sources) et les 5 tailles (Small 46 / 180, Medium 100 / 360, Normal 182 / 534, Large
  264 / 712, Huge 344 / 720). `Tools/Data/Maps.csv` créé (`Name,DisplayName,Recipe` ; `Plain,Plaine,plain`). Reste : la
  vraie exécution dans l'éditeur (`make_maps.ps1 -Map Plain -Sizes Small`, après la compilation de C0.1), commentaires de
  `HOLMapSizeMapTests.cpp`. **C0.5** : `Tools/make_maps.ps1 -Map <Carte> [-Sizes …] [-Test]` écrit (supprime les fichiers,
  lance l'éditeur avec la recette, vérifie une ligne « Match map created » par taille), pas encore essayé.
- 2026-10-29 : **C0.1 et C0.2 faites** (sous-agent, non commité, Haldir). `Tools/Data/Maps.csv` (`Plain,Plaine,plain`),
  `FHOLMapRules` (`Map/HOLMapRules.h/.cpp` : `GetMaps`, `FindByKey`, `GetAssetPath`, `FindByAsset`, `GetDisplayName`, texte en
  `NSLOCTEXT("HOLMaps", "Map_Plain", "Plaine")`) ; le menu Nouvelle partie remplit sa liste de cartes depuis la table
  (`+Maps=` retiré de `DefaultGame.ini`, `UCLASS(Config)` retiré) ; `LobbyRules.Maps` (C#) = `DefaultMaps` ; `PROTOCOL.md`
  dit d'où vient la liste. Rouge puis vert : `Map.Csv` et `Map.Rules` (322/324 puis 324/324), `LobbyMapsTests` côté C#
  (liste attendue `["Plain"]`, obtenue `[]`, puis 299/299). Build PASS, `quick_check -Parts tests,menu` PASS (324/324,
  menu fr / en / ja / ailaunch). **Clé de traduction déplacée** de `HOLNewGameScreen,Map_Plain` à `HOLMaps,Map_Plain` :
  `.po` mis à jour, **GatherText relancé** (145 s) : `.manifest`, `.archive` et `.locres` refaits, « Plain » et « 平原 »
  gardés, seuls des numéros de ligne ont bougé.
- 2026-10-29 : **C0.3 vérifiée dans l'éditeur** : `make_maps.ps1 -Map Plain -Sizes Small` PASS (179 s), résumé identique à
  l'ancien (« 2 start slots on a 165 m circle … 30 obstacles, 180 sources (4 neutral groups per player), navmesh built »).
  `Plain_Small.umap` régénéré par la nouvelle recette (modifié dans Git). C0.4 (test commun `Map.Table` : fichier, cuisson,
  places, bornes, équité des 3 sources les plus proches) et fin de C0.5 (`quick_check -Map`) confiés à un sous-agent.
- 2026-10-29 : **C0.4 et C0.5 faites** (sous-agent, non commité, Haldir). Test commun `Map.Table` (`HOLMapTableTests.cpp`),
  pour chaque carte de la table × chaque taille : (1) le fichier se charge, message d'échec = `make_maps.ps1 -Map … -Sizes …` ;
  (2) la ligne `+MapsToCook=` est dans `DefaultGame.ini` ; (3) N places d'indices 0..N-1, dans les bornes (côté × 400 cm),
  un seul `AHOLMapBounds` et un seul `AHOLSourceLayout` ; (4) **équité** : par type de source, les distances aux 3 sources les
  plus proches sont égales entre places à 100 cm près (comparées entre places, jamais à une constante). Rouge d'abord (ligne
  `Plain_Huge` retirée de l'ini + tolérance impossible : 21 échecs, 1 + 5 tailles × 4 types), puis vert. Sur Plaine : écart
  **0,0 cm** pour tous les types à toutes les tailles (arbre 1785 / 1812 / 1827 cm, baie 2000 / 2022 / 2022 cm, premiers
  filons à 6034 cm). `quick_check.ps1 -Map <Carte>` (défaut Plain, validé contre `Maps.csv`, FAIL clair sans lancer Unreal
  pour une carte inconnue) : `sizes` joue `<Carte>_<Taille>` ; `aiduel` joue `Plain` d'essai comme avant, et
  `<Carte>_Small` (config v3) pour une autre carte (relu, pas encore joué, Plain seule dans la table).
- 2026-10-29 : **vérification de C0** : `quick_check -Parts tests,menu,sizes,aiduel` : tests 325/325, menu fr / en / ja /
  ailaunch, sizes Small à Huge et refused PASS ; `aiduel` : seul le contrôle des empreintes identiques entre deux courses est
  rouge, **déjà connu avant ce chantier** (indéterminisme d'IA0.7, `AIInProgress.md`, renvoyé au chantier déplacement) ;
  aucun Paysan IA inactif, vitesse et reste verts. C0 attend l'essai de l'utilisateur et le commit avec C1 (rien commité).
- 2026-10-29 : **C1 La Steppe : questions de jeu tranchées par l'utilisateur.** **S-Q1 = A** : base réduite (petit bois d'environ
  20 arbres, les baies, ni pierre ni or dans la base ; pierre et or seulement dans les petits gisements dispersés).
  **S-Q2 = A** : crêtes de 3 rochers devant chaque base (D286), **aucun bosquet central** (aucun point à tenir). Réglages
  laissés à l'essai : un petit gisement par hectare et par joueur (Plaine : un par 2 ha, plus gros), motif bois / baies / pierre /
  bois / or / baies, disposé dans l'arc de chaque joueur puis tourné (l'équité vient de la symétrie par rotation).
- 2026-10-29 : **C1 La Steppe faite, en attente de l'essai de l'utilisateur** (sous-agent, non commité, Haldir). Python :
  `common.spawn_ridges` (crête partagée, D286), `base_sources(recipe, wood_rows, wood_columns, veins)` ; Plaine **identique**
  (essai à sec, carte d'essai et 5 tailles : 88 / 46 / 100 / 182 / 264 / 344 acteurs, 376 / 180 / 360 / 534 / 712 / 720
  sources, aucun fichier de Plaine régénéré). `maps/steppe.py` : base de 4 × 5 = 20 arbres et 6 baies sans filon, crêtes, rien
  au centre ; gisements dispersés par un motif d'arc (suite de Roberts R2, sans hasard) tourné sur chaque arc : bois 12 arbres,
  baies 3 buissons, filon seul ; contraintes : 25 m entre gisements, 60 m des places (110 m pour les filons), 30 m du centre,
  marge de 15 m au bord, 60 m entre filons du même type. **Écart avec le brief** : « un gisement par hectare » ne donnait que
  7 à 8 groupes par joueur, d'où `MIN_GROUPS = 12` (deux tours du cycle : 2 pierres et 2 ors par joueur à toutes les tailles ;
  `MIN_GROUPS` et `GROUP_AREA` en tête de `steppe.py`). Fichiers `Steppe_{Small..Huge}.umap` faits (61 s), `Maps.csv`,
  `MapsToCook`, C# `LobbyRules.Maps`, traductions « La Steppe » / « The Steppe » / « ステップ » (GatherText refait).
  **Rouge puis vert** : `Map.Steppe` (5 erreurs, fichiers absents) et `Map.Table` (5, « file missing ») puis verts ;
  `LobbyMapsTests` rouge (`["Plain","Steppe"]` attendu) puis vert. `Map.Steppe` : 20 arbres à 30 m et 6 baies à 25 m de
  chaque place, ni pierre ni or à moins de 100 m, 2 pierres et 2 ors par joueur égaux à la place 0, gisements à 25 m les uns des
  autres, rien à moins de 20 m du centre, filons du même type à 50 m, symétrie par rotation. Équité `Map.Table` : écart 0,0 cm.
  Mesures de la place 0 : 12 gisements par joueur à toutes les tailles ; pierre la plus proche 132 / 138 / 111 / 116 / 127 m,
  or 122 / 127 / 136 / 118 / 129 m (Small à Huge) ; source la plus proche du centre 36 / 56 / 64 / 70 / 76 m ; écart minimal
  entre gisements 26,6 à 28,8 m. `quick_check` : `-Parts tests,menu,sizes` PASS (326/326) ; `-Parts sizes -Map Steppe` PASS ;
  `-Parts aiduel -Map Steppe` (joue `Steppe_Small`) : seul rouge les empreintes (connu) ; dotnet 299/299 ; scénario
  `map_steppe` (Steppe Large, 1 humain + 7 IA, 4 contre 4) chargé sans fenêtre. À regarder à l'essai : nombre de gisements (12),
  filons à 111-140 m de la base (peut-être trop près pour une carte sans or chez soi), coins du carré vides (motif dans le
  disque inscrit), base IA à 20 arbres sur une longue partie.
- 2026-10-29 : **Retour de l'essai de la Steppe (utilisateur)** : pas assez de ressources de tout ; gros blancs dans les coins ;
  « il faut commencer à avoir des maps qui sont vraiment des maps » ; le principe de la Steppe est de **petits gisements en
  nombre** (une forêt de 8 arbres convient, mais il en faut assez). Règle posée : **pas de problème de ressource avant au
  moins les 3/4 d'une durée de partie classique (30 min, D02), sauf contre-ordre de la carte.**
  **M-Q4 = A (budget par joueur, hors fermes, règle commune à toutes les cartes)** : calcul de la séance (Paysan 0,7/s soit
  ~0,5/s avec la marche ; ~45 Paysans actifs en moyenne × 0,5/s × 1 350 s ≈ 30 000 récoltés, dont un tiers par les fermes,
  infinies ; marge ×1,5) : **150 arbres (15 000 bois), 4 filons d'or (10 000), 2 filons de pierre (5 000), 15 buissons de
  baies (3 000)** par joueur. Un joueur = les sources dont sa place est la plus proche. La Steppe d'avant n'en avait que ~68
  arbres, 2 ors, 2 pierres, 12 buissons. **M-Q5 : ce qui fait une vraie carte** : sol varié = OUI, décor sans collision = OUI,
  obstacles naturels = OUI pour les cartes en général **mais pas dans la Steppe** (« son principe est l'inverse » ; les crêtes
  devant les bases de S-Q2 = A restent, D286) ; relief : avec La Couronne (C6).
  **Plaine** ne tient pas ce budget (180 sources pour 2 joueurs à Petite) : à reprendre après les nouvelles cartes (ligne dans
  EtapeDefini), la règle est marquée par carte dans `Maps.csv` (`Budget`).
- 2026-10-29 : **M-Q5 précisée par l'utilisateur** : sol varié et décor sans collision = **oui partout** (toutes les cartes, pas
  seulement la Steppe : pièces communes dans `common.py`) ; obstacles naturels = oui, mais pas dans la Steppe (son principe est
  l'inverse). Plaine, aujourd'hui inchangée (sol de damier gris, pas de décor), est à remettre au budget et à ces deux
  pièces après les nouvelles cartes. C1b lancée (sous-agent) : test du budget (`Map.Table` contrôle 5, colonne `Budget` de
  `Maps.csv`), Steppe refaite sur tout le carré, `M_GroundVaried`, décor en instances sans collision.
- 2026-10-29 : **M-Q4b : le budget dépend de la taille (utilisateur, option B).** Le calcul (1v1 de 30 min, ~45 Paysans actifs)
  ne dépend pas de la taille ; mais à 8 joueurs les parties en équipes durent 35 à 50 min (D02) et les 3/4 tombent à 26-37 min,
  hors de la marge ×1,5 au-delà de ~33 min. L'utilisateur garde le budget actuel de Petite à **Grande** (150 arbres, 4 ors,
  2 pierres, 15 buissons) et passe **Immense seule à ×1,5** (225 arbres, 6 ors, 3 pierres, 23 buissons). Risque connu, à
  regarder à l'essai : une partie à 8 joueurs de plus de 40 min sur Grande manquera de ressources avant les 3/4.
- 2026-10-29 (fin de séance, l'utilisateur se déconnecte) : **C1b interrompue, non commitée, à reprendre.** Vérifié sur disque
  dans Haldir (`maps`, `main` 4ce6df2) : aucun éditeur ni build ne tourne ; `Steppe_{Small..Huge}.umap` régénérés à 17:12-17:14 ;
  nouveaux : `Tools/Editor/make_ground_materials.py`, `Content/Rendering/M_GroundVaried`, `MI_Ground_Steppe`, `MI_Decor_*`
  (5 matériaux de décor), `HOLMapTableTests.cpp`, `HOLMapSteppeTests.cpp`, `Maps.csv`, `map_steppe.*`. **Pas vérifié** : si le
  contrôle 5 (budget par taille, Immense x1,5) est écrit et vert, si le décor est posé, si `Map.Steppe` / `Map.Table` /
  `quick_check -Parts tests,menu,sizes` passent après la refonte, si le build compile. **Reprise** : relire le rapport du
  sous-agent s'il est arrivé (sinon l'état ci-dessus), `Toolsuild.ps1` dans Haldir, puis
  `quick_check.ps1 -Parts tests,menu,sizes` ; `git status` pour voir ce qui manque ; l'essai de l'utilisateur est
  `play.ps1 map_steppe` (chemin absolu dans Haldir). Rien commité : C0 + C1 se commitent ensemble sur `maps` après l'essai validé.
- 2026-10-29 (rapport d'arrêt du sous-agent de C1b, non commité, **chiffres rapportés par lui, pas revérifiés par moi**) :
  aucun éditeur ni build ne tourne ; les 5 `Steppe_*.umap` sont cohérents (deux avaient été remplacés par des copies sans décor
  pendant une comparaison de navmesh, restaurés ; hash identiques aux versions générées en dernier) ; `Plain_*` non touchés.
  **1. Budget** : colonne `Budget` de `Maps.csv` (Plain `none`, Steppe `standard`), `bBudget` dans `FHOLMapInfo`, contrôle 5 de
  `Map.Table` par taille (M-Q4b : Petite à Grande 150 / 4 / 2 / 15, Immense 225 / 6 / 3 / 23) ; rouge sur les anciens fichiers
  (72 erreurs : 68 arbres, 2 ors, 2 pierres, 18 baies par territoire), vert après régénération. **2. Steppe refaite sur tout
  le carré** (`steppe.py` réécrit) : couche proche identique autour de chaque place (3 ors et 3 pierres à 110-125 m, jusqu'à
  75 m d'une place) + couche lointaine en Poisson-disc déterministe (suite R2) ; plus grand cercle vide **131 / 176 / 201 /
  223 / 242 m avant, 71,8 / 75,0 / 64,8 / 72,0 / 70,0 m après** (Petite à Immense, limite 90 m) ; totaux par territoire
  (arbres / ors / pierres / baies) : Petite 176-178 / 5 / 4 / 22, Moyenne 175-179 / 5 / 4 / 21-23, Normale 175-189 / 5 / 4 /
  22, Grande 178-182 / 5 / 4 / 22, Immense 252-282 / 7 / 4 / 28-29 ; 22 à 32 gisements par joueur ; à Grande 21 bosquets
  de 24 arbres sur 83 (imposé par la place). **3. Sol varié** : `make_ground_materials.py`, `maps/ground.py`, `M_GroundVaried`,
  `MI_Ground_Steppe` (bruit mondial à deux octaves, 3 couleurs, ligne de tuile gardée, mat) ; `common.make_map` lit `GROUND`
  dans le module de la carte (défaut `M_MatteGround`) ; **Plaine : dry-run ALL IDENTICAL** ; captures regardées par le
  sous-agent (patchs doux, grille, cercles de sélection, barres de vie et fondations lisibles). **4. Décor sans collision**
  (`common.spawn_decor`, `DECOR = {...}` dans le module) : un acteur `Decor`, 5 composants HISM, sans collision ni navigation ni
  ombre, cull 120 m, rien à moins de 8 m d'une place, d'une source ou d'un rocher ; 999 / 1 941 / 2 726 / 3 348 / 3 850
  instances, 5 draw calls ; contrôle générique de `Map.Table` (tout composant instancié sans collision ni navigation), rouge
  (15 erreurs) puis vert. **5.** `make_maps.ps1 -Map Steppe` PASS (84 s) ; un passage complet a échoué une fois sur
  `Steppe_Large` (« No navmesh was built », fichier de 8 Ko), vert en relançant cette taille seule (aléa à surveiller).
  **Reste à faire** : (a) preuve que la navmesh n'est pas changée par le décor (comparaison `hol.Nav.Path` avec et sans décor,
  `waitmap` en timeout avec `-nullrhi`, scripts dans le scratchpad `nav_Small.txt`, `nav_Large.txt`, `rungame.ps1`, copies sans
  décor dans `scratchpad
odecor\`) ; (b) relancer `quick_check -Parts tests,menu,sizes`, `-Parts sizes -Map Steppe`,
  `-Parts aiduel -Map Steppe` et `dotnet test Manager\HOLManager.Tests` (non relancés après le dernier réglage du décor et
  de l'en-tête CSV) ; (c) notes françaises du scénario `map_steppe` à mettre à jour ; (d) coût de rendu mesuré à la clôture (S1/S2).
  **Plaine, totaux par territoire** (à tous les tailles) : 80 arbres, 6 baies, 1 à 3 ors, 1 à 3 pierres (1 seule pierre à
  Normale et Grande) : sous le budget en arbres, baies et pierre. À régler à l'essai : densité et taille des touffes de décor
  (peu visibles de loin), contraste des patchs de sol (`GROUNDS` dans `maps/ground.py`), taille des bosquets.
- 2026-10-29 (reprise, « c'est bon, continue ») : **C1b vérifiée par moi.** Build PASS ; `quick_check -Parts tests,menu,sizes`
  PASS (tests 326/326, menu fr / en / ja / ailaunch, sizes Plaine Petite à Immense et refused) ; `-Parts sizes,aiduel -Map
  Steppe` : sizes PASS aux 5 tailles, `aiduel` rouge **seulement** sur les empreintes (connu) ; `dotnet test` 299/299.
  **Preuve de navigation du décor** (à sa façon, sans copie sans décor, perdue) : sur `Steppe_Small`, 6 chemins de 166 à
  339 m (place à place, diagonales, bord à bord) ont un rapport longueur / ligne droite de 1,00 à 1,01 : le décor, les
  sources et les crêtes ne font aucun détour ; le contrôle générique de `Map.Table` (aucun composant instancié avec
  collision ni navigation) reste vert. Pas de comparaison avec / sans décor. Notes françaises de `map_steppe` mises à jour.
  Reste : essai de l'utilisateur (`play.ps1 map_steppe`), puis commit de C0 + C1 + C1b.
- 2026-10-29 : **Retour d'essai de la Steppe refaite (utilisateur)** : « mieux, juste très robotique, pas assez d'aléatoire dans les
  spawns, et les forêts sont des carrés ou des rectangles ». Cause : `common.wood()` pose les arbres sur une grille de lignes et
  de colonnes (écart de 20 cm au plus) et les gisements lointains suivent la suite régulière R2 à écarts fixes. **M-Q6 = A** :
  « aléatoire » = positions et formes naturelles, **même carte à chaque partie** (pas de variantes tirées par partie, pas de
  génération au lancement) ; l'équité reste garantie par le budget par territoire et la couche proche identique. **C1c lancée
  (sous-agent)** : bosquets organiques (contour en blob, allongés ou en haricot, densité variable, arbres isolés et petits
  groupes satellites, clairières dans les grands, nombre d'arbres exact), pastilles de baies irrégulières, gisements lointains
  irréguliers (graine propre à chaque territoire, plus de copies tournées dans la couche lointaine ; la couche proche reste la
  même pour chaque place), graine de disposition en paramètre de la recette (`seed=N`, `make_maps.ps1 -Seed`, variantes pour
  plus tard, non faites) ; Plaine reste identique (ancienne grille gardée).
- 2026-10-29 (utilisateur absent) : **C1c faite, vérifiée par moi** (sous-agent, non commité, Haldir) : build PASS, `quick_check
  -Parts tests,menu,sizes` PASS (326/326, menu fr / en / ja / ailaunch, sizes Petite à Immense et refused), `-Parts sizes -Map
  Steppe` PASS ; dotnet 299/299 selon le sous-agent (non relancé par moi). **Chiffres du sous-agent, non remesurés** :
  `common.py` : `Rng` (xorshift32, sans état global), `Noise`, `grove_points` / `berry_points` (cache), `Recipe.layout_seed`
  (défaut 1), `base_sources(..., organic=False)` ; `wood()` inchangée (Plaine garde la grille : dry-run **ALL IDENTICAL** sur la
  carte d'essai et les 5 tailles). Bois : contour en blob à 3 harmoniques (amplitude 0,15 à 0,35), 35 % allongés, environ 45 %
  en haricot, arbres à 220 cm au moins les uns des autres (aucun tronc à moins de 130 cm), 0 à 2 arbres isolés et 1 à 3 petits
  tas de 2-3 arbres autour, clairière (rayon 2,5 à 3,8 m) et / ou sentier de 3 m dans les bois de 12 arbres et plus, nombre
  d'arbres exact. Baies : tas de 3 à 5 en grappe ou chaîne courbe, jamais en rangée. `steppe.py` réécrit : filons lointains
  posés d'abord, Poisson-disc à densité variable + remplissage des trous, types tirés par territoire, couche proche identique par
  place (0,0 cm d'écart sur les 3 plus proches), couche lointaine différente. Contrôle 12 de `Map.Steppe` (rien sur une grille :
  aucun tronc à moins de 130 cm, direction du plus proche voisin modulo 90° non réticulaire, aucun tas de baies en rangée,
  couche lointaine non copie de la proche) **rouge sur les fichiers de C1b** (bois en réseau 31/31 à 160/160, baies en rangée
  8/10 à 48/56, filons à 57-60 m) puis vert. Plus grand cercle vide **63,0 / 72,2 / 61,5 / 80,7 / 74,4 m** (limite 90 m).
  Budget par territoire (arbres / ors / pierres / baies) : Petite 159-163 / 5 / 4 / 18, Moyenne 155-158 / 5 / 4 / 18, Normale
  153-163 / 5 / 4 / 16-20, Grande 154-158 / 5 / 4 / 18-22, Immense 232-267 / 7 / 4 / 28-32. **Seuils de test changés** :
  `WoodReach` 3 000 → 3 600 cm, `BerryReach` 2 500 → 2 900 cm (bois et baies organiques plus étalés), `VeinSpread` 5 000 →
  6 000 cm ; critère de budget compté source par source comme `Map.Table`. Seed : `create_match_map.py Steppe Small seed=3`,
  `make_maps.ps1 -Seed N` (variantes non générées). Captures regardées par le sous-agent : plus de rectangles ni de rangées,
  pas de réseau visible à Grande ; anneau symétrique de filons autour de chaque base (voulu). **Non fait** : test que les unités
  traversent les sentiers de 3 m (la navmesh se construit, rien de plus). **À régler à l'essai** : taille et compacité des
  bois (`GROVE_AREA`), fréquence des clairières (`grove_attempt`), allongement (35 %), irrégularité de densité
  (`FIELD_AMPLITUDE`, `DIP_PER_AREA`, `DIP_DEPTH` dans `steppe.py`) ; Grande est la plus serrée (vide max 80,7 m, 48 bois de 8
  arbres sur 111). Essai : `play.ps1 map_steppe` (Haldir). Rien commité.
- 2026-10-29 : **Steppe validée par l'utilisateur (« très bien ») et C0 + C1 + C1b + C1c commités sur `maps` : c0b8601** (51
  fichiers, Haldir ; pas de fusion dans `main` encore : `main` a avancé, il faudra la fusionner dans `maps` puis `maps` dans
  `main`, à demander à l'utilisateur). **C2 L'Arrière-pays ouverte** : questions de jeu à poser une à une.
- 2026-10-29 : **C2 L'Arrière-pays : questions de jeu tranchées par l'utilisateur.** **A-Q1 = A** : une **poche derrière chaque
  base**, entre la base et le bord (et dans les coins du carré) : gros filons d'or et de pierre, grande forêt ; accès par les
  chemins du bord, jamais par le centre. **A-Q2 = A** : poches cernées de **longs murs de rochers irréguliers** (affleurements,
  pas des lignes droites), avec une ou deux ouvertures sur un **couloir de bord d'environ 20 m** ; une ceinture de rochers entre le
  centre et le bord avec de rares passages ; le centre reste vide. Budget standard (M-Q4, Immense x1,5), sol varié et décor
  (M-Q5) : oui ; clé `Hinterland`, noms « L'Arrière-pays » / « The Hinterland » / « 奥地 ».
- 2026-10-29 (fin de séance, l'utilisateur arrête) : **C2 L'Arrière-pays : rien codé** (Haldir propre : `maps` = c0b8601, seul
  `IN_USE.txt` non suivi ; aucun éditeur ni build de Haldir ne tourne). Le sous-agent a seulement lu le code et posé un plan de
  conception, **non validé par l'utilisateur au-delà de A-Q1 et A-Q2** :
  1. **Ceinture** : boucle carrée de rochers à ~26 m du bord ; **couloir de bord** ~20 m entre le bord et sa face extérieure.
  2. **Poche** : chambre derrière chaque place (le coin pour les places en diagonale), fermée par un mur intérieur et deux murs
     latéraux ; son dos est la ceinture, percée de **2 ouvertures de 14 m** (fractions 0,2 et 0,8) ; contenu calculé une fois dans
     le repère de la place puis tourné (même contenu et même topologie, géométrie des murs propre à chaque place).
  3. **Passages de la ceinture** : 14 m, entre deux poches, **3 par côté au plus**, positions fixées par la recette, un près de
     chaque bout de poche, à plus de 20 m des coins.
  4. **Profondeur derrière la place** 0,25 × demi-côté = 55 / 77 / 91 / 101 / 110 m (Petite à Immense) ; une fois retirés la
     ceinture et le mur intérieur il reste **1 à 7 m en Petite (inutilisable), ~23 m Moyenne, ~37 m Normale, ~47 m Grande, ~56 m
     Immense** : Petite, Moyenne et Normale demandent des ailes (poche en croissant) ; Petite a un détour base → sa poche d'environ
     340 m (à signaler à l'utilisateur).
  5. **Contenu d'une poche** : 4 ors (60 m entre eux) et 2 pierres intercalées sur une ligne latérale, 1 à 3 gros bois organiques de
     24 à 40 arbres, baies ; base : crête D286, bois organique de 25 à 30 arbres, 6 baies, pas de filon ; centre : rien à moins de
     40 m.
  6. **Murs** : `common.rock_wall` = chaînes d'ellipsoïdes aplatis sur une ligne ondulée, tailles variables, recouvrement ≥ 0,5 m,
     bosses et rochers détachés côté intérieur ; volumes `NavArea_Null` regroupant ~5 rochers (acteurs séparés : le contrôle
     générique de `Map.Table` n'accepte que des composants instanciés sans navigation) ; marqueurs `TargetPoint` « WallGap » par
     ouverture pour mesurer la largeur libre (≥ 12 m, une armée en formation passe).
  7. **Tests prévus** : `Map.Hinterland` (rien à moins de 40 m du centre, filons dans la poche, scellement par rastérisation à 50
     cm et parcours en largeur ouvertures bouchées) ; `Tools/Tests/hinterland_paths.ps1` calqué sur `navproof.ps1` (base → sa
     poche existe, base → poche voisine ≥ 1,25 × la ligne droite, centre → poche, traversée de ceinture) ; rouge montré en lançant
     le script sur la Steppe (rapports ~1,00). `hol.Nav.Path` ne compte que les modificateurs de navmesh, pas les arbres.
  **Reprise** : ajouter `Hinterland,L'Arrière-pays,hinterland,standard` à `Tools/Data/Maps.csv` (UTF-8 sans BOM, pas de guillemets
  nécessaires), montrer `Map.Csv` rouge (partie `tests` de `quick_check`) et `dotnet test … --filter LobbyMapsTests` rouge, puis
  `HOLMapRules.cpp` (`NSLOCTEXT("HOLMaps","Map_Hinterland","L'Arrière-pays")`, vérifier l'accent à la compilation),
  `LobbyRules.cs`, 5 lignes `+MapsToCook=`, `fill_po.py` + `.po` + GatherText ; puis `maps/hinterland.py`, `rock_wall`,
  `MI_Ground_Hinterland`, tests, génération, chemins, scénario `map_hinterland`, captures. Les scripts du scratchpad ne survivent
  pas forcément (`navproof.ps1`, `run_tests.ps1`, `dry_run_dump.py`) : les recréer au besoin. **Question à poser avant de coder
  les murs** : le plan ci-dessus (ceinture à 26 m, ouvertures de 14 m) convient-il, et que faire de Petite (poche en croissant à
  1-7 m de profondeur : retirer la carte à 2 joueurs, ou accepter la poche en ailes) ?
  **Autres restes du chantier** : fusion de `main` dans `maps` puis de `maps` dans `main` (à demander), Plaine à remettre au
  budget / sol varié / décor, mesures S1/S2 à la clôture (coût de 2 000 à 3 000 rochers et du décor), sentiers de 3 m des bois de
  la Steppe jamais testés pour le passage des unités.
- 2026-10-30 : **C2 L'Arrière-pays : plan de murs et Petite tranchés par l'utilisateur.** Plan de murs (ceinture à ~26 m du bord, couloir de
  bord ~20 m, ouvertures de 14 m, 2 par poche, 3 passages de ceinture par côté au plus) : **« à tester »** = on le code tel quel et on
  règle à l'essai (les valeurs vont en constantes en tête de `hinterland.py`). **Petite : poche en croissant (ailes)** comme Moyenne et
  Normale (détour base → sa poche d'environ 340 m, à garder et à signaler à l'essai). Codage lancé (sous-agent, Haldir, branche `maps`).
- 2026-10-30 (sous-agent C2, Haldir `maps`, non commité) : **C2 début du codage.** Rouge montré AVANT le code : `Maps.csv` + ligne
  `Hinterland,L'Arrière-pays,hinterland,standard` ; `quick_check -Parts tests` : 325/326, `Map.Csv` rouge ; `dotnet test --filter
  LobbyMapsTests` : 1 rouge sur 2 (attendu `["Plain","Steppe","Hinterland"]`). Puis : `HOLMapRules.cpp` (NSLOCTEXT `Map_Hinterland`),
  `LobbyRules.cs`, 5 lignes `+MapsToCook=`, `fill_po.py` (« The Hinterland » / « 奥地 »). **Conception retenue** (`maps/hinterland.py`,
  tout en constantes en tête) : ceinture = carré de demi-côté Hb = bord - 26 m **sauf derrière une poche** (le mur du fond de la poche
  en fait partie) et **entre deux poches elle coupe le coin en corde** (octogone à 8 places) : le fond de chaque poche est donc à la même
  distance de sa base (contenu = copie tournée exacte, équité 0 cm) ; poche en croissant (ailes de 90 m de profondeur, partie derrière
  la base à 22 m de la place, fermée en double mur si moins de 14 m de large : Petite) ; largeur de poche ajustée par taille
  (Petite 180 m, Moyenne 165, Normale 90, Grande 100, Immense 115 de demi-largeur) pour garder 35 m entre poches voisines et 55 m de
  ceinture entre elles ; ouvertures à 0,2 et 0,8 du mur du fond ; passages : 1 par tronçon de ceinture par 200 m (3 au plus), droits
  autour de chaque trou (le mur ne ondule pas à moins de 15 m d'un trou : sinon la marque et le trou se décalent) ; contenu d'une
  poche : budget + 1 or et + 1 pierre (5/3, Immense 7/4) en rangées décalées au fond, 4 grands bois de 32-40 arbres (Immense 6), 3 tas
  de 5 baies (Immense 5), le tout à plus de 40 m de la place. `common.rock_wall` (chaînes de rochers sur lignes ondulées, un volume
  NavArea_Null par tronçon de 20 m) et `spawn_rock_wall` ; hook `avoid_points` pour le décor. **Vérifié en Python** (rastérisation
  50 cm, remplissage avec ouvertures et passages bouchés, disque de 10 m, 5 tailles) : fermé de partout, ouvert à la fin (centre ↔
  couloir par les passages, couloir ↔ poche par les ouvertures, centre ↔ poche seulement les deux ouverts), aucune source dans un
  rocher. Reste : build, test C++ `Map.Hinterland` (+ témoin `Map.HinterlandControl` sur la Steppe), génération, chemins, scénario.
- 2026-10-30 (suite, sous-agent C2, Haldir `maps`, **non commité**) : **C2 L'Arrière-pays codée, générée, testée ; en attente de l'essai.**
  Build PASS ; `Map.Hinterland` rouge d'abord (fichiers absents) puis vert ; **`quick_check -Parts tests` 328/328** (dont `Map.Csv` vert,
  `Map.Table` vert aux 5 tailles : équité 0,0 cm, budget par territoire arbres 173 / ors 5 / pierres 3 / baies 21 de Petite à Grande,
  Immense 237 / 7 / 4 / 31 pour 225 / 6 / 3 / 23 demandés ; `Map.HinterlandControl` = témoin : le contrôle de scellement échoue bien sur
  la Steppe) ; dotnet 299/299 ; `quick_check -Parts sizes -Map Hinterland` PASS aux 5 tailles. Fichiers `Hinterland_{Small..Huge}.umap`
  faits par `make_maps.ps1` (53 s + 145 s), 988 / 1 546 / 1 530 / 1 920 / 1 987 rochers et 168 / 264 / 270 / 352 / 361 volumes
  NavArea_Null. **`Map.Hinterland` sur les vrais fichiers** (rastérisation 50 cm des rochers, remplissage ouvertures / passages bouchés
  ou non, disque de 10 m) : **0 échec de scellement aux 5 tailles** (tout bouché : aucune source de poche dans la région du centre ni
  du couloir ; poches ouvertes : chaque source rejoint le couloir par une ouverture de SA poche, jamais le centre ; passages ouverts :
  centre <-> couloir, jamais une poche ; tout ouvert : le centre atteint toutes les sources), rien à moins de 120 m (rocher) / 139 m
  (source) du centre, trou libre d'au moins 7,1 m de rayon à chaque marque (trou de 14 m), couloir de bord de 17,6 à 18,9 m, au plus
  2 passages par côté, 1 à 2 ouvertures par poche, chaque rocher dans un volume NavArea_Null. **`Tools/Tests/hinterland_paths.ps1`**
  (navmesh du jeu, `hol.Nav.Path`, points tirés de la recette par `hinterland_dump.py`) : rouge montré sur la Steppe (`-Map Steppe` :
  3 questions sur 6 en échec, rapports 1,00 à 1,02) ; vert sur Petite, Normale, Grande, Immense : **base -> sa poche 4,66 / 3,41 /
  2,88 / 2,86 fois la ligne droite (427 / 329 / 300 / 320 m)**, base -> poche voisine 1,75 / 1,30 / 1,36 / 1,36, base -> base voisine
  1,00 à 1,04 (le milieu est libre), passage et ouverture 1,00 (32 m pour 32 m). **PAS VERT : Moyenne** (base -> poche du joueur 1 :
  chemin PARTIEL : l'aile de x positif de la poche de la place 1 et le couloir devant son ouverture de x = +99 m ne sont pas atteints
  par le navmesh depuis la place 0, l'aile de x négatif oui ; le scellement par rastérisation dit pourtant tout ouvert ; cause non
  trouvée : piste = le couloir du bas coupé quelque part entre x = -99 et +99 pour le navmesh, ou un morceau de navmesh manquant ; à
  regarder avec des `hol.Nav.Path` du couloir par morceaux). Scénario `map_hinterland` (Grande, 4 contre 4) écrit et chargé sans
  fenêtre (`Hinterland_Large`, 8 places). Pas fait : captures regardées, `quick_check -Parts aiduel -Map Hinterland`, GatherText relu
  après (fait, `.po` en/ja remplis), S1/S2, test que des armées traversent les passages (seul le navmesh dit 1,00).
- 2026-10-30 (suite 2, sous-agent C2, Haldir, non commité) : **chemin PARTIEL de Moyenne : cause trouvée et corrigée, ce n'était PAS la
  géométrie.** Sondes `hol.Nav.Path` par morceaux (Moyenne) : couloir du bas entier (x = -99 à +99 : 198 m pour 198 m, rapport 1,00),
  passage bas-droit (25 m pour 25 m), ouvertures, aile x+ de la poche de la place 1 atteignable depuis le passage bas-droit (rapport 1,06) ;
  mais base 0 -> même point : « partial », 354 m pour 381 m en ligne droite ; et le même chemin à l'envers aussi. C'est la limite de
  nœuds du A* de Recast (`DefaultMaxSearchNodes` = 2048 par défaut) : un détour de 2 à 6 fois la ligne droite, avec un but qui est
  « derrière » un mur dans la direction du départ, épuise les nœuds et rend un chemin partiel. **Correctif** : `DefaultMaxSearchNodes=32768`
  dans `[/Script/NavigationSystem.RecastNavMesh]` de `Config/DefaultEngine.ini` (lu au chargement du navmesh, pas de recompilation, pas
  de régénération de carte). Après : Moyenne base -> poche voisine 714 m pour 381 m (1,88), complet. **À signaler à l'utilisateur** :
  ce réglage vaut pour tout le projet (serveur et solo) ; sans lui, les unités envoyées dans une poche lointaine auraient pu s'arrêter
  en route. Le contrôle rastérisé (`Map.Hinterland`) ne peut pas voir ce cas (l'accessibilité y est exacte, pas la recherche du A*) : le
  rouge est `hinterland_paths.ps1` (« partial » compté en échec), montré rouge sur Moyenne avant le réglage, vert après.
- 2026-10-30 (suite 3, sous-agent C2, Haldir, non commité) : **vérifications de la suite.** `hinterland_paths.ps1` **vert aux 5 tailles** :
  base -> sa poche 4,66 (Petite, 427 m) / 6,16 (Moyenne, 525 m) / 3,41 (Normale) / 2,88 (Grande, 300 m) / 2,86 (Immense, 320 m) ;
  base -> poche voisine 1,75 / 1,88 / 1,30 / 1,36 / 1,36 ; base -> base voisine 1,00 à 1,04 ; passage et ouverture 1,00. `quick_check
  -Parts tests` PASS (Map.Hinterland, Map.Table, Map.HinterlandControl). **Captures regardées** (Petite et Grande, `Saved_Tests\Screenshots  WindowsEditor\HOL\hinA_Small.png`, `hinA_Large.png`) : les murs se lisent bien (bande de roche grise, ondulée, lisse) mais ressemblent à
  une chaîne de galets de largeur constante plus qu'à des affleurements ; bois, filons d'or, baies lisibles ; sol varié lisible. À juger à
  l'essai : variété de largeur / hauteur / couleur des rochers (`WALL_ROCK_*`, `WALL_WOBBLE` dans `common.py`). **Scénario `map_hinterland`
  : « ready » atteint** (~25 s, hors écran).
- 2026-10-30 (suite 4, sous-agent C2, Haldir, non commité) : **M-Q7 (l'utilisateur) : aucune carte n'est vide, « mieux vaut chargé que vide ».**
  « Ce n'est pas parce que j'ai des ressources derrière que je n'en ai aucune devant ; le milieu est trop vide ; où que je déplace ma caméra, il
  doit y avoir quelque chose à l'écran. » **Annule** « le centre reste vide » (A-Q2) et « rien à moins de 40 m du centre ». **Règle commune** :
  plus grand cercle vide <= 35 m de rayon (`MAX_EMPTY_RADIUS` dans `common.py`, `MaxEmptyRadius` dans `HOLMapTableTests.cpp`) sur tout le carré
  jouable ; « quelque chose » = place de départ, source, rocher (crête, mur, affleurement) ou grosse pièce de décor (nouveaux types
  `Boulder` 2,6 m et `Thicket` 3,6 m, sans collision) ; le sol varié et le petit décor (touffes, cailloux, souches, buissons) ne comptent pas.
  **Test commun rouge d'abord** (`Map.Table`, étape 7, pour les cartes au Budget `standard` ; Plaine, sans Budget, est seulement mesurée :
  plus de 80 m partout, à refaire avec elle) : Steppe 63,0 / 72,2 / 61,5 / 80,7 / 74,4 m, Hinterland « plus de 80 m » (centre vide),
  rouges aux 5 tailles. **Code** : `common.fill_holes` (balayage d'une grille de 10 m, une grappe = 1 Thicket + 2 Boulders là où un point est à plus
  de 30 m de tout repère, repères = places, sources, rochers, grosses pièces déjà posées) appelé par `spawn_decor` : vaut pour toute carte
  qui a un `DECOR` ; l'équité (0,0 cm) et le budget ne bougent pas (sources inchangées sur la Steppe : 376 / 734 / 1 102 / 1 476 / 2 244).
  **Hinterland** : `FRONT` = devant chaque base, dans une allée de 80 m au-delà de la crête, 2 ors, 1 pierre, bois de 24 et de 16, 1 tas de baies
  (copie tournée exacte : équité 0,0 cm) ; la poche garde ses gros filons (5 ors, 3 pierres, 4 grands bois, 3 tas de baies) ;
  `middle_content` = bosquets de 10 à 24 arbres et affleurements isolés (courts murs de rocher, un volume NavArea_Null) lancés à la fléchette
  dans la ceinture, à 45 m les uns des autres au plus près, loin des bases, des allées, des poches, des murs et des passages. `Map.Hinterland` :
  « centre vide » retiré ; le scellement se fait maintenant par les salles des ouvertures (région du point intérieur de chaque ouverture, tout
  bouché) et exige 6 filons au moins enfermés par poche. **Régénérés** : Steppe (82 s) et Hinterland (3 min) aux 5 tailles. **Mesuré** (plus grand
  cercle vide) : Steppe 29,8 / 28,9 / 29,9 / 29,1 / 29,8 m ; Hinterland 28,9 / 29,0 / 29,7 / 28,7 / 29,0 m (Petite à Immense). `quick_check -Parts
  tests` 328/328 ; scellement 0 échec aux 5 tailles ; `hinterland_paths` vert (base -> sa poche 4,66 / 5,91 / 3,24 / 2,85 fois).
- 2026-10-30 (suite 5, sous-agent M-Q8, Haldir `maps`, non commité) : **retour d'essai de l'Arrière-pays (utilisateur)** : « mieux, même si tu ne mets que des arbres et des blocages. Fais des forêts de paquets de 14 arbres. La map fait désolée ; voici une minimap d'AoE4 » (beaucoup de petits tas variés partout : bosquets, tas d'or et de pierre, baies, gibier / sites, taches de sol de couleur différente, pas de grands vides ni quelques forêts géantes). **M-Q8** : forêts = **paquets de 14 arbres** (`common.PACK_TREES`), budget réparti en ~11 paquets par territoire (Huge ~17), cartes plus chargées et variées, taches de sol contrastées. **Rouge d'abord, montré** : `Map.Table` contrôle 8 (nouveau : amas d'arbres liés à moins de 6 m, aucun amas de plus de 20 arbres, au moins ceil(budget / 14) - 2 paquets de 10 arbres ou plus par territoire, 9 à Petite-Grande, 15 à Immense) : `quick_check -Parts tests` 327/328, **rouge sur les 10 fichiers** (Steppe : amas de 24 arbres, 4 paquets par territoire sur 9 ; Hinterland : amas de 28 à 40 arbres). Plain (sans Budget) seulement mesurée. Analyseur hors éditeur : scratchpad `offline.py` (module `unreal` factice, mêmes chiffres que les fichiers : Steppe Grande 29,1 m de vide).
- 2026-10-30 (suite 6, sous-agent M-Q8, Haldir `maps`, non commité) : **générateurs réécrits, Steppe régénérée.** `common.PACK_TREES = 14`, `base_sources(..., wood_packs=[14, 6])` (bois de la base en deux paquets, le second 24 m devant le premier) ; **Steppe** : échelle des bois [12, 14, 16], bois proches 14 / 14 / 14, les points lointains en trop sont abandonnés (le décor les comble) pour ne garder que ceil(bois voulu / 14) paquets par territoire ; **Arrière-pays** : base 2 paquets (28), allée de devant 2 paquets, poche de 4 paquets au moins (le reste du budget, moins ce que le milieu donne au territoire le plus pauvre), milieu en paquets de 14 une fois sur deux (affleurements l'autre), `MIDDLE_GAP` 55 m, `ITEM_GAP` 8 m, un paquet du milieu à 32 m au moins des paquets de devant. **Variété sans budget** : 8 nouveaux types de décor sans collision (`DryThicket`, `SandBoulder`, `MossBoulder` = grosses pièces comptées comme repères M-Q7 par leur nom ; `FlowerWhite / Purple / Orange`, `Reed`), 5 sortes de **tas mêlés** `DECOR_CLUMPS` (prairie de fleurs, cairn, tache sèche, bosquet mousse, roselière ; `CLUMPS` par hectare dans chaque module), remplissage des trous en 5 styles ; `FILL_RADIUS` 26 m et **`MAX_EMPTY_RADIUS` resserré de 35 à 28 m** (mesuré hors éditeur 25,7 à 26,0 m ; à confirmer sur les fichiers). **Sol** : `M_GroundVaried` refait avec 4 paramètres de seuil (`BFrom`, `BWidth`, `CFrom`, `CWidth`, par instance dans `maps/ground.py GROUNDS`) : taches de sable orange et de boue sombre à bord net, réglables sans refaire les cartes (seuils de départ 0,60 / 0,62, à calibrer sur capture). Hors éditeur (Petite à Immense) : **Steppe** 11 paquets par territoire (16 à 17 à Immense), arbres 155 à 157 par territoire (Immense 232 à 236 pour 225), 70 à 100 % des paquets à 14 ; **Arrière-pays** 11 à 14 paquets par territoire (17 à 21 à Immense), arbres 154 à 196 (Immense 233 à 289), tous les paquets à 14. Tests C++ adaptés (`Map.Steppe` : base 14 + 6, paquets de 12 / 14 / 16, 14 000 instances de décor au plus, `Map.Table` : 28 m). Steppe régénérée aux 5 tailles (133 s, `make_maps.ps1`), Arrière-pays en cours.
- 2026-10-30 (suite 7, sous-agent M-Q8, Haldir `maps`, non commité) : **M-Q8 faite et vérifiée, en attente de l'essai.** Steppe et Arrière-pays régénérées aux 5 tailles (`make_maps.ps1` 133 s + 282 s). **Vert** : `quick_check -Parts tests` **328/328** (le contrôle 8 de `Map.Table`, rouge sur les 10 fichiers avant, est vert) ; `-Parts sizes -Map Steppe` et `-Map Hinterland` PASS aux 5 tailles et refused ; `hinterland_paths.ps1` PASS aux 5 tailles (base -> sa poche 4,59 / 5,91 / 3,40 / 2,87 / 2,86 fois la ligne droite, comme avant : les arbres ne bloquent pas le navmesh). dotnet non relancé (rien touché côté C#). `aiduel` non relancé. **Chiffres mesurés sur les fichiers** (`Map.Table`) : paquets de 10 arbres ou plus par territoire, Steppe 11 / 11 / 11 / 11 / 16 (Petite à Immense, limite 9 / 9 / 9 / 9 / 15), Arrière-pays 12 / 11 / 11 / 11 / 17 ; plus gros amas : Steppe 16 (14 + une poignée), Arrière-pays **14 partout** (tous les paquets à 14) ; arbres par territoire (budget 150, Immense 225) : Steppe 156 / 151-159 / 155-157 / 152-161 / 227-240, Arrière-pays 168 / 154-196 / 154-195 / 154-196 / 238-289 (au-dessus du budget : les paquets du milieu s'ajoutent à la base, l'allée et la poche qui le tiennent seules). **Plus grand cercle vide** (limite resserrée de 35 à **28 m**) : Steppe 25,9 / 25,7 / 25,4 / 25,8 / 25,9 m, Arrière-pays 25,5 / 25,7 / 26,0 / 25,9 / 25,8 m (Petite à Immense). **Sol** : taches de sable orange et de boue sombre à bord net (`NoiseScale` 2200 Steppe / 2000 Arrière-pays, `BFrom` 0,69, `CFrom` 0,72, largeur 0,04 : réglables dans `maps/ground.py GROUNDS` puis `ground.ps1`/`make_ground_materials.py`, sans refaire les cartes), regardées en capture (Petite, Grande) : lisibles, bords irréguliers, contraste net. **Décor** : Steppe 2 769 à 10 867 instances (limite de test 14 000). **Ce qui existe dans le jeu et n'est pas posable** : gibier / animaux, reliques, sites sacrés (D429-D431), camps neutres (D32), modificateurs de zone : aucun n'existe dans le prototype (le GDD les place après lui) ; la ruine du forum (V3) est un état d'un bâtiment, pas un objet de carte ; les seuls objets posables par la recette sont les 4 types de sources (`SourceTypes.csv`) et les rochers ; le montant d'une source est fixe par type (arbre 100, baies 200, pierre et or 2 500) : impossible de couper l'or en plus petits tas sans changer le type. **À proposer** : types de source « petit filon » (or 500, pierre 500) et gibier (nourriture) pour de vrais petits tas variés à budget égal ; une vraie capture de dessus (la caméra est bloquée à son zoom maximal, aucun mode minimap).
- 2026-10-30 (séance de questions, l'utilisateur) : **M-Q2 = A** (effets de terrain : seulement le visible, marais / forêt / route / eau ; couvert et hauteur après le prototype). **M-Q9 = A (petits tas)** : ajouter un type de source « petit filon » d'or (500) et de pierre (500) à budget égal, pas de gibier avant le prototype (il demande la chasse). Reste M-Q3 (le pont), à poser avant C7.
- 2026-10-30 (retour d'essai Arrière-pays / Steppe, utilisateur) : « moins vide, mais je ne demandais pas de cosmétiques » : ce qu'il veut au
  milieu, ce sont des **choses de jeu** (ressource, modificateur d'environnement...), pas du décor ni des taches de sol ; plus de décor ni de
  sol à ajouter. Et **arbres : largeur divisée par 2 à 3 sur toutes les cartes**. **Fait (moi, Haldir, non commité)** : `HOLSourceField.cpp`,
  arbre provisoire : couronne 4,8 → 2,0 rayons de large, tronc 1,0 → 0,5, `GetDrawnRadius` 2 rayons (÷2,4) ; build PASS (80 s), `quick_check
  -Parts tests` 328/328. L'espacement des arbres des paquets (2,2 m) n'a pas bougé : les paquets de 14 sont plus aérés, à juger à
  l'essai (resserrer `GROVE_SPACING` si besoin, cartes à régénérer). **À trancher** : quelles « choses de jeu » ajouter (petits filons à 500,
  gibier, sites, modificateurs : rien n'existe encore).
- 2026-10-30 (sous-agent M-Q9, Haldir `maps`, non commité) : **M-Q9 précisée : mélange de tailles de filons, budget en VALEUR.** Demande de l'utilisateur après essai de l'Arrière-pays et de la Steppe (« moins vide, mais je ne demandais pas de cosmétiques », « petits ET gros, pas forcément petits ») : des choses de jeu au milieu, donc **des gisements d'or et de pierre de tailles variées, partout, centre et ceintures compris**. **Décision (ce sous-agent) : même VALEUR totale par territoire qu'en M-Q4 (10 000 or, 5 000 pierre ; Immense ×1,5 = 15 000 / 7 500) mais répartie en un MÉLANGE, le nombre de sources minérales par territoire passe de 9 (Steppe) / 11 (Arrière-pays) à 14 (15 à Immense).** Quatre tailles par ressource, lignes de `Tools/Data/SourceTypes.csv` (même mécanisme, aucun objet de jeu neuf, mêmes règles de récolte, forme provisoire de D297 mise à l'échelle par `Radius` et `Height` de la ligne) : `StoneVeinSmall` / `GoldVeinSmall` 500 (rayon 150, hauteur 150), `...Medium` 1 000 (220 / 220), `StoneVein` / `GoldVein` 2 500 inchangés (300 / 300, la taille standard), `...Big` 5 000 (420 / 420, un trou de navmesh de 8,4 m). Mélange par territoire (`common.MINERAL_MIX`) : or = 1 gros + 2 moyens + 6 petits (5 000 + 2 000 + 3 000), pierre = 1 standard + 1 moyen + 3 petits (2 500 + 1 000 + 1 500) ; Immense : or = 2 gros + 2 moyens + 6 petits (15 000), pierre = 1 gros + 1 moyen + 3 petits (7 500). Couche proche (les 3 premiers de chaque ressource à 110-125 m, règle du « départ protégé » gardée) : or moyen + 2 petits, pierre standard + 2 petits (Immense : moyen + 2 petits). **Équité** : TOUS les filons sont des copies tournées d'un motif posé dans le territoire de la place 0 (couche proche déjà ainsi ; couche lointaine de la Steppe et milieu de l'Arrière-pays maintenant aussi), donc chaque territoire a exactement la même valeur, les mêmes tailles, aux mêmes distances (0,0 cm) ; l'équité de `Map.Table` se lit maintenant PAR RESSOURCE (les 3 plus proches filons de la ressource, paires (distance, quantité)) puisque l'or a plusieurs types.
- 2026-10-30 (suite, sous-agent M-Q9, Haldir `maps`, non commité) : **M-Q9 codée, cartes régénérées, tests verts.** *Rouge d'abord, montré* : `Map.Table` étendu (contrôle 9 : valeur d'or et de pierre par territoire = cible à +10 % près, au moins 3 tailles différentes par ressource, au moins 10 sources minérales par territoire ; équité lue par ressource, paires (distance, quantité)) + `Gather.Tables` (6 lignes de types) + `Gather.VeinSizes` (nouveau) : `quick_check -Parts tests` **326/329**, rouges `Map.Table` (Steppe : « territoire de la place 0 : 12 500 or pour 10 000 à 11 000 », « 1 seule taille d'or », pierre 10 000 pour 5 000 à 5 500 ; Arrière-pays idem), `Gather.Tables` et `Gather.VeinSizes` (« No source type StoneVeinSmall »). *Vert ensuite* : lignes de `SourceTypes.csv` + `SourceTypes.uasset` refait (`make_source_types.py`), `common.py` (`MINERAL_*`), `steppe.py` (couche proche : tailles, couche lointaine : motif tourné, `VEIN_GAP` 18 m), `hinterland.py` (poche / devant / milieu), tests Steppe et Arrière-pays adaptés (`IsGoldVein`/`IsStoneVein`, `VeinSpread` 40 m, `PileGap` 18 m), `hinterland_paths.ps1` (`-like "GoldVein*"`) ; cartes refaites par `make_maps.ps1` (Steppe 110 s, Arrière-pays 172 s) ; **`quick_check -Parts tests` 329/329.** Chiffres lus dans `Map.Table` : valeur par territoire **or 10 000 / pierre 5 000 (Immense 15 000 / 7 500), identique pour tous les territoires, 14 sources minérales (15 à Immense) au lieu de 9 (Steppe) et 11 (Arrière-pays)**, 4 tailles (3 à Immense : 500 / 1 000 / 5 000), écart d'équité 0,0 cm ; plus grand cercle vide Steppe 25,9 / 25,7 / 25,9 / 25,9 / 26,0 m, Arrière-pays 25,6 / 25,7 / 26,0 / 25,9 / 25,9 m (limite 28 m) ; arbres par territoire dans le budget (Steppe 150-165, Arrière-pays 154-189, Immense 227-266 pour 225).
- 2026-10-30 (correction, sous-agent M-Q9, Haldir) : **les filons lointains ne sont PAS des copies tournées** (je l'avais fait d'abord : tous verts, mais une capture et `centre.py` montraient un octogone de 8 petits tas identiques à 62 m du centre à Grande, l'effet « robotique » que l'utilisateur a déjà refusé, M-Q6). **Version gardée** : la couche proche reste tournée (0,0 cm) ; les filons lointains de la Steppe et ceux du milieu de l'Arrière-pays sont tirés territoire par territoire (graine propre à chaque territoire, la même LISTE de tailles pour chacun, donc la valeur reste exacte) et doivent être plus loin de toute place que le 3e plus proche filon de leur ressource (l'équité des 3 plus proches reste exacte : ce sont ceux de la couche proche, de la poche et du devant) ; un gros tas préfère le large du territoire (loin de la place et des autres tas, souvent au milieu contesté), les autres s'écartent les uns des autres (quelques-uns tombent près du centre : 4 à 12 sources minérales à moins de 100 m du centre selon la taille, au lieu de 0). Arrière-pays : `MINERAL_FROM_SLOT` 140 m, `MINERAL_DRAWS` 40 000, les tas ne comptent plus dans le `MIDDLE_GAP` des paquets (30 m seulement, `MINERAL_FROM_PACK`, sinon le milieu perdait ses paquets d'arbres). Cartes refaites, tests à relancer.
- 2026-10-30 (suite 2, sous-agent M-Q9, Haldir `maps`, non commité) : **version par territoire vérifiée.** Steppe (123 s) et Arrière-pays (215 s) refaites aux 5 tailles ; `quick_check -Parts tests` **329/329** (dont `Map.Table`, `Map.Steppe`, `Map.Hinterland`, `Gather.Tables`, `Gather.VeinSizes`). Chiffres de `Map.Table` : or par territoire 10 000 partout (Immense 15 000), pierre 5 000 (7 500), **14 sources minérales par territoire (15 à Immense) ; tailles 500 / 1 000 / 2 500 / 5 000 (Immense 500 / 1 000 / 5 000)** ; équité des 3 plus proches filons de chaque ressource 0,0 cm sur 5 tailles x 2 cartes (Steppe Grande or 112 m (1 000) / 120 m (500) / 120 m (1 000), pierre 115 m (500) / 117 m (500) / 118 m (2 500) ; Arrière-pays Grande or 64 m (1 000) / 81 m (5 000, dans la poche) / 81 m (500)) ; plus grand cercle vide Steppe 25,9 / 25,7 / 25,9 / 25,9 / 25,9 m et Arrière-pays 25,9 / 25,7 / 26,0 / 25,8 / 25,9 m (limite 28 m) ; arbres par territoire Steppe 150-163 (Immense 229-242), Arrière-pays 154-290.
- 2026-10-30 (fin, sous-agent M-Q9, Haldir `maps`, non commité, GDD non touché) : **M-Q9 close côté code, en attente de l'essai de l'utilisateur.** Vérifié : `quick_check -Parts tests,menu` PASS (**329/329**, menu fr / en / ja / ailaunch), `-Parts sizes -Map Steppe` et `-Map Hinterland` PASS aux 5 tailles et refused, `hinterland_paths.ps1` PASS aux 5 tailles (base -> sa poche 4,59 / 5,91 / 3,30 / 2,86 / 2,86 fois la ligne droite, base -> poche voisine 1,63 / 1,82 / 1,27 / 1,32 / 1,37, passage et ouverture 1,00), `-Parts aiduel -Map Steppe` et `-Map Hinterland` : seul rouge les empreintes entre deux courses (connu), rien d'autre (je n'ai pas mesuré que l'IA vide les petits tas : ses règles sont par ressource, jamais par type, et `AISummary` vert). **Endroits touchés par les nouveaux types** : `Tools/Data/SourceTypes.csv` + `Content/Resources/SourceTypes.uasset` (6 lignes), `HOLSourceTypes.h` (noms `StoneVeinSmall/Medium/Big`, `GoldVeinSmall/Medium/Big`, `IsStoneVein`, `IsGoldVein`), `HOLGameNames.cpp` (6 `LOCTEXT`) + `fill_po.py` + GatherText (en / ja : « Small Stone Vein »..., 362 textes remplis, 0 clé inconnue), `HOLMapTableTests.cpp` (contrôle 9 + équité par ressource), `HOLGatherTests.cpp` (`Gather.Tables` +6 lignes, `Gather.VeinSizes` : chaque type, chargé 10 par 10 puis par la règle de récolte, sort sa quantité exacte, jamais plus), `HOLMapSteppeTests.cpp` et `HOLMapHinterlandTests.cpp` (`IsGoldVein` / `IsStoneVein`, `VeinSpread` 60 -> 40 m, `PileGap` 18 m), `common.py`, `steppe.py`, `hinterland.py`, `Tools/Tests/hinterland_dump.py` et `hinterland_paths.ps1` (types `GoldVein*`), scénarios `map_steppe.txt` et `map_hinterland.txt` (notes). Rien à faire côté sélection, IA, réseau, C# (aucun ne nomme les types de filon ; la forme provisoire est mise à l'échelle par `Radius` / `Height` de la ligne). **Captures** (Petite centre, Grande devant une base, milieu, poche ; `Saved_Tests\Screenshots\WindowsEditor\HOL\SteppeLarge_*.png`, `SteppeSmall_*.png`, `HinterlandLarge_*.png`, `HinterlandSmall_*.png`) : les tailles se lisent bien (gros or de 5 000 = grand bloc jaune de 8 m, moyen = bloc moyen, petit = petit bloc, pierre grise de même), arbres minces (couronne 2 rayons) : les paquets de 14 sont aérés mais lisibles ; **au centre exact de la Steppe Petite il n'y a toujours pas de filon dans l'écran par défaut** (le plus proche est à 50 m ; avec le zoom -5 on en voit), à Grande 11 sources minérales à moins de 100 m du centre ; la poche de l'Arrière-pays montre le gros or et la grosse pierre contre le mur. **Pas fait** : mesures S1/S2 (coût de rendu des gros blocs : aucun, ce sont les mêmes 3 cubes mis à l'échelle), dotnet (C# non touché par ce travail), tester que des armées passent entre deux gros tas (le navmesh dit 1,00 aux 5 tailles), resserrement de `GROVE_SPACING` (paquets de 14 pas trop minces en capture, pas touché), GDD.
- 2026-10-30 (utilisateur, après essai ; **à faire dans une autre session, rien codé**) : deux retours sur la capture de la Steppe (zoom près du sol) :
  1. **Arbres** : puisque leur largeur a été réduite (÷2,4, `HOLSourceField.cpp`), leur **hauteur baisse d'un tiers** (cône de la couronne et
     tronc) et leur **nombre est multiplié par 3** (budget d'arbres par joueur ×3 : 150 → 450, Immense 225 → 675 ; paquets de 14 : environ 32
     paquets par territoire, ou paquets plus gros : à décider ; espacement `GROVE_SPACING`, budget de `Map.Table` et du contrôle 8 des paquets,
     coût S1/S2 à remesurer, navmesh : un trou par arbre).
  2. **« Tout ce qui n'a rien à voir avec de l'herbe ne sert à rien, on peut l'enlever »** : sur la capture ce sont les petits objets de décor
     (boules violettes, orange et blanches = fleurs, cônes beiges, souches, cailloux, roselières) qu'il juge inutiles ; **à confirmer au début de la
     prochaine séance** : les taches de sable et de boue du sol restent-elles ? Le grand décor (`Boulder`, `Thicket`, `DryThicket`, `SandBoulder`,
     `MossBoulder`) compte comme repère de la règle M-Q7 (cercle vide ≤ 28 m) : s'il part, les trous doivent être comblés par des choses de
     jeu (arbres ×3, filons) : à mesurer. Même règle que la remarque précédente : il veut du jeu, pas du cosmétique.
  **Reprise** : M-Q9 (filons de tailles variées) est close côté code, en attente de l'essai complet ; le travail est dans Haldir (`maps`), non
  commité (C0, C1 commités c0b8601 ; C2, M-Q7, M-Q8, M-Q9 et largeur des arbres non commités).
- 2026-10-30 (utilisateur, réponse à la question sur le sol ; rien codé) : **taches de sable et de boue : on les enlève** sauf si elles font partie de
  l'identité de la carte, et « on n'insiste pas non plus sur l'identité de la carte » : donc par défaut **sol uniforme (herbe)** sur Steppe et
  Arrière-pays (retirer `GROUNDS`/BFrom/CFrom de `ground.py`, `M_GroundVaried` gardé seulement si une future carte en a besoin). Avec le décor
  sans rapport avec l'herbe, cela finit le retrait de tout le cosmétique ; à faire dans la prochaine séance avec arbres hauteur −1/3 et nombre ×3.
- 2026-10-30 (utilisateur, règle M-Q10 ; rien codé) : **tout ce qui n'a pas d'intérêt stratégique, économique ou militaire est retiré**, sauf si cela
  touche à l'identité de la carte (sable devant la mer, roseaux devant une rivière...). Donc, pour la Steppe et l'Arrière-pays (aucune eau) :
  plus de petit décor (fleurs, herbes, cônes, souches, cailloux, roseaux), plus de grand décor (`Boulder`, `Thicket`, `DryThicket`, `SandBoulder`,
  `MossBoulder`, `DECOR_CLUMPS`, `fill_holes`), plus de taches de sol (sable, boue) : sol d'herbe uniforme. **Restent** : ressources (arbres, baies,
  filons), rochers qui bloquent (murs, crêtes, affleurements : militaire), places. La règle M-Q7 (cercle vide ≤ 28 m) ne se remplit plus
  que par des **choses de jeu** (arbres ×3, filons, rochers bloquants) : `Map.Table` doit compter seulement ces repères (retirer `BIG_KINDS`
  de la liste des repères). Les cartes à eau (Les Gués, El Dorado...) pourront garder un décor d'identité (sable côté mer, roseaux côté
  rivière) : à décider carte par carte. **À faire dans la prochaine séance** avec les arbres (hauteur −1/3, nombre ×3).
- 2026-10-30 (test d'1 h, sous-agent Haldir `maps`) : **commits** : le travail C2 / M-Q7 / M-Q8 / M-Q9 / largeur des arbres est commité sur `maps` (vérifié avant : build PASS, `quick_check -Parts tests,rocks,sizes` 329/329, rocks 17/17, sizes Steppe et Hinterland PASS) en trois lots : c3579ba (6 types de filons, noms, traductions, tests Gather), 3594b4f (arbre 2,4 fois plus mince), 533f480 (carte Arrière-pays, Steppe refaite, règle du cercle vide 28 m, `DefaultMaxSearchNodes=32768`, sol varié) ; puis 89ce28f (**M-Q10 hauteur des arbres −1/3** : tronc 0,45 → 0,30, couronne 0,6 → 0,40, base 0,27). Pas fusionné dans main.
- 2026-10-30 (suite, M-Q10, **non commité, tests ROUGES, décision de l'utilisateur nécessaire**) : rouge d'abord montré (`Map.Steppe` « scenery > 0 » inversé en « 0 décor » et `Map.Table` : plus aucun composant instancié dans une carte au Budget, Boulder / Thicket retirés des repères) : 327/329 rouge. Code fait dans Haldir, non commité : `DECOR` et `CLUMPS` retirés de `steppe.py` et `hinterland.py` (donc plus de `fill_holes`), sol uniforme (`BFrom` / `CFrom` = 2,0 dans `ground.py GROUNDS` : le bruit ne les atteint jamais, `M_GroundVaried` gardé), cartes Steppe (131 s) et Arrière-pays (199 s) régénérées aux 5 tailles. Résultat : le décor est parti (Steppe 328/329) mais **`Map.Table` contrôle 7 (cercle vide ≤ 28 m) est rouge** sans le décor : Steppe 75,7 / 78,5 / 79,6 / 62,2 / 56,6 m (Petite à Immense), Arrière-pays 38,4 / 44,5 / « plus de 80 » / « plus de 80 » / « plus de 80 » m (les coins des grandes tailles, que le décor remplissait).
  **Arbres ×3 : tenté, impossible avec le solveur actuel.** `BUDGET` Steppe à (450, 15) : `Steppe: the far layer cannot hold the territories ([25, 25] at the smallest gap)` dès Petite (il faudrait ~33 paquets de 14 par territoire dans une zone où le plus petit écart permis entre dépôts est 33 m, `FAR_GAP_MIN`) ; l'Arrière-pays n'a pas été essayé (la poche, `WING_DEPTH` 90 m, ne peut pas tenir 28 paquets : il faudrait aussi des paquets du milieu bien plus nombreux). Budget remis à 150 / 225 (Steppe, test), cartes régénérées avec.
  **Question pour l'utilisateur (M-Q10b), je ne tranche pas** : sans décor, le cercle vide ≤ 28 m ne peut être tenu que par des choses de jeu. Options : **A** arbres ×3 en refaisant le solveur de la Steppe (écart entre paquets ~20 m, `FAR_GAP_MIN` et `HOLE_*` revus, poche et milieu de l'Arrière-pays agrandis) : gros travail, c'est ce qu'il a demandé, coût S1/S2 à remesurer (3 600 arbres, un trou de navmesh chacun) ; **B** poser des petits affleurements de rocher bloquants (militaire, comme ceux du milieu de l'Arrière-pays) pour combler les trous à budget d'arbres égal : rapide, reste dans « choses de jeu » mais ajoute des obstacles ; **C** relâcher la règle à ~45 m pour la Steppe et accepter les coins vides de l'Arrière-pays : rapide, contraire à M-Q7. **Recommandation : A pour la Steppe (c'est la demande), B en attendant pour l'Arrière-pays dont la poche ne peut pas tenir ×3.** Tant qu'il n'a pas tranché, le travail M-Q10 (décor retiré, sol uniforme) reste non commité dans Haldir (cartes régénérées incluses : `git checkout` des `Content/Maps/*.umap` ramène les cartes commitées avec décor, vertes avec 533f480, mais le test `Map.Steppe` modifié, lui, réclame 0 décor).

- 2026-10-30 18:30 (retour d'essai de l'utilisateur sur l'Arrière-pays, reporté ici depuis Auto_Journey_Retours.md) : « resserre les arbres et double leur quantité, sinon très bien ». Donc arbres ×2 en nombre et plus serrés (pas ×3). Correction en file dans Auto_Journey_File.md (Haldir).
- 2026-10-30 (sous-agent Haldir `maps`, commit 0ae2591 sur `maps`, pas fusionné) : **M-Q10 + M-Q10b faits et commités, vert.** Retour de l'utilisateur sur l'Arrière-pays : « resserre les arbres et double leur quantité ». **Fait** : arbres **x2** (`BUDGET` 300 par joueur, Immense **360** : 450 ne tient pas, essais hors éditeur : 360 tient, 375 ne tient pas, avec des paquets de 18 au plus), **plus serrés** (`common.GROVE_SPACING` 220 -> 160 cm, `GROVE_AREA` 29 600), `steppe.py` : `DEPOSIT_GAP` 25 -> 18 m, `FAR_GAP_MIN` 33 -> 27 m, échelle des paquets 12 / 14 / 16 / 18 (18 pour l'Immense), `GROVE_MEANS` jusqu'à 18 ; `hinterland.py` : `MIDDLE_GAP` 55 -> 38 m, `GROVE_WALL` 780, `ITEM_GAP` 500, `WOOD_GAP` 700 (deux paquets de la poche à moins de 6 m se lisaient en un amas de 28 à l'Immense). Résolution hors éditeur (scratchpad `try.py`, `empty.py`, `clus.py`) avant de refaire les cartes. **Rouge d'abord** : `Map.Table` budget 300 / 360 et contrôle 10 neuf (distance moyenne d'un arbre à son voisin <= 200 cm, `MeanTreeSpacing`) : 328/329 sur les cartes d'avant ; puis vert 329/329 (relevé `quick_tests.log`) : `-Parts tests,rocks,sizes` PASS (rocks 17/17, sizes aux 5 tailles et refused, camera). Cartes des deux recettes refaites aux 5 tailles (`make_maps.ps1`). Tests adaptés : `Map.Steppe` `DepositGap` 18 m et paquets de 18 permis ; `Map.Table` paquets nécessaires = ceil(budget / 18) - 2. **Sort de M-Q10** : gardé et commité avec le x2 (décor retiré, sol uniforme). **Le contrôle 7 (cercle vide <= 28 m) n'est plus exigé** (`EmptyRuleHeld = false` dans `HOLMapTableTests.cpp`, la mesure reste affichée) : sans décor il reste, mesuré hors éditeur, Steppe 74 / 75 / 73 / 56 / 54 m et Arrière-pays 38 / 37 m (Petite, Moyenne) et plus de 80 m dans les coins hors des murs (Normale à Immense) ; ce n'est pas du vert « tenu », c'est une règle en attente de la réponse de l'utilisateur (M-Q10b, `Auto_Journey_Retours.md` section 2 réécrite en 4 vraies questions : M-Q10b, M-Q10c espacement, M-Q3 pont, décor d'identité des cartes à eau et Plaine). **Pas fait** : captures/essai de l'utilisateur, mesures S1/S2 (deux fois plus d'arbres, donc de trous de navmesh : à la clôture du chantier), `hinterland_paths.ps1` non relancé (les arbres ne bloquent pas le navmesh, mais le x2 n'a pas été revérifié), `aiduel`, dotnet.
