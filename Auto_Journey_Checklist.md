# Auto_Journey : check-list des leçons (chaque leçon = un contrôle mécanique)

Créée le 2026-11-05 à la demande de l'utilisateur : des axes d'amélioration revenaient de séance en séance sans être appliqués (plafond de tokens écrit trois fois, dépassé par 4 agents). **Règle de méthode : une leçon n'existe que si elle a un contrôle qu'on peut exécuter ou compter.** Quatre mécanismes :

- **GATE** = `Tools\AutoJourney\session_gate.ps1 -Phase Launch|Merge|Essay|Close` (script, sortie PASS/WARN/FAIL, code 1 s'il y a un FAIL). La session principale le lance : `Launch` avant CHAQUE lancement d'agent, `Merge` avant l'agent de fusion, `Essay` avant de donner une commande `play.ps1`, `Close` avant le bilan.
- **AGENT** = règle numérotée de `Tools\AutoJourney\agent_rules.md`, que chaque énoncé fait lire en premier.
- **RAPPORT** = champ du rapport final à 12 champs (voir `agent_rules.md`) que la session principale vérifie à la réception.
- **SESSION** = ligne que la session principale coche elle-même dans le tableau « Leçons appliquées » du bilan.

**Bilan de fin de séance (obligatoire, vérifié par `-Phase Close`)** : dans `Auto_Journey_Jour.md`, un titre `### Lecons appliquees <AAAA-MM-JJ>` suivi d'une ligne par règle ci-dessous : `<ID> : appliquee / non appliquee : <preuve (nombre, heure, SHA, sortie)>`. Une règle « non appliquée » devient une ligne `[ ]` de `Auto_Journey_File.md` (ou un durcissement du contrôle).

Origines : T2 = test 2 du 2026-10-30 ; Q = « Qualité des essais » du 2026-11-01 ; L1 = leçons du 2026-11-01 ; L3 = 2026-11-03 ; L4 = 2026-11-04 ; L5 = 2026-11-05.

## Avant le lancement
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| T2-1 | Étape 0 par sous-agent en lecture seule ; plan relu contre la procédure ; « rien d'ouvert » interdit | `Auto_Journey_Jour.md` contient le plan de lancement : une ligne par worktree (lancer / raison précise) | SESSION |
| T2-2 | Un agent par worktree, tous lancés d'emblée ; « Pris » n'est pas un interdit | tableau des agents : une ligne par worktree libre, ou la raison écrite | SESSION |
| L3-12 | Étape 0 sautée seulement si la file porte déjà les tâches du jour | la file contient des `[ ]` datés du jour | SESSION |
| L5-1 | Heure lue à `Get-Date` avant CHAQUE lancement ; aucun lancement s'il reste moins de 20 min | `session_gate.ps1 -Phase Launch -End HH:MM` (FAIL si < 20 min) | GATE |
| L5-2 | Tableau agent -> id -> worktree -> branche -> tâche, écrit AU LANCEMENT, relu avant chaque `SendMessage` | titre `## Agents du <date>` dans `Auto_Journey_Jour.md` | GATE |
| L3-1, L4-4 | Plafond de 3 agents C++ vérifié avant chaque lancement et écrit DANS l'énoncé | `Launch` : tickets de `Docs\BuildQueue.state` < 3 ; énoncé « sans C++ » sinon | GATE + AGENT 10 |
| L1-10 | Docs de la copie principale commités ; rien de suivi modifié ni de `*InProgress.md` non suivi | `Launch` : FAIL sur `*InProgress.md` non suivi, WARN sur fichier suivi modifié | GATE |
| L5-7 | Accord de commiter les dossiers non suivis demandé AU LANCEMENT | première question de la séance si le gate le signale | SESSION |
| L4-1, L4-3 | L'arrêt dur est un contrat ; alarme de fin posée au lancement ; prolongation demandée 15 min avant, max 2 cycles | `CronList` montre l'alarme « fin - 15 min » ; heure de fin écrite dans le titre du tableau | SESSION |
| L3-8, L4-9, Q5, L1-14 | Agent d'intégration NOMMÉ au lancement ; il rejoue les essais après la fusion | ligne `intégrateur` du tableau des agents | SESSION |
| L3-9 | La session principale se relance vers 200k : compter ses propres appels d'outils | à 150 appels de la session principale : écrire le point de reprise dans `Auto_Journey_Jour.md` | SESSION |
| T2-3 | Énoncé : chemins absolus, « enchaîne sans t'arrêter », arrêt dur, rapport fixe | l'énoncé commence par « lis Tools\AutoJourney\agent_rules.md » + l'heure d'arrêt calculée par `Launch` | AGENT |

## Pendant les agents
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L1-1, L3-6, L4-7, L5-3 | Un agent est remplacé à 200k tokens, jamais plus tard ; bug dur = agent neuf | **80 appels d'outils** max dans l'énoncé ; `tool_uses` de la notification > 100 = hors règle, tâche suivante sur un agent NEUF (jamais `SendMessage` pour reprendre) | AGENT 2 + RAPPORT `APPELS_OUTILS` |
| L1-2 | Commit par lot vert, au plus 30 min sans commit, fichiers par nom | `RAPPORT COMMITS` (dernier commit à HH:MM) ; `status.ps1` à chaque tour signale 45 min sans commit | AGENT 14 |
| L1-3 | `git merge main` avant le rapport | `RAPPORT merge_main_fait` | AGENT 15 |
| L3-2, L4-5 | Build C++ lancé dans les 15 premières minutes, en `run_in_background`, agent vivant jusqu'à sa fin ; jamais de code non compilé rendu comme fini | `RAPPORT PREMIER_BUILD`, `BUILD` ; « NON COMPILÉ » en tête sinon | AGENT 11 |
| L3-5 | Un seul lot de headers par agent avant le premier build | `RAPPORT PREMIER_BUILD` | AGENT 11 |
| L1-8 | Un seul enregistrement de statut par agent | `RAPPORT` : `*InProgress.md` + ligne d'`EtapeDefini.md` | AGENT 5 |
| L1-9, L4-8 | Numéros D réservés ; question sans réponse = valeur par défaut déjà codée | agents n'écrivent pas `GDD.md` ; fichier `Q-*.md` porte « valeur par défaut codée » | AGENT 4, 17 |
| L1-7 | Toute question : option « Autre », recommandation, valeurs par défaut, au plus 2 par chantier | `RAPPORT QUESTIONS` | AGENT 17 |
| L5-6 | Questions groupées dans UN tableau (question, recommandée, effet), raccourci « recommandées partout » | message de la session principale | SESSION |
| T2-5 | `quick_check` ne compile pas : `build.ps1` d'abord | `RAPPORT TESTS` après `BUILD: PASS` | AGENT 8 |
| T2-6, L5-8 | Un script n'est prêt qu'après un passage de bout en bout ; `net` rouge sans changement = relancer deux fois | sortie collée au rapport | AGENT 8, 9 |
| T2-7 | `Auto_Journey_Retours.md` seulement par `Edit` de lignes précises (un fichier par item dans `Retours\`) | agents écrivent `Retours\Q-*.md` / `E-*.md` | AGENT 17 |
| L1-11 | Machine : l'utilisateur joue depuis un build figé ; heure à PowerShell | commande `play.ps1` pointe un worktree validé par `Essay` | GATE |
| L3-4 | Écriture au bon endroit ; `git status` de la copie principale inchangé | `RAPPORT GIT_STATUS_PRINCIPALE` + `Close` | AGENT 4 + GATE |
| L4-12, T2 | Outil partagé corrigé au lieu d'un contournement privé | `RAPPORT ANOMALIES_PROCESSUS` cite l'outil corrigé | AGENT 9 |
| L4-13 | `Intermediate` périmé : supprimer UHT de CE worktree | AGENT 12 | AGENT |
| L3-11, L5-5 | Régénération des cartes seule, éditeur seul, `-Multiprocess`, mesure de perf, jamais mêlée à un réglage de budget | tâche dédiée dans le tableau des agents ; AGENT 13 | AGENT + SESSION |
| L3-7 | Veilleur : état déjà vu persisté, silencieux si rien n'a changé | ignorer le premier tour | SESSION |
| T2-4 | Au plus 2 questions par chantier et par séance | compte des `Q-*.md` par chantier | SESSION |

## Essais
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| Q1 | Pré-vol `preflight.ps1` PASS, `.look` présent, chaque PNG ouvert et vu | `RAPPORT ESSAIS` : « preflight PASS, PNG ouverts » | AGENT 18 + RAPPORT |
| Q2 | Deux statuts jamais mélangés : `VU À L'ÉCRAN` / `LOGIQUE SEULEMENT` | `RAPPORT ESSAIS` | AGENT 18 |
| Q3 | 3 minutes max, pas d'attente morte (`waitkey`) | durée mesurée par le pré-vol | AGENT 18 |
| Q4 | Un essai d'IA/système = au moins une partie naturelle | `RAPPORT ESSAIS` | AGENT 18 |
| Q5, L5-4 | **Recette indépendante, geste RÉEL, effet mesuré** pour tout essai ; REGLAGE/BUG rejoué par un autre agent avant la commande | ligne `intégrateur` + `RAPPORT TEST_ROUGE assertit l'effet : oui` ; `Essay` avant la commande | GATE + AGENT 7, 19 |
| Q6, Q7 | Réglage à l'œil = balayage ; défaut persistant après 2 réglages = remonter à la cause | `RAPPORT ESSAIS` (nombre de réglages tentés) | AGENT 18 |
| Q8 | Défaut commun = ligne `[ ]` en tête de file avant de relancer les essais concernés | `Auto_Journey_File.md` | SESSION |
| Q9 | Libellés et touches du GDD / grille AoE4 ; effet de gameplay visible sinon `LOGIQUE SEULEMENT` | AGENT 18 | AGENT |
| Q10, L4-10 | Un essai n'est donné qu'après fusion et build vérifié de `main` ; la commande pointe ce worktree | `session_gate.ps1 -Phase Essay -Root <worktree>` PASS (aucun écart de code avec main, propre, binaire plus récent que le dernier commit de code) | GATE |
| Q11 | ~6 essais de 3 min par jour au plus ; le reste « prêt mais non donné » | compte des `[À ESSAYER]` | SESSION |
| Q12, L1-6 | Réponses balisées et identifiées : `E1 OK`, `Q2 A`, `BUG:`, `REGLAGE:`, `A trancher` | la session principale enregistre ; `Close` signale une réponse non traitée dans `Retours\` | GATE |
| L4-11 | Plusieurs cartes/variantes = un scénario de revue (Entrée = suivante) | AGENT 18 | AGENT |

## Fusion et fin de séance
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L3 / T2 | Fusion groupée : toutes les branches prêtes, UN build `-Priority merge`, UN `quick_check`, avance rapide de `main` | énoncé de fusion de `Auto_Journey.md` | SESSION |
| L4-6 | La fusion groupée démarre à « fin - 30 min » | alarme + heure | SESSION |
| Pré-vol | Pré-vol de fusion de chaque branche ; rien n'entre dans `main` sans build PASS et `tests` tout vert sur le résultat | `session_gate.ps1 -Phase Merge -Branches a,b,c` | GATE |
| L1-3 | Les agents ont fusionné `main` avant le rapport : l'agent de fusion ne rejoue que `tests,aiserver,aiduel` | `RAPPORT merge_main_fait` | RAPPORT |
| L4-14 | Fin de séance : tous les worktrees remis à jour (`sync_worktrees.ps1`) ; travail non commité d'abord commité | `session_gate.ps1 -Phase Close` : FAIL si fichier suivi non commité, WARN si retard sur `main` | GATE |
| L1-13 | Test rouge vérifiable par l'ordre des commits | `RAPPORT TEST_ROUGE` : SHA du test antérieur au correctif | RAPPORT |
| L5-bilan | Chaque leçon « appliquée / non appliquée » avec preuve | `Close` FAIL sans le titre `### Lecons appliquees <date>` | GATE |

## Leçons du 2026-11-06 (séance d'1 h du 2026-11-05 après-midi) : contrôles ajoutés
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L6-1 | Au moins 3 tâches `[ ]` étiquetées `sans C++` en file avant de lancer (la séance était vide après 14:12) | `session_gate.ps1 -Phase Launch` : WARN si < 3 | GATE |
| L6-2 | La session principale ne lit jamais la file en entier (42k tokens) : l'étape 0 rend un extrait | consigne de l'étape 0 ; ligne de nettoyage de la file | SESSION |
| L6-3 | À la réception d'un rapport : la copie principale n'a que des docs voulus modifiés, le worktree contient main, est propre, binaire plus récent que le dernier code | `session_gate.ps1 -Phase Report -Root <worktree>` (FAIL / WARN) | GATE |
| L6-4 | Un bug rapporté se rejoue par le scénario et le geste exacts de l'utilisateur ; test « sur le geste réel » = scénario `click` | ligne `[ ]` « Scénario click » ; question de repro posée AVANT de lancer l'agent | SESSION + AGENT |
| L6-5 | Après `git merge main`, `build.ps1` avant tout `quick_check` | `-Phase Report` (WARN binaire périmé) ; ligne `sync_worktrees` + build | GATE |
| L6-6 | Mesures répétées sur un binaire copié, jamais sur celui que `build.ps1` doit écraser | ligne `[ ]` des mesures | AGENT |

## Outils encore à construire (lignes `[ ]` de `Auto_Journey_File.md`)
- L1-12 (a) capture en séquence au pas de simulation (prouver une animation) ; (b) détection par le pré-vol d'un panneau qui en cache un autre ; (c) `check.ps1` qui échoue quand l'audit `Sanity` échoue.
- L3-3 : journaux nommés de `quick_check` (`net`, `aiserver`...) sans `Get-FreshLog` partout.
- Un script `launch_agent` qui lit `Launch`, écrit la ligne du tableau des agents et génère l'énoncé (heure d'arrêt, rapport fixe) : supprimerait le reste de dépendance à ma mémoire.
