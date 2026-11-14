# Auto_Journey : check-list des leçons (chaque leçon = un contrôle mécanique)

Créée le 2026-11-05 à la demande de l'utilisateur : des axes d'amélioration revenaient de séance en séance sans être appliqués (plafond de tokens écrit trois fois, dépassé par 4 agents). **Règle de méthode : une leçon n'existe que si elle a un contrôle qu'on peut exécuter ou compter.** Quatre mécanismes :

- **GATE** = `Tools\AutoJourney\session_gate.ps1 -Phase Launch|Merge|Essay|Close` (script, sortie PASS/WARN/FAIL, code 1 s'il y a un FAIL). La session principale le lance : `Launch` avant CHAQUE lancement d'agent, `Merge` avant l'agent de fusion, `Essay` avant de donner une commande `play.ps1`, `Close` avant le bilan.
- **AGENT** = règle numérotée de `Tools\AutoJourney\agent_rules.md`, que chaque énoncé fait lire en premier.
- **RAPPORT** = champ du rapport final à 13 champs (voir `agent_rules.md`) que la session principale vérifie à la réception.
- **SESSION** = ligne que la session principale coche elle-même dans le tableau « Leçons appliquées » du bilan.

**Bilan de fin de séance (obligatoire, vérifié par `-Phase Close`)** : dans `Auto_Journey_Jour.md`, un titre `### Lecons appliquees <AAAA-MM-JJ>` suivi d'une ligne par règle ci-dessous : `<ID> : appliquee / non appliquee : <preuve (nombre, heure, SHA, sortie)>`. Une règle « non appliquée » devient une ligne `[ ]` de `Auto_Journey_File.md` (ou un durcissement du contrôle).

Origines : T2 = test 2 du 2026-10-30 ; Q = « Qualité des essais » du 2026-11-01 ; L1 = leçons du 2026-11-01 ; L3 = 2026-11-03 ; L4 = 2026-11-04 ; L5 = 2026-11-05 ; texte intégral des leçons L1, L3 et L4 archivé dans `Docs\Archive\Auto_Journey_Lecons_jusqu_au_2026-11-09.md`.

**Devenues des contrôles automatiques de `session_gate.ps1`** (lignes archivées le 2026-11-12 dans `Docs\Archive\Auto_Journey_Nettoyage_2026-11-12.md`, règles toujours en vigueur, cochées au bilan comme les autres avec la sortie du gate pour preuve) : L6-1 et L8-7 (`Launch` FAIL sous 3 lignes `[ ]` `sans C++`), L6-3 (`-Phase Report` à la réception d'un rapport), L7-8 (`Close -Exclude`), L8-3 (heure de fin lue dans `Docs\SessionEnd.txt`), L9-3 (`Essay` : seule la ligne `branches:` compte, nom de scénario accepté), L11-1 (`Launch -Scenarios` : binaire périmé = FAIL), L11-7 (`Launch -NoCpp`).

**Devenues des contrôles de `session_gate.ps1` (2026-11-13)** (lignes archivées dans `Docs\Archive\Auto_Journey_Checklist_nettoyage_2026-11-13.md`, règles toujours en vigueur, cochées au bilan comme les autres) : L6-5 (après `git merge main`, `build.ps1` avant tout `quick_check` : `-Phase Report` WARN binaire périmé), L7-4 (aucun processus détaché à la fin d'un agent : AGENT 20, champ `PROCESSUS_DETACHES`, gate WARN « processus detaches restants » ; la session les arrête avant l'agent suivant du même worktree), L7-6 (`GESTE: réel / interne` et `SON: entendu / non entendu` dans chaque `E-*.md` : AGENT 18, `Close` WARN sinon), L8-1 (essai joué depuis un worktree qui porte le tip de chaque branche citée : `Essay` ; la commande `play.ps1` pointe le worktree qui porte `main` compilé), L9-5 (`commit_docs.ps1` avant chaque `Launch` et `Merge` ; `Merge` distingue docs (WARN) et code (FAIL)), L10-1 (aucun `play.ps1` sans `-Hidden` dans un énoncé : AGENT 0, `Launch` WARN par `no_window_test.ps1`).

**Devenues des règles d'`agent_rules.md` (2026-11-13, même archive)** : L8-5 (AGENT 8 : `known_reds.txt` lu par `quick_check`, « rouge connu » distinct de « REGRESSION »), L8-6 (AGENT 6 : Write/Edit, jamais de heredoc ; `.gitattributes` fait, voir L9-4 ; reste à appliquer par l'énoncé : variables PowerShell de noms distincts, `$r`/`$R`).

**Redondantes ou outils faits (2026-11-13, même archive)** : L7-1 et L9-2 (= ligne « L1-1, L3-6, L4-7, L5-3 » ci-dessous et AGENT 2), L7-3 (fait : `Tools\Scenarios\EditorSlot.ps1`, au plus 2 éditeurs, pris par `Scenarios\check.ps1`), L9-7 et L10-5 (fait : `ab_matrix.ps1`, PASS k/N ; leur règle est condensée dans la ligne L11-10), L11-5 (fait : `commit_docs.ps1` couvre Placeholders*, *InProgress.md suivis, `Docs\SessionEnd.txt`), L11-6 (fait : `play.ps1 -Hidden` ferme ses éditeurs, `play_hidden_test.ps1` ; règle AGENT 0).

**Nettoyage du 2026-11-13 soir** (lignes archivées dans `Docs\Archive\Auto_Journey_Procedure_Checklist_nettoyage_2026-11-13_soir.md`, règles toujours en vigueur, cochées au bilan avec la sortie du gate pour preuve) : devenues des contrôles de `session_gate.ps1` : L5-1, L3-10, L4-2 (`Launch -End` FAIL sous 5 min), L5-2 (`Launch` FAIL sans `## Agents du <date>`), L3-1, L4-4 (`Launch` plafond de 3 agents C++, AGENT 10), L1-10 (`Launch` FAIL sur `*InProgress.md` non suivi, WARN sur fichier suivi modifié), L1-11 et Q10, L4-10 (`Essay -Root` : aucun écart de code avec main, worktree propre, binaire à jour), L3-4 (AGENT 4, champ `GIT_STATUS_PRINCIPALE`, `Report` et `Close`), L7-2 (`Launch -Root -Scenarios` : DLL périmée = FAIL ; tips de fusion : voir L9-6), L8-9 (`Launch` WARN par worktree en retard, `Close` FAIL ; procédure, étapes 0 et 8), pré-vol de fusion (`Merge -Branches`), L4-14 (`Close` FAIL sur fichier non commité ou retard sur main), L5-bilan (`Close` FAIL sans `### Lecons appliquees <date>`), L12-1c (`Launch` WARN mémoire virtuelle < 8 Go ou `cl.exe` d'un autre moteur ; `build.ps1` attend et écrit la colonne `load`). Redites ou outils faits : L4-6 (= L7-7 et L8-8), L11-2 (fait : `build.ps1 -Wait`, modèle d'énoncé et « Builds : ne pas saturer la file »).

## Avant le lancement
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| T2-1 | Étape 0 par sous-agent en lecture seule ; plan relu contre la procédure ; « rien d'ouvert » interdit | `Auto_Journey_Jour.md` contient le plan de lancement : une ligne par worktree (lancer / raison précise) | SESSION |
| T2-2 | Un agent par worktree, tous lancés d'emblée ; « Pris » n'est pas un interdit | tableau des agents : une ligne par worktree libre, ou la raison écrite | SESSION |
| L3-12 | Étape 0 sautée seulement si la file porte déjà les tâches du jour | la file contient des `[ ]` datés du jour | SESSION |
| L5-7 | Accord de commiter les dossiers non suivis demandé AU LANCEMENT | première question de la séance si le gate le signale | SESSION |
| L4-1, L4-3 | L'arrêt dur est un contrat ; alarme de fin posée au lancement ; prolongation demandée 15 min avant, max 2 cycles | `CronList` montre l'alarme « fin - 15 min » ; heure de fin écrite dans le titre du tableau ; **assoupli par la règle de durée du 2026-11-05 PM** : arrêt dur des agents = l'heure de fin, dépassement toléré 15 min au plus, au-delà on demande | SESSION |
| L3-8, L4-9, Q5, L1-14 | Agent d'intégration NOMMÉ au lancement ; il rejoue les essais après la fusion | ligne `intégrateur` du tableau des agents | SESSION |
| L3-9 | La session principale se relance vers 200k : compter ses propres appels d'outils | à 150 appels de la session principale : écrire le point de reprise dans `Auto_Journey_Jour.md` | SESSION |
| T2-3 | Énoncé : chemins absolus, « enchaîne sans t'arrêter », arrêt dur, rapport fixe | l'énoncé commence par « lis Tools\AutoJourney\agent_rules.md » + l'heure d'arrêt calculée par `Launch` | AGENT |

## Pendant les agents
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L1-1, L3-6, L4-7, L5-3 | Un agent est remplacé à 200k tokens, jamais plus tard ; bug dur = agent neuf | **80 appels d'outils** max (jugés sur `tool_uses`), arrêt à **45 appels VUS PAR L'AGENT** dans l'énoncé (L9-2 du 2026-11-07 ; avant : 60 déclarés) ; `tool_uses` de la notification > 100 = hors règle, tâche suivante sur un agent NEUF (jamais `SendMessage` pour reprendre) | AGENT 2 + RAPPORT `APPELS_DECLARES` |
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
| T2-7, L1-5 | `Auto_Journey_Retours.md` seulement par `Edit` de lignes précises (un fichier par item dans `Retours\`) | agents écrivent `Retours\Q-*.md` / `E-*.md` | AGENT 17 |
| L4-12, T2 | Outil partagé corrigé au lieu d'un contournement privé | `RAPPORT ANOMALIES_PROCESSUS` cite l'outil corrigé | AGENT 9 |
| L4-13 | `Intermediate` périmé : supprimer UHT de CE worktree | AGENT 12 | AGENT |
| L3-11, L5-5 | Régénération des cartes seule, éditeur seul, `-Multiprocess`, mesure de perf, jamais mêlée à un réglage de budget | tâche dédiée dans le tableau des agents ; AGENT 13 | AGENT + SESSION |
| L3-7, L1-4 | Veilleur : état déjà vu persisté, silencieux si rien n'a changé | ignorer le premier tour | SESSION |
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
| Q11 | ~6 essais de 3 min par jour au plus ; le reste « prêt mais non donné » | compte des `[À ESSAYER]` | SESSION |
| Q12, L1-6 | Réponses balisées et identifiées : `E1 OK`, `Q2 A`, `BUG:`, `REGLAGE:`, `A trancher` | la session principale enregistre ; `Close` signale une réponse non traitée dans `Retours\` | GATE |
| L4-11 | Plusieurs cartes/variantes = un scénario de revue (Entrée = suivante) | AGENT 18 | AGENT |

## Fusion et fin de séance
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L3 / T2 | Fusion groupée : toutes les branches prêtes, UN build `-Priority merge`, UN `quick_check`, avance rapide de `main` | énoncé de fusion de `Auto_Journey.md` | SESSION |
| L1-3 | Les agents ont fusionné `main` avant le rapport : l'agent de fusion ne rejoue que `tests,aiserver,aiduel` | `RAPPORT merge_main_fait` | RAPPORT |
| L1-13 | Test rouge vérifiable par l'ordre des commits | `RAPPORT TEST_ROUGE` : SHA du test antérieur au correctif | RAPPORT |

## Leçons du 2026-11-06 (séance d'1 h du 2026-11-05 après-midi) : contrôles ajoutés
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L6-2 | La session principale ne lit jamais la file en entier (42k tokens) : l'étape 0 rend un extrait | consigne de l'étape 0 ; ligne de nettoyage de la file | SESSION |
| L6-4 | Un bug rapporté se rejoue par le scénario et le geste exacts de l'utilisateur ; test « sur le geste réel » = scénario `click` | ligne `[ ]` « Scénario click » ; question de repro posée AVANT de lancer l'agent | SESSION + AGENT |
| L6-6 | Mesures répétées sur un binaire copié, jamais sur celui que `build.ps1` doit écraser | ligne `[ ]` des mesures | AGENT |

## Outils encore à construire (lignes `[ ]` de `Auto_Journey_File.md`)
- L1-12 (a) capture en séquence au pas de simulation (prouver une animation) ; (b) détection par le pré-vol d'un panneau qui en cache un autre ; (c) `check.ps1` qui échoue quand l'audit `Sanity` échoue.
- L3-3 : journaux nommés de `quick_check` (`net`, `aiserver`...) sans `Get-FreshLog` partout.
- Un script `launch_agent` qui lit `Launch`, écrit la ligne du tableau des agents et génère l'énoncé (heure d'arrêt, rapport fixe) : supprimerait le reste de dépendance à ma mémoire.

## Leçons du 2026-11-05 soir (séance autonome 19:12-21:38) : contrôles ajoutés
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L7-5 | Test rouge non observé quand un symbole manque : le test ne compile pas | l'énoncé d'un agent C++ impose un stub qui compile + test AVANT le premier build (rouge observé), budget de deux builds, et le champ TEST_ROUGE donne « observé » ou « supposé » | AGENT + RAPPORT |
| L7-7 | Fusion anticipée (20:00 au lieu de 21:35) : a laissé du temps à deux intégrateurs qui ont trouvé 4 défauts invisibles aux auteurs | première fusion groupée dès que 3 branches sont vertes, pas à « fin - 30 min » ; intégrateur nommé AU LANCEMENT | SESSION |


## Leçons du 2026-11-06 (séance interactive 10:53-13:00, cartes exclues) : contrôles ajoutés
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L8-2 | Un blocage en mêlée (soldat en idle ou début d'animation d'attaque en boucle devant sa cible) n'est détecté par aucun test : l'audit Sanity ne voit que les rotations | audit « cible à portée, aucune attaque depuis N s » dans `hol.Test.Sanity` ; devient le test rouge de `BUG-melee-stuck-attack` | AGENT C++ |
| L8-4 | Une réponse d'essai sonore ambiguë (« UI.Open F, UI.Open A » = en fait Open puis Close) a coûté une question de plus | `E-audio_compare.md` porte d'avance une ligne `Ui.Open = _` par cue ; le panneau d'écoute affiche le nom du cue à côté de la lettre | AGENT 18 |
| L8-8 | Le build de fusion (11 à 18 min) est le chemin critique ; la fusion 3 est partie à 12:30 pour finir à 12:47 | lancer la fusion C++ dès que la branche C++ est prête (avant les autres) et fusionner en second tour les branches script-only sans rebuild (comme `heal-sleep` en fusion 3) | SESSION |

## Leçons du 2026-11-07 après-midi (séance 12:10 -> 16:10, 29 agents, 2 fusions) : contrôles à ajouter
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L9-1 | 3 agents (Glorfindel-15/16, Celeborn-18) ont re-notifié leur rapport en boucle (minuteur de réveil / Monitor propre à l'agent) et ont consommé des appels | l'énoncé interdit « minuteur, cron, Monitor » ; la session principale fait TaskStop à la 2e notification identique | AGENT + SESSION |
| L9-4 | Fins de ligne mêlées : conflit TOTAL à chaque fusion (3 fois), 2 lignes perdues | CRLF partout (arbre de travail en CRLF par `.gitattributes` `* text=auto eol=crlf`, index en LF, AUCUN BOM) ; `session_gate.ps1 -Phase Close` : une ligne WARN par worktree et pour la copie principale qui nomme les fichiers mêlés et la commande `eol_check.ps1 -Fix` (**FAIT 2026-11-12**) ; ligne de statut par agent dans un fichier à part (comme Retours\) plutôt qu'une ligne du fichier partagé | OUTIL + AGENT |
| L9-6 | La fusion 1 a pris 55 min et 99 appels : les tips de 3 branches ont bougé pendant la fusion (net-playtest en 3 temps) | figer les branches à fusionner (aucun agent actif dessus) AVANT de lancer l'agent de fusion ; énoncé avec tips relevés à la minute (`git rev-parse <branche>` au lancement, jamais des SHA recopiés d'un rapport : L7-2) | SESSION |
| L9-8 | Haldir (3 agents), Sauron, Arwen, Elrond sans tâche utile de 14:20 à 16:10 : la file « sans C++ » était vide et aucun chapitre du GDD n'est libre (tous ont un *InProgress.md) | l'étape 0 liste les chapitres GDD libres (sans *InProgress.md) ; en fin de séance précédente préparer 3 lignes sans C++ par worktree ; réserver un créneau MACHINE CALME en début de séance pour les mesures S1/S2 (jamais possible en cours de séance) | SESSION + ÉTAPE 0 |
| L9-9 | Mode autonome choisi alors que l'utilisateur était présent : ses réponses (Q-melee-stall A, sites, panneau B) n'ont pas été appliquées en séance | si l'utilisateur répond pendant une séance, proposer de passer en mode interactif (réponse = ligne de tête + relance de l'agent du worktree) | SESSION |
| L9-10 | Essais sonores : aucun agent n'entend rien ; 12 écoutes réparties en 12 scénarios | un seul panneau d'écoute (touche = cue suivant, lettre = vote, résumé écrit à la fin) ; statut LOGIQUE SEULEMENT à vie pour les agents | AGENT 18 |

## Lecons du 2026-11-08 (seance interactive 10:25-13:40) : controles ajoutes
| ID | Regle | Controle | Ou |
|---|---|---|---|
| L10-2 | net rouge 15/15 sur 2 branches puis sur main : le premier reflexe etait de suspecter les branches | au premier net rouge apres un net vert du jour : relancer net sur main SEUL (worktree propre) avant de toucher aux branches | SESSION |
| L10-3 | 'sans C++' ne dit pas que le binaire est a jour : la fusion 5 a du rebuild (BuildingSelected inexistant) | l'agent de fusion lit session_gate -Phase Report (binaire plus recent que le dernier commit de code) avant son quick_check | AGENT + GATE |
| L10-4 | Edit echoue sur le C++ en CRLF : 5 agents ont perdu 3 a 5 appels | l'enonce d'un agent C++ donne la methode (remplacement PowerShell sur chaine exacte) | AGENT |

## Lecons du 2026-11-10 (seance autonome 09:57-10:55, 1 h, 15 agents, 3 fusions) : controles a ajouter
| ID | Regle | Controle | Ou |
|---|---|---|---|
| L11-3 | Un agent C++ de 30 min sur un bug deja corrige (infobulles : l'essai du 09-11 tournait sur un binaire d'avant d813698) | un bug rapporte commence par un verdict de 5 min sur main a jour, sans build ; l'agent C++ part seulement si le verdict est rouge | SESSION + AGENT |
| L11-4 | ~20 lignes sur 58 de la file etaient perimees ; les lignes faites n'etaient pas cochees (les agents n'editent pas le fichier) | la session principale coche en un passage a la fin, `session_gate -Phase Close` echoue si un rapport « fait » n'a pas de ligne cochee (fait a la main le 2026-11-10) | SESSION + GATE |
| L11-8 | Fusion en 3 tours : un message a l'integrateur en cours est arrive apres son build | liste figee des branches ecrite dans le tableau AVANT de lancer l'integrateur, attente jusqu'a une heure fixe, pas de message en cours | SESSION |
| L11-9 | Aucun essai donne ni integrateur de recette nomme ; ESSAIS-A-JOUER.md date de la veille | integrateur de recette nomme au lancement, ou « aucun essai donne » ecrit dans le bilan ; ESSAIS-A-JOUER.md rejoue sur main fusionne avant d'etre donne | SESSION |
| L11-10, L10-5, L9-7 | Mesures de melee a N=8 bruitees (base 3/8 le 09-11, 5/8 le 10-11) ; un format FAIL k/N lu a l'envers | N=16 minimum via ab_matrix.ps1 (PASS k/N) avant de changer un defaut ; lire un tableau de mesure par un agent neuf avant toute conclusion | AGENT |
| L11-11 | Une fiche supprimee apres un OK de l'utilisateur a ete recreee (E-fog_trees.md) | l'agent lit Retours\REPONSES-*.md avant de creer une fiche d'essai | AGENT |
| L11-12 | 15 fiches de Retours\ portent une reponse a reporter, signalees par Close mais jamais purgees | ligne de file « purger Retours » en debut de seance suivante | SESSION |
| L11-13 | Heures annoncees de memoire (decalage de 2 min) | Get-Date avant toute annonce d'heure (deja L5-1, appliquee aux agents, pas a mes messages) | SESSION |

## Lecons du 2026-11-12 (seance a partir de 10:39) : controles a ajouter
| ID | Regle | Controle | Ou |
|---|---|---|---|
| L12-1 | La file de build est le goulot (2026-11-12, `build_queue_report.ps1 -Since "2026-11-12 10:39"`, releve de HALDIR-1112c a 11:08) : 1 seul build fini en 30 min (Glorfindel, 17 min, FAIL), 7 tickets en attente jusqu'a 25 min (dont 2 tickets Elrond), debit ~1 build / 17 min, le dernier de la file attend ~2 h. Cause : DLL des 8 worktrees du 2026-11-11 : chaque PREMIER build du jour recompile la fusion de main ; des agents « sans C++ » compilent aussi (scenarios a jouer apres `git merge main`) : le plafond de 3 agents C++ ne borne pas la file (releve detaille archive le 2026-11-13) | PROPOSE, `build.ps1` inchange : (a) au debut de seance, `sync_worktrees.ps1` SANS `-NoBuild`, en serie, machine calme, AVANT le premier lancement ; `session_gate -Phase Launch` FAIL (et non WARN) au premier lancement si un worktree a plus de N = 5 commits `Source` de retard ou une DLL plus vieille que le dernier commit `Source` de main (le calcul existe deja : `Say-StaleBySource`) ; (b) plafond EFFECTIF : `session_gate -Phase Launch` compte les tickets en attente + compilant (pas seulement les agents declares C++) et dit FAIL a plus de 3 tickets pour tout agent qui va compiler ou jouer un scenario ; (c) `build.ps1 -Wait` : un 2e ticket du meme worktree derriere un ticket en attente = WARN (Elrond 10:43 + 11:00) ; (d) l'enonce de chaque agent qui compile donne l'attente estimee (tickets devant x duree mediane du jour) pour qu'il prepare le travail sans C++ pendant l'attente au lieu de sonder | OUTIL + SESSION |
| L12-1b | Complement (diagnostic HALDIR-1112d, 11:28) : la lenteur ne vient PAS du volume a compiler mais de la machine (Glorfindel : 18 actions comme Haldir la veille, 1006 s au lieu de 99 a 134 s ; UBA tue des processus « Low on memory » ; memoire virtuelle libre 549 Mo ; ProjectShiva, autre moteur invisible de `build.ps1`, compile 5 worktrees a la fois avec 2 editeurs et deux jeux ouverts). FAIL de Glorfindel = code, pas machine (fusion incomplete du revenu : `GetIncomePerMinute`, `GetGoldIncome`) (preuves detaillees archivees le 2026-11-13) | (1) seance : demander a l'utilisateur de mettre ProjectShiva en pause (ou de fermer les jeux) tant que la file HoL tourne ; sinon compter 10 a 20 min par build, soit 1 h 30 a 2 h pour les 6 en attente. (2) `build.ps1`, juste avant `$StartedAt = Get-Date` (ligne 288) : relever `Win32_OperatingSystem.FreeVirtualMemory` et les `cl.exe`/UBT HORS `$EngineRoot` ; ecrire `build : machine chargee (N compilations d'un autre moteur, M Go virtuels libres)` et ajouter une colonne `load` a `BuildQueue.csv` ; attendre (poll) tant que memoire virtuelle libre < 8 Go. (3) controle : `session_gate -Phase Launch` WARN si un cl.exe d'un autre moteur tourne ou si la memoire virtuelle libre < 8 Go ; `build_queue_report.ps1` signale toute duree > 3 x la mediane de la veille pour le meme nombre d'actions | OUTIL + UTILISATEUR |
| L12-2 | sync_worktrees lance avec -NoBuild au debut : DLL du 2026-11-11 partout, chaque premier build du jour recompile la fusion de main | `sync_worktrees.ps1` SANS `-NoBuild` avant le premier lancement ; sinon `session_gate -Phase Launch` FAIL si un worktree a plus de 5 commits `Source` de retard sur sa dll | GATE + SESSION |
| L12-3 | `build.ps1 -Wait` passe en arriere-plan apres 180 s : 5 agents bloques, arret dur 12:30 depasse (messages d'arret a 12:38) | l'enonce donne du travail sans binaire pendant l'attente et un delai maximal d'attente (`build.ps1 -MaxWaitMin`) apres lequel l'agent commite en WIP et rend la main | OUTIL + AGENT |
| L12-4 | Des agents C++ ont ecrit du code avant de savoir si leur binaire demarrait | premier geste d'un agent C++ : `LaunchCheck` ; champ de rapport `LAUNCHCHECK_DEBUT` | AGENT + RAPPORT |
| L12-5 | Recette 12:26 : tests de regles pures en partie triviaux (30/45/60 s jamais assertis, identite) | au moins un test par regle qui passe par une vraie chaine ; champ de rapport `TESTS_TAUTOLOGIQUES : 0`, compte par la recette | AGENT + RAPPORT |
| L12-6 | Recette 12:26 : index de Retours perimes, EtapeDefini disait encore « pas dans main » apres la fusion | l'agent de docs met a jour les index (Retours, LISEZ-MOI, EtapeDefini « dans main ») dans le meme commit que la fusion ; gate Close WARN si EtapeDefini dit « pas dans main » pour une branche ancetre de main | AGENT + GATE |
| L12-7 | 1er build de 17 min a 10:56, diagnostic lance a 11:28, utilisateur prevenu a 11:33 : 37 min perdues alors que la cause (machine saturee par un autre projet) dependait de lui | un build qui depasse 3 fois sa duree habituelle (mediane de BuildQueue.csv pour le meme nombre d actions) fait ecrire a build.ps1 « BUILD LENT » et la session principale lance le diagnostic de charge et previent l utilisateur dans les 5 min ; le veilleur le verifie a chaque passage | OUTIL + PRINCIPALE |
| L13-1 | 2026-11-13 11:12 : 6 agents sur 8 coupes par des 403 Cloudflare ; la reprise avec 3 puis 5 agents simultanes n a plus rien donne | un lancement de reprise commence par 2-3 agents et monte par paliers ; le bilan note le nombre d agents simultanes au moment d une erreur d API | PRINCIPALE |
| L13-2 | `LaunchCheck` compare la dll d un module aux sources de tous les modules (faux « perime » chez Galadriel) ; `session_gate -Phase Launch` compte 5 agents C++ d apres le texte des `IN_USE.txt` (faux FAIL a 13:00) | un test par module dans `launch_check_test.ps1` (rouge d abord) ; le plafond C++ lit les tickets vivants de `Docs\BuildQueue.state`, pas du texte libre | OUTIL |
| L13-3 | Fast-forward de main refuse a 13:39 : `Auto_Journey_File.md` non commite dans la copie principale et touche par 3 branches ; deblocage manuel par la session principale | avant l agent de fusion, la session principale commite les docs de seance (par nom de fichier) ; `preflight_merge.ps1` signale tout fichier modifie des deux cotes | PRINCIPALE + OUTIL |
| L13-4 | `quick_check -Parts net` rouge 3/3 chez Elrond et 15/15 chez Arwen sous charge (Maj relache avant « & »), vert 17/17 a la fusion | un net rouge « valeurs vides » est refait une fois machine calme avant d etre note REGRESSION ; le maintien de Maj (27d91a4) est verifie par un rejeu net a la fusion | OUTIL + AGENT |
| L13-5 | Preflight en FAIL pour 294 fichiers `Content/Audio/Candidates/` non suivis (Arwen) : 3 essais bloques, personne ne decide | le gate Close liste les dossiers non suivis de plus de 50 fichiers et la question « commiter ou exclure » part dans Retours des qu elle bloque un preflight | GATE + PRINCIPALE |
| L13-6 | Scenarios ecrits sans verifier que l objet est visible : `spear_arm` (camera min 15 m, Lancier ~70 px), `neutrals_minimap` (carre minuscule), `deploy` (`waitlog D=Pack` jamais emis) | le `.look` declare la taille en pixels attendue et le texte exact du journal attendu, verifies contre le code avant le premier preflight | AGENT |
| L13-7 | 38 variantes balayees sans verifier la chaine : un dict partage entre deux cles (deepcopy) faussait toutes les mesures | apres 2 balayages sans vert, verifier les donnees d entree (valeurs resolues par cle) avant un 3e (complete la regle 18) | AGENT |
| L13-8 | 4 agents arretes a 45 appels avant d avoir ouvert les PNG et ecrit la fiche d essai | l enonce reserve les 8 derniers appels a PNG + fiche E + journal ; sinon l agent le dit dans NON VERIFIE | AGENT |

## Axes d'amelioration de la seance du 2026-11-13 soir (16:10 -> minuit, autonome, utilisateur absent des 19:50)
| ID | Constat (preuve) | Regle / controle a mettre en place | Ou |
|---|---|---|---|
| L14-1 | Presque tous les agents depassent 45 appels (46 a 90 ; Glorfindel 48 sans build, Galadriel ~90) : attentes de build en arriere-plan, et 3 a 6 appels perdus par agent sur des Edit/Write refuses « fichier non lu » | l'enonce ordonne UN premier appel groupe qui lit (Read) tous les fichiers a editer ; les fiches `E-*.md`, EtapeDefini et la file s'editent par un outil de ligne (`Tools\AutoJourney\edit_line.ps1`, a ecrire) au lieu de PowerShell libre ; le rapport donne le compte VU, pas declare | AGENT + OUTIL |
| L14-2 | La file de build a ete le goulot de 16:30 a 18:20 (builds de 16 a 80 min, machine saturee par un autre projet) puis ne l'a plus ete (builds de 20 a 90 s, vm libre 27 Go) : le plafond fixe de 3 agents C++ et le lancement par vagues n'ont pas ete adaptes a la charge | le plafond C++ se calcule sur `vm libre` et le temps median de la file (`BuildQueue.csv`, « BUILD LENT ») : 1 sous 8 Go, 3 normal, 5 si la file est vide depuis 30 min ; l'etape 0 mesure la charge en premier | PRINCIPALE + GATE |
| L14-3 | Une AUTRE session de l'utilisateur travaillait dans le worktree Arwen (branche d751-root, « Faction choice », commits 19:11-19:17 non lies a la seance) : `sync_worktrees` y a fusionne main (19:30) et la recette 1 a conclu a tort que le correctif du carre n'etait pas dans main | `sync_worktrees` et `status.ps1` sautent et signalent un worktree dont un commit a moins de 2 h n'est pas de la seance (liste `Agents du <date>`) ; une recette verifie `git merge-base --is-ancestor <sha du correctif> main` avant d'ecrire « absent de main » | OUTIL + AGENT |
| L14-4 | Questions posees par `AskUserQuestion` avec options et recommandation (utilisateur present 17:00-19:50) : 10 questions fermees en 30 min ; reponses ambigues (« tres bien » apres un GIF, « OK » apres un « a trancher », « fais un gif ») | une reponse ambigue est ecrite telle quelle (`non confirmee`), jamais comptee comme decision, et reposee UNE fois avec un exemple ; une demande de GIF/planche = tache de l'agent concerne, la question reste ouverte | SESSION |
| L14-5 | Presque tous les essais prets ont un geste INTERNE (`hol.UI.Choose`, `hol.Select.Grid`, `hol.HUD.Click`) : seul `deploy` (touche D + clic reel) est reel ; les recettes les passent PASS « sur l'affichage » (Q5/L5-4 non remplie) | lot dedie « gestes reels » : un scenario par essai sur le modele `neutrals_qground`/`deploy` (vraie touche, vrai clic) ; le champ `GESTE:` de la fiche est verifie par le gate Essay ; exception documentee pour les essais d'ecoute (L7-6) | AGENT + GATE |
| L14-6 | Le test « rouge » n'est vu rouge a l'execution que sur la moitie des lots C++ : 72ee4c4, 57d9f3e, b85afbb, a6d850c, 924fb96 (par mutation) ... « ne compile pas » ou mutation temporaire tient lieu de rouge | stub qui compile + test AVANT le premier build (L7-5, non appliquee) ; champ `TEST_ROUGE: observe / suppose` obligatoire et `Report` du gate refuse « suppose » sans justification ; une mutation temporaire est ecrite comme telle | AGENT + GATE |
| L14-7 | `LaunchCheck` en debut de lot oublie par ~8 agents C++ (L12-4 non appliquee) ; binaire perime non detecte avant le build | `Tools\build.ps1` lance `Get-StaleBinaryProblem` lui-meme au debut et l'imprime (plus de consigne a retenir) | OUTIL |
| L14-8 | Les scenarios attendent par `frames N` : en fenetre cachee ~150 images/s, `frames 600` dure 4 s (a633b97, puis legions_warrior encore le 19:30) ; les `waitlog` places apres un autre pas expirent | `check.ps1`/`no_window_test` refusent `frames` > 30 hors prise de camera et signalent un `waitlog` apres un `wait` de plus de 10 s ; le modele de scenario donne `wait <s>` | OUTIL |
| L14-9 | Defauts d'affichage communs a 3 essais (recette 3) : titre « Scenario ... en preparation » replie sur 2 lignes et en francais en EN/JA ; icones de talents 13-18 px, infobulle ~6 px, « Nourriture » colle a sa case ; Nomade/Regicide encore grises « pas encore dans le jeu » | ligne `[ ]` de tete de file (Q8) ; la verification 1080p devient un point obligatoire du `.look` (capture 1920x1080 en plus du 1280x720) | AGENT + SESSION |
| L14-10 | Les essais audio dependent de 140 fichiers hors git (`Content/Audio/Candidates/`, exclus avec l'accord de l'utilisateur) : le preflight echoue dans tout worktree qui ne les a pas ; copie manuelle dans Faramir | le dossier des candidats est liste dans la fiche d'essai ET dans `LISEZ-MOI.md` ; `play.ps1` signale « candidats absents » avec la commande de copie | AGENT + OUTIL |
| L14-11 | Fusions rapides quand tous les agents ont fini : n°2 (21:17-21:24) et n°4a (22:26-22:33) en 7 a 15 min (build de 20-90 s, file vide) ; l'agent de fusion avait 24 a 29 appels | confirme L7-7 / L8-8 : lancer la fusion des que 3 branches sont vertes, pas a une heure fixe ; une avance rapide d'une branche deja testee sur son propre binaire (main = ancetre) n'a pas besoin d'un nouvel agent (`preflight_merge` puis `merge --ff-only`) | SESSION |
| L14-12 | Fiches `Q-*.md` : 7 des 11 de la soiree ont ete archivees par l'agent de docs, mais l'archive de l'agent a aussi commite `Auto_Journey_Jour.md` et les fiches E en cours d'edition par la recette (`commit_docs.ps1`) | `commit_docs.ps1 -Only <fichiers>` ; la session principale annonce « docs gelees » avant le nettoyage de fin | OUTIL + SESSION |
| L14-13 | Session prolongee deux fois (23:00 puis minuit) : les consignes d'agents, `SessionEnd.txt` (WARN « pas d'aujourd'hui ») et les alarmes ont du etre refaits a la main a chaque fois | `Docs\SessionEnd.txt` accepte une date ; une commande `extend_session.ps1 <HH:MM>` met a jour le fichier, les heures du `brief_commun.md` et rappelle de recreer les alarmes | OUTIL |
