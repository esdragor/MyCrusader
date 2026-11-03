# Rigging et animation d'unités de foule (RTS médiéval, Mass Entity, UE 5.x) — état au 2026-11-03

Légende de fiabilité : [A] source primaire Epic/éditeur, [B] source secondaire sérieuse ou fil de forum Epic (employé Epic), [C] blog/agrégateur/marketing, à vérifier. Les pages WebFetch ont été résumées par un petit modèle : les citations sont à revérifier sur la page avant décision. Plusieurs pages Epic (annonce 5.8, notes de version complètes) étaient inaccessibles (403) ou incomplètes.

## 1. Sources de rigging et d'animation (Mixamo, Cascadeur, mocap IA, text-to-motion, auto-riggers, packs Fab)

### Takeaway
Mixamo reste gratuit et utilisable commercialement (bipèdes seulement, support Adobe quasi nul), c'est la base la moins chère pour un solo. Pour du combat médiéval spécifique, les packs Fab (MC Longsword, etc.) à squelette Manny sont plus sûrs que la génération IA, dont les licences commerciales et la qualité (glissement des pieds) sont inégales et à vérifier outil par outil.

### Cited Findings
- Mixamo : licence gratuite avec compte Adobe, personnages et animations utilisables sans redevance en projets commerciaux, pas de redistribution des fichiers bruts ; auto-rigger et bibliothèque limités aux bipèdes humanoïdes ; guide tiers daté du 2026-07-17, mis à jour 2026-09-20 [B/C] — [Cinevva guide](https://app.cinevva.com/guides/free-character-animations-rigging) ; [Cinevva AI animation](https://app.cinevva.com/guides/ai-animation-generators-3d)
- Aucune déclaration officielle Adobe datée de 2026 trouvée ; une réponse de forum Adobe de 2024 (d'un membre de la communauté, pas d'Adobe) dit qu'Adobe a abandonné le support et que la licence précède le rachat ; le site fonctionne d'après le guide tiers — [Forum Adobe, FAQ licence Mixamo](https://community.adobe.com/t5/mixamo-discussions/mixamo-faq-licensing-royalties-ownership-eula-and-tos/td-p/13234775) ; [Forum Adobe, jeux commerciaux](https://community.adobe.com/t5/mixamo-discussions/mixamo-characters-for-commercial-games/m-p/11815020)
- Cascadeur : la version gratuite et la version éducation n'autorisent aucun usage commercial [A] — [Cascadeur education](https://cascadeur.com/education) ; offre « Indie » payante pour revenus < 100 k$/an, ~12 €/mois d'après un guide (Sept. 2026), tarifs « changés plus d'une fois » ; autres sites donnent des chiffres contradictoires (Pro 199 $/an, 49 $/mois) — [Cascadeur plans](https://cascadeur.com/plans) ; [Cinevva AI animation](https://app.cinevva.com/guides/ai-animation-generators-3d) ; [Capterra](https://www.capterra.com/p/10015822/Cascadeur/). Les licences Indie/Pro annuelles seraient « perpétuelles » pour les versions sorties pendant l'abonnement [C].
- Mocap vidéo IA, prix indicatifs (blogs/agrégateurs, désaccord entre sources) : DeepMotion à partir de ~9-15 $/mois, gratuit 60 s/mois, palier gratuit non commercial, payant commercial ; Plask ~18 $/mois annuel, 15 s/jour gratuites, conditions commerciales non indiquées ; Rokoko Vision/Video gratuit en une caméra (sortie dite utilisable commercialement), Plus ~20 $, Pro ~40 $ ; Move.ai à l'usage dès ~0,012 $/s (ou 99 $/mois), jugé le meilleur qualitativement ; Meshcapade : aucun prix trouvé — [Cinevva AI animation](https://app.cinevva.com/guides/ai-animation-generators-3d) ; [StraySpark](https://www.strayspark.studio/blog/ai-mocap-indie-developers-2026-comparison) ; [3dxdev](https://3dxdev.com/ai-motion-capture-from-video-in-2026-every-tool-compared/) ; [Film Threat](https://filmthreat.com/features/10-best-ai-tools-for-turning-video-into-3d-character-motion/)
- Défauts communs du mocap vidéo et du text-to-motion : glissement/flottement des pieds, dérive du bassin, tremblements sur mouvements rapides ; clips text-to-motion de 1 à 10 s ; à nettoyer dans un éditeur — [Cinevva AI animation](https://app.cinevva.com/guides/ai-animation-generators-3d) ; test comparatif 2025 : DeepMotion et Plask tremblent/glissent plus que Move.ai — [StraySpark](https://www.strayspark.studio/blog/ai-mocap-indie-developers-2026-comparison)
- Text-to-motion : DeepMotion SayMotion (export FBX/GLB/BVH, lancé 2024, prix actuels à vérifier) — [CG Channel](https://www.cgchannel.com/?p=156294) ; Hunyuan Motion (Tencent, open source, export FBX/BVH) : sources contradictoires sur l'usage commercial, texte de licence non vu — [Hunyuan Motion page tierce](https://to3d.pages.dev/models/hunyuan-motion-1-0) [C]
- Auto-riggers : Meshy (gratuit 100 crédits/mois sous CC BY 4.0 donc attribution requise, payant ~20 $/mois, sortie à soi), Tripo (gratuit 200 crédits/mois, FBX/GLB), AccuRIG (FBX/USD, mouvements ActorCore sous licence séparée), AutoRig.online ; qualité du rig variable selon le modèle plus que selon l'outil — [Cinevva guide](https://app.cinevva.com/guides/free-character-animations-rigging) ; [Cinevva AI animation](https://app.cinevva.com/guides/ai-animation-generators-3d). Blender Rigify non couvert par une source dans cette session.
- Packs Fab combat médiéval repérés (licences Fab standard non relues) : « MC Longsword Vol. 1 » (180 animations, squelette Mannequin UE5), « Medieval Man » (25 anims, squelette SKM_Manny_Simple), « Low Poly Medieval Warrior » (épée/bouclier, squelette Mannequin), « Medieval soldiers » (archer annoncé « soon ») — [Fab MC Longsword](https://www.fab.com/listings/f8493959-85b5-47b6-9fd0-e6fcf6abb1ff) ; [Fab Medieval soldiers](https://www.fab.com/listings/20ccf969-6c0b-4ec8-a68b-d8ffce930be7) (IDs de listing non vérifiés un par un : l'association nom/URL vient d'un résumé de recherche, à confirmer)
- Animations libres : Quaternius Universal Animation Library et Kenney Mini Characters en CC0 — [Cinevva guide](https://app.cinevva.com/guides/free-character-animations-rigging)
- UE 5.8 (juin 2026) apporte baking d'animation automatisé et options de pieds pour le retargeting ; Direct Mesh Controls (expérimental), Control Rig Physics (Bêta) — [GameFromScratch](https://gamefromscratch.com/unreal-engine-5-8-released/) ; [CG Channel](https://www.cgchannel.com/?p=176531)

### Inferences
- Pour un jeu de mêlée/arc médiéval en lot, la combinaison la plus économique et sûre légalement : Mixamo (locomotion, idle, morts génériques) + 1-2 packs Fab de combat sur squelette Manny + retouches Cascadeur Indie si besoin ; le text-to-motion IA ne se justifie que pour du prototypage.
- Les sources Mixamo « frozen » impliquent un risque de disparition : exporter et archiver tout ce qui est utilisé maintenant.
- Les unités de foule ont besoin de peu de clips (marche, course, idle, attaque, mort) : la qualité fine du mocap IA compte moins que pour les héros.

### Gaps
- Textes de licence exacts (Adobe FAQ actuelle, Fab « Standard License », Hunyuan Motion, Plask, Meshcapade) non lus directement.
- Prix et existence d'un palier commercial de Meshcapade, Move.ai à jour : non vérifiés.
- Qualité réelle d'un pack d'archer sur Fab : aucun pack d'archer confirmé.
- Rigify/Blender : non recherché faute de budget d'appels.
- Cavalerie (cheval + cavalier, animations quadrupèdes) : non couverte ; seules AutoRig.online et Meshy/Tripo annoncent des quadrupèdes (affirmations éditeur).

## 2. Retargeting vers Manny/squelette UE5 et automatisation CLI/Python

### Takeaway
L'IK Retargeter est automatisable en Python via IKRetargetBatchOperation ; depuis UE 5.5 l'ancienne fonction duplicate_and_retarget est dépréciée au profit de run_batch_retarget(inputs). La signature varie selon les versions : coder contre la 5.8 réelle.

### Cited Findings
- `IKRetargetBatchOperation.duplicate_and_retarget` existe (5.3) ; la doc 5.7 la déclare dépréciée, remplacée par `run_batch_retarget` avec la structure `IKRetargetBatchOperationInputs` (assets_to_retarget : séquences/blendspaces/montages, ik_retarget_asset, préfixe/suffixe/recherche-remplacement, use_source_path, include_referenced_assets, overwrite_existing_files) [A, via résumé de recherche] — [Python API IKRetargetBatchOperation 5.7](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/IKRetargetBatchOperation?application_version=5.7) ; [IKRetargetBatchOperationInputs](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/IKRetargetBatchOperationInputs.html)
- UE 5.7 : le solveur « stretch limb » de l'IK Rig est recommandé pour les retargets performants au runtime ; mode FK « Copy Local » par chaîne, seuils de LOD par opérateur, profilage du retarget — [Tom Looman, 5.7 perf](https://tomlooman.com/unreal-engine-5-7-performance-highlights/) [B]
- 5.8.1 : correctifs IK Retargeter (import JSON du RigMapper, rotations non normalisées) [A] — [Forum Epic, 5.8 released](https://forums.unrealengine.com/t/unreal-engine-5-8-released/2729274)
- Epic : le ISKM avec AnimSequenceTransformProvider offre « une retarget simple entre squelettes similaires » et de la variation par instance — [Forum Epic AnimBank/ISKM](https://forums.unrealengine.com/t/animbank-and-instanced-skinned-mesh-component/2734825) [B]

### Inferences
- Pipeline CLI plausible : UnrealEditor-Cmd + script Python (`-ExecutePythonScript`) appelant run_batch_retarget sur un dossier de FBX importés ; aucun script complet trouvé, à écrire et tester sur 5.8.
- Choisir le squelette Manny UE5 comme squelette pivot maximise la compatibilité des packs Fab (la plupart sont sur Manny).

### Gaps
- Aucun script d'exemple fonctionnel trouvé ; pas de vérification des noms exacts de champs en 5.8.
- Qualité du retarget Mixamo vers Manny (chaînes, mapping automatique) non documentée ici.
- Squelette custom pour cavaliers/chevaux : non couvert.

## 3. Techniques de rendu de foule : VAT, Animation to Texture, ISKM/AnimBank, Nanite skinné, Mass Avatars, LOD

### Takeaway
En novembre 2026 (UE 5.8, probablement la dernière 5.x), deux voies concurrentes pour des milliers d'unités animées sous Mass : VAT (AnimToTexture + ISM, éprouvé) et Instanced Skinned Mesh Component (ISKM) avec AnimBank / AnimSequenceTransformProvider (nouveau, recommandé par Epic pour 5.8 mais plus bogué, avec une fuite VRAM Mass signalée en 5.8). Nanite skinné/squelettique reste présenté comme expérimental pour les cas de foule ; prudence.

### Cited Findings
- Epic (employé Euan_Carmichael, 2026-06-08) : une fois la 5.8 sortie, Epic recommandera probablement Mass + ISKM + AnimSequenceTransformProvider ; ce fournisseur génère des données d'animation au format GPU, comme AnimBank, avec plus de souplesse pour la variation par instance et le retargeting simple ; avertissement : fonctions nouvelles, potentiellement plus de bogues que le pipeline VAT + Mass ; démo de foule MetaHuman sur Fab annoncée avec la 5.8 — [Forum Epic AnimBank/ISKM](https://forums.unrealengine.com/t/animbank-and-instanced-skinned-mesh-component/2734825) [B]
- ISKM hérite de Skinned Mesh Component, pas de Skeletal Mesh Component : pas d'Animation Blueprint ; on passe de l'entité Mass « déshydratée » (ISKM) à l'acteur « hydraté » (skeletal mesh + AnimBP) pour les unités proches ; le plugin Animation Sharing (Leader Pose) est « beaucoup plus lent » que ISKM — même source [B]
- Réponse Epic antérieure (~mi-2025, post d'environ 417 jours à la date de la recherche) : VAT était la solution recommandée pour l'animation à grande échelle, le vrai instancing de skeletal mesh n'étant pas sur la feuille de route — [Forum Epic, Instanced Skeletal Mesh & Animation Bank](https://forums.unrealengine.com/t/instanced-skeletal-mesh-animation-bank-example/2655952) [B]
- Bogue 5.8 (posté 2026-08-10, D3D12 Windows) : les foules Mass utilisant SkinnedMeshInstance ne libèrent jamais les pistes d'animation à la destruction des entités ; le pool croît à chaque vague (20 entités concurrentes : 20 pistes puis 39 après la 2e vague) et la VRAM suit le cumul ; pas de réponse Epic sur la page ; correctif proposé : observateur sur FMassRepresentationFragment ; pas de contournement recommandé — [Forum Epic, bug fuite GPU](https://forums.unrealengine.com/t/mass-crowds-using-skinnedmeshinstance-leak-gpu-memory-every-wave-of-spawned-and-killed-entities-permanently-grows-the-animation-track-pool-5-8/2742508) [B]. Statut au 2026-11-03 inconnu (peut être corrigé en 5.8.x).
- La doc Mass Gameplay dit l'animation en Instanced Static Mesh « expérimentale » (cité par un résumé de recherche) — [Overview of Mass Gameplay](https://dev.epicgames.com/documentation/ar-ar/unreal-engine/overview-of-mass-gameplay-in-unreal-engine) [A, citation non revérifiée]
- MetaHuman Crowds (5.8) : fonction **Experimentale**, Epic conseille la prudence avant de livrer ; ISKM pour les personnages éloignés sans AnimBP de post-traitement (pas de correctifs d'animation) ; LOD haut détail retirés ; échelle annoncée « centaines sur mobile, milliers sur plateformes haut de gamme », rendu Nanite ou LOD dynamique — [Doc MetaHuman Crowds](https://dev.epicgames.com/documentation/metahuman/metahuman-crowds-in-unreal-engine) ; [GameFromScratch 5.8](https://gamefromscratch.com/unreal-engine-5-8-released/) ; [CG Channel](https://www.cgchannel.com/?p=176531)
- Nanite Skeletal Mesh (doc Epic) : listé comme type de composant supporté ; activer « Enable Nanite Support » dans l'éditeur de skeletal mesh ; un seul draw call par mesh, ombres Virtual Shadow Maps, **instancing avec animation banks**, pas de LOD géométriques (LOD d'animation) ; **morph targets non supportés** ; déformation de mesh rigide limitée ; WPO limité ; nécessite DX12 SM6 ; la page lue ne donne pas de libellé de statut (Expérimental/Production) — [Doc Nanite](https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine) [A, via résumé]
- Nanite skinning est étiqueté « Experimental » dans l'analyse 5.7 de Tom Looman (usage principal : feuillage Nanite animé par skinning plutôt que WPO) ; option `NanitePixelProgrammableDistance` pour désactiver le Nanite sur les skinned meshes lointains — [Tom Looman 5.7](https://tomlooman.com/unreal-engine-5-7-performance-highlights/) ; [Digital Production 5.7](https://digitalproduction.com/2025/09/22/unreal-engine-5-7-experimental-tech-measured-expectations/) [B]
- Notes de version 5.8 : aucun libellé de statut pour Nanite squelettique/Mass/crowds ; seuls MegaLights, Movie Render Graph, Live Link Hub, Dataflow et Chaos Cloth Panel Editor sont « Production-Ready » ; rapport utilisateur non confirmé : meshes Nanite skinnés absents de la scène Lumen en ray tracing matériel — [Forum Epic, 5.8 released](https://forums.unrealengine.com/t/unreal-engine-5-8-released/2729274) [B]
- Plugins VAT du marché : AnimToTexture (plugin Epic, base de plusieurs outils), Auto VATs, AnimToMesh 2.0 (exige AnimToTexture, EditorScriptingUtilities, PCG ; instancer pour milliers d'instances), OverCrowd (Niagara + VAT ; version 5.7 incompatible avec les foules faites en 5.6), JBernardic Vertex Animation Toolkit — [Forum Epic Auto VATs](https://forums.unrealengine.com/t/metacode-auto-vats-automatic-vat-convertor/2737326) ; [Forum Epic OverCrowd](https://forums.unrealengine.com/t/overcrowd-overcrowd-massive-crowd-simulation/2669477) ; [Forum Epic JBernardic](https://forums.unrealengine.com/t/jbernardic-vertex-animation-toolkit-crowd-movement/2840895) [C : annonces d'éditeurs]
- UE 5.7 Mass : time-slicing des processeurs (`FMassEntityQuery::FExecutionLimiter`), appels d'observateurs groupés (~20x moins d'exécutions dans CitySample) — [Tom Looman 5.7](https://tomlooman.com/unreal-engine-5-7-performance-highlights/) [B]
- UE 5.8 serait la dernière version majeure UE5 (UE6 en préparation, previews « fin 2027 environ » d'après GameFromScratch ; Epic page pt-BR : « dernière version majeure prévue ») — [GameFromScratch](https://gamefromscratch.com/unreal-engine-5-8-released/) ; [Epic 5.8](https://www.unrealengine.com/news/unreal-engine-5-8-is-now-available?lang=pt-BR)

### Inferences
- Pour HeroesOfLiberty (Mass déjà utilisé) : architecture à deux niveaux (a) ISKM/AnimSequenceTransformProvider ou VAT pour la masse, (b) acteur skeletal mesh + AnimBP pour héros/unités sélectionnées ou en combat rapproché, en accord avec le schéma « hydratation » décrit par Epic.
- Vu le statut « nouveau, plus de bogues » et la fuite VRAM en 5.8 (dynamique : unités qui meurent et apparaissent, exactement un RTS), VAT + Mass reste le choix prudent tant que la fuite n'est pas confirmée corrigée ; ISKM mérite un prototype isolé.
- Nanite squelettique : intérêt faible pour des unités à faible nombre de polygones, morph targets non supportés ; ne pas en dépendre.
- Avec UE6 à ~2027+, le choix de pipeline en 5.8 est le dernier de la lignée 5 : un développement lancé maintenant restera sur 5.8 pendant longtemps.

### Gaps
- Statut officiel (Expérimental/Bêta/Production) de Nanite Skeletal Mesh en 5.8 : non trouvé dans les pages lues.
- Contenu officiel de la documentation AnimSequenceTransformProvider/AnimBank : non trouvé.
- Si la fuite de pistes ISKM a été corrigée dans 5.8.x : inconnu.
- Doc de la configuration Mass (MassRepresentation, UMassStationaryVisualizationTrait) non lue en détail.
- Aucune notion d'« impostor » animé officiel pour Mass trouvée ; seulement conseils de blogs.

## 4. Variation de matériaux/meshes et couleurs d'équipe

### Takeaway
La voie documentée pour Mass + ISKM est le per-instance custom data (CustomDataFloats via AddBatchedCustomDataFloats) ; pour les acteurs/composants classiques, Custom Primitive Data. Les deux alimentent un matériau à masque de couleur (équipe, variantes).

### Cited Findings
- `FSkinnedMeshInstanceVisualizationDesc` possède un tableau `CustomDataFloats` propagé à `UInstancedSkinnedMeshComponent::PerInstanceCustomData` ; `FMassInstancedSkinnedMeshInfo::AddBatchedCustomDataFloats` (surcharges uniforme par LODSignificance, ou par mesh dans l'ordre de Desc.Meshes) n'est sûr que depuis un Mass Processor ; si on ajoute du custom data, il faut le fournir pour chaque instance — [Epic API FMassInstancedSkinnedMeshInfo](https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/MassRepresentation/FMassInstancedSkinnedMeshInfo) ; [AddBatchedCustomData](https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/MassRepresentation/FMassInstancedSkinnedMeshInfo/AddBatchedCustomData) [A]
- Custom Primitive Data pour rayure/couleur d'équipe : tutoriel communautaire basé sur la présentation GDC 2022 de Matt Oztalay — [Forum Epic, Team Color Stripe](https://forums.unrealengine.com/t/community-tutorial-making-a-team-color-stripe-with-custom-primitive-data/1214415) [B]
- Bogue UE-361698 : modifier les Custom Primitive Data dans le panneau Détails d'un acteur BP avec Skeletal Mesh ne se reflète pas dans le viewport (5.7, correctif visé 5.8) — [Issue tracker UE-361698](https://issues.unrealengine.com/issue/UE-361698) [A]
- ISKM permet de la variation par instance via AnimSequenceTransformProvider (décalage de phase, etc.) — [Forum Epic AnimBank/ISKM](https://forums.unrealengine.com/t/animbank-and-instanced-skinned-mesh-component/2734825) [B]

### Inferences
- Masque de couleur : une texture de masque (R = armure/tabard d'équipe, G = variation) + 1-4 floats de custom data par instance suffisent pour équipe + teinte + variation d'animation (décalage de phase) sans multiplier les matériaux, donc sans casser le batching.
- Comme VAT et ISM partagent aussi PerInstanceCustomData, la même logique de matériau peut servir aux deux voies.

### Gaps
- Aucun exemple chiffré du coût de variantes de mesh modulaires (casque/arme) dans Mass.
- Variation d'armes (épée/lance séparées) sur ISKM : non documentée.

## 5. Budgets et chiffres de performance connus

### Takeaway
Pas de benchmark Epic officiel pour Mass + ISKM ; les chiffres disponibles sont des blogs/démos (à valider sur le matériel cible). Ordre de grandeur : VAT + instancing réduit énormément le coût CPU par rapport aux skeletal meshes classiques, et les skeletal meshes proches doivent rester quelques dizaines.

### Cited Findings
- Démo de foule de stade GPU-driven : VAT + HISM font passer le temps game thread de 34,15 ms à 0,66 ms vs skeletal meshes, 6 072 spectateurs, RTX 3070 (sans Mass) — [daily.dev, résumé](https://daily.dev/posts/novel-high-performance-framework-for-gpu-driven-crowd-systems-t1max17p3) [C]
- Blog StraySpark (auteur promeut ses propres outils, test unique) : coût de simulation de foule Mass 1 000 agents 1,2 ms (RTX 4080) / 1,8 (4060) / 2,1 (PS5) ; 5 000 : 2,8 / 4,5 / 5,2 ms ; 10 000 : 4,6 / 7,8 / 9,1 ms ; 20 000 : 8,2 / 14,1 / 16,5 ms ; en dessous de ~5 000 le rendu domine, au-dessus c'est la simulation ; un agent skeletal mesh coûte ~50x un agent static mesh instancié ; consoles en difficulté au-delà de ~5 000 sans culling agressif ; LOD suggérés : skeletal 0-25 m, VAT 25-60 m, ISM sans animation 60-150 m, impostor/billboard au-delà (distances indicatives, incohérentes entre sections) — [StraySpark Mass AI](https://www.strayspark.studio/blog/crowd-traffic-simulation-ue5-mass-ai) [C]
- Anecdotes 5.6 : pénalité à la création de nombreuses instances skinnées liée à « Post Tick Component Update » (fil de forum, détails non revérifiés) — résumé de recherche sur [Forum Epic](https://forums.unrealengine.com/t/instanced-skeletal-mesh-animation-bank-example/2655952) [C]
- Le plugin MetaHuman Crowd est annoncé pour « centaines d'acteurs sur mobile à milliers sur plateformes haut de gamme » (chiffre marketing Epic, sans mesure) — [GameFromScratch](https://gamefromscratch.com/unreal-engine-5-8-released/) [B]
- Epic ne donne pas de chiffres de performance dans la page MetaHuman Crowds — [Doc MetaHuman Crowds](https://dev.epicgames.com/documentation/metahuman/metahuman-crowds-in-unreal-engine) [A]

### Inferences
- Pour un RTS (caméra haute, milliers d'unités visibles, beaucoup à faible taille à l'écran), viser : quelques dizaines d'acteurs skeletal (héros, unités en duel proche), des centaines à milliers en VAT/ISKM, le reste en LOD statique ; mesurer avec Unreal Insights et stat unit sur la machine de test réelle.
- Le budget CPU de simulation (Mass) devient dominant au-delà de ~5 000 agents : vérifier le time-slicing 5.7+.

### Gaps
- Aucune mesure indépendante, reproductible, de Mass + ISKM ou VAT sur UE 5.7/5.8 avec nombres d'unités RTS.
- Coût GPU du skinning (ms GPU par 1 000 instances) : non trouvé.
- Budgets triangles/bones recommandés par Epic pour foules : non trouvés.
- Pas de chiffres de mémoire VRAM pour AnimBank/VAT par clip.

## 6. Synthèse des incertitudes à lever (pour le rapporteur)
- Vérifier sur la page Adobe officielle la licence Mixamo en vigueur (FAQ) et la page Cascadeur/Plask/Move.ai pour usage commercial avant tout achat.
- Prototyper ISKM + AnimSequenceTransformProvider vs VAT sur 5.8.x, avec test de spawn/kill répété (fuite de pistes).
- Lire les notes de version officielles 5.6, 5.7 et 5.8 (non accessibles ici) pour le statut précis de Nanite skinning et de Mass Representation.
