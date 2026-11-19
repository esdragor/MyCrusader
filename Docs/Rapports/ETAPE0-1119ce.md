# ETAPE0-1119ce : ce qui reste de l'étape 5 et du portage des tests de monde (lecture seule, 17:22-17:30)

Agent ETAPE0-1119ce, aucune édition de code. Mesures faites dans Elrond (branche integ-19 ae64d0e) par Grep / Get-Content. MESURÉ = compté aujourd'hui ; ESTIMÉ = repris d'un plan ou déduit ; non vérifié en § E.
Constats de départ : main est maintenant b68b1c9 (le journal y dit « wave 16-F launched » : la vague 16-F ci-dessous peut déjà exister, à comparer) ; integ-17 et integ-18 NE SONT PAS dans main (`merge-base --is-ancestor` = 1) : tout ce qui est marqué « integ-x » n'est pas dans main.

## A. Tableau des lots

| Lot | État | Preuve | Reste |
|---|---|---|---|
| Écran Nouvelle partie NG-1..NG-6b, Statistiques ST-1..ST-3 | fait dans main | journal L22-37 ; essais x4 joués OK ~00:40 | rien |
| HUD H-1..H-11b (11 panneaux) | fait dans main | 5344427 puis 41dc1a0 ; essais x6 joués 10:12 | écart noté : `UHOLForumAlertHolder::ShowEvent` bSeesForum=false (L38) |
| C1 (16 consoles), C2 (26), M1, R1 + R1 suite, S1 (pause, FHOLPauseRules) | fait dans main | 5344427, 2170d83 | repli diplomatie de S1 : HOLGameState.cpp:1705-1706 (`EvaluateEnd >= 0 ? ... : Change.bFormed`), MESURÉ, ~2 l. |
| P1a/P1b, P2 (ordres du joueur par la planche), K1a/K1b (construction) | fait dans main, repli gardé | 2170d83, 3af40f0 | replis retirés dans integ-17 (2828188 orders-nofallback, c5e46ab boards-nofallback) : pas dans main |
| R2a, R2b, R3 (production : portes, verdict de mise en file, ralliement) | fait dans main | 3af40f0 (journal 16:02) ; 5dcf3b6 | AdvanceQueues, GetCost, naissance, porte de population : restent C++ (D839) |
| G1, G2a, G2b (règles de la grille) + pont `UHOLCommandGridBoard` | fait dans main | 2170d83, 3af40f0 | CheckResearch (CG.cpp, lit `HOLTechnology::FFacts`) jamais porté en script |
| G0 (15 Test_CG* repointés, 6 fonctions pontées dont GetCycleRank) | integ-18 (compilé, 1779/1779) | 1ca94df, 1bad9a4 ; 28dfc85 | rien |
| C9-1, C9-2 (Pause/Resume/Surrender, Units.*, Stop/Hold/..., suppression Select.Order/OrderLine) | integ-17 (compilé, 1755/1755) | 5e2b8df, 978c021, d1e88ac, 82c20de | entrer dans main |
| C10-1, C10-2 (pointage + OrderSource/Heal/Attack/PlaceNear...) | integ-18 (compilé, vert) | 4d72982, 8d853a0 ; liaison `UHOLControllerCommandLibrary` | entrer dans main |
| C10-3 (caméra Focus/Zoom/FocusSlot/FocusNear + Build.Place/Click) | pas commencé | C9-PLAN § 6 : « peu rentable » -90 net, 158-168 scénarios à rejouer | à ne pas faire (voir C) |
| Consoles encore C++ dans PC.cpp (MESURÉ integ-19) | 23 `FAutoConsoleCommand` : Select (Banner, LeftClickGround, AttackMove, AttackMoveNear, Group, Army, Category, Hero, Grid, OrderCell, Subgroup, Report), Camera (TestCursorGround, FocusSlot, Focus, FocusNear, Zoom, Report), Build (Place, Click, Report), Units.Bars, Tech.Report | PC.cpp 5380 l. | 4 gestionnaires d'entrée + 5 rapports restent par raison dite ; Group/Army/Category/Hero/Subgroup (5) non attribués à aucun lot (ESTIMÉ -60 net) |
| S2a (DefeatGate + MatchEndVerdict), S2c (FallGate, KingGate) | integ-18 | 910bfc5, 07c67b9 | rien |
| S2 retrait des replis C++ (4 points) | integ-19 (non compilé) | 6b549e4 (HALDIR-1119cb) | build + preuve réseau ; AdvanceForums (S2b / S3) NON migré par choix D839 |
| S4 absence / reconnexion | abandonné | décision du 11-19 | rien |
| C3 / C4 / C5 (43 corps `Do<X>`, MESURÉ 43 dans PC.cpp ; 51 `Request<X>`) | pas commencé, question posée | Retours\Q-etape5-corps-serveur.md [À RÉPONDRE], reco A = C4 + C5a (~ -440 net) | réponse de l'utilisateur |
| C6 / C7 | abandonné | C6-PLAN § 4 (net négatif ou nul) | remplacés par L1a-L4 |
| C8 (sous-groupes, groupes de contrôle) | pas commencé | C8-PLAN § 4-5 : ESTIMÉ -60 net, C8-4 « à ne pas faire » ; HALDIR-1119cc : crochet existe | après L4, facultatif |
| L1a (3 tests Rois / forum tombé / rage), L1b (2 tests forum nomade / population) | integ-19 (non compilé) | 3f54c3a, 02d5b79, b0a2906 | build, rouge vu ; 2 valeurs en dur de L1b (cellule 10 au lieu de hol.Nomad.ForumCell, population Paysan = 1) |
| L2 (extraits `HOLCommandGrid::` retirés de 5 tests C++) | écrit non compilé, hors integ-19 | grid-l2 1ba65fb, -53 l. | fusion après vert d'integ-19 ; couvertures perdues en cours (SAURON-1119ce) |
| L3a (19 corps `*Native` sur 21 retirés de CG.cpp) | écrit non compilé, hors integ-19 | grid-l3a fae3c2c, -182 l. | GetCommandNative / GetOrderNative partent après L2 ; mesure de `GetAgePanelOpacity` (par image) via le pont `IsAgeOpen` NON FAITE |
| L3b (19 fonctions de `HOLCommandGridRead::`, -166 l. C++, -768 l. script) | écrit non compilé, hors integ-19 | grid-l3b 80f6340 | 3 balayages perdus + jumeau `GetCycleRankOfType` (HALDIR-1119ce en cours) |
| L4 (build + preuves : tier_grid_client_run, absence de « no script grid board » client ET serveur dédié) | pas commencé | C6-PLAN § 3 | ~20 l. de .ps1 pour le serveur dédié, non écrites |
| G3 / L6 (contenu de `RefreshGrid`, HOLGameScreen.cpp 2012 l.) | pas commencé | C6-PLAN § 4 : -140 net | « peu rentable », corps du switch jamais lu |
| Tests de monde, lots 1-5 de `UHOLTestWorldLibrary` | fait dans main (1-5) ; 16 enveloppes de plus dans integ-19 | 0e0cb1a ... 7cc5478 ; 6ac4a89 | HOLTestWorldLibrary.cpp = 1063 l. |
| TierCheat, DisconnectDisplay (C++ retirés) | fait dans main | 2170d83 | rien |
| AIBuildOrders (C++ supprimé, 227 l.), 5 tests C++ Disconnect (-422 l.), AI.Orders.Rule | integ-17 (AI.Orders.Rule, 10 Disconnect) puis integ-19 (c952de4) | 667d3ad ; c952de4 | HOLDisconnectTests.cpp = 385 l. : Rules (DefaultWindowSeconds non exposé), Gains (2 assertions à ajouter au script), Technologies (capture de journal), WindowEnd (mode Diplomacy) |
| UnitTierProduction (monde) | integ-19, écrit non compilé | a41c7b8 HOLUnitTierWorld_Test.as | rouge à voir (`"2/2/1/1/1"` -> `"2/2/1/1/0"`), puis retirer HOLUnitTierProductionTests.cpp (221 l.) |
| Building (Foundation en partie, Selection entier, Damage en partie) | integ-19, écrit non compilé | 6ac4a89 | C++ HOLBuildingTests.cpp 1051 l. intact ; Selection à retirer après vert ; restent C++ : partie propriétaire du Foundation, gravats, navmesh, Appearance, ForumColour, ArcheryRange, FarmField, FarmClick, Walkable, FarmShape |
| BuildingRegistry | script porté (HOLBuildingRegistry_Test.as), C++ 116 l. TOUJOURS PRÉSENT | MESURÉ | retirer, sauf la « boucle qui détruit » (à vérifier) |
| Victory (1475 l.), StartingHero (788), AIOrders (467), HeroAbilityOrder (102), HeroTalentOrder (134) | pas commencé (en partie : AIOrders 8 cas script en 667d3ad, HeroTalent 4 tests script fd31374) | PLAN-TESTS-1119 § 2 | ESTIMÉ ~45 % / 40 % / 30 % portables ; Victory et StartingHero C++ sont aussi touchés par L2 |
| 10 fichiers de tests à naissance Mass (FarmBuilders, ProductionPopulationEnd, DuelOrder, GarrisonOrder, Gather, DeployButton, DiplomacyBranch, DiplomacyAllies, HeroReturn, Regicide) | restent C++ | D838 / D854 | rien |
| HOLTestSequence (D1) | abandonné | D853 | rien |

Compte du C++ de tests (MESURÉ integ-19) : 97 fichiers, 35 993 l. dans `Source\HeroesOfLiberty\Tests` ; 7 fichiers, 831 l. dans `HeroesOfLibertyUI\Tests`. Le 11-19 `git diff --shortstat main integ-19 (*.cpp *.h)` : 22 fichiers, +1667 / -2512 (integ-19 a donc déjà retiré ~850 l. nettes hors main).
Replis `no script board` : plus de repli C++ côté règles (PlayerOrders, pause, S2, marché) ; il reste le jeu de journaux d'absence (HOLBoardLog, Market x4, Camp, Garrison, SiteBoard) qui sont des refus, pas des replis. Repli réel encore là : grille (`*Native` : 50 occurrences dans CG.cpp, 21 dans CG.h, 22 dans RL.cpp, retirés par L3a/L3b) et S1 diplomatie (HOLGameState.cpp:1705).

## B. Lignes périmées de `Auto_Journey_File.md`, section Migration D851

- L30 `[~] NG-6a` : NG-6b est fait (retrait du C++ de Nouvelle partie, L37, main 5344427).
- L32 `[?] ST-3` « attend l'essai » : essai joué OK ~00:40 (Jour : « ecran_statistiques OK »), dans main.
- L35 `[?] H-1` et L36 `[?] H-2` « attend l'essai » : alert_population OK et ecran_hud_bannieres OK, dans main depuis 20a5e2f.
- L39 `[ ] étape 5 vague 2` : R1 suite, C1 suite, S1, P1/P2, K1, G1/G2 sont faits (L45, L48).
- L41 `[~] C1 / M1` : dans main (L37, L45).
- L42 `[ ] HUD H-5 à H-11` : tous faits (L37, L38, L45).
- L43 `[ ] étape 5 ~45 lots` : vue d'ensemble, à remplacer par le tableau A (C6 / C7 / S4 / D1 abandonnés, C8 facultatif, C3-C5 en question).
- L48 « RESTE : retrait des replis P1 et K1 » : fait dans integ-17 (2828188, c5e46ab), pas dans main ; « CheckResearch / GetCycleRank » : GetCycleRank pont fait (1bad9a4), CheckResearch non.
- L49 `[ ] S2` : S2a et S2c faits (integ-18), replis retirés (integ-19) ; seule S2b reste non migrée (choix).
- L50 `[~] grille` : G0 fait, L1a-L3b écrits ; plus de « G3 à part » urgent.
- L51 `[?] C3/C4/C5` : toujours vrai (question sans réponse) ; la fin de la ligne (R2a lancé 13:09) est périmée : R2a/R2b/R3 sont dans main.
- L52 `[~] reprise 16` : integ-17 32fd9fb et integ-18 8371d9a périmés (integ-18 = 28dfc85, integ-19 = ae64d0e).
- L53 `[ ] portage des tests de monde` : lots 1 à 5, TierCheat, DisconnectDisplay, Disconnect (en grande partie), AIBuildOrders sont faits ; reste = Victory, StartingHero, AIOrders, Building (suite), Disconnect (4 tests).
- L54 `[ ] C9 / C10` : C9-1, C9-2, C10-1, C10-2 faits (integ-17 / integ-18) ; ne reste que C10-3 (déconseillé).
- L55 `[ ] lobby_match intermittent` : corrigé 1ace000 (attend 20 s les 2 joueurs), integ-17.
- L40 : ancienne ligne `[x]` redondante avec L37, à supprimer.

## C. Ce qui reste, par gain et par risque joueur

| Rang | Chantier | C++ retiré (net) | Risque joueur |
|---|---|---|---|
| 1 | L2 + L3a + L3b (retrait du natif de la grille) | ESTIMÉ -1 000 (déjà écrit : -53, -182, -166 mesurés sur les commits ; reste ~ -600 : GetCommandNative / GetOrderNative, CG.h, 2 fonctions RL) | élevé si un monde n'a pas de plateau (grille vide) : L4 obligatoire ; GetAgePanelOpacity par image à mesurer |
| 2 | Tests de monde Victory + StartingHero + AIOrders + Building (suite) | ESTIMÉ -1 200 à -1 500 (Victory ~650, StartingHero ~300, AIOrders ~140, Building ~300) | nul (tests) ; coût : liaisons dans HOLTestWorldLibrary |
| 3 | Retraits de tests déjà portés : UnitTierProduction 221 + BuildingRegistry 116 + Building Selection + Disconnect Gains | MESURÉ ~ -340 à -450 | nul, après rouge vu |
| 4 | C3 / C4 / C5 (corps `Do<X>`) | ESTIMÉ -440 net (A : C4 + C5a seulement) | réseau : élevé pour C3a / C5b, nul pour C4 (triches) ; attend l'utilisateur |
| 5 | Request<X> (51 dans PC.cpp, C11) | ESTIMÉ ~ -400 (8 l. chacune) mais NON compté et dépend des appelants restants | moyen |
| 6 | G3 / L6 RefreshGrid | ESTIMÉ -140 net | visuel moyen à élevé |
| 7 | C9 queue (Group, Army, Category, Hero, Subgroup ; hors Grid / OrderCell cités par 71 scénarios) | ESTIMÉ -60 net | nul (dev) |
| 8 | C8 sous-groupes | ESTIMÉ -60 net | moyen (touches 1-9) |
| 9 | C10-3 caméra + placement | ESTIMÉ -90 net | nul (dev) mais 158 scénarios à rejouer |
| 10 | S1 repli diplomatie HOLGameState.cpp:1705 | MESURÉ ~ -2 | moyen (fin de partie par alliance) |

Verdict : après L2-L4 la migration « utile » est presque finie ; le gros reste est les tests (rang 2-3). Les rangs 6-9 coûtent un build ou des replays pour moins de 150 l.

## D. Vagues proposées (4 agents chacune ; Arwen gelé, Faramir hors périmètre ; Elrond et Glorfindel = build integ-20 et preuves)

Condition commune : la session principale a peut-être déjà lancé une « 16-F » (journal) ; comparer avant de relancer. Fichiers vérifiés disjoints entre agents d'une même vague ; entre vagues, L2 touche HOLVictoryTests / HOLStartingHeroTests / HOLRegicideTests / HOLDamnedWarriorTests / HOLProductionPopulationEndTests.cpp, d'où « script seul » pour Victory et StartingHero en 16-F.

### Vague 16-F (base = integ-19 ae64d0e ; sans build ; un build d'integ-20 = integ-19 + L2 + L3a + L3b + balayages + couvertures par Elrond en parallèle)
1. CELEBORN, nouvelle branche `test-world-8` : test de monde « règles de forum / défaite sans naissance » de HOLVictoryTests.cpp en script (UN test `Test_VictoryForumRules`, rouge par falsification d'une valeur) ; fichiers : `Script\Tests\HOLVictoryWorld_Test.as` (neuf), `Source\HeroesOfLiberty\Tests\HOLTestWorldLibrary.h/.cpp` (enveloppes). C++ : oui (liaison, groupée au build suivant). Lire d'abord la liste des cas sans naissance (ESTIMÉ 45 %). Attend : un build vert + rouge vu.
2. GALADRIEL, nouvelle branche `disconnect-3` : compléter `Test_DisconnectGains` (CountPlayerStatesOfSlot == 0 après Bob.DestroyActor ; Load.Amount == 0), + exposer `DefaultWindowSeconds` si trivial ; fichiers : `Script\Tests\HOLDisconnect2_Test.as`, `Source\HeroesOfLiberty\Tests\HOLGameStateTestLibrary.h/.cpp` (seulement pour DefaultWindowSeconds). C++ : oui (une ligne de liaison) ou non si on laisse Rules. Attend : build vert puis 16-G retire Gains / Rules (-~150 l.).
3. SAURON, nouvelle branche `ai-orders-world` : les cas « ordres seuls » de HOLAIOrdersTests.cpp en script (UN test, falsification) avec les 9 enveloppes de la bibliothèque d'ordres IA déjà là (d2546f4) ; fichiers : `Script\Tests\HOLAIOrdersWorld_Test.as` (neuf) seulement. C++ : non. Attend : build vert, puis retrait du C++ (ESTIMÉ -140).
4. HALDIR, branche `grid-l4-proof` (depuis integ-19) : étendre `Tools\Scenarios\tier_grid_client_run.ps1` (journal client SANS « no script grid board ») et ajouter le même contrôle sur le serveur dédié (~20 l. de .ps1) ; fichiers : `Tools\Scenarios\tier_grid_client_run.ps1` (+ un `.ps1` neuf). C++ : non. Attend : exécuté par Elrond sur integ-20 avec rouge vu (bouchon `return FHOLGridCommandView();` dans `HOLCommandGridBoard.as:13`).
(Ordre de départ : après le départ de HALDIR-1119ce et SAURON-1119ce qui touchent HOLCommandGridSweeps_Test.as et les couvertures L2.)

### Vague 16-G (base = integ-20 construit et vert, L2-L3 fusionnés)
1. CELEBORN : retirer les C++ déjà portés après rouge vu : `HOLUnitTierProductionTests.cpp` (221 l.), `HOLBuildingRegistryTests.cpp` (116 l., sauf boucle destructrice), le cas Selection de `HOLBuildingTests.cpp` (UN rouge : falsification `"2/2/1/1/0"`). Fichiers : ces 3 .cpp. C++ : non (retrait de test). Attend : un build vert.
2. GALADRIEL : retirer `Match.DisconnectGains` (+ Rules si exposé) de `HOLDisconnectTests.cpp` après le rouge vu de `Test_DisconnectGains`. Fichier : `HOLDisconnectTests.cpp`. C++ : non. Attend : build vert.
3. SAURON : retirer de `HOLAIOrdersTests.cpp` les cas portés (après rouge vu) ; en plus, script seul : cas StartingHero sans naissance (`Script\Tests\HOLStartingHeroWorld_Test.as`). Fichiers disjoints de L2 (déjà fusionné). C++ : non. Attend : build vert.
4. HALDIR : retirer le repli S1 `HOLGameState.cpp:1705-1706` (comme 6b549e4 : refus journalisé sans planche, UN test script de diplomatie rouge par falsification) ; fichier : `HOLGameState.cpp`, `Script\Tests\HOLMatchFlow_Test.as`. C++ : non. Attend : build vert + `rpc_diplomacy_run.ps1`.
Hors vagues (attendent l'utilisateur) : C3-C5 (réponse à Q-etape5-corps-serveur), C9 queue, C8, G3.

## E. Non vérifié

- Le contenu de integ-19 hors `git log` (les 40 derniers commits) : je n'ai pas relu les diffs ; l'état « compilé » repose sur le journal, pas sur un binaire.
- Aucun diff de grid-l2 / l3a / l3b lu ; leurs chiffres (-53, -182, -166) viennent du journal.
- Les 3 fichiers `RETRAITS-PLAN-1119.md` § 3-5, `S2-PLAN`, `R2-PLAN`, `K1-PLAN`, `P1-PLAN`, `G1-PLAN` : non lus (RETRAITS-PLAN lu jusqu'au § 2) ; l'état de R2b / R3 dans main repose sur la ligne du journal « main avancé 16:02 » (3af40f0), pas sur le diff.
- Si un cas de HOLVictoryTests / StartingHero / AIOrders est réellement portable sans liaison neuve (ESTIMÉ 45 / 40 / 30 %, jamais relu cas par cas).
- Que la « boucle destructrice » de BuildingRegistry soit couverte en script.
- Combien de `Request<X>` ont encore des appelants C++ (rang 5 ESTIMÉ).
- Les 4 appels du HUD, les tests UI (831 l.) et `HOLSpecializationGridTests.cpp` (20 appels, restent tant que SetSpecialization est C++).
- Le total de quick_check après L3b (baisse de 5 tests d'accord décidée par la session principale ; compte exact non recalculé).
- Si la vague 16-F lancée par la session principale (journal) recoupe mes 4 agents : je n'ai pas lu sa ligne.
