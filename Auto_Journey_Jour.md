# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11), `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_soir_archive_2026-11-14.md` (séance autonome du 2026-11-13 soir : bilan, leçons, tableau d'agents ; résumés du 2026-11-13 matin et du 2026-11-12) et `Docs\Archive\Auto_Journey_Jour_2026-11-14_archive_2026-11-15.md` (séance autonome du 2026-11-14 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-15_archive_2026-11-16.md` (séance autonome du 2026-11-15 : tableau d'agents, fusions n°1-7, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-16_apres-midi_archive_2026-11-16.md` (séance interactive du 2026-11-16 après-midi : tableau d'agents, lignes d'exception, bilan, leçons ; résumé dans ce fichier) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Chapitres du GDD non commencés

**Relevé du 2026-11-15** : voir Docs\ReleveGDD_2026-11-15.md (état réel de A1-A4 et B1-B6, branchements C++ à faire, questions en attente). L'ancien relevé du 2026-11-12 est archivé dans Docs\Archive\Auto_Journey_Jour_Chapitres_GDD_releve_2026-11-12_archive_2026-11-15.md.

## Agents du 2026-11-20

SÉANCE DE FONCTIONS DU 2026-11-20 (« commence un auto journée », 08:45 -> 17:00, AUTONOME). Réponses de l'utilisateur à 08:4x (interface de choix) : fin à 17:00 (`Docs\SessionEnd.txt`) ; fusion dans main EN LOCAL si build PASS et tests verts sur le résultat exact, jamais de push ; mode autonome (veilleur toutes les 30 min, alarme de clôture 16:45) ; les trois docs non suivis de `Docs\` ne sont pas commités. Règles : agents Sonnet 5.5 de 15 min / 25 appels, sans build ni quick_check ; code neuf en Angelscript (D839) ; builds, tests (UN éditeur à la fois) et fusion groupée par la session principale. État au lancement : main = 410cb7d (local, rien poussé), 8 worktrees sur main (Faramir `fog` +33 commits propres à sa branche), Celeborn 16 fichiers non suivis (sondes périmées), Haldir 1 (`Crown.umap`). Journée précédente archivée telle quelle : `Docs\Archive\Auto_Journey_Jour_2026-11-19_soir_archive_2026-11-20.md`.

| Agent | id | worktree | branche | tâche | C++ | lancé |
|---|---|---|---|---|---|---|
| NETTOYAGE-1120-file | a69f9223 | MyCrusader (docs) | main | étape 0a : nettoyage d'`Auto_Journey_File.md` ; FINI 08:53, commit 927e6ae | non | 08:52 |
| NETTOYAGE-1120-retours | a2e4ac6e | MyCrusader (docs) | main | étape 0a : nettoyage du dossier `Retours\` ; FINI 08:51, commit 61eb155 | non | 08:52 |
| NETTOYAGE-1120-procedure | ab46cfab | MyCrusader (docs) | main | étape 0a : `Auto_Journey.md`, check-list, `brief_commun.md` ; FINI 08:56, commit d2b70be | non | 08:52 |
| (session principale) | - | MyCrusader (docs) | main | étape 0a : archive d'`Auto_Journey_Jour.md` (copie intégrale) ; FINI 08:56 : commit 901e391, 8 worktrees synchronisés (docs seules, aucun build) | non | 08:53 |
| ETAPE0-1120-A | a14d31af | MyCrusader (lecture seule) | main | étape 0 : D861, D860, lot C++ du socle ; FINI 09:03, rapport `Docs\Rapports\ETAPE0-1120-A.md` | non | 08:57 |
| ETAPE0-1120-B | a3855592 | MyCrusader (lecture seule) | main | étape 0 : D859, D858, branchements, état des worktrees ; FINI 09:03, rapport `Docs\Rapports\ETAPE0-1120-B.md` | non | 08:57 |
| HALDIR-1120a | a4aae530 | HeroesOfLiberty-Haldir | wall-line | D859 1a + 1b : `HOLDawnValues.as`, `HOLDawnPowers.as` (valeurs d'Honneur et Lumière sacrée) | non | 09:05 |
| GLORFINDEL-1120a | a1700760 | HeroesOfLiberty-Glorfindel | ability-columns | D859 1c : ligne `LightSeal` de `UnitAbilities.csv` | non | 09:05 |
| CELEBORN-1120a | ae80bc59 | HeroesOfLiberty-Celeborn | duel-rewards | D859 1d (rayon 12 m dans `Effects.csv`) + 3a (`OrdersOfEnded`, boucle `Ended`) | non | 09:05 |
| SAURON-1120a | abcdb009 | HeroesOfLiberty-Sauron | tool-known-reds | D858 2a : Hallebardier (CSV, test 17 lignes, données ; PAS `HOLAIArmy.as`) | non | 09:05 |
| GALADRIEL-1120a | a3735a6f | HeroesOfLiberty-Galadriel | specialization-trial | D860 H3 : Alt + clic sur la mini-carte (script), puis D833 | non | 09:05 |
| ELROND-1120a | a2992dbc | HeroesOfLiberty-Elrond | integ-25 | D861 (b) : engin monté garde son ordre de déplacement, NON COMPILÉ | oui | 09:05 |
| FARAMIR-1120a | ab4588dd | HeroesOfLiberty-Faramir | zone-effect (neuve, depuis main ; `fog` intacte) | lot C++ morceau 4 : zone de ralentissement du Sceau de lumière, NON COMPILÉ | oui | 09:05 |

Plan de lancement (étape 0, 09:04) : 7 agents d'emblée (5 sans C++, 2 C++ : Elrond D861 et Faramir morceau 4, sous le plafond de 3) ; Arwen = intégrateur, ne code pas (builds, tests, éditeur, fusion par la session principale). Vague 2 prévue à la fin des agents : Haldir = D860 H2 sol (C++, `HOLSignalInput.*`, `HOLSelectionMarkers.cpp`), Sauron = D858 2b (`HOLAIArmy.as`), Elrond = D861 (a), Galadriel = morceau 6 / D833 puis `HOLFactionSlot` (3b), Glorfindel = morceau 2 (`hol.Cheat.Honor`, script si la garde de triche est lisible). Exceptions : Faramir sur une branche neuve parce que `fog` garde le WIP D827/D828 non compilé (à reprendre dans le lot C++ avec un build) ; les 16 sondes non suivies de Celeborn et `Crown.umap` de Haldir sont laissées telles quelles (ni commitées ni supprimées, à décider par l'utilisateur) ; Arwen ne reçoit pas D834 (worktree d'intégration).

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
