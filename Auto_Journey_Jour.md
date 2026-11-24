# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11), `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_soir_archive_2026-11-14.md` (séance autonome du 2026-11-13 soir : bilan, leçons, tableau d'agents ; résumés du 2026-11-13 matin et du 2026-11-12) et `Docs\Archive\Auto_Journey_Jour_2026-11-14_archive_2026-11-15.md` (séance autonome du 2026-11-14 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-15_archive_2026-11-16.md` (séance autonome du 2026-11-15 : tableau d'agents, fusions n°1-7, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-16_apres-midi_archive_2026-11-16.md` (séance interactive du 2026-11-16 après-midi : tableau d'agents, lignes d'exception, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_matin_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 matin : tableau d'agents, bilan, leçons, plan de lancement ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_apres-midi_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 après-midi : tableau d'agents, plan de lancement, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_soir_archive_2026-11-21.md` (séance autonome du 2026-11-20 soir, passée minuit : tableaux d'agents du 2026-11-20 et du 2026-11-21, bilan, leçons, exceptions de gate ; résumé dans ce fichier)  et `Docs\Archive\Auto_Journey_Jour_2026-11-21_matin_archive_2026-11-21.md` (séance autonome du 2026-11-21 matin 10:40 -> 14:40 : tableau d'agents, bilan, leçons, exceptions de gate ; résumé dans ce fichier)  et `Docs\Archive\Auto_Journey_Jour_2026-11-21_soir_archive_2026-11-22.md` (séance autonome du 2026-11-21 soir 19:00 -> 23:00 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-22_archive_2026-11-23.md` (séance autonome du 2026-11-22 : tableau d'agents, bilan, leçons ; résumé dans ce fichier ; y sont aussi les résumés anciens du 2026-11-15 au 2026-11-20 après-midi, sortis le 2026-11-23) et `Docs\Archive\Auto_Journey_Jour_2026-11-23_matin_archive_2026-11-23.md` (séance autonome du 2026-11-23 matin 08:47 -> 12:00 : tableau d'agents, clôtures, anomalies, bilan, leçons, plan de lancement ; y sont aussi les résumés des 2026-11-20 soir, 2026-11-21 matin et 2026-11-21 soir, sortis le 2026-11-23 ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-23_apres-midi_archive_2026-11-24.md` (séance autonome du 2026-11-23 après-midi 14:47 -> 19:00 : tableau d'agents, bilan, leçons, plan de lancement ; y est aussi le résumé du 2026-11-22, sorti le 2026-11-24 ; résumé dans ce fichier) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Chapitres du GDD non commencés

**Relevé du 2026-11-15** : voir Docs\ReleveGDD_2026-11-15.md (état réel de A1-A4 et B1-B6, branchements C++ à faire, questions en attente). L'ancien relevé du 2026-11-12 est archivé dans Docs\Archive\Auto_Journey_Jour_Chapitres_GDD_releve_2026-11-12_archive_2026-11-15.md.

## Agents du 2026-11-24

fin et mode demandés à 10:38 (fin 14:00, autonome ; priorité Docs\Priorite_Socle_Factions_2026-11-16.md)
check-list lue (L26) à 10:50 après le nettoyage 0a (commits 5d97eb3, 173d31b, 8da775a) ; veilleur 30 min et alarme 13:45 posés
Plan de lancement (ETAPE0-1124, 10:5x ; énoncés : `Docs\Rapports\ENONCES-1124.md` ; sync_worktrees tournait à 10:43 : lancer Glorfindel et Haldir seulement après `Docs\Rapports\SYNC-1124-debut.txt` complet ; 3 tâches C++ : E5, E6, E7) :
- Arwen | integ-37 | ARWEN-1124a : aperçu de visée de Lumière sacrée (REGLAGE 23/11) | sans C++ | ENONCES-1124 section E1
- Faramir | zone-effect | FARAMIR-1124a : D900 Lumière sacrée hors portée, le héros s'approche puis lance | sans C++ | section E2
- Celeborn | integ-41 | CELEBORN-1124a : D901 la ligne de palissade saute les cases bloquées | sans C++ | section E3
- Sauron | integ-63 | SAURON-1124a : PLAN grille fine D898 + D904 (docs seules, aucun code) | sans C++ | section E4
- Glorfindel | ability-columns | GLORFINDEL-1124a : D891 place du héros lue au pas de simulation (HOLHeroLibrary.cpp:402) | C++ (liaison) | section E5
- Elrond | integ-25 | ELROND-1124a : D905 libellés du bouton de spécialisation (HOLGameNames.cpp) | C++ (table de textes) | section E6
- Galadriel | socle-ai | GALADRIEL-1124a : S2-a restes (ordre Morale avant Combat, Rage nécrotique) | C++ (processeur Mass) | section E7
- Haldir | integ-58 | HALDIR-1124i : intégrateur de recette (pré-vol tower_wood et specialization_slot sur main, puis passes integrate.ps1) | sans C++ | section E8

| Agent | id | Worktree | Tâche | C++ | Lancé | Fini | Résumé |
|---|---|---|---|---|---|---|---|
| NETTOYAGE-A | 0a | copie principale | nettoyage Auto_Journey_Retours.md (en-tête recalculé) | non | ~10:3x | 10:4x | en-tête Retours au 2026-11-24 : 0 [À RÉPONDRE], main = 2ae5360 docs seulement, rien poussé depuis 0fef853 ; pas de rapport écrit |
| NETTOYAGE-B | 0a | copie principale | nettoyage Auto_Journey_File.md | non | ~10:3x | 10:42 | 173d31b : 18 lignes archivées telles quelles dans Docs\Archive\Auto_Journey_File_faits_2026-11-24.md (14 [x], D890 faite, 3 notes S4-a périmées) ; pas de rapport écrit |
| NETTOYAGE-C | 0a | copie principale | nettoyage Auto_Journey_Jour.md (journée du 2026-11-23 après-midi archivée) | non | ~10:3x | 10:41 | 5d97eb3 : Jour 245 lignes retirées vers l'archive, section « Agents du 2026-11-24 » créée, 8 fichiers ; pas de rapport écrit |
| NETTOYAGE-D | 0a | copie principale | nettoyage procédure Auto_Journey.md + Auto_Journey_Checklist.md | non | ~10:3x | 10:41 | 8da775a : copie complète archivée (450 l.), 9 lignes redondantes de la check-list retirées ; pas de rapport écrit |
| ETAPE0-1124 | ETAPE0 | copie principale | plan de lancement : 8 worktrees, 3 tâches C++ (E5, E6, E7), énoncés ENONCES-1124.md | non | 10:43 | 10:47 | 9470d27 : plan + énoncés E1-E8 ; check-list L26 lue 10:50, gate Launch posé par agent (GATE-1124-launch-*.txt) ; pas de rapport ETAPE0-1124.md |
| ARWEN-1124a | 1124a | Arwen (integ-37) | E1 aperçu de visée de Lumière sacrée (REGLAGE 23/11) | non | 10:50 | 10:54 | 3 commits (ceacd08 rouge, 0beca84 correctif) ; NON COMPILÉ ; rouge non vu à l'exécution |
| FARAMIR-1124a | 1124a | Faramir (zone-effect) | E2 D900 Lumière sacrée hors portée : le héros s'approche puis lance | non | 10:50 | 10:54 | 3 commits (bf056c6 rouge, e40e40c règle) ; règle de marche seule, branchement TryCastPower attend une liaison C++ ; NON COMPILÉ |
| CELEBORN-1124a | 1124a | Celeborn (integ-41) | E3 D901 la ligne de palissade saute les cases bloquées | non | 10:50 | 10:54 | 5 commits (c285b65 rouge, 75025ee règle) ; NON COMPILÉ ; 3 tests script ajoutés |
| SAURON-1124a | 1124a | Sauron (integ-63) | E4 PLAN grille fine D898 + D904 (docs seules) | non | 10:50 | 10:55 | 32e48fa : plan en 10 étapes G1-G10 (PLAN-GRILLE-FINE-1124.md), Q-grille-fine-structure.md posée ; aucun code, NON COMPILÉ |
| GLORFINDEL-1124a | 1124a | Glorfindel (ability-columns) | E5 D891 place du héros lue au pas de simulation (HOLHeroLibrary.cpp:402) | oui | 10:50 | ~11:10 | 2 commits (3dda65b rouge, abe5629 correctif) ; build PASS 726 s (BUILD LENT, fini 11:10) ; tests non lancés, rouge non vu |
| ELROND-1124a | 1124a | Elrond (integ-25) | E6 D905 libellés du bouton de spécialisation (HOLSpecializationSlot.as, fill_po.py) | non | 10:50 | 10:55 | 2 commits (b9eb35f rouge, bb610cc correctif) ; écrit en script malgré le C++ prévu ; NON COMPILÉ ; 27 appels au lieu de 25 ; scénario specialization_slot à mettre à jour |
| GALADRIEL-1124a | 1124a | Galadriel (socle-ai) | E7 S2-a ordre Morale avant Combat + ResetHonor | oui | 10:50 | ~10:58 | 4 commits (956f741 rouge, 63c4562 ini, 8c2bbf3) ; build PASS 308 s à 10:58 ; tests non lancés, rouge non vu |
| HALDIR-1124i | 1124i | Haldir (integ-58) | E8 pré-vol tower_wood et specialization_slot sur main | non | 10:50 | 10:55 | 0 commit (docs de la principale) ; preflight tower_wood PASS (9 PNG) et specialization_slot PASS (3 PNG), GESTE interne ; RECETTE-1124a.md non écrit |
| ARWEN-1124b | 1124b | Arwen (integ-37) | rejeu faction_aim_free avec l'aperçu de visée | non | 10:56 | 11:03 | c8b0ec8 ; preflight PASS (56 s), 4 PNG vus, GESTE interne ; sans build |
| FARAMIR-1124b | 1124b | Faramir (zone-effect) | S2-c Lumière sacrée appelle ClearMalusAround | non | 10:58 | ~11:02 | 3 commits (8b6fd50 rouge, 04b5db0 correctif) ; NON COMPILÉ ; 8b6fd50 en conflit à l'integ-64 (voir anomalie 11:05) |
| CELEBORN-1124b | 1124b | Celeborn (integ-41) | D901 compile et essai wall_line_skip | non | 10:59 | 11:03 | 7644229 + merge main c318eef ; preflight wall_line_skip PASS (31 s), 2 PNG vus, VU À L'ÉCRAN ; 0 erreur Angelscript au chargement |
| ELROND-1124b | 1124b | Elrond (integ-25) | scénario specialization_slot aux libellés D905, puis D893 étape 1 (moitié script) | non | 10:59 | ~11:08 | 4 commits (f45b6e5 rouge, 186f8b7 correctif) ; NON COMPILÉ ; preflight specialization_slot PASS (66 s), 3 PNG vus |
| GALADRIEL-1124b | 1124b | Galadriel (socle-ai) | S2-a restes : Rage nécrotique dans le moteur de moral, étape 1 | oui | 11:00 | 11:0x | 2 commits (f04bb00 rouge, 5d58440 correctif) ; NON COMPILÉ au rapport (build lancé 11:03 en file derrière Glorfindel puis Sauron) ; tests non joués |
| ARWEN-1124c | 1124c | Arwen (integ-37) | faction_aim_free joué avec le geste RÉEL | non | 11:04 | 11:08 | f483c4b + merge main 1fbf4c0 ; preflight PASS (63 s), 3 PNG vus, GESTE réel, VU À L'ÉCRAN ; sans build |
| FARAMIR-1124c | 1124c | Faramir (zone-effect) | REVUE-1123pm gênant 4 : retrait du relais HOLMoraleRules.as | non | 11:03 | ~11:11 | ed8cbc9 (refactor, 6 fichiers de test migrés compilés au chargement, 0 erreur, non exécutés) ; pas de build ; merge main non fait |
| CELEBORN-1124c | 1124c | Celeborn (integ-41) | grille fine étape G1 (D898) | non | 11:05 | 11:08 | 2 commits (6a8b750 rouge, e9cf204 correctif) ; NON COMPILÉ ; test Test_WallLineFineGridLineIsSeventeenCells (17 cases) |
| CELEBORN-1124d | 1124d | Celeborn (integ-41) | grille fine étape G2 (D898), occupation en sous-cases | non | 11:10 | 11:15 | 2 commits (79d6817 rouge, 6f71391 correctif) ; NON COMPILÉ ; merge main fait ; test Test_WallLineFineGridOccupiedTileSkipsItsSubCells ; 0 erreur Angelscript au chargement |
| ANOMALIE | - | integ-64 | 11:05 integ-64 conflit au cherry-pick de 8b6fd50 (2 rouges FARAMIR sur HOLFactionPowerServer.as) : passe relancée sans lui | - | 11:05 | - | integrate.ps1 : 6 rouges appliqués, INTEG-64.txt incomplet (tests des rouges) |
| ANOMALIE | - | Glorfindel | GLORFINDEL build 726 s BUILD LENT : au plus 2 agents C++ (L25-5) | - | 10:54 | 11:10 | build PASS mais 726 s > 3 x médiane 146 s (21 actions, vm 45,3 Go), file Galadriel, Sauron, Glorfindel |
| GLORFINDEL-1124b | 1124b | Glorfindel (ability-columns) | lot C++ GetHeroUnitId + GetChosenOptionForSlot (liaisons script) | oui | 11:12 | 11:22 | 11f94f4 rouge, 0214001 correctif (dans main via integ-67) ; build lancé 11:17 ; rouge non vu |
| ARWEN-1124d | 1124d | Arwen (integ-37) | sacred_light_heal : preuve que la zone rend des PV (reste RECETTE-1123pa b) | non | 11:10 | 11:19 | 604a7b2 (dans main) ; total soigné 0.00 -> 705.00, PV héros 180 -> 330 ; LOGIQUE SEULEMENT ; 19 appels |
| FARAMIR-1124d | 1124d | Faramir (zone-effect) | ai_sacred_light : ligne de file périmée | non | 11:11 | 11:17 | 0 commit (règle et lecteur déjà dans main, 0d81744 / 3974596) ; preflight PASS ; ligne de file cochée |
| ELROND-1124c | 1124c | Elrond (integ-25) | D905 traductions EN/JA : collecte des textes, HOL_CHECK_CULTURE | non | 11:08 | 11:20 | c54cbaf (check.ps1, dans main) ; 27 appels au lieu de 25 |
| GALADRIEL-1124c | 1124c | Galadriel (socle-ai) | Rage nécrotique étape 2 : cadence +25 % = levier Rate du moral | oui | 11:23 | 11:26 | a371040 rouge, 21facdc correctif (dans main) ; build lancé 11:25 ; rouge DamnedWarrior.Rage vu 2199/2200 à 11:27 |
| CELEBORN-1124e | 1124e | Celeborn (integ-41) | grille fine G3 (D898) : début et fin sous la souris | non | 11:16 | 11:22 | 9820aad rouge, 62eedb4 correctif (PieceBox) ; NON COMPILÉ ; 0 erreur AS au chargement |
| ELROND-1124d | 1124d | Elrond (integ-25) | D905 EN/JA : assets de localisation + vérification à l'écran | non | 11:28 | 11:33 | f426bd4 (10 fichiers .po/.locres, dans main) ; specialization_slot EN et JA preflight PASS, VU À L'ÉCRAN |
| DIAG-1124a | diag | Glorfindel (ability-columns) | integ-64 : Hero.GetHeroLocationStep rouge | non | 11:29 | ~11:37 | 5088e41 : défaut du harnais de test (alpha 1), pas du correctif ; NON COMPILÉ |
| GLORFINDEL-1124c | 1124c | Glorfindel (ability-columns) | F10 : origine du grand cercle pâle de faction_aim_free (lecture seule) | non | 11:28 | 11:33 | 0 commit ; origine identifiée, verdict voulu (anneau de portée) |
| HALDIR-1124j | 1124j | Haldir (integ-58) | F6 : session_gate Essay refuse une commande abrégée (AX-9) | non | 11:28 | 11:37 | 14df0c1 rouge (4 FAIL / 2 PASS), fda2fd6 correctif (dans main) ; 26 appels |
| PREP-1124b | prep | copie principale | énoncés ENONCES-1124b (10 tâches F1-F10 sans C++) | non | ~11:20 | 11:25 | 9814b86 : ENONCES-1124b.md + 11 lignes de file closes après contrôle contre main |
| ARWEN-1124e | 1124e | Arwen (integ-37) | F5 journal du moral du duel | non | 11:28 | 11:36 | e7c76e2 rouge VU 11:35 (2199/2200), ca50207 correctif (dans main) ; vert non relancé |
| FARAMIR-1124e | 1124e | Faramir (zone-effect) | F1 bannière plantée par l'IA de l'Aube | non | 11:28 | 11:36 | 6b567f1 (dans main) : bannière (14992, 28) à 200 cm des blessés, aucun défaut ; scénario ai_sacred_light durci |
| ELROND-1124e | 1124e | Elrond (integ-25) | F4 D877 moitié A : scénario deploy_far | non | 11:35 | 11:40 | 7c790ca + aac8127 (dans main) ; preflight PASS, VU À L'ÉCRAN, défaut NON reproduit ; E-deploy_far.md |
| GLORFINDEL-1124d | 1124d | Glorfindel (ability-columns) | F8 outil stale_quickcheck.ps1 | non | 11:37 | 11:42 | 12d8040 rouge (4 FAIL / 7), 0602b09 outil (dans main) ; 7 PASS ; pas encore branché dans session_gate |
| GALADRIEL-1124d | 1124d | Galadriel (socle-ai) | régression Combat.DamnedWarrior.Rage depuis l'étape 1 | non | 11:35 | ~11:42 | correctif du test (le jeu était juste) ; NON COMPILÉ au rapport (build 11:36 en cours) |
| ARWEN-1124f | 1124f | Arwen (integ-37) | F7 outil replace_text.ps1 | non | 11:38 | 11:41 | 5fd822d rouge (4 ok / 9 rouges sur 13), 8bb61f9 outil (dans main), 5a349b7 ; 13/13 |
| FARAMIR-1124f | 1124f | Faramir (zone-effect) | D894 colonne faction dans FPartHashes | oui | 11:38 | ~11:45 | aac9d9a rouge, ea2c420 + ef5f1ef ; BUILD FAIL 625 s (OpenWorld/CloseWorld hors portée, membres privés d'AHOLGameState) |
| HALDIR-1124k | 1124k | Haldir (integ-58) | gate_test.ps1 complet rejoué (fin de F6) | non | 11:38 | 12:01 | 0 commit ; 112 PASS / 3 FAIL (1287 s), les 3 FAIL anciens (modèle d'énoncé) ; arrêt dur 11:54 dépassé (21 min de test) |
| CELEBORN-1124f | 1124f | Celeborn (integ-41) | grille fine G5 : une pièce par sous-case (serveur) | non | 11:28 | 11:45 | c383070 rouge VU 11:36 (2204/2205), c1cae68 correctif (dans main) ; cvar hol.Wall.SubDivision défaut 1 |
| GLORFINDEL-1124e | 1124e | Glorfindel (ability-columns) | F3 / D900 : le héros s'approche puis lance (branchement TryCastPower) | non | 11:53 | ~12:08 | 66e0be0 rouge, c74f5bf correctif ; NON COMPILÉ ; Q-lumiere-sacree-approche.md ; branche seulement |
| ARWEN-1124g | 1124g | Arwen (integ-37) | F8/F9 outil build_wait.ps1 (L15-6) | non | 11:53 | 11:58 | ac4a336 rouge (25 / 28), 7ea4e01 outil (dans main) ; 28/28 ; build_wait.ps1 ensuite utilisé par HALDIR-1124m |
| ELROND-1124f | 1124f | Elrond (integ-25) | F9 gate Launch : DLL en retard de plus de 5 commits | non | 11:53 | 11:55 | 0 commit (tâche déjà faite) ; 4 appels |
| CELEBORN-1124g | 1124g | Celeborn (integ-41) | grille fine G4 (C++) : SnapFootprint par type, GetWallLineWorld au pas | oui | 11:48 | ~11:57 | aa1bbfe rouge, 405e578 ; BUILD FAIL au 1er essai (11:56, C2039 TypeName) corrigé par 20d4888 |
| GALADRIEL-1124e | 1124e | Galadriel (socle-ai) | Rage nécrotique étape 3 : site du décompte | oui | 11:58 | 12:06 | 8d7eee2 rouge (non vu), c02786d ; build PASS 163 s (12:09), tests 2209/2209 (12:13) ; branche seulement |
| CELEBORN-1124h | 1124h | Celeborn (integ-41) | G4 compilation corrigée, test grille fine | oui | 12:00 | 12:12 | d8d2939 (test seul) ; build PASS 39 s, tests 2206/2206 ; branche (dans integ-69) |
| FARAMIR-1124g | 1124g | Faramir (zone-effect) | correction de compilation de D894 (réflexion FindFProperty) | oui | 12:02 | 12:15 | ad0288f ; build PASS (5 s), quick_check tests 2187/2187 PASS (QC-FARAMIR-1124g) ; branche seulement |
| HALDIR-1124l | 1124l | Haldir (integ-58) | recette indépendante n°1 (RECETTE-1124a) sur main 9d40517 | non | 12:02 | 12:10 | wall_line_skip PASS et faction_aim_free PASS, 6 PNG vus, VU À L'ÉCRAN ; build_wait absent de main (build.ps1 92 s) |
| GLORFINDEL-1124f | 1124f | Glorfindel (ability-columns) | F3 / D900 : PendingCasts hors global (correctif integ-68) | non | 12:40 | ~12:45 | 849c057 (classe UHOLFactionPendingHolder) ; build en file derrière Sauron ; branche seulement |
| HALDIR-1124m | 1124m | Haldir (integ-58) | recette indépendante n°2 (RECETTE-1124b) sur main 25e29ab | non | 12:25 | 12:42 | 5 essais PASS : faction_aim_free, sacred_light_heal, deploy_far, specialization_slot (ja), ai_sacred_light ; build_wait.ps1 PASS 484 s ; 4 fiches E mises à jour |
| ANOMALIE | - | integ-66 | 12:00 integ-66 : build des rouges FAIL (470 s) | - | 12:00 | - | 5 rouges appliqués, build en échec : passe relancée sans eux (integ-67) |
| ANOMALIE | - | integ-68 | 12:39 integ-68 : tests verts 0/0 (Global variable 'PendingCasts' must be const, HOLFactionPowerServer.as:67) | - | 12:39 | 12:45 | F3 global Angelscript de GLORFINDEL-1124e ; corrigé par GLORFINDEL-1124f (849c057) |
| ANOMALIE | - | integ-69 | 12:57 integ-69 : tests 2203/2203 PASS, parts net,build FAIL (399 s) | - | 12:57 | - | net vide sous charge ; QC-NET-integ69.txt (net, netsig3) relancé sur machine calme, en vérification à 13:00 |

## Bilan du 2026-11-24

- Séance autonome 10:38 -> 14:00 (arrêt dur de DOCS-1124b 13:25), session principale + agents Sonnet 5.5 ; SIX passes d'`integrate.ps1` (integ-64 à 69) : integ-64 FAIL 11:28 (conflit de 8b6fd50 puis `Hero.GetHeroLocationStep` = défaut du harnais, DIAG-1124a), integ-65 PASS 11:50 -> main `9d40517` (tests 2208/2208, net et build PASS), integ-66 FAIL build des rouges 12:00, integ-67 PASS 12:23 -> main `25e29ab` (tests 2201/2201, net et build PASS), integ-68 FAIL 12:39 (tests 0/0 : global Angelscript `PendingCasts`), integ-69 en cours de vérification net (12:57 : build PASS, tests 2203/2203, parts net,build FAIL 399 s ; `QC-NET-integ69.txt` rejoue net et netsig3 machine calme, sequence PASS à 13:00). RIEN poussé (origin/main = 0fef853).
- Entre pour le joueur (dans main) : aperçu de visée de Lumière sacrée (cercle de débogage 6 m, vert à portée / orange hors portée, `faction_aim_free`) ; D901 la ligne de palissade saute les cases bloquées (trou rouge puis 7 fondations) ; D905 libellés du bouton de spécialisation en FR / EN / JA ; D893 étape 1 (`AnswerQueueProductionFor` refuse une unité de voie verrouillée) ; moral : ordre Morale avant Combat, `ResetHonor`, Rage nécrotique étapes 1 et 2 (dégâts +40 % puis cadence +25 % par le moteur de moral), Lumière sacrée qui retire le malus autour de sa zone (S2-c, 04b5db0), relais `HOLMoraleRules.as` retiré ; grille fine des murs G1 à G3 et G5 derrière `hol.Wall.SubDivision` (défaut 1 : rien ne change à l'écran) ; liaisons `GetHeroUnitId` / `GetChosenOptionForSlot` ; outils `build_wait.ps1`, `replace_text.ps1`, `stale_quickcheck.ps1`, gate Essay AX-9 (commande abrégée refusée).
- Reste sur branche : F3 D900 (le héros s'approche puis lance : 66e0be0, c74f5bf, 849c057 sur ability-columns, `Q-lumiere-sacree-approche.md`, crochet d'annulation C++ à faire) ; Rage nécrotique étape 3 (socle-ai, 8d7eee2 + c02786d, build PASS et tests 2209/2209 sur son worktree, rouge non vu, pas dans main ; non compilée au rapport de GALADRIEL-1124e) ; D894 colonne faction du hash de pas (zone-effect, aac9d9a à ad0288f, build PASS 5 s, tests 2187/2187 sur son worktree) ; grille fine G4 C++ (integ-41, d8d2939, dans integ-69) ; rouge 8b6fd50 (S2-c) jamais vu à l'exécution (le commit est dans main avec son correctif 04b5db0, entré par integ-65 après le conflit d'integ-64). Questions ouvertes : `Q-grille-fine-structure.md` (2 questions), `Q-lumiere-sacree-approche.md` (2 points), valeurs par défaut codées.
- Essais prêts (recette indépendante HALDIR-1124m, RECETTE-1124b, sur main 25e29ab, build_wait PASS 484 s, gate Essay PASS 12:43) : 5 PASS (faction_aim_free, sacred_light_heal, deploy_far, specialization_slot en ja, ai_sacred_light) ; RECETTE-1124a : wall_line_skip PASS sur main 9d40517 ; geste réel pour faction_aim_free, deploy_far, specialization_slot et wall_line_skip (la Maison est posée en interne) ; sacred_light_heal = LOGIQUE SEULEMENT (preuve de règle) ; tower_wood reste « prêt mais non donné » (geste interne, pré-vol sur 173d31b).
- Anomalies : 2 conflits de rouges (integ-64 : 8b6fd50 sur `HOLFactionPowerServer.as` ; un 2e, integ-66 build des rouges FAIL) ; build lent GLORFINDEL 726 s (> 3 x médiane 146 s, file Galadriel, Sauron, Glorfindel) et FARAMIR-1124f FAIL 625 s ; net vide sous charge (integ-69) ; F3 global Angelscript (`PendingCasts`, integ-68 tests 0/0, corrigé par 849c057, non intégré) ; JA : « Voie A du palier 1 » / « Voie B du palier 1 » restent en français dans l'UI japonaise (RECETTE-1124b), texte non localisé ; 6 agents C++ non compilés à leur rapport (FARAMIR-1124f/g, CELEBORN-1124g, GALADRIEL-1124d/e, GLORFINDEL-1124e) ; `session_gate -Phase Close` non lancé avant ce bilan.
- Détail : tableau « Agents du 2026-11-24 » ci-dessus ; rapports `Docs\Rapports\*-1124*.md`, `INTEG-64` à `INTEG-69`, `RECETTE-1124a` et `RECETTE-1124b`.

### Lecons appliquees 2026-11-24
- T2-1 : appliquee : ETAPE0-1124 (9470d27, Docs\Rapports\ENONCES-1124.md), plan de lancement écrit au Jour
- T2-2 : appliquee : 8 worktrees lancés d'emblée à 10:50 (plan de lancement du Jour)
- L3-12 : appliquee : étape 0 faite (ETAPE0-1124, 10:43 -> 10:47), non sautée
- L5-7 : non vérifié : aucune trace écrite de la question « dossiers non suivis » ; 13 fichiers non suivis (Docs\Rapports, Retours) à ce bilan
- L4-1, L4-3 : appliquee : fin 14:00 écrite (Jour l.13), alarme 13:45 et veilleur 30 min posés (Jour l.14)
- L3-8, L4-9, Q5, L1-14 : appliquee : intégrateur HALDIR-1124i nommé au plan, recettes RECETTE-1124a (12:02) et RECETTE-1124b (12:25)
- L3-9 : non vérifié : appels d'outils de la principale non relevés
- L1-1, L3-6, L4-7, L5-3 : appliquee : appels déclarés de 4 à 27 (ELROND-1124c 27, HALDIR-1124j 26), aucun au-delà de 45 vus ; `tool_uses` non relevés au bilan
- L5-6 : non vérifié : séance autonome, aucun tableau de questions groupées
- L3-11, L5-5 : non vérifié : aucune régénération de cartes du jour
- L3-7, L1-4 : appliquee : veilleur 30 min posé (Jour l.14)
- T2-4 : appliquee : 2 `Q-*.md` neuves (grille fine, approche de la Lumière sacrée), chantiers distincts, 2 questions au plus chacune
- Q5, L5-4 : appliquee : faction_aim_free, deploy_far, specialization_slot et wall_line_skip au geste réel, rejoués par HALDIR-1124l / 1124m sur main (gate Essay PASS 12:43)
- Q8 : non appliquee : « Voie A / B du palier 1 » non traduit en JA (RECETTE-1124b, 12:42) sans ligne [ ] de tête de file à l'heure de ce bilan
- Q11 : appliquee : 5 essais prêts (faction_aim_free, deploy_far, specialization_slot, wall_line_skip, tower_wood) + sacred_light_heal LOGIQUE SEULEMENT hors compte, au plus 6
- L1-3 : non appliquee : merge_main_fait: non chez ARWEN-1124d à g, CELEBORN-1124f/h, ELROND-1124c à f, HALDIR-1124j/k ; oui chez GLORFINDEL-1124e/f, GALADRIEL-1124c/e, FARAMIR-1124d/f, CELEBORN-1124g
- L6-2 : non vérifié : lecture de la file par la principale non relevée
- L6-4 : non vérifié : aucun bug de l'utilisateur à rejouer ce jour
- L6-6 : non vérifié : aucune mesure répétée du jour
- L7-7 : appliquee : première avance de main à 11:50 (integ-65, 9d40517), seconde à 12:24 (integ-67, 25e29ab)
- L8-4 : non vérifié : aucun essai sonore du jour (SON non entendu)
- L8-8 : non vérifié : lot C++ (G4, F3) dans integ-69 après les scripts, non chronométré contre la règle
- L9-1 : non vérifié : pas de relevé de re-notifications en boucle
- L9-6 : non vérifié : listes figées avant chaque passe non relevées dans ce bilan
- L9-8 : non vérifié : tâches des agents inactifs non relevées
- L9-9 : non vérifié : utilisateur absent de la séance
- L10-2 : appliquee : integ-69 net FAIL (399 s) -> net et netsig3 rejoués machine calme (QC-NET-integ69.txt) avant de toucher aux branches
- L10-3 : non vérifié : session_gate Report non relevé
- L11-3 : appliquee : ELROND-1124e (deploy_far, verdict sans C++ : défaut non reproduit) avant tout C++ ; FARAMIR-1124d et ELROND-1124f : tâche déjà dans main, 0 commit
- L11-4 : non vérifié : gate Close non lancé avant ce bilan
- L11-8 : non vérifié : listes figées non relevées
- L11-9 : appliquee : intégrateur de recette HALDIR (RECETTE-1124a / b) ; Retours\ESSAIS-A-JOUER.md (10:55) pas relu au bilan
- L11-10, L10-5, L9-7 : non vérifié : aucune mesure de mêlée du jour
- L11-11 : non vérifié : lecture de Retours\REPONSES non tracée
- L11-12 : non vérifié : purge de Retours\ non relevée
- L11-13 : non vérifié : heures des agents prises à Get-Date (rapports), pas de relevé pour la principale
- L12-1 : appliquee : SYNC-1124-debut.txt (10:42) avant les lancements Glorfindel et Haldir
- L12-1b : appliquee : BUILD LENT noté (726 s GLORFINDEL, 625 s FARAMIR-1124f, vm libre 45,3 Go)
- L13-1 : non vérifié : aucun 403 relevé
- L13-4 : appliquee : integ-69 net rejoué machine calme avant d'être noté REGRESSION (QC-NET-integ69.txt)
- L13-5 : non vérifié : dossiers non suivis de plus de 50 fichiers non relevés
- L13-6 : appliquee : objet nommé et vu sur chaque PNG de RECETTE-1124a / b (cercle vert / orange, Mangonneau monté, 7 fondations)
- L13-7 : non vérifié : aucun balayage du jour
- L14-2 : non appliquee : 3 tâches C++ au lancement (E5, E6, E7) puis 4 builds C++ en file à 11:40 (Galadriel, Faramir, Celeborn, Glorfindel)
- L14-4 : appliquee : 1 `AskUserQuestion` (fin et mode, 10:38), aucune réponse ambiguë comptée
- L14-5 : appliquee : faction_aim_free passé au geste réel (f483c4b), deploy_far, specialization_slot, wall_line_skip réels ; reste tower_wood et sacred_light_heal à l'écran seulement
- L14-6 : non appliquee : rouge non vu pour FARAMIR-1124b/f, CELEBORN-1124g, GLORFINDEL-1124a/b/e, GALADRIEL-1124e ; vu pour ARWEN-1124e/f/g, GLORFINDEL-1124d, CELEBORN-1124f
- L14-9 : non vérifié : EN et JA vus (ELROND-1124d, c54cbaf), capture 1920x1080 non relevée
- L14-10 : non vérifié : aucun essai audio du jour
- L14-11 : non appliquee : passes integ-64 à 69 de 899 à 1344 s, au-dessus de la durée cible de 7 à 15 min
- L15-4 : non vérifié : usage d'agents_table.ps1 non relevé (tableau complété par DOCS-1124a / b)
- L15-6 : appliquee : build_wait.ps1 écrit (ARWEN-1124g, 28/28, dans main 7ea4e01) et utilisé (HALDIR-1124m : PASS 484 s)
- L15-7 : appliquee : lignes de file périmées signalées au premier appel (FARAMIR-1124d, ELROND-1124f), PREP-1124b a clos 11 lignes (9814b86)
- L15-8 : appliquee : essais rejoués sur main 25e29ab par HALDIR-1124m (5 PASS) avant d'être donnés
- L15-9 : appliquee : aucun agent à 0 commit pour excès de lecture hors tâches de lecture seule (GLORFINDEL-1124c) ou déjà faites
- L16-1 : non vérifié : charge machine non mesurée au bilan hors BUILD LENT
- L16-2 : appliquee : gate Essay PASS 12:43 (binaire 12:33 >= commit de code 12:08) pour les 5 commandes play.ps1
- L16-3 : appliquee : chaque avance de main par integrate.ps1 (integ-65 et integ-67 PASS tests + net + build)
- L16-4 : non vérifié : langue des messages de la principale non relevée
- L16-5 : non vérifié : chiffres de la séance non recomptés par un outil avant ce bilan
- L16-6 : non vérifié : pas de build de rafraîchissement relevé
- L16-7 : non vérifié : pas de sortie tronquée relevée
- L16-8 : appliquee : gate Essay sur les .as / .ini en WARN, aucun FAIL « binaire » contourné (GATE-1124-essay-*.txt tous PASS)
- L17-a : appliquee : integ-64 `Hero.GetHeroLocationStep` rouge lu avant de conclure (DIAG-1124a : défaut du harnais, 5088e41)
- L17-b : non vérifié : aucun refus d'Edit relevé
- L17-c : non vérifié : aucun build orphelin relevé
- L18-a : non vérifié : aucune table générée modifiée par un agent
- L18-b : non vérifié : pas de relevé de nommage entre règles neuves
- L18-c : appliquee : dernier lancement de code 12:40 (GLORFINDEL-1124f), avant fin - 60 min (13:00)
- L18-d : non vérifié : aucune commande ajoutée à une liste enregistrée
- L18-e : appliquee : agents « règle pure + tests » en deux commits (CELEBORN-1124c à f, ARWEN-1124e/f/g)
- L18-f : non vérifié : pas d'échec d'essai attribué au scénario
- L19-1 : non vérifié : gate Close non lancé avant ce bilan, appels de la principale non relevés
- L19-4 : appliquee : stale_quickcheck.ps1 (GLORFINDEL-1124d, 0602b09, 7 PASS) et Grep de quick_check.ps1 (ARWEN-1124e)
- L19-5 : appliquee : fiches E dès le pré-vol PASS (E-sacred_light_heal 11:17, E-deploy_far 11:38), capture « pendant » (deploy_far_walking)
- L19-6 : appliquee : 0 `AskUserQuestion` de travail, 2 `Q-*.md` laissées dans Retours\ avec défaut codé
- L19-7 : appliquee : vm libre 45,3 Go relevée avant le lot C++ (BUILD LENT de GLORFINDEL)
- L20-2 : non appliquee : FARAMIR-1124f membres privés d'AHOLGameState et lambdas hors portée (BUILD FAIL 625 s), CELEBORN-1124g C2039 TypeName
- L20-4 : appliquee : recette RECETTE-1124a à 12:02 et RECETTE-1124b à 12:25, essais rejoués après leurs correctifs
- L20-5 : non appliquee : build incrémental avant rapport non fait par FARAMIR-1124f/g, CELEBORN-1124g, GALADRIEL-1124d/e (file de build pleine)
- L20-7 : non vérifié : rouges de la dernière vague non comptés séparément
- L20-8 : non vérifié : lectures par agent non comptées au bilan
- L21-1 : appliquee : 1 `AskUserQuestion` (lancement seulement, fin et mode à 10:38), 0 pour une question de travail
- L21-2 : non vérifié : liste des fichiers pris par agent non relevée
- L21-3 : appliquee : aucune ligne de table neuve sans `placeholders_map.csv` ; check_placeholders PASS (49 types, 51 entrées)
- L22-1 : non appliquee : GLORFINDEL-1124e a écrit un global Angelscript sans chargement du jeu : integ-68 tests 0/0 (12:39, `PendingCasts` must be const)
- L22-2 : non appliquee : 2 conflits de rouges (integ-64 8b6fd50 sur HOLFactionPowerServer.as, integ-66 build des rouges FAIL)
- L22-3 : appliquee : stale_quickcheck.ps1 posé (0602b09) ; ARWEN-1124e : aucune dépendance de quick_check à l'identifiant touché
- L22-4 : appliquee : tâches déjà dans main dites au premier appel (FARAMIR-1124d 11 appels, ELROND-1124f 4 appels)
- L22-5 : non vérifié : aucun pont C++ -> script neuf du jour
- L23-1 : appliquee : 1 `AskUserQuestion` seulement (lancement 10:38), 0 pendant la séance
- L24-3 : non appliquee : voir L20-5 : 6 rapports C++ NON COMPILÉ ; BUILD du rapport ≠ NON COMPILE seulement pour GALADRIEL-1124e (PASS 12:09), CELEBORN-1124h, FARAMIR-1124g
- L24-4 : non vérifié : ordre gate Launch / Agent et shas entre guillemets non relevés
- L24-5 : non vérifié : refus du classifieur dans ANOMALIES_PROCESSUS non relus au bilan
- L25-4 : non vérifié : TaskStop d'un agent en build non relevé
- L25-5 : non appliquee : 4 agents C++ avec builds de 625 et 726 s en file (Galadriel, Faramir, Celeborn, Glorfindel derrière Sauron)
- L25-6 : appliquee : fin et mode demandés à 10:38 (Jour l.13)
- L26-1 : non appliquee : valeur de /context de la principale non relevée à la clôture (objectif 300k non mesuré)

## Résumé de la séance du 2026-11-23 après-midi

- Séance autonome 14:47 -> 19:00 (priorité Docs\Priorite_Socle_Factions_2026-11-16.md), session principale + agents Sonnet 5.5 ; SIX passes d'`integrate.ps1` (integ-58 à 63, toutes PASS), 6 avances de main en local : b5a285b 15:37, c3578b1 16:01, 86a2e15 16:23, a984c61 16:46, 3b6d29c 17:07, b5299e5 17:28 (tests 2198/2198, net et build PASS) ; RIEN poussé depuis 12:24 (origin/main = 0fef853).
- Dans main pour le joueur : S1 lancer de Lumière sacrée sans bannière, zone au point visé à 1200 cm (D890, D897), Honneur au soin et à la mort ; S2 moral lu par le combat, Triomphe / Démoralisé / Hésitation, atténuation par camp ; S3 tour garnie 1 + occupants flèches (D899), Paysans construisent au relâché d'une ligne de palissade, F9 / F10 ; S4 l'IA plante sa bannière et lance à 1200 cm, ai_garrison ; hors lot : D893 (3 unités palier 1, déblocage NON BRANCHÉ), D895 suivi mutuel, SP3 bouton de spécialisation, integrate.ps1 + regen_tables.
- Essais : 6 donnés (faction_click_cast REGLAGE D897, ai_sacred_light OK, wall_line_long BUG + design D898 puis OK après correction, ai_garrison OK + doute D899, faction_aim_free OK sans aperçu de visée = REGLAGE en tête du socle) ; prêts mais NON donnés : tower_wood et specialization_slot ; LOGIQUE SEULEMENT : Honneur à la mort, tout le moral, D895, déblocage D893, bannière de l'IA.
- Questions : 6 [À RÉPONDRE] posées puis répondues 18:1x par l'interface de choix (D900 à D905), Q-commanderie-palier1 = A (D896) ; REVUE-1123pm : 0 bloquant, 5 gênants dont G1 à G3 corrigés, G4 et G5 en file ; anomalies : gate Launch FAIL 3 fois, 14 rapports C++ non compilés, budget de la principale à 646,5k (L26-1).
- Détail (tableau d'agents, bilan, tableau « Lecons appliquees 2026-11-23 », plan de lancement, résumé du 2026-11-22) : `Docs\Archive\Auto_Journey_Jour_2026-11-23_apres-midi_archive_2026-11-24.md`.

## Résumé de la séance du 2026-11-23 matin

- Séance autonome 08:47 -> ~12:00 (fin ramenée de 16:50 à 12:00 par l'utilisateur), session principale + agents Sonnet ; TROIS passes d'`integrate.ps1` : integ-54 -> main `47f6fe4` (10:01), integ-55 -> `3f7d89e` (10:52), integ-56 -> `ca4220a` (11:33) ; main = `ca4220a` pour le code, docs seulement ensuite ; integ-57 abandonnée (conflit de Placeholders.txt).
- Dans main pour le joueur : S1-a soin de zone Lumière sacrée, S1-b lancer par le serveur, S1-c panneau de faction cliquable, S1-d en script, S4-a l'IA lance Lumière sacrée, S4-b garnison de l'IA, tir de tour T7 / T7b / T8, palissade, XP du duel 150, lot de liaisons C++, B2 / G1 corrigés.
- Sur branche, non intégré : ability-columns, integ-55 (EndWallDrag), specialization-trial, integ-41 (portes et murs F9 / F10), zone-effect, socle-ai (S2-a non compilé), integ-25 ; 6 agents C++ non compilés à leur rapport, builds en file trop longs.
- Essais : 0 VU À L'ÉCRAN (tower_wood, wall_line_long, talents_heal = LOGIQUE SEULEMENT) ; 0 `AskUserQuestion` en séance, 7 questions [À RÉPONDRE] posées.
- Réponses de l'utilisateur à 12:05 : D890 à D895 (GDD.md) pour les 7 questions ; leçon neuve L25-6 (fin et mode demandés au lancement).
- Détail (tableau d'agents, clôtures, anomalies, bilan, tableau « Lecons appliquees 2026-11-23 », plan de lancement) : `Docs\Archive\Auto_Journey_Jour_2026-11-23_matin_archive_2026-11-23.md`.

