# Heroes of Liberty : guide des artistes 3D

Ce guide est pour vous si vous faites des modèles, des animations ou des icônes pour le jeu et que vous travaillez
seulement dans l'éditeur Unreal. Vous n'avez pas besoin de lire le code, ni de toucher aux tableaux du jeu, ni de lancer
de script. Chaque affirmation de ce guide a été vérifiée dans le projet ; les références techniques sont en fin de
document, pour les développeurs. Ce qui n'est pas encore dans le jeu est regroupé à la section 12 (« À venir »).

## 0. Installer son poste (une fois)

Pas besoin de Visual Studio ni de compiler : l'équipe range dans le projet l'éditeur du jeu déjà compilé.

1. Installez **Git** (https://git-scm.com) et **Git LFS** (https://git-lfs.com), puis tapez dans un terminal :
   `git lfs install`.
2. Récupérez le projet : `git clone <adresse donnée par l'équipe> HeroesOfLiberty`.
3. **Le moteur** : l'équipe vous donne le zip du moteur « UE 5.8 HOL ». Décompressez-le (par exemple dans
   `C:\UE\UE_5.8_HOL`), puis, dans un terminal ouvert dans le dossier du projet :
   `powershell -ExecutionPolicy Bypass -File Tools\Art\register_engine.ps1 -EnginePath C:\UE\UE_5.8_HOL`.
   Si l'éditeur ne démarre pas ensuite, installez une fois `Engine\Extras\Redist\en-us\vc_redist.x64.exe` du moteur.
4. Ouvrez l'éditeur en double-cliquant **`OpenEditor.cmd`**, à la racine du projet. Pas le fichier `.uproject` : le
   script met d'abord en place l'éditeur compilé.

Chaque jour : `git pull`, puis `OpenEditor.cmd`. Ce qu'il peut vous dire :

| Message | Que faire |
|---|---|
| « moteur … introuvable » | refaites l'étape 3 |
| « faites pour un autre moteur » | demandez le zip du moteur à jour |
| « WARN le code C++ a changé après ces DLL » | l'éditeur s'ouvre quand même ; demandez la publication des DLL et n'enregistrez pas de fiche en attendant |
| l'éditeur propose de reconstruire (« rebuild ») des modules | répondez **Non** et prévenez l'équipe |

## 1. Le principe en une phrase

Chaque objet du jeu (une unité, un bâtiment, une ressource) a **une fiche d'art** : un petit fichier de l'éditeur nommé
`DA_Art_<Type>`. Vous y posez votre modèle, vos animations, votre icône. Le jeu lit cette fiche avant tout le reste. Une
case laissée vide garde ce que le jeu montre aujourd'hui (le modèle provisoire) : vous pouvez donc livrer un corps sans
ses animations, ou une icône seule.

Les fiches des 51 objets actuels existent déjà, remplies avec l'art provisoire (mannequins, formes simples) et marquées
provisoires : vous **ouvrez** la fiche de votre objet et vous remplacez ce qu'elle contient. La fiche est la seule
source de l'art d'un objet : on ne touche plus aux tableaux du jeu pour changer un modèle.

## 2. Où ranger ses fichiers

Tout votre travail va sous le dossier `Art` du Content Browser, un sous-dossier par objet :

| Famille | Dossier | Exemple |
|---|---|---|
| Unités | `/Game/Art/Units/<Type>/` | `/Game/Art/Units/Spearman/` |
| Bâtiments | `/Game/Art/Buildings/<Type>/` | `/Game/Art/Buildings/Barracks/` |
| Ressources (arbres, filons...) | `/Game/Art/Sources/<Type>/` | `/Game/Art/Sources/GoldVein/` |

`<Type>` est le nom interne de l'objet, en anglais. La commande `hol.Art.Gallery` (section 10) écrit au-dessus de chaque
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

## 3. Créer ou ouvrir la fiche d'un objet

La fiche de chaque objet actuel existe déjà (section 1) : ouvrez `DA_Art_<Type>` dans le dossier de l'objet. Pour un
objet neuf, deux façons :

- **Par le menu (conseillé)** : clic droit sur votre Static Mesh ou votre Skeletal Mesh > rubrique **HOL Art** >
  **Create the DA_Art of this type**. L'éditeur demande **Type Name** (et **Faction**, à laisser vide sauf variante,
  section 4). Il crée `DA_Art_<Type>` dans le bon dossier (`/Game/Art/<Famille>/<Type>/`), y pose votre modèle et
  l'enregistre ; un message vert en bas de l'écran donne son chemin. Un nom inconnu des tableaux du jeu ne crée rien
  (message rouge) ; une fiche qui existe déjà n'est pas écrasée.
- **À la main** : dans le dossier de l'objet, clic droit > **Data Asset** (rubrique « Miscellaneous »), classe
  **HOLArtVisual** ; la nommer `DA_Art_<Type>` (par exemple `DA_Art_Spearman`) ; remplir **Type Name** avec le nom
  interne (`Spearman`) : c'est ce champ qui compte, pas le nom du fichier ; laisser **Faction** vide (la fiche vaut pour
  toutes les factions), sauf pour une variante (section 4).

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
- **Gather / Carry Walk / Carry Idle Sequences** : numéros de la récolte, de la marche chargée et du repos chargé, un par
  ressource (Food, Wood, Stone, Gold).

**Animations** (unités seulement ; la façon conseillée, section 6)
- **Idle / Walk / Attack / Fall / Build Animation** : l'Anim Sequence elle-même de chaque état, choisie dans la liste.
- **Gather / Carry Walk / Carry Idle Animations** : la même chose par ressource.
- Une animation choisie remplace le numéro de la même ligne ; une case vide garde le numéro.

**Tir** (unités à distance, et bâtiments qui tirent comme la tour de bois)
- **Projectile** : le modèle de la flèche, du carreau ou de la balle.
- **Muzzle Smoke** : la fumée à la sortie du canon (Arquebusier).

**Interface**
- **Icon** : l'icône de l'unité (Texture 2D) dans le cadre de garnison. Vide : le jeu prend
  `/Game/UI/Icons/Units/T_Unit_<Type>` s'il existe.
  Chemins des icônes de la cabine (AR5-e, `Tools\Editor\import_art_icons.py`, depuis `Saved\ArtIcons` du scénario `art_icon_all`) :
  unités `/Game/UI/Icons/Units/T_Unit_<Type>`, bâtiments `/Game/UI/Icons/Buildings/T_Building_<Type>`, sources
  `/Game/UI/Icons/Sources/T_Source_<Type>` ; réglages d'icône d'interface (sans mip, compression UserInterface2D, sRGB, groupe UI) ;
  le script remplit aussi Icon de la fiche commune du type. Pour une icône dessinée à la main : même nom, même dossier.
- **Portrait** : réservé, le jeu ne l'affiche pas encore.

**Registre**
- **Status** : `Placeholder` (provisoire, par défaut) ou `Final` (section 8).
- **Author**, **License**, **Source Url** : qui l'a fait, sous quelle licence, d'où il vient (section 7).

## 4. Variantes de faction (les Légions)

Sept unités des Légions sont la même unité que celle de l'Aube, avec un autre nom ; elles ont aujourd'hui le même corps.
Pour leur donner le leur, créez une seconde fiche dans le même dossier :

- par le menu : clic droit sur la fiche commune `DA_Art_<Type>` > **HOL Art** > **Create the faction variant of this
  DA_Art**, puis donnez la faction (`Legions`) ;
- ou à la main : nom `DA_Art_<Type>_Legions` (par exemple `DA_Art_Spearman_Legions`), **Type Name** le même que l'unité
  de l'Aube (`Spearman`), **Faction** `Legions`.

Le jeu cherche, case par case : la fiche de la faction, puis la fiche commune de l'objet, puis ce qu'il montre
aujourd'hui. Une variante peut donc ne remplir que **Skinned Mesh** et garder les animations de la fiche commune.

La variante vaut pour **toutes** les cases d'une unité, y compris **Projectile**, **Muzzle Smoke** et **Icon** : la
flèche d'un archer squelette, la fumée d'un tireur des Légions et l'icône d'une unité des Légions dans la garnison
viennent de la variante si elle les remplit. C'est la faction du **propriétaire** qui compte (celle du tireur, ou du
joueur qui possède le bâtiment) ; cela vaut aussi pour la flèche d'une tour : `DA_Art_TowerWood_Legions` puis
`DA_Art_TowerWood`. Les ressources (arbres, filons) n'ont pas de faction : seule leur fiche commune compte.

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

- **Unité de mesure** : 1 unité Unreal = 1 cm. Un humain fait environ 180 unités de haut (la barre de 1,80 m de la
  galerie, section 10, sert d'étalon).
- **Pivot** : au sol, entre les pieds (au centre de la base pour un bâtiment ou une ressource). Le menu **Check for the
  game** (section 9) le vérifie.
- **Axe avant** : faites regarder le modèle vers l'axe X de l'éditeur.
- **Rotation -90 et échelle 1,5 des unités actuelles** : les unités provisoires sont le mannequin d'Unreal, qui regarde
  vers l'axe Y et qui est petit pour la caméra du jeu. Le jeu le tourne donc de -90 degrés et l'agrandit : 1,5 pour un
  fantassin, 1,8 pour le Héros, 2,25 pour la cavalerie, 3,75 pour le Bélier, 4,5 pour le Mangonneau. **Ces valeurs
  s'appliquent aussi à votre modèle tant que vous ne cochez pas les cases de la fiche** :
  - modèle qui regarde vers X : cochez **Mesh Yaw Offset** et mettez 0 ; modèle qui regarde vers Y comme le mannequin :
    laissez la case décochée ;
  - modèle déjà à la taille voulue en jeu : cochez **Mesh Scale** et mettez 1 ; modèle à l'échelle du mannequin :
    laissez la case décochée.
  Ces deux réglages ne concernent que les unités. Vérifiez toujours le résultat en jeu (section 10).

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
5. Dans la fiche `DA_Art_<Type>` : **Animation Provider** = ce `TP_<Type>`, puis, dans **Animations**, choisissez
   l'animation de chaque état (Idle Animation = repos, Walk Animation = marche, Attack Animation = attaque, Fall
   Animation = mort, Build Animation = construction ; Gather / Carry Walk / Carry Idle Animations = récolte, marche
   chargée, repos chargé, par ressource). **Vous choisissez l'animation, plus son numéro** : au lancement, le jeu
   retrouve lui-même sa place dans la liste du `TP_<Type>`, même si vous réordonnez la liste.

Une animation choisie mais **absente** de la liste du `TP_<Type>` : le jeu écrit une erreur dans le journal qui nomme
l'unité, l'état et l'animation (`Art of 'Archer': the Walk animation ... is not in the sequences of ...`) et garde le
numéro de la ligne **Séquences**. Ajoutez alors l'animation à la liste, ou choisissez-en une qui y est.

Une case d'animation vide garde le numéro (section 3, **Séquences**) : l'ancienne façon marche encore. Un état sans
animation choisie ni numéro coché garde le numéro actuel de l'unité (par exemple l'Archer a aujourd'hui le repos en 2
et la marche en 3), qui pointerait alors dans **votre** liste : choisissez donc l'animation de chaque état dès que vous
posez votre propre Animation Provider. Les fiches livrées ont déjà leurs animations choisies (`Tools\Editor\art_visual_io.py`).
**Check for the game** sur la fiche (section 9) signale un numéro hors de votre liste.

### Sons de la fiche

La fiche a cinq emplacements de son (catégorie **Art > Sounds**) : **Selection Sound** (l'objet est sélectionné),
**Order Sound** (il reçoit un ordre), **Attack Sound**, **Death Sound**, et **Construction Sound** pour un bâtiment.
Glissez-y un son importé (Sound Wave ou Sound Cue) rangé dans le dossier de l'objet. Un emplacement vide garde le son
actuel du jeu (sa table de sons). Comme pour le reste de la fiche, la variante de faction passe d'abord, puis la fiche
commune, emplacement par emplacement. **Check for the game** (section 9) signale un son posé qui ne se charge pas.
Quand chaque emplacement sonne :

| Emplacement | Quand | Comment |
|---|---|---|
| Selection Sound | vous sélectionnez un bâtiment | à plat (2D), au volume plein |
| Order Sound | vous donnez un ordre (clic droit, A + clic, ordre en ligne) : la première unité commandée répond | à plat, seulement chez vous |
| Attack Sound | chaque coup ou tir d'une unité, chaque salve d'une tour (une fois par salve) ; aussi chez un joueur en réseau | dans le monde (3D), au plus 8 sons de combat à la fois |
| Death Sound | une unité tombe | dans le monde (3D), même limite |
| Construction Sound | vous posez la fondation d'un bâtiment, puis quand sa construction se termine | à plat, seulement chez vous |

Sans son d'artiste, les ordres, les coups, les morts et la fin de construction restent **muets** (choix fait pour le jeu) ;
seule la sélection d'un bâtiment a déjà un son provisoire. Pour entendre votre son : `hol.Art.Try <Type>` (section 10).

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
- **Status = Final** : l'objet est terminé. **Prévenez l'équipe** : elle passe l'objet à `Final` dans la liste des
  fiches du projet (`Tools\Data\ArtVisuals.csv`, une ligne par fiche, colonne Status `Provisoire` ou `Final`) et retire
  sa ligne de `Placeholders.txt` (la liste de tout ce qui est provisoire dans le jeu, une ligne par objet, par exemple
  « Lancier Légion (Lancier squelette) [mannequin] »).

Le contrôle automatique du projet (`check_placeholders`) lit ces deux listes :
- un objet n'est **final** que si **toutes** ses fiches (la commune et chaque variante de faction) sont `Final` dans la
  liste des fiches ; une seule fiche encore `Provisoire` le laisse provisoire ;
- un objet final dont la ligne reste dans `Placeholders.txt` fait **échouer** le contrôle ;
- un objet provisoire qui n'a pas de ligne dans `Placeholders.txt` le fait aussi échouer.

## 9. Vérifier dans l'éditeur (clic droit > HOL Art > Check for the game)

Clic droit sur un **Static Mesh**, un **Skeletal Mesh** ou une fiche **DA_Art** (classe HOLArtVisual) > rubrique **HOL
Art** > **Check for the game**. Le résultat s'écrit dans l'Output Log, une ligne par règle, `[OK]` ou `[KO]` (les `[KO]`
en rouge), avec un résumé vert ou rouge en bas de l'écran.

| Sur | Ce qui est vérifié |
|---|---|
| Static Mesh | pivot au sol (bas du modèle à 0, à 2 cm près) ; 1 à 4 emplacements de matériau ; triangles (LOD 0) : 8 000 au plus sous `/Art/Units/`, 40 000 ailleurs ; nom qui commence par `SM_` ; rangé sous `/Game/Art/` |
| Skeletal Mesh | pivot au sol ; 1 à 4 emplacements de matériau ; nom qui commence par `SKM_` ; rangé sous `/Game/Art/` |
| Fiche DA_Art | Type Name connu des tableaux du jeu ; rangée sous `/Game/Art/` ; chaque case remplie (Mesh, Skinned Mesh, Animation Provider, Icon, Portrait) s'ouvre bien ; chaque séquence cochée (Idle, Walk, Attack, Fall, Build) est un numéro de votre liste d'animations |

Les seuils (triangles, matériaux) sont provisoires : ils seront revus quand le vrai art arrivera.

## 10. Voir son travail

Lancez le jeu depuis l'éditeur (Play), ouvrez la console (touche `²` ou `~`) et tapez :

- **`hol.Art.Lab`** (le plus simple, à taper en premier) : dans une partie Plain en cours, une seule commande pose tout
  l'atelier : combat coupé, construction rapide et ressources, galerie complète (`hol.Art.Gallery All`, forêt dégagée),
  éclairage Herbe fraîche, caméra sur la galerie des unités, et l'aide des commandes `hol.Art.*` affichée à l'écran.
  `hol.Art.Lab Neige` (ou `Steppe sèche`, `Automne`, `Forêt sombre`) choisit un autre éclairage ; les commandes
  ci-dessous restent disponibles une par une.
- `hol.Art.Gallery` : dans une partie normale, pose un exemplaire de chaque objet du jeu (lu dans les tableaux du jeu :
  un objet ajouté y apparaît tout seul), loin de votre Forum, combat coupé, avec son nom interne et son nom en jeu écrits
  au sol. `hol.Art.Gallery Units` (ou `Buildings`, `Sources`, `All`) n'en montre qu'une famille ;
  `hol.Art.GalleryFrame Units` (ou `Buildings`, `Sources`) place la caméra sur une famille. Les ressources sont de vraies
  ressources du jeu. **Repères d'échelle** à côté de chaque objet :
  - une **barre de 1,80 m** (un homme debout) ;
  - pour un bâtiment : le **contour de son emprise** au sol, la **grille** des cases de 4 m et de leurs sous-cases à
    l'intérieur, et un **poteau** à un coin qui monte à la hauteur prévue du bâtiment.
- `hol.Art.Light <biome>` : montre la galerie sous la lumière, la brume, le sol et le décor d'un biome du jeu :
  `fresh_grass`, `dry_steppe`, `autumn`, `dark_forest`, `snow` (ou son numéro) ; `hol.Art.Light default` revient à
  l'aspect de la carte. Un nom inconnu ne change rien et écrit la liste des noms dans l'Output Log.
- `hol.Art.Try <Type> [N]` : dans une partie normale déjà lancée, pose N exemplaires (1 à 50) de l'objet à côté de votre
  Forum, avec construction rapide et +1000 ressources, pour le voir jouer (par exemple `hol.Art.Try Spearman 5`). Les
  unités posées sont **sélectionnées** et la caméra va sur elles : un clic droit les fait marcher ou attaquer. Un
  bâtiment est posé fini ; une ressource (par exemple `hol.Art.Try GoldVein`) est posée avec sa quantité normale. Un nom
  inconnu affiche les noms proches.

Les fiches sont lues au démarrage du jeu : après un enregistrement, arrêtez puis relancez Play.

## 11. Taille des fichiers et travail à plusieurs

Les fichiers d'art passent par **Git LFS** (gros fichiers rangés à part, avec verrous) : tout ce qui est sous
`Content/Art/` (fiches, modèles, textures importés) et sous `ArtSource/` à la racine du projet (vos fichiers sources :
FBX, PSD, textures sources ; ce dossier n'entre pas dans le jeu). Rangez vos sources dans
`ArtSource/<Units|Buildings|Sources>/<Type>/`, comme dans `Content/Art`.

**Une fois par poste** : installez Git LFS (https://git-lfs.com) puis tapez `git lfs install` dans un terminal.

**Verrouiller avant de modifier.** Un `.uasset` ne se fusionne pas : si deux personnes modifient le même fichier, le
travail de l'une est perdu. Avant de toucher un fichier existant, prenez son verrou ; tant que vous ne l'avez pas, le
fichier est en lecture seule sur votre poste (l'éditeur refuse de l'enregistrer : c'est voulu). Dans un terminal ouvert
dans le dossier du projet :

| Pour | Tapez |
|---|---|
| voir qui tient quoi | `git lfs locks` |
| prendre un fichier | `git lfs lock Content/Art/Units/Spearman/DA_Art_Spearman.uasset` |
| le rendre, **après** avoir envoyé (push) votre travail | `git lfs unlock Content/Art/Units/Spearman/DA_Art_Spearman.uasset` |

Si `lock` répond que le fichier est déjà verrouillé, il affiche le nom de la personne : voyez avec elle. Un fichier
**neuf** (nouveau modèle, nouvelle texture) n'a pas besoin de verrou tant qu'il n'est pas envoyé. L'éditeur (Source
Control « Git ») n'affiche pas encore les verrous : passez par le terminal.

Le stockage LFS du compte GitHub est limité : gardez les textures en 2048 ou moins et ne versionnez pas les exports
intermédiaires. Hors de ces deux dossiers, un fichier de plus de 100 Mo reste refusé par GitHub.

## 12. À venir (pas encore dans le jeu)

Ces points sont en préparation ; ce guide sera mis à jour quand ils seront dans le jeu.

- **Fin des colonnes d'art dans les tableaux** : aujourd'hui le tableau des unités garde encore ses anciennes colonnes de
  modèle et d'animations, derrière la fiche (la fiche passe en premier). Elles vont disparaître, de même que le modèle
  des bâtiments et des ressources dans leurs tableaux : la fiche restera la seule source de l'art.
- **Menu sur une Anim Sequence** (« Check for the game » : squelette, durée, nombre d'images), contrôle de **Projectile**
  et **Muzzle Smoke** et des triangles d'un Skeletal Mesh, et **See in the Workshop** (affiche la commande `hol.Art.Try`
  à taper pour voir l'objet).
- **Icônes** : une commande qui photographie chaque objet sous le même angle pour en faire l'icône.
- **Sons** de la fiche : ordre, attaque, mort et construction branchés en jeu (la sélection d'un bâtiment l'est déjà).

---

## Notes pour les agents (vérification, fichier:ligne, base `main` 0f81c1e)

1. Fiche = `UHOLArtVisual : UPrimaryDataAsset`, `Source/HeroesOfLiberty/Art/HOLArtVisual.h` ; `EHOLArtStatus`
   Placeholder / Final `:18-21`, défaut Placeholder `:142`. Champ vide ou case décochée = inchangé :
   `HOLArtVisual.cpp:35-64`. Recherche sous `/Game/Art` seulement et cuisson : `HOLArtVisual.cpp:22`, `:74`, `:82`,
   `Config/DefaultGame.ini:196-197`. Doublon / TypeName vide : `HOLArtVisual.cpp:158`, `:164` (`LogHOLArt`).
   (lignes de `HOLArtVisual.cpp` reprises de la note précédente, base 84ad66f, non relues sur 0f81c1e)
2. 51 fiches existantes : `Tools/Data/ArtVisuals.csv` (51 lignes hors en-tête, toutes `Provisoire`) et 51
   `Content/Art/**/DA_Art_*.uasset` ; écrites par `Tools/Editor/make_art_visuals.py:5-7`, Status Placeholder `:126`,
   manifeste `Provisoire` `:139` (le relancer réécrit le manifeste en `Provisoire` : un `Final` posé à la main serait
   perdu). D925 (GDD) : DA_Art seule source après retrait des colonnes.
3. Menus d'éditeur : `Script/Art/Editor/HOLArtAssetMenu.as:140-186` (`UScriptAssetMenuExtension`, Category « HOL Art »,
   SupportedClasses UStaticMesh `:142`, USkeletalMesh `:159`, UHOLArtVisual `:176` ; « Check for the game » `:144`,
   `:161`, `:178` ; « Create the DA_Art of this type » `:150`, `:167` ; « Create the faction variant of this DA_Art »
   `:184`). Création par Python, refus type inconnu `:112-120`, fiche existante non écrasée `:126`, message `:136`.
   Contrôles : `CheckStaticMesh` `:68-79`, `CheckSkeletalMesh` `:81-89` (pas de triangles), `CheckArtVisual` `:91-110`,
   `AddCommon` `:62-66`, rapport `Log` + `Print` `:41-58`. Règles et seuils : `Script/Art/HOLArtChecks.as:16-22`
   (pivot 2 cm, 4 matériaux, 8000 / 40000 triangles, `/Game/Art/`), `:24-64`, chemin `:96-104`.
4. Variante de faction : Projectile / MuzzleSmoke `Source/HeroesOfLiberty/Presentation/HOLArrowSubsystem.cpp:85-108`
   (ShooterFaction), `SetUnitShooter` `:382` ; client `Net/HOLNetUnitDisplay.cpp:158`, `:172` ; tour
   `Buildings/HOLBuilding.cpp:417` (`SetBuildingShooter`, slot propriétaire). Icône : `Script/UI/HOLGarrisonFrameRules.as:27-35`
   (DA_Art faction puis commun puis `T_Unit_<Type>`), `OwnerFactionKey` `:39-42`, appel `Script/UI/HOLGarrisonFrame.as:133`.
   Ressources sans faction : `Resources/HOLSourceField.cpp:238-242` (NAME_None). Portrait : aucune lecture dans
   `Source/HeroesOfLiberty/**/*.cpp` (grep). 7 unités des Légions : `Tools/Data/FactionOverrides.csv:2,4,6,8,10,12,14`.
5. Provisoire / final : `Tools/check_placeholders.ps1:12-15` (manifeste, Status Provisoire|Final), `:30-36` (final =
   toutes les lignes du type Final), `:42-47` (non final : Mesh CSV vide ou mannequin sans ligne = problème), `:51-52`
   (final encore listé = problème). Dans 0f81c1e le jugement « provisoire » lit encore `Row.Mesh` du CSV (`:43-44`).
6. Galerie : `Script/Art/HOLArtGallery.as:1-11` (tables, combat coupé, vraies sources `HOLSourceAdd::AddNeutralSource`,
   `hol.Art.GalleryFrame`), repères `:109-119` (contour, case 4 m, sous-case, poteau ; barre `ReferenceHeightCm` 180),
   `:141-195` (emprise `Footprint`, grille, poteau `Height`), `hol.Art.Light` `:197-202`, `:526-577` (clés HOLBiome via
   `hol.Biome.Index` `:544` ; nom inconnu = ligne `LogHOLMatch` « Art light: unknown biome » `:533-541`).
7. `hol.Art.Try` : `Script/Art/HOLArtTry.as:1-5` (unité sélectionnée + caméra, bâtiment fini, source à sa quantité),
   sélection répétée jusqu'aux N copies `:146-156`, N borné 1..50 `:219`, partie Plain requise `:244`, FastBuild +
   Resources 1000 `:247-248`, noms proches `:237`.
8. Noms internes : 1re colonne de `Tools/Data/UnitTypes.csv`, `BuildingTypes.csv`, `SourceTypes.csv`. -90 / échelles :
   colonnes `MeshYawOffset` / `MeshScale` de `UnitTypes.csv` (encore présentes dans 0f81c1e, en-tête ligne 1).
9. Transform Provider et `Layers` : `TransformProviderFactory.cpp:32`, `:111`, `AnimSequenceTransformProviderData.h:662`,
   `Tools/Editor/make_skinned_mannequin.py:148` (note précédente, non relue). Console : `Config/DefaultInput.ini:95-96`.
   Registre de licences : `Docs/assets_art.csv:1`.
   Git LFS (D926) : fin de `.gitattributes` (`Content/Art/**`, `ArtSource/**` : `filter=lfs diff=lfs merge=lfs -text
   lockable`, après les lignes `binary` qu'elles remplacent) ; les 51 DA_Art convertis en pointeurs par
   `git add --renormalize` (commit f026453, historique non réécrit) ; test `Tools/Tests/art_lfs_test.ps1`. Poste des
   agents : `git config lfs.setlockablereadonly false` (sinon les DA_Art sont en lecture seule et les scripts d'éditeur
   `make_art_visuals.py` / `import_art_icons.py` ne peuvent plus les écrire) ; hooks `git lfs install` posés le
   2026-11-26 (`pre-push` envoie les objets LFS). Plugin « Git » de l'éditeur 5.8 : lit la version de git-lfs
   (`GitSourceControlUtils.cpp:428`) mais ne prend aucun verrou : verrous en ligne de commande.
10. À venir, branches hors de `main` 0f81c1e (`git merge-base --is-ancestor`) : `art-strip-1125ar` (retrait des colonnes,
    champs par ressource, check_placeholders par DA_Art), `art-editor2-1125ar` (menu AnimSequence, See in the Workshop,
    contrôles Projectile / MuzzleSmoke / triangles SkeletalMesh), `art-anim-1125ar` (animations par référence),
    `art-icons-1125ar` (`hol.Art.Icon`), `art-scripts-1125ar` ; sons (AR6) non commencés.
11. NON VÉRIFIÉ : libellés des menus de l'éditeur 5.8 (« Data Asset », « Transform Provider Data ») et apparence réelle
    du menu « HOL Art » et de la boîte de saisie Type Name / Faction (lus dans le script, pas vus à l'écran) ; pivot et
    axe avant des bâtiments et ressources ; rien vu à l'écran pour ce guide.
12. Kit d'éditeur des artistes (D927, section 0) : **après chaque intégration dans `main` qui touche le C++**, depuis le
    worktree d'intégration compilé : `Tools\Art\publish_editor_binaries.ps1 -From <worktree> -Root <worktree>` (copie
    `UnrealEditor.modules` et les DLL qu'il nomme vers `ArtistBinaries\Win64`, écrit `source_commit.txt` ; refuse un C++
    non commité et une DLL plus vieille que le dernier commit de son module `Source/<Module>`, UBT ne reliant que les
    modules changés), puis commiter `ArtistBinaries` (LFS, ~14 Mo par publication). `OpenEditor.cmd` ->
    `Tools\Art\open_editor.ps1` (moteur par `EngineAssociation` dans `HKCU:\Software\Epic Games\Unreal Engine\Builds`,
    BuildId du kit = BuildId du moteur, recopie dans `Binaries\Win64` sauf poste de dev `Intermediate\Build` sans `-Force`,
    WARN si un commit C++ n'est pas ancêtre de `source_commit.txt`). Moteur une fois : `Tools\Art\register_engine.ps1`.
    Pas dans `Binaries\Win64` directement : chaque build y réécrit les DLL (arbres sales dans tous les worktrees). Test :
    `Tools\Tests\artist_binaries_test.ps1` (dossiers temporaires ; `--template=` : le hook modèle `reference-transaction`
    de `~\.git-templates` interroge les verrous du distant au clonage et bloque sans fin). Zip du moteur : pas encore d'outil.
13. Sons de la fiche (AR6-d) : `UHOLCombatSoundSubsystem` (`Presentation/HOLCombatSoundSubsystem.h`) : `OnCombatSound`
    (coups `HOLCombatProcessor`, salve de tour près de `RecordArrowShot`, client : `UHOLNetUnitDisplayProcessor` rejoue la
    cadence de toute unité qui frappe, `AHOLBuilding::ShowArrowShot`) et `OnOrderSound` (`AHOLPlayerController` : `OrderAt`,
    `AttackMoveAt`, `OrderLineAt`, `RequestPlaceBuilding` / `RequestPlaceWallLine` = Kind `Place`) ; script :
    `HOLGameScreenScript.HandleOrderSound` (journal « Order sound: <Kind> <Type> »), fin de construction dans
    `RefreshMinorAlerts` (`GetLastFinishedBuildingType`) ; tests `HeroesOfLiberty.Audio.OrderSoundEvents`,
    `HeroesOfLiberty.Combat.BuildingShotSound`, `HOLArtSound_Test`, `HOLMinorEvents_Test`.
