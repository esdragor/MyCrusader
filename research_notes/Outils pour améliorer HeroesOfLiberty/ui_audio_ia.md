# Outils IA et non-IA pour l'art d'interface/icônes et l'audio (musique, bruitages, voix) — RTS médiéval commercial, développeur solo (état au 2026-11-03)

Légende de fiabilité : **[H]** = page officielle de l'éditeur lue directement ; **[M]** = presse spécialisée ou blog sérieux ; **[B]** = agrégateur, revendeur, blog à intérêt commercial, chiffres souvent contradictoires. Les prix et licences changent vite : tout ce qui est [M]/[B] est à revérifier sur la page officielle avant achat. Les résumés de recherche web ont parfois été contradictoires ; je signale les conflits.

---

## 1. Art UI / icônes / portraits / concept art : outils, cohérence de style, droits commerciaux, automatisation

### Takeaway
Pour un solo en CLI/agent, les pistes les plus nettes sont (a) **FLUX.2 via l'API BFL** (droits commerciaux de l'API, prix à l'image, LoRA/fine-tuning proposés par BFL), (b) **ComfyUI local en headless** (API HTTP, workflows JSON exportables), (c) **GPT Image** (OpenAI, fond transparent supporté selon la doc officielle), (d) **Scenario** (API qui entraîne des modèles/LoRA de style, mais prix et licence non vérifiés). **Midjourney n'a toujours pas d'API officielle** (automatisation interdite par ses CGU, selon sources secondaires). La clarté des droits d'auteur est limitée : le Copyright Office américain exige une contribution humaine, les prompts seuls ne suffisent pas.

### Cited Findings
**Midjourney**
- Pas d'API publique officielle ni annoncée, état au 2026-08-18 (date de vérification d'un revendeur) ; accès par Discord/web avec abonnement ; CGU restreignant l'accès automatisé et la revente ; V8 Alpha lancée le 2026-03-17, V8.2 le 2026-07-24 ; les « API Midjourney » tierces (ex. CometAPI) sont des wrappers non officiels avec risque de bannissement. [B/M, sources secondaires, CGU officielles non lues] — [unifically.com](https://unifically.com/blogs/midjourney-api), [CometAPI](https://www.cometapi.com/zh-CN/models/midjourney/mj-turbo-upload/)
- Usage commercial permis ; entreprises > 1 M$ de revenu annuel doivent prendre Pro/Mega. [B] — [tech-insider.org](https://tech-insider.org/au/?p=1901)

**FLUX / Black Forest Labs (BFL)**
- FLUX.2 [klein] 4B : poids ouverts sous Apache 2.0 (usage commercial libre) ; klein 9B et FLUX.2 [dev] : licence non commerciale, licence commerciale payante de BFL requise pour les déployer dans un produit. [H, FAQ BFL] — [help.bfl.ai](https://help.bfl.ai/articles/3670520907-can-i-use-the-api-for-a-commercial-application)
- Page de tarifs BFL (lue 2026-11-03) : licences « open weights » auto-hébergées par paliers (Builder 10K images/mois, Platform 100K, Professional, Enterprise) avec droits de fine-tuning/LoRA ; la page mentionne aussi un « FLUX 3 Image Klein » (nouveau modèle, non détaillé ici). Les montants des licences n'étaient pas visibles. [H, partiel] — [bfl.ai/pricing](https://bfl.ai/pricing)
- API : 1 crédit = 0,01 USD ; tarif par mégapixel de sortie ; modèles pro, max, flex, klein 4B/9B ; [dev] sans API hébergée (local, non commercial) ; endpoints fine-tunés facturés au prix du modèle de base (bêta publique). Les montants exacts étaient masqués ; une source secondaire cite klein à 0,014 $/image (4B) et 0,015 $/image (9B) et flex à 0,05 $/MP — non confirmé. [H partiel / B] — [docs.bfl.ai](https://docs.bfl.ai/quick_start/pricing), [help.bfl.ai (résumé de recherche)](https://help.bfl.ai/articles/3670520907-can-i-use-the-api-for-a-commercial-application)
- Source secondaire : l'usage de l'API inclut les droits commerciaux sans licence de modèle séparée. [M] — [help.bfl.ai](https://help.bfl.ai/articles/3670520907-can-i-use-the-api-for-a-commercial-application)

**GPT Image (OpenAI)**
- Doc officielle (lue 2026-11-03) : modèles listés `gpt-image-2.5-sunburst` (précision d'édition) et `gpt-image-2.5-flare` (génération rapide) ; les deux supportent `background: "transparent"` avec `output_format: "png"` ou `"webp"`. La page était tronquée avant les prix ; les noms de modèle sont inhabituels, à revérifier. [H] — [developers.openai.com](https://developers.openai.com/api/docs/guides/image-generation)
- Contradiction : des blogs disent que gpt-image-2 refuse le fond transparent (recommandent gpt-image-1.5) ; un seul le décrit « en préversion » avec 2.5. Prix par image très variables selon les sources (de ~0,006 à ~0,21 $ en 1024² selon la qualité) — non confirmés. [B] — [wavespeed.ai](https://wavespeed.ai/blog/posts/gpt-image-2-2026/), [picassoia](https://blog.picassoia.com/gpt-image-2-api-pricing-cost-per-image-parameters-and-transparent-pngs)

**Scenario**
- Doc officielle : l'API Scenario donne « un accès programmatique à tout ce qu'on peut faire dans l'appli web », y compris entraînement de modèles personnalisés sur ses propres jeux de données (styles, personnages, objets) et composition de LoRA ; l'authentification se fait par clé API. Rien sur les plans ni les termes commerciaux. [H, partiel] — [docs.scenario.com](https://docs.scenario.com/)
- Prix : très contradictoires entre agrégateurs (paliers à 15/45/75 $ ou « Pro 20 $ » ; un plan gratuit ajouté en octobre 2026 selon un seul agrégateur). Aucun énoncé fiable des droits commerciaux. [B] — [costbench](https://costbench.com/software/ai-media-apis/scenario-api/), [aitoolsatlas](https://aitoolsatlas.ai/tools/scenario)

**Leonardo**
- API facturée séparément de l'abonnement ; droits commerciaux sur les plans payants (le plan gratuit : sources en désaccord) ; prix d'API très divergents (de 9 $ à 72 $/mois selon les sites). Aucune page officielle lue. [B] — [dev.to](https://dev.to/techsifted/leonardo-ai-pricing-2026-free-credits-paid-plans-and-api-costs-16a2), [apiframe](https://apiframe.ai/guides/leonardo-ai-api-guide)

**Ideogram**
- API vendue à part, portail développeur avec solde de crédits propre ; ~0,03 à 0,09 $/image (août 2026, un tracker) ; droits commerciaux : sources contradictoires (gratuit inclus / payant seulement / palier Maestro). [B] — [eesel.ai](https://eesel.ai/blog/ideogram-pricing), [costbench](https://www.costbench.com/compare/ideogram-vs-leonardo-ai/)

**Recraft**
- Sorties SVG vectoriel (utile pour icônes) ; plan gratuit sans usage commercial (images publiques) ; plans payants avec droits commerciaux (cité août 2026) ; SVG ≈ 2 crédits contre 1 pour le raster ; API : ~0,04 $/raster, ~0,08 $/vecteur selon un avis, en conflit avec les calculs par crédit. [B] — [pricepertoken](https://pricepertoken.com/recraft-pricing), [invideo](https://invideo.io/blog/recraft-ai-image-generator/)

**ComfyUI / Stable Diffusion local**
- ComfyUI démarre un serveur HTTP local (127.0.0.1:8188) ; mode headless ; workflows exportés au format « API » JSON ; soumission par POST `/prompt`, état via `/history/{prompt_id}`, schéma des nœuds via `/object_info/{Node}` ; un LoRA se branche entre le chargeur de checkpoint et le sampler. [M, doc communautaire/Modal] — [docs.comfy.org](https://docs.comfy.org/development/comfyui-server/comms_overview?ref=toolarium.ru), [Modal](https://modal.com/docs/examples/comfyapp)

**Droit d'auteur des sorties IA (États-Unis)**
- Rapport Copyright Office partie 2 (publié le 2025-01-29 selon la majorité des sources ; une source isolée dit 2026-03-17, non confirmé) : l'authorship humaine est requise ; les prompts seuls ne suffisent pas à fonder un droit d'auteur ; analyse au cas par cas (sélection/arrangement humains protégeables). Rien de spécifique aux assets de jeux. [M] — [Akin Gump](https://www.akingump.com/en/insights/ai-law-and-regulation-tracker/us-copyright-office-releases-part-2-of-ai-report), [Mintz](https://www.mintz.com/insights-center/viewpoints/54731/2025-02-07-us-copyright-office-publishes-second-part-report-ai)

**Marchés d'assets (Fab, etc.)**
- Fab/Epic Marketplace : les vendeurs doivent étiqueter « CreatedWithAI » quand une part matérielle est générée ; Fab et ArtStation Marketplace n'autorisent pas l'entraînement d'IA sur les assets ; l'application de la règle est jugée incohérente par la communauté ; Cubebrush interdit tout contenu IA. [M] — [80.lv](https://80.lv/articles/epic-games-adresses-ai-generated-content-plaguing-fab), [Epic forums](https://forums.unrealengine.com/t/update-on-products-generated-with-ai/2523501/14), [CG Channel](https://www.cgchannel.com/2025/05/cubebrush-bans-ai-generated-content-from-its-online-marketplace/)

### Inferences
- Pour la cohérence de style en pipeline automatisé : FLUX.2 + LoRA (local sous licence BFL appropriée, ou fine-tune via l'API BFL) ou Scenario (modèle de style entraîné) sont les deux voies les plus directes ; les images de référence suffisent pour du prototypage. Comme klein 4B est Apache 2.0, un pipeline ComfyUI + klein 4B (+ LoRA entraîné maison) est la seule voie « poids ouverts » clairement libre de licence commerciale à payer — à confirmer par un test de qualité sur icônes médiévales.
- Pour des icônes détourées : GPT Image (fond transparent) ou Recraft (SVG) sont plus pratiques ; sinon détourage en post-traitement (hors périmètre de cette recherche).
- Un jeu dont l'art est généré sans retouche humaine aura une protection par copyright faible aux États-Unis ; retouche, composition et sélection humaines renforcent la position. Les droits « d'usage » accordés par les éditeurs ne créent pas de copyright.
- Midjourney est à écarter si l'on veut l'automatisation par agent (risque de bannissement).

### Gaps
- Pas de page officielle lue pour les prix/licences de Leonardo, Ideogram, Recraft, Scenario, Midjourney (CGU) : fiabilité [B].
- Prix par image exacts de BFL (masqués) et prix officiels de gpt-image 2.x non obtenus.
- Qualité réelle de chaque outil sur des icônes/portraits médiévaux et capacité de LoRA de style sur icônes : non testée ni trouvée.
- Kits d'UI médiévaux sur Fab/itch.io/Kenney/Humble et leurs licences : non recherchés (aucune source consultée).
- Litiges sur les données d'entraînement des modèles d'image (Getty/Stability, artistes contre Midjourney, etc.) en 2026 : non couverts.

---

## 2. Musique : outils, licences pour jeux (Steam, Content ID), litiges et accords de labels en 2026

### Takeaway
**ElevenLabs Music** est le seul outil IA dont j'ai lu les termes officiels, et ils sont favorables à un jeu indépendant : l'« Indie Game » (< 500 k$ de revenu cumulé du jeu et < 1 M$ de revenu annuel de l'éditeur) est expressément couvert, mais un jeu qui dépasse ces seuils exige un plan Enterprise. **Suno** (Pro 8 $/mois, droits commerciaux) est le plus puissant mais est dans un litige actif avec UMG et Sony (nouveau procès en septembre 2026 après le lancement de v6). Warner a réglé avec Suno et Udio, UMG avec Udio ; Sony continue contre Udio. Risque Content ID : réel pour la diffusion YouTube/Twitch de bandes-annonces, mais Steam ne fait pas de Content ID (rien trouvé).

### Cited Findings
**ElevenLabs Music — termes officiels (mis à jour le 2026-10-09)**
- Droits médias : « tout usage commercial en ligne et hors ligne » sauf cinéma, TV, radio et « Studio Games » pour tous les plans sauf Enterprise Music ; un « Indie Game » (jeu avec toutes éditions/DLC ayant généré < 500 000 $ depuis sa sortie, développé/édité par une entité dont le revenu tous jeux < 1 000 000 $ sur son dernier exercice, net des frais de plateforme et remboursements, affiliés inclus) n'est pas un Studio Game ; si le jeu cesse de se qualifier, 60 jours pour passer à un plan couvrant les Studio Games, puis arrêt de l'usage de la sortie dans ce jeu (copies déjà distribuées utilisables). [H] — [elevenlabs.io/music-terms](https://elevenlabs.io/music-terms)
- Plans Free/Starter/Creator/Pro : usage individuel ; Scale < 10 employés ; Business < 50 employés ; attribution « Eleven Music » obligatoire sur Free seulement ; diffusion en streaming interdite sur Free et Starter, permise dès Creator ; l'utilisateur doit respecter les règles de divulgation IA de chaque plateforme (§ 4.4) ; création de bibliothèques de musique et revente de modèle interdites. Aucune mention de Content ID. [H] — [elevenlabs.io/music-terms](https://elevenlabs.io/music-terms)
- Prix : Free 0 $ (10 000 crédits/mois), Starter 6 $ (30 000), Creator 22 $ (121 000), Pro 99 $ (600 000, sortie PCM 44,1 kHz via API), Scale 299 $, Business 990 $ ; crédits partagés entre musique, SFX, voix. [H] — [elevenlabs.io/pricing](https://elevenlabs.io/pricing)
- Licences d'entraînement : accords avec Merlin et Kobalt (opt-in des artistes), lancement août 2025. [M] — [Music Business Worldwide](https://musicbusinessworldwide.com/eleven-music-new-ai-rival-to-suno-launches-with-merlin-kobalt-licensing-deals-in-the-bag)

**Suno**
- Page tarifs officielle (2026-11-03) : Free 0 $ (50 crédits/jour, pas de téléchargement ni de droits commerciaux) ; Pro 8 $/mois (2 500 crédits, 20 téléchargements/mois, droits commerciaux, stems, modèles personnalisés) ; Premier 24 $/mois (10 000 crédits, 60 téléchargements, Suno Studio) ; renvoie aux CGU pour les limites d'usage commercial. [H] — [suno.com/pricing](https://suno.com/pricing)
- Droits commerciaux : conservés pour les morceaux créés pendant l'abonnement Pro/Premier, y compris après résiliation ; les morceaux créés en Free restent non commerciaux ; les CGU ne garantissent pas qu'un copyright existe sur la sortie. [B] — [dynamoi.com](https://dynamoi.com/learn/guides/suno-commercial-rights-explained), [sonilo.com](https://sonilo.com/blog/suno-v5-5-pricing-commercial-use)

**Litiges et accords (états au plus tard fin septembre 2026)**
- Warner a réglé avec Suno (2025-11-26) : Suno doit lancer en 2026 des modèles sous licence ; téléchargements payants/limités ; les anciens modèles seront retirés. [M] — [TheWrap](https://www.thewrap.com/warner-music-group-partnership-suno-settles-lawsuit-ai-music-company), [The Star](https://www.thestar.com.my/tech/tech-news/2025/11/26/warner-music-group-settles-copyright-case-with-suno-for-licensed-ai-music)
- UMG a réglé avec Udio (annoncé le 2025-10-29) : plateforme par abonnement à lancer en 2026, créations dans un « walled garden », filtrage/empreintes avant lancement ; Warner a réglé avec Udio en novembre 2025. Export restreint selon une source secondaire, non confirmé par la source primaire de presse lue. [M] — [Music Business World Wide](https://www.musicbusinessworldwide.com/universal-music-settles-udio-lawsuit-strikes-deal-for-licensed-ai-music-platform), [Decrypt](https://decrypt.co/349360/warner-music-moves-from-litigation-to-licensing-with-udio)
- UMG et Sony ont déposé un deuxième procès contre Suno (Massachusetts) quelques jours après le lancement de son modèle v6 (septembre 2026) : > 60 000 enregistrements ; Suno affirme que v6 est entraîné sur contenu licencié. Sony a déposé le 2026-07-20 un second procès contre Udio (30 117 enregistrements, SDNY). BMG et Believe ont des accords avec Suno. [M] — [Billboard](https://www.billboard.com/pro/umg-sony-hit-suno-with-lawsuit-after-new-ai-music-model/), [Hollywood Reporter](https://www.hollywoodreporter.com/music/music-industry-news/umg-sony-music-file-second-lawsuit-against-suno-1236705369/), [LawCommentary](https://www.lawcommentary.com/articles/sony-escalates-legal-battle-against-udio-with-new-45-billion-ai-music-copyright-lawsuit)
- Contradiction : une page (AI Vortex) dit encore Warner en litige avec Udio ; elle est obsolète. [B] — [aivortex.io](https://www.aivortex.io/legal/ai-case-law/suno-udio-music-ai/)

**Stable Audio**
- Stable Audio 2.5 : modèle propriétaire annoncé comme entraîné sur données sous licence, « commercially safe » ; licence Community de Stability gratuite sous 1 M$ de revenu annuel (la source cite Stable Diffusion, non vérifié pour Stable Audio) ; prix API : 0,20 $/fichier, baisse à ~0,08 $ en octobre 2026 selon un seul tracker. [B] — [dynamoi.com](https://dynamoi.com/learn/ai-music-distribution/can-i-distribute-stable-audio-commercially), [usagepricing](https://www.usagepricing.com/blueprint/activity/stability-ai-2026-10-23-stable-audio-free-plan)

**Content ID**
- Content ID fait de la correspondance d'empreintes, sans considérer si la piste est humaine ou IA ; des fournisseurs d'IA ou des tiers peuvent enregistrer des morceaux générés et réclamer à tort ; YouTube n'arbitre pas, les disputes sont entre utilisateurs ; conseillé : conserver prompts, dates, projets/stems. Cas voisin : un YouTuber a reçu des avertissements après des voix IA superposées à la musique de Silent Hill. Aucune source sur Steam et Content ID. [B] — [sunowatermark.com](https://sunowatermark.com/youtube-content-id-ai-music/), [dtptips](https://dtptips.com/ai-music-copyright-nightmare-how-ai-tracks-are-abusing-content-id-and-threatening-creators), [pulse.bot](https://www.pulse.bot/gaming/news/youtuber-copyright-struck-after-others-layer-ai-voiceovers-on-video-game-music-ca9c4d2d-f959-4e95-a8d4-895e483f0166/)

**Bibliothèques classiques (non-IA)**
- Epidemic Sound a une page « game development » (licence sans redevance continue) ; je n'ai pas pu confirmer si un jeu Steam commercial est couvert par la licence « Commercial » ; des avis signalent de la friction (liste blanche, réclamations) pour les jeux et streams ; la préautorisation Content ID est annoncée pour YouTube, pas pour Steam. [B] — [epidemicsound.com](https://www.epidemicsound.com/game-development/music-for-video-games/), [freevisuals.net](https://www.freevisuals.net/reviews/epidemic-sound)

### Inferences
- Pour un solo, **ElevenLabs Music Creator (22 $/mois)** est le choix le plus défendable juridiquement : termes écrits qui nomment les jeux indépendants, entraînement sous licence, API accessible. Surveiller le seuil Indie (500 k$ de revenu cumulé du jeu) : au-delà, passage à Enterprise obligatoire, donc à prévoir si le jeu rencontre le succès. À vérifier : la formulation exacte « individuel » pour les plans de moins de Scale (un solo reste couvert).
- Suno donne de meilleurs résultats musicaux généralement connus, mais le risque juridique (procès UMG/Sony en cours, droits d'auteur incertains sur la sortie) est supérieur ; pour un jeu commercial, le garder pour du prototypage ou de la musique non centrale.
- Risque Content ID : surtout en aval (YouTube/Twitch/trailers). Atténuation : garder les fichiers sources et prompts, ne pas distribuer la bande-son en streaming sauf plan permettant (ElevenLabs Creator+), déclarer la musique en liste blanche si nécessaire.

### Gaps
- AIVA, Soundraw, Udio (état du produit licencié fin 2026) : aucun terme ni prix officiel lu.
- Termes de service officiels de Suno (clause exacte sur l'usage commercial dans un jeu, Content ID) non lus ; état exact du modèle « v6 sous licence Warner » non vérifié par la source officielle.
- Bibliothèques médiévales/orchestrales royalty-free spécifiques (Epic/Fab, Humble Bundle, etc.) : non recherchées ; Epidemic Sound pour Steam : à confirmer par écrit auprès de l'éditeur.
- Résolution des procès Sony/UMG vs Suno/Udio après septembre 2026 : non trouvée.

---

## 3. Bruitages, voix des unités et conseils audio Unreal

### Takeaway
Pour les bruitages : **ElevenLabs SFX** (API documentée, durées 0,5-30 s, option de boucle) + **bundles Sonniss GDC** (gratuits, sans redevance, entraînement IA interdit) + **Freesound** (attention aux licences CC-BY-NC). Pour les voix (barks) : l'infrastructure ElevenLabs est documentée côté SFX/musique, mais je n'ai **pas** trouvé de source primaire sur la licence des voix ni la qualité du français ; à vérifier. Côté Unreal, les **Sound Concurrency** (max count, résolution, ducking via Volume Scale, voice steal) sont la brique centrale de limitation de voix d'un RTS ; MetaSounds les accepte directement.

### Cited Findings
**ElevenLabs SFX**
- Endpoint `POST https://api.elevenlabs.io/v1/sound-generation` ; paramètres `text`, `duration_seconds` (0,5 à 30), `loop`, `prompt_influence` (0-1, défaut 0,3), `model_id` (`eleven_text_to_sound_v2` par défaut ou `v3`) ; formats MP3/Opus/PCM/WAV ; WAV/PCM 44,1 kHz exige le plan Pro ou plus ; MP3 192 kbps exige Creator+. [H] — [ElevenLabs API](https://elevenlabs.io/docs/api-reference/text-to-sound-effects/convert)
- Les SFX sont inclus dans tous les plans, crédits partagés ; plans payants avec licence commerciale ; plan gratuit sans licence commerciale (attribution requise selon des sources secondaires). [H pour la grille / B pour le détail gratuit] — [elevenlabs.io/pricing](https://elevenlabs.io/pricing), [costbench](https://costbench.com/software/ai-music-apis/elevenlabs-sfx-api/free-plan/)
- Prix SFX en API cités à 0,12 $/minute ou 40 crédits/seconde : incohérent entre sources. [B] — [modelslab](https://modelslab.com/sound-effects-api-pricing.md)

**Sonniss GameAudioGDC**
- Sons « royalty free et utilisables commercialement » (jeux vendus, vidéos monétisées) ; attribution non requise ; redistribution comme SFX/pack interdite ; **entraînement IA/ML strictement interdit** ; le dernier bundle sur la page lue est GDC 2024 (9 parties, 23,5 Go), pas de GDC 2025 ; GDC 2026 a sa propre page (une source secondaire : > 347 fichiers, 7,47 Go, 17 éditeurs). [H pour les termes / M pour 2026] — [sonniss.com](https://sonniss.com/gameaudiogdc/), [rekkerd.org](https://rekkerd.org/sonniss-releases-gdc-2026-game-audio-bundle/)

**Freesound**
- Trois licences : CC0 (tout usage, y compris vente), CC-BY (attribution obligatoire), CC-BY-NC (**pas d'usage commercial** : exclu pour un jeu vendu) ; Sampling+ retirée progressivement ; les remixes héritent des restrictions ; panneau de préférences « IA générative » ; API Freesound existante avec filtre sur ces préférences. [H] — [freesound.org/help/faq](https://freesound.org/help/faq/)

**Unreal : limitation de voix, ducking**
- Sound Concurrency : Max Count par groupe, règle de résolution, Volume Scale (ducking des anciennes voix), Voice Steal Release Time ; MetaSounds accepte un Concurrency Set via Voice Management ; un fil de forum signale décrochages quand de très nombreux MetaSounds sont actifs (anecdotique) : essayer de relever la limite de voix du moteur et d'utiliser les commandes de debug audio. [H pour la doc / B pour le forum] — [Epic Sound Concurrency](https://dev.epicgames.com/documentation/en-us/unreal-engine/sound-concurrency-reference-guide), [forum Epic](https://forums.unrealengine.com/t/metasound-stop-working-at-all-or-stuck-when-too-many/2321014)

### Inferences
- Pour un RTS : un Concurrency Set « voix d'acquittement d'unités » avec Max Count bas (2-4), résolution « arrêter le plus ancien/plus silencieux » et une courte durée de Voice Steal, plus un set « combat » plus large, est le modèle classique ; le ducking par Volume Scale peut baisser les combats pendant les voix importantes. (Inférence de conception issue de la doc ; non testée.)
- Générer les SFX en WAV 44,1 kHz exige le plan Pro (99 $) ; sinon en MP3/Opus à transcoder, ou un mois Pro pour une grande fournée.
- Éviter Freesound CC-BY-NC ; privilégier CC0 ; consigner les crédits des CC-BY dans les écrans de remerciements.

### Gaps
- **Voix/barks** : licence des voix ElevenLabs et d'autres TTS pour jeux commerciaux (voix clonées, voix de bibliothèque), qualité en français vs anglais d'Eleven v3 : aucune source primaire trouvée dans cette passe.
- Audio Modulation (bus mixes, control buses) pour le ducking RTS, bonnes pratiques d'acquittement d'unité : rien de spécifique trouvé ; seule la doc Sound Concurrency a été consultée.
- Packs Epic/Fab de sons de bataille médiévale et leurs licences, bibliothèques médiévales payantes : non recherchés.
- Détails de la licence complète Sonniss (/gdc-bundle-license/) non lus ; GDC 2026 non lu en primaire.

---

## 4. Divulgation IA sur Steam et autres règles de plateforme (sortie commerciale 2026)

### Takeaway
Steam exige de déclarer l'IA seulement pour le **contenu livré avec le jeu et consommé par les joueurs** (art, son, narration, localisation). Les outils qui améliorent l'efficacité du développement (ex. Copilot) sont hors périmètre depuis la révision de janvier 2026. Le contenu pré-généré est évalué comme du contenu non IA ; le contenu généré en direct demande des garde-fous.

### Cited Findings
- Questionnaire de contenu Steam : section IA générative ; « Pre-Generated » (créé avec l'aide d'outils IA pendant le développement) évalué comme du contenu non IA ; « Live-Generated » (créé pendant l'exécution) exige de décrire les garde-fous ; le contenu sexuel Adults Only généré en direct est interdit ; « Efficiency gains through the use of these tools is not the focus of this section ». [H] — [Steamworks](https://partner.steamgames.com/doc/gettingstarted/contentsurvey#aicontent)
- Révision de janvier 2026 : le formulaire précise qu'il vise l'IA utilisée pour créer « content that ships with your game, and is consumed by players » (artwork, sound, narrative, localization) ; les outils internes du développeur sont hors champ ; une déclaration sur l'IA utilisée pour la page de store reste requise. [M] — [GamingOnLinux](https://www.gamingonlinux.com/2026/01/valve-tweak-steam-ai-disclosure-form-for-developers-to-clarify-its-for-content-consumed-by-players/), [PC Gamer](https://www.pcgamer.com/software/ai/steam-updates-ai-disclosure-form-to-specify-that-its-focused-on-ai-generated-content-that-is-consumed-by-players-not-efficiency-tools-used-behind-the-scenes/)
- Les divulgations s'affichent publiquement sur la page Steam ; un bouton de signalement dans l'overlay existe pour le contenu illégal des jeux à génération en direct. [M] — [Outlook Respawn](https://respawn.outlookindia.com/amp/story/gaming/gaming-news/valve-clarifies-steam-ai-policy-focus-shifts-to-content-consumed)
- ElevenLabs Music impose à l'utilisateur de respecter les règles de divulgation IA de chaque plateforme (§ 4.4). [H] — [elevenlabs.io/music-terms](https://elevenlabs.io/music-terms)
- Fab : étiquette « CreatedWithAI » pour les vendeurs de contenu (applicable si on vend des assets, pas si on les achète). [M] — [80.lv](https://80.lv/articles/epic-games-adresses-ai-generated-content-plaguing-fab)

### Inferences
- Pour ce projet : cocher « Pre-Generated » dès que de l'art, de la musique, des SFX ou des voix IA sont livrés dans le jeu ; conserver un registre par asset (outil, date, plan d'abonnement) pour justifier la licence en cas de litige ou de relecture Valve.
- Rien de trouvé sur les autres plateformes (Epic Games Store, GOG, consoles) : voir Gaps.

### Gaps
- Dates exactes et texte intégral de la révision Valve de 2026 (la page Steamworks lue n'indique pas de date) ; les blogs tech-insider/finalboss/tbreak contenaient des chiffres incohérents et n'ont pas été utilisés.
- Règles IA de l'Epic Games Store, GOG, consoles, et réglementations (UE AI Act obligations de transparence, 2026) : non couvertes.

---

## 5. Ce qui est pilotable en CLI/API par un agent

### Takeaway
Pilotables de façon propre : ElevenLabs (SFX, musique — API Pro+ pour PCM 44,1 kHz), BFL (API d'images), OpenAI (images), Scenario (API déclarée complète), ComfyUI (HTTP local, JSON), Freesound (API). Non pilotables officiellement : Midjourney. Suno : API officielle non confirmée dans mes sources.

### Cited Findings
- ElevenLabs : API documentée pour SFX (voir § 3) ; la grille de prix mentionne la sortie « 44.1kHz PCM audio output via API » dès Pro ; page d'API séparée. [H] — [elevenlabs.io/pricing](https://elevenlabs.io/pricing), [API SFX](https://elevenlabs.io/docs/api-reference/text-to-sound-effects/convert)
- BFL : API hébergée pour pro/max/flex/klein, endpoints fine-tunés, traitement par lots. [H] — [docs.bfl.ai](https://docs.bfl.ai/quick_start/pricing)
- OpenAI : API images avec `background: "transparent"`. [H] — [developers.openai.com](https://developers.openai.com/api/docs/guides/image-generation)
- Scenario : API « accès à tout ce que fait l'appli », entraînement de modèles inclus. [H] — [docs.scenario.com](https://docs.scenario.com/)
- ComfyUI : API HTTP locale + workflows JSON. [M] — [docs.comfy.org](https://docs.comfy.org/development/comfyui-server/comms_overview?ref=toolarium.ru)
- Freesound : API existante avec filtres de licence. [H] — [freesound.org/help/faq](https://freesound.org/help/faq/)
- Midjourney : pas d'API officielle (voir § 1). [B] — [unifically.com](https://unifically.com/blogs/midjourney-api)

### Inferences
- Pipeline suggéré pour agent : (1) icônes/portraits : ComfyUI + klein 4B (ou API BFL / gpt-image) via scripts ; (2) SFX : API ElevenLabs en lots avec un manifeste (prompt, durée, plan) conservé comme preuve de licence ; (3) musique : ElevenLabs Music (API Pro+) ou génération manuelle ; (4) import dans Unreal via scripts Python/commandlets (hors périmètre de cette recherche).

### Gaps
- Existence et conditions d'une API officielle Suno/Udio/Stable Audio (Stability a une API selon des sources secondaires) : non confirmées par source primaire.
- Limites de débit, coûts effectifs et fiabilité en lot de chaque API : non mesurés.
