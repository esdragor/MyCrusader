# Auto_Journey : la file de travail

Procédure : `Auto_Journey.md` (section « La file de travail »). États : `[ ]` à faire, `[~]` en cours, `[x]` faite, `[?]` attend l'utilisateur (voir `Auto_Journey_Retours.md`), `[B]` bloquée par une autre tâche.
Une tâche = une phrase, le journal à lire, le premier test rouge, 15-30 min. Le dernier agent d'un worktree ajoute la suivante qu'il voit venir.
**Semée le 2026-10-30 à 18:10 d'après les rapports du test 1 : l'étape 0 doit la confronter à l'état réel avant de lancer.**

## Arwen — `ai-ia1` — IA et Volcan (journaux : `AIInProgress.md`, `VolcanoInProgress.md`)
- [x] Réponses IA/Volcan enregistrées (D571-D575 au GDD, prochain **D576**) ; aide aux coéquipiers (2665240, testée en règle, aucune partie 2v2 jouée) ; Volcan V2 (ae476b5, 493/493)
- [ ] Volcan V3 (tirage libre, tracés symétriques ; V-Q2 = B, débloquée), puis V4 et suivantes du plan V0-V12 qui ne demandent pas d'essai
- [?] Volcan V-Q1 (fenêtres de Catastrophe) : « A trancher », à discuter avec l'utilisateur
- [?] IA4 arbitrage des vies D426 : « A trancher »
- [ ] IA4 Paysans : un Paysan ne frappe que s'il est sûr de gagner le duel (interprétation à confirmer, voir Retours)
- [ ] Technologies (GDD § 11) ou Arbre de talents (§ 9.6) : ouvrir le `*InProgress.md` et le plan si la file se vide

## Faramir — `fog` — Brouillard (`FogInProgress.md`)
- [ ] Bords du brouillard moins « rough » (grain plus fin) — réponse de l'utilisateur au scénario `fog`
- [ ] Les lignes d'ordre doivent rester visibles (jaune, blanche, rouge) — réponse de l'utilisateur
- [ ] F7.3 (b) fantômes de bâtiments teintés (plan : `HOLVisionRules::BuildingDrawsFromMemory`, `ELook::Memory`) puis (c) arbres coupés hors de vue
- [ ] F8, F9 ; texture de cases et lecteur pour la mini-carte du prototype (façon AoE4, décision du 2026-10-30)
- [ ] Mesures S5 (F6f.2) à la clôture, pas pendant un test

## Haldir — `maps` — Cartes (`MapsInProgress.md`)
- [ ] Arbres : doubler la quantité et les resserrer (réponse de l'utilisateur à l'essai Arrière-pays)
- [?] M-Q10b (solveur, cercle vide, décor), M-Q3 pont des Gués, décor des cartes à eau : questions dans Retours
- [ ] M-Q10 non commité : le ranger dans un état cohérent et vert (ou le séparer) avant toute autre chose
- [ ] C3 El Dorado (M-Q2 = A, effets de terrain visibles seulement) ; Plaine remise au budget (D564) et au sol uniforme
- [ ] `hinterland_paths.ps1` et `quick_check -Parts aiduel` sur les cartes sans décor

## Elrond — `tiers` — Paliers (`VictoryInProgress.md`)
- [?] Revérifier les médaillons d'âge au scénario `tiers` (correctif 1093e52, essai de l'utilisateur)
- [B] Rejouer `-Parts net` et `victory` quand `ai-ia1` est dans `main` (le `victory` rouge y est corrigé)
- [ ] P5 : compléter D569 au GDD (le numéro « D564 » est pris ; prochain libre à relire en § 22)
- [ ] V4 messages du chantier victoire (`VictoryInProgress.md`)

## Celeborn — `hero-j8-close` — Clôture de J8 (`JourneyInProgress.md`)
- [ ] `quick_check -Parts hero` (non lancé) ; `-Parts tests,combat` déjà vert
- [?] Essai du scénario `surplus` (Hommes d'armes en surnombre) — à créer s'il n'existe pas encore
- [ ] Mesures au repos C2 (`Tools/Tests/rest_measures.ps1`), seules, hors test
- [ ] Paquet et serveur à recuire (C4), seuls, hors test
- [ ] Sons du cri et de l'étendard (provisoires)

## Galadriel — `galadriel-tab` — Interface et divers (`InterfaceInProgress.md`)
- [?] Essai du scénario `tab` (ordres après Tab) : directions « nord/sud/est » du texte à vérifier
- [ ] Mini-carte du prototype façon AoE4 (camps en couleurs, sources explorées, cadre de caméra, clic gauche = caméra, clic droit = déplacement, alertes qui clignotent ; suit le brouillard) — après la texture de cases de Faramir
- [ ] Barre du bas trop large sous 1500 px
- [ ] Un test de contrôleur pour le double clic « même type » caché (regression F7.3a)

## Glorfindel — `net-playtest` — Réseau (`NetPlaytestInProgress.md`)
- Rien d'ouvert avant l'essai E5 avec un ami ; seulement si un reste apparaît (remesure MaxServers D502, r9, R8d).

## Fusions (agent de fusion, seulement si l'énoncé les autorise)
- [ ] Ordre : `ai-ia1`, `galadriel-tab`, `tiers`, `hero-j8-close`, `fog`, `maps` (détail dans `Auto_Journey_Retours.md`, section 3)
