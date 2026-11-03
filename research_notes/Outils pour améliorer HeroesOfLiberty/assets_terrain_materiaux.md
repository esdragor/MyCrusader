# Assets, terrain, végétation et matériaux pour un RTS médiéval en UE5.x (état au 2026-11-03)

Note de méthode : ~20 appels web. Plusieurs pages officielles ont répondu 403/429 (fab.com/licenses, syntystore.com/pages/license-information) ou étaient vides (notes de version UE 5.8 Epic). Les statuts de fonctionnalités viennent donc de presse secondaire. Ce qui n'a pas été trouvé figure dans « Gaps », pas inventé.

## 1. Marketplaces et packs : Fab, Megascans, Synty, KitBash3D, Sketchfab, itch.io, Kenney, Quaternius, CGTrader, TurboSquid

### Takeaway
Fab est la place centrale ; Megascans n'est plus gratuit en bloc depuis le 1er janvier 2025 (seul un sous-ensemble reste gratuit). Pour un RTS commercial solo, les sources les plus sûres juridiquement et au mieux rapport coût/effort sont Fab (licence Standard, usage en jeu autorisé), Synty (licence commerciale, 5 sièges par achat, hors IA générative) et les CC0 (Quaternius, Kenney, Poly Haven, ambientCG). KitBash3D, Sketchfab, CGTrader, TurboSquid, itch.io n'ont pas pu être documentés : voir Gaps.

### Cited Findings
- Fab a ouvert le 22 octobre 2024 ; les Megascans gratuits sur Fab ont couru jusqu'au 31 décembre 2024, puis, à partir du 1er janvier 2025, seule une sélection de 1500+ assets Megascans est restée gratuite sous licence Fab Standard — [Résultats de recherche Epic forum/80.lv (synthèse)](https://forums.unrealengine.com/t/access-legacy-megascans-on-bridge-and-quixel-com/2142664) ; [80.lv](https://80.lv/articles/epic-games-to-launch-tool-for-free-megascans-downloads-in-fab/)
- Mise à jour Epic du 26 mars 2025 : les Megascans réclamés gratuitement avant le 22/10/2024 restent sur Quixel.com/Bridge (pas migrés vers Fab) ; les « Claim All » du 28/11/2024 ou après reçoivent une licence Fab Standard ; certains assets obsolètes ne migrent pas — [Megascans update, March 2025](https://forums.unrealengine.com/t/megascans-update-march-2025/2419561)
- Pour les comptes UE gratuit ayant réclamé, la licence Fab EULA permet l'usage dans tout moteur/outil (l'Epic Content License limitait à Unreal Engine) — [Megascans update, March 2025](https://forums.unrealengine.com/t/megascans-update-march-2025/2419561)
- Fab propose les licences CC-BY (gratuit) et Standard (gratuit ou payant), avec paliers Personal/Professional (seuil communautaire environ 100 000 $ de revenu annuel, à vérifier dans l'EULA) — [Epic doc : licenses and pricing in Fab](https://dev.epicgames.com/documentation/fab/licenses-and-pricing-in-fab) (via recherche) ; seuil non confirmé : [forum Epic](https://forums.unrealengine.com/t/how-to-determine-personal-vs-professional-license/2082366)
- Résumé de licence Fab : on peut distribuer commercialement ses projets contenant les assets Fab ; ce résumé n'est pas juridiquement contraignant ; certains messages avertissent que des licences peuvent permettre l'extraction des assets d'un jeu — [Fab license threads](https://forums.unrealengine.com/t/fab-license-terms-standard-license/2094243) (synthèse de recherche, EULA non lue directement)
- Politique IA de Fab (message du 22 mai 2025) : les vendeurs doivent marquer « Created with AI » ; un contenu mal marqué viole les conditions ; un éditeur a vu 41 000+ assets IA retirés ; pas d'interdiction de l'IA ; détection automatique et limites d'upload « à l'étude » ; les acheteurs peuvent masquer les assets IA via les préférences de contenu — [Update on products generated with AI](https://forums.unrealengine.com/t/update-on-products-generated-with-ai/2523501)
- Application jugée faible par la communauté (signalements d'IA non marquée aboutissant à « no action taken ») — [forum Epic](https://forums.unrealengine.com/t/reporting-ai-generated-content-that-isnt-tagged-as-such-gets-a-no-action-taken-response/2597888) ; Cubebrush a banni tout contenu IA — [80.lv](https://80.lv/articles/cubebrush-marketplace-prohibits-ai-generated-content)
- Synty : licence « One Time Purchase » = 5 sièges par achat, non limitée par moteur/OS/plateforme ; usage commercial ; l'IA générative (dont utilisation comme données d'entrée/jeu de données) est exclue, une licence « Custom » existe pour cela — [Synty One Time Purchase Licence](https://syntystore.com/pages/one-time-purchase-licence) ; [Standard Subscription Licence](https://syntystore.com/pages/standard-subscription-licence)
- POLYGON Fantasy Kingdom : page store listant UE 4.26 à 5.4, FBX fournis, effets convertis en Niagara ; Knights Pack et Dark Fantasy : « 5 seats per copy » ; certaines pages affichent « Sold out » avec rabais — [Synty Fantasy Kingdom](https://syntystore.com/products/polygon-fantasy-kingdom) ; [Knights Pack](https://syntystore.com/products/polygon-knights-pack)
- Revente/redistribution d'assets Synty interdite et « asset flipping » banni : source tierce seulement — [Game Asset Licenses Explained](https://cinevva-site.pages.dev/guides/game-asset-licenses)
- Packs médiévaux Fab repérés (prix non récupérés) : Medieval Village de Marco Maria Rossi (496 meshes, 41 matériaux maîtres, Nanite/Lumen ; gratuit jusqu'au 30 décembre 2025 donc probablement terminé), Modular Medieval Town (Town, Castle, Village, Lumen/Nanite), Castle Forge: Modular Medieval Castle (murs, tours, portes ; Nanite/SM6), Modular Medieval Village avec « Medieval Mega Bundle » — [CG Channel, 18/12/2025](https://www.cgchannel.com/2025/12/get-400-free-assets-for-building-a-medieval-village-in-unreal-engine/) ; [listing Fab](https://www.fab.com/listings/0032f89e-d48c-471b-9440-463addc815f7)
- Kenney « Medieval RTS » : CC0 mais 2D (120 assets) ; Quaternius : généralement CC0, pack « 3D Fantasy RTS » listé CC0 sur GDevelop, « Fantasy Props MegaKit » (200+ props médiévaux) ; un FAQ 2026 de Playgama ajoute une clause de non-revente (non standard CC0, à vérifier) — [Kenney](https://kenney.nl/assets/medieval-rts) ; [GDevelop 3D Fantasy RTS](https://gdevelop.io/es-es/asset-store/free/3d-fantasy-rts-3d-fantasy-rts) ; [Quaternius Fantasy Props MegaKit](https://quaternius.com/packs/fantasypropsmegakit.html) ; [Playgama](https://playgama.com/blog/business-faqs/can-you-use-kenney-and-opengameart-assets-in-commercial-web/)

### Inferences
- Pour 7 cartes et plusieurs factions, la stratégie la plus économique : un kit modulaire médiéval Fab Nanite (murs/bâtiments) + Synty pour la cohérence stylisée (low-poly) OU Megascans/Poly Haven pour le réalisme ; ne pas mélanger low-poly Synty et PBR photoréaliste (cohérence de style).
- Avec un jeu commercial solo, éviter les assets Fab marqués « Created with AI » par prudence (qualité et risque de retrait futur), et garder les reçus/licences de chaque asset dans un registre.
- Synty : si un jour des outils IA servent à générer des variantes à partir de leurs modèles, la licence l'interdit ; rester sur usage direct.

### Gaps
- Prix actuels des packs Fab/Synty (non affichés dans les résultats) ; état exact 2026 de la gratuité Megascans (aucune source 2026 trouvée).
- KitBash3D (offre UE, licence jeux, packs médiévaux), Sketchfab (CC, boutique payante), itch.io, CGTrader, TurboSquid : aucune recherche aboutie dans le budget d'appels ; à documenter avant décision.
- Texte intégral de l'EULA Fab (redistribution dans un jeu empaqueté, extraction par les joueurs) : fab.com/licenses a renvoyé 403.
- Page de licence Synty complète : 429.

## 2. Sources PBR gratuites et génération IA de textures

### Takeaway
Poly Haven (CC0 confirmé par sa FAQ) et ambientCG (CC0 selon des guides tiers) sont les sources sûres pour un jeu commercial sans attribution. Côté IA, Adobe Substance 3D Sampler propose Text-to-Texture via Firefly (présenté par Adobe comme sûr commercialement) ; pour Meshy, Scenario, Leonardo, Polycam, textures.com : non vérifié.

### Cited Findings
- Poly Haven : « tous nos assets sont CC0, usage pour n'importe quel but, y compris commercial » ; catalogue d'environ 520 modèles, 780 textures, 980 HDRI (chiffres d'un guide tiers) — [Poly Haven FAQ](https://polyhaven.com/faq) ; [Poly Haven license](https://polyhaven.com/license) ; [Cinevva guide 2026](https://app.cinevva.com/guides/free-textures-hdris-materials)
- ambientCG : matériaux CC0, usage commercial sans attribution, fichiers bruts incluables dans un jeu (source tierce, pas la page de licence d'ambientCG) — [Cinevva guide 2026](https://app.cinevva.com/guides/free-textures-hdris-materials)
- Substance 3D Sampler 4.4 (bêta, annoncé en mars 2024) : Text-to-Texture via Firefly, conversion en matériau PBR complet, Content Credentials attachés ; Adobe le décrit comme conçu pour un usage commercial sûr (Firefly entraîné surtout sur images sous licence ouverte, domaine public et Adobe Stock) — [Adobe blog, 18/03/2024](https://blog.adobe.com/en/publish/2024/03/18/adobe-announces-firefly-powered-features-substance-3d-apps) ; [GameBusiness.jp](https://www.gamebusiness.jp/article/2024/03/21/22870.html) ; sécurité commerciale : source vendeur, non vérifiée de façon indépendante
- Revente de textures IA Sampler sur une place de marché : question restée sans réponse officielle — [Adobe community](https://community.adobe.com/t5/substance-3d-sampler-discussions/generative-ai-textures-in-prodcuts/m-p/14688580)

### Inferences
- Pour le jeu lui-même (non revente), le risque principal est l'incertitude juridique du droit d'auteur sur les sorties IA (non traité par ces sources) ; Firefly/Substance reste le choix le plus défendable.
- Un flux efficace : base CC0 (ambientCG/Poly Haven) pour sol/roche/bois, puis variations teinte/macro-variation dans UE plutôt que génération IA.

### Gaps
- Licences de Meshy, Scenario, Leonardo, Polycam, textures.com (non trouvées ; la recherche Meshy n'a rien renvoyé).
- Vérification directe de la page de licence ambientCG.

## 3. Techniques terrain/végétation UE 5.x

### Takeaway
Dans UE 5.7 (novembre 2025) le framework PCG est « production-ready » et Substrate aussi ; Nanite Foliage et le Procedural Vegetation Editor sont Experimental ; MegaLights est Beta en 5.7 puis Production Ready en 5.8. UE 5.8 est sorti le 17 juin 2026 (probablement la dernière 5.x), avec Mesh Terrain Experimental. Le projet utilise déjà la 5.8 (Hazelight 5.8.4 selon les notes du dépôt).

### Cited Findings
- UE 5.7 : PCG production-ready ; PCG Editor Mode (splines, peinture de points, volumes) ; PVE Experimental ; Nanite Foliage Experimental (Nanite Voxels et Assemblies en font partie) ; Substrate production-ready ; MegaLights Beta ; correctifs 5.7.2/5.7.3 liés à Lumen et à des normales invalides aux bords de composants Landscape en build empaqueté — [UE 5.7 Released (forum Epic)](https://forums.unrealengine.com/t/unreal-engine-5-7-released/2673913) ; [Digital Production, 12/11/2025](https://digitalproduction.com/2025/11/12/unreal-engine-5-7-foliage-pcg-and-in-editor-ai/)
- UE 5.8 : sorti le 17 juin 2026 ; MegaLights production-ready ; Mesh Terrain (3D, surplombs/grottes) Experimental ; PVE produit des assets « natifs Nanite » ; Lumen Lite (champs d'irradiance, cible 60 FPS matériel modeste/Switch 2) ; PCG permet des retouches manuelles sur la sortie procédurale — [GameFromScratch](https://gamefromscratch.com/unreal-engine-5-8-released/) ; [Digital Production 14/05/2026](https://digitalproduction.com/2026/05/14/unreal-engine-5-8-preview-rolls-in/)
- Guide de performance Lumen d'Epic : le niveau de scalabilité Low désactive Lumen, Medium désactive les réflexions Lumen au profit du SSR ; garder la scène ray tracing sous ~100 000 instances ; Virtual Shadow Maps et nombre de lumières sont des goulots fréquents ; pas d'exclusion de Lumen par objet (on passe par un post-process volume) — [Lumen Performance Guide](https://dev.epicgames.com/documentation/unreal-engine/lumen-performance-guide-for-unreal-engine) ; [forum Epic](https://forums.unrealengine.com/t/is-there-a-way-to-disable-lumen-gi-per-object-or-material/575002)

### Inferences
- Pour un RTS avec beaucoup d'unités : PCG (production-ready) pour semer arbres/buissons/rochers par carte est le meilleur levier ; éviter de bâtir la production sur Nanite Foliage / PVE / Mesh Terrain (Experimental, peu de recul).
- Lumen coûte cher avec une caméra aérienne et beaucoup d'unités animées ; un éclairage simple (lumière directionnelle + skylight, éventuellement GI statique/Lumen basse qualité) est probablement plus sûr — à profiler avec Unreal Insights (aucune source RTS spécifique trouvée).

### Gaps
- Statut 5.8 de Nanite Foliage/Voxels/Assemblies, Landscape, Water, Runtime Virtual Textures : notes de version officielles illisibles, rien dans la presse consultée.
- Techniques RVT, blending de couches Landscape, décals, Sky/Atmosphere, eau : aucune source récupérée (connaissance générale non citée ici : RVT et layer blending sont des fonctions de longue date de UE5, à confirmer dans la doc).

## 4. Meilleur gain visuel par effort pour une caméra RTS vue de dessus

### Takeaway
Aucune source solide trouvée ; seules les recommandations de performance Lumen sont citables. Le reste est de l'inférence.

### Cited Findings
- Les limites de Lumen, VSM et nombre de lumières citées en § 3 sont les seuls éléments sourcés — [Lumen Performance Guide](https://dev.epicgames.com/documentation/unreal-engine/lumen-performance-guide-for-unreal-engine)

### Inferences
- Classement probable (non sourcé, jugement) : 1) étalonnage et post-process (LUT, contraste, saturation, brume/exponential height fog) ; 2) direction de lumière basse, ombres lisibles ; 3) variation de sol (couches + macro-variation, décals de chemins/terre) ; 4) densité de props lisible (arbres PCG aux bords, rochers) ; 5) unité lisible : contraste unités/sol prime sur le photoréalisme ; 6) brouillard de guerre cohérent avec la palette.
- La cohérence de style (un seul fournisseur ou un traitement couleur unifié) l'emporte sur la qualité individuelle des assets.

### Gaps
- Retours d'expérience de RTS UE5 publiés (postmortems, GDC) non recherchés faute de budget.
- Aucune source sur le look du brouillard de guerre ni les décals en vue aérienne.
