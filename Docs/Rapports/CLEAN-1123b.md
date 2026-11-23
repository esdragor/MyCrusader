# CLEAN-1123b : nettoyage de Auto_Journey_File.md (étape 0a, 2026-11-23)

APPELS_DECLARES: 11 (arrêt à 25)
HEURE_DEBUT/HEURE_FIN: 08:49 / 08:52 (arrêt dur 09:05)
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (docs seulement, aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (docs)   TEST_ROUGE_SEPARE: non (sans objet)
COMMITS: 1, d601023 à 08:51, merge_main_fait: sans objet (commit directement sur main, docs ; jamais de push)
TESTS: aucun ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune

## FAIT
- Archive : `Docs\Archive\Auto_Journey_File_faits_2026-11-23.md` (26 lignes copiées telles quelles, groupées par section, en-tête + titres `## Section`).
- `Auto_Journey_File.md` : 8 renvois d'une ligne (Prochaine séance, Arwen, Celeborn, Elrond, Faramir, Galadriel, Glorfindel, Haldir) + une phrase ajoutée à la ligne 2 (seizième nettoyage).
- Contrôle mécanique : 218 lignes avant = 192 restantes (hors renvois) + 26 archivées ; après = 200 (192 + 8 renvois).
- Preuve : tous les sha cités par les lignes archivées sont ancêtres de main (git merge-base --is-ancestor, tous `Y` sauf 564db8d, voir doutes). Aucune branche integ-* consultée au-delà de main : main contient integ-53.
- Archivées : [x] 61 (REVUE-1122 gênant 1, 3efc630), 70 (Aura de l'Aube), 121 + 122 (lignes de mouvement), 132, 133 ; PÉRIMÉES déjà balisées « périmée (ETAPE0-1122 §3) » ou « FERMÉE » et sha vérifiés dans main : 11, 15, 77, 78, 83, 84, 101, 102, 108, 119, 142, 155, 156, 158, 159, 171, 172, 173, 179, 180 (numéros = lignes AVANT nettoyage).
- Section « Socle commun des factions » : NON TOUCHÉE (aucune ligne S1 à S4 ni « restes isolés » marquée faite dans main ; aucun sha à l'appui).

## COMPTES (après nettoyage)
- Lignes `[ ]` ouvertes : 91.
- Dont sans le texte « C++ » dans la ligne : 14.
- Dont étiquetées « sans C++ » : 27 (une ligne peut porter les deux mentions, ex. « sans C++ pour 10 à 12 » ou « migration finie : C++ seulement si... »).
- `[x]` restant : 1 (ligne 109 de l'ancien numéro, voir doutes).

## DOUTES (lignes gardées)
- Ancienne ligne 109 `[x]` Bientôt tronqué (GALADRIEL-1122g) : 564db8d N'EST PAS dans main (0577971 non testé par le filtre de sha ; branche specialization-trial, non compilée) : gardée malgré le `[x]`, la ligne 110 (D889) en est la suite.
- Ancienne ligne 119 archivée mais sa sous-ligne 120 `[FAIT NON COMPILE]` (hol.UI.Type, 74745b8, e53ee06 dans main) gardée : compilation et rejeu chat_input non confirmés.
- Lignes 145 à 149 (suivi, e3ee5ad dans main) et 166 (6aee2a7, cf625af dans main) : sha dans main mais tests / compilation non confirmés par un rapport lu ici : gardées.
- Ancienne ligne 82 (Test_CGTables, « périmée très probablement, non revérifiée en jeu ») : gardée.
- Ancienne ligne 157 (Suivre morceau 2, FindEnemyInRange) : probablement couverte par la ligne 159 archivée (GetEnemiesNear), mais le nom diffère : gardée.
- Ancienne ligne 179 archivée alors que sa fin dit « reste C++ + éditeur : ligne SecondaryCentre » : le reste est porté par les lignes 170 et 178 (SC3) ; balise « périmée ETAPE0-1122 §3 » suivie.
- Lignes 16 et 17 (SAURON-1120sa, 1121a, sous-lignes de l'item 2 « neutrals », dont ancienne 15 archivée) : gardées, elles traitent de virtuous_knight (reste UE_LOG ApplyDamageRedirect).

## NON VÉRIFIÉ
- Le contenu des branches integ-* hors main (seul « ancêtre de main » a été testé) ; les lignes Socle S1 à S4 non confrontées au code (pas de lecture de Source\ ni Script\).
- `EtapeDefini.md` non mis à jour par cet agent (périmètre limité à `Auto_Journey_File.md`) : à noter par la session principale.
- Placeholders.txt : sans objet (aucun provisoire ajouté ni retiré).

## ANOMALIES
- Copie principale : `Auto_Journey.md`, `Auto_Journey_Checklist.md`, `Auto_Journey_Jour.md` modifiés et `Docs\Rapports\CLEAN-1123a/c/d.md` + `Docs\Archive\Auto_Journey_Procedure_Checklist_nettoyage_2026-11-23.md` non suivis : travail des autres agents de nettoyage, non touchés. Mon commit ne contient que `Auto_Journey_File.md` et l'archive.

PROCESSUS_DETACHES: aucun (aucun processus lancé)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE (après commit) : ` M Auto_Journey_Jour.md`, `?? Docs/Rapports/CLEAN-1123a.md`, `?? CLEAN-1123c.md`, `?? CLEAN-1123d.md` (autres agents) ; inchangée hors docs voulus : oui
