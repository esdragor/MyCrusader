# Étapes définies

Les titres seulement : ce qui a déjà été abordé et peut être fait, et les bugs remontés.
Une ligne disparaît dès que c'est fait ou corrigé. Le détail est dans les `*InProgress.md`.

**Chaque session met ce fichier à jour à chaque avancée** (question tranchée, boucle faite, commit, fusion, bug trouvé
ou corrigé), en même temps que son journal, dans `C:\Users\baudo\Documents\MyCrusader\EtapeDefini.md` (chemin absolu,
même depuis un worktree). Relire juste avant d'éditer et ne toucher que ses lignes.

*Nettoyé le 2026-11-04 16:3x (passe de nettoyage : sauvegarde de l'ancienne version dans le scratchpad de la session).
« (à vérifier) » = point que la passe n'a pas pu confirmer.*

## État des branches (2026-11-04 16:3x, `main` = 1f5d8d8, poussé jusqu'à b284036)

- **Dans `main`** (fusion groupée 532e638 le 2026-11-04) : `ai-ia1` (Arwen), `tiers` (Elrond), `fog` (Faramir), `net-playtest` (Glorfindel), `galadriel-tab` (Galadriel). Les cinq worktrees sont à 1f5d8d8, libres côté code (Faramir : la partie C++ de B2 est commitée sur `fog` 19adf23, non compilée, conflit avec main à résoudre).
- **Fusion groupée `tiers` (Elrond, 1a473c6) = galadriel-tab + fog B2 + maps autel + Cavalerie C1 (assets régénérés), VERTE 936/936 + net, PAS ENCORE dans `main`** (2026-11-04 17:5x) : l'avance rapide attend que la session Légions commite `GDD.md` ; voir `Auto_Journey_Retours.md` § 3.
- **PAS dans `main`** : `maps` (Haldir, 9387cd3, voir Cartes), `visual` (Sauron, 3b3247c, voir Visuel), `hero-j8-close` (Celeborn, 09cb852, voir Duel et Bugs).
- Essais validés le 2026-11-04 : spectateur d'équipe, `diplomacy_compare`, volcan, `talents_later`. Seule attente de l'utilisateur dans `Retours\` : `E-maps-review.md`.

## À faire

### En tête de file (Galadriel, `Auto_Journey_File.md`)
- Diplomatie, retours de l'essai `E-diplomacy` et `E-diplomacy_locked` (2026-11-04 15:55) : voir « Bugs remontés » ; tête de file de Galadriel. **17:03 : fait sur `galadriel-tab` (commit 9f64436 + scénarios) : rupture mutuelle (bug), « En attente » supprimé, taxe écrite une fois, tribut par ressource ; build PASS, quick_check tests+net 921/921 PASS, pré-vols `diplomacy` et `diplomacy_locked` PASS ; essais `Retours/E-diplomacy.md` et `E-diplomacy_locked.md` ; pas fusionné dans `main` (`DiplomacyInProgress.md`).**
- Outils de test (Arwen, `ai-ia1`, 2026-11-04 17:0x) : défilement par le bord coupé dans `check.ps1` et `quick_check.ps1` (`bEdgeScroll=False`), `-Multiprocess` dans `quick_check.ps1`, `crossings.ps1`, `hinterland_paths.ps1` ; `log_lock_test`, `check sanity_ok`, `quick_check -Parts tests` PASS ; commité sur `ai-ia1`, pas dans `main`.
- Décisions numérotées au GDD le 2026-11-04 (prochain numéro **D672**) : D663-D665 (arbres 400, Steppe x2 et vraies forêts, sols et autel, de Haldir), D666 (VI-Q3 = B, bannières), D667-D670 (TB-Q1, TB-Q2, MM-Q1, AIQ-H), D671 (réponses Diplomatie du 15:55). D658 confirmée par l'utilisateur (la ligne est dans la branche `maps`).

### Animations
- Variantes de chutes par type (2 ou 3, choisies par unité) *(après le prototype)*

### Unités et héros
- **Légions Noires, la 2e faction du prototype (D11, § 13.3), 2026-11-04 : dossier COMPLET, PRÊT À LANCER, RIEN codé** ; attend un worktree (l'utilisateur le prépare) ; L0 socle multi-faction (rôles + surcharges par faction) puis Aube contre Légions jouable (Zombie, socle en reskin, Guerrier Damné, fiche du Seigneur Damné), puis cadavres / Nécromancien / Squelette, Spectre, spécialisations, technologies, IA, art, essais (L0 à L14) ; kit du héros (L8b) et talents (L11) après la clôture du chantier héros ; décisions à numéroter au GDD ; LQ-1 à LQ-14 TOUTES tranchées (Zombie sans nourriture en coût, deux malédictions, 1 cadavre = 1 Squelette, cadavres 75 s / 60 par joueur, Autel de sang et Fosse des damnés = bâtiments, Fosses de labeur et Tour des nécromanciens = technologies) ; reste les chiffres, proposés à chaque étape *(`LegionsInProgress.md`)*
- Cavalerie (D672, 2026-11-04) : CV-Q1 à CV-Q5 tranchées (charge oui, cheval placeholder, stats en bloc, IA à tous les niveaux, bannières) ; C1 (Écurie + Cavalier léger en données et script) FAITE et DANS `main` (fusion 9f9c591, 2026-11-04), NON compilée, NON exécutée, rien de poussé ; Glorfindel libéré ; reprise = liste des commandes de `CavalryInProgress.md` quand le build sera permis ; C2 (LOCTEXT, charge, forêt, tests C++ de la grille : C++) attend l'autorisation de builder ; bannières de l'Écurie avec le chantier visuel *(`CavalryInProgress.md`)*
- Deuxième attaque à déclencheur (charge, D345) : C2 de la cavalerie, C++
- Clôture de J8 (héros) : J8a, J8b, J8c et leurs correctifs sont dans `main` ; reste les mesures au repos (S2 3,05 ms contre le seuil 3, bissection commencée dans Elrond ; seuil S3 / S4 relevé à 6,5 ms, D586 : `chapter_measures.ps1` à ajuster (à vérifier)) et le paquet / serveur à recuire *(`JourneyInProgress.md`, `Docs/Mesures_2026-10-31.md`)*
- Glissement du rang des Hommes d'armes en surnombre (D577) : dans `main`, rien à faire.

### Fin de partie et paliers
- Conditions de victoire restantes : Nomade (D354, D207), garnison du Roi (D365, avec la garnison) ; vies par palier et paliers P1-P5 dans `main` ; `net` (3 vies chez le client 2) non rejoué (à vérifier) *(`VictoryInProgress.md`)*

### Cartes (branche `maps`, Haldir, PAS fusionnée)
- Les sept cartes du prototype (D496, D498) : Steppe, Arrière-pays, El Dorado, Cols, Forêt profonde, Couronne, Gués faites ; site du volcan et relais (5 fixes de 12 m + anneau, D647 / D658) écrits sur toutes ; **40 cartes régénérées le 2026-11-04 : budget d'arbres 400 par joueur (Immense 600), forêts de 200 arbres ou plus, sol texturé par carte (CC0), ambiance lumineuse** *(`MapsInProgress.md`, `Docs\AssetsExternes.md`)*
- Autel du volcan sur Les Gués (2 joueurs) : tranché D673 (2026-11-04), sur le pont du milieu ; à coder dans `fords.py` *(`MapsInProgress.md`)*
- 2026-11-04 17:03 : build du worktree PASS, `quick_check -Parts tests,sizes` 922/922, autel VU À L'ÉCRAN (`maps_review`), Steppe Immense régénérée, commit 14a9bf0 (branche `maps`)
- Reste avant fusion : `crossings` / `hinterland_paths` / `aiduel` non rejoués après la régénération ; autel des Gués à mettre sur le pont (D673) ; essai `E-maps-review` (Entrée = carte suivante) en attente de l'utilisateur
- El Dorado : marais 30 m à 60 % et tout l'or au centre figés (D674, 2026-11-04). Sols : une teinte distincte par carte (D675, à coder, valider par `maps_review`). Non mesuré : coût en i/s du sol CC0 à 2 000 unités non mesuré

### Volcan et option Catastrophe *(D205, D208, D377-D380, D398-D400, D427, D649, `VolcanoInProgress.md`)*
- V1-V8, V10-V12, gisement neuf tiré de la graine (D649), réplication du site au client (V9a), IA qui évacue et tient l'autel : dans `main`, essai volcan validé
- Reste V9b (traces au sol de l'alerte), V9c (lave, basalte : aujourd'hui ni lave ni autel dessinés en jeu), V4b sur le chargement des cartes (avec `maps`) ; valeurs du gisement provisoires

### Technologies et Moines *(GDD § 11, `TechnologiesInProgress.md`)*
- T1-T7 (table, recherche, effets, IA avec retards D648 / D657), Chapelle et Moine, « Bientôt » : dans `main`
- Reste : icônes (attendent l'art), WorkerDefence / GarrisonSpeed (T-Q9, T-Q10), soin du Moine vu à l'écran d'un client réel (à vérifier)

### Brouillard de guerre *(`FogInProgress.md`)*
- F0-F9 dans `main` ; reste `-Full` et mesures de clôture, essai de l'utilisateur sur les fantômes / arbres (à vérifier), contrôle `netfogview` (coupe d'arbres vue) écrit, à jouer ; image du brouillard d'un éliminé non vue

### Talents du héros *(`TalentsInProgress.md`)*
- T1-T6, fenêtre d'arbre (TQ4 = A), rangées 7-9 grisées « plus tard », profil de l'IA 5 + 3 et table de contres D662 : dans `main`
- Reste : adaptation de l'IA aux sceaux repérés codée mais DÉSACTIVÉE (variable `hol.AI.TalentAdapt` à porter en C++, éclairage des héros adverses absent) ; collecte de textes `HOLTalentTree,Later` à refaire ; effets des rangées 7-9 et courbe d'XP (après le prototype) ; *Étendard du rempart* et *Double étendard* (briques absentes) ; liste des passifs pris (TQ5)

### Relais neutres et centres secondaires *(`RelaysInProgress.md`)*
- R0-R6 dans `main` ; reste la vérification en vrai réseau à 2 clients, essai `relays_map` à rejouer sur les cartes de `maps`, RQ3 (IA des relais : défaut B provisoire) (à vérifier)

### Unités neutres et camps *(`NeutralsInProgress.md`)*
- N1-N5 et table `Camps.csv` dans `main` ; reste N2 (placement dans les cartes), N6 (IA du joueur), N7 (affichage), NQ1 / NQ2 (à vue ou sur coup, à qui va l'XP : réglages derrière `hol.Camp.Aggro` / `Credit`) (à vérifier)

### Duel de héros *(GDD § 10, `DuelInProgress.md`)*
- D1-D5, D4 (défier / répondre), D8 (fenêtre), D9 (IA), cercle dessiné et sortie des non-duellistes (`hol.Duel.ClearCircle`) : dans `main`
- Reste : capacités propres des héros (D121, D122), récompenses de fin de duel branchées (D7), textes de la fenêtre non localisés (à vérifier), ordre d'origine non repris après `ClearCircle` (à vérifier)

### Marché et tributs *(GDD § 5.5, D372, `MarketInProgress.md`)*
- Marché (A Z E acheter / Q S D vendre), IA Difficile, tributs, alertes de la mini-carte : dans `main`, essai validé
- Reste : icône d'or (« 100 [icône or] -> X », avec les icônes de ressources), MK-Q4 (pas du cours à l'achat), tribut à toutes les ressources (voir Bugs)

### Siège *(GDD § 6.1, `SiegeInProgress.md`)*
- S0-S8 dans `main` (Bélier, Mangonneau, Atelier de siège, IA Difficile) ; reste `aiduel` non relancé, escorte étirée de la vague (observation), icônes ; Trébuchet / Canon / conversion après le prototype

### Biomes *(D651-D656, D660, `BiomeInProgress.md`)*
- B0 et règles pures B1 (Angelscript), B2 côté gestionnaire (champ `biome` du salon) : dans `main` (b9460b1)
- B2 côté C++ (`FHOLMatchConfig::Biome`, lobby du client, `hol.Biome.Report`) : `fog` 2b425d4 = main fusionné, compilé (PASS), `tests` 923/923, `netfogtrees` et `netfogview` PASS (2026-11-04 16:55, Faramir) ; pas dans `main` ; reste B1 visuel (sol et lumières au chargement), B3 décor au chargement, B4 (première ligne de la liste = « Défaut : <biome de la carte> », D660), B5

### Visuel des bâtiments (branche `visual`, Sauron, PAS fusionnée) *(`VisualInProgress.md`)*
- Style stylisé et franc, kit Kenney CC0 ; Forum « K » (donjon 3 x 3, quatre tourelles) retenu, couleur du joueur par bannières (VI-Q3 = B, `SM_Forum`), tests 903/903 ; correctif du contrôle de couleur de `-Parts net` (9a669f4) NON compilé, à rejouer ; reste coût S1, fusion, puis les autres bâtiments ; limites du kit pour la variété des factions (question ouverte)

### Équilibrage et modes de jeu
- Durée de construction des bâtiments divisée par 2 (D557) : commitée, validation de l'utilisateur (à vérifier)
- Mode *Battlefield* : salon où l'on choisit ses armées et où l'on apparaît avec *(après le prototype, D416)*

### Réseau et lanceur *(`NetPlaytestInProgress.md`, `LauncherInProgress.md`)*
- M6 : R1-R8c, spectateur de salon (SP1, D642), éliminé = spectateur, équipe entière tombée = toute la carte (D646), `MaxServers` 4 (D502), outillage de journaux frais (`Get-FreshLog`) : dans `main` ; reste `quick_check -Parts net,lobby` avec serveur de test recuit, `aitech` et `netspec` non rejoués, `Game.po` à recompiler, bâtiment « ruine » de l'éliminé
- **Essai E5 avec un ami (R8d), à faire plus tard** : r9 publiée, kit `HOLLauncher-kit.zip` prêt (copie sur le bureau) ; lancer `Gestionnaire HOL.bat`, envoyer le kit, jouer un salon ; seul inconnu : ports UDP 7777-7799 par la box ; ensuite R9.2
- Lanceur : L0-L14 faits, publié 0.3.0 (2026-11-03) ; reprise = L15 (réglages + langue), plan L14-L24 dans `LauncherInProgress.md` ; périmètre validé (vérifier fichiers, options, désinstaller, mise à jour automatique, fr / en / ja, etc.)

### IA *(`AIInProgress.md`)*
- IA0 à IA3 closes et IA4 (Paysans D572, arbitrage des vies D579, aide aux coéquipiers D571, niveaux Facile / Normal / Difficile / Évolutive, IAQ-S D619-D621) : dans `main`
- Reste : snipe et fermes reconstruites non vus en vraie partie ; repli de la vague punitive (AIQ-H = A, D670, rien à coder) ; IA5 (idée validée le 2026-11-04, après le prototype) : tournoi automatique IA contre IA pour régler les curseurs, parties IA contre joueur analysées, paliers Très facile / Très difficile après mesures

### Sons
- Refaire tous les sons du jeu (demande du 2026-10-29) : alertes, voix du héraut, fanfares provisoires (VSCO 2 CE, CC0) ; chantier `AudioInProgress.md` : A0-A1 faites (packs Kenney CC0 hors dépôt), suivant A2 (table événement vers son en script, dans le worktree Arwen, AU-Q2) ; AU-Q1 à AU-Q3 répondues le 2026-11-04 (musique CC0, Sonniss hors dépôt, D676)

### Performance
- Revue de performance : P1 (registre de bâtiments) et P2 (cache du terrain de la mini-carte) commités (fe5dca4) ; mesures S1 / S2 avant / après non faites ; P3-P5 en réserve *(`PerformanceInProgress.md`)*

### Interface *(`InterfaceInProgress.md`)*
- Mini-carte faite, essai validé (« très bien », meilleure résolution plus tard : terrain 480 x 480 fait, rendu 1920 x 1080 et coût non mesurés)  ; icônes de ressources à venir

### GDD
- Review du GDD : séances closes, reste la courbe d'XP (Q04, à régler en test) ; prochain numéro **D672** ; D659-D660 (valeurs fraîches, biomes) et D665 (couleurs des sols, autel non vu) « à confirmer » ; TB-Q1, TB-Q2, MM-Q1, AIQ-H numérotées D667-D670

### Moteur
- Moteur propre HeroesOfLiberty `C:\UE\UE_5.8_HOL` (avec cible Server) ; AS_3 appartient à ProjectShiva (ne pas l'écraser) ; serveur dédié cuit de nouveau *(`NetPlaytestInProgress.md`)*
- Angelscript au fil des chantiers : le neuf en script *(D483, D484 ; `AngelscriptInProgress.md`, en veille)*

## Bugs remontés

### Retours d'essai
- **Diplomatie, `E-diplomacy` (2026-11-04 15:55, PRIORITÉ, Galadriel)** : (a) BUG : un joueur passé Ennemi reste « Allié » dans l'interface de l'utilisateur (test rouge sur le livre répliqué) ; (b) RÉGLAGE : supprimer « En attente de l'autre » (passer en Ennemi prend effet directement) *(`DiplomacyInProgress.md`)*
- **Diplomatie, `E-diplomacy_locked`** : (a) la taxe de 30 % écrite UNE fois en bas du panneau, pas sur chaque bouton ; (b) le tribut propose TOUTES les ressources (bois, pierre, nourriture, or), pas seulement l'or
- Diplomatie, mineurs : présentation 2 (panneau compact) déborde sur le bloc du héros quand Triches est déplié ; auras entre alliés non faites (choix à poser) ; défilement par le bord de l'écran qui fausse les scénarios en `check` (outil)
- **Neutres rouge (`hero-j8-close`, Celeborn, PAS fusionnée)** : soldat inactif / boucles de contournement au front de la mêlée ; l'audit `neutrals` n'est pas 3 sur 3 (rouge ~6 passages sur 10) ; `ClaimPlace` (c9c98a4) et Q-neutrals-hold = C (`ClaimStuck`) codés ; correctif de camp (coups reçus entre deux pensées, 1baede3) NON compilé ; scénarios de mesure à supprimer ; `melee_loops` reste partiel *(`NeutralsInProgress.md`, `DuelInProgress.md`)*

### Aléas et petits restes
- `net 2 clients` réduit de `-Parts hero` : FAIL une fois (2026-10-29), vert au passage suivant

## Fichiers de développement

### Références
- `GDD.md` : conception du jeu, journal des décisions (§ 21) et point de reprise de la review (§ 22)
- `UnitStats.md` : tableau des stats des unités, produit par `Tools/Data/make_unit_stats.py` (ne pas modifier à la main)
- `Manager/PROTOCOL.md` : protocole du gestionnaire de salons (M6)
- `Tools/Scenarios/README.md` : scénarios de test, lancés par `play.ps1`
- `Auto_Journey.md` (procédure et leçons), `Auto_Journey_File.md` (file de travail), `Auto_Journey_Retours.md` (essais, questions, fusions), `Auto_Journey_Jour.md` (bilans) : journée autonome / séances interactives ; commits partout sauf push

### Chantiers en cours (à la racine)
- `AIInProgress.md` : IA du prototype (D421-D427, D527-D531, D552-D556, D558-D561, D571-D585, D619-D621) ; IA5 à venir
- `AudioInProgress.md` : sons (ouvert 2026-11-03, A2 suivant)
- `BiomeInProgress.md` : biomes du salon (D651-D656)
- `CavalryInProgress.md` : cavalerie (ouvert 2026-11-04, D672, C1 dans `main` (9f9c591) non compilée, C2 C++ en attente)
- `DiplomacyInProgress.md` : mode Diplomatie (D366-D376, D385-D387, D650) ; bugs de l'essai en tête de file
- `DuelInProgress.md` : duel de héros (branche `hero-j8-close` pour les boucles de mêlée)
- `FogInProgress.md` : brouillard (F0-F9 dans `main`, clôture à faire)
- `JourneyInProgress.md` : parcours du joueur et héros (J8 : clôture, mesures)
- `LegionsInProgress.md` : Légions Noires, 2e faction du prototype (dossier complet le 2026-11-04, rien codé, prêt à lancer dans un worktree donné par l'utilisateur ; L0 socle multi-faction en tête)
- `MapsInProgress.md` : les sept cartes du prototype (branche `maps`)
- `MarketInProgress.md` : Marché et tributs
- `NetPlaytestInProgress.md` : réseau M6 (salons, serveurs dédiés, spectateurs)
- `NeutralsInProgress.md` : unités neutres et camps
- `RelaysInProgress.md` : relais neutres et XP de territoire
- `SiegeInProgress.md` : siège léger
- `TalentsInProgress.md` : arbre de talents
- `TechnologiesInProgress.md` : technologies et Moines
- `VictoryInProgress.md` : conditions de victoire et paliers
- `VisualInProgress.md` : visuel des bâtiments (branche `visual`, worktree Sauron)
- `VolcanoInProgress.md` : volcan et Catastrophe
- `LauncherInProgress.md` : lanceur (clos sauf L15+ et l'essai avec un ami)

### En pause ou presque clos
- `UnitStatsInProgress.md` : stats des unités, reste la charge (D345.3) avec la cavalerie
- `AnimationsInProgress.md` : chaîne de fabrication et catalogue des animations, resservira à chaque nouvelle unité
- `AngelscriptInProgress.md` : Angelscript (D483, D484), en veille
- `InterfaceInProgress.md` : grille du Paysan (D340), mini-carte ; reste mineur
- `PerformanceInProgress.md` : revue de performance, P3-P5 en réserve
- `ScenariosInProgress.md` : mécanisme des scénarios (mode d'emploi : `Tools/Scenarios/README.md`)
- `TileSizeInProgress.md` : case de 4 m (D344) ; reste S5 (positions sur 13 bits) à mesurer à la prochaine série de mesures

### Archivés (`Docs/Archive/`)
- `FrontEndInProgress.md` : menu (D240 à D251)
- `MillInProgress.md` : moulin (D346)
- `ImplementationInProgress.md` : M0 à M2 (suite dans `JourneyInProgress.md`)
- `MovementInProgress.md` : déplacement (D405 à D415)
- `RidgeRockInProgress.md` : rochers (D439 à D444)
- `CrossbowInProgress.md` : l'Arbalétrier et le Champ de tir (D486 à D489)
- `MapSizeInProgress.md` : tailles de carte (D490 à D496)
- `TestSpeedInProgress.md` : quick_check en pas fixe et sans dessin pendant les attentes
- `EtapeDefiniInProgress.md` : lignes d'EtapeDefini prises hors des grands chantiers (A à U toutes closes, 2026-10-29)
- `Auto_Journey_Test1_2026-10-30.md` : archive du test 1 de la journée autonome
