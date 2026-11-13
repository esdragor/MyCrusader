# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Dernier bilan : 2026-11-13 matin et reprise (séance interactive 10:16-14:05)

- Main = ecff29a : fusion groupée 481bf43 (fog + maps + net-playtest + visual + d751-root + galadriel-tab ; build merge PASS, tests 1221/1221, net PASS) puis docs, D792 au GDD (c17f8b1), leçons L13-1 à L13-8 (ecff29a) ; rien poussé.
- Fusionné : D788 infobulle de revenu et A4 canaux (Glorfindel), D791 + HUD de la rage + filtre d'infobulle de la Caserne (Galadriel), D783-D786 et bornes 30/45/60 s (Haldir), N7 minicarte (Arwen), `build.ps1 -MaxWaitMin` et eol_check (Faramir), styles des ressources (Sauron).
- NON fusionnées : `tiers` (Elrond : correctif G6 f `ToggleDeploy` 27d91a4, ni recompilé ni testé après le merge de main ; scénario `deploy` attend `D=Pack` jamais produit) ; `hero-j8-close` (Celeborn : D787 prise de lance B e0cb438 + rebake a11e355, pose à juger à l'œil).
- Essais : `resources_compare` (arbre C confirmé) et `legions_roster_gesture` (OK) joués par l'utilisateur, fiches archivées ; 4 LOGIQUE SEULEMENT : `neutrals_minimap`, `spear_arm`, `deploy`, `legions_warrior`.
- Incidents : 403 Cloudflare de l'API à 11:12 (6 agents sur 8 coupés, arrêt à 11:20, reprise 11:41 à 3-5 agents) ; boucle git push / hook `reference-transaction` 10:17-10:46 (autre session) ; builds en file 16 à 40 min.
- Suite : Elrond (recompiler, tests,net, `deploy.txt`), Celeborn (pose B), Galadriel (LaunchCheck par module, `legions_warrior`), Arwen (carré du camp), BUG-melee-stuck-attack, SP2 / SG2 / `bSitesLaidByMaps` ; détail dans l'archive du matin.

## Dernier bilan : 2026-11-12 (séance autonome 10:26-13:00, 26 agents, détail dans l'archive du 2026-11-13)

- Main = 6d46a74 : fusion groupée 4851635 (maps + fog + hero-j8-close, sans C++, tests 1200/1200) puis commits de docs ; rien poussé.
- Dans main : 4 chapitres du GDD en règles pures, sans appelant en jeu (Centres secondaires D384, Signaux, choix de spécialisation de palier, option Nomade) ; LaunchCheck compare au fichier C++ le plus récent ; eol_check au gate Close ; cause du bras du Lancier trouvée (A_Spearman_Attack, test rouge arm_body_check.py), pas corrigée.
- Écrit mais NON COMPILÉ (branches pas prêtes) : Arwen d751-root WIP 308419b (N7 point de camp sur la minicarte), Elrond tiers WIP cc3213e (G6 e, G4 filtre, scénario deploy jamais lancé), Galadriel galadriel-tab WIP 3224cbd (rage 40 %, hol.Rage.Visual 4), Glorfindel net-playtest WIP 6703b9f (infobulle de revenu D780, GoldLine retirée), Sauron visual 6c7292f (styles des ressources figés).
- Cause : machine saturée par un autre projet (1,2 Go de RAM libre à 11:52) ; aucun agent C++ n'a compilé après 10:56 ; binaires périmés partout sauf Haldir, copie principale comprise.
- Réponses de l'utilisateur du 2026-11-12 vers 13:00 : les 8 Q-* tranchées (D783-D791 au GDD, 36c202e), reportées dans les journaux et la file (1f2ad3b), fiches archivées dans `Docs\Archive\Retours_2026-11-12\`.
- Défauts codés à changer après ces réponses : D783 (150 bois + 400 pierre), D784 Q2 A, D785 Q1 C, D786 Q1 B, D787 prise B (lance à reposer), D788 C (taux corrigé des trajets), D791 (seul le bonus monte à 40 %, Galadriel monte les deux) : lignes `[ ]` de `Auto_Journey_File.md`.
- Attend l'utilisateur : 0 question, 0 essai jouable (7 fiches E-* en `[NE PAS JOUER]`, binaire périmé).
- Leçons L12-1 à L12-7 : toutes dans `Auto_Journey_Checklist.md` ; non appliquées le 2026-11-12 : plafond de 3 agents C++, arrêt dur (5 agents bloqués dans build.ps1 -Wait), sync de début en -NoBuild.
- REPRISE : première tâche de chaque worktree = `Tools\build.ps1` machine calme, puis quick_check, avant tout nouveau code (lignes PREMIER GESTE en tête de file).

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

## Agents du 2026-11-13 (soir)

Séance AUTONOME 16:10 -> 21:00 (arrêt dur 21:00, +15 min toléré), fusion groupée dans main autorisée, GDD.md D792 déjà commité (ecff29a), Content/Audio/Candidates/ exclu via .git/info/exclude, jamais de push.

Etape 0 (PLAN-1113s, Fable, a11a7e15776d114a2, 16:15-16:22) : machine saturee (commit libre 1,9-3,3 Go, CPU 90-100 %, 4 editeurs ProjectShiva), 7 binaires sur 8 perimes (seul Faramir frais) ; un seul build a la fois (ordre Elrond, Galadriel, Glorfindel, Arwen, Haldir) ; integrateur = Faramir (fusion groupee a 20:00 : tiers + hero-j8-close + branches du soir, un build -Priority merge, un quick_check tests,net,build,aiserver,aiduel). Gate Launch 16:22 : 1 FAIL « 5 agents C++ » = faux positif connu (texte d'IN_USE.txt, L13-2), 3 agents C++ reels (Elrond, Galadriel, Glorfindel). Exception a la procedure : lancement en 2 vagues (5 agents a 16:25, puis Arwen, Haldir, Sauron a ~17:00) pour limiter les erreurs API 403 vues le matin avec 8 agents.

| Nom | Id | Worktree | Branche | Tache | Lance |
|---|---|---|---|---|---|
| NETTOYAGE-1113e a-d | a94b059a3d63c1af4, abf9e92df44779fc8, acb6eae43808c159e, aba99f0b76402ae7a | principale | main | 0a : 3018bfa, a0bf64a, bbb811d, c7431f5 | 16:10 |
| ELROND-1113d | a61f2c7a4cce2e030 | Elrond | tiers | merge main, build (C++ 1), quick_check tests,net, deploy rouge/preflight | 16:25 |
| GALADRIEL-1113c | aa635b7ecdaf26550 | Galadriel | galadriel-tab | L48 market libelle (rouge), L47 icones talents ; build rang 2 | 16:25 |
| GLORFINDEL-1113c | a68f76eb35005a0d9 | Glorfindel | net-playtest | L54 D788 scenario+.look puis build rang 3 ; L58 S6/S7 | 16:25 |
| FARAMIR-1113c (integrateur) | afde029828d5ff5bb | Faramir | fog | previols talents, fog_orders, map_eldorado, legions_warrior ; L46 build hud ; sans C++ | 16:25 |
| (Glorfindel-1113c fini 16:32, 45 appels) | a68f76eb35005a0d9 | Glorfindel | net-playtest | e10be35 scenario income_tooltip + HoverIncome C++ NON COMPILE ; cle IncomeTip manquante des .po (JA « /min ») | 16:25 |
| GLORFINDEL-1113d | a7568d9edb3f525c0 | Glorfindel | net-playtest | REPRISE : build rang 3, GatherText/fill_po, preview income_tooltip, E-income_tooltip, L58 | 16:35 |
| (Faramir-1113c fini 16:39, 45 appels) | afde029828d5ff5bb | Faramir | fog | previols PASS + PNG vus : talents, fog_orders, map_eldorado, legions_warrior (scenario corrige a633b97) ; build hud vert, known_reds 85f9997 ; 4 fiches E-* commitees 27c5547 ; fog = main + 3 commits | 16:25 |
| ARWEN-1113c | a454d725c49c941ad | Arwen | d751-root | carre du camp lisible (rouge), preview neutrals_minimap, audio stone/metal ; build rang 4 si besoin | 16:42 |
| HALDIR-1113c | a0e978cbf5f63cb71 | Haldir | maps | Q Nomade/specialisation/signaux, B4 foret (regle pure), SG2 si memoire | 16:42 |
| SAURON-1113c | aacd6893782d30441 | Sauron | visual | Arquebusier : planche 10 echantillons hors jeu, resources_game ; sans C++ | 16:42 |
| FARAMIR-1113d | addf39bf39b7d793b | Faramir | fog | outils : plafond C++ du gate (L13-2), L87-91 ; sans C++ | 16:42 |
| (Elrond-1113d fini 16:52, 46 appels) | a61f2c7a4cce2e030 | Elrond | tiers | build PASS 16:43, tests 1224/1224, net 17/17 ; 3762f1a : cause du rouge deploy = HUD (grille d'ordres pas redessinee apres montage), corrige, non recompile ; nouveau rouge : engin monte ne tire pas sur un batiment ; deploy LOGIQUE SEULEMENT | 16:25 |
| ELROND-1113e | a8e807bfa46907372 | Elrond | tiers | REPRISE : diagnostic engin monte qui ne tire pas, build, preflight deploy, E-deploy | 16:55 |
| (Sauron-1113c fini 16:54, ~43 appels) | aacd6893782d30441 | Sauron | visual | 3812599 planche arquebus_sheet.png 10 candidats A-J hors jeu, Q-arquebusier-modele [A REPONDRE] ; resources_game laisse non suivi (echec jamais diagnostique) ; sans C++ | 16:42 |
| CELEBORN-1113f | a3eb8560498eaea10 | Celeborn | hero-j8-close | BUG-melee-stuck-attack : diagnostic, test rouge, C++ seulement si cause + memoire >= 8 Go avant 19:00 | 16:57 |
| (Galadriel-1113c fini 17:04, ~90 appels) | aa635b7ecdaf26550 | Galadriel | galadriel-tab | f13e060/b372781/1671613 D781 Marche code, NON COMPILE (build 16:43 encore en cours) | 16:25 |
| (Faramir-1113d fini 17:03, 47 appels) | addf39bf39b7d793b | Faramir | fog | b9e6c03 rouge, 97a0622 correctifs gate L13-2/L87/L88-90, 97239e4 ; gate_test jamais vert (run tue par stray_processes) ; L91 non faite | 16:42 |
| GALADRIEL-1113d | adaeceb61028fc194 | Galadriel | galadriel-tab | REPRISE : attendre/relancer le build, tests, build:market, preview market, puis L47 | 17:10 |
| FARAMIR-1113e | a66a0ffa7a0075793 | Faramir | fog | gate_test jusqu'au vert (17 min, arriere-plan), L91 minimal ; sans C++ | 17:10 |
| SAURON-1113d | ae90b161230d966d5 | Sauron | visual | GIF animes de l'Arquebusier (demande de l'utilisateur), sans C++ | 17:15 |
| (Celeborn-1113f fini 17:17, ~52 appels) | a3eb8560498eaea10 | Celeborn | hero-j8-close | BUG-melee-stuck-attack : gel NON reproduit (12 passes, A/B SlotDetour et FarPlaceRetarget hors de cause), deja valide par l'utilisateur le 2026-11-10 (D769, 2b5b556) -> a clore ; detecteur melee_contact_idle.py 382e2ce ; limite maa_attack 5 cm 132b234 ; GIF lancier_attaque_A_B_angles.gif ; binaire copie de Faramir (non recompile) | 16:57 |
| (Galadriel-1113d fini 17:27, 43 appels) | adaeceb61028fc194 | Galadriel | galadriel-tab | build PASS 17:20 ; tests 1222/1222, build:market PASS, preview market PASS 3 PNG vus -> E-market [A ESSAYER] VU A L'ECRAN (GESTE interne) ; 7d14e07 prete (main merge) ; L47 : 24 icones dessinees hors jeu (make_talent_icons.py), Q-talents-icons = A TRANCHER, pas dans le jeu | 17:10 |
| (Celeborn-1113e fini 16:42, 43 appels) | a93e8f0ebdcb9eb5e | Celeborn | hero-j8-close | merge main bf152d5, planche pose B (Saved\c1113e\planche_pose_B.png, gitignore), bras gauche « leve » = effet de camera (preuve), arm_body_check spear VERT / maa_attack ROUGE 4,3 cm ; Q-spear-grip-b-look [A REPONDRE] ; BUG-melee-stuck-attack analyse non commencee (a relancer quand le nombre d'agents baisse) | 16:25 |
