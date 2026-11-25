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
- **AR0-b (script / éditeur)** : écriture et reconstruction du manifeste, `check_placeholders.ps1` le lit, test. **Volet check FAIT** (CELEBORN-1125ar, branche art-manifest-1125ar, rouge 65b71bf vu 19:27 15/19, correctif 49af701, 19/19 PASS 19:28) : `Tools\Data\ArtVisuals.csv` en-tête seul ; type Final (toutes ses lignes Final) = plus exigé provisoire, FAIL s'il est encore dans Placeholders.txt. RESTE : écriture / reconstruction du manifeste par l'éditeur (après AR0-a compilé).
- **AR0-c (preuve à l'écran)** : un premier `DA_Art` réel. Proposé : Lancier squelette des Légions (`DA_Art_Spearman_Legions`)
  avec un corps déjà existant d'un autre type, pour voir la variante de faction en partie (scénario, PNG vus).
- **AR0-d (D925, migration d'un coup)** : après AR0-a dans `main` : commande d'éditeur (Python) qui crée un `DA_Art` par
  type depuis le CSV, puis retrait des colonnes visuelles du CSV, de `FHOLUnitTypeRow` et des scripts de regen ; tests des tables
  suivis ; en jeu rien ne change (essai : une partie naturelle, PNG avant / après identiques).
- Puis AR2 (panneau) et AR1 (Atelier).

### Risques levés le 2026-11-25 19:3x (relevé en lecture seule, moteur `UE 5.8 HOL` sous `C:\UE\UE_5.8_HOL`)

- **Séquences : FAISABLE.** `UAnimSequenceTransformProviderData` (`Engine\Source\Runtime\Engine\Classes\Animation\AnimSequenceTransformProviderData.h:872`)
  expose `GetSequences()` (UFUNCTION, :919), chaque entrée porte son `UAnimSequence` (:187). Le `DA_Art` tiendra donc une
  **référence d'AnimSequence par état**, résolue en numéro à l'enregistrement du type (lecture inverse déjà faite à
  `HOLInterpolatedInstancesProcessor.cpp:730-733`). Un artiste crée un `TP_*` dans l'éditeur sans script (« Transform Provider
  Data ») ; piège à documenter dans `ARTISTES.md` : ajouter au moins une entrée à `Layers` (:996), sinon rien ne joue
  (`Tools\Editor\make_skinned_mannequin.py:148`). v1 d'AR0-a garde les numéros ; la référence arrive avec AR0-d.
- **Panneau : À CONTOURNER.** Le plugin Angelscript ne gère pas les Editor Utility Widgets (aucune occurrence) ; il offre
  `UScriptEditorMenuExtension`, `UScriptAssetMenuExtension` (menus contextuels du Content Browser filtrés par classe),
  des boîtes de saisie générées depuis une fonction / struct (`ScriptEditorPrompts.h:32-35`) et `UScriptEditorSubsystem`.
  **AR2 révisé** : tout en texte, fichiers `.as` sous `Script\Art\Editor\` (exclus du jeu) : menus contextuels sur
  `UHOLArtVisual`, `SkeletalMesh`, `StaticMesh`, `AnimSequence` (« Vérifier », « Créer le DA_Art de ce type », « Voir dans
  l'Atelier »), contrôles imprimés en vert / rouge, édition des champs dans le panneau de détails natif du DataAsset.
  Un panneau UMG ne viendrait qu'en cas de besoin, généré par Python (`PythonScriptPlugin` actif).
- **Paquet : FAISABLE.** `Config\DefaultGame.ini` n'a aucun réglage AssetManager ; il cuit des dossiers fixes
  (`DirectoriesToAlwaysCook` :174-182). Ajouter `/Game/Art` à cette liste (le plus simple, même modèle que l'existant).
- **Git : DÉCISION À VENIR (AQ-3, AR7).** Dépôt GitHub sans LFS (`.gitattributes` :6-14 en `binary`, aucun `lockable`) :
  pas de verrou sur un `.uasset`, et GitHub refuse un fichier de plus de 100 Mo hors LFS (FBX, textures d'artiste).
  Passer à LFS `lockable` pour les nouveaux fichiers d'art est possible sans réécrire l'historique ; question à poser
  avec AR7, pas avant que des artistes arrivent.

## Questions ouvertes

- AQ-1 répondue A le 2026-11-25 18:5x (GDD D924) : table visuelle séparée, lue par le jeu avant le CSV,
  repli sur les colonnes du CSV tant qu'elle est vide. Fiche archivée dans
  `Docs\Archive\Retours_2026-11-25\Q-artistes-source-visuel.md`.
- AQ-2 répondue A le 2026-11-25 19:3x (GDD D925) : migration d'un coup après AR0-a (commande d'éditeur qui crée tous les
  `DA_Art` depuis le CSV, puis retrait des colonnes visuelles du CSV et des scripts de regen). Fiche archivée dans
  `Docs\Archive\Retours_2026-11-25\Q-artistes-migration.md`.

## Journal

- 2026-11-25 18:4x (CLAUDE-1125ar, session principale, docs seulement) : chantier ouvert à la demande de l'utilisateur
  (idées données en séance, reprises dans le Plan ci-dessus) ; relevé par lecture des CSV et des scripts de regen ;
  question AQ-1 posée (`Retours\Q-artistes-source-visuel.md`) ; `EtapeDefini.md` mis à jour (en tête et « Fichiers de
  développement ») ; aucun code, aucun numéro D, rien commité.
- 2026-11-25 18:5x (CLAUDE-1125ar, docs seulement) : AQ-1 = A (réponse de l'utilisateur), inscrite GDD D924 ; fiche archivée dans `Docs\Archive\Retours_2026-11-25\` ; AR0 tranché, suivant : conception de la table visuelle (AR0) puis AR1 + AR2 ; rien codé, rien commité.
- 2026-11-25 19:0x (CLAUDE-1125ar, docs seulement) : conception AR0 écrite (section « Conception AR0 ») par lecture du code : un `DA_Art` par type et par faction, repli champ par champ, 9 lectures relevées dont 6 sites à changer (dont le CRC de faction `HOLUnitSubsystem.cpp:328`), registre par l'Asset Registry, cuisson de `/Game/Art`, manifeste `Tools/Data/ArtVisuals.csv` écrit par l'éditeur pour `check_placeholders` ; lots AR0-a (C++), AR0-b, AR0-c ; À VÉRIFIER : liste des séquences de `UAnimSequenceTransformProviderData` ; question AQ-2 posée (`Retours\Q-artistes-migration.md`, d'un coup ou type par type) ; rien codé, rien commité.
- 2026-11-25 19:2x (CLAUDE-1125ar) : fin de séance repoussée à 20:30 ; check-list L29 lue ; lancés en parallèle : GLORFINDEL-1125ar = AR0-a (C++, worktree Glorfindel, branche art-visual-1125ar, énoncé Docs\Rapports\ENONCE-GLORFINDEL-1125ar.txt, ne compile pas : la principale compile (a) puis (b)) ; CELEBORN-1125ar = AR0-b (sans C++, worktree Celeborn, branche art-manifest-1125ar, manifeste Tools\Data\ArtVisuals.csv + check_placeholders) ; relevé en lecture seule des risques (liste des séquences du TP_*, panneau d'éditeur faisable par un agent, cuisson / LFS) ; docs commitées b1420c7.
- 2026-11-25 19:3x (CLAUDE-1125ar) : AQ-2 = A (réponse de l'utilisateur), GDD D925, fiche archivée ; lot AR0-d ajouté au plan (après AR0-a dans main).
- 2026-11-25 19:3x (CLAUDE-1125ar) : relevé des risques revenu (séquences faisables, panneau à contourner par menus d'éditeur Angelscript, cuisson par DirectoriesToAlwaysCook, Git sans LFS) : section « Risques levés » ; AR2 révisé (menus d'éditeur en script, pas d'EUW).
- 2026-11-25 19:4x (CLAUDE-1125ar) : fin de séance levée par l'utilisateur (« fais un max tant que je ne dis pas stop », SessionEnd 2026-11-26 08:00, c386ed2) ; SAURON-1125ar lancé = AR1-a galerie `hol.Art.Gallery` (sans C++, worktree Sauron, branche art-gallery-1125ar) ; AR0-d partira dès qu'AR0-a sera dans main.
- 2026-11-25 19:2x (CELEBORN-1125ar, sans C++) : AR0-b volet check fait, branche art-manifest-1125ar : rouge 65b71bf (check_placeholders_test 15/19 vu 19:27), correctif 49af701 (19/19 PASS 19:28) ; manifeste `Tools\Data\ArtVisuals.csv` (en-tête seul), `check_placeholders.ps1 -ManifestFile` ; choix à confirmer : type Final = TOUTES ses lignes (Type, Faction) Final ; test 6 réparé (ligne « Lancier Aube » devenue « Lancier Légion ») ; RESTE : écriture du manifeste par l'éditeur après AR0-a ; branche prête, non fusionnée.
- 2026-11-25 19:3x (GLORFINDEL-1125ar, C++ NON COMPILÉ) : AR0-a, branche art-visual-1125ar : rouge 063de9e (UHOLArtVisual + UHOLArtVisualRegistry dans Source/HeroesOfLiberty/Art/HOLArtVisual.h/.cpp, bouchon : Apply/HasFactionAsset/BuildMap vides ; HOLArtVisual::Apply branché après Row->ToConstants dans FindTemplate et dans le CRC de GetTemplateKey ; tests Tests/HOLArtVisualTests.cpp : HeroesOfLiberty.Art.Visual.EmptyAssetKeepsCsv (garde, vert attendu sur les deux), .MeshReplacesCsvAtRegistration, .FactionAssetGetsOwnMassType, .DuplicateIsAnError (rouges attendus sur 063de9e)) ; correctif 424cfea (résolveur commun puis faction champ par champ, registre Asset Registry /Game/Art, doublon = erreur nommant les 2 fichiers, PrimaryAssetTypesToScan HOLArtVisual /Game/Art AlwaysCook dans Config/DefaultGame.ini, module AssetRegistry au Build.cs). Prochaine étape : la principale compile 063de9e puis 424cfea et lance Automation RunTests HeroesOfLiberty.Art.Visual ; RESTE : sites HOLUnitReceptionSubsystem.cpp:608 (MeshScale client, lecture par unité : prévoir un cache par (type, faction) plutôt qu'un Apply par appel), HOLArrowSubsystem.cpp:89/:104, HOLBuilding.cpp:930/:935/:1237, HOLSourceField.cpp:236, icône script HOLGarrisonFrameRules.as:31 (FindArtVisual exposé), séquences par ressource (FHOLResourceSequences) non couvertes en v1, séquence absente du TP = erreur + repli non fait.
- 2026-11-25 19:4x (CLAUDE-1125ar) : **AR0-b FAIT EN BRANCHE** (CELEBORN-1125ar, art-manifest-1125ar) : rouge 65b71bf vu 19:27 (check_placeholders_test 15/19), correctif 49af701 ; `Tools\Data\ArtVisuals.csv` (en-tête seul : Type,Faction,Family,Asset,Fields,Status), `check_placeholders.ps1 -ManifestFile` : un type Final n'est plus exigé provisoire, Final encore listé = FAIL nommé ; un type n'est Final que si toutes ses lignes (Type, Faction) le sont ; test rejoué par la principale 19/19 PASS, check_placeholders PASS ; à intégrer avec AR0-a ; RESTE : écriture du manifeste par l'éditeur (après AR0-a). CELEBORN-1125ar2 lancé = AR4-a `hol.Art.Try` (sans C++, branche art-try-1125ar).
- 2026-11-25 19:3x (SAURON-1125ar, sans C++) : AR1-a galerie hol.Art.Gallery [Units|Buildings|Sources|All] (Script\Art\HOLArtGallery.as), branche art-gallery-1125ar : rouge 73dd7c9 (HOLArtGallery_Test.as, PlanGrid vide ; rouge non vu par l agent, a lancer), correctif aa93a15 + 9d2e9f0 (compile, etiquettes), scenario art_gallery 2ef1d19 ; preflight PASS 19:34, 3 PNG vus (51 places : 21 unites, 19 batiments, 11 sources, 0 refus) ; E-art_gallery.md ; reste : sources = formes d attente (UHOLSourceSubsystem::AddSource a exposer, lot C++ groupe), familles plus grandes qu un cadre.
- 2026-11-25 19:4x (CELEBORN-1125ar2) : **AR4-a EN BRANCHE** (art-try-1125ar) : rouge 826a38b (HOLArtTry_Test, bouchons, a voir rouge par la principale), correctif baf2b2c + e5fd62e (Forum 1300, camera avant selection), scenario art_try 9dbade3 ; preflight PASS 2 fois, PNG vus : placement OK (5 Arquebusiers libres a cote du Forum, camera dessus), MAIS selection non faite (le heros reste selectionne) donc marche et tir NON vus : LOGIQUE SEULEMENT, pas d'E-art_try.md. RESTE : selection des unites posees (BoxGround dans la meme image que la naissance ?), source de ressource non posee, slot du joueur 1 suppose 0.
- 19:44 HALDIR-1125ar AR0-a2 (worktree Haldir, branche art-visual2-1125ar) : (a) 7aa886a rouge (4 tests Mass HeroesOfLiberty.Art.Visual.* + 1 script), (b) 84ad66f resolveurs par champ + lecteurs reception/fleches/batiments/ressources/icone ; NON COMPILE, a compiler par la principale ; reste : faction aux fleches, test acteur batiment/source.
- 2026-11-25 20:2x (CLAUDE-1125ar, vérifications de la principale, rouges et verts VUS à l'exécution, quick_check -Parts tests) :
  **AR0-a** (GLORFINDEL-1125ar, art-visual-1125ar) : build 063de9e PASS 52 s, rouge VU 2267/2270 (Art.Visual.DuplicateIsAnError, .FactionAssetGetsOwnMassType, .MeshReplacesCsvAtRegistration) ; build 424cfea PASS 127 s, VERT 2270/2270. RESTE (repris par AR0-a2) : lecteurs hors unités ; hors v1 : séquences par ressource, séquence absente du TP_* = erreur + repli.
  **AR1-a** (SAURON-1125ar, art-gallery-1125ar) : `hol.Art.Gallery` pose les 51 types (21 unités, 19 bâtiments, 11 sources) ; rouge 73dd7c9 VU 2266/2268 (HOLArtGallery_Test x2), vert 9d2e9f0 2268/2268 ; scénario art_gallery preflight PASS, 3 PNG vus, `Retours\E-art_gallery.md`. RESTE : vraies sources (exposer `UHOLSourceSubsystem::AddSource`, C++), cadrage par famille, étiquettes des sources cachées.
  **AR4-a** (CELEBORN-1125ar2, art-try-1125ar) : `hol.Art.Try <Type> [N]` ; rouge 826a38b VU 2266/2269 (HOLArtTry_Test x3), vert e5fd62e 2269/2269 ; scénario art_try preflight PASS mais la sélection des unités posées ne se fait pas : LOGIQUE SEULEMENT (pas d'E). RESTE : sélectionner les unités posées, poser une ressource (aucune route script), bâtiment à l'écran.
  **AR0-a2** (HALDIR-1125ar, art-visual2-1125ar depuis 424cfea) : réception (cache type/faction), flèches (asset commun seul : la faction du tireur n'est pas connue là), bâtiments, ressources, icône de garnison ; build 7aa886a PASS 122 s, rouge VU 2270/2275 (Art.Visual.ReceptionMeshScaleFromAsset, .ArrowsReadProjectileAndSmoke, .BuildingMeshFactionThenCommonThenCsv, .SourceMeshFromCommonAsset, HOLGarrisonFrameRules_Test.Test_GarrisonFrameUnitIconPathReadsTheArtAssetFirst) ; build 84ad66f en cours.
  Suivant : vert d'AR0-a2, puis UNE intégration (integrate.ps1) des 5 branches (AR0-a, AR0-a2, AR0-b, AR1-a, AR4-a), puis AR0-d (migration d'un coup, D925).
- 2026-11-25 20:4x (CLAUDE-1125ar) : **AR0-a2 VERT** : build 84ad66f PASS 31 s, quick_check tests 2275/2275. Intégration groupée lancée dans Glorfindel : `integrate.ps1 -Branch integ-86-art -Greens art-visual2-1125ar,art-manifest-1125ar,art-gallery-1125ar,art-try-1125ar` (sans -Reds : chaque rouge déjà vu à l'exécution par la principale ; le rouge 7aa886a a pour parent 424cfea, absent de main ; simulation à blanc PASS, aucun conflit, aucun doc de la copie principale touché).
  Plan d'AR0-d découpé : **AR0-d1** = DA_Art créés depuis le CSV + manifeste + test Mass `Art.Visual.MigrationKeepsEveryType` (constantes du CSV == champs visuels vidés + Apply du DA_Art, pour chaque type) ; **AR0-d2** = retrait des colonnes visuelles (CSV, FHOLUnitTypeRow, scripts de regen, tests).
  Lancés en parallèle (branches depuis 84ad66f) : HALDIR-1125ar2 = AR0-d1 (art-migrate-1125ar, compile lui-même) ; SAURON-1125ar2 = AR2-a menus d'éditeur Vérifier / Créer le DA_Art (art-editor-1125ar) ; CELEBORN-1125ar3 = AR7-a `Docs\ARTISTES.md` (art-doc-1125ar).
- 2026-11-25 20:0x (CELEBORN-1125ar3, sans C++, docs seulement) : **AR7-a FAIT EN BRANCHE** (art-doc-1125ar depuis 84ad66f, commit ca66aa8) : `Docs\ARTISTES.md` pour artistes 3D sans Claude (dossiers /Game/Art, champs du DA_Art et repli, variantes Legions (7 unites de FactionOverrides.csv), conventions -90 / MeshScale, Transform Provider Data et piege Layers, licences + Docs\assets_art.csv, Placeholder / Final, hol.Art.Gallery / hol.Art.Try, limite GitHub 100 Mo) ; notes fichier:ligne en fin. NON VERIFIE : libelles exacts des menus de l'editeur 5.8 (Data Asset, Transform Provider Data), pivot / axe avant des batiments et ressources ; hol.Art.Gallery / Try et le check Final sont sur leurs branches AR, pas dans 84ad66f. Branche prete, non fusionnee.
- 2026-11-25 20:5x (CLAUDE-1125ar) : **AR7-a FAIT EN BRANCHE** (CELEBORN-1125ar3, art-doc-1125ar ca66aa8) : `Docs\ARTISTES.md` écrit pour des artistes sans Claude (rangement, champs du DA_Art, variantes de faction, conventions, Transform Provider Data et piège de Layers, licences, Status, hol.Art.Gallery / hol.Art.Try, limite 100 Mo). Constat à reprendre (lot **AR0-a3**) : Projectile, MuzzleSmoke, Icon et le Mesh des ressources ne lisent que le DA_Art commun (pas la variante de faction) ; Portrait n'est lu nulle part. NON VÉRIFIÉ : libellés des menus de l'éditeur 5.8, axe avant du mannequin (déduit de -90), pivot / axe des bâtiments et ressources.
- 2026-11-25 21:1x (CLAUDE-1125ar) : **integ-86-art** (Glorfindel) : 4 verts fusionnés, build PASS 135 s, tests 2280/2280 PASS, parties build PASS, **net FAIL** : `net 2 clients` (triche de l'armée accordée 4 fois sur 2) et `net hero return` (or d'Alice non retiré) ; reproduit 2 fois sur integ ET sur **main c386ed2 seul** (Celeborn, binaire 17:36, aucun code changé depuis 8531390) : rouge ANTÉRIEUR aux branches art-*. Fiche `Retours\BUG-net-cheat-hero-return.md` (cause inconnue, L29-3 : run instrumenté d'abord) ; les 2 cas inscrits dans `Tools\Tests\known_reds.txt` sur integ-86-art (8e5f5a2) ; partie net relancée pour confirmer avant l'avance rapide de main.
  **AR0-d1 FAIT EN BRANCHE** (HALDIR-1125ar2, art-migrate-1125ar depuis 84ad66f) : test seul 87ab94d, rouge VU 2275/2276 (Art.Visual.MigrationKeepsEveryType) ; 69f88d5 = `Tools\Editor\make_art_visuals.py` + 51 DA_Art sous /Game/Art/{Units,Buildings,Sources}/<Type>/ (tous Provisoire) + `Tools\Data\ArtVisuals.csv` 51 lignes ; vert VU 2276/2276, check_placeholders PASS (vus par l'agent, build 484 s). Conflit d'ajout attendu sur ArtVisuals.csv avec AR0-b : garder les 51 lignes. RESTE : essai en partie (PNG avant / après), AR0-d2.
  **AR2-a** (SAURON-1125ar2, art-editor-1125ar) : rouge a65b15f (bouchons HOLArtChecks.as + 5 tests), correctif 13ac70e (règles + `Script\Art\Editor\HOLArtAssetMenu.as`), merge main e3b5ba0 ; NON COMPILÉ par l'agent (build mort avec lui) : la principale compile a65b15f puis teste.
- 2026-11-25 21:2x (CLAUDE-1125ar) : relance net sur integ-86-art = aucune régression (2 rouges connus) ; **main avancé en ff sur 8e5f5a2** : AR0-a, AR0-a2, AR0-b, AR1-a, AR4-a DANS main.
  Migration : `integ-86-art` fusionné dans art-migrate-1125ar (c65ff11, ArtVisuals.csv = 51 lignes gardées, check_placeholders PASS).
  **AR2-a** : rouge a65b15f VU 2275/2280 (HOLArtChecks_Test x5) ; vert e3b5ba0 = 0/0 : `Script\Art\Editor\HOLArtAssetMenu.as` ne compile pas (22 erreurs : signatures, LoadSynchronous sur const, bOverride* non BlueprintReadOnly, ExecutePythonCommand) -> SAURON-1125ar3 corrige (C++ liaison + script).
  Lancés (compilent et testent eux-mêmes) : HALDIR-1125ar3 = **AR0-d2** (champs par ressource dans DA_Art, puis retrait des colonnes visuelles ; branche art-strip-1125ar depuis c65ff11) ; CELEBORN-1125ar4 = **AR1-b / AR4-b** (AddSource exposé, vraies sources dans la galerie, sélection de hol.Art.Try ; branche art-gallery2-1125ar depuis main). En file : **AR0-a3** (faction du tireur pour Projectile / MuzzleSmoke, faction pour Icon).
