# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11), `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_soir_archive_2026-11-14.md` (séance autonome du 2026-11-13 soir : bilan, leçons, tableau d'agents ; résumés du 2026-11-13 matin et du 2026-11-12) et `Docs\Archive\Auto_Journey_Jour_2026-11-14_archive_2026-11-15.md` (séance autonome du 2026-11-14 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-15_archive_2026-11-16.md` (séance autonome du 2026-11-15 : tableau d'agents, fusions n°1-7, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-16_apres-midi_archive_2026-11-16.md` (séance interactive du 2026-11-16 après-midi : tableau d'agents, lignes d'exception, bilan, leçons ; résumé dans ce fichier) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Chapitres du GDD non commencés

**Relevé du 2026-11-15** : voir Docs\ReleveGDD_2026-11-15.md (état réel de A1-A4 et B1-B6, branchements C++ à faire, questions en attente). L'ancien relevé du 2026-11-12 est archivé dans Docs\Archive\Auto_Journey_Jour_Chapitres_GDD_releve_2026-11-12_archive_2026-11-15.md.

## Agents du 2026-11-19

SÉANCE DE FONCTIONS DU SOIR (« commence ton Auto_Journey », 21:08 -> 23:00, AUTONOME ; la première depuis la fin de la migration, D857). Réponses de l'utilisateur à 21:1x (interface de choix) : fin à 23:00 (`Docs\SessionEnd.txt`) ; fusion dans main acceptée EN LOCAL si build PASS et tests verts sur le résultat, jamais de push ; mode autonome ; les trois docs non suivis de `Docs\` ne sont pas commités, et `Docs\Priorite_Socle_Factions_2026-11-16.md` ORIENTE la séance (socle commun des factions : 1 emplacement de faction + jauge + livre de pouvoirs, 2 capacité visée au sol, 3 moteur de moral, 4 spécialisation, 5 récompenses du duel, 6 fortifications, 7 IA d'une autre faction ; en parallèle Aube A1 et A2). Règles : agents Sonnet 5.5 de 15 min / 25 appels, sans build ni quick_check ; code neuf en Angelscript (D839) ; builds, tests (UN éditeur à la fois) et fusion groupée par la session principale, qui est l'intégrateur ; essais donnés depuis un worktree qui porte main compilé. État au lancement : main 2ff2dbf (local, 456 commits d'avance sur origin), les 8 worktrees sur main (Faramir `fog` +26 commits propres à sa branche), Celeborn 16 fichiers modifiés non commités, Haldir 1 ; veilleur toutes les 30 min, alarme de clôture 22:45. Le tableau de la séance de migration du même jour (reprises 13 à 17) est archivé : `Docs\Archive\Auto_Journey_Jour_2026-11-16_soir_au_2026-11-19_migration_archive_2026-11-19.md`.

| Agent | id | worktree | branche | tâche | C++ | lancé |
|---|---|---|---|---|---|---|
| NETTOYAGE-1119n-file | ab66fb6f | MyCrusader (docs) | main | étape 0a : nettoyage d'`Auto_Journey_File.md` | non | 21:12 |
| NETTOYAGE-1119n-retours | ab148a9e | MyCrusader (docs) | main | étape 0a : nettoyage du dossier `Retours\` | non | 21:12 |
| NETTOYAGE-1119n-procedure | a30db92b | MyCrusader (docs) | main | étape 0a : `Auto_Journey.md`, check-list, `brief_commun.md` | non | 21:12 |
| (session principale) | - | MyCrusader (docs) | main | étape 0a : archive d'`Auto_Journey_Jour.md` et de l'en-tête d'`Auto_Journey_Retours.md` ; FINI 21:16 : commit d02ae9f, 8 worktrees synchronisés, gate Launch sans FAIL | non | 21:13 |
| ETAPE0-1119n-A | a021779b | MyCrusader (lecture seule) | main | étape 0 : priorités 1-2 + Aube A1 ; FINI 21:20 : 4 tâches, rapport `Docs\Rapports\ETAPE0-1119n-A.md` | non | 21:14 |
| ETAPE0-1119n-B | a4d2c8ac | MyCrusader (lecture seule) | main | étape 0 : priorités 3-5 + Aube A2 + état de Celeborn et Haldir ; FINI 21:20 : 3 tâches, rapport `Docs\Rapports\ETAPE0-1119n-B.md` | non | 21:14 |
| ELROND-1119n1 | ae261387 | HeroesOfLiberty-Elrond | faction-resource | règle pure de la jauge de ressource de faction (priorité 1, A4) | non | 21:23 |
| GALADRIEL-1119n1 | aea92e83 | HeroesOfLiberty-Galadriel | power-book | règle pure de l'état des boutons du livre de pouvoirs (priorité 1, A6a) | non | 21:23 |
| GLORFINDEL-1119n1 | a91ae324 | HeroesOfLiberty-Glorfindel | ground-ability | règle pure d'une capacité visée au sol (priorité 2, L6a) | non | 21:23 |
| CELEBORN-1119n1 | aa4556ac | HeroesOfLiberty-Celeborn | duel-rewards | règle pure des récompenses du duel (priorité 5, D7) | non | 21:23 |
| HALDIR-1119n1 | a794ea96 | HeroesOfLiberty-Haldir | templar-options | spécialisation : Templiers à 3 options (priorité 4) | non | 21:23 |
| SAURON-1119n1 | a0f2c420 | MyCrusader (docs) | main | question `Retours\Q-hallebardier-valeurs.md` (Aube A1, valeurs à choisir, D345) | non | 21:23 |
| intégrateur = session principale | - | HeroesOfLiberty-Arwen | integ-24 | fusion des commits rouges puis des commits verts, `quick_check -Parts tests` deux fois, avance rapide de main | non | - |

Plan de lancement (étape 0, 21:20) : Elrond, Galadriel, Glorfindel, Celeborn, Haldir = une règle pure en script chacun (ci-dessus) ; Sauron = docs (question du Hallebardier) ; Arwen = intégration (porte main compilé) ; Faramir NON LANCÉ : branche `fog` à +26 commits propres (WIP D827/D828), aucune tâche du socle des factions n'y tient ce soir. Exceptions : aucun C++ ce soir (pas de build avant 23:00) ; écartés et reportés au lot C++ d'une autre séance : réplication et empreinte de la jauge, commande serveur de ciblage au sol, effet de zone côté Mass, moteur de moral M3-M5 et M7, Aube A2 (redirection de dégâts dans `HOLCombatProcessor`), effet de `hol.Cheat.Specialization` (pont `HOLRpcBridge`), faction du siège dans le tableau de spécialisation + 3e case de grille ; Aube A1 (ligne `Halberdier`) attend la réponse à `Q-hallebardier-valeurs` et une régénération de l'asset `UnitTypes` par l'éditeur.

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
