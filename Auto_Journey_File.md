# Auto_Journey : la file de travail

Procédure : `Auto_Journey.md` (section « La file de travail »). États : `[ ]` à faire, `[~]` en cours, `[x]` faite, `[?]` attend l'utilisateur (voir `Auto_Journey_Retours.md`), `[B]` bloquée par une autre tâche.
Une tâche = une phrase, le journal à lire, le premier test rouge, 15-30 min. Le dernier agent d'un worktree ajoute la suivante qu'il voit venir.
**Semée le 2026-10-30 à 18:10 d'après les rapports du test 1 : l'étape 0 doit la confronter à l'état réel avant de lancer.**

## Arwen — `ai-ia1` — IA et Volcan (journaux : `AIInProgress.md`, `VolcanoInProgress.md`)
- [ ] IA4 D579 : lire le niveau de l'emplacement (réponse de l'utilisateur 2026-10-30 : à faire, **sans autre décision**) : config .json `difficulty`, `FHOLMatchConfig.Slots[i].Difficulty` -> `Level` dans `FHOLAISummary` (C++ `Read`, test rouge d'abord) ; aujourd'hui `HOLAIBrain::LevelOf` renvoie Normal, donc Facile n'est joué qu'en test ; un scénario `.json` avec `"difficulty": "Easy"` peut servir à l'essai (`AIInProgress.md`)
- [ ] IA4 D579 (D585, réponse de l'utilisateur 2026-10-30 = B) : Facile rappelle aussi la vague d'attaque dès que le forum est attaqué (`ThinkArmy`, `DefendAnyway`) ; test rouge d'abord sur les tables réelles : une vague en route + forum attaqué -> Facile la rappelle, Normal non ; plus lourd à tester (retour d'une vague en route) (`AIInProgress.md`)
- [ ] IA4 Paysans (D584, réponse de l'utilisateur 2026-10-30 = B) : un Paysan n'achève le blessé que si aucun autre soldat ennemi sain n'est à moins de ~5 m (500 cm) du blessé (`StrikeTargetFor` de `Script/AI/HOLAIArmy.as`) ; test rouge d'abord sur les tables réelles : blessé isolé = frappe, blessé entouré de 3 sains = ne frappe pas ; le rayon est un réglage (`AIInProgress.md`)
- [x] Réponses IA/Volcan enregistrées (D571-D575 au GDD, prochain **D576**) ; aide aux coéquipiers (2665240, testée en règle, aucune partie 2v2 jouée) ; Volcan V2 (ae476b5, 493/493)
- [x] Volcan V3 faite (d7e78d0, tirage libre sans mémoire, main 84299fe) ; reste : V4 et suivantes du plan V0-V12 qui ne demandent pas d'essai
- [x] Volcan V-Q1 = A (D580, valeurs provisoires, à régler en vraie partie)
- [x] IA4 arbitrage des vies D426 : tranché, D579 (voir la tâche suivante)
- [x] FAITE le 2026-10-30 (5cf3d39, dans main 84299fe, tests 560/560 ; question B ouverte dans Retours § 2 : tenir compte des autres soldats autour du blessé) IA4 Paysans (D572 **CONFIRMÉE**, option A, 2026-10-30) : un Paysan achève un soldat ennemi seulement si le duel (rapport de force D553, PV restants compris) est gagné avec une marge >= 1,5 (un Lancier à 5 PV oui, un soldat sain jamais) ; sinon il fuit ou travaille ; test rouge d'abord sur la vraie chaîne (AIInProgress.md)
- [x] (sous-étape 1 faite le 2026-10-30 20:50, 9d69bd1 dans main, tests 564/564 : règle pure Facile + journal « forum attaqué » ; test rouge puis vert sur les tables réelles) IA4 arbitrage des vies selon la difficulté (D579) : Facile = sacrifie toujours l'armée ; Normal = feintes, sauve l'économie, refait plus loin dès qu'elle sait perdre ; Difficile = snipe, évalue sa perte, peut laisser la seconde vie pour écraser avant la capture puis punir chez l'attaquant ; journaliser chaque situation « forum attaqué » (vies, rapport de force, décision) ; réglages à régler en test (AIInProgress.md)
- [ ] IA4 D579 Normal : feintes (reculer, contourner), sacrifie l'armée mais pas l'économie, refait économie et armée plus loin dès qu'elle sait perdre (rien d'écrit : la décision `Rules` s'applique)
- [ ] IA4 D579 Difficile : snipe les faibles, évalue sa perte, peut laisser la seconde vie pour garder assez d'armée et écraser avant la capture, puis punit chez l'attaquant (rien d'écrit)
- [ ] IA4 D579 Évolutive (D329) : passe de Facile à Difficile avec le temps (rien d'écrit)
- [ ] Technologies (GDD § 11) ou Arbre de talents (§ 9.6) : ouvrir le `*InProgress.md` et le plan si la file se vide

## Faramir — `fog` — Brouillard (`FogInProgress.md`)
- [ ] **Lignes d'ordre qui grésillent (retour de l'essai `fog_orders`, 2026-10-30, enregistré dans `FogInProgress.md`)** : « ça grésille un peu sur la ligne qui se trouve dans l'une des deux calques du brouillard » ; épaisseur : « essayons plus épais si ça permet d'éviter cela » (6,25 cm écarté) : (1) diagnostiquer le grésillement dans les deux couches (pochoir personnalisé `r.CustomDepth=3` / `M_FogPost` B-spline / ligne dessinée deux fois), test rouge d'abord si possible (capture hors écran avant / après), (2) tester 12,5 cm par `hol.Orders.LineWidth` (option B : seulement sous brouillard actif, ou C : partout), (3) refaire l'essai `fog_orders` pour l'utilisateur ; teinte des fantômes = B (0,6) tranchée, rien à changer
- [x] **BUG EN TÊTE (retour de l'utilisateur, 2026-10-30) : corrigé le 2026-10-30 20:16 (Faramir, 062c70e dans `main`)** : cause = le scénario `fog_orders` coupait lui-même le brouillard (`hol.Fog.Render 0` pour une capture de comparaison, ~1 s après les ordres), pas le pochoir ni les lignes d'ordre ; rouge d'abord (`waitlog Fog render: mode 1` en timeout), vert ensuite ; essai `fog_orders` posé dans Retours section 1. **À confirmer par l'utilisateur sur son écran** (le jeu réel n'a pas montré de second bug dans les captures hors écran).
- [x] Bords du brouillard moins « rough » — faite (B-spline, 84386a7), **bords ronds validés** par l'utilisateur (2026-10-30)
- [x] Les lignes d'ordre restent visibles — faite (84386a7, 30b5a0c) mais **non vérifiée par l'utilisateur** (essai `fog_orders` prêt dans Retours depuis la correction du bug) ; épaisseur : voir la ligne `[ ]` en tête (réponse du 2026-10-30)
- [x] F7.3 (b) fantômes de bâtiments teintés (solo / hôte) : faite et commitée (Faramir, b5c3a1f + 87368e4, `fog`, 2026-10-30 20:44) : règle pure rouge d'abord, `RefreshBuildingMemory`, `ELook` lu par la vue, teinte au cvar `hol.Fog.GhostTint` (question dans Retours section 2), scénario `fog_ghosts` (essai section 1) ; **pas de fantôme chez un client réseau** (à faire : acteurs fantômes) ; (c) arbres coupés hors de vue : non commencé
- [ ] F7.3 (c) arbres coupés hors de vue (fantômes de sources) ; et fantômes de bâtiments chez un client réseau (acteurs fantômes)
- [ ] F8, F9 ; texture de cases et lecteur pour la mini-carte du prototype (façon AoE4, décision du 2026-10-30)
- [ ] Mesures S5 (F6f.2) à la clôture, pas pendant un test

## Haldir — `maps` — Cartes (`MapsInProgress.md`)
- [ ] **Bosquets compacts, arbres qui se touchent (réponse de l'utilisateur à l'essai `tree_spacing` et à M-Q10c, 2026-10-30, enregistrée dans `MapsInProgress.md`)** : « 1.4 est mieux mais une forêt doit être comme tous les arbres qui se touchent, compacte, un minimum de blanc entre les arbres » ; « jusqu'à ce qui se touche » : régler `GROVE_SPACING` de `Tools/Editor/maps/common.py` vers le contact des troncs (<= 1,4 m) ; mesurer les conséquences sur le budget d'arbres (Immense jusqu'à 450 à 1,4 m), les paquets, la navigation et le passage entre les troncs (`hinterland_paths.ps1`, `crossings.ps1`) et `Map.Table` (test rouge d'abord) ; **si le passage des unités ou les budgets de D564 sont en jeu, poser une VRAIE question à l'utilisateur dans `Auto_Journey_Retours.md` avant de trancher**
- [x] Arbres : quantité x2 et plus serrés (0ae2591, réponse de l'utilisateur à l'essai Arrière-pays)
- [x] (fait 788ae13 sur `maps`, 2026-10-30 soir : mesure seule, tests 530/530) M-Q10b = A (2026-10-30, D578) : grands espaces vides acceptés ; retirer la règle du cercle vide (contrôle `EmptyRuleHeld` de `Map.Table` : supprimer ou passer en simple mesure) ; à rediscuter quand le gibier et les points d'intérêt existent (`MapsInProgress.md`)
- [x] (fait 788ae13 : scénario `tree_spacing` + commande `hol.Sources.Grove`, démarré hors écran sans erreur, essai posé dans Retours section 1) Scénario ou essai in-game pour juger l'espacement des arbres (1,4 / 1,6 / 1,9 m côte à côte) : l'utilisateur ne peut pas lire une capture ; M-Q10c répondue le 2026-10-30 (ligne `[ ]` en tête) (`GROVE_SPACING` de `Tools/Editor/maps/common.py`)
- [x] M-Q3 = A (2026-10-30) : le pont des Gués tombé est un chantier qu'un Paysan relève sans coût (D496 confirmée), à coder avec la carte Les Gués
- [ ] Règle de décor (D576) : chaque carte son identité propre si elle aide à lire le terrain (rochers en montagne, sable ou terre brune en aride, sable / roseaux près de l'eau), jamais de généralité ni de remplissage ; à décider carte par carte à sa création
- [x] (périmée : M-Q10 commité avec 0ae2591, arbre propre) M-Q10 non commité : le ranger dans un état cohérent et vert (ou le séparer) avant toute autre chose
- [ ] C3 El Dorado (M-Q2 = A, effets de terrain visibles seulement)
- [x] (fait 2026-10-30 20:52 sur `maps`, `Map.Table` rouge puis vert, 543/543, sizes PASS ; reste `aiduel`/chemins de Plaine non revérifiés) **Plaine remise au budget standard (D564) et au sol uniforme (confirmé, D576)**
- [x] (fait 2026-10-30 20:35 : paths PASS 5 tailles, aiduel seul rouge connu des empreintes) `hinterland_paths.ps1` et `quick_check -Parts aiduel` sur les cartes sans décor

## Elrond — `tiers` — Paliers (`VictoryInProgress.md`)
- [ ] V4 messages (réponse de l'utilisateur 2026-10-30 = B) : élision française dans les messages de victoire (« Le forum d'Aragorn », « Le Roi d'Aragorn » au lieu de « de Aragorn ») : règle pure avec test rouge d'abord (« de » + voyelle ou h muet -> « d' »), **français seulement** (anglais et japonais inchangés) ; `Source/HeroesOfLiberty/Match/HOLForumMessages.cpp` (`MessageFell`, `MessageHeld`, `MessageKingDied`) ; `VictoryInProgress.md`
- [x] Médaillons d'âge revérifiés au scénario `tiers` : « Validé » (2026-10-30), P4 validé
- [x] `-Parts victory` rejoué vert sur `tiers` = `main` 436096c (37 s) et `build:tiers` vert (28 s), passages séparés (2026-10-30 20:20) ; `-Parts net` reste à rejouer (paquet/serveur interdits ce jour)
- [x] P5 : D569 complété au GDD (2026-10-30 20:15, GDD non commité)

## Elrond — `tiers` — Talents (`TalentsInProgress.md`, ouvert 2026-10-30)
- [ ] **Modifier le cri *Serrez les rangs* (D581, TQ2 = A, réponse de l'utilisateur 2026-10-30, changement de règle du cri)** : le cri met vraiment les unités en formation défensive (immobiles ou vitesse réduite pendant le cri, GDD § 13.2) en plus de la réduction de dégâts ; puis le talent *Pas de l'Aube* leur laisse la réduction en avançant à vitesse réduite ; test rouge d'abord sur la vraie chaîne (`HeroAbility.InMatch`, processeur de mouvement) ; **avec l'aide de Celeborn si c'est le code du héros** (`HOLHeroAbilityRules`, J8c) ; `TalentsInProgress.md`
- [x] T0 relevé : aucun code de talent ; plan T0-T7 écrit, ligne dans `EtapeDefini.md`
- [x] T1 règles pures en script (`Script/Talents/HOLTalentRules.as`, 12 tests, rouge puis vert) et T2 table `Tools/Data/Talents.csv` (24 talents, test C++ `Talents.Csv`) : commits 5cf8c1b, 41f067f, `tests` 543/543, **dans `main` 1ff1ad5** (2026-10-30 20:24)
- [x] T3 état du héros : choix par emplacement dans la fiche (C++), ordre réseau « choisir un talent » (`ServerHeroTalent`) refusé par `HOLHeroTalent::CanPick` (règle C++ qui doit rester égale à celle du script : test `Test_TalentServerAndScriptRulesAgree`), test `HeroTalent.Orders`, commit baafc55 sur `tiers`, `tests` 545/545 ; TQ1 en attente (Retours, « Talents »)
- [x] T4a *Boucliers levés* (Capitaine, rangée 2 : +1 armure dans l'aura, `HOLCombat::GetDamage(..., DefenderArmorBonus)`, test `HeroTalent.ShieldsUp`, rouge puis vert, 798e38f sur `tiers`)
- [ ] T4b reste : *Pas de l'Aube* : TQ2 = A tranchée (D581), voir la ligne `[ ]` en tête ; reste l'écran T5 (le script appelle `RequestHeroTalent`, lit `GetHeroTalentCount/Family/Seals/Keys`)

## Celeborn — `hero-j8-close` — Clôture de J8 (`JourneyInProgress.md`)
- [x] **Glissement du rang : demi-places réunies, poussée sans prise de place, jamais hors du combat** *(codé et commité 2026-10-30 20:2x, `hol.Combat.RankSlide`, test `Combat.RankSlide`, scénario `rank_slide`, D517 réconcilié au GDD D577 ; essai `rank_slide` accepté (« c'est très bien comme ça [...] suffisant pour l'instant ») et « qui pousse » = A (2026-10-30, enregistrés dans `JourneyInProgress.md`) ; reste la fusion)* (D577, décision de l'utilisateur sur l'essai `surplus`, ses mots dans `JourneyInProgress.md`) : un soldat de derrière qui voit un début de place pousse les lignes devant lui sans prendre leur place, le rang glisse à gauche ou à droite de la cible et les deux demi-places deviennent une place ; un frappeur n'est jamais poussé hors du combat, et la première ligne ne se décale pas si cela l'empêche de taper (même en changeant de cible). Réconcilier avec D517 (pas de déplacement de côté). Test rouge d'abord sur la vraie chaîne ; le scénario `surplus` (`hol.Combat.SurplusReassign 1/0`) sert à l'essai
- [x] `quick_check -Parts hero` PASS (115 s, 2026-10-30 20:23) ; `-Parts tests,combat` PASS (531/531) ; `-Parts move,formation,combat` PASS (103 s) ; après `git merge main` (1ff1ad5) recompilé : `-Parts tests,combat` PASS 544/544 (20:42). **Fusion dans `main` NON faite** : `main` a avancé (84299fe) et son arbre a un `GDD.md` modifié non commité ; `hero-j8-close` = 678a7ac, à fusionner par la session de fusion (merge de `main` puis recompiler/retester)
- [x] Essai du scénario `surplus` fait : « c'est mieux » mais règle de glissement du rang demandée (tâche ci-dessus, D577) ; le scénario reste utilisable
- [ ] Mesures au repos C2 (`Tools/Tests/rest_measures.ps1`), seules, hors test
- [ ] Paquet et serveur à recuire (C4), seuls, hors test
- [ ] Sons du cri et de l'étendard (provisoires)

## Galadriel — `galadriel-tab` — Interface et divers (`InterfaceInProgress.md`)
- [x] Essai du scénario `tab` : « Tout est bon » (2026-10-30), correctif des ordres après Tab validé ; fusion de `galadriel-tab` dans `main` faite (f0c7b06)
- [ ] Mini-carte du prototype façon AoE4 (camps en couleurs, sources explorées, cadre de caméra, clic gauche = caméra, clic droit = déplacement, alertes qui clignotent ; suit le brouillard) — après la texture de cases de Faramir
- [x] Barre du bas trop large sous 1500 px : déjà faite (étape T, 2026-10-29) ; garde-fou `build:barwidth` élargi à 800x600 et 1024x768, PASS (76a76e8, dans main)
- [x] Test de contrôleur du double clic « même type » caché (régression F7.3a) : partie `fogclick` de `quick_check`, rouge d'abord puis vert (76a76e8, dans main c28179f)

## Galadriel — `galadriel-tab` — Marché (`MarketInProgress.md`)
- [ ] **MK-Q1 = C (D582, réponse de l'utilisateur 2026-10-30)** : ajouter le jeu C à `HOLMarket::Preset` dans `Script/Economy/HOLMarketRules.as` (prix de base nourriture 80, bois 80, pierre 120, or 100 ; taxe 30 %, 3 points par lot, retour 1 point / 10 s, bornes 20-300) et en faire le jeu par défaut ; test rouge d'abord (prix de base, aller-retour chiffré) ; puis M2 (table CSV) ; `MarketInProgress.md`
- [x] Chapitre ouvert (2026-10-30 20:30) : M0 relevé (aucun code Marché), plan M1-M6 ; M1 règle pure en script (`Script/Economy/HOLMarketRules.as`, 11 tests, rouge d'abord 530/541 puis vert 554/554), commit 7be2ba9, dans `main` (7293e52)
- [x] Question MK-Q1 tranchée le 2026-10-30 : C (D582), voir la ligne en tête
- [ ] M2 table CSV (après MK-Q1), M3 type de bâtiment `Market` (âge 1), M4 cours commun en partie + commande, M5 interface, M6 IA et scénario `market`

## Glorfindel — `net-playtest` — Réseau (`NetPlaytestInProgress.md`)
- Rien d'ouvert avant l'essai E5 avec un ami ; seulement si un reste apparaît (remesure MaxServers D502, r9, R8d).

## Glorfindel — `net-playtest` — Technologies (`TechnologiesInProgress.md`, GDD § 11)
- [ ] **Appliquer les réponses T-Q1 à T-Q6 (D583, 2026-10-30)** à `Tools/Data/Technologies.csv` (branche `net-playtest`) : **placer Brouette à la Ferme** (et Charrue à la Ferme, pas au Centre principal), *Liturgie* et *Bénédiction des armes* au Sanctuaire, *Hallebardes trempées* à la Caserne, *Vœu de pauvreté* à la Chapelle ; palier seul (`MinHeroLevel` = 0) ; table sans palier 3 ; test `Technologies.Table` adapté d'abord (rouge) ; puis T3 (état de recherche) avec une file de 5 (T-Q6 = A, `MaxQueue` D283), en codant d'abord les 11 technologies jouables (camps, ferme, Centre) ; `TechnologiesInProgress.md`
- [x] T0 : relevé de l'existant (8 types de bâtiments, aucune commande « Rechercher », palier et niveau du héros lisibles) ; plan T1-T8 ; questions T-Q1 à T-Q6 posées (Retours, section 2)
- [x] T1 : table `Tools/Data/Technologies.csv` (27 lignes, paliers 1 et 2) + `FHOLTechnologyRow` + `make_technologies.py` (non lancé) + test `HeroesOfLiberty.Technologies.Table` (rouge 20:43, vert 20:48) ; commit 0ad1d13 sur `net-playtest`
- [x] T2 : règle pure de disponibilité en Angelscript (`HOLTechnologyRules.as`, 5 tests) ; `MinTier`/`MinHeroLevel` prévus pour T-Q1
- [ ] Fusion dans `main` : à refaire (recompiler après la fusion de `main` 7e6778a, `quick_check -Parts tests`, fast-forward)
- [ ] T3 à T8 : état de recherche, file et commande, effets (économie d'abord), Aube, IA, interface

## Fusions (agent de fusion, seulement si l'énoncé les autorise)
- [x] Fusions faites le 2026-10-30 (19:20) : `ai-ia1`, `galadriel-tab`, `tiers`, `hero-j8-close`, `fog`, `maps` toutes dans `main` (f0c7b06), tests verts ; prochaines fusions : seulement les nouveaux commits de ces branches (ordre `ai-ia1`, `galadriel-tab`, `tiers`, `hero-j8-close`, `fog`, `maps`)
