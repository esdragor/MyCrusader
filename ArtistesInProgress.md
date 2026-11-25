# Intégration par les artistes : avancement

Chantier ouvert le 2026-11-25 18:4x (CLAUDE-1125ar, session principale, copie principale `main`, journaux seuls, aucun code).
Demande de l'utilisateur : aider au mieux l'intégration des assets par des **artistes qui n'auront pas accès à Claude** :
« un level où l'on peut spawn les trucs, remplacer son mesh, save, jouer les anims... ».

But : un artiste ouvre l'éditeur, voit chaque objet du jeu dans son vrai contexte, remplace le modèle en deux clics,
enregistre, et le voit en partie, sans toucher aux CSV, sans PowerShell et sans agent.

Distinct de `PlaceholdersInProgress.md` (la liste de ce qui est provisoire et l'ordre des lots : ce chantier fournit
l'outil, `Placeholders.txt` reste la liste) et de `VisualInProgress.md` / `AnimationsInProgress.md` / les chantiers audio
(qui produisent des assets ; ce chantier fait l'atelier où un humain les branche).

## État de départ (relevé du 2026-11-25, par lecture)

- Le visuel vit dans les CSV, colonnes mêlées aux règles : `Tools/Data/UnitTypes.csv` (`Mesh`, `MeshYawOffset`,
  `MeshScale`, `SkinnedMesh`, `AnimationProvider`, `*Sequence(s)`, `*PlayRate`, `*Prop(s)`, `Projectile`, `MuzzleSmoke`),
  `BuildingTypes.csv` (`Mesh`, `Shape`), `SourceTypes.csv` (`Mesh`).
- Les DataTables sont **régénérées depuis les CSV** (`Tools/Editor/make_unit_types.py`, `make_building_types.py`,
  `make_source_types.py`, `Tools/AutoJourney/regen_tables.ps1`) : « never typed in the editor », chaque relance remplace
  toutes les lignes. Une modification faite par un artiste dans l'éditeur est donc écrasée au prochain regen d'agent.
- `Tools/check_placeholders.ps1` (dans `quick_check`) échoue si un type change de statut sans que sa ligne de
  `Placeholders.txt` et son entrée de `Tools/Data/placeholders_map.csv` suivent : une livraison d'artiste qui ne les met pas
  à jour casse le contrôle des agents.
- Outils de revue existants, réutilisables comme modèle : `Script/Visual/HOLResourceReview.as`, `HOLForumReview.as`,
  `HOLRocksReview.as` (commandes `hol.*.Review`, scénarios `*_compare`).
- Le mannequin porte `MeshYawOffset = -90` et `MeshScale = 1.5` dans les tables : convention à documenter ou à faire
  disparaître pour les modèles d'artiste.

## Plan (lots à numéroter quand AQ-1 est tranchée)

- **AR0 : où vit le visuel** : TRANCHÉ, A = table visuelle séparée (D924). Suite : concevoir la table (forme : DataAsset par type ou DataTable propre, champs, dossier `/Game/Art/`), la lecture avec repli sur le CSV, et `check_placeholders.ps1` qui la lit.
- **AR1 : niveau Atelier** (`AssetLab.umap`) : galerie par famille (unités, bâtiments, ressources, décor, projectiles,
  effets) construite depuis les tables ; statut tiré de `Placeholders.txt` au-dessus de chaque objet (filtres) ; repères
  d'échelle au sol (mannequin 1,80 m, case et sous-case, emprise `Footprint`, hauteur `Height`) ; caméra de jeu RTS ou
  libre ; lumière des 5 biomes, couleurs des joueurs, brouillard ; bouton « Foule » (50 unités par le vrai chemin de rendu).
- **AR2 : panneau de l'éditeur** (Editor Utility Widget) : choisir un type, prendre un asset, aperçu, « Enregistrer » ;
  contrôles vert / rouge avant d'enregistrer : pivot, axe avant, échelle contre l'emprise, triangles, LOD, matériaux,
  collisions, noms (`SM_`, `SKM_`, `T_`) et dossier, squelette compatible, sockets exigés par le code (arme, bannière,
  sortie de balle et fumée de l'Arquebusier, départ des flèches de tour), licence (ligne préremplie de `Docs/assets_art.csv`).
  « Enregistrer » met aussi à jour `Placeholders.txt` et `placeholders_map.csv`.
- **AR3 : animations** : états joués par unité (repos, marche, attaque, récolte par ressource, construction, mort,
  montage du Mangonneau D869b, rechargement de l'Arquebusier), manquantes en rouge avec l'emprunt en attendant ; boucle,
  vitesse, lecture à la vitesse réelle de l'unité (glissement des pieds) ; instants clés (impact, lâcher du projectile).
- **AR4 : « Jouer avec »** : petite partie d'essai, triches actives (construction rapide, +1000), objet posé à côté du
  Forum ; scénario de comparaison avant / après lancé depuis l'éditeur, sans `play.ps1`.
- **AR5 : cabine de capture des icônes** : caméra et lumière fixes, sort `T_Unit_<Type>` (cadre de garnison), icônes de
  la grille, portraits ; liste des textures attendues par le code et absentes.
- **AR6 : sons** : table d'écoute par objet (sélection, ordres, attaque, mort, construction), vide / rempli, remplacer.
- **AR7 : autour de l'artiste** : binaires précompilés (ouvrir l'éditeur sans compiler), dossier réservé (par exemple
  `Content/Art/`) que les agents ne touchent pas, verrouillage LFS des `.uasset` ou équivalent d'`IN_USE`, bouton
  « Envoyer » qui ne commite que ce dossier, fiche `ARTISTES.md` (unités 1 uu = 1 cm, pivot, axe avant, budgets, sockets,
  licences, lancer l'Atelier).

Ordre conseillé : AR0, puis AR1 + AR2, puis AR4, puis AR3, puis AR5 et AR6 ; AR7 au fil de l'eau.

## Conception AR0 : la table visuelle (D924) — 2026-11-25 19:0x, par lecture du code, RIEN CODÉ

### Forme : un DataAsset par type (et par faction quand le visuel diffère)

- Classe C++ `UHOLArtVisual : UPrimaryDataAsset`. C++ et non script : ses lecteurs sont l'enregistrement du type Mass et
  quatre autres sites C++ (exception de `CLAUDE.md` : Mass + liaison exposée au script).
- **Un fichier par type**, rangé à côté de ses maillages : `/Game/Art/<Units|Buildings|Sources>/<Type>/DA_Art_<Type>`,
  variante de faction `DA_Art_<Type>_<Faction>` (`@` est interdit dans un nom d'asset). La vérité est dans deux champs
  de l'asset, `TypeName` et `Faction` (vide = toutes) ; le nom de fichier n'est qu'une convention pour les humains.
- Pourquoi pas une seule DataTable : un `.uasset` est binaire, non fusionnable. Une table unique = deux artistes sur deux
  types différents se bloquent. Un fichier par type = verrou LFS par type, travail en parallèle.
- **Les variantes de faction sont un besoin, pas une option** : les 7 lignes `DisplayName` de
  `Tools/Data/FactionOverrides.csv` (Lancier squelette, Garde noire, Archer squelette, Arbalétrier squelette, Cavalier
  funeste, Bélier d'os, Mangonneau d'os) sont le même type que l'Aube avec un autre nom : leur corps propre passera par
  `DA_Art_<Type>_Legions`. Idem Zombie (corps du Paysan) et Guerrier damné (corps de l'Homme d'armes), types à part.

### Champs (tous facultatifs : vide = niveau suivant du repli)

- Unités : `Mesh` (statique, repli), `SkinnedMesh`, `AnimationProvider` (`TP_*`, `UAnimSequenceTransformProviderData`),
  séquences par état (repos, marche, attaque, chute, construction ; récolte, marche chargée, repos chargé par ressource),
  vitesses de lecture (marche, récolte, construction), `MeshYawOffset`, `MeshScale`, `Projectile`, `MuzzleSmoke`.
  Accessoires (`GatherProps`, `CarryProps`, `BuildProp`, `AttackProp`) : ce sont des numéros d'une liste fixe du code,
  pas des assets : gardés tels quels en v1.
- Bâtiments : `Mesh`. `Shape` (`BlockAndTower`, `Range`) reste au CSV : il décrit le provisoire, pas l'art.
- Ressources : `Mesh`.
- Communs : `Icon`, `Portrait`, `Status` (Provisoire / Final), `Author`, `License`, `SourceUrl` (ligne du registre
  `Docs/assets_art.csv` préremplie depuis ces champs).
- Sons par objet : plus tard (AR6), pas en v1.
- **Séquences : référence plutôt que numéro.** Le CSV donne un numéro dans le `TP_*` (`IdleSequence 2`, `FallSequence 5`).
  Pour l'artiste, l'asset tient l'`AnimSequence` de chaque état, et le numéro est retrouvé à l'enregistrement du type
  dans la liste du `TP_*` (absente = erreur nommée + repli). **À VÉRIFIER** : que `UAnimSequenceTransformProviderData`
  expose sa liste de séquences. Sinon, numéro + nom affiché par le panneau (AR2/AR3).

### Règle de repli, champ par champ

Asset de la faction → asset commun du type → ligne CSV de la faction → ligne CSV de base. Un champ vide passe au
niveau suivant : un artiste peut poser le corps et garder les animations empruntées.

### Où brancher : 9 lectures relevées, 6 sites à changer

Un résolveur unique, `HOLArtVisual::Apply(FHOLUnitTypeConstants&, TypeName, FactionKey)` (et ses jumeaux bâtiment /
ressource), appelé juste après `Row->ToConstants(TypeName)` :
- `Source/HeroesOfLiberty/Simulation/HOLUnitSubsystem.cpp:397` : enregistrement du type Mass. Couvre par les constantes
  le rendu instancié, les séquences, `MeshScale` des vues (`:1250`), `HOLCorpse.cpp:105`,
  `HOLInterpolatedInstancesProcessor.cpp:435` et `:730`.
- `HOLUnitSubsystem.cpp:328` : **le CRC qui décide si une faction reçoit son propre type Mass doit passer par le même
  résolveur**, sinon un Lancier squelette au corps propre garderait le type (et le corps) du Lancier de l'Aube.
- Lecteurs directs de la ligne : `Net/HOLUnitReceptionSubsystem.cpp:608` (`MeshScale` côté client),
  `Presentation/HOLArrowSubsystem.cpp:89` et `:104` (`Projectile`, `MuzzleSmoke`), `Buildings/HOLBuilding.cpp:930`,
  `:935`, `:1237` (`Mesh` ; `:993` `Shape` reste CSV), `Resources/HOLSourceField.cpp:236` (`Mesh`).
- Script : `Script/UI/HOLGarrisonFrameRules.as:31` (icône par chemin conventionnel `T_Unit_<Type>`) lit d'abord `Icon`.
- Coût : aucun par entité ni par pas, la résolution a lieu une fois à l'enregistrement du type, comme les `TryLoad` actuels.
  Serveur dédié : même comportement qu'aujourd'hui (rien dessiné).

### Découverte et paquet

- Sous-système `UHOLArtVisualRegistry` (GameInstance) : au démarrage, l'Asset Registry liste les `UHOLArtVisual` sous
  `/Game/Art` → carte (Type, Faction) → asset. Deux assets pour la même clé = erreur qui nomme les deux fichiers.
- **Paquet** : `/Game/Art` doit être cuit (type d'asset primaire dans les réglages de l'Asset Manager), sinon le jeu
  publié n'a pas les assets et retombe en silence sur le CSV. Un test du paquet le vérifie.

### Placeholders et agents : un manifeste texte généré

- `Tools/Data/ArtVisuals.csv` (Type, Faction, champs posés, Status), **écrit par l'éditeur** (bouton « Enregistrer » du
  panneau AR2, et commande d'éditeur qui le reconstruit en entier) ; jamais tapé à la main, jamais lu par le jeu.
- Sert à `check_placeholders.ps1` (un type `Final` n'a plus de ligne dans `Placeholders.txt`, un `Provisoire` la garde),
  aux diffs lisibles en revue (un `.uasset` ne se lit pas), et aux agents qui ne lisent pas de `.uasset`.
- Un test d'éditeur compare le manifeste aux assets : écart = FAIL « manifeste périmé ».

### Lots proposés

- **AR0-a (lot C++, un build)** : `UHOLArtVisual`, `UHOLArtVisualRegistry`, résolveur, 6 sites branchés, réglage de
  cuisson. Tests C++ Mass d'abord rouges (D838) : asset vide = constantes du CSV (rien ne change) ; asset avec `Mesh` =
  constante remplacée ; asset `Legions` sur `Spearman` = type Mass distinct (CRC) ; doublon = erreur ; séquence absente
  du `TP_*` = erreur + repli.
- **AR0-b (script / éditeur)** : écriture et reconstruction du manifeste, `check_placeholders.ps1` le lit, test.
- **AR0-c (preuve à l'écran)** : un premier `DA_Art` réel. Proposé : Lancier squelette des Légions (`DA_Art_Spearman_Legions`)
  avec un corps déjà existant d'un autre type, pour voir la variante de faction en partie (scénario, PNG vus).
- Puis AR2 (panneau) et AR1 (Atelier).

## Questions ouvertes

- AQ-1 répondue A le 2026-11-25 18:5x (GDD D924) : table visuelle séparée, lue par le jeu avant le CSV,
  repli sur les colonnes du CSV tant qu'elle est vide. Fiche archivée dans
  `Docs\Archive\Retours_2026-11-25\Q-artistes-source-visuel.md`.
- **AQ-2** : migration vers les `DA_Art` d'un coup ou type par type. Fiche `Retours\Q-artistes-migration.md`, posée le
  2026-11-25 19:0x, aucune valeur codée (ne bloque pas AR0-a : le repli sur le CSV marche dans les deux cas).

## Journal

- 2026-11-25 18:4x (CLAUDE-1125ar, session principale, docs seulement) : chantier ouvert à la demande de l'utilisateur
  (idées données en séance, reprises dans le Plan ci-dessus) ; relevé par lecture des CSV et des scripts de regen ;
  question AQ-1 posée (`Retours\Q-artistes-source-visuel.md`) ; `EtapeDefini.md` mis à jour (en tête et « Fichiers de
  développement ») ; aucun code, aucun numéro D, rien commité.
- 2026-11-25 18:5x (CLAUDE-1125ar, docs seulement) : AQ-1 = A (réponse de l'utilisateur), inscrite GDD D924 ; fiche archivée dans `Docs\Archive\Retours_2026-11-25\` ; AR0 tranché, suivant : conception de la table visuelle (AR0) puis AR1 + AR2 ; rien codé, rien commité.
- 2026-11-25 19:0x (CLAUDE-1125ar, docs seulement) : conception AR0 écrite (section « Conception AR0 ») par lecture du code : un `DA_Art` par type et par faction, repli champ par champ, 9 lectures relevées dont 6 sites à changer (dont le CRC de faction `HOLUnitSubsystem.cpp:328`), registre par l'Asset Registry, cuisson de `/Game/Art`, manifeste `Tools/Data/ArtVisuals.csv` écrit par l'éditeur pour `check_placeholders` ; lots AR0-a (C++), AR0-b, AR0-c ; À VÉRIFIER : liste des séquences de `UAnimSequenceTransformProviderData` ; question AQ-2 posée (`Retours\Q-artistes-migration.md`, d'un coup ou type par type) ; rien codé, rien commité.
