# GLORFINDEL-1126b0 : essai de faisabilité B0 (D928, atelier léger, import à la volée)

Branche `art-runtime-b0-1126ar` (worktree Glorfindel). Essai, pas un outil : il répond à Q1 à Q6 avec des preuves.

## Verdicts

| Q | Question | Verdict | Preuve |
|---|---|---|---|
| Q1 | Modèle fixe (FBX / glTF) chargé à la volée dans la build packagée | **OUI pour glTF / glb, NON pour FBX** | Jeu packagé (`Saved/Packages/b0-runtime`), `hol.Art.RuntimeLoad <forum.glb>` : « import returned true, 7 object(s) », « static mesh Forum slots 6 extent X=770 Y=776 Z=493 », 6 `MaterialInstanceDynamic` créés à l'exécution. Géométrie fidèle ; matériaux à plat : 3 fonctions de matériau d'Interchange absentes de la cuisson (`ConvertFromDiffSpec`, `CustomRotator`, `MF_PhongToMetalRoughness`, « Material import might be wrong »). Textures : NON DÉMONTRÉ (les glb exportés n'avaient aucune image). FBX : « Cannot import file. The source data is not supported. Try enabling the [fbx] extension » ; le traducteur FBX ne déclare son format que sous `WITH_EDITOR` (`InterchangeFbxTranslator.cpp:262`-`:268`). |
| Q2 | Ce modèle remplace le corps d'un bâtiment dans le vrai chemin de rendu | **OUI** | « Buildings: 1 piece(s) of 1 building(s) now draw Forum » puis « … now draw spearman_static » : le composant du corps créé par `AHOLBuilding::BuildFinalMesh` (`HOLBuilding.cpp:1257`) dessine le modèle importé. Captures `_forum_glb` (Forum réimporté, blanc) et `_mannequin_on_forum` (le Forum remplacé par le mannequin d'1,80 m). Limite : l'essai remplace le composant ; une fiche DA_Art ne peut pas encore désigner un modèle chargé à la volée. |
| Q3 | Squelette (USkeletalMesh) à la volée | **NON** | Fabrique Interchange entièrement sous `#if WITH_EDITOR && WITH_EDITORONLY_DATA` : `InterchangeSkeletalMeshFactory.cpp:1049` (BeginImportAsset_GameThread) et `:1287` (ImportAsset_Async) ; squelette seul : `InterchangeSkeletonFactory.cpp:32`. Hors Interchange, construire un USkeletalMesh n'existe qu'en éditeur : `SkeletalMesh.h:2561` `Build()` (bloc `WITH_EDITOR` `:2502`-`:2588`), `CommitMeshDescription` `:622` (bloc `WITH_EDITORONLY_DATA` à partir de `:533`). **En vrai** : `arquebusier_skinned.glb` (glTF avec `skins` 1) importé dans le jeu packagé : 1 matériau, **0 modèle à squelette**. |
| Q4 | Animation (UAnimSequence) à la volée | **NON** | Fabrique Interchange sous `#if WITH_EDITOR && WITH_EDITORONLY_DATA` : `InterchangeAnimSequenceFactory.cpp:1122`, `:1250`, `:1417`. Écrire les clés d'une animation passe par `GetController()` (`AnimSequenceBase.h:249`, bloc `WITH_EDITOR` `:238`-`:281`). **En vrai** : `arquebusier_attack.glb` (`animations` 1) : 1 matériau, **0 animation**. |
| Q5 | Corps et animations à la volée dans le rendu en masse des unités (TP_*) | **NON** | `UAnimSequenceTransformProviderData` : `CachedData` (bornes, données précalculées) n'est construit que par `CacheDerivedData` à la cuisson ou dans l'éditeur (`AnimSequenceTransformProviderData.cpp:718`-`:731` Serialize, `:774`-`:779` PostLoad, sous `WITH_EDITOR`) ; une build packagée ne fait que relire. Et Q3 / Q4 sont NON : il n'y a de toute façon ni corps ni séquence à y mettre. Aperçu restant possible : aucun avec le moteur seul (pas de USkeletalMesh à la volée) ; avec un greffon tiers qui fabrique lui-même les données de rendu (glTFRuntime, MIT, non téléchargé), un acteur SkeletalMeshComponent seul dans le level, hors chemin Mass. |
| Q6 | Taille et durée du package Development (Windows) | Client **1136 Mo**, serveur 1063 Mo ; **31 min** de travail | `package.ps1 : PASS (b0-runtime, 87 min ; Windows 1136 MB, WindowsServer 1063 MB)`. 87 min au total dont ~57 min d'attente du verrou d'UBT (builds de Haldir puis d'Arwen, machine saturée : 107 Go / 111 Go de mémoire, UBA tue et relance des compilations). Travail réel 13:50-14:21 : build client + serveur ~25 min, cuisson 4 min, scripts précompilés ~45 s. Un import glTF à la volée : 0,0 s mesuré (fichiers de 0,3 à 1,9 Mo). |

## Ce qui a été fait

1. Lecture du moteur (Q3 à Q5, références ci-dessus).
2. Sources : `Tools/Editor/b0_export_gltf.py` (éditeur caché, GLTFExporter) a écrit `Saved/B0/forum.glb` (SM_Forum), `spearman_static.glb` (SM_Mannequin), `arquebusier_skinned.glb` (SKM_Arquebusier) et `arquebusier_attack.glb` (A_Arquebus_Attack). FBX : `Content/Knight/Soi_Armour_A.fbx`, seul FBX du dépôt.
3. Commande d'essai `hol.Art.RuntimeLoad <fichier> [Buildings]` (Angelscript, appelle `UInterchangeManager::ImportAsset` en mémoire sous `/Game/RuntimeImport`).
4. Build packagée Development (`package.ps1 -Version b0-runtime`), séquence `hol.Test.Run Saved_Tests/b0.seq` hors écran (`-RenderOffScreen`) : partie Plain, captures, 5 imports.

## Captures

- `GLORFINDEL-1126b0_before.png` : le Forum normal (texturé).
- `GLORFINDEL-1126b0_forum_glb.png` : le même Forum réimporté à la volée en glTF : forme exacte, tout blanc (pas de texture dans la source, matériaux à plat).
- `GLORFINDEL-1126b0_mannequin_on_forum.png` : le corps du Forum remplacé à la volée par le mannequin (petite figure blanche au centre).

## Recommandation

**Atelier léger pour les modèles fixes en glTF / glb, pas pour les unités.**

- Modèles fixes (bâtiments, ressources, décor, accessoires) : faisable avec le moteur seul, dans la vraie build. L'artiste exporte en **glTF / glb** (Blender et les autres le font) ; le FBX est refusé hors éditeur. À ajouter pour un vrai atelier : cuire les 3 fonctions de matériau d'Interchange (`DirectoriesToAlwaysCook`), vérifier les textures avec une vraie source texturée, et faire lire à la fiche le modèle chargé (aujourd'hui l'essai remplace le composant).
- Unités (squelette, animations) : **impossible avec le moteur seul**. Un greffon tiers (glTFRuntime, à demander) donnerait au mieux un aperçu à part, jamais le vrai rendu des unités, puisque les données `TP_*` ne se construisent qu'en éditeur (Q5). Pour les unités, il faut donc l'**éditeur emballé** (kit D927, `OpenEditor.cmd`) ou un **aperçu fait de notre côté** : l'artiste envoie son glb, nous importons dans l'éditeur et renvoyons capture ou build.
- Je ne recommande pas d'atelier léger complet : il est irréalisable pour les unités.

## Code de l'essai (isolé, sans effet sur le jeu)

- `Script/Art/RuntimeImport/HOLArtRuntimeLoad.as` : commande de développeur `hol.Art.RuntimeLoad` ; rien ne s'exécute sans elle. Elle refuse un chemin vide, absent ou qui n'est pas un modèle avant d'appeler Interchange (`Refusal`). Tests : `HOLArtRuntimeLoad_Test.as` (4 tests).
- `Tools/Editor/b0_export_gltf.py` : export des sources de l'essai, lancé à la main seulement.
- **Aucun module ni greffon ajouté** au `.uproject` ou au `Build.cs` : Interchange et GLTFExporter sont `EnabledByDefault` dans le moteur et faisaient déjà partie du jeu packagé. Coût pour le jeu : aucun hors de la commande.
- Pas de C++ dans ce lot. Le build du worktree a seulement rattrapé le C++ de main.

## Non vérifié

- Textures à la volée : aucune source glb avec images n'a été essayée.
- Rendu des matériaux une fois les 3 fonctions d'Interchange cuites.
- La fiche DA_Art qui pointerait vers un modèle chargé à la volée (non essayé).
- Un FBX via le traducteur FBX : impossible hors éditeur d'après le code (`:262`-`:268`) ; seul le refus est vu.
- Le défaut du nom de dossier (« Model » pour tous les imports dans le jeu packagé) est corrigé après la mesure (liste explicite de caractères), et vérifié par `Test_RuntimeLoadPackageName`, pas en jeu packagé.

## Tests et commits

- `quick_check -Parts tests` : 2331/2332 FAIL au premier passage. Le seul échec était `Test_RuntimeLoadPackageName`, à cause d'un défaut du test lui-même (barres obliques inverses non échappées : « C0pearmanstatic ») ; corrigé dans 5750c9d. Second passage : **2332/2332 PASS**, sans avertissement de script sur `Script/Art/RuntimeImport`.
- Le test du refus de chemin a été écrit en même temps que la règle (pas de rouge séparé vu) : la file UBT était bloquée par les builds de Haldir et d'Arwen.
- Commits : 7d41451 (commande d'essai + export glTF), 658a7a6 (rangement dans `Script/Art/RuntimeImport`, refus des chemins, tests), un correctif du nom de dossier, 5750c9d (tests corrigés, forme tableau de `GetComponentsByClass`), 8accbf3 (merge de main), puis ce rapport.
- Sans C++ : prête à intégrer par `integrate.ps1` (aucun build demandé par la branche).
- Hors dépôt, restés sur le disque du worktree : `Saved/Packages/b0-runtime` (2,2 Go), `Saved/B0/*.glb`, `Saved_Tests/b0.seq`, `Saved_Tests/b0.json`.
