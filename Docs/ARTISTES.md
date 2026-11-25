# Heroes of Liberty : guide des artistes 3D

Ce guide est pour vous si vous faites des modèles, des animations ou des icônes pour le jeu et que vous travaillez
seulement dans l'éditeur Unreal. Vous n'avez pas besoin de lire le code, ni de toucher aux tableaux du jeu, ni de lancer
de script. Chaque affirmation de ce guide a été vérifiée dans le projet ; les références techniques sont en fin de
document, pour les développeurs.

## 1. Le principe en une phrase

Chaque objet du jeu (une unité, un bâtiment, une ressource) peut avoir **une fiche d'art** : un petit fichier de
l'éditeur nommé `DA_Art_<Type>`. Vous y posez votre modèle, vos animations, votre icône. Le jeu lit cette fiche avant
tout le reste. Une case laissée vide garde ce que le jeu montre aujourd'hui (le modèle provisoire) : vous pouvez donc
livrer un corps sans ses animations, ou une icône seule.

## 2. Où ranger ses fichiers

Tout votre travail va sous le dossier `Art` du Content Browser, un sous-dossier par objet :

| Famille | Dossier | Exemple |
|---|---|---|
| Unités | `/Game/Art/Units/<Type>/` | `/Game/Art/Units/Spearman/` |
| Bâtiments | `/Game/Art/Buildings/<Type>/` | `/Game/Art/Buildings/Barracks/` |
| Ressources (arbres, filons...) | `/Game/Art/Sources/<Type>/` | `/Game/Art/Sources/GoldVein/` |

`<Type>` est le nom interne de l'objet, en anglais. La commande `hol.Art.Gallery` (section 9) écrit au-dessus de chaque
objet son nom interne puis son nom en jeu. Les noms actuels :

- **Unités** : LightInfantry, LightInfantryNanite, Archer, Peasant, Zombie, Spearman, ManAtArms, DamnedWarrior, King,
  Hero, Crossbowman, Monk, Ram, Mangonel, LightCavalry, Arquebusier, Halberdier, VirtuousKnight, TempleSergeant,
  InfirmaryBrother, PaviseCrossbowman.
- **Bâtiments** : MainCentre, House, LumberCamp, MiningCamp, Mill, Barracks, Farm, Sanctuary, ArcheryRange, Market,
  SiegeWorkshop, Chapel, Stable, Forge, SecondaryCentre, TowerWood, Palisade, PalisadeGate, StoneWall.
- **Ressources** : Tree, BerryBush, StoneVein, GoldVein, StoneVeinSmall, StoneVeinMedium, StoneVeinBig, GoldVeinSmall,
  GoldVeinMedium, GoldVeinBig, FarmField.

Dans ce dossier : vos maillages (`SM_` pour un modèle fixe, `SKM_` pour un modèle à squelette), textures (`T_`),
matériaux, animations, et la fiche `DA_Art_<Type>`. **Le jeu ne cherche les fiches que sous `/Game/Art`**, et seul ce
dossier les emporte dans le jeu publié : une fiche rangée ailleurs est ignorée.

## 3. Créer la fiche d'un objet

1. Dans le dossier de l'objet : clic droit > **Data Asset** (rubrique « Miscellaneous »), classe **HOLArtVisual**.
2. La nommer `DA_Art_<Type>` (par exemple `DA_Art_Spearman`).
3. L'ouvrir et remplir **Type Name** avec le nom interne (`Spearman`). C'est ce champ qui compte, pas le nom du fichier.
4. Laisser **Faction** vide (la fiche vaut pour toutes les factions), sauf pour une variante (section 4).
5. Remplir les cases voulues, enregistrer.

Deux fiches pour le même objet et la même faction sont une erreur : le jeu garde la première et écrit dans son journal
(catégorie `LogHOLArt`) un message qui nomme les deux fichiers. Une fiche sans Type Name est signalée de la même façon.

### Les champs, un par un

Une case vide (ou une case à cocher décochée) = le jeu garde ce qu'il montre aujourd'hui pour cet objet.

**Corps**
- **Mesh** : le modèle fixe (Static Mesh). Pour un bâtiment ou une ressource, c'est son modèle. Pour une unité, c'est le
  corps de secours, utilisé seulement si elle n'a pas de modèle à squelette.
- **Skinned Mesh** : le modèle à squelette d'une unité (Skeletal Mesh), celui qui s'anime.
- **Animation Provider** : la liste des animations de l'unité (un « Transform Provider Data », section 6).
- **Mesh Yaw Offset** (case à cocher, unités) : la rotation, en degrés, ajoutée au modèle pour qu'il regarde vers l'avant.
- **Mesh Scale** (case à cocher, unités) : le facteur d'échelle du modèle à l'écran.

**Séquences** (unités seulement ; chaque valeur a sa case à cocher)
- **Idle Sequence** : numéro de l'animation de repos dans la liste de l'Animation Provider.
- **Walk Sequence** : numéro de la marche ; **Walk Play Rate** : sa vitesse de lecture (1 = normale).
- **Attack Sequence** : numéro de l'attaque.
- **Fall Sequence** : numéro de la mort (chute).
- **Build Sequence** : numéro de l'animation de construction ; **Build Play Rate** : sa vitesse.
- **Gather Play Rate** : vitesse des animations de récolte ; **Carry Walk Play Rate** : vitesse de la marche chargée.
  Les animations de récolte, de marche chargée et de repos chargé elles-mêmes (une par ressource) ne sont pas encore
  dans la fiche : demandez à l'équipe de les brancher.

**Tir** (unités à distance)
- **Projectile** : le modèle de la flèche, du carreau ou de la balle.
- **Muzzle Smoke** : la fumée à la sortie du canon (Arquebusier).
- Ces deux champs ne sont lus que dans la fiche commune : une variante de faction ne les change pas encore.

**Interface**
- **Icon** : l'icône de l'unité (Texture 2D) dans le cadre de garnison. Vide : le jeu prend
  `/Game/UI/Icons/Units/T_Unit_<Type>` s'il existe. Lue dans la fiche commune seulement.
- **Portrait** : réservé, le jeu ne l'affiche pas encore.

**Registre**
- **Status** : `Placeholder` (provisoire, par défaut) ou `Final` (section 8).
- **Author**, **License**, **Source Url** : qui l'a fait, sous quelle licence, d'où il vient (section 7).

## 4. Variantes de faction (les Légions)

Sept unités des Légions sont la même unité que celle de l'Aube, avec un autre nom ; elles ont aujourd'hui le même corps.
Pour leur donner le leur, créez une seconde fiche dans le même dossier :

- nom : `DA_Art_<Type>_Legions` (par exemple `DA_Art_Spearman_Legions`) ;
- **Type Name** : le même que l'unité de l'Aube (`Spearman`) ;
- **Faction** : `Legions`.

Le jeu cherche, case par case : la fiche de la faction, puis la fiche commune de l'objet, puis ce qu'il montre
aujourd'hui. Une variante peut donc ne remplir que **Skinned Mesh** et garder les animations de la fiche commune.

| Nom en jeu (Légions) | Type Name | Fichier |
|---|---|---|
| Lancier squelette | `Spearman` | `DA_Art_Spearman_Legions` |
| Garde noire | `ManAtArms` | `DA_Art_ManAtArms_Legions` |
| Archer squelette | `Archer` | `DA_Art_Archer_Legions` |
| Arbalétrier squelette | `Crossbowman` | `DA_Art_Crossbowman_Legions` |
| Cavalier funeste | `LightCavalry` | `DA_Art_LightCavalry_Legions` |
| Bélier d'os | `Ram` | `DA_Art_Ram_Legions` |
| Mangonneau d'os | `Mangonel` | `DA_Art_Mangonel_Legions` |

Le Zombie (`Zombie`) et le Guerrier damné (`DamnedWarrior`) sont des unités à part entière : leur fiche est
`DA_Art_Zombie` / `DA_Art_DamnedWarrior`, sans faction.

## 5. Conventions de modélisation

- **Unité de mesure** : 1 unité Unreal = 1 cm. Un humain fait environ 180 unités de haut.
- **Pivot** : au sol, entre les pieds (au centre de la base pour un bâtiment ou une ressource).
- **Axe avant** : faites regarder le modèle vers l'axe X de l'éditeur.
- **Rotation -90 et échelle 1,5 des unités actuelles** : les unités provisoires sont le mannequin d'Unreal, qui regarde
  vers l'axe Y et qui est petit pour la caméra du jeu. Le jeu le tourne donc de -90 degrés et l'agrandit : 1,5 pour un
  fantassin, 1,8 pour le Héros, 2,25 pour la cavalerie, 3,75 pour le Bélier, 4,5 pour le Mangonneau. **Ces valeurs
  s'appliquent aussi à votre modèle tant que vous ne cochez pas les cases de la fiche** :
  - modèle qui regarde vers X : cochez **Mesh Yaw Offset** et mettez 0 ; modèle qui regarde vers Y comme le mannequin :
    laissez la case décochée ;
  - modèle déjà à la taille voulue en jeu : cochez **Mesh Scale** et mettez 1 ; modèle à l'échelle du mannequin :
    laissez la case décochée.
  Ces deux réglages ne concernent que les unités. Vérifiez toujours le résultat en jeu (section 9).

## 6. Animations

Le jeu n'utilise pas d'Animation Blueprint pour les unités : il lit une liste d'animations, le **Transform Provider
Data**, et chaque état de l'unité joue un numéro de cette liste.

1. Importez votre Skeletal Mesh et ses Anim Sequences dans le dossier de l'unité.
2. Créez un asset **Transform Provider Data** (clic droit dans le dossier, cherchez « Transform Provider ») ; quand
   l'éditeur demande la classe, choisissez **Anim Sequence Transform Provider Data**. Nommez-le `TP_<Type>`.
3. Dans ce fichier : **Skinned Asset** = votre Skeletal Mesh ; **Sequences** = une entrée par animation. La première
   entrée porte le numéro 0, la deuxième le 1, etc.
4. **Piège : Layers doit avoir au moins une entrée.** Ajoutez une entrée (laissée par défaut) à **Layers**. Sans elle,
   rien ne joue, sans message clair.
5. Dans la fiche `DA_Art_<Type>` : **Animation Provider** = ce `TP_<Type>`, puis cochez chaque séquence et donnez son
   numéro dans votre liste (Idle = repos, Walk = marche, Attack = attaque, Fall = mort, Build = construction).

Une séquence non cochée garde le numéro actuel de l'unité (par exemple l'Archer a aujourd'hui le repos en 2 et la marche
en 3), qui pointera alors dans **votre** liste : cochez donc toutes les séquences dès que vous posez votre propre
Animation Provider.

## 7. Licences

N'importez que :
- vos propres créations ;
- ou des fichiers sous licence **CC0** ;
- ou des fichiers sous une licence **écrite** qui permet l'usage dans un jeu vendu (gardez le texte de la licence).

Chaque source extérieure (pack, site, auteur) a sa ligne dans le registre `Docs/assets_art.csv` du projet : fichier ou
pack, source, licence, adresse, date de téléchargement, preuve de licence (chemin du texte de licence), où il est
utilisé. Remplissez aussi **Author**, **License** et **Source Url** dans la fiche. Si vous ne pouvez pas modifier ce
registre, envoyez ces informations à l'équipe avec votre livraison.

## 8. Provisoire ou final

- **Status = Placeholder** (par défaut) : votre travail est un essai, l'objet reste compté comme provisoire.
- **Status = Final** : l'objet est terminé. Prévenez l'équipe : sa ligne sort alors de `Placeholders.txt` (la liste de
  tout ce qui est provisoire dans le jeu, une ligne par objet, par exemple « Lancier Légion (Lancier squelette)
  [mannequin] »). Une fiche `Final` dont la ligne reste dans cette liste fait échouer le contrôle automatique du projet.

## 9. Voir son travail

Lancez le jeu depuis l'éditeur (Play), ouvrez la console (touche `²` ou `~`) et tapez :

- `hol.Art.Gallery` : pose tous les objets du jeu côte à côte, avec leur nom interne et leur nom en jeu ;
  `hol.Art.Gallery Units` (ou `Buildings`, `Sources`, `All`) n'en montre qu'une famille.
- `hol.Art.Try <Type> [N]` : dans une partie normale déjà lancée, pose N exemplaires de l'objet à côté de votre Forum,
  avec construction rapide et +1000 ressources, pour le voir jouer (par exemple `hol.Art.Try Spearman 5`). Un nom
  inconnu affiche les noms proches. Les ressources ne se posent pas encore ainsi.

Les fiches sont lues au démarrage du jeu : après un enregistrement, arrêtez puis relancez Play.

## 10. Taille des fichiers et travail à plusieurs

Le projet est sur GitHub : **un fichier de plus de 100 Mo est refusé**. Gardez chaque fichier sous cette limite
(textures en 2048 ou moins, fichiers sources lourds hors du projet). Il n'y a pas encore de verrou de fichier : prévenez
l'équipe avant de modifier une fiche ou un modèle qu'un autre a pu toucher (un `.uasset` ne se fusionne pas).

---

## Notes pour les agents (vérification, fichier:ligne, base 84ad66f sauf mention)

1. Fiche = `UHOLArtVisual : UPrimaryDataAsset`, champs et défauts : `Source/HeroesOfLiberty/Art/HOLArtVisual.h:31-155`
   (TypeName :44, Faction :48, Mesh :52, SkinnedMesh :55, AnimationProvider :58, MeshYawOffset :60-64, MeshScale :66-70,
   séquences :73-125, Projectile/MuzzleSmoke :129-132, Icon/Portrait :135-138, Status/Author/License/SourceUrl :142-151).
2. Champ vide ou case décochée = inchangé : `HOLArtVisual.cpp:35-64` (`IsNull`, lambda `Set` sur `bOverride*`).
3. Recherche sous `/Game/Art` seulement : `HOLArtVisual.cpp:22`, `:74`, `:82` ; cuisson : `Config/DefaultGame.ini:196-197`
   (`PrimaryAssetTypesToScan`, `CookRule=AlwaysCook`). Registre = `UGameInstanceSubsystem` (`HOLArtVisual.h:161-169`),
   d'où « relancez Play ».
4. Doublon / TypeName vide : `HOLArtVisual.cpp:158`, `:164`, journal `LogHOLArt` `:17`, `:98`.
5. Repli faction -> commun -> CSV : `HOLArtVisual.h:24-28`, `:221-229` ; unités : `HOLUnitSubsystem.cpp:333` (CRC de
   faction), `:406` (enregistrement). Faction `Legions` = clé de `Tools/Data/FactionOverrides.csv`, résolue par
   `HOLFactionRules::ResolveKey` (`HOLArtVisual.cpp:194`).
6. Faction NON lue (NAME_None) : flèches `Presentation/HOLArrowSubsystem.cpp:91`, `:107` ; icône
   `Script/UI/HOLGarrisonFrameRules.as:32`, repli `T_Unit_<Type>` `:35` ; ressources `Resources/HOLSourceField.cpp:242`.
   Bâtiments : `Buildings/HOLBuilding.cpp:1241` (ResolveMesh). Portrait : aucune lecture dans `HOLArtVisual.cpp`.
7. 7 unités des Légions : `Tools/Data/FactionOverrides.csv:2,4,6,8,10,12,14` (lignes `DisplayName`).
   Zombie / DamnedWarrior = lignes propres de `Tools/Data/UnitTypes.csv`.
8. Noms internes : 1re colonne de `Tools/Data/UnitTypes.csv`, `BuildingTypes.csv`, `SourceTypes.csv`.
9. -90 / échelles : colonnes `MeshYawOffset` (8) et `MeshScale` (9) de `UnitTypes.csv` (toutes -90 ; 1.5, Hero 1.8,
   LightCavalry 2.25, Ram 3.75, Mangonel 4.5) ; appliqué en rotation de lacet : `HOLUnitSubsystem.cpp:537`.
   « Mannequin regarde vers Y » : convention du mannequin UE5, déduite de l'offset -90, non vue à l'écran.
10. Séquences de l'Archer (Idle 2, Walk 3) : colonnes 12-13 de `UnitTypes.csv`. Séquences par ressource (`Gather*`,
    `CarryWalk*`, `CarryIdle*`, colonnes 15-19) absentes de la fiche.
11. Transform Provider : `C:\UE\UE_5.8_HOL\Engine\Source\Editor\UnrealEd\Private\Factories\TransformProviderFactory.cpp:32`,
    `:111` (choix de classe) ; `Layers` : `Engine\Source\Runtime\Engine\Classes\Animation\AnimSequenceTransformProviderData.h:662`,
    `InitLayerPools` `Private\Animation\AnimSequenceTransformProviderData.cpp:1426` ; modèle :
    `Tools/Editor/make_skinned_mannequin.py:148` (« The runtime needs at least one layer »).
12. Registre de licences : `Docs/assets_art.csv:1` (en-tête) ; règle CC0 / licence écrite : `Placeholders.txt` étiquette
    `[kit]` et `ArtistesInProgress.md` (AR2, AR7).
13. Final et Placeholders : branche `art-manifest-1125ar` (AR0-b, 49af701), `Tools/check_placeholders.ps1:13-14`, `:30`
    (manifeste `Tools/Data/ArtVisuals.csv`, pas encore écrit par l'éditeur). Exemple de ligne : `Placeholders.txt:20`.
14. `hol.Art.Gallery` : branche `art-gallery-1125ar` (9d2e9f0), `Script/Art/HOLArtGallery.as:1`, `:160` (étiquette
    nom interne + nom en jeu), `:234`. `hol.Art.Try` : branche `art-try-1125ar` (e5fd62e), `Script/Art/HOLArtTry.as:1-2`,
    `:183` (ressource non posée), `:192` (noms proches), `:199` (partie Plain requise). Absents de la base 84ad66f :
    disponibles après l'intégration des branches AR.
15. Console : `Config/DefaultInput.ini:95-96` (Tilde, ²). GitHub 100 Mo et absence de LFS : `.gitattributes:6-14`
    (`binary`, aucun `lockable`).
16. NON VÉRIFIÉ : libellé exact du menu « Data Asset » / « Transform Provider Data » dans l'éditeur 5.8 ; pivot et axe
    avant des bâtiments et ressources (aucune correction de lacet ni d'échelle lue pour eux) ; rien vu à l'écran.
