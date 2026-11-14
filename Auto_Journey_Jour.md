# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11), `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_soir_archive_2026-11-14.md` (séance autonome du 2026-11-13 soir : bilan, leçons, tableau d'agents ; résumés du 2026-11-13 matin et du 2026-11-12) ; à la prochaine séance, déplacer de même la journée précédente.
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

## Agents du 2026-11-14

Séance autonome lancée à 10:18, fin 14:15, fusion groupée acceptée. Exception : nettoyage 0a allégé (le dossier Retours\ a déjà été nettoyé le 2026-11-14 au matin : Docs\Archive\Retours_2026-11-14\).

| Agent | id | worktree | tâche | C++ | lancé |
|---|---|---|---|---|---|
| ARWEN-1114a | ab934782ad4dcc5ca | HeroesOfLiberty-Arwen | gain audio Pierre/Forge + nouveaux candidats | non | 10:43 |
| CELEBORN-1114a | a3e34d0decc9b8290 | HeroesOfLiberty-Celeborn | D795 prise de lance B, maa_attack, clore melee-stuck | non | 10:43 |
| ELROND-1114a | ac195c40c7bd23f69 | HeroesOfLiberty-Elrond | garnison Cavalry 0->1 puis B1 Places | non puis oui | 10:43 |
| FARAMIR-1114a | af91a21702c3bf1be | HeroesOfLiberty-Faramir | outils L14 (edit_line, sync/status, build, commit_docs, extend_session) | non | 10:43 |
| GALADRIEL-1114a | adc2746379654fce3 | HeroesOfLiberty-Galadriel | titre du panneau de scénario (repli, EN/JA) | non | 10:43 |
| GLORFINDEL-1114a | aeb37e4f0505ba61e | HeroesOfLiberty-Glorfindel | Sites sacrés suite C++ (S7 Nomade, S6, S3) | oui | 10:43 |
| HALDIR-1114a | a2eb3f9e4ed6fe94d | HeroesOfLiberty-Haldir | BUG nomad_start 1 et 2, puis D796 | oui | 10:43 |
| SAURON-1114a | aee2e8266808d4724 | HeroesOfLiberty-Sauron | neutres en clairière, Placeholders (SANS Arquebusier) | non | 10:43 |
| ARWEN-1114b | — | HeroesOfLiberty-Arwen | audio Pierre/Forge tour 3 : import, scénarios, preflight, fiches | non | 11:0x |
| ARWEN-1114c | — | HeroesOfLiberty-Arwen | D798 élus dans la table, tour 4 voix/tirs/cheval | non | 12:3x |
| ARWEN-1114d | — | HeroesOfLiberty-Arwen | build, import des 3 sons D798, tour 4 corrigé (rejeté ensuite, D801) | build seul | 12:54 |
| CELEBORN-1114b | — | HeroesOfLiberty-Celeborn | planche de chute prise B, preflight spear_arm, E-spear_arm | non | 10:5x |
| CELEBORN-1114c | — | HeroesOfLiberty-Celeborn | résidu #2 melee_stuck x16 | non | 11:1x |
| CELEBORN-1114d | — | HeroesOfLiberty-Celeborn | melee_stuck 8/16 : variance (pas de régression) | non | 12:0x |
| ELROND-1114b | — | HeroesOfLiberty-Elrond | garnison F7 lot 1 (colonne GarrisonSlots) | oui | 11:1x |
| ELROND-1114c | — | HeroesOfLiberty-Elrond | garnison F7 lot 2 serveur | oui | 11:4x |
| ELROND-1114d | — | HeroesOfLiberty-Elrond | garnison F7 : parité script/C++, merge main | oui | 12:3x |
| FARAMIR-1114b | — | HeroesOfLiberty-Faramir | outils L14 : commit_docs -Only, extend_session, FrameWaits | non | 11:0x |
| FARAMIR-1114c | — | HeroesOfLiberty-Faramir | gate_test 80 PASS, outils L14 documentés | non | 11:2x |
| FUSION-1114a | — | copie principale | fusion groupée n°1 (7 branches) -> 0ae4a56 / c8af004 | build | 12:0x |
| FUSION-1114b | — | copie principale | fusion groupée n°2 (net-playtest) -> dd2a053 | build | 12:5x |
| FUSION-1114c | — | copie principale | fusion groupée n°3 (d751-root, visual, maps) -> 61336a4 | build | 13:2x |
| GALADRIEL-1114b | — | HeroesOfLiberty-Galadriel | gestes réels market, legions_warrior, talents_icons_compare | non | 10:5x |
| GALADRIEL-1114c | — | HeroesOfLiberty-Galadriel | panneau de scénario sur une ligne (C++) | oui | 11:1x |
| GALADRIEL-1114d | — | HeroesOfLiberty-Galadriel | textes du panneau localisés EN/JA | oui | 11:3x |
| GALADRIEL-1114e | — | HeroesOfLiberty-Galadriel | sites_option / income_tooltip gestes (besoin C++ constaté) | non | 11:4x |
| GALADRIEL-1114f | — | HeroesOfLiberty-Galadriel | libellés des triches (plan, test rouge) | oui | 12:0x |
| GALADRIEL-1114g | — | HeroesOfLiberty-Galadriel | panneau des triches EN/JA vert (f875747) | oui | 12:1x |
| GLORFINDEL-1114b | — | HeroesOfLiberty-Glorfindel | Sites sacrés S3 rôle Holy (75de2c2) | oui | 11:1x |
| GLORFINDEL-1114c | — | HeroesOfLiberty-Glorfindel | verbes hol.Mouse.Click / hol.Mouse.Hover (573cafb) | oui | 11:5x |
| HALDIR-1114b | — | HeroesOfLiberty-Haldir | Nomade : forum constructible dès le départ | oui | 11:0x |
| HALDIR-1114c | — | HeroesOfLiberty-Haldir | départ dispersé (NomadScatter), D796 couvert des forêts retiré | oui | 11:3x |
| HALDIR-1114d | — | HeroesOfLiberty-Haldir | fondation vue, nomad_start refait | non | 11:5x |
| HALDIR-1114e | — | HeroesOfLiberty-Haldir | D797 groupes sur toute la carte, forum page II | oui | 12:2x |
| HALDIR-1114f | — | HeroesOfLiberty-Haldir | rejeu D797 (dispersion, fondation vues) | non | 13:0x |
| HALDIR-1114g | — | HeroesOfLiberty-Haldir | opacité du panneau Palier II (ef6fc88), nomad_start rejoué | oui | 13:1x |
| SAURON-1114b | — | HeroesOfLiberty-Sauron | rochers et décor des cartes, tour 1 (rocks_compare) | non | 11:00 |
| SAURON-1114c | — | HeroesOfLiberty-Sauron | rochers tour 2 (rocks_compare2, D800) | non | 12:44 |
| SAURON-1114d | — | HeroesOfLiberty-Sauron | enquête sol de biome gris (scénarios sans mapSize) | non | 13:0x |
| docs | — | copie principale | docs, Retours, clôture (cet agent) | non | 13:39 |

### Bilan du 2026-11-14

- **Séance** : autonome 10:18 -> 14:15, fusion groupée acceptée ; utilisateur absent 10:5x-12:1x puis présent (réponses de 12:5x).
- **main** : d761305 -> f539041 par 3 fusions groupées : n°1 = 7 branches (d751-root, visual, galadriel-tab, maps, hero-j8-close, tiers, fog ; main 0ae4a56 / c8af004) ; n°2 = net-playtest (dd2a053) ; n°3 = d751-root, visual, maps (61336a4). Tests 1263 -> 1265 -> 1267 verts ; net, build, aiserver, aiduel PASS à chaque fusion. **RIEN POUSSÉ.**
- **Fait** : outils L14 (edit_line.ps1, ForeignSession dans sync/status, FrameWaits dans check/no_window_test, commit_docs -Only, extend_session, sortie capturée de build_queue_test, gate_test 80 PASS) ; garnison F7 (Cavalry défaut 1 = D793, colonne GarrisonSlots, lot 2 serveur, parité script/C++) ; Nomade (forum constructible dès le départ, départ dispersé puis sur toute la carte D797, forum page II case C D803, couvert des forêts retiré D796) ; Sites sacrés S3 (rôle Holy) ; verbes `hol.Mouse.Click` / `hol.Mouse.Hover` ; panneau de scénario sur une ligne + textes localisés EN/JA + libellés des triches ; audio Pierre/Forge : 3 sons élus D798, tour 4 REJETÉ D801 ; rochers tours 1-2 (D800, D802) ; neutres en clairière ; prise de lance B D795 ; melee_stuck 8/16 = variance (pas de régression, dépend du pas de simulation) ; sol de biome gris = scénarios sans mapSize (pas de régression).
- **Décisions** : D797-D803 (prochain D804). Règles 13-15 de « Qualité des essais » ajoutées (commande complète, AskUserQuestion, Backspace dans les compare).
- **Essais prêts à jouer** : nomad_start ([À ESSAYER], gestes internes). **Questions ouvertes** : Q-arquebusier-modele seulement (chantier exclu à la demande de l'utilisateur).
- **Non fait** : D799 réseau Nomade, garnison lots 3-4, Backspace dans les compare (règle 15), audio tour 5, proportion des rochers, Légions modèle client, régénération des 7 cartes.
- **Non vérifié** : aucun essai joué par une personne sur le build fusionné sauf les retours de l'utilisateur de 12:5x ; sons jamais entendus par un agent ; gestes réels incomplets (talents_icons_compare cases de grille, sites_option « Oui » encore interne, infobulle income).
- **Anomalies de processus** : le classifieur de permissions a refusé plusieurs `git merge main` d'agents (Elrond, Sauron, Galadriel) et un commit groupé ; plusieurs agents ont dépassé 45 appels (jusqu'à ~66) ; un agent de nettoyage a collé des lignes de Auto_Journey_File.md (réparé, dec60d6) ; trois BUILD LENT (358, 649, 817 s : charge de plusieurs agents C++ en parallèle, mémoire virtuelle libre 19-23 Go, pas de blocage) ; ids d'agents permutés dans le tableau (corrigé) ; journal F7 d'Elrond non écrit par l'agent (écrit par la session principale, f3f18cc) ; `edit_line.ps1` coupé par `Select-Object` en sortie de pipe (3 agents).

### Lecons appliquees 2026-11-14

| Règle | État | Preuve |
|---|---|---|
| T2-1 | non évalué | étape 0 / nettoyage 0a allégé (Retours déjà nettoyé le matin) |
| T2-2 | appliquée | 8 agents, un par worktree, lancés à 10:43 (tableau ci-dessus) |
| L3-12 | non évalué | — |
| L5-7 | non évalué | — |
| L4-1, L4-3 | appliquée | fin 14:15 tenue, clôture lancée 13:39, sans prolongation |
| L3-8, L4-9, Q5, L1-14 | non appliquée | aucun intégrateur de recette n'a rejoué les essais sur le build fusionné |
| L3-9 | non évalué | — |
| T2-3 | appliquée | énoncés avec chemins absolus, arrêt dur, rapport fixe (cette clôture) |
| L1-1, L3-6, L4-7, L5-3 | partielle | plusieurs agents au-delà de 45 appels (jusqu'à ~66), aucun au-delà de 80 |
| L1-2 | non évalué | — |
| L1-3 | partielle | `git merge main` refusé par le classifieur de permissions (Elrond, Sauron, Galadriel) |
| L3-2, L4-5 | non évalué | trois BUILD LENT (358, 649, 817 s) |
| L3-5 | non évalué | — |
| L1-8 | partielle | journal F7 d'Elrond écrit par la session principale (f3f18cc) |
| L1-9, L4-8 | appliquée | D797-D803 écrits par la session principale seule |
| L1-7 | appliquée | Q-nomad-forum-palier, Q-rocks_compare2 avec options et défaut |
| L5-6 | non évalué | utilisateur absent au moment des questions |
| T2-5 | non évalué | — |
| T2-6, L5-8 | non évalué | — |
| T2-7, L1-5 | appliquée | Retours et file édités par lignes précises (cette clôture) |
| L4-12, T2 | appliquée | outils partagés corrigés (edit_line, sync/status ForeignSession, check FrameWaits) |
| L4-13 | non évalué | — |
| L3-11, L5-5 | non évalué | régénération des 7 cartes non lancée |
| L3-7, L1-4 | non évalué | — |
| T2-4 | appliquée | au plus 2 questions par chantier (Nomade : Q-nomad-net, Q-nomad-forum-palier) |
| Q1 | appliquée | nomad_start : preflight PASS, PNG vus (HALDIR-1114g 13:23) |
| Q2 | appliquée | E-nomad_start VU À L'ÉCRAN, gestes internes déclarés |
| Q3 | non évalué | — |
| Q4 | non évalué | — |
| Q5, L5-4 | non appliquée | gestes réels incomplets, pas de recette indépendante sur main fusionné |
| Q6, Q7 | partielle | audio tour 4 réglé par la synthèse puis rejeté (D801) : tour 5 change de source (échantillons) |
| Q8 | appliquée | Backspace des compare = ligne `[ ]` en tête de la section Faramir |
| Q9 | non évalué | — |
| Q11 | appliquée | 1 essai donné (nomad_start) |
| Q12, L1-6 | appliquée | réponses de 12:5x enregistrées en D797-D803 |
| L4-11 | appliquée | rocks_compare / rocks_compare2 / audio_compare = scénarios de revue ; règle 15 (Backspace) ajoutée |
| L3 / T2 | appliquée | 3 fusions groupées, un build chacune, tests 1263 -> 1267 |
| L1-3 (fusion) | partielle | cf. refus du classifieur ci-dessus |
| L1-13 | non évalué | — |
| L6-2 | non évalué | — |
| L6-4 | non évalué | — |
| L6-6 | non évalué | — |
| L7-5 | non évalué | — |
| L7-7 | non évalué | — |
| L8-2 | appliquée | melee_stuck x16 = 8/16, variance prouvée (CELEBORN-1114c/d) |
| L8-4 | non évalué | — |
| L8-8 | non évalué | — |
| L9-1 | non évalué | — |
| L9-4 | appliquée | CRLF vérifiés ; lignes collées de la file réparées (dec60d6) |
| L9-6 | non évalué | — |
| L9-8 | non évalué | — |
| L9-9 | appliquée | réponses de l'utilisateur présent appliquées le jour même (D797-D803) |
| L9-10 | non appliquée | sons jamais entendus par un agent ; tour 4 rejeté à l'écoute |
| L10-2 | non évalué | — |
| L10-3 | non évalué | — |
| L10-4 | non évalué | — |
| L11-3 | non évalué | — |
| L11-4 | appliquée | 17 lignes faites retirées de la file (Docs\Archive\Auto_Journey_File_faits_2026-11-14_soir.md) |
| L11-8 | non évalué | — |
| L11-9 | non appliquée | aucun intégrateur de recette nommé |
| L11-10, L10-5, L9-7 | appliquée | mêlée mesurée à N=16 |
| L11-11 | non évalué | — |
| L11-12 | appliquée | 6 fiches répondues rangées (Docs\Archive\Retours_2026-11-14_c\) |
| L11-13 | appliquée | heures par Get-Date |
| L12-1, L12-1b | non évalué | BUILD LENT expliqué par la charge des agents C++ en parallèle (19-23 Go libres) |
| L12-2 | appliquée | sync_worktrees en fin de séance (clôture) |
| L12-3 | non évalué | — |
| L12-4 | non évalué | — |
| L12-5 | non évalué | — |
| L12-6 | appliquée | EtapeDefini mis à jour après la fusion n°3 (f539041) |
| L12-7 | non évalué | — |
| L13-1 | non évalué | — |
| L13-2 | non évalué | — |
| L13-3 | non évalué | — |
| L13-4 | appliquée | net PASS à chaque fusion |
| L13-5 | non évalué | — |
| L13-6 | non évalué | — |
| L13-7 | non évalué | — |
| L13-8 | non évalué | — |
| L14-1 | non appliquée | dépassements jusqu'à ~66 appels |
| L14-2 | non évalué | — |
| L14-3 | appliquée | ForeignSession dans sync_worktrees / status |
| L14-4 | non évalué | utilisateur absent ; règle 14 ajoutée |
| L14-5 | partielle | verbes hol.Mouse faits ; talents_icons_compare, sites_option encore internes |
| L14-6 | non évalué | — |
| L14-7 | non évalué | — |
| L14-8 | appliquée | FrameWaits dans check.ps1 / no_window_test |
| L14-9 | appliquée | panneau de scénario sur une ligne, EN/JA |
| L14-10 | non évalué | — |
| L14-11 | appliquée | fusions n°2 et n°3 courtes, agents finis |
| L14-12 | appliquée | commit_docs -Only ; commit de clôture par nom |
| L14-13 | non évalué | — |
