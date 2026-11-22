# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11), `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_soir_archive_2026-11-14.md` (séance autonome du 2026-11-13 soir : bilan, leçons, tableau d'agents ; résumés du 2026-11-13 matin et du 2026-11-12) et `Docs\Archive\Auto_Journey_Jour_2026-11-14_archive_2026-11-15.md` (séance autonome du 2026-11-14 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-15_archive_2026-11-16.md` (séance autonome du 2026-11-15 : tableau d'agents, fusions n°1-7, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-16_apres-midi_archive_2026-11-16.md` (séance interactive du 2026-11-16 après-midi : tableau d'agents, lignes d'exception, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_matin_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 matin : tableau d'agents, bilan, leçons, plan de lancement ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_apres-midi_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 après-midi : tableau d'agents, plan de lancement, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_soir_archive_2026-11-21.md` (séance autonome du 2026-11-20 soir, passée minuit : tableaux d'agents du 2026-11-20 et du 2026-11-21, bilan, leçons, exceptions de gate ; résumé dans ce fichier)  et `Docs\Archive\Auto_Journey_Jour_2026-11-21_matin_archive_2026-11-21.md` (séance autonome du 2026-11-21 matin 10:40 -> 14:40 : tableau d'agents, bilan, leçons, exceptions de gate ; résumé dans ce fichier)  et `Docs\Archive\Auto_Journey_Jour_2026-11-21_soir_archive_2026-11-22.md` (séance autonome du 2026-11-21 soir 19:00 -> 23:00 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Chapitres du GDD non commencés

**Relevé du 2026-11-15** : voir Docs\ReleveGDD_2026-11-15.md (état réel de A1-A4 et B1-B6, branchements C++ à faire, questions en attente). L'ancien relevé du 2026-11-12 est archivé dans Docs\Archive\Auto_Journey_Jour_Chapitres_GDD_releve_2026-11-12_archive_2026-11-15.md.

## Agents du 2026-11-22

Séance autonome (« lance une auto journey ») 16:17 -> 20:20 (fin choisie par la session principale, aucune heure donnée ; `Docs\SessionEnd.txt`). 0 `AskUserQuestion` (L21-1, L23-1). Modèles : la session principale reste sur Opus (utilisateur, 2026-11-22 : « on reste sur opus ») ; sous-agents Sonnet 5.5, 15 min au plus (consigne du 2026-11-16, inchangée).

check-list lue (L23) : 16:23, en entier, apres 0a (5d40153, 1447448) et avant le premier agent de travail. Alarme fin - 15 min posee (20:05), veilleur 30 min. Integrateur : agent de recette a nommer a la premiere avance de main, au plus tard 19:05 (L20-4) ; dernier lancement de code 19:20 (L18-c).

| Agent | id | Worktree | Tâche | C++ | Lancé | Fini | Résumé |
|---|---|---|---|---|---|---|---|
| CLEAN-1122a | 0a | copie principale | nettoyage Jour + Retours.md | non | 16:19 | fini | Jour 353 -> 84 l., 2 archives créées ; doute E-secondary_centre.md:26 |
| CLEAN-1122b | 0a | copie principale | nettoyage Auto_Journey_File.md | non | 16:19 | 16:22 | 13 lignes archivees (controle mecanique : rien de perdu), 89 [ ] dont 26 sans C++ |
| CLEAN-1122c | 0a | copie principale | nettoyage Retours\ | non | 16:19 | 16:23 (2 passes) | 0 fiche archivable, 2 index réécrits ; relancé pour 4 références périmées (E-secondary_centre, E-deploy, EtapeDefini l.162, compte) |
| CLEAN-1122d | 0a | copie principale | nettoyage procédure + check-list | non | 16:19 | 16:21 | L20-6 retirée, règle 14 / 0a / L19-6 « interactif seulement » ; puis brief_commun.md 4 et agent_rules.md 7 alignes sur L19-8/L22-2 (deux commits) |
| ETAPE0-1122 | 0 | lecture seule | plan de lancement (Docs\Rapports\ETAPE0-1122.md) | non | 16:23 | 16:30 | 2 vagues ; 0 conflit, 0 dll perimee ; D877-D884 et D824 recoupes dans GDD.md par la session principale |
| ELROND-1122a | v1 | Elrond integ-25 | D877 moitie A : rouge dans deploy.txt (deploy.txt, deploy.look, E-deploy.md) | non | 16:31 | 16:37 | 90ff8d3 ; rouge NON VU : deploy PASS 96 s, l'engin se monte 16 s apres l'ordre (fenetre de 15 s du scenario trop courte) ; pas de C++ |
| ELROND-1122b | v2 | Elrond integ-25 | deploy moitie B + camera + preflight (deploy.txt, deploy.look) | non | 16:39 | en cours | |
| GLORFINDEL-1122a | v1 | Glorfindel ability-columns | pont de Suivre en instance (HOLFollowBridge.*, HOLPlayerController.*, HOLFollowBridgeTests.cpp) | oui | 16:31 | 16:36 | 6aee2a7 rouge, cf625af vert, NON COMPILE ; build du worktree lance par la session principale |
| CELEBORN-1122a | v1 | Celeborn integ-41 | D880 aura de l'Aube (HOLMoraleProcessor.cpp, HOLMoraleRulesTests.cpp) | oui | 16:31 | 16:36 | 67f30df rouge, 32167d1 vert, NON COMPILE ; attend Arwen pour le build groupe |
| ARWEN-1122a | v1 | Arwen integ-37 | opacite de furtivite en donnee d'instance (HOLInterpolatedInstancesProcessor.cpp, HOLUnitOpacity.*), sans uasset | oui | 16:31 | 16:36 | 09868d9 rouge, b620434 vert, NON COMPILE ; verrou par unite et par pas a retirer avant main |
| ARWEN-1122b | v2 | Arwen integ-37 | opacite : une copie par pas au lieu d'un verrou par unite (memes fichiers) | oui | 16:38 | en cours | |
| CELEBORN-1122b | v2 | Celeborn integ-41 | D884 donnees du tir de TowerWood (BuildingTypes.csv, ligne de batiment, generateur, tests) ; D880 fige a 520450b | oui | 16:38 | en cours | |
| FARAMIR-1122a | v1 | Faramir zone-effect | D881 fond opaque Arbre de talents (fenetre, talents_heal_nocheat.*) | non | 16:31 | 16:37 | 0e328e9 ; fenetre translucide VUE (alpha 0,88, HOLUIStyles.h:23) mais ++ net par-dessus : cause a trouver avant tout correctif |
| FARAMIR-1122b | v2 | Faramir zone-effect | D881 : ou la marque ++ est dessinee (lecture seule + mesure du PNG) | non | 16:39 | en cours | |
| GALADRIEL-1122a | v1 | Galadriel specialization-trial | infobulle or et bandeau Scenario (HOLTopBar.as, HOLTopBar_Test.as) | non | 16:31 | 16:35 | 8376a47 rouge, 45b871c vert, NON VERIFIE (ni charge ni vu), inerte sans SetBannerRect ; 25 appels |
| GALADRIEL-1122b | v2 | Galadriel specialization-trial | brancher SetBannerRect (HOLGameScreenScript.as), compiler, voir le PNG FR | non | 16:36 | en cours | |
| HALDIR-1122a | v1 | Haldir wall-line | D824 zones de furtivite sur Forest (HOLStealthFilter.as + test) | non | 16:31 | 16:39 | f1b4bc9 rouge, 5e9e381 vert (19139aa avait perdu les regles) ; 4 zones Forest, 0 erreur AS au chargement ; 6 tests pas lances |
| integ-49 | passe | Haldir | Celeborn D880 (520450b) + Haldir (wall-line) : rouges 67f30df, f1b4bc9 | oui | 16:40 | en cours | |
| SAURON-1122a | v1 | Sauron integ-48 | neutrals_clairiere : pre-vol, PNG, fiche E | non | 16:31 | 16:33 | deja valide par l'utilisateur (archive 2026-11-20), 350223e docs, pas de doublon |
| SAURON-1122b | v2 | Sauron integ-48 | L22-6 gate Close : auteur/worktree (session_gate.ps1, MainCodeFilter.ps1, tests) | non | 16:34 | en cours | |

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
