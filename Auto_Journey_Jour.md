# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir) et `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Dernier bilan : 2026-11-11 (13:24-18:02, détail dans l'archive du 2026-11-12)

- Main = fd03266 après 3 fusions (rien poussé) ; tests 1173/1173, aiserver, aiduel PASS ; net non relancé sur le résultat fusionné.
- Fusionné : N7 état des camps, Palier 3 G4-G6 d (bouton), T4b Mains du guérisseur, Légions L4 (rage, IA, traductions), S5 prouvé sur le réseau, notereset/preflight, neutrals 3v3, essais de Sauron ; build de main réparé par Haldir (39ba5ec).
- Non fusionné : galadriel-tab a3e90a8 (conflit notereset avec fog), net-playtest c8bde0e/WIP S6 (test ROUGE), visual f0670cf+ (scénarios) : lignes 95, 100 et 57 de `Auto_Journey_File.md`.
- Scénario neutrals ramené à 3 Hommes d'armes : masque le résidu D641 (noté dans `NeutralsInProgress.md` et `Retours\E-neutrals.md`).
- Réponses de l'utilisateur du 2026-11-11 aux Q-*.md reportées (commit bacee4b) ; `Retours\ESSAIS-A-JOUER.md` nettoyé le 2026-11-12 (0 essai prêt).
- Leçons non appliquées le 2026-11-11 : plafond de 45 appels (agents à 46-64), commits de docs sur main pendant une fusion, calls.md partagé ; 2 tips rouges commités par des agents.

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

## Programme du 2026-11-12 (séance AUTONOME, fin 13:00, plan de l'étape 0 écrit à 10:40)

Départ : main = b62464f (nettoyage des docs commité). Fusion groupée dans main autorisée par l'utilisateur, jamais de push. sync_worktrees lancé avec -NoBuild (exception : séance de 2 h 30, les agents compilent à leur premier build) ; Galadriel en conflit (fusion de main annulée, `Source/HeroesOfLiberty/Dev/HOLTestSequence.cpp`, verbe notereset en double avec fog). Glorfindel (net-playtest 6941e5b) : binaire périmé, +5 commits hors de main dont le WIP S6 rouge (2d47271, 0c8792c).

Plafond : 3 agents C++ en même temps. Premiers C++ : Galadriel, Glorfindel, Elrond. Arwen et Celeborn démarrent sans C++ et passent au C++ à la libération d'une place.

| Worktree | Branche | Tâche de départ (ligne de `Auto_Journey_File.md`) | C++ | Suivante |
|---|---|---|---|---|
| Arwen | d751-root | Neutres N7 côté script (ligne « sans C++ » N7), puis ligne C++ N7 point sur la minicarte + réplication des camps vus ; `NeutralsInProgress.md` | non au départ | N7 C++ à la 1re place libre |
| Celeborn | hero-j8-close | BUG neutrals : bras du Lancier dans le corps, diagnostic par capture d'abord (Lancier neutre contre Lancier du joueur) ; `NeutralsInProgress.md` | non au départ | correctif (C++ si la pose est codée), puis BUG-melee-stuck-attack |
| Elrond | tiers | Palier 3 G6 e (réplication de la phase) puis f (scénario deploy) ; `Palier3InProgress.md` | oui | bouton case D, G4 filtre Faction |
| Faramir | fog | Scénarios qui ne démarrent jamais chez l'utilisateur (legions_roster_gesture, disconnect_1v1, audio_compare_buildings_stone/_metal) ; cas ajouté au preflight | non | Q-play-hidden-wait (A en place, C verbe quit) ; worktree de la fusion groupée |
| Galadriel | galadriel-tab | 1er geste : fusion de main, conflit HOLTestSequence.cpp (garder notereset de fog), build ; puis Rage L4 plafond 40 % (script), Rage visuel défaut 4 ; `LegionsInProgress.md` | oui | REGLAGE market, REGLAGE talents_icons_compare ; infobulle Caserne après la fusion de net-playtest |
| Glorfindel | net-playtest | 1er geste : build (binaire périmé) ; infobulle de revenu par ressource (Q-sites-gold-line = Autre), abandon de GoldLine et du rouge 0c8792c ; `SacredSitesInProgress.md`, `InterfaceInProgress.md` | oui | Sites S7 option de partie |
| Haldir | maps | Pas de régénération des 7 cartes (éditeur seul, file de build vide : impossible avec 7 agents) ; chapitre du GDD : D384 Centres secondaires contre `RelaysInProgress.md` (plan et règles pures) | non | § 5.2 / § 6.2 à relire |
| Sauron | visual | Placeholders ressources : figer Arbre = C, baies / pierre / or = A ; `PlaceholdersInProgress.md` | non | ligne `[?]` Trajets d'ordres de `Placeholders.txt` (OK de l'utilisateur) |
| Outils/docs | (session principale) | Reporter Q-legions-ai-warrior, Q-play-hidden-wait, Q-talents-heal-visual dans leur journal ; `-NoCpp` au modèle d'énoncé | non | réécrire la ligne « essais ratés du 10-31 » |

Fusion groupée de fin de séance : dans Faramir (fog = main, arbre propre), candidates = tiers, d751-root, hero-j8-close, fog, visual, galadriel-tab (conflit résolu), net-playtest (seulement si le WIP S6 est remplacé par un tip vert), maps (docs seules).

## Agents du 2026-11-12

| Nom | Id | Worktree | Branche | Tache | Lance |
|---|---|---|---|---|---|
| ARWEN-1112a | adfd8d813d0ee3470 | Arwen | d751-root | Neutres N7 (script puis C++ minicarte) | 10:39 |
| CELEBORN-1112a | a0ac31f1f11ca88cc | Celeborn | hero-j8-close | BUG bras du Lancier neutre, puis melee-stuck | 10:39 |
| ELROND-1112a | ad24ecdf475537e57 | Elrond | tiers | Palier 3 G6 e/f, bouton case D, G4 filtre (C++ 1) | 10:39 |
| FARAMIR-1112a | ab77d8eddd5c5a203 | Faramir | fog | Scenarios jamais demarres + preflight, Q-play-hidden-wait | 10:39 |
| GALADRIEL-1112a | ae7e4e7f809c1165b | Galadriel | galadriel-tab | Conflit de fusion, Rage L4 plafond et visuel (C++ 2) | 10:39 |
| GLORFINDEL-1112a | a2bf758e6f3867c63 | Glorfindel | net-playtest | Build, infobulle de revenu, retrait GoldLine (C++ 3) | 10:39 |
| HALDIR-1112a | a97ce746cad9802ef | Haldir | maps | Chapitre GDD D384 Centres secondaires (sans C++) | 10:39 |
| SAURON-1112a | a6d3ffa74439af3cb | Sauron | visual | Styles des ressources figes, Placeholders | 10:39 |
| DOCS-1112a | aaffdd14aada5859e | principale | main | Reponses a reporter, file Outils, checklist L9-4 | 10:39 |
| FARAMIR-1112b | a08fc0e0731156818 | Faramir | fog | Verification de bout en bout des correctifs de lancement (build, preflight complet) | 10:53 |
| HALDIR-1112b | adb6472c08608ea72 | Haldir | maps | File Haldir a jour, chapitre GDD suivant (plan + regles pures, sans C++) | 10:56 |
| ELROND-1112b | a6c5eb1961aa98db1 | Elrond | tiers | Build, tests de G6 e et G4, scenario deploy (G6 f) | 11:00 |
| HALDIR-1112c | a0cb33d97eebfddb7 | Haldir | maps | Outils sans C++ : eol_check dans le gate Close, qground, lecon file de build | 11:02 |
| PLAN-1112a (Fable) | ab57a1aadaee24405 | principale | main | Releve neuf des chapitres du GDD sans code (lecture seule) | 11:02 |
| CELEBORN-1112b | af0a38de549450e8d | Celeborn | hero-j8-close | Pose de la lance corrigee (main gauche devant le torse), rebake, capture en jeu | 11:10 |
| HALDIR-1112d | a9afb7cff505ee4f0 | Haldir | maps | Diagnostic en lecture seule de la lenteur des builds (17 min au lieu de 200 s) | 11:28 |
| HALDIR-1112e | a095437eadcfc8d68 | Haldir | maps | Chapitre A1 Signaux sur la carte : journal + regles pures en script (sans build) | 11:33 |
| DOCS-1112b | a1d82b0c5596b555a | principale | main | Decisions du 2026-11-11 au GDD § 22 (GDD non commite) | 11:33 |
| HALDIR-1112f | a0480ef84ff6458e1 | Haldir | maps | Chapitre A2 choix de specialisation de palier : journal + regles pures en script (sans build) | 11:46 |
| HALDIR-1112g | aa042c6c1dd71fc9f | Haldir | maps | Chapitre A3 option Nomade : journal + regles pures en script (sans build), arbre propre a 12:15 | 11:59 |
| FUSION-1112 | a99b6f3c8d7b723e1 | Haldir | maps | Fusion groupee sans C++ : maps + fog + hero-j8-close, tests, sans avance rapide (faite par la session principale) | 12:11 |
| VERIF-1112 (Fable) | ad15a2665d5c49e03 | principale | main | Recette independante de la fusion, des 4 chapitres, des outils et de Retours (lecture seule) | 12:20 |
| CLOTURE-1112a | a408de02f66b91cc8 | principale | main | Corrections de docs demandees par la recette (Retours, LISEZ-MOI, EtapeDefini, fiches E-*, lignes de file) | 12:29 |

GLORFINDEL-1112a fini 12:43 (net-playtest WIP 6703b9f ; 58 appels) : GoldLine S6 retiree, infobulle de revenu D780 posee (souche GetIncomePerMinute, test rouge Gather.Income, infobulle dans RefreshTopBar), NON COMPILE, NON TESTE (1er build en echec sur un arbre a moitie edite, corrige dans les sources, non confirme) ; Q-income-tooltip-rate ; branche pas prete.
ARWEN-1112a fini 12:40 (d751-root 3955b05, 42cc580, WIP 308419b ; 39 appels) : Neutres N7 point de camp sur la minicarte ecrit (CampViews replique au proprietaire, GatherCamps, scenario neutrals_minimap), NON COMPILE, NON TESTE ; Q-neutrals-camp-hover ; branche pas prete.
SAURON-1112a fini 12:39 (visual a6e673f, b315f0b, 31194ae, 6c7292f ; ~48 appels) : styles des ressources figes dans HOLSourceField.cpp (arbre C, baies/pierre/or A), NON COMPILE, jamais vu a l ecran ; Q-resources-tree-size ; Placeholders « marques d ordres » [?] -> [style] ; branche pas prete.
FARAMIR-1112b fini 12:39 (fog 87c4682, fff9792 dans main ; 0e9a649, e517855 docs hors main ; ~30 appels) : faille du controle de binaire perime trouvee et corrigee (comparaison a la date du commit -> au fichier C++ le plus recent), FAIL prouves sur cas reels ; preflights complets de legions_roster_gesture et disconnect_1v1 NON FAITS (pas de build), etape 41 jamais vue.
ELROND-1112b fini 12:38 (tiers WIP cc3213e ; ~34 appels) : scenario deploy ecrit, jamais lance ; G6 e et G4 toujours NON COMPILES, aucun test vu ; branche pas prete.
Anomalie d outil relevee par 2 agents : un appel bloquant passe en arriere-plan apres 180 s, donc build.ps1 -Wait ne bloque pas l agent ; Wait-Process a expire sans fin de build.
CLOTURE-1112a fini 12:37 (e8ec836, 49 appels) : index de Retours a 20 fichiers (2 BUG, 7 E, 1 F, 8 Q), 5 fiches E-* en [NE PAS JOUER], EtapeDefini (4 chapitres « dans main 4851635 », ligne FUSION GROUPEE), 6 lignes de file ; Placeholders et 3 Q-* commites.
12:38 : arret dur depasse pour ARWEN-1112a, ELROND-1112b, FARAMIR-1112b, GLORFINDEL-1112a, SAURON-1112a, tous bloques dans build.ps1 -Wait (le build d Elrond tourne depuis 12:03, 6 tickets derriere) : message d arret envoye a chacun (commit WIP non compile, journal, rapport). Les builds en file sont laisses en vie.
VERIF-1112 fini 12:26 (20 appels, lecture seule) : GIT PASS (main 4851635, rien pousse, aucun C++ entre aujourd hui) ; CONTENU PASS avec reserves (les 4 chapitres n ont aucun appelant : aucun effet en jeu ; tests de Nomade et Centres secondaires en partie triviaux ; 30/45/60 s jamais assertis) ; OUTILS FAIL (LaunchCheck ne regarde qu une dll sur deux, eol_check PASS a tort si git ls-files echoue) ; RETOURS FAIL (index perimes, 5 fiches E-* a titres contradictoires) ; DOCS FAIL partiel (EtapeDefini dit encore « pas dans main »). Corrections confiees a CLOTURE-1112a, defauts d outils mis en file.
FUSION-1112 fini 12:17 (12 appels) : maps + fog fff9792 + hero-j8-close 1d12401 fusionnes dans Haldir, aucun conflit, diff C++ vide, tests 1200/1200, values PASS, launch_check_test 14 PASS sur ee5dd8c. 12:18 : main avance par --ff-only a 4851635 (ee5dd8c + docs GDD/EtapeDefini/Jour seulement), gate Merge sans FAIL, RIEN POUSSE. Binaire de la copie principale toujours perime (aucun build possible, machine saturee).
HALDIR-1112g fini 12:09 (maps 938087b/ae9d9af, merge 525d636, 34 appels, tests 1200/1200) : chapitre option Nomade ouvert, regles pures, Q-nomad ; branche prete.
GALADRIEL-1112a fini 12:10 (galadriel-tab 24461a8 fusion de main, 236e56b test rouge, 56 appels) : build de la fusion PASS en 3994 s (dont ~60 min d attente), tests 1171/1174 (3 rouges = ses tests rouges) ; correctifs rage 40 % et hol.Rage.Visual 4 ECRITS, NON COMMITES, NON COMPILES ; branche PAS prete. Non relance (aucun build possible avant 13:00).
12:12 : accord de l utilisateur pour commiter GDD.md (616719a, D770-D782). Gate Merge de 12:10 : FAIL GDD.md non commite (leve par ce commit) et branches en retard de mes commits de docs (l agent de fusion refusionne main).
HALDIR-1112f fini 11:57 (maps 567fd50/1124a13/581f21b, merge 3ff176d, 40 appels, tests 1193/1193) : chapitre choix de specialisation ouvert, regles pures, Q-specialization ; branche prete.
Constat 11:52 (question de l utilisateur « pourquoi ca s est degrade ») : 1,2 Go de RAM libre sur 64 ; 6 editeurs ProjectShiva = 33 Go reserves (5 lances entre 11:38 et 11:49), StarRail 10 Go, ProjectRED 5 Go, HeroesOfLiberty ~5 Go. Galadriel compile depuis 10:56.
HALDIR-1112e fini 11:43 (maps 76f66b8/9d30e2c/16a51cd, merge 8d918a6, 35 appels, tests 1186/1186) : chapitre Signaux ouvert, regles pures, Q-signals (duree de vie, coequipier elimine) ; branche prete.
Constat 11:43 : Galadriel compile toujours (depuis 10:56), 7 tickets derriere, 4,3 Go de RAM libre.
DOCS-1112b fini 11:37 (219f4eb, 40 appels) : D770-D782 ecrites au GDD § 22 (reponses du 2026-11-11), prochain numero D783 ; GDD.md modifie NON commite (accord de l utilisateur requis) ; journaux des chantiers et EtapeDefini a jour.
Plan de fin revu a 11:33 : la fusion groupee se fera dans HALDIR (binaire a jour, pas de build a attendre) a 12:20 avec les branches sans C++ (maps, fog, hero-j8-close) plus celles reellement compilees et vertes ; pas dans Faramir comme prevu (son build attend dans la file).
HALDIR-1112d fini 11:30 (maps 93d95fd, 16 appels) : CAUSE de la lenteur des builds = machine saturee, pas le volume (18 actions comme hier, 1006 s au lieu de ~120 s ; « Low on memory 86.7gb/89.8gb » dans le journal UBT). Charge : 5 compilations ProjectShiva (autre moteur, invisibles pour build.ps1), 2 editeurs ProjectShiva, 2 jeux ouverts depuis 9:05. Echec de Glorfindel = erreur de code (GetIncomePerMinute, GetGoldIncome). Utilisateur prevenu a 11:31 (notification). Lecon L12-1b ecrite.
CELEBORN-1112b fini 11:28 (hero-j8-close a786d64, merge 1d12401, 44 appels) : aucune pose credible avec la prise actuelle (~40 variantes mesurees), question Q-spear-grip (A garder, B mains echangees recommande, C lance au centre), rien vu en jeu ; branche prete (docs). Celeborn NON relance : toute suite demande un binaire frais et la file de build est bloquee (exception, en attente du diagnostic HALDIR-1112d).
HALDIR-1112c fini 11:26 (maps d1933b7/4852870, merge 0e1b908, 51 appels) : eol_check branche dans le gate Close (WARN, gate_test tout PASS en 490 s), risque qground ferme pour 2 scenarios, lecon L12-1 (debit ~1 build / 17 min, mediane d attente ~21 min) ; branche prete.
CELEBORN-1112a fini 11:08 (hero-j8-close 966ae02/57bb7c4/60365b3, merge b9db9f1, 45 appels) : cause du bras dans le corps = animation A_Spearman_Attack (coude gauche 17,7 cm dans le tronc), test rouge arm_body_check.py, pas corrige, pas vu en jeu (binaire perime). PLAN-1112a fini 11:12 (46 appels) : releve neuf des chapitres ecrit (A = 4, B = 6, C = 27).
Constat 11:08 : premier build de Glorfindel = 1038 s et FAIL (10:39-10:56) ; Galadriel compile depuis 10:56 ; 6 tickets derriere.
ELROND-1112a fini 10:59 (tiers e13e1f7/2210424/38cca88, merge 6810ce6, 52 appels) : G6 e et G4 filtre ecrits, NON COMPILE (build en file 16 min sans demarrer), branche pas prete. HALDIR-1112b fini 10:59 (maps fbecf19, merge ea8aeeb, 44 appels) : liste des chapitres GDD epuisee, 4 lignes d ecarts du § 6.2 en file.
Constat 11:00 : file de build = goulot (7 tickets, le premier build tourne depuis 10:40).
HALDIR-1112a fini 10:54 (maps 12aee4a/91b39ce/b140bd7, merge 1b5a1ed, 48 appels, tests 1180/1180) : D384 pas couvert par les relais, regles pures des Centres secondaires en script, Q-secondary-centers (cout, base de population) ; branche prete.
Rapports : DOCS-1112a fini 10:45 (6d77b69, 44 appels). FARAMIR-1112a fini 10:50 (fog 323b73a/42e5e41/953fafd, main 3b94faf, 45 appels) : causes des 4 essais jamais demarres = joueur non nomme (disconnect_1v1), sons non commites d Arwen (audio_compare), binaire perime (legions_roster_gesture) ; binaire de la copie principale perime : a recompiler apres la fusion.
Exception : FARAMIR-1112b lance avec le gate -NoCpp alors qu il compile (7 worktrees dans la file de build) : la verification des essais est la priorite de la seance, il attend son tour par build.ps1 -Wait.

Modeles (consigne du 2026-11-12) : etape 0 et verification sur Fable 5.2, tout le reste sur Opus 5.5. Arret dur des agents 12:30, fusion groupee dans Faramir ensuite (sans push), fin 13:00.

## Bilan du 2026-11-12

Ligne de cloture (12:48) : CLOTURE-1112b (afdf07fb6d01c8712, 14 appels) a remis Arwen, Elrond et Galadriel a jour de main (conflits de journaux resolus par union ; correctifs rage de Galadriel commites en WIP 3224cbd). BILAN-1112 (abdff72b09ac0cad5, 20 appels) a ecrit ce bilan (060fc05). sync_worktrees -NoBuild de fin a 12:46 : les 8 worktrees a jour de main, binaires perimes partout sauf Haldir. session_gate -Phase Close a 12:47 : aucun FAIL ; WARN : Docs\SessionEnd.txt pas mis a jour (date du 2026-11-10), fins de ligne melees (copie principale 35 fichiers d archive, Celeborn 20, Arwen 1, Glorfindel 1, Sauron 1), 5 fiches E-* avec reponse encore dans Retours\ (gardees expres : essais NE PAS JOUER a redonner). Rien pousse. Paquet et serveur non refaits (machine saturee). 26 agents au total.

Seance autonome 10:26 -> 13:00, 24 agents (Opus 5.5 pour le travail, Fable 5.2 pour l'etape 0, le releve du GDD et la recette). Fusion sans push autorisee par l'utilisateur. Rien pousse.

DANS MAIN (4851635 puis commits de docs) :
- 4 chapitres du GDD ouverts en regles pures, sans effet en jeu (aucun appelant) : Centres secondaires (D384), Signaux (A1), choix de specialisation de palier (A2), option Nomade (A3). Tests 1200/1200.
- Outils : controle de lancement des scenarios (binaire perime compare au fichier C++ le plus recent, FARAMIR-1112b) ; eol_check branche dans le gate Close (HALDIR-1112c).
- Diagnostic des 4 essais jamais demarres (joueur non nomme, sons non commites, binaire perime).
- Diagnostic du bras du Lancier : animation A_Spearman_Attack, test rouge arm_body_check.py, pas corrige.
- D770-D782 au GDD § 22 (616719a, accord de l'utilisateur a 12:12), prochain D783.
- Nettoyage des docs (etape 0a) et corrections de la recette (CLOTURE-1112a, e8ec836).

ECRIT MAIS NON COMPILE (branches pas pretes) :
- Arwen d751-root WIP 308419b : Neutres N7, point de camp sur la minicarte. Suite : build, test, scenario neutrals_minimap.
- Elrond tiers WIP cc3213e : Palier 3 G6 e et G4 filtre, scenario deploy jamais lance. Suite : build, quick_check tests,net, preflight deploy.
- Galadriel galadriel-tab 236e56b : fusion de main compilee (build PASS 3994 s), correctifs rage 40 % et hol.Rage.Visual 4 ecrits NON COMMITES. Suite : commit, build, tests.
- Glorfindel net-playtest WIP 6703b9f : infobulle de revenu D780, GoldLine retiree. Suite : code vert dans UHOLGatherProcessor, build, test Gather.Income.
- Sauron visual 6c7292f : styles des ressources figes (arbre C, baies/pierre/or A). Suite : build, preflight, PNG vus.

FAIT MARQUANT : la file de build a ete bloquee par la machine saturee. Build de Glorfindel 1038 s en echec. Galadriel 3994 s dont ~60 min d'attente. 1,2 Go de RAM libre a 11:52 ; 6 editeurs ProjectShiva = 33 Go, 2 jeux = 15 Go. 5 agents C++ sur 5 n'ont rien pu compiler apres 10:56.

ATTEND L'UTILISATEUR :
- 8 questions dans Retours\ : Q-income-tooltip-rate, Q-neutrals-camp-hover, Q-resources-tree-size, Q-spear-grip, Q-secondary-centers, Q-signals, Q-specialization, Q-nomad.
- 0 essai jouable : binaire de la copie principale perime, a recompiler machine calme.
- Recette : Q-income-tooltip-rate recommande C mais le code pose A ; dans Galadriel, le plafond de rage est monte des deux cotes (bonus ET malus) : a confirmer.

REPRISE : premiere tache de chaque worktree = `Tools\build.ps1` machine calme, puis quick_check, avant tout nouveau code.

### Lecons appliquees 2026-11-12

| Regle | Etat | Preuve |
|---|---|---|
| 0a nettoyage des docs avant tout agent | oui | DOCS-1112a et nettoyage faits avant les lancements de 10:39 |
| T2-1 etape 0 | oui | plan ecrit a 10:40 (## Programme du 2026-11-12), Fable 5.2 |
| T2-2 un agent par worktree d'emblee | oui | 9 agents lances a 10:39 (8 worktrees + docs) |
| L3-12 | sans objet | etape 0 faite |
| L5-1 / L3-10 / L4-2 Get-Date et gate Launch | oui | gate Launch avant chaque lancement ; dernier lancement 12:29 (CLOTURE-1112a) |
| L5-2 tableau agent -> id | oui | 24 lignes avec id ; messages d'arret de 12:38 aux 5 ids du tableau |
| L3-1 / L4-4 plafond de 3 agents C++ | non | 7 tickets en file a 11:00 ; Arwen, Sauron, Celeborn, Faramir ont compile en plus des 3 agents C++ declares |
| L1-10 docs commites | en partie | gate Merge 12:10 FAIL GDD.md non commite, leve par 616719a |
| L5-7 accord de commit | oui | accord de l'utilisateur pour GDD.md a 12:12 |
| L4-1 / L4-3 arret dur | non | arret 12:30 depasse : 5 agents bloques dans build.ps1 -Wait, arretes par message a 12:38, rapports jusqu'a 12:43 |
| L3-8 / L4-9 integrateur nomme | en partie | FUSION-1112 et VERIF-1112 nommes a 12:11 et 12:20, pas au lancement |
| L3-9 appels de la session principale | non verifie | rien au journal |
| T2-3 enonce type | non verifie | enonces non relus par ce bilan |
| L1-1 / L3-6 / L4-7 / L5-3 plafond 80 appels | oui | tous sous 60, max 58 (GLORFINDEL-1112a) |
| L9-2 arret a 45 appels vus | en partie | 8 agents au-dessus de 45 : 58, 56, 52, 51, 49, ~48, 48, 46 |
| Agents sous 300k tokens | oui | max ~187k |
| L1-2 commit par lot | en partie | correctifs rage de Galadriel NON COMMITES ; 4 WIP commites seulement sur ordre a 12:38 |
| L1-3 git merge main avant le rapport | oui | merges Haldir (1b5a1ed... 525d636), Celeborn 1d12401, Galadriel 24461a8, Elrond 6810ce6 |
| L3-2 / L4-5 build dans les 15 min, NON COMPILE annonce | en partie | builds demandes tot mais en file ; 5 branches annoncees NON COMPILE |
| L3-5 un lot de headers | non verifie | - |
| L1-8 un statut par agent | oui | journaux + EtapeDefini par CLOTURE-1112a |
| L1-9 / L4-8 numeros D, defauts codes | oui | GDD ecrit par DOCS-1112b seul (D770-D782) ; Q-signals, Q-nomad, Q-specialization avec defauts codes |
| L1-7 / T2-4 questions | oui | 1 question par chantier ; 4 questions de Haldir reportees (plafond atteint) |
| L5-6 tableau de questions | sans objet | seance autonome |
| T2-5 build avant quick_check | oui | Galadriel build PASS puis tests 1171/1174 |
| T2-6 / L5-8 script de bout en bout | en partie | scenarios deploy et neutrals_minimap jamais lances |
| T2-7 / L1-5 Retours par fichier | oui | 8 Q-*.md dans Retours\ |
| L1-11 / L10-1 build fige, pas de fenetre | oui | aucune commande play.ps1 donnee |
| L3-4 ecriture au bon endroit | voir ligne de cloture | gate Close |
| L4-12 outil partage corrige | oui | LaunchCheck corrige par FARAMIR-1112b ; defauts restants mis en file (recette) |
| L4-13 / L3-11 / L5-5 / L6-6 / L9-7 / L10-5 / L11-10 | sans objet | ni UHT, ni regeneration, ni mesure |
| L3-7 / L1-4 veilleur | non verifie | - |
| Q1-Q4, Q6, Q7, Q9, L4-11, L7-6, L8-1, L8-4, L9-10 essais | sans objet | aucun essai produit ; preflights de FARAMIR-1112b NON FAITS |
| Q5 / L5-4 / L11-9 recette independante | oui | VERIF-1112 (Fable) 12:26 ; 5 defauts corriges par CLOTURE-1112a (e8ec836) |
| Q8 defaut commun en tete de file | oui | lignes PREMIER GESTE ajoutees par ce bilan |
| Q10 / L4-10 / Q11 aucun essai sans preflight | oui | 0 essai donne |
| Q12 / L1-6 / L11-4 reponses et cochage | voir ligne de cloture | gate Close |
| L3 / T2 fusion groupee | oui | FUSION-1112 maps + fog + hero-j8-close, tests 1200/1200, --ff-only a 4851635 |
| L4-6 / L7-7 / L8-8 fusion anticipee, sans rebuild | oui | fusion lancee 12:11, diff C++ vide |
| Pre-vol gate Merge | en partie | FAIL GDD.md non commite, leve par 616719a avec accord |
| L9-6 / L11-8 branches figees | en partie | liste figee, mais FARAMIR-1112b a commite sur fog jusqu'a 12:39 (0e9a649, e517855 hors main) |
| L4-14 / L8-9 sync des worktrees, debut | en partie | sync_worktrees -NoBuild (exception notee) : DLL du 2026-11-11 partout, une cause de la file chargee |
| L4-14 / L8-9 sync des worktrees, fin | voir ligne de cloture | - |
| L1-13 test rouge avant correctif | en partie | Haldir rouge puis vert (12aee4a -> 91b39ce, 76f66b8 -> 9d30e2c, 567fd50 -> 1124a13, 938087b -> ae9d9af) ; Glorfindel, Arwen : rouge non observe (non compile) |
| L5-bilan | oui | ce titre |
| L6-2 extrait de la file | non verifie | - |
| L6-4 bug rejoue au geste reel | en partie | bras du Lancier mesure par arm_body_check.py, rien vu en jeu |
| L6-5 build apres git merge main | non | binaires perimes dans tous les worktrees apres fusion de main |
| L7-1 tool_uses releves | oui | appels notes a chaque rapport |
| L7-2 / L11-1 binaire perime au Launch | en partie | faille corrigee par FARAMIR-1112b ; recette : LaunchCheck ne regarde qu'une dll sur deux |
| L7-3 / L7-4 / L8-5 / L10-4 / L11-11 / L11-13 | non verifie | - |
| L7-5 stub + test rouge avant build | en partie | Glorfindel stub GetIncomePerMinute + test rouge, rouge non observe |
| L8-2 audit melee bloquee | non | BUG-melee-stuck pas repris |
| L8-6 / L9-4 fins de ligne | oui | eol_check au gate Close (HALDIR-1112c, dans main) |
| L9-1 re-notification en boucle | oui | aucune boucle relevee au journal |
| L9-5 commit_docs avant Merge | en partie | GDD.md hors couverture : FAIL Merge 12:10 |
| L9-8 chapitres GDD libres | oui | releve neuf PLAN-1112a 11:02 (A = 4, B = 6, C = 27), 4 chapitres ouverts par Haldir |
| L9-9 mode interactif | sans objet | une question de l'utilisateur a 11:52, reponse donnee |
| L10-2 | sans objet | pas de net rouge |
| L10-3 binaire avant quick_check de fusion | oui | FUSION-1112 : diff C++ vide, launch_check_test 14 PASS sur ee5dd8c |
| L11-2 build.ps1 -Wait | en partie | utilise, mais l'appel passe en arriere-plan apres 180 s : l'agent ne bloque pas vraiment |
| L11-3 verdict avant agent C++ | sans objet | - |
| L11-5 / L11-6 | oui | faits le 2026-11-11 |
| L11-7 gate -NoCpp | en partie | -NoCpp utilise pour FARAMIR-1112b qui compilait (exception notee) |
| L11-12 purger Retours | oui | DOCS-1112a 10:45, index de Retours a 20 fichiers (CLOTURE-1112a) |
| L12-1 / L12-1b charge machine | non | ecrites a 11:08 et 11:28, aucun outil ; ProjectShiva non mis en pause |
| Pas de push | oui | VERIF-1112 GIT PASS, rien pousse |
| Gate Close | voir ligne de cloture | - |

### Lecons nouvelles 2026-11-12

- L12-1c charge machine : `session_gate -Phase Launch` WARN si un cl.exe d'un autre moteur tourne ou si la memoire virtuelle libre < 8 Go ; `build.ps1` attend tant que < 8 Go et l'ecrit dans son journal et dans `BuildQueue.csv`.
- L12-2 sync de debut : `sync_worktrees.ps1` SANS `-NoBuild` avant le premier lancement ; sinon `session_gate -Phase Launch` FAIL si un worktree a plus de 5 commits `Source` de retard sur sa dll.
- L12-3 attente de build : `build.ps1 -Wait` ne bloque pas (arriere-plan apres 180 s) : l'enonce donne du travail sans binaire pendant l'attente et un delai maximal d'attente (`-MaxWaitMin`) apres lequel l'agent commite en WIP et rend la main.
- L12-4 binaire d'abord : un agent C++ lance `LaunchCheck` (son binaire demarre) avant d'ecrire du code ; champ de rapport `LAUNCHCHECK_DEBUT`.
- L12-5 regles pures : au moins un test par regle qui passe par une vraie chaine, aucun test tautologique ; la recette compte les asserts sur valeur calculee (champ `TESTS_TAUTOLOGIQUES : 0`).
- L12-6 index de docs : l'agent de docs met a jour Retours\LISEZ-MOI, l'index de Retours et EtapeDefini « dans main » dans le meme commit que la fusion ; gate Close WARN si EtapeDefini dit « pas dans main » pour une branche ancetre de main.
