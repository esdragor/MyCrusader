# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11), `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_soir_archive_2026-11-14.md` (séance autonome du 2026-11-13 soir : bilan, leçons, tableau d'agents ; résumés du 2026-11-13 matin et du 2026-11-12) et `Docs\Archive\Auto_Journey_Jour_2026-11-14_archive_2026-11-15.md` (séance autonome du 2026-11-14 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-15_archive_2026-11-16.md` (séance autonome du 2026-11-15 : tableau d'agents, fusions n°1-7, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-16_apres-midi_archive_2026-11-16.md` (séance interactive du 2026-11-16 après-midi : tableau d'agents, lignes d'exception, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_matin_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 matin : tableau d'agents, bilan, leçons, plan de lancement ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_apres-midi_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 après-midi : tableau d'agents, plan de lancement, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_soir_archive_2026-11-21.md` (séance autonome du 2026-11-20 soir, passée minuit : tableaux d'agents du 2026-11-20 et du 2026-11-21, bilan, leçons, exceptions de gate ; résumé dans ce fichier)  et `Docs\Archive\Auto_Journey_Jour_2026-11-21_matin_archive_2026-11-21.md` (séance autonome du 2026-11-21 matin 10:40 -> 14:40 : tableau d'agents, bilan, leçons, exceptions de gate ; résumé dans ce fichier)  et `Docs\Archive\Auto_Journey_Jour_2026-11-21_soir_archive_2026-11-22.md` (séance autonome du 2026-11-21 soir 19:00 -> 23:00 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-22_archive_2026-11-23.md` (séance autonome du 2026-11-22 : tableau d'agents, bilan, leçons ; résumé dans ce fichier ; y sont aussi les résumés anciens du 2026-11-15 au 2026-11-20 après-midi, sortis le 2026-11-23) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Chapitres du GDD non commencés

**Relevé du 2026-11-15** : voir Docs\ReleveGDD_2026-11-15.md (état réel de A1-A4 et B1-B6, branchements C++ à faire, questions en attente). L'ancien relevé du 2026-11-12 est archivé dans Docs\Archive\Auto_Journey_Jour_Chapitres_GDD_releve_2026-11-12_archive_2026-11-15.md.

## Agents du 2026-11-23

Séance autonome (« lance une autojourney en prenant en prio Priorite_Socle_Factions_2026-11-16.md ») 08:47 -> 16:50 (fin choisie par la session principale, objectif ~8 h ; `Docs\SessionEnd.txt`). Priorité : `Docs\Priorite_Socle_Factions_2026-11-16.md` (lignes S1 à S4 d'`Auto_Journey_File.md`). 0 `AskUserQuestion` (L21-1, L23-1). Session principale sur Opus, sous-agents Sonnet 5.5, 15 min au plus.
Exception de gate : CLEAN-1123a-d (0a) lancés avec le FAIL L19-1 (check-list lue après le nettoyage 0a, qui la modifie, et avant le premier agent de travail, comme le 2026-11-22).

| Agent | id | Worktree | Tâche | C++ | Lancé | Fini | Résumé |
|---|---|---|---|---|---|---|---|
| CLEAN-1123a | 0a | copie principale | nettoyage Jour + Retours.md | non | 08:50 | | |
| CLEAN-1123b | 0a | copie principale | nettoyage Auto_Journey_File.md | non | 08:50 | | |
| CLEAN-1123c | 0a | copie principale | nettoyage Retours\ | non | 08:50 | | |
| CLEAN-1123d | 0a | copie principale | nettoyage procédure + check-list | non | 08:50 | | |

## Résumé de la séance du 2026-11-22

- Séance autonome 16:23 -> ~19:40 (étape 0 ETAPE0-1122 16:23-16:30, 8 worktrees lancés 16:31 : Elrond, Glorfindel, Celeborn, Arwen, Faramir, Galadriel, Haldir, Sauron), session principale + agents Sonnet ; CINQ passes d'`integrate.ps1` (integ-49 à 53), la dernière integ-53 `5a920a1` PASS du premier coup (tests 2097/2097, `net` et `build` PASS).
- `main` NON avancé : `main` = `4b6b573`, 13 commits de docs depuis le dernier code (`1276d54`), aucun code, faute d'accord de fusion au lancement (Q-accord-fusion ; D838 = séance de migration, `Auto_Journey.md` l. 75 et l. 188 se contredisent) ; le gate Essay est FAIL pour les 4 essais prêts.
- Livré pour le joueur dans integ-53 SEULEMENT (pas dans `main`) : VU À L'ÉCRAN : deploy et order_lines_blocks au vrai clic droit, secondary_centre, chat_input, income_tooltip, bandeau Scénario dégagé FR/EN/JA ; LOGIQUE SEULEMENT : suivi, soin par clic, aura de l'Aube D880, D882, zones de furtivité (rien de rendu : Q-stealth-rendu), tir de tour T1-T6.
- Non intégré : integ-41 (Celeborn, T1 à T6 verts, T7 et T8 à faire), specialization-trial (« Bientôt » EN : StatusFit rouge), integ-48 (gate Close L22-6, Q-gate-close-l22-6) ; REVUE-1122 : aucun bloquant, 5 gênants dont 2 corrigés (opacité de furtivité vidée entre parties, ordres manuels du suivi, 2e non prouvé seul), 3 restent en ligne [ ] de la file.
- 5 questions ouvertes (Q-accord-fusion, Q-heal-mark-couche-2, Q-gate-close-l22-6, Q-stealth-rendu, Q-follow-own-units), 0 `AskUserQuestion` ; défauts : 3 builds rouges d'agents C++ sans build (L20-5), refus du classifieur contourné une fois (GLORFINDEL-1122g), anomalie python3.13 PID 40180 non tuée.
- Détail (tableau d'agents, bilan, tableau « Lecons appliquees 2026-11-22 ») : `Docs\Archive\Auto_Journey_Jour_2026-11-22_archive_2026-11-23.md`.

## Résumé de la séance du 2026-11-21 soir

- Séance autonome 19:00 -> 23:00 (« lance une autojourney »), session principale + 85 identifiants d'agents Sonnet ; main 6fe9d84 -> 1276d54 en LOCAL, rien poussé, 5 avances par `integrate.ps1` (integ-42, 43, 44, 46, 48) ; integ-45 (build rouge UHT du pont de Suivre) et integ-47 (tests 0/0) abandonnées ; tests 2061/2061 et `net`/`build` PASS à chaque avance.
- Livré pour le joueur : furtivité (règle « vue par camp », filtre réseau), moral (fragment D866, processeur, aura de l'Aube, bannière, cri), centre secondaire « tout centre », D877 deploy en 2 moitiés, infobulle de revenu et traductions EN/JA, jauge de faction sous le bloc héros, lignes d'ordre multi-blocs, chat Tab.
- NON intégré : série Glorfindel du pont de Suivre (`UHOLFollowBridge::Get()` renvoie le CDO, 19 727 erreurs AS ; reste sur ability-columns) ; dessin de l'opacité de furtivité ; tir de la tour de bois (Q-towerwood-tir).
- Essais en fin de séance : 6 [À ESSAYER] dont 4 prêts VU À L'ÉCRAN (secondary_centre, chat_input, order_lines_blocks, income_tooltip) ; deploy PAS prêt (rejeu après D877), talents_heal LOGIQUE SEULEMENT. Les 5 questions et le commit de GDD.md ont été répondus le 2026-11-22 (D880 à D884).
- Rouges : L19-8 non appliquée pour les passes 2, 3 et 5 ; exceptions de gate Merge (GDD.md non commité, 4 avances) ; leçons L22-1 à L22-7 dans `Auto_Journey_Checklist.md`.
- Détail (tableau d'agents, bilan, tableau « Lecons appliquees 2026-11-21 », exceptions de gate) : `Docs\Archive\Auto_Journey_Jour_2026-11-21_soir_archive_2026-11-22.md`.

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
