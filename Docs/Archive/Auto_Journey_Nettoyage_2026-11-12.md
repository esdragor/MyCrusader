# Auto_Journey : passages archivés au nettoyage du 2026-11-12 (étape 0a, agent 0a-4)

Texte retiré de `Auto_Journey.md` et `Auto_Journey_Checklist.md` le 2026-11-12, recopié tel quel. Raison de chaque retrait en tête de bloc. Aucune règle en vigueur n'a changé : chaque passage est remplacé dans le fichier d'origine par une phrase courte qui renvoie ici.

## 1. Auto_Journey.md, « Comment écrire l'énoncé d'un agent » (puce « Arrêt dur »)

Raison : mesure historique du test 2 (tokens), remplacée par le plafond d'appels d'outils (80 jugés sur `tool_uses`, arrêt à 45 vus par l'agent).

> ancienne mesure du test 2 : ~250k tokens (au test : 50 à 160k tokens, 10 à 30 min par agent, jamais près de 300k).

## 2. Auto_Journey.md, « Pièges techniques rencontrés » (dernière puce)

Raison : fait. `unit_stats_attack(_enemy)` rejoués (f274d32, 2026-11-09) puis joués par l'utilisateur le 2026-11-09 (Retours\ESSAIS-A-JOUER.md, REPONSES-2026-11-09.md) ; `disconnect_review` joué par l'utilisateur le 2026-11-09 ; `E-audio_panel` donné après la fusion 1 (4398e3a, 2026-11-08), il reste dans `Retours\` comme essai sonore pour l'utilisateur seul.

> - Rejouer `unit_stats_attack(_enemy)` sur un binaire d'après G1 (PNG ouverts) avant de les donner à l'utilisateur ; `E-audio_panel` et `E-disconnect_review` se donnent après les fusions.

## 3. Auto_Journey.md, « Builds : ne pas saturer la file » (premier paragraphe, début)

Raison : mesure du test 2 remplacée le 2026-11-03 par le plafond de 3 agents C++ (même section, puce « Plafond d'agents C++ »).

> Un premier build après un header prend ~200 s et la file de `Tools\build.ps1` est le goulot. **Test 2 : 6 à 7 agents en même temps ont tenu** (attente de build de 3 à 9 min, jamais bloquante) ; on lance tous les worktrees d'emblée, les agents de docs/GDD et ceux qui ne compilent pas tout de suite, les autres sans se soucier de la file (**remplacé le 2026-11-03 par le plafond de 3 agents C++ ci-dessous**).

## 4. Auto_Journey.md, « Fusions dans `main` » (deuxième paragraphe)

Raison : procédure branche par branche du test 2, remplacée le 2026-11-03 par la fusion groupée (« Builds : ne pas saturer la file »). Les règles encore en vigueur qu'il contenait (accord donné au lancement, jamais de fusion dans la copie principale, correction du script de test en premier, rien n'est poussé) sont redites dans la phrase de remplacement.

> (Procédure d'origine du test 2, branche par branche : depuis le 2026-11-03 la fusion groupée de « Builds : ne pas saturer la file » est la règle par défaut.) Autorisé le jour d'absence (décision A du 2026-10-30) et pour le test 2 (confirmé le 2026-10-30) ; sinon seulement si l'énoncé les autorise. Un **agent de fusion**, seul, après chaque fin d'agent ou toutes les heures : dans l'ordre fixe de `Auto_Journey_Retours.md` section 3 (`ai-ia1` d'abord), pour chaque branche dont le `quick_check` des parties touchées est vert : `git merge` dans le worktree de la branche (jamais dans la copie principale, qui porte des docs non commités), `quick_check -Parts` des parties touchées, puis fast-forward de `main`. Une correction du **script de test** qui fait échouer les autres branches (ex. `victory`) passe en premier. Rien n'est poussé.

## 5. Auto_Journey_Checklist.md : lignes de leçons devenues des contrôles automatiques de `session_gate.ps1`

Raison : chaque ligne est vérifiée réellement par `Tools\AutoJourney\session_gate.ps1` (vérifié par Grep le 2026-11-12, numéros de ligne du script entre crochets) ; la règle reste en vigueur, sa preuve au bilan est la sortie du gate. L6-1 est en outre contredite par L8-7 (WARN devenu FAIL).

| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L6-1 | Au moins 3 tâches `[ ]` étiquetées `sans C++` en file avant de lancer (la séance était vide après 14:12) | `session_gate.ps1 -Phase Launch` : WARN si < 3 | GATE |
| L6-3 | À la réception d'un rapport : la copie principale n'a que des docs voulus modifiés, le worktree contient main, est propre, binaire plus récent que le dernier code | `session_gate.ps1 -Phase Report -Root <worktree>` (FAIL / WARN) | GATE |
| L7-8 | Une règle Close ne doit pas échouer sur un worktree hors périmètre | session_gate -Phase Close accepte -Exclude <worktree> (Haldir exclu par l'utilisateur : 48 fichiers non commités d'une autre session) | GATE |
| L8-3 | L'heure de fin n'était lue que si `-End` était passé (les gates `Merge`/`Essay` affichaient « fin dure 13:30 ») | l'heure de fin écrite une fois dans un fichier de séance (`Docs\SessionEnd.txt`) lu par tous les gates | GATE |
| L8-7 | La file n'avait plus qu'une ligne `sans C++` en fin de séance (les agents libres n'ont pas pu être relancés) | `session_gate -Phase Launch` passe de WARN à FAIL sous 3 lignes `sans C++` au démarrage | GATE |
| L9-3 | session_gate -Phase Essay lit le mot « branches » dans la prose des fiches E-audio_compare_*.md : 17 FAIL, 12 essais sonores non donnés ; -Essay doit porter le nom de la fiche, pas du scénario | ne parser que la ligne ranches: ; accepter le nom du scénario et retrouver sa fiche | OUTIL (ligne de file) |
| L11-1 | Les 8 worktrees partaient avec des binaires perimes : un build de rattrapage de 397 s (Elrond, note de scenario) a fait attendre 25 a 30 min les 3 agents C++ (build d'Arwen 1002 s) | session_gate Launch : build de rattrapage seulement pour une tache qui joue un scenario ou touche au C++ ; binaire perime + tache de scenario = FAIL | GATE **[x] FAIT 2026-11-11** (Arwen-outils, d751-root : Launch -Scenarios = FAIL, sans = WARN, aussi quand les Source sont plus recentes que la DLL ; gate_test.ps1 cas « Source ecrit apres le binaire ») |
| L11-7 | Le gate Launch dit FAIL « plafond de 3 agents C++ » meme pour une tache sans C++ | WARN si la tache est declaree sans C++ | GATE **[x] FAIT 2026-11-11** (Arwen-outils, d751-root : `session_gate -Phase Launch -NoCpp` ; plafond compte depuis les tickets vivants de Docs\BuildQueue.state + IN_USE.txt qui cite « C++ » sans « sans C++ » et ne commence pas par « libre » ; gate_test.ps1 5 cas) |

Où le script les vérifie : L6-1/L8-7 [141-143, FAIL sous 3 lignes `sans C++`] ; L6-3 [162-182, `-Phase Report`] ; L7-8 [22, 85, 98, 273, `-Exclude`] ; L8-3 [12-13, 47-60, `Docs\SessionEnd.txt`] ; L9-3 [224-258, ligne `branches:` seule, nom de scénario accepté] ; L11-1 [145-160, `-Scenarios`] ; L11-7 [19, 104-127, `-NoCpp`].
