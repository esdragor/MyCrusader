# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Agents du 2026-11-13

Séance INTERACTIVE 10:16 -> 14:15 (arrêt dur des agents 14:15, +15 min toléré), fusion groupée dans main autorisée, jamais de push. Départ : main = 6d46a74.

| Nom | Id | Worktree | Branche | Tache | Lance |
|---|---|---|---|---|---|
| NETTOYAGE-1113a | a8ad10aa6e4d8cd16 | principale | main | 0a : Jour + Retours (6c9c0f9) | 10:17 |
| NETTOYAGE-1113b | ad993f292d1c4fb12 | principale | main | 0a : File de travail (870f3f8) | 10:17 |
| NETTOYAGE-1113c | a850b77d806a71b21 | principale | main | 0a : dossier Retours (a6f6705) | 10:17 |
| NETTOYAGE-1113d | a672c7ec5ddc9f370 | principale | main | 0a : procédure + checklist (2503fed) | 10:17 |
| PLAN-1113a (Fable) | abf950ecd678b5fcb | principale | main | Étape 0 : plan de lancement (lecture seule) | 10:26 |

## Programme du 2026-11-13

Étape 0 (PLAN-1113a, relevé de 10:24 à 10:30 : `status.ps1`, git, dates des dll, `Docs\BuildQueue.csv`, charge machine). Rien n'a été lancé ni compilé par l'étape 0.

- **ALERTE machine, à régler AVANT tout lancement (10:28)** : un `git push` (pid 26788, lancé à 10:17:23 par une session Claude Code, `rtk git push`, dépôt `esdragor/MyCrusader`) n'a pas fini : le hook `.git\hooks\reference-transaction` (hook LFS d'auto-déverrouillage, daté du 2025-08-27) lance `git remote set-head --auto origin`, qui relance le hook, sans fin. 1173 processus `git` / `bash` à 10:28 (environ 110 de plus par minute), mémoire engagée 88,0 Go sur 88,7 Go (le fichier d'échange grossit), 17 Go de RAM physique libre seulement en apparence. Un build ou un éditeur lancé dans cet état échouera par manque de mémoire. L'étape 0 n'a rien tué (lecture seule). Autres charges : StarRail (12 Go engagés), l'éditeur de ProjectShiva (7 Go), ProjectRED-Win64-Shipping (5 Go) ; aucun `cl.exe` ; file de build vide.
- **Binaires, état réel (dll comparée au fichier `Source` le plus récent, 2 modules)** : frais dans Arwen, Celeborn, Faramir, Glorfindel, Haldir, Sauron (builds PASS finis après la séance du 2026-11-12, entre 13:29 et 14:05, `Docs\BuildQueue.csv`) ; PÉRIMÉS dans Elrond (dernier build FAIL à 14:06, `RoundTrip` introuvable dans `HOLNetTests.cpp`, 0072553), Galadriel (module HeroesOfLiberty seul : `HOLRageVisual.cpp` 12:10 contre dll 11:41) et la copie principale (2 modules). Aucun `quick_check` n'a tourné sur ces binaires.
- **File de build, ordre** : 1 Elrond (corriger `RoundTrip` puis `build.ps1 -LiveCoding=0 -Wait`), 2 Galadriel (un seul .cpp), 3 la place libre va à Glorfindel dès son `quick_check` rendu (code vert de D788). Arwen et Sauron ne compilent pas au départ. La copie principale se recompile après la fusion (essais rejoués depuis Faramir sinon).
- **Fusion groupée** : dans Faramir à 13:20 (branches commitées vertes et à jour de main à 13:10 au plus tard) : `fog`, `maps`, `hero-j8-close`, `d751-root`, `visual`, `galadriel-tab`, puis `tiers` et `net-playtest` seulement si vertes ; conflits attendus : verbe `notereset` (garder celui de fog), `HOLPlayerState.cpp/.h` (Arwen et Glorfindel) ; UN build `-Priority merge`, UN `quick_check -Parts tests,net`, avance rapide de main vers 13:50, jamais de push.
- **Essais visés (VU À L'ÉCRAN, 3 min, pré-vol PASS)** : dès ce matin sans attendre la fusion : `audio_compare_buildings_stone` et `_metal` (Arwen), `legions_roster_gesture` et `disconnect_1v1` (pré-vol dans Faramir, binaire frais = contenu de main), `spear_arm` (Celeborn, D787), `neutrals_minimap` (Arwen), `resources` (Sauron) ; après build : `legions_warrior` (Galadriel) ; incertain : `deploy` (Elrond). Rejeu sur main fusionné de 13:50 à 14:10.
- **Lignes périmées de `Auto_Journey_File.md`** : PREMIER GESTE « build » d'Arwen (l. 20), Celeborn (l. 25), Faramir (l. 41), Glorfindel (l. 55), Sauron (l. 85) : le build est fait, reste LaunchCheck + `quick_check` ; « NON COMPILÉ » des l. 21, 22, 57, 87 : compilé, non testé ; Celeborn et Haldir n'ont aucun commit devant main ; note « NON COMMITÉ à garder » de l'`IN_USE.txt` de Galadriel : commité (3224cbd). Toujours vraies : l. 36 Elrond, l. 45 Galadriel, l. 47 (a3e90a8 pas dans main), l. 86 (f0670cf pas dans main).

| Worktree | Branche | Tâche | C++ | Suivante |
|---|---|---|---|---|
| Arwen | `d751-root` (+4 commits, 8 fichiers Source, dll fraîche) | l. 20-22 : LaunchCheck, `quick_check -Parts tests,net`, commit vert de N7 (D789 : rien au survol) ; premier rouge = 3955b05 / 42cc580 doivent passer ; puis pré-vol `neutrals_minimap` ; `NeutralsInProgress.md` | non au départ | l. 14 du haut de file : pré-vol `audio_compare_buildings_stone` / `_metal` (candidats non commités à garder), fiches en `[À ESSAYER]` |
| Celeborn | `hero-j8-close` (= main, dll fraîche, 18 scénarios de repro non suivis à garder) | l. 26, D787 : prise de lance B dans `Tools\Editor\soldier_poses.py`, rebake, rouge 966ae02 (`arm_body_check.py`), scénario `spear_arm` ; `NeutralsInProgress.md` | non | l. 28 `maa_attack` à l'œil ; puis l. 29 BUG-melee-stuck-attack (C++, quand une place se libère) |
| Elrond | `tiers` (+6, 8 Source, dll PÉRIMÉE, build FAIL) | l. 36-37 : corriger `RoundTrip` dans `HOLNetTests.cpp`, build (file n° 1), LaunchCheck, `quick_check -Parts tests,net`, commit vert de G6 e + G4 ; rouge = e13e1f7 ; `Palier3InProgress.md` | oui (n° 1) | G6 f : pré-vol du scénario `deploy` (jamais lancé), fiche d'essai |
| Faramir | `fog` (+2 docs, dll fraîche) | pré-vols sur binaire frais des fiches `[NE PAS JOUER]` (`legions_roster_gesture`, `disconnect_1v1`), puis outils sans C++ : l. 101 (L12-1c, mémoire virtuelle < 8 Go, rouge dans `gate_test.ps1`), l. 98 (une dll par module, rouge dans `launch_check_test.ps1`), l. 103, 99, 100 ; `ScenariosInProgress.md` | non | arrêt à 13:10 ; hôte de la fusion groupée à 13:20, puis rejeu des essais sur main fusionné |
| Galadriel | `galadriel-tab` (+3, 5 Source, dll PÉRIMÉE) | l. 46, D791 d'abord (script : malus à 30, bonus à 40, test rouge sur le malus), puis l. 45 : build (file n° 2), LaunchCheck, `quick_check -Parts tests`, commit vert (rouge 236e56b) ; `LegionsInProgress.md` | oui (n° 2) | l. 47 pré-vol `legions_warrior` (Rage.Visual 4) ; l. 50 REGLAGE market ; l. 48 infobulle de la Caserne |
| Glorfindel | `net-playtest` (+4, 7 Source, dll fraîche) | l. 55 : LaunchCheck, `quick_check -Parts tests,net` (seul rouge attendu : `Gather.Income`), puis l. 56, D788 : test exigé à environ +350/min et code vert dans `UHOLGatherProcessor::CountGatherer` ; `SacredSitesInProgress.md` | oui (n° 3, après son quick_check) | commit vert, scénario de l'infobulle ; puis A4 Canaux de discussion (règle pure `HOLChatRules.as`) |
| Haldir | `maps` (= main, dll fraîche, `Content/Maps/Crown.umap` non suivi à ne pas toucher) | l. 65-68, D783-D786 : 4 réglages de script, test rouge d'abord pour chacun, `quick_check -Parts tests` ; journaux `SecondaryCentersInProgress.md`, `SignalsInProgress.md`, `SpecializationInProgress.md`, `NomadInProgress.md` | non | l. 80 tests à durcir (30/45/60 s) ; puis l. 71 `bSitesLaidByMaps` ou l. 72 SG2 (C++, quand une place se libère) |
| Sauron | `visual` (+5, 2 Source, dll fraîche) | l. 85-87 : LaunchCheck, `quick_check -Parts tests` (rouge a6e673f doit passer), pré-vol et PNG de `resources_compare`, commit vert, ligne de `Placeholders.txt` à retirer ; `VisualInProgress.md` | non au départ | l. 92 `neutrals` dans une clairière ; l. 90 lignes `[?]` de `Placeholders.txt` |
| Outils/docs | copie principale, `main` (docs seulement, commits par nom de fichier) | index de `Retours\` et `ESSAIS-A-JOUER.md` au fil des fiches `[À ESSAYER]`, l. 82 (`GDD.md` l. 3024, vieux « Prochain numéro D214 »), lignes périmées ci-dessus à annoter dans la file, `EtapeDefini.md` | non | l. 106 (L12-6) : docs « dans main » dans le même commit que la fusion |

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
