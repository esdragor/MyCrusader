# Animations in progress

**Journal du chantier « animations faites par Claude »**, ouvert le 2026-10-16. Tâche récurrente : chaque nouvelle
unité ou activité du jeu demandera des animations ; ce fichier tient la **chaîne de fabrication** (méthode, outils,
règles), le **catalogue** des animations (faites, à faire, à remplacer) et le **journal**. Les étapes de jeu qui en ont
besoin (J5c.1 pour la récolte, plus tard le combat, la construction…) restent menées dans `JourneyInProgress.md` et
renvoient ici pour la fabrication.

Origine : J5c (la vie de la récolte), source des animations tranchée le 2026-10-16 (option A, la recommandation) :
**faites par Claude**, boucles courtes écrites pose par pose en Python dans Unreal sur le mannequin UE4, sur place.
Écartés : Mixamo (compte Adobe et reciblage), pack Fab (achat). Une animation faite ici reste **remplaçable** plus
tard sans toucher au code : la table des types les désigne par leur nom.

**Où travailler (règle de l'utilisateur, 2026-10-16) : dans le worktree `C:\Users\baudo\Documents\MyCrusader-J5c`,
branche `j5c-anims`**, jamais dans la copie principale pour ce chantier : scripts de `Tools/Editor/`, fabrication par
commandlet, séquences de `Content/Units/Mannequin/Animations/`, C++ du branchement, compilation (`Build.bat … 
-Project="C:\Users\baudo\Documents\MyCrusader-J5c\HeroesOfLiberty.uproject"`), tests et jeu d'essai, tous lancés
depuis ce dossier. Commits sur `j5c-anims`, **fusionnés dans `main` après validation de l'utilisateur** ; après une
fusion, remettre `j5c-anims` au niveau de `main` avant la tâche d'animation suivante. Seuls les journaux
(`AnimationsInProgress.md`, `JourneyInProgress.md`) restent dans la copie principale, partagée avec les autres sessions.
Raisons : le jeu d'essai de l'utilisateur ne bloque plus la compilation (binaires séparés), le code à moitié écrit
n'entre pas dans les compilations des autres sessions. Le moteur reste commun : prévenir les autres sessions avant
chaque compilation ou lancement d'Unreal.

Cohabitation : mêmes règles que les autres chantiers (ne compiler ni lancer l'éditeur que si aucune autre session ne
le fait, rien de commité avant validation de l'utilisateur). Fichiers binaires pris par ce chantier, à ne pas
régénérer sur `main` pendant qu'il tourne : table des types d'unités, fournisseurs `TP_Mannequin_Classic*`,
`Content/Units/Mannequin/Animations/`.

---

## Chaîne de fabrication

Repères techniques (repérage du 2026-10-16, lecture seule) :
- **Squelette** : `/Game/CloseCombat/Character/UE4Mannequin/Meshes/UE4_Mannequin_Skeleton` (mannequin UE4, 68 os :
  `root` → `pelvis` → `spine_01..03` → `clavicle` / `upperarm` / `lowerarm` / `hand` `_l` `_r`, `neck_01` → `head` ;
  `pelvis` → `thigh` / `calf` / `foot` / `ball` ; os `ik_*` à ne pas toucher). Maillage du Paysan :
  `/Game/Units/Mannequin/SKM_Mannequin_Classic`, fournisseur `TP_Mannequin_Classic` (partagé avec les soldats), fait par
  `Tools/Editor/make_skinned_mannequin.py` (liste `SEQUENCES` : l'ordre est l'indice). Regarde +Y (`MeshYawOffset` -90).
  **Conventions d'axes (journalisées le 2026-10-16)** : espace composant en cm, **avant = +Y, haut = +Z, droite du
  personnage = -X** (`hand_r` en x = -56,6) ; pose de référence en **A** (bras à ~45° vers le bas, coudes un peu
  pliés). Chaque os court le long de son **X local** : `+X` vers l'enfant pour la colonne, le cou et tout le côté
  gauche des bras, **`-X` pour le côté droit des bras** (squelette en miroir) ; pour les jambes c'est l'inverse
  (`thigh_l` / `calf_l` en `-X`, `thigh_r` / `calf_r` en `+X`) ; `foot` → `ball` le long de `±Y` local. Échelles toutes
  à 1. D'où le choix de ne **jamais écrire d'angles dans les axes locaux** (voir « Description d'un geste »).
- **API Python (5.8)** : `AnimSequenceFactory` (`target_skeleton`) + `AssetTools.create_asset` ; `seq.controller` :
  `open_bracket` → `set_frame_rate` → `set_number_of_frames` → `add_bone_curve` + `set_bone_track_keys(os, pos, rot,
  échelle)` (images + 1 clés) → `close_bracket` → `save_loaded_asset` ; mise à jour en place par
  `remove_all_bone_tracks`. **Aucune piste sur `root`** (reste sur place). Pose de référence et relecture :
  `AnimPoseExtensions` (`get_reference_pose`, `get_bone_pose` / `set_bone_pose` en `LOCAL` / `WORLD`,
  `get_anim_pose_at_time`). `unreal.Rotator(roll, pitch, yaw)` : passer par mots-clés. **Vérifié en 5.8** : le
  contrôleur s'obtient par `seq.get_editor_property("controller")` ; la fabrique initialise déjà le modèle ; les
  parents des os par `AnimationLibrary.find_bone_path_to_root(une séquence du squelette, os)` (le 2e nom est le
  parent ; `AnimPoseExtensions` n'a pas de lecture du parent) ; pose de référence par `get_ref_bone_pose` ; relecture
  `AnimDataEvalType.RAW` et `COMPRESSED` toutes deux possibles en commandlet.
- **Lancement** : commandlet `UnrealEditor-Cmd.exe … -run=pythonscript -script=…` **depuis PowerShell** (Git Bash casse
  `/Game/...`), éditeur fermé ; prévenir les autres sessions avant.
- **Limites de Mass** (`MassVisualizationComponent.cpp:766-768`) : l'animation d'une instance n'est transmise qu'au
  **changement d'indice de séquence** (vitesse de lecture, départ, fondu figés à ce moment) ; un geste non bouclé reste
  sur sa dernière image et ne se rejoue pas sans changer d'indice ; pas de couches ni de masques d'os : **chaque séquence
  est un corps entier** (marche chargée = jambes d'une marche + bras réécrits) ; boucles libres sur le GPU, sans
  événement de fin. Donc : **des boucles**, une vitesse de lecture par type.

Outils (J5c.1a, 2026-10-16), tous dans `Tools/Editor/` :

| Fichier | Rôle | Unreal ? |
|---|---|---|
| `peasant_poses.py` | **les gestes**, clé par clé (dictionnaire `GESTURES`) : le seul fichier à retoucher pour un geste | non |
| `anim_rig.py` | gréement : maths (quaternions `(x, y, z, w)` comme `FQuat`), cinématique directe, IK à deux os des bras et des jambes, interpolation des clés | non |
| `preview_animation.py` | **planche de contact** PNG sans dépendance (zlib + struct) | non |
| `make_peasant_animations.py` | relève le squelette, **fabrique** chaque geste de `GESTURES` et le **relit** | oui (commandlet) |
| `Data/UE4MannequinSkeleton.json` | squelette de référence relevé (nom, parent, transformées locale et composant) | écrit par le précédent |
| `Data/A_SW_Unarmed_Walk.json` | bassin et jambes de la marche source, clé par clé, et sa vitesse de pied d'appui | écrit par le précédent |
| `peasant_props.py` | **les pièces** du Paysan (J5c.2) : hache, pioche, panier, fagot, hottes ; primitives, os, repère, triangles ; partagé par l'aperçu et le futur maillage `SKM_Peasant` | non |
| `Data/A_SW_Idle.json` | bassin et jambes du repos source (repos chargés) | écrit par `make_peasant_animations.py` |
| `make_peasant_mesh.py` | **maillage du Paysan** (J5c.2c-e) : `M_UnitPlayerColorProps`, `SKM_Peasant` (corps de `SKM_Mannequin_Classic` + pièces de `peasant_props.mesh_parts`, LOD par LOD), `TP_Peasant` ; relecture des pièces | oui (commandlet) |
| `Data/SKM_Mannequin_Body.json` | les 20 719 sommets du maillage du Paysan (LOD 0, pose de liaison) et l'os qui pèse le plus sur chacun : corps gris de l'aperçu, contrôle des pièces contre le corps | écrit par `make_peasant_animations.py --dump-only` |

**Description d'un geste** (`peasant_poses.py`, détail en tête de `anim_rig.py`) : tout en **termes du corps**,
jamais en axes locaux d'os. Positions et directions en `(droite, avant, haut)` cm ; rotations en `(torsion, flexion,
côté)` degrés (torsion vers sa droite, penché en avant, penché vers sa droite), appliquées à l'os **dans les axes de la
pose de référence** et emportées par les os parents (une clé sur `spine_03` s'ajoute à celle de `spine_01`). Le geste :
`name`, `fps`, `stance` (décalage constant des pieds), `elbows` (direction des coudes), `fist` (doigts fermés : angles
des phalanges 01-02-03), `keys`. Une clé : `t` (s), `ease` du segment qui y arrive (`linear`, `in` accélère, `out`
freine, `inout`), `pelvis` (décalage du bassin), `rot` ({os: angles} ; aide `torso(torsion, flexion, côté)` qui répartit
du bassin à `spine_03` et retourne la tête vers le travail), `axe` (`grip` = main gauche sur le manche, `dir` = du
pommeau vers le fer, `spacing` = écart de la main droite), `elbows`. Interpolation linéaire (sphérique pour `dir`)
après l'easing ; **dernière clé = première** (boucle). Les **bras** sont résolus par IK à chaque image (les deux mains
restent sur le manche, paume en travers, doigts à l'opposé de l'épaule) ; les **jambes** aussi, pieds **cloués** à
leur place quel que soit le bassin (pas de glissement). Os écrits : bassin, colonne, cou, tête, clavicules, bras,
mains, jambes, pieds, doigts (44 pistes) ; **jamais `root` ni `ik_*`**, `ball` et pouces laissés en référence.

Ajouts du 2026-10-16 (pioche, cueillette, marche chargée) : `tool` remplace `axe` (accepté encore) et porte au niveau
du geste `kind` (`axe` ou `pick`, pour l'aperçu) et les tailles du manche ; **mains libres** `hands` (`at` position,
`palm` direction de la paume, `fingers` facultatif) au lieu d'un outil ; **geste sur une base** : `base` = chemin d'une
séquence source (la marche) dont le bassin et les jambes sont **copiés image par image** (relevés par le commandlet
dans `Data/<nom de la source>.json`), qui fixe la durée et la cadence (les temps des clés sont étirés sur sa durée,
jambes non résolues) ; `steady_chest` (0-1, part du balancement du bassin reprise au buste), `hands_frame: "chest"`
(les mains suivent le buste), `scene` (aperçu : tronc, rocher, buisson et bourse, charge). Aide `torso(…, nod=,
pelvis=)` : tête penchée vers le bas, bassin laissé à la base. **Clavicules** : une rotation « côté » vers sa droite
**abaisse** l'épaule droite et une « torsion » vers sa droite la **recule** (mesuré) : les constantes de la hache
s'appellent désormais `SHOULDERS_DOWN` / `SHOULDERS_BACK` (ce qu'elles font, valeurs inchangées : la hache validée
ne bouge pas, vérifié à 1e-15 près) ; `SHOULDERS_UP` / `SHOULDERS_FORWARD` pour les gestes suivants. Puis (marches
chargées bois, pierre) : `swing` sur une main libre (part du trajet du pied du même côté que la main fait en sens
inverse : bras qui balance avec la marche) ; `prop` (aperçu seulement : fagot `logs` ou sac `sack`, posé dans la pose
de référence et porté par le buste) ; `RENAMES` dans le script de fabrication (renommage en place, avant la
fabrication, par `EditorAssetLibrary.rename_asset`) ; aperçu `--compare g1,g2,g3 [--at 0.25]` (gestes côte à côte au
même point de leur boucle).

Ajouts J5c.2 (2026-10-16, outils et charges visibles) :
- **Pièces** (`peasant_props.py`, pur Python) : chaque pièce = quelques primitives (boîte, tronc de cône à section
  elliptique, prisme), **liée à 100 % à un seul os**, un id par état visible (0 corps seul, 1 hache, 2 pioche, 3 panier
  de baies, 4 fagot, 5 hotte de pierre, 6 hotte d'or). Repère de pièce droit (main droite) : pour une charge, origine à
  son « ancre » (termes du corps, pose de référence), X avant, Y droite, Z haut (convention d'acteur d'Unreal) ; pour un
  outil, origine au **pommeau**, X le long du manche vers le fer, Y vers le **tranchant** (fer de hache, pointe de
  pioche), Z = X × Y. `bone_local()` donne le repère dans l'os (cm, quaternion `FQuat`) ; `bind_mesh()` donne ce que le
  futur maillage doit contenir (sommets en espace composant dans la pose de liaison, triangles, une couleur sRGB par
  triangle, os) : **le constructeur de `SKM_Peasant` (J5c.2c) prend ses triangles là**, l'aperçu dessine les mêmes.
  `python Tools/Editor/peasant_props.py` imprime la table.
- **Os porte-outil `ik_hand_gun`** : seule exception à la règle « aucune piste `ik_*` ». Os non animé du mannequin, enfant
  de `ik_hand_root` (à l'origine, jamais clé) ; les gestes à outil (`has_tool` : hache, pioche) y écrivent à chaque
  image le **repère de l'outil** (`Rig.tool_frame`) : la hache et la pioche liées à cet os suivent **exactement les deux
  mains** (la main droite glisse le long du manche). Le tranchant est fixe dans la main droite : direction du fer juste
  avant la frappe (`Rig.strike()`, l'image où il s'arrête le plus net), comme le dessinait l'aperçu. Corps des gestes
  inchangé (le calcul du corps est séparé : `_solve`). `ik_hand_l` / `ik_hand_r` (enfants) suivent l'os ; personne ne
  les lit. Le script de fabrication refuse toute autre piste `ik_*` et vérifie la présence de celle-ci.
- **`steady_spine`** (0-1) : reprise du mouvement du bassin répartie sur `spine_01..03` (un tiers, deux tiers, tout) :
  pour une base dont le bassin tourne beaucoup (repos d'escrime, ~50° vers sa droite), le buste et la charge regardent
  devant sans pli à une seule articulation.
- **Aperçu** : pièces **pleines**, ombrées, sur leurs os, avec tampon de profondeur ; **corps gris** = sommets du maillage
  portés chacun par son os principal (cache les pièces derrière lui, ne cache pas la silhouette) ; la ligne de sortie
  donne en plus, pour un outil, la **distance des mains à l'axe du manche porté par `ik_hand_gun`**, et pour chaque
  pièce les **points du corps dedans** (mains et doigts exclus : elles tiennent les pièces). `--all` : planches
  `<nom>_contact_sheet.png` (12 images) et `<nom>_zoom.png` (6 images, zoom 2) de chaque geste, comparaisons
  `A_Peasant_Carry_comparaison.png` et `A_Peasant_IdleCarry_comparaison.png` (~3 min). Clé de geste `props` = pièces
  dessinées (l'ancienne clé `prop` et la boîte `scene: load` ont disparu).
- **Fabrication** : avant de refaire une séquence existante, le script la relit (os du corps, brut) et journalise de
  combien le corps a bougé (jambes et bassin / colonne et tête / bras et mains) ; relecture de l'os porte-outil
  (repère hors ligne contre Unreal, mains à l'axe du manche). `--dump-only` relève aussi le corps.

1. **Poses clés** : écrire ou retoucher le geste dans `peasant_poses.py`.
2. **Aperçu** (sans Unreal, ~1 s) : `python Tools/Editor/preview_animation.py [geste] [--frames 12] [--zoom 1.6]
   [--out <png>]` (défaut `Saved/AnimPreview/<geste>_contact_sheet.png`). Trois rangées : de face (sa droite à gauche
   de l'image), de profil (avant à droite), vue RTS (de haut à 55°, avant-droite) ; rouge = membres droits, bleu =
   gauches, marron = manche (ou la charge portée), noir = fer de hache ou tête de pioche (tranchant fixe dans la main
   droite, orienté pour mener le coup à la frappe) ; points gris = trajet du fer (ou de la main droite) sur la boucle ;
   vert = la scène : tronc placé où est le fer à la frappe (l'image où il s'arrête le plus net, marquée d'un tiret),
   rocher sous la pointe de la pioche, buisson et bourse. La ligne de sortie donne la frappe et **l'écart des mains à
   leur cible** (bras trop courts si > 0).
3. **Fabrication** (commandlet, ~1 min, éditeur fermé, autres sessions prévenues) :
   ```powershell
   & "C:\UE\UE_5.8_AS_3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
     "C:\Users\baudo\Documents\MyCrusader\HeroesOfLiberty.uproject" -run=pythonscript `
     -script="C:\Users\baudo\Documents\MyCrusader\Tools\Editor\make_peasant_animations.py" -unattended -nop4 -nosplash `
     -abslog="<dossier temporaire>\anim.log"
   # Lignes "HOLAnim" du journal : squelette relevé, axes, fabrication, relecture.
   ```
   Le script relève le squelette (JSON), puis crée `/Game/Units/Mannequin/Animations/<name>` ou le **refait en place**
   (`remove_all_bone_tracks`, jamais supprimé), 30 i/s, sans mouvement de racine, puis le relit (brut et compressé) :
   écart avec la cinématique hors ligne, racine à l'origine, première = dernière image, durée. `-abslog` évite
   d'écrire dans le journal des autres sessions. `--dump-only` dans la valeur de `-script` : squelette seul.
4. **Branchement** : séquence ajoutée au fournisseur d'animations du maillage et nommée dans la table des types.
5. **Vérification** : planche de contact, puis en jeu ; l'utilisateur juge le geste.
6. **Retouches** : ajuster les poses clés d'après son regard, refaire l'aperçu, refabriquer.

Règles :
- Lisible à la distance de la caméra du RTS avant tout : gestes amples, silhouette claire, peu de détails fins.
- Désynchronisation par départ décalé au hasard (comme D234).

## Catalogue

| Animation | Unité | Utilisée par | État |
|---|---|---|---|
| Coup de hache (bois) | Paysan | J5c.1 | **fabriquée** (`A_Peasant_ChopWood`, 1,2 s, J5c.1a) ; **validée sur l'aperçu** par l'utilisateur (2026-10-16) ; **branchée** (indice 2, J5c.1b-d, vitesse 1,0)  ; **validée en jeu** (essai du 2026-10-16), commitée a9e4269 ; **J5c.2b** : piste `ik_hand_gun` (repère de la hache), corps identique (0,000 cm), refabriquée ; fusionnée dans `main` (fad246d) |
| Coup de pioche (pierre, or) | Paysan | J5c.1 | **fabriquée** (`A_Peasant_MinePick`, 1,333 s, une séquence pour les deux) ; **branchée** (indice 3, vitesse 1,0)  ; **validée en jeu** (essai du 2026-10-16), commitée a9e4269 ; **J5c.2b** : piste `ik_hand_gun` (repère de la pioche), corps identique (0,000 cm), refabriquée ; fusionnée dans `main` (fad246d) |
| Cueillette (baies) | Paysan | J5c.1 | **fabriquée** (`A_Peasant_GatherBerries`, 1,8 s) ; **branchée** (indice 4, vitesse 1,0)  ; **validée en jeu** (essai du 2026-10-16), commitée a9e4269 |
| Marche chargée, nourriture | Paysan | J5c.1 / J5c.2 | **fabriquée** (`A_Peasant_CarryFood`, renommée depuis `A_Peasant_CarryWalk` ; 1,0 s, jambes de `A_SW_Unarmed_Walk`, charge contre le ventre) ; vitesse de lecture **3,57** (règle du projet) ; **validée sur l'aperçu pour la nourriture** (2026-10-16) ; **branchée** (indice 5)  ; **validée en jeu** (essai du 2026-10-16), commitée a9e4269 |
| Marche chargée, bois | Paysan | J5c.1 / J5c.2 | **fabriquée** (`A_Peasant_CarryWood`, mêmes jambes et cadence, fagot de 4 bûches sur l'épaule droite) ; vitesse **3,57** ; **branchée** (indice 6)  ; **validée en jeu** (essai du 2026-10-16), commitée a9e4269 ; **J5c.2a** : fagot réel (bûches de 4,5 cm) monté de 9 cm et décalé de 4 cm vers sa droite (il entrait de 4,5 cm dans la tête et l'épaule), main droite suivie (12, 14, 162) → (17, 14, 167,5) ; validée sur l'aperçu et en jeu, fusionnée dans `main` (fad246d) |
| Marche chargée, pierre et or | Paysan | J5c.1 / J5c.2 | **fabriquée** (`A_Peasant_CarryStone`, mêmes jambes et cadence, sac sur le dos, buste penché en avant) ; vitesse **3,57** ; **branchée** (indice 7)  ; **validée en jeu** (essai du 2026-10-16), commitée a9e4269 ; **J5c.2a retouchée** : hotte sur le dos, **mains sur les bretelles** devant les épaules (jambes, penché et rebond identiques à 0,000 cm) ; validée sur l'aperçu et en jeu, fusionnée dans `main` (fad246d) |
| Repos chargé, nourriture | Paysan | J5c.2 | **fabriquée** (`A_Peasant_IdleCarryFood`, 11,333 s, 340 images, bassin et jambes de `A_SW_Idle`, panier contre le ventre, bras de la marche chargée, trois respirations) ; validée sur l'aperçu (2026-10-16) ; **branchée** (indice 8 de `TP_Peasant`, J5c.2c-d) ; validée en jeu, fusionnée dans `main` (fad246d) |
| Repos chargé, bois | Paysan | J5c.2 | **fabriquée** (`A_Peasant_IdleCarryWood`, mêmes base et durée, main droite qui tient le fagot, bras gauche pendant) ; validée sur l'aperçu ; **branchée** (indice 9) ; validée en jeu, fusionnée dans `main` (fad246d) |
| Repos chargé, pierre et or | Paysan | J5c.2 | **fabriquée** (`A_Peasant_IdleCarryStone`, mêmes base et durée, mains aux bretelles, penché en avant de ~10° de plus que debout) ; validée sur l'aperçu ; **branchée** (indice 10) ; validée en jeu, fusionnée dans `main` (fad246d) ; l'or prend celle-ci (avec la hotte d'or) |
| Pièces visibles : hache, pioche, panier de baies, fagot, hottes de pierre et d'or | Paysan | J5c.2 | **dans le maillage** `SKM_Peasant` (J5c.2c-e) ; vues en jeu (captures `props_*.png`, planche `props_sheet.png`) ; essai de l'utilisateur validé, fusionnées dans `main` (fad246d) ; A/B S7 : GPU +1,06 ms (D323) ; piste : pièces allégées aux LOD 2-3 |
| Pose au dépôt | Paysan | — | **abandonnée** (2026-10-16 : pas de geste, comme AoE4 ; le serveur ne s'arrête pas au dépôt) |
| Repos, marche | toutes | M2 (D234) | empruntées au pack `CloseCombat` (épéiste) |
| Bâtisseur (coup de marteau) + marteau | Paysan | J6a.4 | **fabriqué, branché, validé en jeu** (`A_Peasant_Build`, 1,333 s, deux coups ; marteau pièce 7 sur `ik_hand_gun` ; séquence 11 de `TP_Peasant` ; bâtisseur tourné vers son chantier) ; fusionné dans `main` (3b750bc) |

## Prochaine étape exacte

**J5c close** (2026-10-16 : D318 à D323 au § 21 du GDD, paragraphe « Parcours J5c » au § 16.6) ; J5c.1, J5c.2 et le
bâtisseur de J6a.4 sont dans `main`.

**1. Fait (2026-10-17, mycrusader-32)** : `main` avancé à 7595c81 avec l'accord de mycrusader-a5 (partie « anims » de
`quick_check`, `hol.Units.AnimReport`, `stress_ab.ps1`, fusions de J6b.1 et J6b.2) ; `j5c-anims` = `main`.

**2. Pièces allégées au LOD 3 : faites et mesurées (GPU S7 −0,49 ms), non commitées ; reste l'essai de l'utilisateur** (jeu lancé depuis le worktree J5c, zoom maximal sur des Paysans chargés), puis commit sur `j5c-anims` (scripts + `SKM_Peasant.uasset`), avance rapide de `main` avec l'accord des sessions actives, D-numéro au GDD (précision de D323). Détail au journal du 2026-10-17. Passation d'origine :
- But : récupérer une partie du **+1,2 ms de GPU** de S7 dû aux pièces (3,34 ms au repos, 2,15 avant J5c ; A/B : passe
  de base +0,70 ms, passe de vitesses +0,42 ms ; les sommets des pièces passent le shader de sommets même repliés par
  WPO). Le fil de jeu et les appels de dessin ne bougent pas.
- Où : worktree `C:\Users\baudo\Documents\MyCrusader-J5c`, branche `j5c-anims` (d'abord remise au niveau de `main` si le
  point 1 est fait). Le maillage est fait LOD par LOD par `Tools/Editor/make_peasant_mesh.py` (corps du même LOD de
  `SKM_Mannequin_Classic` + toutes les pièces de `peasant_props.py` en pleine définition ; poids des LOD 1-3 transférés du
  LOD 0). Pièces : +1 894 sommets / +940 triangles à chaque LOD, soit +7,5 % au LOD 0 et +81 % au LOD 3.
- Pistes : une version simplifiée de chaque pièce pour les LOD 2-3 (boîtes au lieu de cylindres, moins de bûches, hotte
  en une boîte), outils retirés au LOD 3 (les charges restent) ; mesurer d'abord la répartition des Paysans par LOD dans
  S7 pour savoir quel LOD compte.
- Vérifier : relecture de chaque LOD par le script, `quick_check` (partie « anims »), captures, puis **A/B S7** avec
  `Tools/Tests/stress_ab.ps1` (côté A : un worktree détaché au commit de base, compilé, ou `MyCrusader-A` s'il est libre,
  règle « Cohabitation » de `JourneyInProgress.md` ; côté B : le worktree J5c ; A B A B A B, annoncés aux autres
  sessions ; trace Insights `-trace=cpu,gpu` si besoin). Puis essai de l'utilisateur (aspect de loin), commit, fusion.

Prochaines animations : à la demande des étapes de jeu (combat de M5, autres unités), même chaîne, même worktree.

## Journal

### 2026-10-16 — Ouverture

- Chantier ouvert à la demande de l'utilisateur (les animations seront une tâche récurrente). Source tranchée : faites
  par Claude (option A).
- Repérage (sous-agent, lecture seule) : squelette, API Python, table des types, choix chez le client, limites de Mass
  (voir « Repères techniques »). Aucun script du projet ne crée encore d'`AnimSequence`. Le serveur décharge et renvoie
  le Paysan au pas suivant : pas de temps pour un geste de pose au dépôt sans changer le jeu. Personne ne tourne le
  Paysan vers sa source (il frapperait dans le vide). Pas de commande de zoom pour les captures (`hol.Camera.Zoom` à
  ajouter). Pas de numpy / matplotlib dans le Python local : aperçus à faire sans dépendance.

### 2026-10-16 — J5c.1a : chaîne de fabrication et coup de hache

- Outils écrits (voir « Chaîne de fabrication ») : gestes en termes du corps, **IK des bras sur un manche commun** et
  des jambes à pieds cloués, aperçu PNG hors ligne, fabrication et relecture par commandlet. Deux lancements d'Unreal
  en tout (le premier a relevé le squelette et validé l'API du premier coup, le second a fabriqué le geste final).
- Squelette relevé : 68 os, échelles à 1 ; la cinématique hors ligne redonne la pose de référence d'Unreal à
  **0,0001 cm** près. Conventions d'axes : voir « Repères techniques ».
- **`A_Peasant_ChopWood`** (`/Game/Units/Mannequin/Animations/`) : boucle de **1,2 s**, 36 images à 30 i/s, 44 pistes
  (aucune sur `root` ni `ik_*`). Armé au-dessus de l'épaule droite (torse tourné de 45° à droite, penché en arrière,
  fer derrière l'épaule) ; descente **en 0,2 s** en diagonale (accélère, fer écarté à droite à mi-course, arrêt net) ;
  **frappe à 0,20 s** à hauteur de taille, fer à 110 cm devant lui et 116 cm de haut, main droite glissée vers la
  gauche sur le manche, torse penché en avant ; rebond court ; remontée lente (0,32 → 1,2 s) devant la poitrine puis
  par-dessus l'épaule. Pieds écartés (gauche en avant), genoux un peu fléchis, pieds immobiles ; mains fermées.
- **Relecture dans Unreal** : écart max cinématique hors ligne / Unreal **0,001 cm** (brut) et **0,006 cm**
  (compressé) sur bassin, tête, mains, avant-bras, mollets, pieds et deux bouts de doigts, à chacune des 37 clés ;
  racine à **0,000 cm** de l'origine ; première et dernière image identiques (**0,000 cm**) ; durée 1,2000 s.
  L'aperçu montre donc exactement ce qui est fabriqué. Mains sur le manche à 0,0 cm sur toute la boucle.
- Itérations sur l'aperçu : la main gauche n'atteignait pas le manche en haut de l'armé (orientation de la main
  instable quand l'avant-bras longe le manche) : orientation désormais fixée par la direction épaule → prise, avant
  l'IK ; descente rendue diagonale (fer écarté à droite à mi-course) pour une silhouette lisible de haut.
- Pas encore vu dans Unreal en image (pas de capture en jeu, pas de branchement) ; la hache n'existe pas dans le jeu
  (geste seul, à vide).
- **Coup de hache validé sur l'aperçu** par l'utilisateur (« ça a l'air bon pour le coup de hache »).

### 2026-10-16 — Pioche, cueillette, marche chargée

- Chaîne étendue (outil `pick`, mains libres, geste sur une base, buste stable : voir « Chaîne de fabrication ») ; la
  hache validée ne change pas (poses identiques à 1e-15, même planche au pixel près avant le changement de dessin du
  fer). Une seule fabrication pour les trois (avec la hache refaite à l'identique) : un lancement d'Unreal.
- **`A_Peasant_MinePick`** (pierre et or) : **1,333 s**, 40 images, 44 pistes. Armé droit au-dessus de la tête
  (tête de pioche derrière la tête, bras levés), descente en 0,27 s, **frappe à 0,267 s** : torse plié (~46°),
  genoux fléchis, bassin descendu de 15 cm, tête de pioche à 86 cm devant et 38 cm de haut, pointe à 66 cm devant et
  **23 cm du sol** (rocher bas) ; rebond, remontée lente (0,4 → 1,333 s). Rythme plus lent et plus vertical que la
  hache.
- **`A_Peasant_GatherBerries`** : **1,8 s**, 54 images, 44 pistes. Accroupi et penché (bassin -14 cm, torse ~45°),
  mains à tour de rôle : la droite cueille (deux endroits du buisson) puis porte à la bourse à la hanche gauche et
  tient une branche, la gauche cueille puis revient à la bourse. Les cueillettes basses manquent leur cible de
  **2,9 cm** au plus (bras tendu au bout ; invisible, laissé tel quel). Geste plus discret que les coups : à juger de
  la caméra.
- **`A_Peasant_CarryWalk`** : jambes et bassin **copiés** de `A_SW_Unarmed_Walk` (sur place : racine 0, bassin 0 entre
  première et dernière image, boucle de la source fermée, **1,0 s**, 30 images), bras réécrits (charge tenue contre le
  ventre vers 110 cm, buste penché en arrière de 8°, 60 % du balancement du bassin repris), 46 pistes (avec `ball`).
  **Pieds identiques à la source à 0,001 cm** (aucun glissement ajouté).
- **Relecture** : écart cinématique hors ligne / Unreal **0,031 cm** (pioche), **0,005 cm** (cueillette, marche
  chargée ; compressé), racine à 0,000 cm, première = dernière image (0,000 cm), durées 1,3333 / 1,8 / 1,0 s.
- **Vitesse de lecture de la marche chargée** : pied d'appui **168,2 cm/s** (`A_SW_Unarmed_Walk`) contre **246,2 cm/s**
  (`A_SW_Walk_F`, comme mesuré pour D234). Règle du projet (`WalkPlayRate` = vitesse / pied d'appui, sans l'échelle du
  maillage : 600 / 246 = 2,44 pour le Paysan) : **3,57** pour la marche chargée. Avec l'échelle 1,5 du maillage (ce
  que demandait la consigne) on aurait 1,62 pour la marche et 2,38 pour la marche chargée : **la règle actuelle ignore
  `MeshScale`** (foulée 1,5 fois trop rapide à l'œil si l'échelle compte) — à trancher avec l'utilisateur au
  branchement ; le script affiche désormais les deux valeurs (affichage pas encore relancé).
- Avis de l'utilisateur sur les aperçus : la marche chargée « est bien pour la nourriture » ; il en veut **une pour le bois
  et une pour la pierre et l'or**. Le client connaît déjà la ressource portée (`LoadResource`, 2 bits envoyés pour un
  porteur) : aucun bit en plus. Pioche et cueillette : pas d'objection.

### 2026-10-16 — Marches chargées bois et pierre/or

- **`A_Peasant_CarryWalk` renommée `A_Peasant_CarryFood`** (geste `carry_food`) par `EditorAssetLibrary.rename_asset`
  dans le script (aucun référenceur avant, ancien chemin absent après, pas de redirecteur) ; poses **identiques** (écart
  0,0 avec celles d'avant) ; planches refaites sous le nouveau nom, anciennes effacées.
- **`A_Peasant_CarryWood`** : mêmes jambes, bassin et cadence (1,0 s, 30 images, 46 pistes). Fagot de 4 bûches de 95 cm
  sur l'épaule droite, l'avant un peu relevé, tenu par la main droite levée à côté du visage ; bras gauche qui
  **balance avec la marche** (vers l'avant quand le pied gauche est en arrière, ±14 cm) ; corps penché vers la gauche
  (3,7 à 5,3°) contre la charge, buste un peu en arrière.
- **`A_Peasant_CarryStone`** (pierre et or) : sac sur le dos, son col tenu des deux mains devant l'épaule droite ;
  buste penché **en avant de 15 à 23°** de plus que debout (10,6 à 18,2° du bassin au cou, contre -4,6° au repos),
  tête relevée ; le buste plonge un peu plus à chaque pas (clés à 0,37 et 0,9 de la boucle, quand le bassin de la marche
  est au plus bas). Les jambes étant partagées, le poids ne se lit que par le buste.
- Silhouettes de la vue RTS bien distinctes (`Saved/AnimPreview/A_Peasant_Carry_comparaison.png`, au quart de la
  boucle) : charge au ventre et buste droit (nourriture), long fagot au-dessus de l'épaule (bois), sac sur le dos et
  buste penché (pierre/or).
- **Relecture** (un lancement d'Unreal pour les six gestes) : écart cinématique hors ligne / Unreal **0,000 cm** brut,
  **0,005 / 0,006 cm** compressé (bois / pierre) ; racine 0,000 cm ; première = dernière image ; durée 1,0 s ;
  **pieds identiques à `A_SW_Unarmed_Walk` à 0,001 cm** pour les trois marches chargées ; pied d'appui 168,2 cm/s,
  donc **une seule vitesse de lecture pour les trois : 3,57** (règle du projet ; 2,38 avec `MeshScale`), affichage
  des deux valeurs vérifié dans ce lancement. Hache, pioche et cueillette refaites à l'identique (mêmes écarts
  qu'avant).

### 2026-10-16 — Worktree pour le branchement

- Marches chargées bois et pierre-or montrées à l'utilisateur. À sa demande, le travail continue dans le **worktree**
  `MyCrusader-J5c` (branche `j5c-anims`, depuis cd39c6c ; voir « Où travailler » en tête). Les six séquences et les
  outils (`anim_rig.py`, `peasant_poses.py`, `preview_animation.py`, `make_peasant_animations.py`, `Data/`) y sont
  copiés : **les refabriquer là-bas désormais** ; leurs copies non suivies dans la copie principale sont à effacer à la
  fusion. Première compilation du module dans le worktree lancée.

### 2026-10-16 — J5c.1 b, c, d : branchement (worktree)

- **Fournisseur** : `make_skinned_mannequin.py` liste les huit séquences (0 repos, 1 marche, 2 hache, 3 pioche, 4
  cueillette, 5 / 6 / 7 marches chargées nourriture / bois / pierre-or ; ajouts **à la fin seulement**) et gagne
  `--providers-only` (fournisseurs refaits en place, maillages et matériau intouchés). Les deux fournisseurs
  `TP_Mannequin_Classic` / `_Nanite` refaits par commandlet : 8 séquences, aucune avec mouvement de racine. Les soldats
  gardent repos 0 / marche 1.
- **Table des types** (`UnitTypes.csv`, `FHOLUnitTypeRow`, `FHOLUnitTypeConstants`) : colonnes `GatherSequences` et
  `CarryWalkSequences` (structure `FHOLResourceSequences`, `(Food=4,Wood=2,Stone=3,Gold=3)` / `(Food=5,Wood=6,
  Stone=7,Gold=7)` pour le Paysan, -1 partout pour les soldats), `GatherPlayRate` 1,0, `CarryWalkPlayRate` **3,57**.
  Indices hors du fournisseur rejetés (`ensure`, repos / marche à la place) ; longueurs de **toutes** les séquences
  lues par la fabrique (`SequenceLengths`, départ décalé au hasard) au lieu de `IdleLength` / `WalkLength`.
- **Vitesse des marches : la règle tient déjà compte de `MeshScale`** : le client joue `WalkPlayRate / MeshScale`
  (code de D234, `HOLInterpolatedInstancesProcessor`). La table garde la règle « vitesse / pied d'appui » (2,44 ; 3,57)
  et **l'écran joue 1,63 pour la marche et 2,38 pour la marche chargée**. La note du 2026-10-16 (« la règle ignore
  `MeshScale` ») était fausse : rien à trancher sur ce point, seulement à juger à l'œil. Les gestes ne sont pas divisés
  par l'échelle.
- **Ressource connue du client** (option A) : `FHOLUnitStateFragment::GatherResource` à côté de l'activité (écrite
  par le serveur avec l'activité : ressource de la source) ; sur le fil, les 3 bits d'activité valent Idle 0,
  Moving 1, GatherTrip 3 et « récolte + ressource » **4 à 7** (`HOLUnitReplication::EncodeActivity` /
  `DecodeActivity`, au `Quantize` / `Dequantize`) : **0 bit de plus** (60 bits, 62 pour un porteur, inchangés). La
  valeur 2 n'est jamais écrite (une seule valeur libre pour une 5e activité). Une autre ressource récoltée compte comme
  un changement d'activité (envoi immédiat). `hol.Units.Where` ajoute « , gathers Wood » en fin de ligne (les motifs
  de `quick_check` sur « Gathering » inchangés) ; nouvelle vérification `quick_check` : chaque client voit l'autre
  Paysan « Gathering, gathers Wood », avec et sans charge.
- **Choix chez le client** : fonction pure `HOLUnitAnimation::Choose` (`Presentation/HOLUnitAnimation.h`) : récolte
  → geste de la ressource, quelle que soit la vitesse ; en marche avec une charge (toute activité) → marche chargée
  de la charge ; sinon repos / marche comme avant, hystérésis étendue à toutes les marches. Un seul chemin pour le
  solo (simulation : `FHOLGatherFragment`) et le client (état reçu) (D272). **Paysan tourné vers sa source** à
  l'affichage seulement, d'un coup (D304), au début du geste : en solo sa source, chez le client la source debout la
  plus proche de la ressource récoltée (`FindNearestStanding` avec filtre de ressource, une fois par geste).
- `hol.Camera.Zoom <pas>` ajouté (molette simulée ; hauteur mini de la caméra 15 m).
- **Vérifications** : compilation du premier coup ; tests **41/41** (nouveaux `Animation.Choice`, `Animation.Table`
  — la table et le fournisseur désignent bien chaque séquence par son nom —, `Net.Reduction` / `ElementFormat` /
  `SendRule` étendus) ; `quick_check` **PASS** (118 s puis 105 s avec la nouvelle vérification). Captures en jeu
  (solo, 15 m, `anim_*.png` dans `Saved_Tests` du worktree) : coups de pioche et cueillette visibles (poses qui
  changent d'une image à l'autre), marche chargée pierre (sac sur le dos, buste penché) et bois (main levée à
  l'épaule) reconnaissables ; à 15 m les Paysans restent petits, le jugement des gestes revient à l'essai.

### 2026-10-16 — Essai en jeu validé, commit

- Session d'essai lancée depuis le worktree (deuxième écran, `Plain`) : gestes de récolte, marches chargées et
  orientation vers la source **validés par l'utilisateur** (« c'est bon »). Commit **a9e4269** sur `j5c-anims`
  (séquences, outils, branchement). Reste : S1 / S2 / S7 courts, fusion dans `main`, S5 court ; puis J5c.2 (hache,
  pioche, fagot, sac, panier visibles).

### 2026-10-16 — Fusion de main dans j5c-anims, vérifications

- `main` (4c1d6e0, J6a.1 / J6a.2) fusionné dans `j5c-anims` (a9e4269) sans conflit ; aucune correction de code.
- **Compilation** (worktree) : réussie en 45 s (9 actions).
- **`quick_check`** : 1er passage **FAIL** en 97 s, tests 47/47, seule la vérification réseau « client 1 : its wood
  rose at the deposit, the other's stock never received » en échec (`others []` au lieu de `others [slot 0 …]`) :
  course de fin, le client 2 s'était déjà déconnecté (12.38.39.012, fermé côté serveur à 39.043) avant le dernier
  rapport du client 1 (39.164) ; rien à voir avec la fusion. 2e passage **PASS en 93 s**, tests **47/47**. Fragilité
  à corriger un jour dans `quick_check` (attendre les deux rapports avant les sorties).
- **Stress tests courts** hors écran, machine chargée (CPU 46-66 % avant chaque lancement, deux éditeurs
  ProjectShiva ouverts) — 1er / 2e passage :
  - S1 10 s : 2,753 PASS / 3,309 FAIL ms par pas ; rendu 161 / 139 fps.
  - S2 10 s : 3,244 FAIL / 3,823 FAIL ms ; rendu 169 / 147 fps.
  - S7 20 s : 3,632 FAIL / 3,251 FAIL ms ; rendu **106 / 132 fps (FAIL, seuil 144)**, fil de jeu 8,15 / 7,45 ms ;
    récolte 1 195 / 1 196 bois/s, 0 corps sorti d'une emprise.
  - S1 (fantassins immobiles, que J5c.1 ne touche pas) varie de 20 % d'un passage à l'autre : la charge domine,
    rien de concluant. Les S7 chargés de `main` du 2026-10-15 (`ab_S7_*`) donnaient 3,19-3,58 ms mais 154-169 fps :
    le rendu de S7 sous 144 fps est **à surveiller** (coût possible des animations au fil de jeu). **S1 / S2 / S7 à
    refaire au repos** avant de fusionner dans `main`.

### 2026-10-16 — A/B S7 main / j5c-anims

- A = `main` (4c1d6e0, sans J5c.1), B = worktree `j5c-anims` (avec J5c.1) ; même moteur AS_3, S7 20 s hors écran,
  alterné A B A B A B, un processus à la fois ; machine chargée (Discord, navigateurs, éditeurs ProjectShiva).
  Journaux dans le scratchpad de la session (`ab_logs/ab1_A` … `ab6_B`).

| # | Côté | CPU avant (%) | ms/pas (max) | fps (1 % bas) | Jeu / rendu / GPU (ms) | Draws | Bois/s |
|---|------|---------------|--------------|---------------|------------------------|-------|--------|
| 1 | A | 84 / 88 / 83 | 3,057 (13,1) | 148,0 (59,9) | 6,53 / 3,42 / 2,69 | 267 | 1 195 |
| 2 | B | 67 / 60 / 71 | 2,991 (13,1) | 153,7 (63,4) | 6,38 / 2,74 / 2,20 | 267 | 1 206 |
| 3 | A | 51 / 73 / 75 | 3,102 (13,8) | 149,5 (55,9) | 6,57 / 2,83 / 2,24 | 267 | 1 195 |
| 4 | B | 31 / 72 / 76 | 2,929 (12,4) | 158,7 (65,9) | 6,18 / 2,75 / 2,34 | 267 | 1 198 |
| 5 | A | 50 / 45 / 59 | 3,502 (15,8) | 119,2 (43,3) | 8,21 / 3,36 / 2,59 | 267 | 1 198 |
| 6 | B | 86 / 90 / 75 | 3,770 (18,9) | 89,7 (20,7) | 10,76 / 4,13 / 2,48 | 267 | 1 195 |
| moy. | A | | 3,220 | 138,9 (53,0) | 7,10 / 3,20 / 2,51 | 267 | |
| moy. | B | | 3,230 | 134,0 (50,0) | 7,77 / 3,21 / 2,34 | 267 | |

- Moyennes tirées par la paire 5-6 (B lancé à 75-90 % de CPU) ; sur les paires 1-2 et 3-4, B fait **mieux** que A
  (2,96 contre 3,08 ms, 156 contre 149 fps, fil de jeu 6,28 contre 6,55 ms). Le bruit de charge (±2 ms au fil de
  jeu) couvre tout écart : **pas de coût mesurable à l'A/B**. Les 106 / 132 fps de B du matin venaient de la charge.
- **Trace Insights** (`-trace=cpu`, un A et un B, CPU 95-99 % avant les deux ; S7 identiques : 3,52 / 3,40 ms,
  123 / 124 fps, fil de jeu 7,97 / 7,92 ms), statistiques du fil de jeu sur toute la trace (~5 590 images) :
  `HOLInterpolatedInstancesProcessor` **0,754 → 0,900 ms par image** (médiane 0,722 → 0,863), soit **+0,15 ms
  (+19 %)** : c'est le coût de `HOLUnitAnimation::Choose` par unité (2 000 unités). Rien d'autre ne bouge
  (`MassVisualizationComponent EndVisualChanges` 1,88 / 1,81 ms, `InstancedSkinnedMeshComponent` 0,58 / 0,57 ms,
  LOD / visualisation HOL identiques). Rendu et draw calls inchangés (267).
- **Verdict** : coût de J5c.1 **faible**, au fil de jeu seulement, ~0,15 ms par image (~2 % d'une image à 150 fps),
  aucun au fil de rendu ni au GPU, aucun au pas de simulation. Les S7 sous 144 fps viennent de la charge de la
  machine. À recontrôler au repos avant la fusion, comme prévu.

### 2026-10-16 — Fusion dans `main`

- A/B S7 : coût de J5c.1 **faible**, sur le fil de jeu seul (`HOLInterpolatedInstancesProcessor` +0,15 ms par image,
  choix de séquence pour 2 000 unités) ; simulation, rendu, GPU et appels de dessin inchangés. Piste si besoin un jour :
  ne refaire le choix qu'au changement d'état.
- **`main` avancée en avance rapide jusqu'à `j5c-anims` (8d18407)** ; copies non suivies des outils et séquences
  effacées de la copie principale (la branche les apporte). Reste : S1 / S2 / S7 **au repos** (mesures reportées,
  machine chargée : deux ProjectShiva ouverts), S5 court depuis `main` (serveur recuit) ; `quick_check.ps1` peut
  échouer dans la partie « net » quand un client quitte avant le dernier rapport de l'autre (à corriger un jour).
  Suite : J5c.2 (outils et charges visibles), dans le worktree remis au niveau de `main`.

### 2026-10-16 — J5c.2 commencée : outils et charges visibles

- Demandée par l'utilisateur (« commence ») : hache, pioche, panier, fagot, sac visibles, dans le worktree. Repérage en
  lecture seule lancé (plusieurs maillages squelettiques par type dans Mass, données par instance pour cacher les
  pièces, fabrication d'un maillage squelettique par script, coût sur S7). Questions à l'utilisateur après le repérage.
- **Repérage J5c.2** (lecture seule) : un 2e maillage squelettique par type marche mais coûte **~+2 ms** de fil de jeu à
  2 000 Paysans (tout le travail par instance est refait pour chaque composant, chaque image). Retenu (choix
  d'exécution, la recommandation) : **un seul maillage `SKM_Peasant`** = mannequin + toutes les pièces fusionnées, chaque
  pièce liée à 100 % à un os, son fournisseur `TP_Peasant` (mêmes séquences), pièce visible choisie par un **4e float**
  de donnée par instance, autres pièces repliées par **WPO** dans un matériau propre au Paysan : ~0 ms de fil de jeu,
  0 appel de dessin de plus, quelques centaines de triangles. Fabrication par **Geometry Script** (plugin déjà actif :
  `AppendBox` / `AppendCylinder`, `SetAllVertexBoneWeights`, `AppendMesh`, `CopyMeshToSkeletalMesh`). Outils : liés à
  `hand_r` ils glisseraient jusqu'à ~14 cm le long du manche ; exact avec une piste sur `ik_hand_gun` (os non animé)
  portée par la hache et la pioche refaites. Pièges : 4 floats écrits pour tous les types d'un composant, donnée par
  instance dans le WPO à confirmer, boîtes englobantes, pièce changée d'un coup pendant le fondu de 0,2 s.
  Sous-étapes proposées : 2a pièces et aperçu, 2b os porte-outil, 2c maillage / fournisseur / table, 2d choix et
  donnée par instance, 2e matériau, 2f vérification (A/B S7 avec trace).
- **Q1 J5c.2 tranchée** (2026-10-16, choix de l'utilisateur, autre que la recommandation « + bourse ») : **hache pour le
  bois, pioche pour la pierre et l'or, rien pour les baies** (cueillette à mains nues) ; outil visible pendant le geste.
- **Q2 J5c.2** (choix de l'utilisateur) : **hotte** sur le dos, garnie de pierre ou d'or selon la ressource ; bois :
  4 bûches sur l'épaule (inchangé). **Q3** (choix de l'utilisateur) : la nourriture **garde la marche contre le ventre**,
  avec un **panier de baies** tenu là (pas de hotte pour les baies). Conséquence : la marche pierre-or (`A_Peasant_CarryStone`)
  passe des mains au col d'un sac aux **mains sur les bretelles de la hotte** (retouche des bras seulement).
- **Q4** (choix de l'utilisateur, contre la recommandation de cacher le panier) : **tout montrer à l'arrêt**, et adapter
  l'animation pour que la charge reste tenue : **repos chargés** (panier contre le ventre ; aussi bûches tenues et
  mains aux bretelles, pour la cohérence, à juger sur l'aperçu).
- Choix d'exécution : fusion des pièces dans `SKM_Peasant` (pas de 2e maillage), outils exacts par une piste
  `ik_hand_gun` (hache et pioche refaites).

### 2026-10-16 — J5c.2a / J5c.2b : pièces, os porte-outil, repos chargés (worktree, rien de commité)

- **`Tools/Editor/peasant_props.py`** (nouveau) : six pièces, chacune liée à un seul os (voir « Chaîne de fabrication »).

  | id | pièce | os | taille X × Y × Z (cm, repère de la pièce) | couleurs | triangles |
  |---|---|---|---|---|---|
  | 1 | hache | `ik_hand_gun` | 84 × 19 × 5 (manche 82,5 cm Ø 3,2, fer 20 cm de tranchant) | bois, fer, acier | 64 |
  | 2 | pioche | `ik_hand_gun` | 83 × 46 × 5 (pointe à 24 cm, ciseau à 22 cm) | bois, fer | 88 |
  | 3 | panier de baies | `spine_03` | 26 × 29 × 23 (Ø 29 × 24 au bord, 17 cm + tas) | osier, baies rouges, 2 feuilles | 144 |
  | 4 | fagot (4 bûches) | `spine_03` | 97 × 28 × 51 (bûches de 95 cm, Ø 9) | écorce, bout coupé clair | 96 |
  | 5 | hotte de pierre | `spine_03` | 64 × 37 × 64 (hotte 52 cm, Ø 25 × 18 → 36 × 26, 2 bretelles de 9 segments) | osier, cuir, gris | 380 |
  | 6 | hotte d'or | `spine_03` | idem | osier, cuir, jaune | 380 |
  Total ~1 150 triangles (une seule pièce visible à la fois).
- **Corps relevé** (deux lancements `--dump-only`, ~1 min et 20 s) : 20 719 sommets du LOD 0 de
  `SKM_Mannequin_Classic`, os par poids de peau (l'indice d'os suit l'ordre du squelette ; `get_all_bones_info` ne
  marche pas sur la copie : « TargetMesh has no bone attributes ») ; au LOD 1 les poids manquaient (os par segment le
  plus proche). Sert au corps gris de l'aperçu et au contrôle des pièces : surface du dos -20,6 cm à 142 cm de haut,
  dessus de l'épaule 160,5 cm (x = 12), avant de la poitrine +11,5 cm à 138 cm.
- **Placement contre le corps** : hotte reculée (front à ~2 cm des omoplates), bretelles à ~1,2 cm de la peau ;
  **fagot remonté de 9 cm et décalé de 4 cm vers sa droite** (avec de vraies bûches de 4,5 cm de rayon, la place
  validée en traits entrait de 4,5 cm dans la tête, l'épaule et le bras levé) et **main droite de `A_Peasant_CarryWood`
  déplacée** de (12, 14, 162) à (17, 14, 167,5) pour rester sous le fagot. Restes : 9 à 21 points du corps dans le fagot
  ou la hotte, 0,4 à 0,8 cm au plus (contact) ; 0 dans le panier, la hache, la pioche.
- **Os porte-outil** : hache et pioche refabriquées avec la piste `ik_hand_gun` (45 pistes) ; relecture : repère
  hors ligne / Unreal **0,000 cm, 0,000°** (brut), **0,007 cm, 0,074°** (compressé) ; **mains à l'axe du manche**
  0,001 / 0,023 cm (brut) et 0,009 / 0,024 cm (compressé) pour hache / pioche, 0,000 cm hors ligne ; **corps
  identique à la version d'avant** (relu dans Unreal avant et après : jambes et bassin, colonne et tête, bras et mains
  **0,000 cm**) ; hors ligne, poses identiques à `HEAD` à 0 près pour tous les gestes hors bras retouchés.
- **`A_Peasant_CarryStone`** : mains aux bretelles (prises à ±13,5 cm, 15 cm devant, 140 cm de haut, paumes vers la
  poitrine, coudes écartés) ; relecture face à la version d'avant : jambes et bassin, colonne et tête **0,000 cm**, bras
  et mains 20,1 cm (voulu).
- **Repos chargés** (nouveaux, à la fin de `GESTURES`) : `A_Peasant_IdleCarryFood`, `…Wood`, `…Stone`, sur bassin et
  jambes de **`A_SW_Idle`** (11,333 s, 340 images, sur place : racine 0, bassin 0,21 cm entre première et dernière
  image, boucle de la source refermée ; **garde d'escrime**, bassin tourné de ~50° vers sa droite, pied gauche devant) ;
  `steady_spine` 1,0 remet buste et charge face à l'avant ; trois respirations (buste qui se lève un peu, épaules) ;
  bras de la marche chargée de la même charge (bois : main droite au fagot, bras gauche pendant ; pierre : mains aux
  bretelles, penché de 3 à 7° contre -4,6° debout). Relecture : écart cinématique / Unreal 0,001-0,017 cm brut,
  0,027 cm compressé, racine 0,000 cm, première = dernière image, pieds identiques à `A_SW_Idle` à 0,001 cm. Vitesse
  de lecture : 1,0 (la ligne « vitesse de lecture » du journal n'a pas de sens pour un repos).
- **Fabrication** : un lancement (78 s) pour les neuf séquences (six refaites, trois créées) ; ChopWood, MinePick,
  CarryWood, CarryStone modifiées, GatherBerries et CarryFood refaites à l'identique (0,000 cm). Fournisseurs
  `TP_Mannequin_*`, table des types et C++ **non touchés** (J5c.2c-d).
- **Aperçus** (`Saved/AnimPreview/` du worktree) : `<séquence>_contact_sheet.png` et `<séquence>_zoom.png` pour les neuf,
  `A_Peasant_Carry_comparaison.png`, `A_Peasant_IdleCarry_comparaison.png`.
- Points ouverts : jugement de l'utilisateur sur l'aperçu (fagot plus haut et main droite déplacée, hotte et bretelles,
  repos sur la garde d'escrime — sinon un repos chargé pieds parallèles sans base) ; au maillage (J5c.2c), vérifier que le
  fournisseur et « Optimize for Instancing » gardent `ik_hand_gun` (os sans poids aujourd'hui) et le sens des
  triangles ; `copy_mesh_from_skeletal_mesh` ne donne pas les noms d'os (ordre du squelette supposé, vérifié par les
  effectifs symétriques et la distance aux os).
- **Aperçus avec les pièces validés** par l'utilisateur (« ça a l'air bon ») : fagot remonté, idles chargés sur la garde
  de `A_SW_Idle`, mains aux bretelles de la hotte. Commit **3a83e25** sur `j5c-anims`, puis fusion de `main` (J6a.3,
  91f3a6b) dans la branche (950c670, sans conflit). Suite : J5c.2c-f (maillage `SKM_Peasant`, fournisseur, table,
  choix de la pièce, matériau, vérification).

### 2026-10-16 — J5c.2c, d, e : maillage du Paysan, choix de la pièce, matériau (worktree, rien de commité)

- **Maillage `SKM_Peasant`** (`Tools/Editor/make_peasant_mesh.py`, nouveau, commandlet ~1 min) : copie de
  `SKM_Mannequin_Classic` (mêmes LOD, réglages, emplacements de matériau), puis **chaque LOD écrit à part** par
  `CopyMeshToSkeletalMesh` (sans réduction) : corps du même LOD du mannequin + toutes les pièces à pleine définition (une
  pièce lue de la caméra RTS ne doit pas être réduite à rien). Les LOD 1-3 du mannequin se lisent sans leurs poids (données
  de rendu, pas de copie CPU des poids) : **poids du LOD 0 transférés** (`TransferBoneWeightsFromMesh`, point de surface
  le plus proche, os du squelette dont l'ordre est vérifié égal à celui du maillage). « Optimize for Instancing » sur
  chaque LOD. Sommets de rendu par LOD **25 055 / 8 683 / 5 716 / 4 009** (mannequin 23 309 / 6 733 / 3 803 / 2 152, soit
  +1 746 partout) ; triangles corps 41 052 / 10 261 / 5 130 / 2 565 **+ 868** de pièces. 68 os ; `ik_hand_gun` porte
  désormais 290 sommets (hache, pioche), `spine_03` les charges. Relecture de chaque LOD : sommets par masque et os
  exacts (120 / 170 / 302 / 168 / 594 / 196 / 196), couleurs intactes.
- **Pièces fusionnées une fois** (`peasant_props.mesh_parts` / `part_buffers`) : sommets soudés (position, normale,
  couleur ; facettes plates), **masque des pièces dans l'alpha de la couleur de sommet** (bit id - 1, 0 = corps toujours
  montré) : l'osier et les bretelles de la hotte (clé `part`) servent aux deux hottes (masque 48), seul le tas diffère
  (pierre 16, or 32) : 1 746 sommets au lieu de 2 308. D'où **8 pièces au plus** par maillage (`FHOLResourceProps::MaxId`).
- **Sens des triangles** : les maillages d'Unreal sont tournés à l'inverse de `peasant_props` ((b - a) x (c - a) vers
  l'intérieur, mesuré sur 4 106 triangles du corps) ; premier essai non inversé : pièces dessinées à l'envers (faces du
  fond vues de l'intérieur, noires). Le script inverse chaque triangle, garde les normales vers l'extérieur, et vérifie la
  convention sur le corps à chaque fabrication. UV planaires par face (les tangentes recalculées en veulent).
- **Couleurs** : Geometry Script passe les couleurs de sommet en linéaire (sRGB → linéaire) et la fabrication du maillage
  squelettique les garde telles quelles : le shader lit directement la couleur linéaire de la valeur sRGB écrite (pas de
  conversion au matériau) ; l'alpha n'est pas transformé (masque exact).
- **Matériau `M_UnitPlayerColorProps`** (usages SkeletalMesh + InstancedSkinnedMesh) : corps = couleur du joueur (donnée
  0-2, mat, comme `M_UnitPlayerColor`), pièce = couleur de sommet ; **WPO** : une pièce dont le masque n'a pas le bit de la
  donnée 3 est repliée sur l'origine de l'objet (triangles d'aire nulle : rien de dessiné, ombres comprises). **Vérifié
  avant** : le facteur de sommet GPU skin lit la donnée par instance en sommet (`GetMaterialVertexParameters` remplit
  `CustomDataOffset/Count` depuis la scène GPU) ; confirmé en jeu (pièces montrées ou cachées par Paysan). Pas de repli à
  l'opacité masquée nécessaire.
- **Fournisseur `TP_Peasant`** : les 8 séquences de `TP_Mannequin_Classic` dans le même ordre (relues du fournisseur) + les
  trois repos chargés (8 nourriture, 9 bois, 10 pierre-or). `TP_Mannequin_*` et `SKM_Mannequin_*` non touchés (soldats).
- **Table des types** : colonnes `CarryIdleSequences` `(Food=8,Wood=9,Stone=10,Gold=10)`, `GatherProps`
  `(Food=0,Wood=1,Stone=2,Gold=2)`, `CarryProps` `(Food=3,Wood=4,Stone=5,Gold=6)` (structure `FHOLResourceProps`) ;
  Paysan sur `SKM_Peasant` / `TP_Peasant` ; soldats -1 / 0. Bornes vérifiées à la fabrique (`ensure`).
- **Choix** (`HOLUnitAnimation::Choose`) : sort aussi la pièce ; récolte → outil (hache, pioche, rien pour les baies ; la
  charge n'est pas montrée pendant le geste) ; marche chargée → charge ; **debout avec une charge → repos chargé de la
  charge + la charge** ; hystérésis inchangée (un repos chargé n'est pas une marche). La pièce part **à chaque image** en
  4e float (couleur + pièce, structure de 4 floats : `FVector4f` est aligné sur 16 octets et Mass recopie à n'importe quel
  décalage) ; tous les types en écrivent 4 (les soldats 0). Elle change d'un coup pendant le fondu de 0,2 s.
- **Vérifications** : compilation du premier coup ; tests **50/50** (`Animation.Choice` : outils, charges, repos chargés,
  hystérésis ; `Animation.Table` : colonnes, `SKM_Peasant`, séquences 8-10 par leur nom, soldats sur le mannequin) ;
  `quick_check` **PASS** (95 s ; repassé PASS en 149 s, tests 50/50, après la borne des pièces ramenée à 0-8) ; un passage FAIL sur « no two Peasants at rest closer than 90 cm » (86 cm, simulation,
  pas touchée ici), repassé PASS. Captures en jeu (solo, 2 560 × 1 440, `Saved_Tests/Screenshots/WindowsEditor/HOL/` du
  worktree, `props_*.png`, planche **`props_sheet.png`**) : hache dans les mains (en partie derrière l'arbre), pioche
  sur la pierre et sur l'or, baies à mains nues, marches avec panier, fagot, hotte de pierre, hotte d'or, et **les 13
  Paysans debout avec leur charge** (repos chargés, couleurs justes). Script de capture : `props_capture.ps1` du
  scratchpad de la session (ordre de marche donné quand tous sont pleins : ils gardent leur charge, D296).
- Points ouverts : A/B S7 + trace (en attente) ; jugement de l'utilisateur en jeu (taille des pièces, fagot au repos,
  hotte) ; coût sommets : +1 746 sommets skinnés par Paysan à tous les LOD (+7,5 % au LOD 0, +81 % au LOD 3), à mesurer au
  GPU, sinon pièces simplifiées aux LOD 2-3 ; boîtes englobantes : la hache levée dépasse un peu celle du maillage (non
  vu en jeu).
- **Essai en jeu de J5c.2 validé** par l'utilisateur (« très bien ») : outils, charges, repos chargés. Commit sur
  `j5c-anims`, puis fusion de `main` (J6a.4 8e31b19, panneau de triches cd75e85) sans conflit. Vérification de la
  version fusionnée et A/B S7 lancés ; puis avance rapide de `main`.

### 2026-10-16 — J5c.2 : fusion de main, vérifications, A/B S7

- **Version fusionnée** (`j5c-anims` fad246d = J5c.2 88d8f43 + `main` cd75e85 : J6a.4 et panneau de triches), fusion
  automatique sans conflit : compilation du worktree **réussie du premier coup** (89 s, aucune correction) ; tests
  **54/54**, `quick_check` **PASS en 146 s** (toutes les parties, dont build, build site, build trial, net 2 clients),
  du premier coup, sans flake.
- **A/B S7** : A = worktree détaché `MyCrusader-A` à cd75e85 (sans J5c.2, compilé pour l'occasion ; la copie
  principale avait le travail en cours de J6a.5), B = worktree `j5c-anims` fusionné. Même moteur AS_3, S7 20 s hors
  écran (888 × 500, scalability 2), un échauffement par côté puis alterné A B A B A B, un processus à la fois, de 17:57
  à 18:10. **Aucun chevauchement** avec les compilations des autres sessions (journaux d'UBT : compilation de la copie
  principale à 18:10:39, de `MyCrusader-A` par une autre session à 18:12, après la dernière mesure à 18:10:05 ; aucune
  partie lancée depuis la copie principale entre 17:50 et 18:10). Machine chargée (autres sessions, ProjectShiva,
  navigateurs) : 13-84 % de CPU avant les mesures. Journaux : `ab_logs/j5c2_*` du scratchpad de la session.

| # | Côté | CPU avant (%) | ms/pas (max) | fps (1 % bas) | Jeu / rendu / GPU (ms) | Draws | Bois/s |
|---|------|---------------|--------------|---------------|------------------------|-------|--------|
| éch. | A | 50 / 61 / 48 | 2,574 (11,2) | 194,8 (81,9) | 5,05 / 2,27 / 2,09 | 267 | 1 198 |
| éch. | B | 29 / 32 / 43 | 3,101 (15,4) | 148,4 (56,1) | 6,63 / 2,70 / 3,11 | 267 | 1 189 |
| 1 | A | 13 / 17 / 15 | 2,287 (12,7) | 240,9 (94,4) | 4,08 / 1,95 / 2,08 | 267 | 1 192 |
| 2 | B | 16 / 25 / 14 | 3,231 (12,7) | 134,5 (56,5) | 7,32 / 2,94 / 3,16 | 267 | 1 198 |
| 3 | A | 43 / 54 / 84 | 2,812 (12,4) | 170,2 (72,0) | 5,77 / 2,64 / 2,11 | 267 | 1 198 |
| 4 | B | 50 / 51 / 61 | 2,825 (12,0) | 166,6 (70,4) | 5,90 / 2,62 / 3,16 | 267 | 1 198 |
| 5 | A | 58 / 45 / 66 | 2,766 (12,0) | 166,9 (63,6) | 5,89 / 2,63 / 2,10 | 267 | 1 200 |
| 6 | B | 54 / 53 / 62 | 2,921 (12,0) | 162,7 (63,5) | 6,04 / 2,71 / 3,16 | 267 | 1 195 |
| trace | A | 38 / 41 / 58 | 2,749 (15,4) | 175,8 (71,5) | 5,59 / 2,52 / 2,16 | 267 | 1 189 |
| trace | B | 34 / 25 / 26 | 2,740 (10,7) | 183,9 (75,3) | 5,35 / 2,42 / 3,22 | 267 | 1 198 |
| moy. 1-6 | A | | 2,622 | 192,7 (76,7) | 5,25 / 2,41 / 2,10 | 267 | 1 197 |
| moy. 1-6 | B | | 2,992 | 154,6 (63,5) | 6,42 / 2,76 / 3,16 | 267 | 1 197 |
| B − A | | | +0,370 | −38,1 (−13,2) | +1,17 / +0,35 / **+1,06** | 0 | 0 |

- Les moyennes du fil de jeu et du pas sont tirées par la paire 1 (A 1 exceptionnellement rapide, B 2 lent à charge
  égale ; la trace B, à charge comparable, donne 5,35 ms). Sur les paires 3-4 et 5-6 : +0,08 ms par pas, fil de jeu
  +0,14 ms, rendu +0,03 ms, −3,9 fps ; sur la paire des traces, B fait même mieux que A au CPU. Le **GPU**, lui, est
  stable et net : A 2,08-2,16 ms sur 5 passages, B 3,11-3,22 ms sur 5 passages, **+1,06 ms (+50 %)**, quelle que soit la
  charge. Draws inchangés (267 : les pièces sont dans le même maillage), récolte inchangée (1 197 bois/s). Seuils S7 :
  B échoue au pas de 3,0 ms deux fois (éch., 2) et aux 144 fps une fois (2), A jamais (charge ; à refaire au repos).
- **Trace Insights** (`-trace=cpu,gpu`, un A et un B, ~8 000 images chacune, statistiques sur toute la trace) :
  fil de jeu `FEngineLoop::Tick` 5,77 / 5,63 ms ; `MassVisualizationComponent EndVisualChanges` 1,33 / 1,28 ms ;
  `InstancedSkinnedMeshComponent` 0,458 / 0,449 ms (même chiffre sous `SKM_Mannequin_Classic` / `SKM_Peasant`) ;
  `HOLInterpolatedInstancesProcessor` **0,718 → 0,740 ms (+22 µs, +3 %)** (4 floats par instance au lieu de 3, choix de
  la pièce) ; `HOLVisualizationProcessor` 83 → 86 µs, `HOLVisualizationLODProcessor` 71 → 75 µs ;
  `USkinnedMeshComponent_TickComponent` 187 / 181 µs. Rendu : `FSkinningSceneExtension` 137 / 133 µs, `Skinning`
  107 / 105 µs, inchangés. GPU (moyenne par passe) : `Scene` 2,00 → 2,56 ms, **`BasePass` 0,73 → 1,43 ms (+0,70)**,
  **`RenderVelocities` 0,65 → 1,07 ms (+0,42)**, `Skinning` 31 / 31 µs (le skinning se fait dans le shader de sommets
  de chaque passe), `PrePass` 49 / 49 µs. À prendre comme indication : la capture du ciel en temps réel (coût fixe) passe
  aussi de 0,55 à 0,98 ms et le TSR baisse de 0,45 à 0,25 ms (bulles et recouvrements dans les minuteurs GPU) ; le total
  +1,06 ms des 5 + 5 passages reste le chiffre sûr.
- **Verdict** : J5c.2 ne coûte **rien de mesurable au fil de jeu** (+0,02 ms au processeur d'instances, dans le bruit),
  **rien au fil de rendu**, **aucun draw** ; elle coûte **~1 ms de GPU** par image à 1 662 Paysans à l'écran, dans la
  passe de base et la passe de vitesses : chaque Paysan fait passer ses 1 746 sommets de pièces dans le shader de
  sommets (skinning + WPO) à chaque passe, pièces repliées comprises (le WPO les rend invisibles, pas gratuites), et le
  WPO oblige la passe de vitesses. Aujourd'hui l'image est limitée par le fil de jeu (5-6 ms contre 3,2 ms au GPU) :
  pas de fps perdus ici, mais un tiers du GPU de l'image en plus.
- **Recommandation** : alléger les pièces aux **LOD 2-3** (probablement la plupart des Paysans en vue RTS, répartition
  par LOD non mesurée ; les pièces y pèsent +81 % de sommets au LOD 3) : pièces simplifiées (quelques boîtes), voire outils retirés au LOD 3 en gardant les charges ;
  puis remesurer le GPU en S7. À vérifier aussi à la résolution réelle de l'utilisateur (2 560 × 1 440) avant de
  trancher ; pas bloquant pour la fusion. S1/S2/S7 au repos restent à faire.
- **J5c.2 fusionnée dans `main`** (avance rapide jusqu'à fad246d). Coût (A/B S7, machine chargée) : fil de jeu et de
  rendu inchangés, 267 appels de dessin, **GPU +1,06 ms par image** (passe de base +0,70, passe de vitesses +0,42 :
  les 1 746 sommets de pièces passent le shader de sommets même repliés, et le WPO impose la passe de vitesses) ; sans
  effet sur les i/s aujourd'hui (image limitée par le fil de jeu). Piste : pièces simplifiées aux LOD 2-3 (voire sans
  outils au LOD 3), à remesurer. Restent : S1 / S2 / S7 au repos, S5 court ; animation du bâtisseur (marteau).

### 2026-10-16 — Bâtisseur : marteau et coup de marteau (worktree, rien de commité)

- Première phase seulement (geste, pièce, aperçus, fabrication) : **aucun C++, ni table, ni fournisseur, ni maillage**
  touchés. Worktree `MyCrusader-J5c` (`j5c-anims` au niveau de `main` aaa38dc, avec `EHOLUnitActivity::Building` de
  J6a.4).
- **Marteau** (`peasant_props.py`, **pièce 7**, bit 6 du masque : 7 pièces sur 8 possibles) : marteau de charpentier
  tenu d'une main, sur `ik_hand_gun` comme la hache et la pioche (repère d'outil : origine au pommeau, X le long du
  manche, Y vers la **face de frappe**). Manche en bois de **40 cm** (Ø 3,6 → 2,6 cm), tête en fer de **12 cm** en
  travers du manche à 37 cm du pommeau : face ronde Ø 4,4 cm (acier) à 7 cm de l'axe, panne en coin à 5 cm de l'autre
  côté, son tranchant en travers du manche. **72 triangles, 148 sommets** soudés. Le maillage `SKM_Peasant` n'est
  **pas refait** (le marteau y entrera au branchement : `make_peasant_mesh.py` prend toutes les pièces de
  `build_props`).
- **Outil à une main** (`anim_rig.py`) : clé de geste `tool.hands = 1` : `grip` = place de la **main droite** sur le
  manche (`knob_offset` = 7 cm du pommeau, `handle_length` = 37 cm jusqu'à la tête), main gauche **libre** (`hands`
  avec la seule main gauche dans les clés) ; le repère de l'outil sur `ik_hand_gun`, la face fixée dans la main
  droite (sens du mouvement de la tête juste avant la frappe) et le contrôle « main à l'axe du manche » ne portent
  que sur la main droite. **Gestes existants identiques** : poses des neuf autres gestes recalculées avant / après
  la modification, écart **0** (exactement, pistes `ik_hand_gun` comprises).
- **`A_Peasant_Build`** (geste `build`, en fin de `GESTURES`) : boucle de **1,333 s** (40 images à 30 i/s), **deux
  coups** (un toutes les 0,667 s), 45 pistes (44 du corps + `ik_hand_gun`). Penché sur l'ouvrage (buste à 31-37° de
  la verticale, bassin descendu de 14 à 16 cm, genoux fléchis), pieds écartés **cloués** (dérive 0,000 cm), main
  gauche posée à plat sur la poutre à gauche de l'impact (immobile). Par coup : **armé** au-dessus de la tête (main à
  160 cm, coude levé sur le côté, tête du marteau à 184 cm derrière la main) → descente en **0,167 s** (l'avant-bras
  d'abord, le poignet casse à la fin) → **frappe** à 0,167 s puis 0,833 s : face **à plat** (5° et 2° de la verticale)
  sur le dessus d'une poutre à **75 cm de haut, 66 cm devant**, 9 puis 15 cm à sa droite (le 2e coup 5 cm plus à
  droite) → rebond 0,1 s → remontée lente devant l'épaule (0,3 → 0,667 s). Trajet de la tête du marteau ~1 m par
  coup, lisible de haut.
- **Aperçu** (`preview_animation.py`) : scène `beam` (bloc vert du sol au dessus de la poutre, dessus = face du
  marteau à la frappe, croix à l'impact), `--picks f1,f2,…` (images choisies), clé de geste `preview` (images des
  planches de `--all` : ici toutes les 5 images, les deux frappes y sont), contrôles nouveaux : face à chaque frappe,
  enfoncement du marteau et de la main gauche dans la poutre, dérive des pieds et fermeture de la boucle (gestes sans
  base), main libre comptée dans le contrôle de l'outil contre le corps. Planches (worktree) :
  `Saved/AnimPreview/A_Peasant_Build_contact_sheet.png` (8 images, toutes les 5), `A_Peasant_Build_zoom.png` (un coup
  en détail : armé, descente, frappe, rebond, remontée, armé), `A_Peasant_Build_comparaison.png` (hache, pioche,
  marteau à l'armé).
- **Contrôles hors ligne** : main droite sur le manche **0,000 cm** ; mains à leur cible 0,1 cm au plus (main gauche à
  l'armé) ; marteau dans la poutre **0,17 cm** au plus (bord de la face au contact), main gauche 0,00 cm (dessous à
  0,26 cm au-dessus) ; marteau contre le corps, main gauche comprise : **0 point** ; première = dernière image.
- **Fabrication** : `make_peasant_animations.py` gagne `--only=<clé>[,…]` (les autres séquences ne sont pas
  touchées) ; un lancement (13 s, `--only=build`) : séquence créée, **relecture** brute écart FK / Unreal **0,000 cm**,
  compressée **0,005 cm** ; racine 0,000 cm ; première / dernière image 0,000 cm ; durée 1,3333 s ; piste
  `ik_hand_gun` présente, repère hors ligne / Unreal 0,000 cm et 0,000° (brut), 0,009 cm et 0,072° (compressé) ; main
  droite à l'axe du manche 0,000 / 0,010 cm. Les neuf autres séquences et les `Data/*.json` réécrits sont inchangés
  (`git status` : seul `A_Peasant_Build.uasset` nouveau).
- **Points ouverts pour le branchement** : fournisseur `TP_Peasant` (séquence 11 à la fin), colonne de la table
  (séquence et pièce du bâtisseur : p. ex. `BuildSequence` = 11, `BuildProp` = 7, -1 / 0 pour les soldats), maillage
  `SKM_Peasant` refait avec le marteau (+148 sommets à chaque LOD : à peser avec la piste « pièces allégées aux LOD
  2-3 » de l'A/B J5c.2), branche `Building` de `HOLUnitAnimation::Choose` (geste + marteau quelle que soit la
  vitesse, comme la récolte), **orientation vers le chantier** : en solo le bâtiment de `FHOLBuildFragment` (pas
  `Spot`, qui est où le bâtisseur se tient) ; chez le client, qui ne connaît que l'activité, le bâtiment en construction
  répliqué (`AHOLBuilding` non fini) le plus proche, cherché une fois au début du geste comme la source de la récolte
  (`FindNearestStanding`), tourné vers le point le plus proche de son emprise plutôt que vers son centre. Vitesse de
  lecture 1,0 à juger en jeu.
- **Coup de marteau validé sur l'aperçu** par l'utilisateur (« très bien ») ; commité sur `j5c-anims`. Suite :
  branchement (marteau dans `SKM_Peasant`, séquence 11 de `TP_Peasant`, colonnes de la table, branche `Building` de
  `Choose`, bâtisseur tourné vers le bâtiment).

### 2026-10-16 — Bâtisseur branché : marteau, séquence 11, orientation vers le chantier (worktree, rien de commité)

- Worktree `MyCrusader-J5c` (`j5c-anims` 65c765f = `main` aaa38dc + `A_Peasant_Build` et le marteau).
- **Maillage et fournisseur** (`make_peasant_mesh.py`, un lancement de 13 s) : `SKM_Peasant` refait avec le marteau
  (pièce 7, masque 64, `ik_hand_gun`) ; sommets de rendu par LOD **25 203 / 8 831 / 5 864 / 4 157** (avant 25 055 / 8 683 /
  5 716 / 4 009 : **+148 à chaque LOD**), pièces **+1 894 sommets, +940 triangles** (avant +1 746 / +868) ; relecture de
  chaque LOD : masque 64 = 148/148 sommets sur `ik_hand_gun`, les autres pièces inchangées, « relecture des pièces : OK ».
  `TP_Peasant` : **12 séquences**, 0-10 inchangées, **11 `A_Peasant_Build`** (liste `LOADED_IDLES` renommée
  `PEASANT_SEQUENCES`, ajout en fin seulement). Matériau refait à l'identique (le WPO lisait déjà les ids 1-8).
- **Table** : colonnes `BuildSequence`, `BuildPlayRate`, `BuildProp` (ligne, constantes partagées, `ToConstants`) ;
  Paysan **11 / 1,0 / 7**, soldats -1 / 1 / 0 ; bornes vérifiées à la fabrique (`ensure` : séquence dans le fournisseur,
  pièce 0-8) ; `UnitTypes` régénérée (`make_unit_types.py` en commandlet).
- **Choix** (`HOLUnitAnimation::Choose`) : après la récolte, **`Building` → geste du bâtisseur + marteau**, quelle que soit
  la vitesse (poussé, il continue), la charge éventuelle non montrée pendant le geste (comme la récolte) ; nouveau
  drapeau `bFacesSite`. Le bâtisseur qui **marche vers son chantier est `Moving`** (`UHOLBuildProcessor::SyncActivity`) :
  marche ou marche chargée habituelles. Un type sans geste de bâtisseur (soldats) : repos.
- **Orientation** (affichage seul, une fois au début du geste, instantanée comme la récolte, D304 ; même champ
  `FacingYaw` de `FHOLUnitAnimationStateFragment`) : tourné vers le **point de l'emprise le plus proche** (pas son
  centre : à un coin, il regarde le côté devant lui). **Solo** : nouveau champ `FHOLBuildFragment::SiteFootprint`
  (emprise du chantier, écrite par `StartBuilding`, seul changement de la simulation : deux appels, aucune logique
  touchée). **Client** (ne connaît que l'activité) : le bâtiment **non fini** (chantier, ou ses propres fondations) dont
  l'emprise est la plus proche, à moins de rayon + 2 m du corps ; les emprises sont relevées **une fois par image, à la
  première demande** (`TActorIterator<AHOLBuilding>`, quelques bâtiments), rien quand personne ne commence de geste.
- **Vérifications** : compilation du premier coup ; tests **54/54** (`Animation.Choice` : marteau et geste, poussé, avec
  une charge, marche vers le chantier, hystérésis, soldat ; `Animation.Table` : colonnes, séquence 11 = `A_Peasant_Build`
  par son nom, 12 séquences, 0-7 = celles du fournisseur des soldats dans le même ordre, soldats sans geste) ;
  `quick_check` **PASS en 141 s** (toutes les parties, dont build, build site, build trial, net 2 clients où les
  bâtisseurs des clients passent par la recherche du client), du premier coup.
- **Captures en jeu** (solo, 2 560 × 1 440, hors écran, `Saved_Tests/Screenshots/WindowsEditor/HOL/` du worktree) :
  maison A de `quick_check` (-16200, 0), 8 Paysans envoyés en la plaçant : `build_anim_west_0..3`, `_north_0..1`,
  `_south_0..1`, planche **`build_anim_sheet.png`** : cinq bâtisseurs côte à côte face au côté ouest, un ou deux sur les
  côtés nord et sud tournés vers leur côté, marteau dans la main droite, bras levé ou frappe selon le départ décalé.
  Script `build_capture.ps1` du scratchpad de la session (la maison finit en ~6 s à 8 bâtisseurs : captures dans les
  3 premières secondes). Le `quick_check` suivant a vidé le dossier des captures : captures **refaites** (même script, même résultat) et
  copiées avec la planche dans `build_anim\` du scratchpad de la session
  (`C:\Users\baudo\AppData\Local\Temp\claude\c--Users-baudo-Documents-MyCrusader\7fd0d9e2-e7f9-4957-a439-708ba26a0692\scratchpad\build_anim\`).
- Points ouverts : **essai de l'utilisateur** (lisibilité du marteau à la distance du RTS, vitesse 1,0) ; un bâtisseur qui
  enchaîne un chantier voisin **sans marcher** (déjà au contact) garde l'orientation du premier (le geste ne redémarre
  pas) ; coût GPU : +148 sommets de pièce par Paysan à tous les LOD, s'ajoute au +1,06 ms de l'A/B J5c.2 (piste des
  pièces allégées aux LOD 2-3 toujours ouverte) ; puis commit sur `j5c-anims` et fusion dans `main`.
- **Bâtisseur validé en jeu** par l'utilisateur (« ça marche nickel l'anim », chantier posé par `hol.Build.Place`, la
  grille de J6a.5 n'étant pas encore dans la branche). Commité sur `j5c-anims`. Fusion dans `main` après le commit de
  J6a.5 (petite fusion attendue dans `HOLBuildProcessor.cpp`), avec l'accord de mycrusader-58 avant l'avance rapide.
- **Bâtisseur fusionné dans `main`** (avance rapide jusqu'à 3b750bc, avec l'accord de mycrusader-58) après fusion de
  J6a.5 (b5148a2) dans la branche : compilation, tests 62/62, `quick_check` PASS en 151 s.
- **Mesures de clôture** : S1 / S2 / S7 courts au repos et S5 court (serveur recuit) seront faits par mycrusader-58
  dans J6a.6, depuis `main` ≥ bfda9b4 (contient J5c.1, J5c.2 et le bâtisseur) ; chiffres transmis pour la clôture de
  J5c. Ne lancer aucun Unreal pendant cette fenêtre (annoncée).
- **Mesures J6a.6 de mycrusader-58** (`main` bfda9b4, contient toutes les animations ; machine **pas au repos** :
  ProjectShiva relancé deux fois par un autre processus, ~12-13 % de CPU) : S1 2,566 ms PASS, S2 3,178 ms **FAIL**,
  S7 2,790 ms PASS (GPU 3,60 ms contre 2,15 ce matin : +1,45 ms, cohérent avec J5c.2 ; 267 appels de dessin), S5
  **PASS 31 428 o/s** par client (marge 572 o/s ; écart affiché 23,2 cm). Tous les processeurs anciens 10-30 % plus
  lents : la charge. **À refaire au vrai repos** avant la clôture de J5c.
- **Mesures au repos** (mycrusader-58, J6a.6 refait, `main` bfda9b4, ni Unreal ni ProjectShiva ouverts, CPU 8-20 %
  avant) : **S1 2,276 ms, S2 2,882 ms, S7 2,303 ms, S5 30 937 o/s par client (écart 23,3 cm), tous PASS**. S7 :
  1 198 bois/s, image 4,40 / 2,18 / **GPU 3,34 ms** (2,15 ce matin : **+1,2 ms dû aux animations de J5c**, surtout les
  pièces de J5c.2), 223 i/s, 267 appels de dessin. Le +0,148 ms de S7 vient surtout des nouveaux processeurs de J6a.
  Plus aucune mesure en attente pour J5c ; reste la clôture (et, en option, les pièces allégées aux LOD 2-3).

### 2026-10-16 — Clôture de J5c

- Sous-agent, documentation seule, rien de commité. Décisions du chantier au § 21 du GDD : **D318** (source : faites
  par Claude, par script, chaîne et journal ici, worktree fusionné après validation), **D319** (ressource récoltée dans
  les valeurs libres de l'activité, 0 bit ; la dernière valeur libre prise par `Building` en J6a.4 : toute nouvelle
  activité répliquée coûtera un bit), **D320** (pas de geste au dépôt), **D321** (gestes, marches chargées, choix chez le
  client, orientation vers la source, vitesses de lecture), **D322** (outils et charges visibles, charge montrée à
  l'arrêt), **D323** (pièces fusionnées dans `SKM_Peasant`, `ik_hand_gun`, 4e float, WPO ; GPU +1,06 ms à l'A/B, +1,2 ms
  au repos). Paragraphe « Parcours J5c : la vie de la récolte » au § 16.6 (le marteau du bâtisseur y est cité comme
  fait avec la même chaîne, sans numéro propre).
- Catalogue : rien à fabriquer en attente. Piste laissée : pièces allégées aux LOD 2-3.
- **J5c.4 : partie « anims » de `quick_check`** (worktree) : rapport de développement `hol.Units.AnimReport` (ce que
  l'affichage a réellement choisi : séquence et pièce par état d'unité, solo) ; partie de 16 s en solo : Paysans au bois,
  à la pierre, à l'or, aux baies, bâtisseurs, inactifs ; chaque Paysan dessiné doit montrer le geste et la pièce de son
  état au travail, en marche chargée et debout chargé ; `quick_check` PASS deux fois (201 s, 163 s). Un lancement a
  chevauché le `quick_check` de mycrusader-a5 (port réseau commun : « net 2 clients » en échec chez nous) ; relancé
  chacun de son côté. Captures dans le scratchpad (`anims_qc\`).
- Fusion de `main` (J6b.1, 7a3466b) dans `j5c-anims` (3934b62 ; conflit dans `quick_check.ps1` : deux blocs ajoutés au
  même endroit, les deux gardés) : `quick_check` PASS en 219 s, tests 63/63, « build camp » et « anims » PASS ensemble.
  Avance rapide de `main` après le commit de J6b.2 (mycrusader-a5 touche aussi `quick_check.ps1`).

### 2026-10-17 — Pièces allégées : répartition des Paysans par LOD (session mycrusader-32)

- Point 1 de « Prochaine étape exacte » toujours en attente (J6b.2 pas commitée ; mycrusader-a5 préviendra, avance
  rapide de `main` d'accord sur le principe, à redemander avant de la faire).
- Sonde en lecture seule (commandlet, worktree J5c) : `SKM_Peasant` et `SKM_Mannequin_Classic` ont 4 LOD, tailles
  d'écran **1 / 0,5 / 0,25 / 0,125** (hystérésis 0,02) ; sommets du Paysan par LOD **25 203 / 8 831 / 5 864 / 4 157**
  (corps seul 23 309 / 6 733 / 3 803 / 2 152) ; bornes du Paysan étendue (69,4 ; 48,6 ; 97,8), rayon 129,4 cm (les pièces
  l'ont porté de 116,1).
- Sélection du LOD (moteur 5.8, lu dans le code) : faite **sur le GPU, instance par instance**
  (`FInstancedSkinnedMeshSceneProxy::bUseGpuLodSelection`, rayon d'instance = longueur de l'étendue des bornes, 129,4) ;
  taille d'écran = 2 × max(P00, P11) / 2 × R / D ; champ de 90° à l'horizontale (59° à la verticale, 16:9) : taille =
  230 / D (cm). **LOD 3 au-delà de 18,4 m de la caméra**, LOD 2 de 9,2 à 18,4 m, LOD 1 de 4,6 à 9,2 m.
- Donc : S7 (vue « Wide », caméra à 100 m, Paysans entre ~70 et ~170 m) et S7 « Game » (~38 m de haut) : **100 % au
  LOD 3**. En jeu (`AHOLRTSCamera`, tangage 55°, hauteur 15 à 70 m, 35 par défaut) : tout au LOD 3 dès 18,4 m de haut ;
  au zoom maximal (15 m) le bas de l'écran (sol à 15,1-18,4 m) est au LOD 2. Les LOD 0-1 ne se voient jamais en jeu.
- Conséquence : **le LOD 3 est l'aspect du Paysan en jeu** (c'est lui que l'utilisateur a validé) ; les pièces y font
  48 % des sommets. Une version allégée doit donc **se lire pareil depuis la caméra du RTS** ; la piste « outils retirés
  au LOD 3 » est écartée (plus d'outils du tout en jeu). Le LOD 2 ne pèse rien dans S7 : il garde les pièces pleines
  (vue rapprochée du zoom maximal), seul le LOD 3 prend les pièces allégées (constante du script, changeable).
- **Pièces légères** (worktree J5c, rien de commité) : `peasant_props.build_props(sk, light=True)` / `mesh_parts(sk,
  light)` : mêmes ids, os, repères, tailles et couleurs ; outils à 4 côtés au lieu de 6 et sans pommeau, panier et hotte
  à 6 côtés au lieu de 10, sans bord du panier ni cerclages de la hotte, bûches à 5 côtés au lieu de 7, une baie et une
  feuille au lieu de trois et deux, trois morceaux sur le tas au lieu de cinq, pas de fond où la caméra ne regarde
  jamais (panier, hotte, tas), bretelles sur 7 segments au lieu de 9 (points 3 et 8 sautés : la corde passe 0,8 et
  1,7 cm sous eux ; épaule et devant gardés) en boîtes ouvertes aux bouts et côté corps (`box(..., open_faces=)`).
  **1 894 → 966 sommets** (940 → 458 triangles) ; hache 72, pioche 100, panier 118, fagot 120, hotte 224, tas 110 + 110,
  marteau 112. Jeu plein **identique à l'octet près** (tampons comparés avant / après). Aperçu `--light` (planches côte à
  côte dans le scratchpad, `lod\`) : mains au manche à 0,000 cm, mêmes contrôles de pénétration dans le corps (0,4 cm au
  plus pour la hotte, comme la pleine) ; lecture identique à l'échelle de l'aperçu (seuls les cerclages manquent).
- `make_peasant_mesh.py` : `LIGHT_PROPS_LOD = 3` (LOD 0-2 pièces pleines, LOD 3 pièces légères), relecture de chaque LOD
  contre son jeu ; option `--light-lod=N` dans la valeur de `-script` (99 = pièces pleines partout : le maillage d'avant,
  pour l'A/B dans le même worktree en refaisant seulement le maillage).
- `SKM_Peasant` refait (commandlet, worktree J5c) : LOD 0-2 inchangés (pièces pleines +1 894 sommets), **LOD 3 pièces
  légères +966** ; sommets rendus par LOD **25 203 / 8 831 / 5 864 / 3 229** (LOD 3 : 4 157 → 3 229, −22 % pour tout le
  Paysan) ; relecture de chaque LOD contre son jeu de pièces : OK ; `TP_Peasant` refait, mêmes 12 séquences.
- `quick_check -Parts camera,anims` (worktree J5c) : **PASS** en 34 s (chaque état montre sa séquence et sa pièce ;
  captures `anims_*.png` : hottes, outils et charges visibles au zoom maximal).
- **A/B S7 en attente** : mycrusader-a5 (J6b.2) a la machine pour ~40-60 min, attendre son « machine free ». Script prêt :
  scratchpad `ab_lod.ps1` (B = pièces légères, A = `--light-lod=99` ; ordre B A B A B A, maillage refait avant chaque
  passage par `Start-Process`, `quick_check -Parts camera,anims` pendant A1 pour les captures des pièces pleines,
  retour à B à la fin ; journaux `Saved_Tests\Logs\lod_<côté><n>.log` du worktree).
- **A/B S7** (2026-10-17, machine libérée par mycrusader-a5 ; worktree J5c au même code, seul `SKM_Peasant` refait
  entre les passages ; `stress_ab.ps1`, 20 s, hors écran 888 × 500, B A B A B A ; B = pièces légères au LOD 3, A =
  pièces pleines partout) :

  | Passage | GPU (ms) | Fil de rendu (ms) | Fil de jeu (ms) | Simulation (ms/pas) | i/s |
  |---|---|---|---|---|---|
  | B1 | 2,79 | 2,02 | 4,18 | 2,315 | 235,2 |
  | A1 | 3,25 | 2,19 | 4,20 | 2,260 | 233,9 |
  | B2 | 2,76 | 1,95 | 4,16 | 2,252 | 236,2 |
  | A2 | 3,28 | 2,32 | 4,03 | 2,192 | 242,8 |
  | B3 | 2,79 | 1,99 | 4,18 | 2,318 | 234,9 |
  | A3 | 3,27 | 2,11 | 4,17 | 2,268 | 235,6 |
  | **moyennes B / A** | **2,78 / 3,27** | **1,99 / 2,21** | 4,17 / 4,13 | 2,295 / 2,240 | 235,4 / 237,4 |

  **GPU −0,49 ms par image (−15 %)**, très régulier (écart de 0,03 ms au plus dans chaque côté) : environ 40 % du coût des
  pièces repris (S7 avant J5c 2,15 ms, avec les pièces pleines ~3,3). Fil de rendu −0,2 ms (à confirmer, plus bruité).
  Fil de jeu, simulation, appels de dessin (267) et collecte (1 190 bois/s) inchangés : l'image reste limitée par le fil
  de jeu. Chaque échange de maillage relu OK ; maillage final : pièces légères (3 229 sommets au LOD 3).
  `quick_check -Parts camera,anims` avec les pièces pleines : PASS (captures copiées dans le scratchpad,
  `captures_full\`).
- J6b.2 commitée (4df8080, mycrusader-a5) : **fusionnée dans `j5c-anims`** (7595c81, sans conflit ; `quick_check.ps1`
  fusionné tout seul), worktree compilé (29 s), **`quick_check` complet PASS** en 274 s (tests 64/64 ; « build cut » et
  « anims » PASS ensemble ; pièces légères en place dans le worktree, non commitées). Matériau
  `M_UnitPlayerColorProps` remis tel que commité (le script le recrée à l'identique à chaque passage).
- Planche **pièces pleines / légères** (captures de `quick_check`, zoom maximal, agrandies ×2) : scratchpad
  `props_full_vs_light.png` ; fagots, hottes, paniers, pioches, marteaux se lisent pareil (captures de deux parties
  différentes : groupes pas tout à fait les mêmes).
- Avance rapide de `main` à 7595c81 demandée à mycrusader-a5 (ne prend pas les pièces légères, non commitées avant
  l'essai de l'utilisateur).
- **`main` avancé à 7595c81** (accord de mycrusader-a5) ; ses binaires sont à recompiler (Presentation touché), a5
  prévenue. Reste : essai de l'utilisateur sur les pièces légères.
- D324 commitée sur `main` par mycrusader-a5 (e3797f7, touche `quick_check.ps1`) : **avant le commit des pièces légères,
  refusionner `main` dans `j5c-anims`**, recompiler le worktree et repasser `quick_check`.
