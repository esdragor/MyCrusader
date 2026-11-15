# Archive : nettoyage d'Auto_Journey.md et d'Auto_Journey_Checklist.md du 2026-11-15 (étape 0a)

Copie telle quelle des lignes retirées le 2026-11-15 (début de séance). Rien n'est supprimé : chaque ligne source est remplacée par un renvoi d'une ligne vers ce fichier. Les règles restent en vigueur ; les identifiants de la check-list (L9-4, L12-6, Q12, L1-6) restent cités dans le paragraphe « Nettoyage du 2026-11-15 » d'`Auto_Journey_Checklist.md` et cochés au bilan comme les autres.

## 1. Auto_Journey.md : redites d'`agent_rules.md`

### « La session principale est le chef d'orchestre » (= `agent_rules.md` règle 9)
- **Lancer un script une fois de bout en bout avant de dire qu'il est prêt** (test 3) : un script jamais exécuté jusqu'au bout n'est pas prêt.
- **Corriger l'outil partagé (`preflight.ps1`, `check.ps1`) au lieu d'en garder une copie privée, et le dire dans le rapport** (test 3) : une copie privée corrigée laisse le défaut au prochain agent.

### « Comment écrire l'énoncé d'un agent » (= `agent_rules.md` règles 7, 8 et 11)
- Test rouge d'abord sur la vraie chaîne ; un changement, une compilation, un passage du test ; `Tools\build.ps1` (file d'attente), `-LiveCoding=0` ; `quick_check -Parts` des seules parties touchées. Un test rouge peut venir du **script de test** et non du jeu : regarder le dernier `Test mark` du journal de jeu avant de toucher au code.

### « Pipeline en bref » (= `agent_rules.md` section « Outils (L14, 2026-11-14) »)
```
Tools\AutoJourney\status.ps1 -InMain <sha> = ce commit est-il DANS main ou ABSENT (merge-base --is-ancestor), avant d'écrire « absent de main »
ForeignSession (status.ps1 « AUTRE SESSION ? », sync_worktrees.ps1 le saute) = worktree d'une autre session : n'y fusionner ni conclure
Tools\AutoJourney\edit_line.ps1 -File <abs> -Match "<texte>" -NewLine "<ligne>" = changer UNE ligne d'un doc de suivi sans Read ni Edit
Tools\AutoJourney\commit_docs.ps1 -Only "<f1>,<f2>" = commiter seulement ces docs de la copie principale
Tools\AutoJourney\extend_session.ps1 <HH:MM> = repousser la fin de séance (Docs\SessionEnd.txt ; heure passée = lendemain)
FrameWaits (check.ps1, no_window_test.ps1 -FrameWaits Warn|Fail, HOL_CHECK_FRAMES=fail) = « frames N » > 30 hors caméra : WARN par défaut, Fail refuse
```

## 2. Auto_Journey_Checklist.md : lignes devenues des contrôles de `session_gate.ps1 -Phase Close`

Preuve : sortie de `session_gate.ps1 -Phase Close` du 2026-11-15 09:04 : « WARN EtapeDefini dit 'pas dans main' mais commits ... deja dans main » (L12-6), « WARN ... fins de ligne melees dans N fichier(s) suivi(s) ... eol_check.ps1 -Fix » (L9-4), « WARN reponses de l'utilisateur encore dans Retours\ » (Q12, L1-6).

Section « Essais » :
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| Q12, L1-6 | Réponses balisées et identifiées : `E1 OK`, `Q2 A`, `BUG:`, `REGLAGE:`, `A trancher` | la session principale enregistre ; `Close` signale une réponse non traitée dans `Retours\` | GATE |

Section « Leçons du 2026-11-07 après-midi » :
| ID | Règle | Contrôle | Où |
|---|---|---|---|
| L9-4 | Fins de ligne mêlées : conflit TOTAL à chaque fusion (3 fois), 2 lignes perdues | CRLF partout (arbre de travail en CRLF par `.gitattributes` `* text=auto eol=crlf`, index en LF, AUCUN BOM) ; `session_gate.ps1 -Phase Close` : une ligne WARN par worktree et pour la copie principale qui nomme les fichiers mêlés et la commande `eol_check.ps1 -Fix` (**FAIT 2026-11-12**) ; ligne de statut par agent dans un fichier à part (comme Retours\) plutôt qu'une ligne du fichier partagé | OUTIL + AGENT |

Section « Lecons du 2026-11-12 » :
| ID | Regle | Controle | Ou |
|---|---|---|---|
| L12-6 | Recette 12:26 : index de Retours perimes, EtapeDefini disait encore « pas dans main » apres la fusion | l'agent de docs met a jour les index (Retours, LISEZ-MOI, EtapeDefini « dans main ») dans le meme commit que la fusion ; gate Close WARN si EtapeDefini dit « pas dans main » pour une branche ancetre de main | AGENT + GATE |

## 3. Auto_Journey_Checklist.md : relevés datés du 2026-11-12 (colonne « Règle » de L12-1 et L12-1b, ligne entière telle qu'avant)

| ID | Regle | Controle | Ou |
|---|---|---|---|
| L12-1 | La file de build est le goulot (2026-11-12, `build_queue_report.ps1 -Since "2026-11-12 10:39"`, releve de HALDIR-1112c a 11:08) : 1 seul build fini en 30 min (Glorfindel, 17 min, FAIL), 7 tickets en attente jusqu'a 25 min (dont 2 tickets Elrond), debit ~1 build / 17 min, le dernier de la file attend ~2 h. Cause : DLL des 8 worktrees du 2026-11-11 : chaque PREMIER build du jour recompile la fusion de main ; des agents « sans C++ » compilent aussi (scenarios a jouer apres `git merge main`) : le plafond de 3 agents C++ ne borne pas la file (releve detaille archive le 2026-11-13) | PROPOSE, `build.ps1` inchange : (a) au debut de seance, `sync_worktrees.ps1` SANS `-NoBuild`, en serie, machine calme, AVANT le premier lancement ; `session_gate -Phase Launch` FAIL (et non WARN) au premier lancement si un worktree a plus de N = 5 commits `Source` de retard ou une DLL plus vieille que le dernier commit `Source` de main (le calcul existe deja : `Say-StaleBySource`) ; (b) plafond EFFECTIF : `session_gate -Phase Launch` compte les tickets en attente + compilant (pas seulement les agents declares C++) et dit FAIL a plus de 3 tickets pour tout agent qui va compiler ou jouer un scenario ; (c) `build.ps1 -Wait` : un 2e ticket du meme worktree derriere un ticket en attente = WARN (Elrond 10:43 + 11:00) ; (d) l'enonce de chaque agent qui compile donne l'attente estimee (tickets devant x duree mediane du jour) pour qu'il prepare le travail sans C++ pendant l'attente au lieu de sonder | OUTIL + SESSION |
| L12-1b | Complement (diagnostic HALDIR-1112d, 11:28) : la lenteur ne vient PAS du volume a compiler mais de la machine (Glorfindel : 18 actions comme Haldir la veille, 1006 s au lieu de 99 a 134 s ; UBA tue des processus « Low on memory » ; memoire virtuelle libre 549 Mo ; ProjectShiva, autre moteur invisible de `build.ps1`, compile 5 worktrees a la fois avec 2 editeurs et deux jeux ouverts). FAIL de Glorfindel = code, pas machine (fusion incomplete du revenu : `GetIncomePerMinute`, `GetGoldIncome`) (preuves detaillees archivees le 2026-11-13) | (1) seance : demander a l'utilisateur de mettre ProjectShiva en pause (ou de fermer les jeux) tant que la file HoL tourne ; sinon compter 10 a 20 min par build, soit 1 h 30 a 2 h pour les 6 en attente. (2) `build.ps1`, juste avant `$StartedAt = Get-Date` (ligne 288) : relever `Win32_OperatingSystem.FreeVirtualMemory` et les `cl.exe`/UBT HORS `$EngineRoot` ; ecrire `build : machine chargee (N compilations d'un autre moteur, M Go virtuels libres)` et ajouter une colonne `load` a `BuildQueue.csv` ; attendre (poll) tant que memoire virtuelle libre < 8 Go. (3) controle : `session_gate -Phase Launch` WARN si un cl.exe d'un autre moteur tourne ou si la memoire virtuelle libre < 8 Go ; `build_queue_report.ps1` signale toute duree > 3 x la mediane de la veille pour le meme nombre d'actions | OUTIL + UTILISATEUR |
