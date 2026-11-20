# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11), `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_soir_archive_2026-11-14.md` (séance autonome du 2026-11-13 soir : bilan, leçons, tableau d'agents ; résumés du 2026-11-13 matin et du 2026-11-12) et `Docs\Archive\Auto_Journey_Jour_2026-11-14_archive_2026-11-15.md` (séance autonome du 2026-11-14 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-15_archive_2026-11-16.md` (séance autonome du 2026-11-15 : tableau d'agents, fusions n°1-7, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-16_apres-midi_archive_2026-11-16.md` (séance interactive du 2026-11-16 après-midi : tableau d'agents, lignes d'exception, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_matin_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 matin : tableau d'agents, bilan, leçons, plan de lancement ; résumé dans ce fichier) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Chapitres du GDD non commencés

**Relevé du 2026-11-15** : voir Docs\ReleveGDD_2026-11-15.md (état réel de A1-A4 et B1-B6, branchements C++ à faire, questions en attente). L'ancien relevé du 2026-11-12 est archivé dans Docs\Archive\Auto_Journey_Jour_Chapitres_GDD_releve_2026-11-12_archive_2026-11-15.md.

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
