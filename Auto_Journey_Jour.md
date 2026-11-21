# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11), `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_soir_archive_2026-11-14.md` (séance autonome du 2026-11-13 soir : bilan, leçons, tableau d'agents ; résumés du 2026-11-13 matin et du 2026-11-12) et `Docs\Archive\Auto_Journey_Jour_2026-11-14_archive_2026-11-15.md` (séance autonome du 2026-11-14 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-15_archive_2026-11-16.md` (séance autonome du 2026-11-15 : tableau d'agents, fusions n°1-7, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-16_apres-midi_archive_2026-11-16.md` (séance interactive du 2026-11-16 après-midi : tableau d'agents, lignes d'exception, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_matin_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 matin : tableau d'agents, bilan, leçons, plan de lancement ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_apres-midi_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 après-midi : tableau d'agents, plan de lancement, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_soir_archive_2026-11-21.md` (séance autonome du 2026-11-20 soir, passée minuit : tableaux d'agents du 2026-11-20 et du 2026-11-21, bilan, leçons, exceptions de gate ; résumé dans ce fichier)  et `Docs\Archive\Auto_Journey_Jour_2026-11-21_matin_archive_2026-11-21.md` (séance autonome du 2026-11-21 matin 10:40 -> 14:40 : tableau d'agents, bilan, leçons, exceptions de gate ; résumé dans ce fichier) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Chapitres du GDD non commencés

**Relevé du 2026-11-15** : voir Docs\ReleveGDD_2026-11-15.md (état réel de A1-A4 et B1-B6, branchements C++ à faire, questions en attente). L'ancien relevé du 2026-11-12 est archivé dans Docs\Archive\Auto_Journey_Jour_Chapitres_GDD_releve_2026-11-12_archive_2026-11-15.md.

## Agents du 2026-11-21

SÉANCE DU 2026-11-21 SOIR (« lance une autojourney », 19:00 -> 23:00, autonome). Réponses de lancement (19:00, interface de choix) : fin 23:00 (`Docs\SessionEnd.txt`) ; fusion dans main EN LOCAL si build PASS et tests verts sur le résultat exact, jamais de push ; mode AUTONOME (veilleur 30 min à :13/:43, alarme 22:45 ; L21-1 : aucune AskUserQuestion en cours de séance). main = 6fe9d84 au lancement. Nettoyage 0a (4 agents) : 9243427, 0a4fa29, d5667a7, 5d812ee. check-list lue (L21) à 19:03, en entier, par la session principale, après 0a et avant tout lancement d'agent de travail.

| Agent | id | Worktree | Tâche | C++ | Lancé | Fini | Résumé |
|---|---|---|---|---|---|---|---|
| CLEAN-1121e | a7e43872 | principale | 0a : Jour + Retours | non | 19:00 | 19:02 | 0a4fa29 |
| CLEAN-1121f | af35e5bd | principale | 0a : file | non | 19:00 | 19:03 | d5667a7, 1 [x] archivée, 10 [~] traitées |
| CLEAN-1121g | aabd91f5 | principale | 0a : Retours\ | non | 19:00 | 19:01 | 9243427, LISEZ-MOI seul |
| CLEAN-1121h | aa0e20d3 | principale | 0a : procédure + check-list | non | 19:00 | 19:03 | 5d812ee, 4 lignes retirées |
| sync (commande) | — | 8 worktrees | sync_worktrees.ps1 -WaitBuilds | non | 19:04 | 19:04 | 8 à 6b226f8 (+6), aucun build |
| ETAPE0-1121s | ae87928e | lecture seule | plan de lancement | non | 19:05 | 19:10 | Docs\Rapports\ETAPE0-1121s.md ; plafond C++ 3 ; gate : seule la DLL de Celeborn est périmée (l'étape 0 les disait toutes en retard) |
| ARWEN-1121sa | acb211df | Arwen | L61 furtivité « vue par camp » (script) | non | 19:11 | | pris : Script\Fog\HOLStealthRules* |
| CELEBORN-1121sa | ad828dd0 | Celeborn | L29 D866 FHOLMoraleFragment sur archétypes | oui | 19:11 | | pris : Simulation\HOLSimulationFragments.h, HOLSimulationSubsystem.cpp, HOLMoraleRules*, HOLMoraleRulesTests.cpp ; build par la session après |
| ELROND-1121sa | ab28191f | Elrond | L70 Test_CGTables (TowerWood) puis rouge L69 D877 | non (L69 oui, sans build) | 19:11 | | pris : HOLCommandGrid_Test.as, HOLDeployButtonTests.cpp |
| FARAMIR-1121sa | ac1fa015 | Faramir | scénario order_lines_blocks + preflight | non | 19:11 | | pris : Tools\Scenarios\order_lines_blocks.*, Retours\E/BUG-order_lines* |
| GALADRIEL-1121sa | a89d2997 | Galadriel | L104 income_tooltip preflight | non | 19:11 | | pris : Script\UI\HOLTopBar.as, income_tooltip.* |
| GLORFINDEL-1121sa | a0386acd | Glorfindel | L11 chat_input Tab (rejouer sur binaire à jour) | oui si rouge | 19:11 | | pris : UI\HOLScriptScreen.*, HOLChatInputScreen.as, chat_input.* |
| HALDIR-1121sa | a8f5f07a | Haldir | L129-131 SC3 « tout centre » script | non | 19:11 | | pris : HOLCommandGridGroups.as, HOLAudioBuildings.as, HOLSecondaryCentreAll_Test.as (neuf) |
| SAURON-1121sa | a3a76a9c | Sauron | neutrals_clairiere (vérifier d'abord : déjà OK ?) sinon ligne suivante | non | 19:11 | 19:12 | déjà joué OK (archive Retours_2026-11-20) ; l.147 périmée ; rien sans C++ |
| SAURON-1121sb | abf83c40 | Sauron | L28 UE_LOG ApplyDamageRedirect (lot C++, sans build) + l.139/140 en [x] | oui, sans build | 19:13 | 19:15 | 28048ce NON COMPILÉ ; l.140/141 [x] |
| ARWEN-1121sa fin | acb211df | Arwen | (voir plus haut) | non | 19:11 | 19:14 | 990a147 rouge, d886e79 règle, e2e8c5c ; 1 rouge attendu Test_StealthCampViewEnemyBeyondRevealDistanceDoesNotSee |
| ARWEN-1121sb | a600f8fa | Arwen | 1re [ ] sans C++ de sa section, sinon furtivité étape suivante (script) | non | 19:14 | | |
| HALDIR-1121sa fin | a8f5f07a | Haldir | (voir plus haut) | non | 19:11 | 19:15 | 0516682 rouge (5 tests), 7393200 règle |
| HALDIR-1121sb | (voir notif) | Haldir | lot C++ « tout centre » (3 emplois), sans build | oui, sans build | 19:16 | | pris : HOLConstructionRules.*, HOLBuildProcessor.cpp, HOLGameNames.cpp, HOLBuildingTypes.* |
| intégrateur | — | Sauron | worktree d'intégration (integrate.ps1, commandes de la session principale) dès la fin de la vague | — | | | |
| fins vague 1 | — | — | CELEBORN-1121sa 19:15 fd6027d rouge + 7a7172b ; FARAMIR-1121sa 19:15 c399c87/0e8771f preflight order_lines_blocks PASS, E-order_lines_blocks.md ; ARWEN-1121sb 19:16 c68a4ba rouge (2) + c674622 contour ; ELROND-1121sa 19:16 fc901f7 Test_CGTables 14, 0d477b4 rouge D877 ; GLORFINDEL-1121sa 19:16 chat_input VERT (Tab), E-chat_input.md, aucun commit | | | | |
| ELROND-1121sb | a13bb260 | Elrond | correctif D877 (rouge 0d477b4), sans build | oui, sans build | 19:17 | | pris : HOLUnitOrderProcessor.cpp, Siege\HOLDeployRules.*, HOLDeployProcessor.*, HOLDeployButtonTests.cpp |
| CELEBORN-1121sb | ab07e228 | Celeborn | L30 UHOLMoraleProcessor, sans build | oui, sans build | 19:17 | | pris : HOLMoraleProcessor.* (neuf), HOLMoraleRules*, HOLMoraleRulesTests.cpp |
| ARWEN-1121sc | af31bbae | Arwen | liaison C++ vue par camp (UHOLVisionSubsystem), sans build | oui, sans build | 19:17 | | pris : HOLVisionSubsystem.*, HOLStealthRules*.as |
| FARAMIR-1121sb | a49efe0e | Faramir | 1re [ ] sans C++ de sa section | non | 19:17 | | |
| GLORFINDEL-1121sb | a719179d | Glorfindel | 1re [ ] sans C++ de sa section | non | 19:17 | | |
| HALDIR-1121sb fin | aec35b60 | Haldir | (voir plus haut) | oui, sans build | 19:16 | 19:19 | 8c3ea36 rouge Buildings.AnyCentre, 33b8563 IsCentre (2 emplois + nom) |
| HALDIR-1121sc | (voir notif) | Haldir | merge fc901f7 + regen BuildingTypes + Test_CGTables 15 + placeholders | non | 19:19 | | pris : BuildingTypes.csv/.uasset, HOLCommandGrid_Test.as, placeholders_map.csv |
| fins 19:2x | — | — | GLORFINDEL-1121sb d5a5527 rouge + 78f17c4 alerte Population pleine (règle) ; FARAMIR-1121sb 4146ec1 talents_heal (preflight FAIL 2 WARN) ; ELROND-1121sb 212ff44 D877 moitié 1 (NON COMPILÉ) | | | | |
| ELROND-1121sc | (notif) | Elrond | D877 moitié 2 (clic sur cible démonté) | oui, sans build | 19:22 | | pris : HOLUnitOrderProcessor.cpp, HOLDeploy*, HOLDeployButtonTests.cpp, HOLCombatProcessor.cpp (ordres d'engin) |
| FARAMIR-1121sc | (notif) | Faramir | talents_heal : 2 WARN + PNG + fiche E | non | 19:22 | | pris : talents_heal.* |
| GLORFINDEL-1121sc | (notif) | Glorfindel | brancher Population pleine dans HOLGameScreenScript.as | non | 19:22 | | pris : HOLGameScreenScript.as, HOLAlertRules.as |
| fins 19:2x (2) | — | — | HALDIR-1121sc 5a62867 merge fc901f7, regen uasset, ed2566e Test_CGTables 15, placeholders : wall-line prête ; CELEBORN-1121sb 9757c95 rouge + 9c2145b UHOLMoraleProcessor (NON COMPILÉ, non inscrit dans DefaultGame.ini) | | | | |
| CELEBORN-1121sc | (notif) | Celeborn | inscription DefaultGame.ini + ClearAuras | oui, sans build | 19:24 | | pris : DefaultGame.ini (SimulationProcessors), HOLMorale* |
| HALDIR-1121sd | (notif) | Haldir | 1re [ ] sans C++ ou autres emplois MainCentre en script | non | 19:24 | | |
| GALADRIEL-1121sa fin | a89d2997 | Galadriel | (voir plus haut) | non | 19:11 | 19:25 | income_tooltip PASS (4 PNG), 59334c6 scénario, 116bf29 retraduction JA SANS rouge (L19-8 non) ; E-income_tooltip.md ; 30 appels |
| GALADRIEL-1121sb | (notif) | Galadriel | test rouge séparé de 116bf29 + docs | non | 19:25 | | pris : test de HOLTopBar |
| fins 19:23 | — | — | ARWEN-1121sc 537c6ec rouge + 2b0c986 GetCampUnitPositions (NON COMPILÉ), 26 appels ; GLORFINDEL-1121sc : Population pleine déjà dans main (D833), doublon d5a5527/78f17c4 retiré par d6d5270 (branche nette = rien à fusionner) ; leçon : tâche « trouvée » par l'agent = doublon | | | | |
| HALDIR-1121sd fin | a16b4522 | Haldir | (voir plus haut) | non | 19:24 | 19:27 | 1a1b459 rouge HOLCentreFallback_Test, b4628e4 OwnedCentreType (Rally/Queue/RallySite), 0251e52 |
| CELEBORN-1121sc fin | abea607b | Celeborn | (voir plus haut) | oui, sans build | 19:24 | 19:27 | 456c1dc DefaultGame.ini, 6e98882 rouge ProcessorClearsAurasEachStep, 0895ac7 (NON COMPILÉ) |
| fins 19:25 | — | — | FARAMIR-1121sc 2a11ec2 talents_heal : « ++ » contourné, preflight FAIL (HUD overlap HeroPanel x FactionPanel, vrai défaut) = LOGIQUE SEULEMENT ; ELROND-1121sc 6baa6ba rouge ClickOnFarTarget (peut-être vert : cause non diagnostiquée) + 49363b6 bMarching à l'émission ; GALADRIEL-1121sb c945861 test seul (rouge = erreur de compilation, pas une assertion : exception L19-8, entre en vert) | | | | |
| integ-42 (commande) | — | Sauron | integrate.ps1 rouges 990a147,fd6027d,0d477b4 ; verts a7bf803,7a7172b,28048ce,0e8771f,fc901f7 ; parts net,build | oui | 19:17 | 19:31 | FAIL (parts) 832 s : 3 rouges VUS (noms attendus), verts 2000/2000, net PASS, build hud + build barracks REGRESSION ; c68a4ba hors passe (conflit de cherry-pick) |
| vague 3 (19:33) | — | — | tips figés à 19:32 pour la passe 2 : integ-37 b4ded14, integ-41 0895ac7, wall-line 0251e52, specialization-trial 83f7304, zone-effect 2a11ec2, integ-25 807a0b9 ; Glorfindel d6d5270 = net nul, hors passe | | | | |
| FARAMIR-1121sd | (notif) | Faramir | HUD overlap HeroPanel x FactionPanel (script) + talents_heal | non | 19:33 | | pris : HeroPanel/FactionPanel script, talents_heal.* |
| ARWEN-1121sd | (notif) | Arwen | filtre réseau UHOLFogNetSubsystem | oui, sans build | 19:33 | | pris : HOLFogNetSubsystem.*, HOLVisionSubsystem.* |
| CELEBORN-1121sd | (notif) | Celeborn | repose des auras D705 (une source) | oui, sans build | 19:33 | | pris : HOLMoraleProcessor.*, HOLMoraleRules* |
| HALDIR-1121se | (notif) | Haldir | FindMainCentreOf / PointNearMainCentre (D879) | oui, sans build | 19:33 | | pris : leurs fichiers C++, HOLBuildingTypes.* |
| GALADRIEL-1121sc | (notif) | Galadriel | accesseur des coups reçus (UHOLScriptScreen) | oui, sans build | 19:33 | | pris : UI\HOLScriptScreen.* |
| GLORFINDEL-1121sd | (notif) | Glorfindel | liaison position d'unité par id (Suivre) | oui, sans build | 19:33 | | pris : liaison HOLUnitSubsystem, HOLFollowOrders.as |
| DIAG-1121sa | a894f3ec | Sauron | cause et correctif build hud / barracks dans integ-42, quick_check -Parts build | non | 19:32 | 19:35 | attentes de quick_check.ps1 périmées (TowerWood case F, D878) : 9e7a324, quick_check -Parts build PASS 115 s (37 étapes) ; jeu correct |
| fusion main | — | principale | preflight_merge OK ; gate Merge 1 FAIL = GDD.md non commité (D877-D879, autre session, 14:11, aucun recoupement) : EXCEPTION écrite ici, comme ce matin ; question du commit de GDD.md au bilan (Retours) | — | 19:35 | 19:35 | main 6b226f8 -> 9e7a324 (ff local, rien poussé) : build PASS, tests 2000/2000, net PASS, build PASS |
| fins 19:35 | — | — | HALDIR-1121se 3bd682e rouge Buildings.CentrePick, a1360b1 FindCentreIndex (joueur : principal puis secondaire ; ennemi : principal seul, choix de l'agent) ; ARWEN-1121sd 9049921 rouge, 28dddc1 UHOLStealthFilter, 480873b appel dans HOLUnitReplicationSubsystem.cpp ; GALADRIEL-1121sc 9659c97 rouge (2), c6060a0 GetHitsReceivedNow ; tous NON COMPILÉS | | | | |
| RECETTE-1121sa | (notif) | Sauron | recette chat_input + order_lines_blocks sur main 9e7a324 | non | 19:37 | | |

## Résumé de la séance du 2026-11-21 matin
Séance autonome 10:40 -> 14:40 (clôture 14:53) : 23 agents Sonnet, main 61a60e8 -> a987f58 en local (integ-38 PASS 957 s, integ-41 tests 1990/1990), rien poussé. Livré : chat (Tab, marque « (Alliés) »), alertes mineures par type, infobulle de revenu, règle Suivre, scénarios deploy et virtuous_knight, gate Essay acceptant un sha.
Arrêt 11:10 -> 14:09 : AskUserQuestion posé en séance autonome (L21-1). Réponses : deploy BUG D877, TowerWood A (D878), centre secondaire A (D879), virtuous_knight OK.
Non intégrés : Elrond integ-25 (Test_CGTables rouge, case de grille 7 de TowerWood), Haldir SecondaryCentre (conflit uasset). Lot C++ à faire : UE_LOG ApplyDamageRedirect, position d'unité par id (Suivre), accesseur des coups reçus, « tout centre », colonnes d'attaque de TowerWood, FHOLMoraleFragment sur les archétypes.
Détail complet (tableau d'agents, bilan, leçons L21-1 à L21-3 appliquées ou non) : archive ci-dessus.


## Résumé de la séance du 2026-11-20 soir

- Séance autonome « choses en suspens » 22:38 -> ~01:00 (fin prolongée une fois), session principale + ~25 agents Sonnet ; 7 passes d'`integrate.ps1` (integ-31 à 37, 36 et 37 vertes) ; main = f46dc2d (code) + docs, LOCAL, rien poussé ; tests 1977/1977, `net` et `build` PASS, gate Close 00:22 sans FAIL.
- Entré dans main, visible : D869b montage de l'engin (barre dès l'ordre, 2e cycle « Démonter »), chat (2e Entrée rouvre le champ, marque de canal lisible), `virtuous_knight` (redirection prouvée, case E active), « Population pleine » 15 s ; outils : `merge=union` des journaux (L20-1), gate Close L19-1 (L20-3), `regen_tables.ps1` (L20-9), `integrate.ps1` repart de main (37ee2c0).
- Trouvé : 2 erreurs de compilation script à l'intégration (accolade manquante, `WidgetBlueprint::` au lieu de `Widget::`), 1 test faux (SecondMountReturnsToPack), 1 énoncé qui lisait mal D833. Reste en tête de file : chat Tab perdu (C++ probable, reporté), « Allies » sans accent, capture deploy_move_unmounting trop tôt, `hol.Select.Hold` en scénario.
- Détail (tableaux d'agents, bilan, tableau « Lecons appliquees 2026-11-21 », exceptions de gate) : `Docs\Archive\Auto_Journey_Jour_2026-11-20_soir_archive_2026-11-21.md` ; 2 essais [À ESSAYER] (deploy, virtuous_knight) dans `Auto_Journey_Retours.md`.

## Résumé de la séance du 2026-11-20 après-midi

- Séance interactive 15:32 -> 18:00, session principale + ~45 agents Sonnet en 8 vagues ; intégration integ-30 (Arwen) en 4 passes ; main = fd41600 (LOCAL, rien poussé) ; tests 1971/1971 puis 1972/1972, `net` et `build` PASS.
- Visible en jeu : D869 montage/démontage de l'engin (partiel), chat privé cliquable + `/w`, ligne Diplomatie cliquable, Chevalier Vertueux à la Caserne (D873), clairière des neutres sans bosquet (VU À L'ÉCRAN, essai joué OK).
- Sans effet visible : D862, D864/D865, D867, D868, D870, garnison, moral des duels, `integrate.ps1`, `hol.Mouse.Drag`, `hol.Decor.ClearNear Trees`, anneau de 8 m ; réponses D871 à D876 enregistrées.
- Trouvé : 4 erreurs de compilation d'agents sans build, 1 bug de type d'occupant de garnison (a680f3f), conflits de journaux à presque chaque fusion ; recette : 3 défauts en tête de file (deploy partiel, 2e Entrée du chat, scénario virtuous_knight).
- Détail (tableau d'agents, bilan, tableau « Lecons appliquees 2026-11-20 », plan de lancement) : `Docs\Archive\Auto_Journey_Jour_2026-11-20_apres-midi_archive_2026-11-20.md` ; axes L20-1 à L20-9 : `Auto_Journey_Checklist.md`.

## Résumé de la séance de fonctions du 2026-11-20 matin

- Séance autonome 08:45 -> arrêt ~13:01 à la demande de l'utilisateur, session principale + environ 50 agents Sonnet en 11 vagues ; quatre intégrations (integ-26 à integ-29), main 410cb7d -> 991a8bc, tests 1922/1922 (1817 au lancement : 105 tests neufs), `net` et `build` PASS, cinq essais donnés (VU À L'ÉCRAN), 0 question ouverte à la fin (7 réponses D862-D868 enregistrées).
- Entré dans main, visible : Hallebardier de l'Aube (D858), engin de siège qui garde son ordre (D861), signal animé et Alt + clic sur la mini-carte (D860), barre d'Honneur dans le HUD, son de l'Écurie (D834), lignes de mouvement par bloc. Sans effet visible encore : valeurs D859, zone et pose du Sceau de lumière, triche `hol.Cheat.Honor`, visée au sol, garnison, alertes mineures, saisie du chat, pose en ligne des murs, redirection de dégâts A2, WIP D827 / D828.
- Clôture rattrapée à 14:30-15:31 à la demande de l'utilisateur (`session_gate.ps1 -Phase Close` non lancé à l'arrêt) : main a831674 poussé ; les 7 worktrees de branche fusionnés en avance rapide sur a831674 puis build PASS chacun ; Arwen = main 991a8bc compilé, gardé tel quel pour les essais.
- Leçons L19-1 à L19-9 : check-list lue et session principale = lancer seulement (L19-1, non appliquée), pièges script et diagnostics (L19-2), chemins absolus (L19-3), identifiants de données cherchés dans quick_check (L19-4), fiche E dès le pré-vol (L19-5), Q posées dès qu'elles paraissent (L19-6), mémoire libre avant un lot C++ (L19-7), test rouge séparé (L19-8), `integrate.ps1` (L19-9).
- Détail (tableau d'agents, bilan, tableau « Lecons appliquees 2026-11-20 », plan de lancement) : `Docs\Archive\Auto_Journey_Jour_2026-11-20_matin_archive_2026-11-20.md`.

## Résumé de la séance de fonctions du 2026-11-19 soir

- Séance autonome 21:08 -> 23:10, session principale + 22 agents Sonnet : socle commun des factions en règles pures script (jauge de faction, livre de pouvoirs, emplacement de faction, capacité visée au sol, Templiers à 3 options, récompenses du duel, pose en ligne des murs, dépense des pouvoirs par l'IA) + petit lot C++ (jauge répliquée `AHOLGameState::FactionResource`, triche `hol.Cheat.Specialization`) ; main df895b0 puis docs, tests 1817/1817, partie `net` PASS, gate Close sans FAIL ; rien branché en jeu pour le joueur.
- Réponses du soir : D858 (Hallebardier valeurs B, remplace le Lancier), D859 (10 valeurs du socle recommandées), D860 (signal animé + Alt + clic mini-carte), D861 (engin de siège garde son ordre pendant le déploiement) ; `specialization_button` OK. Suite : lot C++ groupé du socle (`Docs\Rapports\LOT-CPP-socle-factions-1119.md`), corrections D860/D861, Aube A1.
- Leçons : un fichier GÉNÉRÉ modifié par un agent sans exécution se relit en `git diff` avant le premier lancement ; l'énoncé d'une règle neuve donne le type canonique des entrées communes (faction, siège, case) ; le gate Close refuse un lot C++ resté hors de main (L15-1).
- Détail (tableau de 22 agents, bilan, tableau « Lecons appliquees 2026-11-19 ») : `Docs\Archive\Auto_Journey_Jour_2026-11-19_soir_archive_2026-11-20.md`.

## Résumé des séances de migration du 2026-11-16 soir au 2026-11-19

- Migration C++ -> Angelscript (D830, D835), 17 reprises du 2026-11-16 21:03 au 2026-11-19 ~20:35, environ 300 agents Sonnet ; TERMINÉE : étape 5 close par D857 (2026-11-19 19:23), les 43 corps serveur `Do<X>` et l'enveloppe réseau restent en C++.
- main = dc993f6 (integ-23) + docs = 2ff2dbf, LOCAL, RIEN POUSSÉ (push : « pas maintenant », à la décision de l'utilisateur) ; build PASS, tests 1782/1782 ; chaîne réseau complète verte sur integ-21 (non rejouée sur dc993f6 : écart de tests seulement).
- Décisions D835 à D857 dans le GDD (commité le 2026-11-19 avec l'accord de l'utilisateur) ; règle permanente : code neuf en Angelscript, C++ par exception (D839) ; tests de la simulation Mass en C++ (D838).
- Rien n'attendait l'utilisateur à la fin : Forge (D856) jouée OK, les 5 essais de revue du HUD joués OK.
- Non vérifié à la clôture : périmètre Ferme + camps de D856 accepté par un « OK » d'essai seulement ; 2 lignes `[~]` de migration douteuses dans la file (MatchConfig, L17-12 Production) ; 6 lignes `[~]` rangées FAITE sur la seule absence du fichier C++ de test.
- Leçons de la reprise 17 : un test porté par un agent sans build peut changer le sens de l'assertion (lire le premier rouge avant de conclure) ; `PlayerArray` se parcourt par indice + `.Get()` en Angelscript ; un build lancé par une session qui s'arrête meurt avec elle ; une archive de la file se fait par section entière.
- Détail (tableaux d'agents du 2026-11-16 au 2026-11-19, bilans des reprises, tableaux « Lecons appliquees » des 2026-11-16, 2026-11-17 et 2026-11-19) : `Docs\Archive\Auto_Journey_Jour_2026-11-16_soir_au_2026-11-19_migration_archive_2026-11-19.md`.

## Résumé de la séance du 2026-11-16 après-midi

- Séance interactive 14:46 -> ~18:00, 27 agents, fusion groupée FUSION-1116a, rien poussé : main 9c9d22b -> 3d3cb3c, tests 1326/1328 (2 rouges connus D831 : NomadFirstForum, NomadReplicated).
- Décisions D827-D834 ; D830 = arrêt du développement C++, passage du maximum en Angelscript, tests compris (plan Docs\Rapports\ANALYSE-1116a.md).
- Non fini : specialization_button échoue sur main (ligne Elrond), signals_altclick sans mapSize (ligne Haldir), WIP Faramir D827/D828 non compilé, défaut du cvar hol.Nomad.ForumCell et tests Nomad à corriger avec la migration D830.
- Worktrees à rebuilder (sync_worktrees -NoBuild 17:45) : Arwen, Elrond, Faramir, Galadriel, Glorfindel, Haldir, Sauron (Celeborn à jour).
- Détail (tableau d'agents, exceptions, bilan, leçons) : `Docs\Archive\Auto_Journey_Jour_2026-11-16_apres-midi_archive_2026-11-16.md`.

## Résumé de la séance du 2026-11-15

- Séance autonome 09:02 -> 14:37 (arrêtée par l'utilisateur), ~65 agents, 7 fusions groupées, rien poussé : main 140cdb7 -> 22ad07b, tests 1267 -> 1320 verts.
- Reste ROUGE non commité, worktrees non synchronisés avec main : Elrond (tiers : texte de la case de spécialisation en cours + rapport HUD, test ForumGrid rouge voulu, git mv du test vers le module UI) ; Galadriel (galadriel-tab : alerte population branchée, test sans PlayerContext, suite dans AlertsMinorInProgress.md).
- Détail (tableau d'agents, fusions, bilan, leçons) : `Docs\Archive\Auto_Journey_Jour_2026-11-15_archive_2026-11-16.md`.
