# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11), `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_soir_archive_2026-11-14.md` (séance autonome du 2026-11-13 soir : bilan, leçons, tableau d'agents ; résumés du 2026-11-13 matin et du 2026-11-12) et `Docs\Archive\Auto_Journey_Jour_2026-11-14_archive_2026-11-15.md` (séance autonome du 2026-11-14 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-15_archive_2026-11-16.md` (séance autonome du 2026-11-15 : tableau d'agents, fusions n°1-7, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-16_apres-midi_archive_2026-11-16.md` (séance interactive du 2026-11-16 après-midi : tableau d'agents, lignes d'exception, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_matin_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 matin : tableau d'agents, bilan, leçons, plan de lancement ; résumé dans ce fichier) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Chapitres du GDD non commencés

**Relevé du 2026-11-15** : voir Docs\ReleveGDD_2026-11-15.md (état réel de A1-A4 et B1-B6, branchements C++ à faire, questions en attente). L'ancien relevé du 2026-11-12 est archivé dans Docs\Archive\Auto_Journey_Jour_Chapitres_GDD_releve_2026-11-12_archive_2026-11-15.md.

## Agents du 2026-11-20

SÉANCE DE FONCTIONS DU 2026-11-20 APRÈS-MIDI (« lance une autojourney », 15:32 -> 18:00, INTERACTIF). Réponses de l'utilisateur à 15:3x (interface de choix) : fin 18:00 (`Docs\SessionEnd.txt`) ; fusion dans main EN LOCAL si build PASS et tests verts sur le résultat exact, jamais de push ; mode interactif (veilleur 5 min, alarme de clôture 17:45) ; nettoyage 0a léger (un agent). check-list lue (L19) à 15:35, en entier, par la session principale, avant tout lancement d'agent de travail. État au lancement : main = a831674 (poussé), 8 worktrees à jour et compilés (rattrapage 14:30-15:31), tous IN_USE libres. Arwen = intégrateur (worktree d'intégration, ne code pas ; builds, tests, fusion par la session principale).

| Agent | id | worktree | branche | tâche | C++ | lancé |
|---|---|---|---|---|---|---|
| NETTOYAGE-1120pm | a16f9e6f | MyCrusader (docs) | main | étape 0a légère : archive du matin, 4 lignes de file [x] ; FINI 15:36, commit 217b0ad | non | 15:34 |
| ETAPE0-1120pm | a3551754 | MyCrusader (lecture seule) | main | étape 0 : plan de lancement ; FINI 15:40, rapport `Docs\Rapports\ETAPE0-1120pm.md` (vm libre 28,7 Gio, integrate.ps1 absent) | non | 15:36 |
| CELEBORN-1120pa | ac3aaa5c | HeroesOfLiberty-Celeborn | duel-rewards | IA Nomade : `HelpersFor` dans `HOLAINomadThink.as` | non | 15:40 |
| ELROND-1120pa | a13d9319 | HeroesOfLiberty-Elrond | integ-25 | D869 : montage/démontage visibles 1,5 s / 1 s, capture « pendant » | oui | 15:40 |
| FARAMIR-1120pa | a420ae28 | HeroesOfLiberty-Faramir | zone-effect | D862 : Sceau par les capacités du Moine | non | 15:40 |
| GALADRIEL-1120pa | aa82426a | HeroesOfLiberty-Galadriel | specialization-trial | saisie du chat branchée dans l'écran de jeu | non | 15:40 |
| GLORFINDEL-1120pa | a47b7730 | HeroesOfLiberty-Glorfindel | ability-columns | D870 : marque du signal ÷ 3 | oui | 15:40 |
| HALDIR-1120pa | addf2a1b | HeroesOfLiberty-Haldir | wall-line | D867 : murs sans plafond de longueur | non | 15:40 |
| SAURON-1120pa | a26d21f3 | HeroesOfLiberty-Sauron | tool-known-reds | D864 + D865 : colonnes `FHOLUnitTypeConstants` (en-tête central, un seul changement) | oui | 15:40 |
| (vague 1 finie) | - | tous | tous | 15:43-15:46 : Celeborn 6c1fe92/e6d984b, Elrond 5e6ff7c/0abe6ca (Q-deploy-visible), Faramir 9b68d8a/c77b90d, Galadriel RIEN (24 appels de lecture, fiche de reprise), Glorfindel 3e14dcb/942996d, Haldir 70d8fa7/de8ad58, Sauron c919d09/f1aa981 (Q-chevalier-ligne) | oui | 15:40 |
| vague 2 | a8f5b6b9..ae5c8fc7 | Galadriel, Glorfindel, Haldir, Faramir, Elrond, Celeborn | tous | 15:45-15:50 FINIE : GALADRIEL-1120pb chat branché (9cb9321/aed7c7b), GLORFINDEL-1120pb D868 en script (7b061b3/4b771a1), HALDIR-1120pb `integrate.ps1` (2e29f58/75d7371, DryRun 22/22), FARAMIR-1120pb alertes mineures (f311fd1/9173e34), ELROND-1120pb cadre de garnison script (588d136/d8a35b9, Q-garrison-portrait), CELEBORN-1120pb lieu des ordres de moral 6a (95cf44a/7d0511b/1aaab39) | non | 15:45 |
| réponses 15:5x | - | - | - | interface de choix : D869 affichage = A+B puis C ; chat privé = A+B ; Chevalier = A | - | 15:51 |
| vague 3 | ae911008..a58432ea | MyCrusader (docs), Elrond, Galadriel, Sauron, Glorfindel | tous | DOCS-1120pa (D871-D873, fiches archivées), ELROND-1120pc (D869 texte + barre), GALADRIEL-1120pc (chat privé clic + /w), SAURON-1120pb (ligne VirtuousKnight), GLORFINDEL-1120pc (lot C++ garnison) | 2 C++ | 15:52 |
| (vague 3 finie) | - | docs, Elrond, Galadriel, Sauron, Glorfindel | tous | 15:54-15:56 : DOCS-1120pa 78441a7 (D871-D874), ELROND-1120pc D869 texte + barre e9ea0f5/8fb9db2 (C++ dont HOLPlayerController.h), GALADRIEL-1120pc chat privé cfdb3b7/9e30982/2be72e9 (39 appels), SAURON-1120pb VirtuousKnight d236755/889a1cc (rouge python vu 15:53 ; UnitTypes + BuildingTypes .uasset à régénérer), GLORFINDEL-1120pc lot C++ garnison 14b8786/65ebacd (46 appels) | oui | 15:52 |
| vague 4 | a5acabf8..a9b0c5c4 | Haldir, Faramir, Celeborn, Galadriel, Glorfindel, Elrond, Sauron | tous | HALDIR-1120pc dessin du glisser, FARAMIR-1120pc nettoyage + ligne suivante, CELEBORN-1120pc moral suite, GALADRIEL-1120pd ligne Diplomatie cliquable + scénario chat_input, GLORFINDEL-1120pd D868 en jeu + nom 3e voie (C++), ELROND-1120pd branchement garnison (dépend d'ability-columns), SAURON-1120pc `Redirect` dans HOLCombatProcessor (C++) | 2-3 C++ | 15:53-15:57 |
| vagues 5-6 (finies 16:00-16:12) | ... | tous | tous | FARAMIR-1120pc (Q-decide-jauge, Q-chevalier-defi-ligne : réponses A/A ~15:59), CELEBORN-1120pc/pd/pe (étape 1 python rouge vu 15:56 ; étape 2 C++ ; pont `UHOLMoraleLibrary`, doublon script supprimé 96a2479/37a91ed), HALDIR-1120pc/pd/pe (dessin du glisser 2b779a4/dfaf618/85184e3 ; edit_line rouge vu 16:01 f80bc99/7365bf7 ; clairière d5cc2c3/74990c6), GALADRIEL-1120pd/pe/pf (ligne Diplomatie 8ab3d8a/0301e05, scénario chat_input ; hol.UI.Type 74745b8/e53ee06), GLORFINDEL-1120pd/pe (D868 en jeu c54698d/ec839e3, GroundMark 0758482 ; Sortir tous 58e69f7/d3fcb3e, Q-garrison-cell = F ~16:10), ELROND-1120pd/pe (garnison branchée f4ca160/56c4496 ; deploy wait 0.3 737a45a/5e5ff34), SAURON-1120pc/pd (Redirect f466127/aa4e528 ; commande de régénération des assets), FARAMIR-1120pd/pe (D875 a1135f2, KnightChallenge ecc0b18/6075561 ; scénarios wall_line_long, virtuous_knight 70f7754) | oui | 15:53 |
| vague 7 = DERNIÈRE C++ (L18-c) | a46dbfe0..ae4b8d1e | Sauron, Haldir, Faramir, docs | tous | SAURON-1120pe anneau 8 m (C++), HALDIR-1120pf `hol.Decor.ClearNear Trees` (C++), FARAMIR-1120pf `hol.Mouse.Drag` (C++), DOCS-1120pb D876 | 3 C++ | 16:12 |
| intégrateur = session principale | - | HeroesOfLiberty-Arwen | integ-30 | passe 2 (16:00) : 1929/1938, 8 rouges attendus + régression `Signals.GroundMark` (D870, corrigée en 0758482) ; passe 3 : conflit quick_check.ps1 résolu par ARWEN-1120pa (2d439c9) ; passe 3b : build FAIL (D869 bouchon doublé + `case` sans accolades), corrigé par ARWEN-1120pb (7a00ce2), rebuild 16:12 | oui | 16:00 |
| vague 8 (corrections) | ... | Glorfindel, Arwen | tous | GLORFINDEL-1120pf type d'un occupant de garnison (a680f3f, bug prouvé par DIAG-1120pa) ; ARWEN-1120pa..pi : conflits de journaux (2d439c9, 8453dbb, 032fad6, 7c4bf0d, 8fe0745), erreurs de compilation (7a00ce2 D869, 1dfe929 script DuelRewards, 9b189f2 .generated.h, 2f144ce GetSlotFaction), attente d'infobulle (9dc5e75) | oui | 16:01-16:41 |
| intégrateur = session principale (fin) | - | HeroesOfLiberty-Arwen | integ-30 | passe 3d 1945/1955 ; passe 4c (tous les verts, ffaef13) build PASS 128 s, 1967/1971 (4 rouges = assets) ; assets régénérés ffff449 ; **tests 1971/1971 (16:32)** ; net,build : net PASS, build hud FAIL (regex), corrigé, **build PASS 16:43** ; gate Merge sans FAIL ; **main = 9dc5e75 (16:44, avance rapide LOCALE, rien poussé)** ; rouges de la dernière vague (74745b8, 58e69f7, 96a2479, ea59204, b1ccdeb, b74710e) NON vus séparément (L19-8) | oui | 16:44 |
| RECETTE-1120pa | aeac09b7 | HeroesOfLiberty-Arwen | integ-30 = main | pré-vols deploy, chat_input, neutrals_clairiere, virtuous_knight ; fiches E si PASS + PNG vus | non | 16:45 |
| sync de fin | - | 7 worktrees | - | `sync_worktrees.ps1` SANS -NoBuild (L15-1) lancé 16:45 | oui | 16:45 |
| intégrateur = session principale (début) | - | HeroesOfLiberty-Arwen | integ-30 | passe 1 : rouges vague 1 (6 sha), build PASS 181 s, tests 1919/1930 = les 11 rouges attendus, nom par nom (15:53) ; passe 2 (15:54) : verts vague 1 + rouges vague 2 (9cb9321, 7b061b3, 75d7371, f311fd1, 588d136, 95cf44a, 8820b8a), 9 rouges attendus | oui | 15:46 | lot C++ unique (D869, D870, D864/65) + vagues script : rouges, build, tests, verts, `net,build`, fusion locale si vert | oui | - |

Plan de lancement (étape 0, 15:40) : 7 agents d'emblée, 3 C++ (Elrond, Glorfindel, Sauron = plafond) ; gate Launch sans FAIL sur les 7 (15:38-15:39). Vague 2 prévue : D868 (`HOLCommandGrid.cpp:63`), alertes mineures (Galadriel, script), garnison et dessin du glisser (C++), `integrate.ps1` (L19-9) ; dernier lancement C++ ~17:00 (L18-c). Exception : Palier 3 G1 (file l.111) non lancé, prévenir l'utilisateur avant.

### Bilan de la séance du 2026-11-20 après-midi (15:32 -> 18:00, interactif)

- **État à la fin** : main = fd41600, LOCAL, rien poussé (avances de main 16:44 = 9dc5e75, 17:04 = 2868866, docs ensuite, puis fd41600 pour amener Faramir à main). Session principale + ~45 agents Sonnet en 8 vagues ; intégration integ-30 (Arwen) en 4 passes : passe 1 = 11 rouges attendus, nom par nom (15:53) ; passe 2 = 8 attendus + 1 régression `Signals.GroundMark` (corrigée 0758482) ; passes 3-4 = 2 conflits de journaux (AubeInProgress, FortificationsInProgress, Palier3InProgress) résolus par agents. Tests 1971/1971 (16:32) puis 1972/1972 (17:03) ; `net` PASS ; `build` PASS après correction d'une regex d'infobulle (9dc5e75). Assets UnitTypes/BuildingTypes régénérés (ffff449).
- **Entré dans main, visible** : D869 montage/démontage de l'engin avec texte et barre (partiel : voir « non vérifié »), chat privé cliquable + `/w`, ligne Diplomatie cliquable, Chevalier Vertueux dans la Caserne (D873, libellés vus en recette), clairière des neutres sans bosquet (`neutrals_clairiere`, VU À L'ÉCRAN).
- **Entré dans main, sans effet visible** : D862 Sceau par les capacités du Moine, D864/D865 colonnes `DamageRedirectPercent` et voisines, D867 murs sans plafond de longueur, D868 en script + en jeu, D870 marque du signal ÷ 3, garnison (cadre, branchement, « Sortir tous »), moral des duels (pont `UHOLMoraleLibrary`), `integrate.ps1`, `edit_line.ps1 -Delete`, `hol.Mouse.Drag`, `hol.Decor.ClearNear Trees`, anneau de 8 m, alertes mineures, saisie du chat branchée.
- **Ce que la séance a trouvé** : 4 erreurs de compilation rendues par des agents sans build (bouchon D869 doublé + `case` sans accolades 7a00ce2 ; paramètre `const` du script DuelRewards 1dfe929 ; include après `.generated.h` 9b189f2 ; `GetSlotFaction` privée 2f144ce) ; 1 vrai bug trouvé par diagnostic (type d'occupant de garnison, a680f3f, DIAG-1120pa) ; conflits de journaux `*InProgress.md` à presque chaque fusion (5 agents de résolution) ; 7 réponses de l'utilisateur par l'interface de choix, enregistrées en minutes (D871 à D876 + Q-decide-jauge). Recette (RECETTE-1120pa 16:44, 1120pb 17:04) : 1 essai donné (`neutrals_clairiere`), `deploy`, `chat_input` et `virtuous_knight` en échec : défauts mis en tête de file (6875503).
- **Non vérifié** : les rouges de la dernière vague (74745b8, 58e69f7, 96a2479, ea59204, b1ccdeb, b74710e, 178bff2) n'ont pas été vus séparément (L19-8) ; D869 : la barre du montage reste à 0 dièse 0,3 s après l'ordre et le 2e cycle de montage reste sur « Montage… » (9469ade seulement partiel) ; chat : la 2e Entrée ne rouvre pas le champ (e3d1969 sans effet, focus: none) ; `virtuous_knight` : `hol.Select.ClickUnit 0` désigne le Paysan, anneau de 8 m et PV du Chevalier non vus ; aucun essai à geste réel joué (neutrals_clairiere = GESTE: interne, SON: non entendu).
- **Gate Close 17:17** : FAIL L19-1 (23 opérations de la session principale au reflog : fusions groupées, avances de main, commits de docs et d'assets ; toutes les corrections de code et diagnostics ont été confiés à des agents), FAIL Faramir pas à main (réglé : main avancé à fd41600), FAIL tableau des leçons absent (ce tableau).
- **Axes d'amélioration validés par l'utilisateur (~17:30)** : L20-1 à L20-9, voir `Auto_Journey_Checklist.md`.

### Lecons appliquees 2026-11-20
| ID | Etat | Preuve |
|---|---|---|
| T2-1 | appliquee | ETAPE0-1120pm (15:36-15:40), plan de lancement écrit sous le tableau des agents, une raison par worktree |
| T2-2 | appliquee | 7 agents d'emblée à 15:40 sur 7 worktrees, Arwen = intégrateur (raison écrite) |
| L3-12 | appliquee | étape 0 faite (rapport `Docs\Rapports\ETAPE0-1120pm.md`), non sautée |
| L5-7 | non appliquee | aucune question sur les dossiers non suivis dans les réponses de 15:3x ; 4 fichiers Docs non suivis encore présents à ce bilan |
| L4-1, L4-3 | appliquee | fin 18:00 dans le titre et `Docs\SessionEnd.txt`, alarme de clôture 17:45 posée (réponses 15:3x) |
| L3-8, L4-9, Q5, L1-14 | appliquee | ligne `intégrateur` à 15:46 (Arwen, integ-30) + RECETTE-1120pa (aeac09b7) ; réserve : l'intégrateur était la session principale (L19-1) |
| L3-9 | non appliquee | aucun point de reprise écrit au Jour ; compte d'appels de la session principale non relevé |
| T2-3 | appliquee | chaque énoncé ouvre par « lis agent_rules.md » + arrêt dur (ex. BILAN-1120pm : 17:40 ou 30 appels) |
| L1-1, L3-6, L4-7, L5-3 | appliquee | arrêts à 24 (GALADRIEL-1120pa), 39 (GALADRIEL-1120pc) et 46 appels (GLORFINDEL-1120pc), fiche de reprise ; aucun dépassement de 80 signalé |
| L5-6 | appliquee | 7 questions posées à l'interface de choix dès leur apparition (D871-D876, Q-decide-jauge), réponses en minutes |
| L3-11, L5-5 | appliquee | aucune régénération de cartes ; assets UnitTypes/BuildingTypes régénérés par commande (ffff449), pas mêlée à un réglage de budget |
| L3-7, L1-4 | appliquee | mode interactif, veilleur 5 min (réponse 15:3x) |
| T2-4 | appliquee | au plus 2 Q par chantier : Elrond 2 (Q-deploy-visible, Q-garrison-portrait), Faramir 2 (Q-decide-jauge, Q-chevalier-defi-ligne), Sauron 1 (Q-chevalier-ligne) |
| Q5, L5-4 | appliquee | RECETTE-1120pa (16:44-16:52) et 1120pb (17:04-17:12) par agents, sur main fusionné, après l'intégration ; aucune commande donnée pour les 3 essais en échec |
| Q8 | appliquee | défauts deploy / chat_input / virtuous_knight en tête de file (6875503) |
| Q11 | appliquee | 1 essai donné (`neutrals_clairiere`), bien sous 6 |
| L4-11 | appliquee | sans objet : aucun essai à variantes multiples cette séance |
| L3 / T2 | appliquee | une intégration integ-30 : build PASS, tests 1971/1971, `net,build`, avance rapide de main 9dc5e75 (16:44) |
| L1-3 | non appliquee | champ `merge_main_fait` non relevé ; integ-30 a rejoué tests complets + `net,build` au lieu de `tests,aiserver,aiduel` seulement |
| L17-a | appliquee | passe 1 : 11 rouges attendus lus nom par nom (15:53) ; passe 2 : régression `Signals.GroundMark` repérée et corrigée (0758482) |
| L17-b | appliquee | archive du matin par UN agent, par section entière (NETTOYAGE-1120pm, 217b0ad) |
| L17-c | non appliquee | `Docs\BuildQueue.state` non cité dans le tableau ; preuve absente |
| L18-a | non appliquee | tables UnitTypes/BuildingTypes régénérées (ffff449) ; aucune relecture `git diff` des fichiers générés notée |
| L18-b | appliquee | aucun conflit de type ni d'espace de noms à l'intégration (les 4 erreurs de compilation sont de syntaxe, d'include ou de visibilité) |
| L18-c | appliquee | dernière vague C++ lancée à 16:12 (avant fin - 60 min = 17:00), main à jour 16:44, sync SANS -NoBuild 16:45 |
| L18-d | appliquee | aucun test de comptage cassé (1971/1971 puis 1972/1972) malgré `hol.Mouse.Drag` et `hol.Decor.ClearNear` ajoutées |
| L18-e | appliquee | passes rouges puis verts : 11 rouges attendus nom par nom (15:53), 8 + 1 à la passe 2, tous les verts à la passe 4c (ffaef13) |
| L18-f | appliquee | scénarios avec `waitlog` ; les 3 échecs de recette ont une cause jeu (deploy, chat) ou index de sélection du scénario (virtuous_knight), pas une relecture trop tôt |
| L19-1 | non appliquee | check-list lue en entier à 15:35 (oui) mais gate Close 17:17 : FAIL, 23 opérations de la session principale au reflog (seuil 3) : fusions groupées, avances de main, commits de docs et d'assets |
| L19-2 | non appliquee | 1 erreur de script rendue par un agent sans build (DuelRewards, paramètre `const`, 1dfe929) + 3 erreurs C++ (7a00ce2, 9b189f2, 2f144ce) ; champ `DIAGNOSTICS_AS` non exigé à la réception |
| L19-3 | appliquee | gate Close : « copie principale propre », aucun fichier de code modifié dans la copie principale |
| L19-4 | non appliquee | l'infobulle de Caserne attendue par `quick_check` (D873) n'a été trouvée qu'au `net,build` après intégration (regex corrigée 9dc5e75) |
| L19-5 | appliquee | fiche `E-neutrals_clairiere.md` écrite au pré-vol PASS ; capture « pendant » du `.look` de `deploy` : a prouvé la barre figée à 0 dièse ; recette indépendante par agents |
| L19-6 | appliquee | questions posées dès leur apparition, réponses en minutes |
| L19-7 | appliquee | étape 0 : vm libre 28,7 Gio avant le lot C++ ; en-tête central D864/D865 changé UNE fois ; builds de 128 à 181 s |
| L19-8 | non appliquee | rouges de la dernière vague non vus séparément (74745b8, 58e69f7, 96a2479, ea59204, b1ccdeb, b74710e, 178bff2) |
| L19-9 | non appliquee | `integrate.ps1` écrit (HALDIR-1120pb 2e29f58, DryRun 22/22) ; usage en intégration réelle non prouvé, passes 1 à 4c conduites par commandes |

Total : 27 appliquees, 10 non appliquees (37 lignes). Les 10 non appliquees : L5-7, L3-9, L1-3, L17-c, L18-a, L19-1, L19-2, L19-4, L19-8, L19-9 ; nouvelles leçons L20-1 à L20-4 dans `Auto_Journey_Checklist.md`.

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
