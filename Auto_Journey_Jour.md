# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11), `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_soir_archive_2026-11-14.md` (séance autonome du 2026-11-13 soir : bilan, leçons, tableau d'agents ; résumés du 2026-11-13 matin et du 2026-11-12) et `Docs\Archive\Auto_Journey_Jour_2026-11-14_archive_2026-11-15.md` (séance autonome du 2026-11-14 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Chapitres du GDD non commencés (relevé NEUF du 2026-11-12 11:10, par les titres du GDD, les `*InProgress.md`, `EtapeDefini.md` et Grep dans `Script\` et `Source\`)

Le relevé du 2026-10-30 est épuisé (archive : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`). Celui-ci part des règles du GDD **sans code**, qu'un journal existe ou non. Ordre conseillé = ordre des lignes. « Règle pure » = Angelscript + `*_Test.as`, sans C++.

**Groupe A : prêt à ouvrir sans question (4)**

| # | § et titre | D | Existe déjà | 1re étape de 15 min, test rouge | C++ | Worktree |
|---|---|---|---|---|---|---|
| A1 | § 15 Signaux sur la carte (roue Alt + clic) | D386, D369, D376 | fil de messages (D445), alliés mutuels dans `Script\Diplomacy\HOLDiplomacyRules.as` ; aucun code de signal (Grep `Signal`, `Ping` vide) ; aucun journal | règle pure `HOLSignalRules.as` : 5 types + neutre, destinataires = alliés mutuels, plafond 3 par 5 s. Rouge : le 4e signal en 5 s est refusé ; un Neutre et un Ennemi ne le reçoivent pas | non (règle) ; oui ensuite (saisie, RPC, icône de mini-carte) | **OUVERT 2026-11-12 11:41 dans Haldir (`maps`, HALDIR-1112e)** : SG0-SG1 faites, `SignalsInProgress.md`, rouge 76f66b8, vert 9d30e2c (1186/1186), Q-signals ; suivant SG2 C++ |
| A2 | § 13 Choix de spécialisation de palier, mécanisme commun | D07, D332-D334, D683, D684 | plans dans `AubeInProgress.md` (l. 127) et `LegionsInProgress.md` (L9), paliers P1-P5 dans main ; aucun code (Grep `Specialization`, `TierChoice` vide) | règle pure `HOLTierChoiceRules.as` : un choix sur deux par palier, délai 30 / 45 / 60 s en parallèle de la file, annulable sans coût. Rouge : abouti à 30 s et pas à 29 ; annulé = rien ; 2e choix du même palier refusé | non (règle) ; léger ensuite (bouton du Forum, réplication) | Elrond (tiers) après G6, sinon Galadriel (L9) — **OUVERT 2026-11-12 11:56 (HALDIR-1112f, Haldir, maps)** : SP1 règles pures `HOLSpecializationRules.as` (nom retenu au lieu de `HOLTierChoiceRules.as`), rouge 567fd50, vert 1124a13, 1193/1193 ; `SpecializationInProgress.md`, `Retours\Q-specialization.md` ; suivant SP2 C++ |
| A3 | § 14.1 / § 14.3 Option Nomade | D207, D354, D402 | case au menu et au salon (`HOLNewGameScreen.cpp`, `HOLMatchConfig.h`), aucune règle ; `VictoryInProgress.md` la range « à venir » | règle pure `HOLNomadRules.as` : défaite à 0 Paysan tant qu'aucun forum n'est achevé, forum détruit en chantier sans vie perdue, stock de départ + prix d'un forum, héros près du plus gros groupe. Rouge : dernier Paysan mort sans forum = défaite, avec forum achevé = non | non (règle) ; oui ensuite (départ sans forum, groupes dispersés, distance minimale en réglage) | Elrond (victoire) ou un libre ; **OUVERT 2026-11-12 12:08 (HALDIR-1112g, Haldir, maps)** : N0-N1 faites (`NomadInProgress.md`, rouge 938087b, vert ae9d9af, 1200/1200), Q-nomad, suivant N2 C++ |
| A4 | § 15 Canaux de discussion | D385, D376, D505 | fil d'annonces, panneau Diplomatie ; aucun code (Grep `Chat` vide) ; `DiplomacyInProgress.md` l. 41 : « autre chantier de réseau » | règle pure `HOLChatRules.as` : destinataires par canal (Alliés, Tous, privé, spectateurs). Rouge : un message Alliés n'arrive ni à un ennemi ni à un spectateur ; un privé au seul destinataire | non (règle) ; oui pour le transport (RPC, saisie Entrée) | Glorfindel (net-playtest) |

**Groupe B : ouvrable après 1-2 questions (6)**

| # | § et titre | D | Existe déjà | Question à poser d'abord | 1re étape, test rouge | C++ | Worktree |
|---|---|---|---|---|---|---|---|
| B1 | § 6 Garnison (étape F7 de Fortifications) | D693, D351, D365, D374, D601 | plan et test rouge écrits (`FortificationsInProgress.md` l. 108) ; seul « pas de garnison chez un allié » est codé ; débloque SC2, le Roi, le forum | places par tour, occupants protégés ou non, quelles unités entrent (cavalerie ?) | `Script\Garrison\HOLGarrisonRules.as`. Rouge : on entre jusqu'aux places libres et pas plus, ennemi et allié refusés | oui (simulation) | Elrond (tiers) |
| B2 | § 9.4 XP d'exploration | D05, D06 | source `Exploration` déclarée « no carrier yet » (`HOLHeroXPRules.h`), brouillard F0-F9 | porteur : surface découverte, lieux découverts (relais, camps, autel) ou les deux ; valeur | règle pure par tranche explorée. Rouge : une tranche neuve paie une seule fois, aussi pendant la récupération | oui (lecture du brouillard) | Faramir (fog) |
| B3 | § 8.2 Zones de furtivité | D383 | points d'accroche décrits (`FogInProgress.md` l. 164), aucune carte n'en pose, aucun code (Grep `Stealth` vide) | distance « tout proche » ; quelles cartes en posent, quel aspect | règle pure « caché sauf ennemi dans la zone ou à moins de N ». Rouge : vu à N - 1, caché à N + 1 | oui (vision par camp) | Faramir, cartes par Haldir |
| B4 | § 8.1 Reste de D33 : couvert en forêt, forêt lente pour cavalerie et siège, défense réduite au gué | D33, D496 | marais, route et charge coupée codés (`Script\Terrain\HOLTerrainRules.as`) ; rien pour la forêt ni le gué dans ce script | « être en forêt » = zone peinte ou proximité d'arbres ; valeurs modérées | facteur de dégâts à distance en forêt dans `HOLTerrainRules.as`. Rouge : moins de dégâts à distance en forêt, mêlée inchangée | léger (le combat lit le facteur) | Haldir (maps) |
| B5 | § 7.3 Ordres Patrouille et Suivre | D30, D340 | Q attaque-déplacement, S arrêt, V maintien ; aucun code (Grep `Patrol`, `Follow` vide), aucun journal | case de la grille de chacun ; ce que fait Suivre (allié, ennemi) et « focus » | règle pure de patrouille (aller-retour, attaque-déplacement à chaque trajet). Rouge : l'unité repart vers A une fois en B | oui (ordres) | Celeborn (ordres d'unités) |
| B6 | § 7.3 / § 15 Raccourcis reconfigurables | D30 | onglet Commandes grisé « jusqu'à M4 », grille en dur ; aucun code (Grep `Rebind`, `KeyRemap` vide) | l'ouvrir maintenant ou le laisser grisé ; grille seule ou tous les raccourcis | table action vers touche en données + règle pure de conflit. Rouge : deux actions sur la même touche = refus | oui (interface C++) | Galadriel (grille) |

**Groupe C : déjà couvert ou sans règle à coder (une ligne de preuve)**

- § 5.1 Ressources, ferme, moulin : dans main (D390, D346) ; chasse, pêche, gibier écartés du prototype (`MapsInProgress.md` l. 425, § 17 n'en liste pas, Grep `hunt`, `deer`, `fish` vide).
- § 5.2 Ressources spéciales : principe sans règle (relu par HALDIR-1112b).
- § 5.3 Population : maisons et plafond joués par l'IA (IA1) ; D391 (population pleine sans file) vit dans les dossiers Légions (Squelettes) et Aube (Renforts).
- § 5.4 Centres secondaires D384 : `SecondaryCentersInProgress.md` (Haldir, maps), SC0-SC1 faites, SC2-SC3 attendent `Retours\Q-secondary-centers.md` et la garnison (B1).
- § 5.5 Marché et tributs : dans main, essai validé (`MarketInProgress.md`).
- § 6 Construction : zone anti-construction D22 codée (`HOLConstructionRules.h`) ; Forge, Académie, murs, portes, tours = F1-F15 de `FortificationsInProgress.md` (Elrond, tiers).
- § 6.1 Siège : S0-S8 dans main (`SiegeInProgress.md`) ; Trébuchet et Canon = `Palier3InProgress.md`.
- § 6.2 Bonus de hauteur, remparts, § 16.5 navigation à deux niveaux : étapes F8 et F12 de Fortifications.
- § 7.1 Unités : socle dans main ; Hallebardier et emblématiques = `AubeInProgress.md`, `LegionsInProgress.md` ; Arquebusier = Palier 3 ; charge = « avec la cavalerie » (`CavalryInProgress.md` C2).
- § 7.2 Combat et moral : D345, D517 dans main ; moral = `MoraleInProgress.md` (M1, M2, M6 faites).
- § 7.3 Sélection, groupes, Paysans inactifs, armée : codés (`HOLGameScreen.cpp`) ; F1-F3 et Tab sur galadriel-tab.
- § 8.1 Marais, route, charge coupée : codés (`HOLTerrainRules.as`) ; zones sacrées ou corrompues = capacités de faction (D40).
- § 8.2 Brouillard : F0-F9 dans main ; hauteur de vue codée (`HOLVisionRules.h`, sol plat aujourd'hui).
- § 8.3 Exploration : principe ; sa seule règle est l'XP (B2).
- § 8.4 Neutres : `NeutralsInProgress.md` (N1-N7).
- § 8.5 Cartes, tailles, pont des Gués : dans main (`MapsInProgress.md`, `HOLMapFordsTests.cpp`) ; biomes = `BiomeInProgress.md`.
- § 9.1 bis à § 9.8 Héros : J8a-J8c dans main (résistance `HOLCombatRules.cpp`, retour `HOLHeroReturn_Test.as`, XP `HOLHeroXPRules.h`) ; § 9.6 = `TalentsInProgress.md` ; capacité 3 et ultime après le prototype (D42).
- § 10 Duel : `DuelInProgress.md` (reste D121, D122, récompenses branchées).
- § 11 Technologies : T1-T7 dans main ; § 11.2 = `Palier3InProgress.md` (G1-G6).
- § 12 Volcan, § 12.2 bis, option Catastrophe, § 16.2 World State Manager : `VolcanoInProgress.md` (V1-V12 dans main, reste V9b, V9c).
- § 13.2 Aube : `AubeInProgress.md` (dossier complet D685-D690, A1 en tête) ; § 13.3 Légions : `LegionsInProgress.md` (L0-L4).
- § 14.1 Victoire : V1-V5 et paliers dans main (`VictoryInProgress.md`) ; Roi en garnison D365 attend B1 ; § 14.2 Snowball : principe.
- § 14.3 Modes : Diplomatie = `DiplomacyInProgress.md` ; Sites sacrés = `SacredSitesInProgress.md` ; Carte explorée = F8 du brouillard.
- § 14.4 IA, déconnexion, réseau : `AIInProgress.md` (IA0-IA4), `DisconnectInProgress.md`, `NetPlaytestInProgress.md`.
- § 15 HUD, panneau Diplomatie, statistiques (`HOLStatsScreen.cpp`), pause, salon : dans main ; héraut et sons = `AudioInProgress.md`.
- § 17.1 Tests prioritaires : `PlaytestInProgress.md` (PT0-PT8).
- **Après le prototype, ne se prennent pas** : § 13.4 à § 13.7 les quatre autres factions, § 12.3 Événements futurs (D209), § 14.3 Battlefield, Domination, Reliques, Scénario (D416), § 9.9 Progression hors partie, Diplomatie V2 (D432), moral de groupe à états (§ 7.2).

**Non vérifié dans ce relevé** : le C++ de `Source\...\Map\HOLTerrainRules.h` pour la forêt et le gué (B4, seul le script a été lu) ; le rattrapage d'XP sur un héros tué (D05) et l'XP pendant la récupération (D06) ; les alertes mineures de D202 (population pleine, recherche terminée) ; la fiche d'un héros adverse (D203) ; l'ordre des étapes F3-F6 avant F7 ; § 9.5, § 9.6, § 10, § 11, § 12 et § 13 jugés au seul état de leur journal, sans relecture du GDD ; aucun worktree n'a été fouillé (journaux de la copie principale et `SecondaryCentersInProgress.md` de Haldir seulement).

### Résumé du 2026-11-14 (séance complète : archive)

- **Séance** : autonome 10:18 -> 14:15, fusion groupée acceptée ; 8 agents lancés à 10:43 (un par worktree), 37 lignes au tableau dont 3 fusions et la clôture.
- **main** : d761305 -> f539041 par 3 fusions groupées (n°1 = 7 branches, n°2 = net-playtest, n°3 = d751-root, visual, maps), tests 1263 -> 1267 verts, net, build, aiserver, aiduel PASS ; après la séance, fusion groupée de l'Arquebusier (main 140cdb7, 16:54). **RIEN POUSSÉ.**
- **Fait** : outils L14 (edit_line, ForeignSession, FrameWaits, commit_docs -Only, extend_session) ; garnison F7 lots 1-2 ; Nomade (forum dès le départ, groupes sur toute la carte D797, forum page II D803, couvert des forêts retiré D796) ; Sites sacrés S3 ; verbes `hol.Mouse.Click` / `hol.Mouse.Hover` ; panneau de scénario EN/JA et triches ; sons élus D798 (tour 4 rejeté D801) ; rochers D800, D802 ; prise de lance B D795.
- **Décisions** : D797-D803. **Essai donné** : nomad_start (gestes internes). **Question ouverte** : Q-arquebusier-modele.
- **Non fait** : D799 réseau Nomade, garnison lots 3-4, Backspace des compare (règle 15), audio tour 5, proportion des rochers, Légions modèle client, régénération des 7 cartes.
- **Anomalies** : `git merge main` d'agents refusés par le classifieur ; agents jusqu'à ~66 appels ; lignes collées dans la file (réparé dec60d6) ; 3 BUILD LENT (358-817 s) ; journal F7 écrit par la session principale ; `edit_line.ps1` coupé par `Select-Object`.
- **Archive** : `Docs\Archive\Auto_Journey_Jour_2026-11-14_archive_2026-11-15.md` (tableau d'agents, bilan, leçons tels quels).

## Agents du 2026-11-15

Séance autonome lancée à 09:02, fin 17:00, fusion groupée acceptée (sans push). Exception : nettoyage 0a sans le dossier Retours\ (déjà nettoyé le 2026-11-14 au soir, 11 fichiers).

| Agent | id | worktree | tâche | C++ | lancé |
|---|---|---|---|---|---|
| NETTOYAGE-0a x3 | a0673aca322fa97a5, ab4a09201e485b1bb, a2e0db8e436dd6ee6 | copie principale | archives Jour/Retours, File, Procédure/Checklist (e91b4f2, c52244f, c8fff23) | non | 09:04 |
| ETAPE0-1115 | acaf5b92f52066709 | lecture seule | plan de lancement (Fable) | non | 09:09 |
| ARWEN-1115a | a46cb350aeee07631 | HeroesOfLiberty-Arwen | audio tour 5 (sons naturels Caserne/Champ de tir/Écurie, D801) | non | 09:17 |
| SAURON-1115a | ad0a06bfc2b35225f | HeroesOfLiberty-Sauron | rochers D802 (réglage, DECOR_KINDS, mapSize, resources_game.*), SANS Arquebusier | non | 09:17 |
| CELEBORN-1115a | ac79e43a4cf1fd115 | HeroesOfLiberty-Celeborn | Neutres 2/4 hypothèse SelfSide (rangement, mesure x8, règle) | non puis oui | 09:17 |
| SAURON-1115a (fini 09:28) | ad0a06bfc2b35225f | HeroesOfLiberty-Sauron | rochers D802 : bf9d823 rouge, 36a4be8 vert, 7d92178 (merge main), Q-rocks-proportion ; 53 tool_uses | non | 09:17 |
| HALDIR-1115a | a93df869e3e9d501c | HeroesOfLiberty-Haldir | D799 réseau Nomade (booléen Nomade par siège) | oui 1/3 | 09:30 |
| HALDIR-1115a (fini 09:52) | a93df869e3e9d501c | HeroesOfLiberty-Haldir | D799 fait : fa31def test (non vu rouge), 0875a5e code, tests 1269/1269, net PASS ; reste fin du forum non répliquée ; ligne 315 d'EtapeDefini écrasée par edit_line puis restaurée par la session principale ; 49 tool_uses | oui | 09:30 |
| FARAMIR-1115a (fini 09:55) | aa04149035f3fc902 | HeroesOfLiberty-Faramir | règle 15 Backspace : da10225 rouge (vu), 53c106f vert, 80cb749 ; tests 1270/1270 ; rejeu compare et gate Close L15-1 non faits ; 41 appels | oui | 09:30 |
| ELROND-1115a (fini 10:01) | a9ca157745ed36801 | HeroesOfLiberty-Elrond | garnison F7 lot 3a : 70cd83f rouge (vu), d60b862 vert, 5a907c7 ; tests 1269/1269 ; net non lancé ; 66 tool_uses (54 vus) | oui | 09:30 |
| GLORFINDEL-1115a (fini 10:03) | a7b125035e9bce977 | HeroesOfLiberty-Glorfindel | disconnect_1v1 rejoué, PASS, [À ESSAYER] ; S7 grisage : e3ccea1 rouge (vu), code vert non commité, test encore KO, Q-sites-grisage ; 59 tool_uses | oui | 09:30 |
| HALDIR-1115b (fini 10:16) | a872a640d3521c553 | HeroesOfLiberty-Haldir | fin du premier forum répliquée : b4ffd4f rouge (vu), 097a26f code, fffe788 ; tests 1269/1269, net PASS ; design déjà tranché (D354, D417-D420), pas de question ; Nomade N3 IA non fait (design) ; 45 appels | oui | 09:53 |
| GALADRIEL-1115c (fini 10:28) | a5a2d96b1127f6fe2 | HeroesOfLiberty-Galadriel | B6 raccourcis reconfigurables : f9b3e24 rouge (vu, 5 KO), 61ec678 vert, 85c0735 ; tests 1275/1275 ; Q-rebind (quand ouvrir l'onglet, quels raccourcis) ; table des 49 raccourcis D30 ; règle non branchée ; 41 appels | non | 10:19 |
| GLORFINDEL-1115c (fini 10:28) | a599c629d23707a12 | HeroesOfLiberty-Glorfindel | B5 Patrouille : 85550d5 rouge (vu), 3a636ce vert, 35cefa2, a0828f9 ; tests 1272/1272 ; Q-patrol-follow (case de grille, Suivre, focus) ; test 1 corrigé après le rouge (rouge déduit) ; règle non branchée ; 40 appels | non | 10:19 |
| SAURON-1115b (fini 10:28) | ac83ef1d4183d67f1 | HeroesOfLiberty-Sauron | B2 XP d'exploration : 2487a66 rouge (vu), cf77faf vert, 2e5f8e3, merge de32333 ; tests 1272/1272 (avant merge) ; Q-exploration-xp (surface / lieux / les deux, défaut C) ; règle non branchée ; 44 appels | non | 10:19 |
| ARWEN-1115c (fini 10:27) | a69bf5b4bd8bb4b92 | HeroesOfLiberty-Arwen | B3 zones de furtivité : 3b370fa rouge (vu), d8bbe87 vert, 71bb3e4 ; tests 1272/1272 ; Q-stealth (N, cartes, aspect) ; règle pure non branchée au jeu ; 41 appels | non | 10:19 |
| FARAMIR-1115c | a56dc2be67712d8a6 | HeroesOfLiberty-Faramir | diagnostic du rouge `quick_check -Parts values` (value_sheet morale.bonus_cap_*) avant la fusion | non | 10:30 |
| GLORFINDEL-1115b (fini 10:13) | a1f3996c59cc84cd9 | HeroesOfLiberty-Glorfindel | S7 grisage VERT : cf7d33f (le KO venait du test : pas de PlayerContext), fed20bd test d'accord NumSites/cartes, 17a9af8 ; tests 1269/1269 ; `quick_check -Parts values` ROUGE avant lui (value_sheet morale.bonus_cap_taken/other) : à vérifier à la fusion ; 33 appels | oui | 10:05 |
| ELROND-1115b (fini 10:22) | a3c9d412d9ae329da | HeroesOfLiberty-Elrond | garnison : 79c5d1f rouge (vu), 82a934e WIP, 8e62c66 ; ExitPoint (3b) VERT, SeparationAndHeal ROUGE (régression, assertion non lue), net PASS, Q-garrison-soin ; tiers NON fusionnable ; 64 tool_uses (53 vus) | oui | 10:03 |
| ELROND-1115c | a9d552040713f6c6f | HeroesOfLiberty-Elrond | régression SeparationAndHeal (lire l'assertion, banc ou code), commit vert, puis 3c éjection | oui | 10:24 |
| FARAMIR-1115b (fini 10:19) | a7ebd6c2503e428f6 | HeroesOfLiberty-Faramir | L15-1 gate Close + L15-2 edit_line -Append : a898371 rouge (vu), 938e972 outils, 6f2de0a ; gate_test 87/87, edit_line_test 20/20 ; outils copiés dans main (commités par la session principale) ; Q-close-gate-l15-1 (2 questions) ; 42 appels | non | 09:56 |
| HALDIR-1115b | a872a640d3521c553 | HeroesOfLiberty-Haldir | fin du premier forum répliquée au client (commence sans compiler, plafond C++ atteint) | non puis oui | 09:54 |
| ELROND-1115a | a9ca157745ed36801 | HeroesOfLiberty-Elrond | garnison F7 lot 3a corps hors terrain puis 3b-3e | oui 2/3 | 09:30 |
| FARAMIR-1115a | aa04149035f3fc902 | HeroesOfLiberty-Faramir | Backspace dans waitkey/compare + gate Close L15-1 (DLL à jour) | oui 3/3 puis non | 09:30 |
| GLORFINDEL-1115a | a7b125035e9bce977 | HeroesOfLiberty-Glorfindel | disconnect_1v1 preflight puis Sites S7 grisage | non puis oui | 09:30 |
| ARWEN-1115a (fini 09:37) | a46cb350aeee07631 | HeroesOfLiberty-Arwen | audio tour 5 en enregistrements libres : 2d8ebdd rouge, 0e98c8b vert, 442b7de ; barracks/range preflight PASS, stable non abouti ; 47 tool_uses | non | 09:17 |
| CELEBORN-1115a (fini 09:43) | ac79e43a4cf1fd115 | HeroesOfLiberty-Celeborn | tâche déjà faite (SelfSide = c5f2279 dans main depuis 11-06) ; A/B x8 6/8 vs 2/8 ; scénarios rangés a3c5908 ; piste du tremblement de marche en bloc ; 42 tool_uses | non | 09:17 |
| GALADRIEL-1115a (fini 09:43) | a3eab82a8f6b5c483 | HeroesOfLiberty-Galadriel | gestes réels : talents_icons_compare (hol.Mouse.Click, feaf662 rouge, dd47594 vert) et income_tooltip (hol.Mouse.Hover), 841f7ec ; sites_option « Oui » encore interne ; 45 tool_uses | non | 09:31 |
| GALADRIEL-1115b (fini 10:00) | a0ac58b8de9e19911 | HeroesOfLiberty-Galadriel | income_tooltip sans HoverIncome (10290ba, preflight PASS, Q-income-tooltip-taille), sites_option touches instable 1/4 (retour hol.UI.Choose, cause UHOLDropdown C++ en [ ]), gate Essay sans FAIL ; 37 appels | non | 09:46 |
| CELEBORN-1115b (fini 10:26) | adca45477189ebd5f | HeroesOfLiberty-Celeborn | tremblement de marche en bloc : cause confirmée (AvoidBodies, détour non mémorisé autour d'un voisin de bloc), 805b4ac rouge (vu), 5e01993 correctif cvar hol.Units.MarchMemberDetourSteps (défaut 8), tests 1269/1269 ; A/B x8 3/8 contre 3/8 (aucun « shake » en 16 passes : rien prouvé en jeu) ; Q-march-member-detour ; 45 appels | oui | 09:45 |
| ARWEN-1115b (fini 09:46) | a4ca08c2585246a0b | HeroesOfLiberty-Arwen | preflight stable PASS, 3 fiches E audio au gabarit, quick_check tests 1268/1268, check_audio_* PASS, tableau des licences, Q-audio-licences-tour5 ; fdbe318 ; 20 appels | non | 09:39 |
| GALADRIEL-1115a | a3eab82a8f6b5c483 | HeroesOfLiberty-Galadriel | gestes réels (talents_icons_compare, income_tooltip, sites_option) | non | 09:30 |
