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
| Coup de hache (bois) | Paysan | J5c.1 | **fabriquée** (`A_Peasant_ChopWood`, 1,2 s, J5c.1a) ; **validée sur l'aperçu** par l'utilisateur (2026-10-16) ; **branchée** (indice 2, J5c.1b-d, vitesse 1,0)  ; **validée en jeu** (essai du 2026-10-16), commitée a9e4269 |
| Coup de pioche (pierre, or) | Paysan | J5c.1 | **fabriquée** (`A_Peasant_MinePick`, 1,333 s, une séquence pour les deux) ; **branchée** (indice 3, vitesse 1,0)  ; **validée en jeu** (essai du 2026-10-16), commitée a9e4269 |
| Cueillette (baies) | Paysan | J5c.1 | **fabriquée** (`A_Peasant_GatherBerries`, 1,8 s) ; **branchée** (indice 4, vitesse 1,0)  ; **validée en jeu** (essai du 2026-10-16), commitée a9e4269 |
| Marche chargée, nourriture | Paysan | J5c.1 / J5c.2 | **fabriquée** (`A_Peasant_CarryFood`, renommée depuis `A_Peasant_CarryWalk` ; 1,0 s, jambes de `A_SW_Unarmed_Walk`, charge contre le ventre) ; vitesse de lecture **3,57** (règle du projet) ; **validée sur l'aperçu pour la nourriture** (2026-10-16) ; **branchée** (indice 5)  ; **validée en jeu** (essai du 2026-10-16), commitée a9e4269 |
| Marche chargée, bois | Paysan | J5c.1 / J5c.2 | **fabriquée** (`A_Peasant_CarryWood`, mêmes jambes et cadence, fagot de 4 bûches sur l'épaule droite) ; vitesse **3,57** ; **branchée** (indice 6)  ; **validée en jeu** (essai du 2026-10-16), commitée a9e4269 |
| Marche chargée, pierre et or | Paysan | J5c.1 / J5c.2 | **fabriquée** (`A_Peasant_CarryStone`, mêmes jambes et cadence, sac sur le dos, buste penché en avant) ; vitesse **3,57** ; **branchée** (indice 7)  ; **validée en jeu** (essai du 2026-10-16), commitée a9e4269 |
| Pose au dépôt | Paysan | — | **abandonnée** (2026-10-16 : pas de geste, comme AoE4 ; le serveur ne s'arrête pas au dépôt) |
| Repos, marche | toutes | M2 (D234) | empruntées au pack `CloseCombat` (épéiste) |

## Prochaine étape exacte

J5c.1a faite ; **J5c.1 b, c et d faits dans le worktree** (2026-10-16, rien de commité, voir le journal) : les six
séquences branchées (fournisseur, table des types, choix chez le client, Paysan tourné vers sa source), tests 41/41,
`quick_check` PASS, captures en jeu `Saved_Tests/Screenshots/WindowsEditor/HOL/anim_*.png` (worktree). Reste
J5c.1f : captures en gros plan dans `quick_check` (`hol.Camera.Zoom` ajouté, mais 15 m au plus près), S1 / S2 / S7
/ S5 courts, **essai de l'utilisateur** (il juge les gestes et la vitesse des marches chargées en jeu), puis commit
sur `j5c-anims` et fusion. À montrer à l'utilisateur : vitesses de lecture (voir le journal du branchement).

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
