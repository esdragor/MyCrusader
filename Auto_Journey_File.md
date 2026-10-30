# Auto_Journey : la file de travail

Procédure : `Auto_Journey.md` (section « La file de travail »). États : `[ ]` à faire, `[~]` en cours, `[x]` faite, `[?]` attend l'utilisateur (voir `Auto_Journey_Retours.md`), `[B]` bloquée par une autre tâche.
Une tâche = une phrase, le journal à lire, le premier test rouge, 15-30 min. Le dernier agent d'un worktree ajoute la suivante qu'il voit venir.
**Semée le 2026-10-30 à 18:10 d'après les rapports du test 1 : l'étape 0 doit la confronter à l'état réel avant de lancer.**

## Arwen — `ai-ia1` — IA et Volcan (journaux : `AIInProgress.md`, `VolcanoInProgress.md`)
- [x] Réponses IA/Volcan enregistrées (D571-D575 au GDD, prochain **D576**) ; aide aux coéquipiers (2665240, testée en règle, aucune partie 2v2 jouée) ; Volcan V2 (ae476b5, 493/493)
- [ ] Volcan V3 (tirage libre, tracés symétriques ; V-Q2 = B, débloquée), puis V4 et suivantes du plan V0-V12 qui ne demandent pas d'essai
- [?] Volcan V-Q1 (fenêtres de Catastrophe) : « A trancher », à discuter avec l'utilisateur
- [?] IA4 arbitrage des vies D426 : « A trancher »
- [ ] IA4 Paysans (D572 **CONFIRMÉE**, option A, 2026-10-30) : un Paysan achève un soldat ennemi seulement si le duel (rapport de force D553, PV restants compris) est gagné avec une marge >= 1,5 (un Lancier à 5 PV oui, un soldat sain jamais) ; sinon il fuit ou travaille ; test rouge d'abord sur la vraie chaîne (AIInProgress.md)
- [ ] Technologies (GDD § 11) ou Arbre de talents (§ 9.6) : ouvrir le `*InProgress.md` et le plan si la file se vide

## Faramir — `fog` — Brouillard (`FogInProgress.md`)
- [ ] **BUG EN TÊTE (retour de l'utilisateur, 2026-10-30)** : le brouillard disparaît dès que des Hommes d'armes se déplacent ou reçoivent un ordre de déplacement, et ne revient pas malgré plusieurs relances du scénario `fog_orders` (il n'a pas pu tester le reste). Suspects : pochoir `r.CustomDepth=3` / `M_FogPost` (84386a7) ou lignes d'ordre (30b5a0c). **Test rouge d'abord** sur la vraie chaîne (ordre de déplacement à une unité du camp local, puis `hol.Fog.RenderReport` et capture hors écran) ; corriger ; puis refaire un essai `fog_orders` (le poser dans Retours, avec la question de l'épaisseur des lignes d'ordre)
- [x] Bords du brouillard moins « rough » — faite (B-spline, 84386a7), **bords ronds validés** par l'utilisateur (2026-10-30)
- [x] Les lignes d'ordre restent visibles — faite (84386a7, 30b5a0c) mais **non vérifiée par l'utilisateur** (essai bloqué par le bug ci-dessus) ; épaisseur des lignes : question en attente dans Retours
- [ ] F7.3 (b) fantômes de bâtiments teintés (plan : `HOLVisionRules::BuildingDrawsFromMemory`, `ELook::Memory`) puis (c) arbres coupés hors de vue
- [ ] F8, F9 ; texture de cases et lecteur pour la mini-carte du prototype (façon AoE4, décision du 2026-10-30)
- [ ] Mesures S5 (F6f.2) à la clôture, pas pendant un test

## Haldir — `maps` — Cartes (`MapsInProgress.md`)
- [x] Arbres : quantité x2 et plus serrés (0ae2591, réponse de l'utilisateur à l'essai Arrière-pays)
- [ ] M-Q10b = A (2026-10-30, D578) : grands espaces vides acceptés ; retirer la règle du cercle vide (contrôle `EmptyRuleHeld` de `Map.Table` : supprimer ou passer en simple mesure) ; à rediscuter quand le gibier et les points d'intérêt existent (`MapsInProgress.md`)
- [ ] Scénario ou essai in-game pour juger l'espacement des arbres (1,4 / 1,6 / 1,9 m côte à côte) : l'utilisateur ne peut pas lire une capture ; M-Q10c reste en attente dans Retours (`GROVE_SPACING` de `Tools/Editor/maps/common.py`)
- [x] M-Q3 = A (2026-10-30) : le pont des Gués tombé est un chantier qu'un Paysan relève sans coût (D496 confirmée), à coder avec la carte Les Gués
- [ ] Règle de décor (D576) : chaque carte son identité propre si elle aide à lire le terrain (rochers en montagne, sable ou terre brune en aride, sable / roseaux près de l'eau), jamais de généralité ni de remplissage ; à décider carte par carte à sa création
- [ ] M-Q10 non commité : le ranger dans un état cohérent et vert (ou le séparer) avant toute autre chose
- [ ] C3 El Dorado (M-Q2 = A, effets de terrain visibles seulement) ; **Plaine remise au budget standard (D564) et au sol uniforme (confirmé, D576)**
- [ ] `hinterland_paths.ps1` et `quick_check -Parts aiduel` sur les cartes sans décor

## Elrond — `tiers` — Paliers (`VictoryInProgress.md`)
- [x] Médaillons d'âge revérifiés au scénario `tiers` : « Validé » (2026-10-30), P4 validé
- [B] Rejouer `-Parts net` et `victory` quand `ai-ia1` est dans `main` (le `victory` rouge y est corrigé)
- [ ] P5 : compléter D569 au GDD (le numéro « D564 » est pris ; prochain libre à relire en § 22)
- [ ] V4 messages du chantier victoire (`VictoryInProgress.md`)

## Celeborn — `hero-j8-close` — Clôture de J8 (`JourneyInProgress.md`)
- [ ] **Glissement du rang : demi-places réunies, poussée sans prise de place, jamais hors du combat** (D577, décision de l'utilisateur sur l'essai `surplus`, ses mots dans `JourneyInProgress.md`) : un soldat de derrière qui voit un début de place pousse les lignes devant lui sans prendre leur place, le rang glisse à gauche ou à droite de la cible et les deux demi-places deviennent une place ; un frappeur n'est jamais poussé hors du combat, et la première ligne ne se décale pas si cela l'empêche de taper (même en changeant de cible). Réconcilier avec D517 (pas de déplacement de côté). Test rouge d'abord sur la vraie chaîne ; le scénario `surplus` (`hol.Combat.SurplusReassign 1/0`) sert à l'essai
- [ ] `quick_check -Parts hero` (non lancé) ; `-Parts tests,combat` déjà vert
- [x] Essai du scénario `surplus` fait : « c'est mieux » mais règle de glissement du rang demandée (tâche ci-dessus, D577) ; le scénario reste utilisable
- [ ] Mesures au repos C2 (`Tools/Tests/rest_measures.ps1`), seules, hors test
- [ ] Paquet et serveur à recuire (C4), seuls, hors test
- [ ] Sons du cri et de l'étendard (provisoires)

## Galadriel — `galadriel-tab` — Interface et divers (`InterfaceInProgress.md`)
- [x] Essai du scénario `tab` : « Tout est bon » (2026-10-30), correctif des ordres après Tab validé ; fusion de `galadriel-tab` dans `main` en attente
- [ ] Mini-carte du prototype façon AoE4 (camps en couleurs, sources explorées, cadre de caméra, clic gauche = caméra, clic droit = déplacement, alertes qui clignotent ; suit le brouillard) — après la texture de cases de Faramir
- [ ] Barre du bas trop large sous 1500 px
- [ ] Un test de contrôleur pour le double clic « même type » caché (regression F7.3a)

## Glorfindel — `net-playtest` — Réseau (`NetPlaytestInProgress.md`)
- Rien d'ouvert avant l'essai E5 avec un ami ; seulement si un reste apparaît (remesure MaxServers D502, r9, R8d).

## Fusions (agent de fusion, seulement si l'énoncé les autorise)
- [ ] Ordre : `ai-ia1`, `galadriel-tab`, `tiers`, `hero-j8-close`, `fog`, `maps` (détail dans `Auto_Journey_Retours.md`, section 3)
