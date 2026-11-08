# Auto_Journey : check-list des leçons (chaque leçon = un contrôle mécanique)

Créée le 2026-11-05 à la demande de l'utilisateur : des axes d'amélioration revenaient de séance en séance sans être appliqués (plafond de tokens écrit trois fois, dépassé par 4 agents). **Règle de méthode : une leçon n'existe que si elle a un contrôle qu'on peut exécuter ou compter.** Quatre mécanismes :

- **GATE** = `Tools\AutoJourney\session_gate.ps1 -Phase Launch|Merge|Essay|Close` (script, sortie PASS/WARN/FAIL, code 1 s'il y a un FAIL). La session principale le lance : `Launch` avant CHAQUE lancement d'agent, `Merge` avant l'agent de fusion, `Essay` avant de donner une commande `play.ps1`, `Close` avant le bilan.
- **AGENT** = règle numérotée de `Tools\AutoJourney\agent_rules.md`, que chaque énoncé fait lire en premier.
- **RAPPORT** = champ du rapport final à 13 champs (voir `agent_rules.md`) que la session principale vérifie à la réception.
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
| L1-1, L3-6, L4-7, L5-3 | Un agent est remplacé à 200k tokens, jamais plus tard ; bug dur = agent neuf | **80 appels d'outils** max (jugés sur `tool_uses`), arrêt à **60 appels déclarés** dans l'énoncé ; `tool_uses` de la notification > 100 = hors règle, tâche suivante sur un agent NEUF (jamais `SendMessage` pour reprendre) | AGENT 2 + RAPPORT `APPELS_DECLARES` |
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

## Leçons du 2026-11-05 soir (séance autonome 19:12-21:38) : contrôles ajoutés
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L7-1 | Le décompte d'appels déclaré par les agents est faux de 10 à 30 % (77 déclarés = 108 réels) : seul tool_uses de la notification compte | à chaque rapport, la session principale lit tool_uses : > 80 = noté hors règle, > 100 = tâche suivante sur agent NEUF et ligne au bilan ; le champ du rapport devient APPELS_DECLARES (arrêt à 60 déclarés), le seuil de 80 se juge sur tool_uses relevé par la session | SESSION |
| L7-2 | Un lancement ne vérifie pas l'état réel du worktree (DLL périmée : Galadriel-7 18 appels perdus ; SHA périmés donnés à la fusion 1) | session_gate.ps1 -Phase Launch -Root <worktree> : FAIL si la DLL du worktree est plus ancienne que le dernier commit de code de main ET que la tâche joue des scénarios ; les énoncés de fusion contiennent les tips relevés par git rev-parse <branche> au moment du lancement, jamais des SHA recopiés d'un rapport | GATE + SESSION |
| L7-3 | Pas de verrou partagé pour l'éditeur : 2 à 8 éditeurs en permanence, Arwen-5 a attendu ~20 appels et ses 3 pré-vols n'ont pas abouti | un ticket d'éditeur dans Docs\BuildQueue.state (comme build.ps1) pris par preflight.ps1 / check.ps1 ; au plus 2 éditeurs simultanés | OUTIL |
| L7-4 | Un agent laisse un processus détaché après sa fin (chain.ps1 de Celeborn-4 a relancé des éditeurs, chain2.ps1 d'Arwen-5) | champ de rapport PROCESSUS_DETACHES (liste ou « aucun ») ; la session principale arrête ceux qui restent avant de lancer l'agent suivant du même worktree (Get-CimInstance Win32_Process filtré sur le worktree) | RAPPORT + SESSION |
| L7-5 | Test rouge non observé quand un symbole manque : le test ne compile pas | l'énoncé d'un agent C++ impose un stub qui compile + test AVANT le premier build (rouge observé), budget de deux builds, et le champ TEST_ROUGE donne « observé » ou « supposé » | AGENT + RAPPORT |
| L7-6 | Les essais passent par des commandes internes (hol.*), jamais par la vraie touche ni le vrai clic ; aucun son entendu | statut explicite GESTE: réel / interne et SON: entendu / non entendu dans chaque Retours\E-*.md ; un essai de geste ou de son reste LOGIQUE SEULEMENT tant que le scénario click à la vraie touche n'existe pas | AGENT 18 + SESSION |
| L7-7 | Fusion anticipée (20:00 au lieu de 21:35) : a laissé du temps à deux intégrateurs qui ont trouvé 4 défauts invisibles aux auteurs | première fusion groupée dès que 3 branches sont vertes, pas à « fin - 30 min » ; intégrateur nommé AU LANCEMENT | SESSION |
| L7-8 | Une règle Close ne doit pas échouer sur un worktree hors périmètre | session_gate -Phase Close accepte -Exclude <worktree> (Haldir exclu par l'utilisateur : 48 fichiers non commités d'une autre session) | GATE |


## Leçons du 2026-11-06 (séance interactive 10:53-13:00, cartes exclues) : contrôles ajoutés
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L8-1 | Un essai se joue depuis un worktree qui porte TOUS les correctifs cités (qground joué depuis Elrond sans les correctifs de mêlée de Celeborn : défaut déjà traité retrouvé, rejeu nécessaire) | `session_gate -Phase Essay -Root` vérifie que le worktree contient le tip de chaque branche citée dans le `E-*.md` ; la commande `play.ps1` donnée pointe toujours le worktree qui porte `main` compilé | GATE (ligne `[ ]` de la file) |
| L8-2 | Un blocage en mêlée (soldat en idle ou début d'animation d'attaque en boucle devant sa cible) n'est détecté par aucun test : l'audit Sanity ne voit que les rotations | audit « cible à portée, aucune attaque depuis N s » dans `hol.Test.Sanity` ; devient le test rouge de `BUG-melee-stuck-attack` | AGENT C++ |
| L8-3 | L'heure de fin n'était lue que si `-End` était passé (les gates `Merge`/`Essay` affichaient « fin dure 13:30 ») | l'heure de fin écrite une fois dans un fichier de séance (`Docs\SessionEnd.txt`) lu par tous les gates | GATE |
| L8-4 | Une réponse d'essai sonore ambiguë (« UI.Open F, UI.Open A » = en fait Open puis Close) a coûté une question de plus | `E-audio_compare.md` porte d'avance une ligne `Ui.Open = _` par cue ; le panneau d'écoute affiche le nom du cue à côté de la lettre | AGENT 18 |
| L8-5 | Les rouges connus (`camera 2p`, `camera 8p`) sont remesurés par chaque agent de fusion | fichier `Tools\Tests\known_reds.txt` (test, depuis quand, cause supposée) lu par `quick_check` : « rouge connu » distinct de « régression » | OUTIL |
| L8-6 | Fins de ligne mêlées (`Auto_Journey_File.md`, `EtapeDefini.md`), heredocs Bash cassés par le hook, `$r`/`$R` confondus en PowerShell : appels perdus | `.gitattributes` (proposé le 2026-11-01, jamais fait) ; consigne d'énoncé : écrire par l'outil Write/Edit, jamais par heredoc ; variables PowerShell de noms distincts | OUTIL + AGENT |
| L8-7 | La file n'avait plus qu'une ligne `sans C++` en fin de séance (les agents libres n'ont pas pu être relancés) | `session_gate -Phase Launch` passe de WARN à FAIL sous 3 lignes `sans C++` au démarrage | GATE |
| L8-8 | Le build de fusion (11 à 18 min) est le chemin critique ; la fusion 3 est partie à 12:30 pour finir à 12:47 | lancer la fusion C++ dès que la branche C++ est prête (avant les autres) et fusionner en second tour les branches script-only sans rebuild (comme `heal-sleep` en fusion 3) | SESSION |
| L8-9 | **Demande de l'utilisateur (2026-11-06) : au DÉBUT et à la FIN de chaque Auto_Journey, tous les worktrees sont remis à jour avec `main`**, pour qu'ils restent à jour le plus longtemps possible | début : `Tools\AutoJourney\sync_worktrees.ps1` AVANT le premier lancement (`session_gate -Phase Launch` : WARN par worktree en retard) ; fin : `sync_worktrees.ps1` après la dernière fusion et le dernier commit de docs, aucun agent ne tourne (`session_gate -Phase Close` : FAIL par worktree en retard, `-Exclude` pour les worktrees hors périmètre) ; un saut (travail non commité) ou un conflit entre dans le bilan | GATE + SESSION |

## Leçons du 2026-11-07 après-midi (séance 12:10 -> 16:10, 29 agents, 2 fusions) : contrôles à ajouter
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L9-1 | 3 agents (Glorfindel-15/16, Celeborn-18) ont re-notifié leur rapport en boucle (minuteur de réveil / Monitor propre à l'agent) et ont consommé des appels | l'énoncé interdit « minuteur, cron, Monitor » ; la session principale fait TaskStop à la 2e notification identique | AGENT + SESSION |
| L9-2 | 5 agents ont dépassé 80 appels (99, 89, 88, 85, 83) : leur décompte déclaré est faux de 25 % | arrêt dans l'énoncé à « 45 appels VUS PAR TOI » (appliqué dès 14:00, jamais dépassé ensuite) ; le modèle d'énoncé de Auto_Journey.md passe de 60 à 45 | AGENT |
| L9-3 | session_gate -Phase Essay lit le mot « branches » dans la prose des fiches E-audio_compare_*.md : 17 FAIL, 12 essais sonores non donnés ; -Essay doit porter le nom de la fiche, pas du scénario | ne parser que la ligne ranches: ; accepter le nom du scénario et retrouver sa fiche | OUTIL (ligne de file) |
| L9-4 | Auto_Journey_File.md aux fins de ligne mêlées : conflit TOTAL à chaque fusion (3 fois), 2 lignes perdues | CRLF + BOM partout depuis Elrond-14 ; eol_check.ps1 dans session_gate -Phase Close ; ligne de statut par agent dans un fichier à part (comme Retours\) plutôt qu'une ligne du fichier partagé | OUTIL + AGENT |
| L9-5 | L'avance rapide a été bloquée 2 fois par des docs non commités de la copie principale (agents qui écrivent dans les deux copies) | script commit_docs.ps1 (Auto_Journey_*.md, EtapeDefini.md, Retours\*) lancé par la session avant CHAQUE Launch et Merge ; session_gate -Phase Merge distingue « main n'a que des docs » (WARN) de « main a du code » (FAIL) | OUTIL |
| L9-6 | La fusion 1 a pris 55 min et 99 appels : les tips de 3 branches ont bougé pendant la fusion (net-playtest en 3 temps) | figer les branches à fusionner (aucun agent actif dessus) AVANT de lancer l'agent de fusion ; énoncé avec tips relevés à la minute | SESSION |
| L9-7 | 7 agents neufs en chaîne sur la mêlée (Celeborn-13 à 19) : chacun relit ~10 appels de contexte et repart de petites séries (N = 4 à 8) bruitées | outil ab_matrix.ps1 (configs x scénarios x N, tableau PASS/N écrit dans un fichier) ; repeat_scenario écrit « PASS k/N » (aujourd'hui « FAIL k/N » avec k = PASS) ; N = 8 minimum avant toute conclusion | OUTIL **FAIT 2026-11-08** (Arwen-21, heal-sleep : ab_matrix.ps1 + ab_matrix_test.ps1 PASS ; repeat_scenario ecrit PASS k/N + verdict OK/RED, test rouge e400b36 vert 83c59e6) |
| L9-8 | Haldir (3 agents), Sauron, Arwen, Elrond sans tâche utile de 14:20 à 16:10 : la file « sans C++ » était vide et aucun chapitre du GDD n'est libre (tous ont un *InProgress.md) | l'étape 0 liste les chapitres GDD libres (sans *InProgress.md) ; en fin de séance précédente préparer 3 lignes sans C++ par worktree ; réserver un créneau MACHINE CALME en début de séance pour les mesures S1/S2 (jamais possible en cours de séance) | SESSION + ÉTAPE 0 |
| L9-9 | Mode autonome choisi alors que l'utilisateur était présent : ses réponses (Q-melee-stall A, sites, panneau B) n'ont pas été appliquées en séance | si l'utilisateur répond pendant une séance, proposer de passer en mode interactif (réponse = ligne de tête + relance de l'agent du worktree) | SESSION |
| L9-10 | Essais sonores : aucun agent n'entend rien ; 12 écoutes réparties en 12 scénarios | un seul panneau d'écoute (touche = cue suivant, lettre = vote, résumé écrit à la fin) ; statut LOGIQUE SEULEMENT à vie pour les agents | AGENT 18 |
