# Outils de génération 3D par IA pour un RTS médiéval sous UE 5.8 (état au 2026-11-03)

Notes en français. Fiabilité : [OFF] = page officielle du fournisseur ; [TIERS-FIABLE] = article indépendant / dépôt public ; [AGRÉGATEUR] = comparateur/blog de prix, souvent contradictoire ; [MARKETING] = communiqué, non vérifié. Aucun test pratique de ma part : tout est issu de pages web. Aucun fil Reddit/Discord de développeurs n'a pu être atteint (voir Lacunes).

## 1. Qualité de sortie (props, bâtiments, rochers, arbres, personnages), polycount, topologie, UV/PBR, besoin de retopo/LOD

### Takeaway
Aucune source indépendante rigoureuse n'a mesuré le polycount ni la topologie des services commerciaux ; les guides de 2026 s'accordent seulement sur le fait que les sorties demandent un nettoyage avant usage jeu. Tripo P2.0 (21 sept. 2026) et Rodin Gen-2 annoncent des quads natifs et des budgets de faces réglables, mais cela reste de l'affirmation éditeur non vérifiée de façon indépendante.

### Cited Findings
- Tripo P2.0 lancé le 21 septembre 2026 (parfois décrit comme « preview ») : quads natifs jusqu'à 25 000 faces, triangles jusqu'à 50 000 ; P1.0 faisait 500 à 20 000 triangles sans quads. Annoncé « pour jeux : personnages, véhicules, props, bâtiments » [MARKETING] — [VoxelMatters](https://www.voxelmatters.com/tripo-ai-launches-p2-0-model-to-generate-native-quad-meshes-for-production-pipelines/), [Digital Production, 2026-09-29](https://digitalproduction.com/2026/09/29/tripo-p2-0-put-quad-mesh-generation-to-the-test/)
- Digital Production (aperçu, pas un banc d'essai mesuré) : « quad natif » est une promesse bien plus large que « un fichier contenant des quads » ; aucune mesure publiée de l'edge flow, des pôles ou de la non-manifold ; l'aptitude au rigging reste non vérifiée ; jusqu'à 4 variantes de nombre de faces à la fois (pas un vrai système de LOD) ; FBX recommandé pour garder les quads (GLB peut trianguler) [TIERS-FIABLE] — [Digital Production](https://digitalproduction.com/2026/09/29/tripo-p2-0-put-quad-mesh-generation-to-the-test/)
- RuntimeWire : Tripo n'a publié aucun benchmark indépendant étayant la revendication « premier modèle à topologie quad native » [TIERS-FIABLE] — [RuntimeWire via recherche](https://runtimewire.com/article/tripo-ai-p2-native-quad-meshes-3d-generation)
- Tripo API (docs officielles) : modèles H3.1 (id d'exemple « v3.1-20260211 », précision visuelle/structure, jusqu'à 2 millions de polygones pour la série H) et P2.0 (quads natifs) ; texture AI jusqu'à 8K ; segmentation de parties V2 ; PBR complet ; contrôle fin du nombre de faces en texte-vers-3D [OFF] — [Tripo developers](https://developers.tripo3d.ai/)
- Meshy : modèles Meshy 7.1 / 6 / 6 Lite, topologie « Smart Topology » T1/T2, textures 2K/4K/8K (8K via modèle de texture « Meshy 7 Flagship »), option « Ultra géométrie » 2K/4K (+5 crédits côté API) [OFF] — [Meshy pricing webapp](https://docs.meshy.ai/en/webapp/pricing), [Meshy API pricing](https://docs.meshy.ai/en/api/pricing)
- Meshy Remesh accepte un `target_polycount` et une topologie quad ; les entrées de rigging sont plafonnées à 380 000 triangles (300 000 en tri, 150 000 en quad après remesh) [OFF] — [Meshy rigging](https://docs.meshy.ai/en/api/rigging-and-animation)
- Rodin Gen-2 (via Runware, tiers) : sorties GLB/FBX/OBJ/USDZ/STL, `meshMode` quad par défaut, préréglages low-poly (guide : quad « low » 8 000 faces, « extra-low » 4 000 ; le schéma JSON donne d'autres chiffres, donc contradictoire) [TIERS/AGRÉGATEUR] — [Runware guide topologie](https://runware.ai/docs/models/hyper3d-rodin-gen-2/guides/topology-and-polygon-budget), [schéma Runware](https://runware.ai/docs/models/hyper3d-rodin-gen-2/schema.json)
- TRELLIS.2 (open, 4 B paramètres) : résolutions 512³/1024³/1536³ en ~3 s / 17 s / 60 s sur H100 ; sortie GLB avec PBR (Base Color, Roughness, Metallic, Opacity) ; décimation d'exemple à 1 000 000 de faces (donc forte retopo/décimation nécessaire pour un RTS) [OFF] — [GitHub microsoft/TRELLIS.2](https://github.com/microsoft/TRELLIS.2)
- Hunyuan3D en local (retour pratique, mai 2026) : maillages souvent non manifold, échelle irréaliste, artefacts (doigts/pieds en trop), topologie moins propre que Tripo, mais « souvent utilisables directement dans les moteurs et Blender » ; qualité hors ligne « légèrement inférieure aux services commerciaux » [TIERS-FIABLE, un seul auteur] — [tspi.at, 2026-05-01](https://www.tspi.at/2026/05/01/opensource3dassets.html)
- Retours communautaires cités par un roundup : cheveux, plis de tissu et détails mécaniques fins demandent un nettoyage important sur Meshy (fils r/blender, rapportés indirectement) [AGRÉGATEUR] — [StackSheriff](https://stacksheriff.com/ai-tools/ai-3d-model-generator-roundup-meshy-tripo3d-luma-more-tested/) ; ce même article précise qu'il n'a pas fait de tests de génération lui-même.
- Guide Wavect (publié 2026-08-13, revu 2026-09-02) : aucun outil n'est présenté comme prêt pour la production pour des personnages déformables ; fixer moteur, plateforme et budget de polys avant de générer ; mesurer le coût par asset accepté sur un pilote payant (prop statique, hard-surface, organique, personnage animé) [TIERS] — [Wavect](https://wavect.io/blog/ai-3d-model-generators-game-development-2026.md)
- Luma : les sorties NeRF/Gaussian Splat ont un nombre de polys trop élevé pour le temps réel et ne sont pas des formats moteur standard ; l'export 3D via Dream Machine est moins direct que Meshy/Tripo [AGRÉGATEUR, non testé] — [StackSheriff](https://stacksheriff.com/ai-tools/ai-3d-model-generator-roundup-meshy-tripo3d-luma-more-tested/)
- Classements de blogs (2026), à prendre comme opinion proche des vendeurs : Meshy = choix par défaut du dev solo/texturing, Tripo = meilleur rapport qualité/prix et low-poly propre pour le jeu, Rodin = « assets héros finaux » — [Krea](https://www.krea.ai/blog/best-ai-3d-model-generators-2026), [IndieHackers](https://www.indiehackers.com/post/best-ai-3d-model-generator-in-2026-i-tested-9-of-the-best-and-here-is-what-i-found-70ecab1a0a), [RapidDirect](https://www.rapiddirect.com/blog/best-8-ai-3d-model-generators/)

### Inferences
- Pour un RTS avec centaines d'unités à l'écran, le goulot n'est pas la génération mais le budget de triangles par instance : il faudra une passe de décimation/LOD (Remesh Meshy, retopo Tripo, ou outil UE type Nanite/auto-LOD) quel que soit l'outil. Nanite aiderait pour bâtiments/rochers statiques mais pas pour les unités déformables (skinned).
- Les props statiques (rochers, arbres, bâtiments, caisses) sont le meilleur cas d'usage ; les personnages et pièces articulées exigent une relecture manuelle.
- Les revendications « prêt pour animation / quad propre » de Tripo P2.0 et Rodin sont à tester soi-même sur 3-4 assets représentatifs avant tout achat.

### Gaps
- Aucune mesure indépendante de polycount, de topologie, de qualité UV ni de PBR pour Meshy 7.1, Tripo H3.1/P2.0 ou Rodin Gen-2.5.
- Pas de retours directs de développeurs de RTS/Unreal trouvés (aucun fil Reddit/Discord récupéré par l'outil de recherche).
- Qualité spécifique « médiéval » (armures, châteaux, chaumières) non documentée.

## 2. Auto-rigging et animation (compatibilité Manny / squelette perso / foules)

### Takeaway
Meshy et Tripo proposent un auto-rig humanoïde avec bibliothèques d'animations, exportables en FBX/GLB, mais aucune source n'indique une compatibilité avec le squelette UE5 Manny/Quinn ; il faudra retarget (IK Retargeter d'UE) vers Manny ou un squelette maison.

### Cited Findings
- Meshy rigging : humanoïdes bipèdes uniquement ; mallages non texturés, non-humanoïdes et bas-reliefs non supportés ; entrée GLB texturé avec visage vers +Z ; sorties FBX et GLB (rigged + animations basiques marche/course) ; nom des os / compatibilité Mixamo ou Manny non précisés ; 5 crédits par rig, 3 crédits par action d'animation (jusqu'à 10 actions par requête) ; Text-to-Motion 3 crédits (swift) ou 10 (prime) [OFF] — [Meshy rigging](https://docs.meshy.ai/en/api/rigging-and-animation), [Meshy API pricing](https://docs.meshy.ai/en/api/pricing)
- Meshy webapp : rigging et animation à 0 crédit dans l'UI ; clips au-delà d'un lot gratuit réservés aux plans payants [OFF] — [Meshy pricing webapp](https://docs.meshy.ai/en/webapp/pricing) ; (contradiction apparente avec les 5 crédits de l'API : UI vs API)
- Meshy annonce 600+ presets de mouvement qui se retargetent ; le roundup juge cela du retargeting, pas de la simulation, adapté au prototypage [AGRÉGATEUR] — [StackSheriff](https://stacksheriff.com/ai-tools/ai-3d-model-generator-roundup-meshy-tripo3d-luma-more-tested/)
- Tripo : Rig Check gratuit, Auto Rig 25 crédits, Animation Retarget 10 crédits par animation (jusqu'à 5 en lot), 100+ mouvements préréglés, rig pour « divers types de modèles » [OFF] — [Tripo developers](https://developers.tripo3d.ai/)
- Rodin : mention d'une sortie en pose T/A pour rigging ; aucune documentation d'os ou de squelette trouvée [TIERS] — [Runware](https://runware.ai/docs/models/hyper3d-rodin-gen-2/guides/topology-and-polygon-budget)
- Le « DCC Bridge » de Tripo couvre Blender, Maya, 3ds Max, Unreal Engine, Unity, Godot, ZBrush, etc. (versions minimales requises) [TIERS, article Digital Production] — [Digital Production](https://digitalproduction.com/2026/09/29/tripo-p2-0-put-quad-mesh-generation-to-the-test/)
- Wavect : aucun outil n'est prêt pour la production de personnages déformables ; vérifier la topologie de déformation [TIERS] — [Wavect](https://wavect.io/blog/ai-3d-model-generators-game-development-2026.md)

### Inferences
- Pour des foules (centaines d'unités), un auto-rig générique par unité est mal adapté : mieux vaut un nombre limité de corps de base riggés sur le squelette du projet (ou Manny) et utiliser l'IA pour les têtes, armes, armures, bannières, bâtiments (pièces rigides). Le retarget IK d'UE5 de l'animation vers un squelette tiers est faisable mais non documenté par ces fournisseurs.
- Un pipeline possible : générer le mesh de personnage, le riger sur le squelette cible dans Blender/UE (skin-transfer), plutôt que de compter sur le rig du fournisseur.

### Gaps
- Aucune confirmation officielle que les os de Meshy/Tripo correspondent à UE Manny/Mixamo.
- Rigging des quadrupèdes/chevaux (important en médiéval) : Tripo dit « divers types », Meshy non (humanoïde seulement) ; non testé.
- Résultats concrets de qualité de skinning non trouvés.

## 3. API / CLI / MCP, formats d'export, automatisation (PowerShell/Python)

### Takeaway
Tripo et Meshy ont tous deux des API REST asynchrones et des outils agent (CLI npm de Tripo ; serveur MCP officiel de Meshy) ; les serveurs MCP/Tripo officiels les plus visibles sont limités à Blender. Rodin est surtout accessible via des plateformes tierces (fal.ai, Runware, WaveSpeed). Les modèles ouverts demandent Python/CUDA, souvent Linux.

### Cited Findings
- Tripo API : REST (exemples Python `requests`) ; CLI `tripo-cli` installable par `npm install -g tripo-cli`, avec une invite pour le configurer dans Codex, Claude Code ou Cursor ; export GLB, FBX, OBJ, USDZ et autres ; 1 crédit = 0,01 USD [OFF] — [Tripo developers](https://developers.tripo3d.ai/) ; (ni SDK ni MCP mentionnés sur cette page)
- Tripo MCP officiel (VAST-AI-Research/tripo-mcp) : MIT, Python, lancé par `uvx tripo-mcp`, marqué alpha, ne supporte que l'addon Blender ; version 0.1.2 sur PyPI, dernière activité visible avril 2025 (possiblement dépassé) [OFF/TIERS] — [GitHub tripo-mcp](https://github.com/VAST-AI-Research/tripo-mcp), [PyPI](https://pypi.org/project/tripo-mcp/)
- Meshy : API prépayée (crédits achetés dans les réglages d'abonnement) ; endpoints : text-to-3D (preview/refine), image-to-3D, multi-image-to-3D, retexture, remesh, UV unwrap, rigging, animation, convert, resize ; tâches asynchrones avec SSE possible ; URLs de fichiers expirent [OFF] — [Meshy API pricing](https://docs.meshy.ai/en/api/pricing), [Meshy rigging](https://docs.meshy.ai/en/api/rigging-and-animation)
- Meshy a un serveur MCP officiel listé sur sa page d'intégrations (commande `claude mcp add meshy`, clé API depuis Settings > API Keys) et un plugin Blender (« Meshy Bridge ») ; pas de plugin Unreal officiel documenté trouvé (les assets s'importent dans Unreal par fichiers) [OFF/TIERS] — [Meshy intégrations](https://www.meshy.ai/integrations), [Meshy Blender MCP guide](https://www.meshy.ai/tutorials/blender-mcp-guide)
- Blender dispose maintenant d'un serveur MCP officiel (Blender 5.1 minimum) [TIERS via recherche, non vérifié en profondeur] — résultat de recherche ci-dessus
- Rodin/Hyper3D : accessible par API chez fal.ai (`fal-ai/hyper3d/rodin/v2`), Runware, WaveSpeed, EmpirioLabs ; formats GLB (défaut), USDZ, FBX, OBJ, STL ; je n'ai pas atteint la doc officielle Hyper3D [TIERS] — [fal.ai llms.txt](https://fal.ai/models/fal-ai/hyper3d/rodin/v2/llms.txt), [Runware](https://runware.ai/docs/models/hyper3d-rodin-gen-2)
- TRELLIS.2 : GLB avec PBR (WebP) + vidéo MP4 ; dépendances CUDA, nvdiffrast, nvdiffrec, CuMesh [OFF] — [GitHub TRELLIS.2](https://github.com/microsoft/TRELLIS.2)
- Un projet communautaire « Unreal-Blender MCP » étend blender-mcp à Unreal via un serveur Python (port 8500) [COMMUNAUTÉ, non vérifié] — [LobeHub](https://lobehub.com/it/mcp/tahooki-unreal-blender-mcp)

### Inferences
- Pour un agent CLI sous Windows, la voie la plus simple est Tripo (CLI npm + REST) ou Meshy (REST + MCP officiel) ; tout se scripte en Python/PowerShell avec `Invoke-RestMethod` et polling de tâche.
- Un pont direct vers l'éditeur UE (import FBX/GLB auto) est à écrire soi-même (Python Unreal ou Interchange) ; aucun fournisseur ne documente un flux automatisé vers UE5.8.

### Gaps
- Existence d'un MCP Tripo plus récent que l'alpha Blender : non trouvée (la page développeurs ne le mentionne pas).
- Doc officielle API Hyper3D (limites de débit, formats, rigging) non lue.
- Compatibilité de ces API avec UE 5.8 spécifiquement (Interchange GLB/FBX) non documentée.

## 4. Tarifs, niveaux gratuits, licence commerciale, propriété, données d'entraînement

### Takeaway
Pour les trois services SaaS, un plan payant donne les droits commerciaux ; le niveau gratuit est restreint (Meshy : CC BY 4.0 avec attribution, Tripo : droits conservés par Tripo, Rodin : sources contradictoires). Les prix des abonnements grand public varient fortement selon les sites : vérifier sur les pages officielles (certaines étaient bloquées en 403 pour mon outil).

### Cited Findings
- Meshy officiel : plans Free (100 crédits/mois), Pro, Premium, Ultra, Studio, Enterprise ; Free = licence CC BY 4.0 (usage commercial permis avec attribution, l'utilisateur est propriétaire) ; plans payants = licence privée, usage commercial complet, propriété à l'utilisateur ; les échecs ne sont pas facturés ; crédits mensuels remis à zéro le 1er, crédits achetés/permanents n'expirent pas [OFF] — [Meshy pricing](https://docs.meshy.ai/en/webapp/pricing)
- Meshy coûts API (crédits) : image-vers-3D maillage seul 20 (7.1/6) ou 5 (6-lite/t2) ; avec texture 2K/4K 30, 8K 35 ; retexture 10 (15 en 8K) ; rigging 5 ; animation 3/action ; remesh 5 ; UV unwrap 5 ; convert 1 [OFF] — [Meshy API pricing](https://docs.meshy.ai/en/api/pricing)
- Prix en dollars Meshy : Pro 20 USD/mois (facturé annuellement, 1 000 crédits), Studio 60 USD/siège/mois (4 000 crédits) d'après un roundup vérifié sur la page de prix le 2026-05-29 ; d'autres agrégateurs donnent d'autres chiffres (16 / 10 USD) [AGRÉGATEUR, contradictoire] — [StackSheriff](https://stacksheriff.com/ai-tools/ai-3d-model-generator-roundup-meshy-tripo3d-luma-more-tested/)
- Meshy Terms of Use (mis à jour le 2026-03-07) : interdit d'utiliser les assets générés pour entraîner/améliorer des modèles d'IA concurrents de Meshy ; rien trouvé sur l'entraînement de Meshy sur le contenu utilisateur [TIERS-FIABLE] — [Meshy Terms](https://www.meshy.ai/cs/terms-of-use)
- Tripo API : 1 crédit = 0,01 USD ; texte-vers-3D 10 à 20 crédits, image-vers-3D 20 à 30, retopo 10–40, texturer un mesh existant 10–30, multi-vue vers 3D 20–30, Auto Rig 25, retarget 10, conversion de format 5–10, segmentation/complétion 30–50 ; tarifs entreprise sur demande [OFF] — [Tripo developers](https://developers.tripo3d.ai/)
- Tripo abonnement grand public : prix contradictoires. Costbench : Pro 19,90 / Max 89,90 / Team 109,90 USD par mois et une hausse du palier supérieur à 329,70 USD en août 2026 (non confirmée sur Tripo) ; StackSheriff (vérifié 2026-05-29 / 2026-07-14) : Free 200 crédits/mois, Pro 13,93 USD/mois facturé annuellement pour 3 000 crédits, Max 53,94 USD pour 25 000 crédits [AGRÉGATEUR, contradictoire] — [Costbench](https://costbench.com/software/ai-3d-generation/tripo-ai/), [Costbench changelog août 2026](https://costbench.com/changelog/tripo-ai-price-increase-2026-08/), [StackSheriff](https://stacksheriff.com/ai-tools/ai-3d-model-generator-roundup-meshy-tripo3d-luma-more-tested/)
- Tripo licence (pages d'aide Tripo, résumés) : usage commercial permis sur plan payant ; utilisateurs gratuits : Tripo conserve les droits sur entrées et sorties (ToS §5.2.1) ; utilisateurs payants : propriété des entrées/sorties (§5.2.2) ; Tripo n'utiliserait pas les données des payants pour l'entraînement ; Tripo n'assume aucune responsabilité sur le statut de droit d'auteur des sorties [OFF (aide), texte des ToS non lu — la page /terms renvoie 403] — [Tripo : usage commercial](https://www.tripo3d.ai/help/privacy-policy/can-i-use-models-commercially), [Tripo : How to use commercially](https://www.tripo3d.ai/help/privacy-policy/how-to-use-tripo-models-commercially)
- Tripo P2.0 : 2 générations gratuites à l'image unique ; multi-vue et usage continu requièrent un abonnement ; sorties du plan gratuit non licenciées commercialement [TIERS] — [Digital Production](https://digitalproduction.com/2026/09/29/tripo-p2-0-put-quad-mesh-generation-to-the-test/)
- Rodin/Hyper3D : prix officiels non confirmés. Agrégateurs : Creator 24 ou 30 USD/mois, Business 120 USD/mois (208 crédits) ; crédits directs à 1,5 USD ; coût facturé au téléchargement selon un site (contradiction sur le moment de facturation) ; droits commerciaux du plan gratuit contradictoires entre sources [AGRÉGATEUR, contradictoire] — [Costbench](https://www.costbench.com/software/ai-3d-generation/rodin-hyper3d/), [hyper3d.ai/pricing](https://hyper3d.ai/pricing)
- Rodin via API tierce : 0,40 USD par génération standard et 0,80 USD en haute qualité (WaveSpeed) ; 0,80 USD par requête (EmpirioLabs) [TIERS, tarifs de revendeurs] — [WaveSpeed](https://www.wavespeed.ai/docs/docs-api/hyper3d/hyper3d-rodin-v2.5-image-to-3d), [EmpirioLabs](https://empiriolabs.ai/models/hyper3d-gen2)
- Stability (SF3D, SPAR3D) : licence communautaire gratuite sous 1 M USD de revenu annuel ; au-delà, licence entreprise ; SPAR3D : usage commercial, dérivés et sorties relèvent de la demande entreprise selon la carte HF [OFF] — [Stability license](https://stability.ai/license), [Fast.io résumé](https://www.fast.io/resources/stability-ai-review-2026.md)
- Luma : Plus 30 USD/mois (droits commerciaux), Pro 90, Ultra 300 (d'après StackSheriff, vérifié 2026-05-29) [AGRÉGATEUR] — [StackSheriff](https://stacksheriff.com/ai-tools/ai-3d-model-generator-roundup-meshy-tripo3d-luma-more-tested/)
- Hunyuan3D (licence communautaire Tencent, texte HF) : territoire = monde entier **hors Union européenne, Royaume-Uni et Corée du Sud** (usage hors territoire non autorisé) ; au-delà de 1 million d'utilisateurs actifs mensuels il faut demander une licence à Tencent ; interdit d'utiliser les œuvres ou leurs sorties pour améliorer un autre modèle d'IA [OFF] — [LICENSE Hunyuan3D-2 (HF)](https://huggingface.co/tencent/Hunyuan3D-2/blob/main/LICENSE) ; la même exclusion territoriale est signalée pour la 2.1 [TIERS] — [Wavect](https://wavect.io/blog/ai-3d-model-generators-game-development-2026.md)
- TRELLIS.2 : code et modèle sous MIT, mais nvdiffrast et nvdiffrec (NVIDIA) ont leur propre licence, décrite par un site d'actualité comme limitée à la recherche/évaluation (à vérifier) [OFF + TIERS] — [GitHub TRELLIS.2](https://github.com/microsoft/TRELLIS.2), [CGWorld](https://cgworld.jp/flashnews/01-202601-TRELLIS2.html)

### Inferences
- Le développeur est probablement en France/UE : la licence Hunyuan3D 2.x exclut explicitement l'UE, donc elle est inutilisable légalement pour un projet basé dans l'UE (à confirmer par un juriste). C'est un point bloquant majeur pour l'option « open weights de Tencent ».
- Pour la distribution commerciale (Steam), choisir un plan payant avec droits écrits (Meshy Pro+, Tripo payant) ; éviter le plan gratuit (Meshy CC BY exige attribution ; Tripo conserve les droits).
- Coût estimé indicatif : à Tripo API, une génération image-vers-3D + texture + rig reste de l'ordre de quelques dizaines de crédits, soit moins de 1 USD par asset ; à Meshy API, ~30 crédits pour image-vers-3D texturé 2K (le prix d'un crédit API n'a pas été confirmé).
- Statut juridique du droit d'auteur sur les contenus générés par IA (UE/France/US) : hors de portée de ces sources ; les fournisseurs dégagent leur responsabilité.

### Gaps
- Page de prix officielle Tripo et ToS Tripo : HTTP 403, donc non lues directement ; les prix d'abonnement sont contradictoires entre agrégateurs.
- Prix du crédit Meshy API en dollars non confirmé.
- ToS officiels de Hyper3D (propriété, entraînement) non trouvés ; Meshy : politique d'entraînement sur les données utilisateur non trouvée.
- Aucune information sur les conditions d'entraînement pour Luma, CSM.

## 5. Options locales / poids ouverts (Hunyuan3D, TRELLIS, etc.), matériel, Windows

### Takeaway
Hunyuan3D 2.1 est la seule option ouverte avec support Windows déclaré et PBR, mais sa licence exclut l'UE ; TRELLIS.2 (MIT) exige 24 Go de VRAM et n'est testé que sous Linux. Aucune version ouverte de Hunyuan3D 3.0 n'a été confirmée.

### Cited Findings
- Hunyuan3D-2.1 (13 juin 2025) : VRAM ≈ 10 Go (forme), ≈ 21 Go (texture), ≈ 29 Go (les deux) ; Shape 3,3 B et Paint 2 B de paramètres ; PBR ; le dépôt déclare supporter macOS, Windows, Linux ; testé Python 3.10 / PyTorch 2.5.1+cu124 [OFF] — [GitHub Hunyuan3D-2.1](https://github.com/Tencent-Hunyuan/Hunyuan3D-2.1)
- Hunyuan3D-2 version portable Windows communautaire : ≥ 6 Go VRAM (forme seule), ≥ 8 Go (texture) ; variantes basse VRAM (Hunyuan3D-2GP) [COMMUNAUTÉ] — résultat de recherche (Hunyuan3D-2-WinPortable, awesome.ecosyste.ms)
- Retour pratique (RTX 3060) : 8–10 Go pour les modèles turbo minimaux, 12–16 Go pour image unique, 24 Go+ pour multi-vue ; RAM système 2–3× la VRAM ; exemple ≈ 10 min par asset à réglages bas ; si les modules CUDA personnalisés échouent à compiler, le calcul retombe sur CPU (heures à jours) ; installation Windows pénible (nvcc, CUDA_HOME, `triton-windows`) [TIERS, un auteur] — [tspi.at](https://www.tspi.at/2026/05/01/opensource3dassets.html)
- TRELLIS.2 : GPU NVIDIA ≥ 24 Go « nécessaire », testé A100/H100 ; CUDA 12.4 recommandé ; Linux seulement testé ; des wrappers ComfyUI communautaires revendiquent 8 Go (non vérifié) ; chiffres de VRAM divergents selon les sites (16/24/40/80 Go selon la résolution) [OFF + AGRÉGATEUR] — [GitHub TRELLIS.2](https://github.com/microsoft/TRELLIS.2), [CodeSota](https://www.codesota.com/news/trellis-2-3d-generation), [CGWorld](https://cgworld.jp/flashnews/01-202601-TRELLIS2.html)
- Hunyuan3D 3.0 : lancé en septembre 2025 comme produit hébergé avec API ; aucune confirmation de poids ouverts ; une annonce de « Hunyuan3D 2.5 poids ouverts » vient d'un post Reddit de janvier 2026 non vérifié ; « Hunyuan3D-Omni » mentionné, non confirmé [TIERS/NON VÉRIFIÉ] — [Pandaily](https://pandaily.com/tencent-unveils-hunyuan-3d-3-0-ai-model-tripling-modeling-accuracy-with-free-access), [Notebookcheck](https://www.notebookcheck.net/Tencent-s-new-Hunyuan-3D-3-0-brings-realistic-AI-model-generation-to-developers-and-creators.1116964.0.html)
- Autres modèles ouverts : Step1X-3D (StepFun, 2025, géométrie + texture, entraînement inclus) ; Seed3D (ByteDance) via API fal.ai (source secondaire) ; SPAR3D / SF3D (Stability, génération rapide sur GPU grand public, licence communautaire < 1 M USD) ; SAM 3D Objects (Meta, reconstruction de recherche, résolution modérée) [TIERS] — [Step1X-3D (arXiv)](https://ar5iv.labs.arxiv.org/html/2505.07747), [Wavect](https://wavect.io/blog/ai-3d-model-generators-game-development-2026.md), [Stability license](https://stability.ai/license)

### Inferences
- Une carte avec 24 Go (type RTX 3090/4090/5090) ouvre TRELLIS.2 en local sous WSL2/Linux ; pour moins de 16 Go, Hunyuan3D-2/2.1 en forme seule avec texture externe est plus réaliste.
- Le coût des modèles locaux est le temps de mise en place et de nettoyage ; pour un solo dev avec un agent CLI, l'API payante est plus simple, sauf si le volume justifie le local.
- Pour la licence commerciale propre depuis l'UE, TRELLIS.2 (MIT, sous réserve des dépendances NVIDIA) est la seule option ouverte sans exclusion territoriale parmi celles étudiées.

### Gaps
- Matériel du développeur inconnu (pas dans ces sources).
- État réel de Hunyuan3D 2.5/3.x/Omni en poids ouverts : non vérifié sur Hugging Face/GitHub.
- Licences exactes de SF3D/SPAR3D pour les sorties, de Step1X-3D, de Seed3D : non vérifiées.
- Compatibilité Windows native de TRELLIS.2 : seulement via ports communautaires non audités.

## 6. Cohérence de style (image-vers-3D, références, multi-vue)

### Takeaway
Les trois grands services acceptent l'entrée image et multi-image/multi-vue, ce qui est le levier principal pour la cohérence ; aucune source n'évalue la cohérence de style entre générations successives.

### Cited Findings
- Tripo : image-vers-multivue (10 crédits), édition multivue (5 crédits par image), multivue-vers-3D (20–30 crédits) [OFF] — [Tripo developers](https://developers.tripo3d.ai/)
- Meshy : multi-image-vers-3D (20 à 35 crédits selon texture) disponible sur 7.1, 6 et 6-lite, mais pas t2/t2.5 ; génération d'images intégrée (nano-banana, GPT Image) pour créer des références avant la 3D ; texte-vers-3D non disponible avec Meshy 7.1 [OFF] — [Meshy API pricing](https://docs.meshy.ai/en/api/pricing), [Meshy pricing webapp](https://docs.meshy.ai/en/webapp/pricing)
- Hunyuan3D en local : entrée multi-vue nécessite 24 Go+ ; les erreurs de géométrie ne peuvent pas être corrigées au texturage, donc entrées multi-vue et réglages soignés importants ; le modèle devine les faces cachées [TIERS] — [tspi.at](https://www.tspi.at/2026/05/01/opensource3dassets.html)
- Wavect : tester la « cohérence au sein d'un ensemble » comme limitation à vérifier pour Tripo ; vocabulaire de modèles paramétriques de Sloyd pouvant ne pas correspondre à la direction artistique [TIERS] — [Wavect](https://wavect.io/blog/ai-3d-model-generators-game-development-2026.md)
- Tripo P2.0 : multi-vue laisse des zones occultées ; séparation automatique des parties pouvant poser des frontières mal placées [TIERS] — [Digital Production](https://digitalproduction.com/2026/09/29/tripo-p2-0-put-quad-mesh-generation-to-the-test/)

### Inferences
- Flux de cohérence recommandé (hypothèse à tester) : produire d'abord des images de concept dans un style unique (générateur d'images + prompt de style fixe), puis les passer en image-vers-3D ou multi-vue, et retexturer avec une même passe de texture (retexture Meshy / AI Texture Tripo) pour uniformiser.
- Un style « RTS vu de haut » tolère mieux les imperfections de détail que des assets héros.

### Gaps
- Aucun test de cohérence de style inter-génération trouvé pour aucun outil.
- Pas de données sur les LoRA/fine-tuning de style côté services 3D.

## 7. Limites connues et retours de développeurs ; nouveaux entrants 2026

### Takeaway
Le consensus des guides 2026 est que l'IA 3D accélère le prototypage et les props statiques mais ne livre pas des assets d'expédition sans retopo/UV/rig manuels ; les prix et licences changent vite et sont mal documentés. Les retours directs de développeurs (Reddit/Discord) n'ont pas pu être collectés.

### Cited Findings
- Guides de comparaison 2026 : « les maillages générés nécessitent souvent un nettoyage avant d'être prêts pour le jeu » ; fixer budget de polys et moteur d'abord [AGRÉGATEUR] — [Wavect](https://wavect.io/blog/ai-3d-model-generators-game-development-2026.md), résumé de recherche (Krea, RapidDirect, IndieHackers)
- Tripo a levé 446 millions de dollars (3 milliards de yuans) et lancé P2.0 en septembre 2026 ; revendication marketing « premier modèle 3D avec topologie quad native », contestée comme non démontrée de façon indépendante [MARKETING + TIERS] — [3DPrintingIndustry](https://3dprintingindustry.com/news/tripo-ai-lands-3-billion-yuan-unveils-quad-topology-model-254576/), [RuntimeWire](https://runtimewire.com/article/tripo-ai-p2-native-quad-meshes-3d-generation)
- Meshy : rigging humanoïde uniquement ; sorties de rigging plafonnées à 380 000 triangles ; liens de fichiers expirants (à télécharger rapidement) [OFF] — [Meshy rigging](https://docs.meshy.ai/en/api/rigging-and-animation)
- Hyper3D/Rodin : sources contradictoires sur le moment où les crédits sont consommés (génération vs téléchargement) et sur les droits de la formule gratuite [AGRÉGATEUR] — [Costbench](https://costbench.com/software/ai-3d-generation/rodin-hyper3d/hidden-costs/)
- Autres entrants cités par les guides : Autodesk Wonder 3D, Sloyd (paramétrique, topologie propre, UV), SAM 3D Objects (Meta), Seed3D (ByteDance), Step1X-3D, 3D AI Studio (agrégateur donnant accès à Meshy/Rodin/Tripo) [TIERS/AGRÉGATEUR] — [Wavect](https://wavect.io/blog/ai-3d-model-generators-game-development-2026.md), [3D AI Studio](https://www.3daistudio.com/3d-generator-ai-comparison-alternatives-guide/best-text-to-3d-generators-2026)
- Luma Genie et CSM : aucune information de 2026 trouvée (ni arrêt ni mise à jour) ; Luma reste décrit comme splat/NeRF peu adapté au jeu [AGRÉGATEUR] — [StackSheriff](https://stacksheriff.com/ai-tools/ai-3d-model-generator-roundup-meshy-tripo3d-luma-more-tested/)

### Inferences
- Le seul vrai critère de choix pour ce projet est un pilote : 3–5 assets médiévaux (maison, tour, rocher, arbre, fantassin) passés dans Tripo (P2.0/H3.1), Meshy 7.1 et Rodin Gen-2.5, comparés en triangles, UV, PBR et temps de nettoyage dans UE5.8.
- Étant donné les contradictions de prix, budgétiser d'abord une enveloppe API de l'ordre de dizaines de dollars pour le pilote et relire les ToS officiels avant d'expédier.

### Gaps
- Pas de retours directs Reddit/Discord/forums de développeurs (l'outil de recherche n'a rien ramené de primaire ; tout est de seconde main).
- SF3D/CSM/Luma Genie : état 2026 et conditions non confirmés.
- Aucune comparaison indépendante chiffrée (triangles, temps de nettoyage) pour des assets médiévaux ou de RTS.
