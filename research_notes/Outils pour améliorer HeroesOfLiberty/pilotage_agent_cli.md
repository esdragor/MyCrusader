# Pilotage d'une chaîne d'art 3D / Unreal 5.8 par un agent CLI (Claude Code, Windows, PowerShell) — état au 2026-11-03

Légende de fiabilité : [H] source primaire (doc officielle, README, page PyPI) ; [M] source secondaire cohérente ; [B] agrégateur / revendeur / non vérifié. Les dates de dernier commit n'ont presque jamais pu être lues dans les pages récupérées : c'est signalé en « Gaps ».

## Blender : MCP, bpy headless, retopologie, LOD, export (Blender 4.x/5.x)

### Takeaway
Le serveur MCP Blender dominant (ahujasid, renommé « mcp-for-blender », ~33 k étoiles) existe et intègre déjà Rodin, Tripo, Hunyuan3D et Meshy, mais exécute du Python arbitraire sans authentification. Pour un pipeline automatisé fiable, le plus robuste reste `blender -b --python` en scripts (non couvert par une source dédiée dans cette recherche, voir Gaps) ; Blender 5.x casse l'ancienne API d'actions.

### Cited Findings
- Dépôt `ahujasid/mcp-for-blender` (ex-`blender-mcp`) : 33,2 k étoiles, 3,0 k forks, 259 commits ; Blender 3.0+ et Python 3.10+ ; l'ancien paquet `blender-mcp` reste compatible sans changer la config. [H] — [GitHub](https://github.com/ahujasid/blender-mcp)
- Un wiki daté du 30 oct. 2026 donne 32 730 étoiles et la version PyPI 2.2.6 du 28 oct. 2026 (non confirmé par une page de release GitHub) [B] — [aiwiki / recherche](https://aiwiki.ai/wiki/blender_mcp)
- Outil `generate_3d` couvrant Hyper3D Rodin, Tripo, Hunyuan3D (Tencent Cloud, comptes Chine/international distincts) et Meshy ; Sketchfab (`search_assets`/`import_asset`) et Poly Haven (CC0, sans clé). [H] — [GitHub](https://github.com/ahujasid/blender-mcp)
- Sécurité : `execute_blender_code` exécute du Python arbitraire ; le socket de l'addon n'a ni authentification ni chiffrement (rester en localhost) ; `BLENDER_MCP_SAFE_MODE=1` existe mais est désactivé par défaut ; télémétrie minimale anonyme par défaut sauf `DISABLE_TELEMETRY=true`. [H] — [GitHub](https://github.com/ahujasid/blender-mcp)
- Le MCP « officiel » de Tripo (VAST-AI-Research/tripo-mcp) : 225 étoiles, 8 commits, « alpha », ne supporte que l'addon Blender Tripo, aucune mention d'Unreal. [H] — [GitHub](https://github.com/VAST-AI-Research/tripo-mcp)
- Blender 5.0 : l'ancienne API d'actions est supprimée (actions à slots), principal changement cassant pour les scripts ; l'importeur FBX C++ devient celui par défaut ; Collada retiré. [B, guide tiers à vérifier] — [Cinevva](https://app.cinevva.com/guides/blender-5-for-game-artists)
- Le même guide indique : Python 3.13 à partir de Blender 5.1 (les add-ons avec wheels compilées doivent être en cp313) et compression meshopt (EXT/KHR_meshopt_compression) dans l'exporteur glTF en 5.2. [B] — [Cinevva](https://app.cinevva.com/guides/blender-5-for-game-artists)
- Doc Python officielle Blender 5.0 avec journal des changements : [H] — [docs.blender.org](https://docs.blender.org/api/5.0/index.html) (page non ouverte, existence seule confirmée par la recherche)
- Retopologie : QuadWild-BiMDF (fork de QuadWild avec solveur Bi-MDF, build CMake, binaires précompilés Linux/Windows/macOS dans les Releases, licence GPL3, appel en ligne de commande : .obj + argument numérique + fichier de config). [M] — [Ecosyste.ms](https://awesome.ecosyste.ms/projects/github.com%2Fcgg-bern%2Fquadwild-bimdf)
- Instant Meshes : outil essentiellement GUI (boutons : champ d'orientation, champ de position, export), build Windows via Visual Studio ; une source note un dernier push le 3 janv. 2022 ; la copie intégrée à Blender est archivée. [M] — [jonike/instant-meshes](https://github.com/jonike/instant-meshes) ; [Blender archive](https://projects.blender.org/archive/blender-archive/src/commit/e4b6180c4eaedabb3e0e5d84a26b21f40b770d7e/intern/instant-meshes)

### Inferences
- Pour Claude Code sous PowerShell, deux modes : (a) MCP Blender interactif (utile pour l'exploration, risqué car code arbitraire sans auth), (b) scripts `bpy` lancés en `blender -b --python script.py -- args` (reproductibles, versionnables dans le dépôt). (b) est préférable pour un pipeline batch ; hypothèse non testée ici.
- QuadWild-BiMDF est le seul outil de retopo quad de la liste utilisable sans GUI ; la qualité « prête pour le jeu » (boucles d'arêtes, UV) n'est pas documentée dans les sources trouvées. Pour des props décoratifs en Nanite, la retopo est souvent inutile : Nanite accepte des maillages denses (voir section UE).
- Épingler la version de Blender du pipeline : les scripts écrits pour 4.x peuvent casser en 5.x (actions, importeur FBX).

### Gaps
- Pas de source primaire lue sur : décimation `bpy` (modificateur Decimate), UV Smart Project en headless, génération de LOD dans Blender, options des exporteurs FBX/glTF en mode background. Dernier numéro de version stable de Blender 5.x au 2026-11-03 non confirmé. Dates de dernier commit de mcp-for-blender non lues.

## Unreal Engine 5.8 : MCP, scripting Python, Interchange, LOD/Nanite, limites du headless

### Takeaway
UE 5.8 (annoncé le 17 juin 2026) embarque un plugin MCP officiel expérimental ; plusieurs serveurs MCP communautaires existent (ChiR24 déclare UE 5.0 à 5.8). Le point faible reste l'import headless : Interchange en commandlet est mal documenté et historiquement instable ; l'import via `AssetImportTask` depuis l'éditeur lancé (avec ou sans `-nullrhi`) est la voie sûre à tester.

### Cited Findings
- UE 5.8 présenté le 17 juin 2026 ; contient un « Unreal MCP Server Plugin » expérimental (passerelle LLM vers les projets UE : création d'assets, tests, optimisation). [M] — [GameFromScratch](https://gamefromscratch.com/unreal-engine-5-8-released/) ; [Forum Epic](https://forums.unrealengine.com/t/unreal-engine-5-8-released/2729274) ; notes officielles : [Epic](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5-8-release-notes) (non lues en détail)
- Procedural Vegetation Editor « Nanite-ready » en 5.8 ; MegaLights production-ready ; Lumen Lite. [M] — [GameFromScratch](https://gamefromscratch.com/unreal-engine-5-8-released/)
- `ChiR24/Unreal_mcp` (« EngineWire ») : ~1,0 k étoiles, 185 forks, 3 011 commits sur `dev`, v0.6.0 stable (Fab, Releases, npm), MIT ; UE 5.0 à 5.8 ; build source nécessite un projet C++, sinon Fab/binaires précompilés (liés à une version mineure du moteur) ; exécution Python via `system_control` (plugin Python Editor Script requis) ; LOD via `manage_geometry` ; pas de mention de Nanite ; import d'assets via `manage_asset` ; Interchange optionnel. [H] — [GitHub](https://github.com/ChiR24/Unreal_mcp)
- `uemcp` (ATDev) 0.1.0, publié le 12 juin 2026, bêta, MIT : aucun plugin C++ ; nécessite Python Editor Script Plugin + « Enable Remote Execution » ; annoncé pour UE 5.0 à 5.6 ; ~31 outils (acteurs, assets dont import, matériaux, Blueprints, niveaux, captures, PIE) + outil `ue_python` donnant tout le module `unreal` ; découverte UDP multicast 239.0.0.1:6766 + TCP ; le protocole de remote execution n'a pas d'authentification. Installation : `claude mcp add unreal -- uvx uemcp`. [H] — [PyPI](https://pypi.org/project/uemcp/)
- Autres : `aadeshrao123/UnrealMCP` (plugin C++ TCP + CLI/serveur Python), `ClaudeUnreal` d'echoulen (UE 5.7+, via Fab, écrit `.mcp.json`), `unrealclaude-mcp-bridge` (Natfii, Node 18+). [B, listings non vérifiés] — [PyPI unrealmcp](https://pypi.org/project/unrealmcp/) ; [echoulen](https://echoulen.github.io/claude-unreal/docs/) ; [LobeHub](https://lobehub.com/de/mcp/natfii-unrealclaude-mcp-bridge)
- Interchange : le framework choisit Interchange si le format est supporté, sinon retombe sur l'ancien système ; FBX Interchange reste « Experimental » (`Interchange.FeatureFlags.Import.FBX`) dans la doc lue ; glTF supporté ; pipelines personnalisables en Blueprint, C++ ou Python (Python enregistré via Startup Scripts des Project Settings) ; l'exemple Python choisit le pipeline selon l'extension .glb/.gltf/.fbx/.usd. [H, version de doc non précisée] — [Epic Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/importing-assets-using-interchange-in-unreal-engine)
- Headless : un fil du forum Epic (2022) rapporte qu'un script d'import Interchange plante en commandlet et que les réglages Nanite du pipeline sont ignorés ; contournement : `AssetImportTask`, qui utilise la pile de pipelines Interchange par défaut. Ancien, pas garanti en 5.8. [M] — [Forum Epic](https://forums.unrealengine.com/t/interchange-with-python-in-an-unreal-commandlet/724404)
- Un message de forum affirme que l'assemblage Nanite est importable via le plugin Interchange OpenUSD en 5.8 (non vérifié dans les notes officielles). [B] — [Forum Epic](https://forums.unrealengine.com/t/nanite-assembly-usd-stage-import-python-script/2706947)

### Inferences
- Avec un fork Hazelight (Angelscript, UE 5.8.x), les plugins MCP binaires précompilés (liés à une version mineure et à la plateforme) risquent de ne pas se charger ; la voie à plus faible risque est `uemcp`-like (Python remote execution, sans plugin) ou des scripts Python lancés via `UnrealEditor-Cmd -ExecutePythonScript` — compatibilité avec le fork non vérifiée.
- Le plugin MCP officiel Epic étant expérimental, ne pas bâtir le pipeline dessus ; garder un script Python d'import (`unreal.AssetImportTask`) qui fonctionne sans MCP, et utiliser le MCP seulement pour l'itération.
- Nanite se règle sur le StaticMesh importé (`nanite_settings.enabled`) : à confirmer par un test, aucune source lue ne le documente pour 5.8 (voir Gaps).

### Gaps
- Aucune source officielle lue sur : réglage Nanite/LOD par pipeline Interchange en 5.8, comportement exact d'Interchange en commandlet en 5.8, Datasmith en headless, compatibilité du fork Hazelight avec les plugins MCP, plugin MCP officiel (nom exact du plugin, liste d'outils, activation). Notes de version 5.8 officielles non lues dans le détail. Dernière activité de uemcp/ChiR24 non datée (hors date de publication PyPI 12 juin 2026).

## CLIs de traitement de maillage (MeshLab, gltf-transform, gltfpack, Simplygon, InstaLOD)

### Takeaway
gltf-transform et gltfpack sont gratuits, scriptables et adaptés au nettoyage/compression de GLB (mais attention : la compression Draco/meshopt/KTX2 n'est pas lisible par l'import UE standard). Simplygon est hors de portée (≈30 k$/an), InstaLOD « Pioneer » est gratuit sous 100 k$ de revenus mais avec attribution et un plugin UE ≥ 5.4.

### Cited Findings
- glTF-Transform : ~2,1 k étoiles ; CLI `optimize`, `draco`, `meshopt`, `resize`, `webp`, `uastc`, `etc1s` ; `simplify` non mentionné sur la page lue. [H] — [GitHub](https://github.com/donmccurdy/glTF-Transform)
- meshoptimizer (zeux) : ~8,8 k étoiles, MIT, v1.3 d'après les instructions de clonage ; `gltfpack` = outil CLI d'optimisation glTF avec binaires précompilés (Releases ou npm). Les drapeaux de simplification (-si, -sa) n'ont pas été lus (page tronquée). [H] — [GitHub](https://github.com/zeux/meshoptimizer)
- Simplygon : l'offre gratuite a été supprimée (rapport 2024 : 35 000 $/an) ; sites d'avis 2026 : départ ≈ 30 000 $/an, évaluation 30 jours. Chiffres contradictoires selon les sources. [B] — [TechRaptor](https://techraptor.net/gaming/news/simplygon-free-tier-abolished) ; [Capterra](https://www.capterra.ie/software/1027207/simplygon)
- InstaLOD 2026 (annoncé fév. 2026, mise à jour de juil. 2026 avec remaillage quad) : édition Pioneer incluant InstaLOD Studio et les intégrations Unity/Unreal, gratuite sous 100 000 $/an de revenus, usage commercial avec attribution ; abonnements non-jeu 1 188–5 988 $/an, licences jeux à partir de 11 880 $/an ; intégration UE ≥ 5.4 ; InstaLOD Pipeline = ligne de commande/traitement par lots. [H/M] — [CG Channel](https://www.cgchannel.com/2026/07/instalod-releases-instalod-2026/) ; [Digital Production](https://digitalproduction.com/2026/02/16/instalod-2026-targets-vr-and-animation-pipelines/)
- Une page communautaire liste un plugin InstaLOD « UE 5.2 et 5.7 » ; un ancien message disait que Pioneer ne couvrait pas les intégrations — contradictoire avec l'annonce 2026. [B] — [Abstract3D](https://community.abstract3d.com/t/instalod-plugin-for-ue5-2-and-ue5-7/1778)

### Inferences
- Pour un projet solo/indé, la combinaison Blender (décimation) + Nanite (pas de LOD manuels) + gltf-transform pour nettoyage évite Simplygon et InstaLOD. InstaLOD Pioneer est une option gratuite à vérifier (UE 5.8 non listé : plugin annoncé jusqu'à 5.7 dans une source).
- Le pipeline UE doit recevoir un GLB/FBX non compressé en Draco/meshopt, sauf si l'import UE les décode ; non vérifié.

### Gaps
- MeshLab/meshlabserver/PyMeshLab : non recherchés/lus. Flags de simplification de gltfpack et commande `simplify` de gltf-transform non confirmés. Pas de tarif Simplygon officiel lu. Compatibilité InstaLOD avec UE 5.8 et avec un fork Hazelight non confirmée.

## Outils de textures (ImageMagick, Substance, upscalers, IA image)

### Takeaway
Real-ESRGAN dispose d'un exécutable Windows portable sans CUDA ni PyTorch ; Substance dispose d'un Automation Toolkit (Python + outils batch) mais la licence Adobe et l'état en 2026 n'ont pas pu être confirmés. ImageMagick n'a pas été étudié.

### Cited Findings
- Real-ESRGAN : ~37,3 k étoiles, licence BSD-3-Clause ; build Windows `realesrgan-ncnn-vulkan-20220424-windows.zip` (v0.2.5.0, Intel/AMD/Nvidia, sans CUDA/PyTorch) ; usage `realesrgan-ncnn-vulkan.exe -i in.jpg -o out.png -n modèle` (options -s 2–4, -t, -g, -f) ; modèles x4plus, x4plus-anime, animevideov3 ; ne gère pas tout ce que fait le script Python (ex. `outscale`). [H] — [GitHub](https://github.com/xinntao/Real-ESRGAN)
- Substance 3D Automation Toolkit : paquet Python (Pysbs) + outils batch (`sbsrender`, `sbscooker`, `sbsbaker` renommé `substance3d_baker`) ; dernières notes de version lues : 15.0.1 / 2025.0.1 (juil. 2025) avec rupture de format des paramètres JSON du baker ; moteur Vulkan sur serveur GPU sans écran ajouté dans une version antérieure. [H] — [Adobe SAT](https://helpx.adobe.com/substance-3d-sat/release-notes.html) ; [demos_batchtools](https://helpx.adobe.com/substance-3d-sat/pysbs-python-api/demo-overview/demos-batchtools.html)
- Meshy propose retexture (API) et text-to-image / image-to-image ; Tripo propose AI Texture (10–30 crédits). [H] — [Meshy docs](https://docs.meshy.ai/llms.txt) ; [Tripo dev](https://developers.tripo3d.ai/)

### Inferences
- Pour des textures de props, les générateurs 3D fournissent déjà des PBR (Tripo PBR désactivé par défaut depuis tripo-cli 0.9.0 : `-p pbr=true`) ; l'upscale Real-ESRGAN peut servir en post-traitement (risque d'artéfacts sur des normal maps : à ne pas upscaler comme une image couleur).

### Gaps
- ImageMagick (version, usage), Substance Sampler CLI, licences Adobe 2026, et outils IA image avec CLI (hors Meshy/Tripo) non couverts.

## Services IA 3D : Tripo, Meshy, Rodin, Hunyuan3D (CLI, SDK, MCP, REST, webhooks, auth, limites)

### Takeaway
Tripo a bien une CLI officielle (`tripo-cli`, npm) avec mode agent, MCP intégré (`tripo mcp`) et Agent Skill ; pas de SDK Python officiel trouvé. Meshy a un MCP officiel et un skill pack mais ni SDK ni CLI. Rodin : API REST asynchrone, doc incomplète récupérée. Hunyuan3D : modèle ouvert local (licence excluant UE/UK/Corée du Sud) ou API Tencent Cloud.

### Cited Findings
**Tripo**
- CLI officielle : `npm install -g tripo-cli` (Node 20+), commandes `tripo make "<prompt>"`, `tripo model|anim|mesh` (texture, rig, convert, decimate, UV), `tripo batch run` (reprise possible), `tripo task get/list/watch` ; options agent `--json --yes --quiet --no-open --no-wait`, `--dry-run` (coût estimé) ; codes de sortie 0 à 9 (3 = auth, 4 = crédits, 6 = échec/timeout, 9 = limite de débit) ; seul HTTP 429 est réessayé automatiquement. [H] — [Doc CLI Tripo](https://developers.tripo3d.com/en/docs/cli.md)
- Auth : `tripo login --region ov` (international, Stripe) ou `--region cn` ; clé `tsk_...` ou variables `TRIPO_API_KEY` / `TRIPO_REGION`. [H] — [Doc CLI Tripo](https://developers.tripo3d.com/en/docs/cli.md)
- Intégration agent : Agent Skill embarqué (`tripo docs --llm`), `tripo mcp` lance la CLI en serveur MCP (`--toolset full` à partir de 0.9.0), plugin Codex séparé. [H] — [Doc CLI Tripo](https://developers.tripo3d.com/en/docs/cli.md)
- Formats/qualité : conversion `--then convert:fbx` (GLTF, OBJ, STL aussi) ; low poly sélectionne `tripo-p2` ; quad avec `-p quad=true` (P2, sortie FBX) ; presets `--for game-pc|game-mobile|...`. [H] — [Doc CLI Tripo](https://developers.tripo3d.com/en/docs/cli.md)
- API : REST, Bearer token ; 1 crédit = 0,01 $ ; text-to-3D 10–20 crédits, image/multivue 20–30, texture 10–30, conversion 5–10, retopologie 10–40, auto-rig 25 ; modèles H3.1 et P2.0 (topologie quad native), requête exemple `v3.1-20260211` ; webhooks supportés (« pushed when tasks complete ») ; quota de concurrence par type de tâche, sans chiffres. SDK Python officiel non mentionné ; les exemples utilisent `requests`. [H] — [Tripo developers](https://developers.tripo3d.ai/)
- Un serveur MCP tiers (`pasie15/tripo-ai-mcp-server`) existe, docs contradictoires sur la variable (TRIPO_API_KEY vs TRIPO_API_SECRET). [B] — [LobeHub](https://lobehub.com/zh/mcp/pasie15-tripo-ai-mcp-server) ; [Glama](https://glama.ai/mcp/servers/@pasie15/tripo-ai-mcp-server)
- Deux paquets CLI semblent exister (`tripo-cli` et `@tripo3d/cli` Go, dépôt vast-enterprise/tripo-cli, jusqu'à v0.5.1 sur pkg.go.dev) ; relation non vérifiée. [B] — [pkg.go.dev](https://pkg.go.dev/github.com/vast-enterprise/tripo-cli)

**Meshy**
- Authentification Bearer, clés `msy_`, clé de test existante ; REST (OpenAPI YAML/JSON publié) ; webhooks (5 actifs, parfois 10 au total selon la version de la page ; HTTPS obligatoire ; désactivation après 20 échecs consécutifs). [H] — [Meshy llms.txt](https://docs.meshy.ai/llms.txt) ; [Webhooks](https://docs.meshy.ai/api/webhooks.md)
- Limites : Pro 20 req/s et 10 tâches simultanées ; Premium 30 ; Ultra 100 ; Studio 60 (20 sur d'anciens plans) ; Enterprise 100 req/s et 100 tâches ; 429 avec `Retry-After` pour le débit, sans pour la concurrence. Pas de palier gratuit listé. [H] — [Rate limits](https://docs.meshy.ai/api/rate-limits.md)
- MCP officiel : `npx -y @meshy-ai/meshy-mcp-server`, clé dans `MESHY_API_KEY`, outils text/image/multi-image-to-3D, remesh, retexture, rig, animate, convert, resize, UV unwrap, téléchargement ; mêmes crédits et limites que le REST ; skill pack `meshy-3d-agent` (`npx skills add meshy-dev/meshy-3d-agent`) ; aucun SDK ni CLI officiel mentionné. [H] — [Meshy AI integration](https://docs.meshy.ai/api/ai.md)
- Dépréciations : le modèle `lowpoly` est retiré le 30 oct. 2026 (bandeau de la doc) — vérifier la page des dépréciations avant d'écrire un script. [M] — [Meshy webhooks (bandeau)](https://docs.meshy.ai/en/api/webhooks)
- Post-traitement API : Remesh, Convert, Resize, UV Unwrap (5 crédits), Retexture, Rigging. [H] — [Meshy llms.txt](https://docs.meshy.ai/llms.txt)

**Rodin (Hyper3D)**
- API HTTP v2 : base `https://api.hyper3d.com/api/v2`, Bearer, multipart/form-data, JSON, génération asynchrone (usage de `jobs.subscription_key` pour le suivi) ; la doc récupérée ne couvre pas limites, formats, webhooks, SDK. [H] — [Doc Rodin](https://docs.hyper3d.ai/)
- Revendeurs : EmpirioLabs facture Gen-2 à 0,80 $/requête ; une page tierce mentionne un accès API lié à un plan payant (Business 120 $/mois) et Rodin Gen-2.5. [B] — [EmpirioLabs](https://empiriolabs.ai/models/hyper3d-gen2) ; [AIXploria](https://www.aixploria.com/?p=51918)

**Hunyuan3D / Tencent**
- Open source : Hunyuan3D-2.1, 4,3 k étoiles, API locale incluse (`api_server.py`, docker, Gradio), fonctionne sous Windows/Linux/macOS, testé Python 3.10 / PyTorch 2.5.1. [H] — [GitHub](https://github.com/Tencent-Hunyuan/Hunyuan3D-2.1)
- Licence : territoire = monde entier sauf UE, Royaume-Uni et Corée du Sud ; au-delà de 1 M d'utilisateurs actifs mensuels, licence séparée. (Le texte exact de la 2.1 n'a pas été lu, la source cite les licences 2.0/2.1.) [M] — [Licence 2.0 (Wind River)](https://open.windriver.com/info/uni-license-list/licenses/tencent-hunyuan-3d-2.0-cla.html) ; [licence 2.1](https://gitserver.onethingai.com/ai-models/Hunyuan3D-2.1/raw/branch/main/LICENSE)
- Cloud : API Hunyuan 3D ouverte aux entreprises hors Chine via Tencent Cloud (200 crédits gratuits), domaine international `hunyuan.intl.tencentcloudapi.com`, version 3.0 ; offre grand public avec 20 générations gratuites par jour. Prix par appel non trouvés. [M] — [Tencent Cloud](https://www.tencentcloud.com/ko/document/product/1284/75533) ; [Verdict](https://www.verdict.co.uk/tencent-launches-hunyuan-3d-engine/)

### Inferences
- La combinaison la plus directe pour Claude Code : `tripo-cli` (scriptable, `--json`, `batch run`) ou le MCP Meshy ; les deux évitent d'écrire un client REST. Meshy est le plus riche en post-traitement (remesh, UV, convert) via API.
- La licence Hunyuan3D exclut l'UE : si le projet ou le développeur est en UE, l'usage local ne serait pas couvert (à vérifier juridiquement ; l'utilisateur écrit en français, ce qui suggère une localisation possible en UE — hypothèse).
- Les limites de débit sont documentées par Meshy, pas par Tripo ni Rodin : prévoir une file d'attente avec retry/backoff dans le script.

### Gaps
- Limites de débit de Tripo et Rodin, tarifs Rodin officiels, webhooks Rodin, tarifs Hunyuan cloud, existence éventuelle d'un SDK Python officiel Tripo (non trouvé, pas prouvé inexistant), prix des plans mensuels Tripo/Meshy.

## Pipeline de bout en bout (prompt → StaticMesh Nanite dans UE) et points de défaillance

### Takeaway
Une chaîne réaliste existe avec des briques décrites ci-dessus, mais aucune source lue ne documente un pipeline complet validé de bout en bout en 5.8 ; les étapes sont des inférences à tester.

### Cited Findings
- Les générateurs sortent GLB/FBX/OBJ/USDZ (Tripo) ou via convert ; Meshy propose remesh/convert/UV unwrap en API. [H] — [Tripo](https://developers.tripo3d.ai/) ; [Meshy](https://docs.meshy.ai/llms.txt)
- Tripo P2.0 vise la topologie quad native pour jeux ; Tripo CLI a un preset `game-pc`. [H] — [Tripo CLI](https://developers.tripo3d.com/en/docs/cli.md)
- L'import UE : Interchange (glTF supporté, pipelines Python) ou `AssetImportTask`. [H/M] — [Epic Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/importing-assets-using-interchange-in-unreal-engine) ; [Forum Epic](https://forums.unrealengine.com/t/interchange-with-python-in-an-unreal-commandlet/724404)

### Inferences
Chaîne proposée (à valider par un prototype d'un seul asset) :
1. `tripo make "…" --for game-pc --then convert:fbx --json` (ou MCP Meshy), sortie dans un dossier de travail du dépôt. Défaillances : crédits (code 4), limite de débit (code 9), échec de tâche (code 6), résultat non conforme au style, rétention limitée des fichiers côté service (Meshy a une page « Asset Retention » : télécharger immédiatement).
2. Nettoyage Blender en headless (`blender -b --python`) : échelle en cm (UE) et axes, normalisation de l'origine, fusion des sommets, décimation éventuelle, UV de secours, export FBX/GLB. Défaillances : API Blender 5.x modifiée, maillages non étanches/non-manifold, UV de mauvaise qualité, matériaux multiples, échelle.
3. Passage optionnel gltf-transform/gltfpack. Défaillance : compression Draco/meshopt/KTX2 non décodée à l'import UE (à tester).
4. Import UE par script Python (`AssetImportTask`, `UnrealEditor-Cmd` ou `uemcp`/`ue_python`), puis `static_mesh.nanite_settings.enabled = True`, collision, matériaux, sauvegarde. Défaillances : Interchange instable en commandlet, réglages de pipeline ignorés en headless (fil 2022), éditeur verrouillé par une compilation d'une autre copie (règle projet : voir mémoire « Démarrage bloqué par une compilation »), compatibilité du fork Hazelight, Nanite inadapté aux matériaux masqués/transparents.
5. Contrôle automatisé : capture via MCP (`uemcp` fournit des captures du viewport) ou test automatisé ; revue humaine du rendu indispensable pour le style (les unités du jeu ont une direction artistique précise).

### Gaps
- Aucune démonstration publique complète de ce pipeline en 5.8 trouvée ; temps et coût par asset non mesurés ; pas de test de compatibilité avec le fork Hazelight ni avec `-ExecutePythonScript` en 5.8.

## Matériel (VRAM) pour la génération locale et alternatives cloud

### Takeaway
Hunyuan3D 2.1 officiel : ~10 Go VRAM pour la forme, 21 Go pour la texture, 29 Go pour les deux ; des forks communautaires descendent à 3–6 Go avec offloading. Les services cloud (Tripo, Meshy, Rodin, Tencent) évitent le GPU local pour quelques centimes à quelques dizaines de centimes par asset.

### Cited Findings
- Hunyuan3D-2.1 : ~10 Go (forme), 21 Go (texture), 29 Go (les deux) selon le README. [H] — [GitHub](https://github.com/Tencent-Hunyuan/Hunyuan3D-2.1)
- Variante communautaire WinPortable : ≥ 3 Go pour la géométrie (optimisations maximales, modèle 2.1) et ≥ 6 Go pour la texture (modèle 2.0), sans optimisations mmgp dans leur build 2.1 ; guides : 12 Go pour forme + texture sur RTX 30/40/50, mode `--low_vram_mode`, RAM système recommandée 24–32 Go. Sources non vérifiées contre le dépôt officiel. [B] — [WinPortable](https://gh.hhj520.top/dwongdev/Hunyuan3D-2-WinPortable) ; [Codersera](https://codersera.com/blog/set-up-hunyuan3d-2-on-windows-a-step-by-step-guide/amp/) ; [tspi.at](https://www.tspi.at/2026/05/01/opensource3dassets.html)
- Cloud : prix Tripo (1 crédit = 0,01 $ ; image-to-3D 20–30 crédits donc ~0,20–0,30 $) ; Rodin Gen-2 via revendeur 0,80 $/requête ; Hunyuan cloud 200 crédits gratuits (entreprises) ; Real-ESRGAN ncnn-vulkan fonctionne sans CUDA. [H/B] — [Tripo](https://developers.tripo3d.ai/) ; [EmpirioLabs](https://empiriolabs.ai/models/hyper3d-gen2) ; [Tencent Cloud](https://www.tencentcloud.com/ko/document/product/1284/75533)

### Inferences
- Sur un PC de dev avec un GPU qui sert aussi à l'éditeur Unreal, la génération locale (21–29 Go) est peu réaliste ; cloud recommandé pour le prototype. Le GPU de l'utilisateur n'a pas été identifié dans cette recherche.
- La licence Hunyuan (exclusion UE/UK/Corée) pèse davantage sur l'usage local que le matériel.

### Gaps
- GPU de la machine du projet inconnu ; VRAM de Trellis/TripoSG et autres modèles ouverts non recherchée ; prix mensuels des abonnements non récupérés.
