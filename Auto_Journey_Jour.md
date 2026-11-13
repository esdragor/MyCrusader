# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Agents du 2026-11-13

Séance INTERACTIVE 10:16 -> 14:15 (arrêt dur des agents 14:15, +15 min toléré), fusion groupée dans main autorisée, jamais de push. Départ : main = 6d46a74.

| Nom | Id | Worktree | Branche | Tache | Lance |
|---|---|---|---|---|---|
| NETTOYAGE-1113a | a8ad10aa6e4d8cd16 | principale | main | 0a : Jour + Retours (6c9c0f9) | 10:17 |
| NETTOYAGE-1113b | ad993f292d1c4fb12 | principale | main | 0a : File de travail (870f3f8) | 10:17 |
| NETTOYAGE-1113c | a850b77d806a71b21 | principale | main | 0a : dossier Retours (a6f6705) | 10:17 |
| NETTOYAGE-1113d | a672c7ec5ddc9f370 | principale | main | 0a : procédure + checklist (2503fed) | 10:17 |
| PLAN-1113a (Fable) | abf950ecd678b5fcb | principale | main | Étape 0 : plan de lancement (lecture seule) | 10:26 |
| ARWEN-1113a | ac5a153e4e1701eee | Arwen | d751-root | N7 : LaunchCheck, quick_check, commit vert, préviol neutrals_minimap | 10:53 |
| CELEBORN-1113a | ada7c3a013d424aa6 | Celeborn | hero-j8-close | D787 prise de lance B, rebake, scénario spear_arm | 10:53 |
| ELROND-1113a | a68a90030ba3ee8bb | Elrond | tiers | RoundTrip, build (C++ 1), G6 e/G4 vert, préviol deploy | 10:53 |
| FARAMIR-1113a | ae27fcc2364d7358f | Faramir | fog | Préviols legions_roster_gesture/disconnect_1v1 puis outils sans C++ | 10:53 |
| GALADRIEL-1113a | a810faea8318d75cd | Galadriel | galadriel-tab | D791 rage, build (C++ 2), quick_check, préviol legions_warrior | 10:53 |
| GLORFINDEL-1113a | aabcfb15a01b099e9 | Glorfindel | net-playtest | quick_check, D788 infobulle de revenu (C++ 3), A4 Canaux | 10:53 |
| HALDIR-1113a | a4de23c614ef7851b | Haldir | maps | D783-D786 réglages en script, tests à durcir | 10:53 |
| SAURON-1113a | afb127d057ea93f0e | Sauron | visual | quick_check, préviol resources_compare, Placeholders | 10:53 |
| GALADRIEL-1113b | ad03afe28f1c0623c | Galadriel | galadriel-tab | REPRISE après coupure API : D791, build (C++ 1), quick_check, préviol legions_warrior | 11:43 |
| GLORFINDEL-1113b | a17bc1298aaf7e211 | Glorfindel | net-playtest | REPRISE : quick_check, D788 (C++ 2), A4 Canaux | 11:43 |
| ELROND-1113b | a48eb9c3880c64c30 | Elrond | tiers | REPRISE : build (C++ 3), G6 e/G4 vert, préviol deploy | 11:43 |
| (Glorfindel-1113b fini 12:26) | a17bc1298aaf7e211 | Glorfindel | net-playtest | 6dfa7ee D788 vert, 59e8f60 A4 vert, tests 1207/1207, net 16 PASS, a859c73 merge main ; branche prête, pas fusionnée | 12:26 |
| HALDIR-1113b | ae4aa5588546c6d50 | Haldir | maps | tests de délais 30/45/60 s à durcir (sans C++), arrêt 13:10 | 12:28 |
| (Haldir-1113b fini 12:36) | ae4aa5588546c6d50 | Haldir | maps | 22dbb3b : 4 tests de bornes 30/45/60 s, tests 1207/1207, merge main 1da908f ; branche prête, pas fusionnée | 12:36 |
| (Elrond-1113b fini 12:45) | a48eb9c3880c64c30 | Elrond | tiers | build PASS, tests 1202/1202, net ROUGE 3/3 (timeout hol.UI.Key Maj/&, machine chargée), deploy preflight FAIL (phase reste Packed, piste portée 5250 > 5000) ; tiers NON prête | 12:45 |
| ELROND-1113c | ac16adc86b094dcbb | Elrond | tiers | bug deploy (test rouge, correctif C++, build), préviol deploy, tool Maj, arrêt 14:15 | 12:48 |
| (Faramir-1113b fini 12:47) | a5ed9c305ee716769 | Faramir | fog | 85aa788/224e89a build.ps1 -MaxWaitMin (L12-3), bae86ec/1e4671d eol_check .git cassé + WARN ?? Retours ; préviol disconnect_1v1 impossible (net-playtest pas dans fog) ; gate_test complet non rejoué | 12:47 |
| SAURON-1113b | a99bf2ce32e5b2ca8 | Sauron | visual | quick_check, préviol resources_compare, Placeholders, arrêt 13:15 | 12:50 |
| (Sauron-1113b fini 13:01) | a99bf2ce32e5b2ca8 | Sauron | visual | tests 1201/1201, préviol resources_compare PASS 10 PNG vus -> E-resources_compare [À ESSAYER] VU À L'ÉCRAN ; resources_game préviol FAIL (non commité) ; 9e1411c + merge main 7ebaa07 ; branche prête | 12:50 |
| (Galadriel-1113b fini 13:02) | ad03afe28f1c0623c | Galadriel | galadriel-tab | 15221be D791 vert, e6b9c75 HUD rage + filtre infobulle Caserne (C++, non retesté), tests 1202/1202 sur le build 1 ; legions_warrior préviol FAIL (LaunchCheck compare la dll du module principal aux sources de tous les modules : faux périmé) -> LOGIQUE SEULEMENT | 11:43 |
| FUSION-1113 | a07fb8de186d7574b | Faramir | fog | fusion groupée fog+maps+net-playtest+visual+d751-root+galadriel-tab (pas tiers), build merge, quick_check tests+net, ff-only main, arrêt 14:15 | 13:07 |
| CELEBORN-1113b (fini 13:04) | a6598849c692c33a7 | Celeborn | hero-j8-close | cause du bras écrite, 38 variantes B balayées hors jeu, aucune verte (meilleur o25a15u8o4), soldier_poses.py non modifié, spear_arm LOGIQUE SEULEMENT, maa_attack vu [x] | 12:50 |
| CELEBORN-1113c (fini 13:31) | a1369bf4bd7183379 | Celeborn | hero-j8-close | arm_body_check spear_attack VERT (cause : clés 0 et 56 partageaient le même dict, deepcopy), e0cb438 pose B + a11e355 rebake ; préviol spear_arm FAIL non lu, aucun PNG vu | 13:09 |
| (Celeborn-1113d fini 13:43) | aafa582ff7cfff719 | Celeborn | hero-j8-close | préviol spear_arm PASS (3553fba scénario recadré, 8788ada journal), 5 PNG vus : pas de traversée visible mais Lancier bleu bras gauche levé douteux, caméra min 15 m -> E-spear_arm LOGIQUE SEULEMENT ; l. 29 non commencée | 13:36 |
| CELEBORN-1113d | voir lancement 13:36 | Celeborn | hero-j8-close | préviol spear_arm : cause du FAIL, 5 PNG vus, fiche E-spear_arm, analyse l. 29, arrêt 14:15 | 13:36 |
| ARWEN-1113b (fini 12:53) | a8912b7b64722100f | Arwen | d751-root | tests 1204/1204 ; net 15 REGRESSION valeurs vides (machine chargée, non concluant) ; 577b744 scénario neutrals_minimap (wait 6), PNG rouge/gris vus ; préviol FAIL 1 seul contrôle (294 fichiers Content/Audio/Candidates non commités) -> E-neutrals_minimap LOGIQUE SEULEMENT ; branche prête sous réserve de net | 12:40 |
| FARAMIR-1113b | a5ed9c305ee716769 | Faramir | fog | préviols legions_roster_gesture/disconnect_1v1 puis outils sans C++, arrêt 13:10, hôte fusion | 12:28 |

## Programme du 2026-11-13

Étape 0 (PLAN-1113a, relevé de 10:24 à 10:30 : `status.ps1`, git, dates des dll, `Docs\BuildQueue.csv`, charge machine). Rien n'a été lancé ni compilé par l'étape 0.

- **ALERTE machine, à régler AVANT tout lancement (10:28)** : un `git push` (pid 26788, lancé à 10:17:23 par une session Claude Code, `rtk git push`, dépôt `esdragor/MyCrusader`) n'a pas fini : le hook `.git\hooks\reference-transaction` (hook LFS d'auto-déverrouillage, daté du 2025-08-27) lance `git remote set-head --auto origin`, qui relance le hook, sans fin. 1173 processus `git` / `bash` à 10:28 (environ 110 de plus par minute), mémoire engagée 88,0 Go sur 88,7 Go (le fichier d'échange grossit), 17 Go de RAM physique libre seulement en apparence. Un build ou un éditeur lancé dans cet état échouera par manque de mémoire. L'étape 0 n'a rien tué (lecture seule). Autres charges : StarRail (12 Go engagés), l'éditeur de ProjectShiva (7 Go), ProjectRED-Win64-Shipping (5 Go) ; aucun `cl.exe` ; file de build vide.
- **Binaires, état réel (dll comparée au fichier `Source` le plus récent, 2 modules)** : frais dans Arwen, Celeborn, Faramir, Glorfindel, Haldir, Sauron (builds PASS finis après la séance du 2026-11-12, entre 13:29 et 14:05, `Docs\BuildQueue.csv`) ; PÉRIMÉS dans Elrond (dernier build FAIL à 14:06, `RoundTrip` introuvable dans `HOLNetTests.cpp`, 0072553), Galadriel (module HeroesOfLiberty seul : `HOLRageVisual.cpp` 12:10 contre dll 11:41) et la copie principale (2 modules). Aucun `quick_check` n'a tourné sur ces binaires.
- **File de build, ordre** : 1 Elrond (corriger `RoundTrip` puis `build.ps1 -LiveCoding=0 -Wait`), 2 Galadriel (un seul .cpp), 3 la place libre va à Glorfindel dès son `quick_check` rendu (code vert de D788). Arwen et Sauron ne compilent pas au départ. La copie principale se recompile après la fusion (essais rejoués depuis Faramir sinon).
- **Fusion groupée** : dans Faramir à 13:20 (branches commitées vertes et à jour de main à 13:10 au plus tard) : `fog`, `maps`, `hero-j8-close`, `d751-root`, `visual`, `galadriel-tab`, puis `tiers` et `net-playtest` seulement si vertes ; conflits attendus : verbe `notereset` (garder celui de fog), `HOLPlayerState.cpp/.h` (Arwen et Glorfindel) ; UN build `-Priority merge`, UN `quick_check -Parts tests,net`, avance rapide de main vers 13:50, jamais de push.
- **Essais visés (VU À L'ÉCRAN, 3 min, pré-vol PASS)** : dès ce matin sans attendre la fusion : `audio_compare_buildings_stone` et `_metal` (Arwen), `legions_roster_gesture` et `disconnect_1v1` (pré-vol dans Faramir, binaire frais = contenu de main), `spear_arm` (Celeborn, D787), `neutrals_minimap` (Arwen), `resources` (Sauron) ; après build : `legions_warrior` (Galadriel) ; incertain : `deploy` (Elrond). Rejeu sur main fusionné de 13:50 à 14:10.
- **Lignes périmées de `Auto_Journey_File.md`** : PREMIER GESTE « build » d'Arwen (l. 20), Celeborn (l. 25), Faramir (l. 41), Glorfindel (l. 55), Sauron (l. 85) : le build est fait, reste LaunchCheck + `quick_check` ; « NON COMPILÉ » des l. 21, 22, 57, 87 : compilé, non testé ; Celeborn et Haldir n'ont aucun commit devant main ; note « NON COMMITÉ à garder » de l'`IN_USE.txt` de Galadriel : commité (3224cbd). Toujours vraies : l. 36 Elrond, l. 45 Galadriel, l. 47 (a3e90a8 pas dans main), l. 86 (f0670cf pas dans main).

| Worktree | Branche | Tâche | C++ | Suivante |
|---|---|---|---|---|
| Arwen | `d751-root` (+4 commits, 8 fichiers Source, dll fraîche) | l. 20-22 : LaunchCheck, `quick_check -Parts tests,net`, commit vert de N7 (D789 : rien au survol) ; premier rouge = 3955b05 / 42cc580 doivent passer ; puis pré-vol `neutrals_minimap` ; `NeutralsInProgress.md` | non au départ | l. 14 du haut de file : pré-vol `audio_compare_buildings_stone` / `_metal` (candidats non commités à garder), fiches en `[À ESSAYER]` |
| Celeborn | `hero-j8-close` (= main, dll fraîche, 18 scénarios de repro non suivis à garder) | l. 26, D787 : prise de lance B dans `Tools\Editor\soldier_poses.py`, rebake, rouge 966ae02 (`arm_body_check.py`), scénario `spear_arm` ; `NeutralsInProgress.md` | non | l. 28 `maa_attack` à l'œil ; puis l. 29 BUG-melee-stuck-attack (C++, quand une place se libère) |
| Elrond | `tiers` (+6, 8 Source, dll PÉRIMÉE, build FAIL) | l. 36-37 : corriger `RoundTrip` dans `HOLNetTests.cpp`, build (file n° 1), LaunchCheck, `quick_check -Parts tests,net`, commit vert de G6 e + G4 ; rouge = e13e1f7 ; `Palier3InProgress.md` | oui (n° 1) | G6 f : pré-vol du scénario `deploy` (jamais lancé), fiche d'essai |
| Faramir | `fog` (+2 docs, dll fraîche) | pré-vols sur binaire frais des fiches `[NE PAS JOUER]` (`legions_roster_gesture`, `disconnect_1v1`), puis outils sans C++ : l. 101 (L12-1c, mémoire virtuelle < 8 Go, rouge dans `gate_test.ps1`), l. 98 (une dll par module, rouge dans `launch_check_test.ps1`), l. 103, 99, 100 ; `ScenariosInProgress.md` | non | arrêt à 13:10 ; hôte de la fusion groupée à 13:20, puis rejeu des essais sur main fusionné |
| Galadriel | `galadriel-tab` (+3, 5 Source, dll PÉRIMÉE) | l. 46, D791 d'abord (script : malus à 30, bonus à 40, test rouge sur le malus), puis l. 45 : build (file n° 2), LaunchCheck, `quick_check -Parts tests`, commit vert (rouge 236e56b) ; `LegionsInProgress.md` | oui (n° 2) | l. 47 pré-vol `legions_warrior` (Rage.Visual 4) ; l. 50 REGLAGE market ; l. 48 infobulle de la Caserne |
| Glorfindel | `net-playtest` (+4, 7 Source, dll fraîche) | l. 55 : LaunchCheck, `quick_check -Parts tests,net` (seul rouge attendu : `Gather.Income`), puis l. 56, D788 : test exigé à environ +350/min et code vert dans `UHOLGatherProcessor::CountGatherer` ; `SacredSitesInProgress.md` | oui (n° 3, après son quick_check) | commit vert, scénario de l'infobulle ; puis A4 Canaux de discussion (règle pure `HOLChatRules.as`) |
| Haldir | `maps` (= main, dll fraîche, `Content/Maps/Crown.umap` non suivi à ne pas toucher) | l. 65-68, D783-D786 : 4 réglages de script, test rouge d'abord pour chacun, `quick_check -Parts tests` ; journaux `SecondaryCentersInProgress.md`, `SignalsInProgress.md`, `SpecializationInProgress.md`, `NomadInProgress.md` | non | l. 80 tests à durcir (30/45/60 s) ; puis l. 71 `bSitesLaidByMaps` ou l. 72 SG2 (C++, quand une place se libère) |
| Sauron | `visual` (+5, 2 Source, dll fraîche) | l. 85-87 : LaunchCheck, `quick_check -Parts tests` (rouge a6e673f doit passer), pré-vol et PNG de `resources_compare`, commit vert, ligne de `Placeholders.txt` à retirer ; `VisualInProgress.md` | non au départ | l. 92 `neutrals` dans une clairière ; l. 90 lignes `[?]` de `Placeholders.txt` |
| Outils/docs | copie principale, `main` (docs seulement, commits par nom de fichier) | index de `Retours\` et `ESSAIS-A-JOUER.md` au fil des fiches `[À ESSAYER]`, l. 82 (`GDD.md` l. 3024, vieux « Prochain numéro D214 »), lignes périmées ci-dessus à annoter dans la file, `EtapeDefini.md` | non | l. 106 (L12-6) : docs « dans main » dans le même commit que la fusion |

## Bilan du 2026-11-13 (séance INTERACTIVE arrêtée par l'utilisateur à 11:20)

- Cause de l'arrêt : erreurs d'API intermittentes « 403 Attention Required (Cloudflare), Failed to authenticate » sur claude-opus-5-5 à partir de ~11:12 : 6 agents sur 8 coupés, y compris des agents repris et un agent neuf ; une sonde sans outil a pourtant répondu. Hypothèse (non prouvée) : limitation de débit par le nombre d'agents en parallèle. Aucune fusion dans main, rien poussé.
- Avant cela (10:16-10:50) : nettoyage des docs 0a (4 agents, commits 6c9c0f9, 870f3f8, a6f6705, 2503fed), sync_worktrees, étape 0 (plan dans « Programme du 2026-11-13 »). Fuite de processus : un git push lancé à 10:17 par une autre session (pas par cette séance) a bouclé sur le hook .git\hooks\reference-transaction (jusqu'à 1915 processus git/bash, mémoire virtuelle libre 1,0 Go), éteinte d'elle-même vers 10:46 ; hook toujours piégé pour un push/fetch.
- Fait : Haldir (maps 93ba4bb) D783-D786 en règles pures, tests 1203/1203 ; Arwen (d751-root 71f94ba) règle N7 verte, tests 1204/1204, préviol neutrals_minimap non fait ; Elrond (tiers 64ac8e2) test Net.DeployPhase corrigé, build non fait ; Galadriel (galadriel-tab 4d4ee79) test rouge du malus 30 %, correctif en fichiers modifiés non commités ; Glorfindel (net-playtest d8e887e) tests rouges Gather.Income et canaux, correctifs non commités ; Faramir (fog 132a8ce) tests rouges L12-1c, 3 fichiers modifiés non commités ; Celeborn et Sauron : rien de commité.
- Question tranchée : Q-chat-channels 1 A, 2 A -> D792 au GDD (GDD.md modifié, NON commité : accord de l'utilisateur requis), prochain D793.
- Pas de fusion groupée, pas d'essai donné, 0 essai prêt. Reprise : branches inchangées, relancer les agents un ou deux à la fois quand l'API répond ; premier geste de chacun = git status puis sa ligne de la file.
## Bilan de la reprise du 2026-11-13 (11:41 -> 14:05, interactive, 3 à 5 agents en parallèle, plus de 403)

- Main = 481bf43 : fusion groupée fog + maps + net-playtest + visual + d751-root + galadriel-tab (fog = cea8c57 puis refusion de main) ; build merge PASS, tests 1221/1221, net PASS du premier coup ; avance rapide refusée une première fois par `Auto_Journey_File.md` non commité dans la copie principale (commité par nom de fichier, refusion, puis ff-only). RIEN POUSSÉ. `GDD.md` (D792) reste non commité.
- Fait : D788 infobulle de revenu (Glorfindel 6dfa7ee) et A4 canaux (59e8f60) ; D791 et rage L4 + filtre infobulle de la Caserne (Galadriel 15221be, e6b9c75) ; bornes 30/45/60 s des délais de spécialisation (Haldir 22dbb3b) ; N7 minicarte (Arwen) ; `build.ps1 -MaxWaitMin`, eol_check en erreur si `.git` cassé (Faramir) ; correctif G6 f `ToggleDeploy` sur un engin Packed qui marche encore (Elrond 27d91a4) ; D787 prise de lance B : `arm_body_check` vert (cause : clés 0 et 56 de `SPEAR_ATTACK` partageaient un dict, Celeborn e0cb438).
- Essais : 2 prêts VU À L'ÉCRAN (`resources_compare`, `legions_roster_gesture`) ; 4 LOGIQUE SEULEMENT (`neutrals_minimap`, `spear_arm`, `deploy`, `legions_warrior`).
- NON fusionnées : `tiers` (Elrond : après le merge de main le code n'est ni recompilé ni testé ; deploy : le scénario attend `D=Pack` que le rapport ne produit pas), `hero-j8-close` (Celeborn : pose B, `spear_arm` à juger à l'œil, merge de main avec conflits de docs résolus en gardant les deux côtés).
- Non vérifié : C++ HUD de la rage à l'écran ; rouge du carré de camp ; PNG de `resources_compare` et `legions_roster_gesture` pas rouverts sur le build fusionné ; `gate_test.ps1` entier non rejoué ; test Gather.Income contrôle un rapport, pas le chiffre +350/min.
- Anomalies de processus : 3 builds en file de 16 à 40 min (une autre session compile ProjectShiva) ; `LaunchCheck` compare la dll d'un module aux sources de tous (faux « périmé », vu chez Galadriel) ; `quick_check` net rouge sous charge (3/3 chez Elrond, 15/15 chez Arwen) mais vert à la fusion (Maj tenu 2 s) ; gate Launch compte 5 agents C++ à cause du texte des `IN_USE.txt` ; chemin `Tools\quick_check.ps1` faux dans mes énoncés (le bon : `Tools\Tests\quick_check.ps1`) ; `Docs\SessionEnd.txt` était périmé depuis le 2026-11-10.
- Reprise : Elrond (recompiler, tests,net, corriger `deploy.txt`), Celeborn (juger la pose B), Galadriel (LaunchCheck module, `legions_warrior` plus près), Arwen (carré du camp lisible, essais audio : décision sur `Content/Audio/Candidates/`), BUG-melee-stuck-attack (analyse non commencée), SP2 / SG2 / `bSitesLaidByMaps` (C++ de Haldir).

### Lecons appliquees 2026-11-13

| Règle | Statut | Preuve |
|---|---|---|
| Gate Launch avant chaque lancement (L8, L11) | appliquee | PASS à 11:41 ; 1 FAIL « 5 agents C++ » à 13:00 (faux positif IN_USE.txt, relevé au bilan) |
| Plafond de 3 agents C++ (L3-1) | appliquee | 3 builds en file à 11:43 puis 1 (Elrond) ; les autres sans C++ |
| Un seul build `-Wait` par agent, pas de sondage (L11-2) | appliquee | aucune boucle ; files de 16 à 40 min attendues |
| Agents d'étape sans fenêtre (règle 0) | appliquee | tous les preflights en `-Hidden` / `-RenderOffScreen` |
| Arrêt à 45 appels vus (L9-2) | appliquee | 41 à 49 appels déclarés, `tool_uses` 29 à 53 |
| Fusion groupée, un build, un quick_check (L11) | appliquee | un build merge 919 s, tests 1221/1221, net PASS |
| Pas de fusion sans quick_check vert | appliquee | tiers et hero-j8-close écartées |
| Essais : préviol PASS + PNG vus avant « À ESSAYER » (règles 1-2) | appliquee | 2 prêts, 4 LOGIQUE SEULEMENT |
| Recette indépendante des essais (règle 5) | non appliquee | pas de rejeu par un agent tiers des 2 essais prêts après fusion |
| Sync des worktrees en début de séance (L8-9) | non appliquee | la reprise a démarré sans `sync_worktrees` (WARN du gate Launch) ; fait en fin de séance, Celeborn avec conflits de docs résolus à la main |
| Pas de question tranchée par l'agent (règle 8) | appliquee | 0 Q-* nouvelle, D792 seule décision reportée |
| Contrôle `GDD.md` non commité | appliquee | laissé non commité, accord de l'utilisateur requis |
| Autres règles de la check-list | non verifiee | non relues une à une dans cette reprise |

## Dernier bilan : 2026-11-12 (séance autonome 10:26-13:00, 26 agents, détail dans l'archive du 2026-11-13)

- Main = 6d46a74 : fusion groupée 4851635 (maps + fog + hero-j8-close, sans C++, tests 1200/1200) puis commits de docs ; rien poussé.
- Dans main : 4 chapitres du GDD en règles pures, sans appelant en jeu (Centres secondaires D384, Signaux, choix de spécialisation de palier, option Nomade) ; LaunchCheck compare au fichier C++ le plus récent ; eol_check au gate Close ; cause du bras du Lancier trouvée (A_Spearman_Attack, test rouge arm_body_check.py), pas corrigée.
- Écrit mais NON COMPILÉ (branches pas prêtes) : Arwen d751-root WIP 308419b (N7 point de camp sur la minicarte), Elrond tiers WIP cc3213e (G6 e, G4 filtre, scénario deploy jamais lancé), Galadriel galadriel-tab WIP 3224cbd (rage 40 %, hol.Rage.Visual 4), Glorfindel net-playtest WIP 6703b9f (infobulle de revenu D780, GoldLine retirée), Sauron visual 6c7292f (styles des ressources figés).
- Cause : machine saturée par un autre projet (1,2 Go de RAM libre à 11:52) ; aucun agent C++ n'a compilé après 10:56 ; binaires périmés partout sauf Haldir, copie principale comprise.
- Réponses de l'utilisateur du 2026-11-12 vers 13:00 : les 8 Q-* tranchées (D783-D791 au GDD, 36c202e), reportées dans les journaux et la file (1f2ad3b), fiches archivées dans `Docs\Archive\Retours_2026-11-12\`.
- Défauts codés à changer après ces réponses : D783 (150 bois + 400 pierre), D784 Q2 A, D785 Q1 C, D786 Q1 B, D787 prise B (lance à reposer), D788 C (taux corrigé des trajets), D791 (seul le bonus monte à 40 %, Galadriel monte les deux) : lignes `[ ]` de `Auto_Journey_File.md`.
- Attend l'utilisateur : 0 question, 0 essai jouable (7 fiches E-* en `[NE PAS JOUER]`, binaire périmé).
- Leçons L12-1 à L12-7 : toutes dans `Auto_Journey_Checklist.md` ; non appliquées le 2026-11-12 : plafond de 3 agents C++, arrêt dur (5 agents bloqués dans build.ps1 -Wait), sync de début en -NoBuild.
- REPRISE : première tâche de chaque worktree = `Tools\build.ps1` machine calme, puis quick_check, avant tout nouveau code (lignes PREMIER GESTE en tête de file).

## Chapitres du GDD non commencés (relevé NEUF du 2026-11-12 11:10, par les titres du GDD, les `*InProgress.md`, `EtapeDefini.md` et Grep dans `Script\` et `Source\`)

Le relevé du 2026-10-30 est épuisé (archive : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`). Celui-ci part des règles du GDD **sans code**, qu'un journal existe ou non. Ordre conseillé = ordre des lignes. « Règle pure » = Angelscript + `*_Test.as`, sans C++.

**Groupe A : prêt à ouvrir sans question (4)**

| # | § et titre | D | Existe déjà | 1re étape de 15 min, test rouge | C++ | Worktree |
|---|---|---|---|---|---|---|
| A1 | § 15 Signaux sur la carte (roue Alt + clic) | D386, D369, D376 | fil de messages (D445), alliés mutuels dans `Script\Diplomacy\HOLDiplomacyRules.as` ; aucun code de signal (Grep `Signal`, `Ping` vide) ; aucun journal | règle pure `HOLSignalRules.as` : 5 types + neutre, destinataires = alliés mutuels, plafond 3 par 5 s. Rouge : le 4e signal en 5 s est refusé ; un Neutre et un Ennemi ne le reçoivent pas | non (règle) ; oui ensuite (saisie, RPC, icône de mini-carte) | **OUVERT 2026-11-12 11:41 dans Haldir (`maps`, HALDIR-1112e)** : SG0-SG1 faites, `SignalsInProgress.md`, rouge 76f66b8, vert 9d30e2c (1186/1186), Q-signals ; suivant SG2 C++ |
| A2 | § 13 Choix de spécialisation de palier, mécanisme commun | D07, D332-D334, D683, D684 | plans dans `AubeInProgress.md` (l. 127) et `LegionsInProgress.md` (L9), paliers P1-P5 dans main ; aucun code (Grep `Specialization`, `TierChoice` vide) | règle pure `HOLTierChoiceRules.as` : un choix sur deux par palier, délai 30 / 45 / 60 s en parallèle de la file, annulable sans coût. Rouge : abouti à 30 s et pas à 29 ; annulé = rien ; 2e choix du même palier refusé | non (règle) ; léger ensuite (bouton du Forum, réplication) | Elrond (tiers) après G6, sinon Galadriel (L9) — **OUVERT 2026-11-12 11:56 (HALDIR-1112f, Haldir, maps)** : SP1 règles pures `HOLSpecializationRules.as` (nom retenu au lieu de `HOLTierChoiceRules.as`), rouge 567fd50, vert 1124a13, 1193/1193 ; `SpecializationInProgress.md`, `Retours\Q-specialization.md` ; suivant SP2 C++ |
| A3 | § 14.1 / § 14.3 Option Nomade | D207, D354, D402 | case au menu et au salon (`HOLNewGameScreen.cpp`, `HOLMatchConfig.h`), aucune règle ; `VictoryInProgress.md` la range « à venir » | règle pure `HOLNomadRules.as` : défaite à 0 Paysan tant qu'aucun forum n'est achevé, forum détruit en chantier sans vie perdue, stock de départ + prix d'un forum, héros près du plus gros groupe. Rouge : dernier Paysan mort sans forum = défaite, avec forum achevé = non | non (règle) ; oui ensuite (départ sans forum, groupes dispersés, distance minimale en réglage) | Elrond (victoire) ou un libre ; **OUVERT 2026-11-12 12:08 (HALDIR-1112g, Haldir, maps)** : N0-N1 faites (`NomadInProgress.md`, rouge 938087b, vert ae9d9af, 1200/1200), Q-nomad, suivant N2 C++ |
| A4 | § 15 Canaux de discussion | D385, D376, D505 | fil d'annonces, panneau Diplomatie ; aucun code (Grep `Chat` vide) ; `DiplomacyInProgress.md` l. 41 : « autre chantier de réseau » | règle pure `HOLChatRules.as` : destinataires par canal (Alliés, Tous, privé, spectateurs). Rouge : un message Alliés n'arrive ni à un ennemi ni à un spectateur ; un privé au seul destinataire | non (règle) ; oui pour le transport (RPC, saisie Entrée) | Glorfindel (net-playtest) |

**Groupe B : ouvrable après 1-2 questions (6)**

| # | § et titre | D | Existe déjà | Question à poser d'abord | 1re étape, test rouge | C++ | Worktree |
|---|---|---|---|---|---|---|---|
| B1 | § 6 Garnison (étape F7 de Fortifications) | D693, D351, D365, D374, D601 | plan et test rouge écrits (`FortificationsInProgress.md` l. 108) ; seul « pas de garnison chez un allié » est codé ; débloque SC2, le Roi, le forum | places par tour, occupants protégés ou non, quelles unités entrent (cavalerie ?) | `Script\Garrison\HOLGarrisonRules.as`. Rouge : on entre jusqu'aux places libres et pas plus, ennemi et allié refusés | oui (simulation) | Elrond (tiers) |
| B2 | § 9.4 XP d'exploration | D05, D06 | source `Exploration` déclarée « no carrier yet » (`HOLHeroXPRules.h`), brouillard F0-F9 | porteur : surface découverte, lieux découverts (relais, camps, autel) ou les deux ; valeur | règle pure par tranche explorée. Rouge : une tranche neuve paie une seule fois, aussi pendant la récupération | oui (lecture du brouillard) | Faramir (fog) |
| B3 | § 8.2 Zones de furtivité | D383 | points d'accroche décrits (`FogInProgress.md` l. 164), aucune carte n'en pose, aucun code (Grep `Stealth` vide) | distance « tout proche » ; quelles cartes en posent, quel aspect | règle pure « caché sauf ennemi dans la zone ou à moins de N ». Rouge : vu à N - 1, caché à N + 1 | oui (vision par camp) | Faramir, cartes par Haldir |
| B4 | § 8.1 Reste de D33 : couvert en forêt, forêt lente pour cavalerie et siège, défense réduite au gué | D33, D496 | marais, route et charge coupée codés (`Script\Terrain\HOLTerrainRules.as`) ; rien pour la forêt ni le gué dans ce script | « être en forêt » = zone peinte ou proximité d'arbres ; valeurs modérées | facteur de dégâts à distance en forêt dans `HOLTerrainRules.as`. Rouge : moins de dégâts à distance en forêt, mêlée inchangée | léger (le combat lit le facteur) | Haldir (maps) |
| B5 | § 7.3 Ordres Patrouille et Suivre | D30, D340 | Q attaque-déplacement, S arrêt, V maintien ; aucun code (Grep `Patrol`, `Follow` vide), aucun journal | case de la grille de chacun ; ce que fait Suivre (allié, ennemi) et « focus » | règle pure de patrouille (aller-retour, attaque-déplacement à chaque trajet). Rouge : l'unité repart vers A une fois en B | oui (ordres) | Celeborn (ordres d'unités) |
| B6 | § 7.3 / § 15 Raccourcis reconfigurables | D30 | onglet Commandes grisé « jusqu'à M4 », grille en dur ; aucun code (Grep `Rebind`, `KeyRemap` vide) | l'ouvrir maintenant ou le laisser grisé ; grille seule ou tous les raccourcis | table action vers touche en données + règle pure de conflit. Rouge : deux actions sur la même touche = refus | oui (interface C++) | Galadriel (grille) |

**Groupe C : déjà couvert ou sans règle à coder (une ligne de preuve)**

- § 5.1 Ressources, ferme, moulin : dans main (D390, D346) ; chasse, pêche, gibier écartés du prototype (`MapsInProgress.md` l. 425, § 17 n'en liste pas, Grep `hunt`, `deer`, `fish` vide).
- § 5.2 Ressources spéciales : principe sans règle (relu par HALDIR-1112b).
- § 5.3 Population : maisons et plafond joués par l'IA (IA1) ; D391 (population pleine sans file) vit dans les dossiers Légions (Squelettes) et Aube (Renforts).
- § 5.4 Centres secondaires D384 : `SecondaryCentersInProgress.md` (Haldir, maps), SC0-SC1 faites, SC2-SC3 attendent `Retours\Q-secondary-centers.md` et la garnison (B1).
- § 5.5 Marché et tributs : dans main, essai validé (`MarketInProgress.md`).
- § 6 Construction : zone anti-construction D22 codée (`HOLConstructionRules.h`) ; Forge, Académie, murs, portes, tours = F1-F15 de `FortificationsInProgress.md` (Elrond, tiers).
- § 6.1 Siège : S0-S8 dans main (`SiegeInProgress.md`) ; Trébuchet et Canon = `Palier3InProgress.md`.
- § 6.2 Bonus de hauteur, remparts, § 16.5 navigation à deux niveaux : étapes F8 et F12 de Fortifications.
- § 7.1 Unités : socle dans main ; Hallebardier et emblématiques = `AubeInProgress.md`, `LegionsInProgress.md` ; Arquebusier = Palier 3 ; charge = « avec la cavalerie » (`CavalryInProgress.md` C2).
- § 7.2 Combat et moral : D345, D517 dans main ; moral = `MoraleInProgress.md` (M1, M2, M6 faites).
- § 7.3 Sélection, groupes, Paysans inactifs, armée : codés (`HOLGameScreen.cpp`) ; F1-F3 et Tab sur galadriel-tab.
- § 8.1 Marais, route, charge coupée : codés (`HOLTerrainRules.as`) ; zones sacrées ou corrompues = capacités de faction (D40).
- § 8.2 Brouillard : F0-F9 dans main ; hauteur de vue codée (`HOLVisionRules.h`, sol plat aujourd'hui).
- § 8.3 Exploration : principe ; sa seule règle est l'XP (B2).
- § 8.4 Neutres : `NeutralsInProgress.md` (N1-N7).
- § 8.5 Cartes, tailles, pont des Gués : dans main (`MapsInProgress.md`, `HOLMapFordsTests.cpp`) ; biomes = `BiomeInProgress.md`.
- § 9.1 bis à § 9.8 Héros : J8a-J8c dans main (résistance `HOLCombatRules.cpp`, retour `HOLHeroReturn_Test.as`, XP `HOLHeroXPRules.h`) ; § 9.6 = `TalentsInProgress.md` ; capacité 3 et ultime après le prototype (D42).
- § 10 Duel : `DuelInProgress.md` (reste D121, D122, récompenses branchées).
- § 11 Technologies : T1-T7 dans main ; § 11.2 = `Palier3InProgress.md` (G1-G6).
- § 12 Volcan, § 12.2 bis, option Catastrophe, § 16.2 World State Manager : `VolcanoInProgress.md` (V1-V12 dans main, reste V9b, V9c).
- § 13.2 Aube : `AubeInProgress.md` (dossier complet D685-D690, A1 en tête) ; § 13.3 Légions : `LegionsInProgress.md` (L0-L4).
- § 14.1 Victoire : V1-V5 et paliers dans main (`VictoryInProgress.md`) ; Roi en garnison D365 attend B1 ; § 14.2 Snowball : principe.
- § 14.3 Modes : Diplomatie = `DiplomacyInProgress.md` ; Sites sacrés = `SacredSitesInProgress.md` ; Carte explorée = F8 du brouillard.
- § 14.4 IA, déconnexion, réseau : `AIInProgress.md` (IA0-IA4), `DisconnectInProgress.md`, `NetPlaytestInProgress.md`.
- § 15 HUD, panneau Diplomatie, statistiques (`HOLStatsScreen.cpp`), pause, salon : dans main ; héraut et sons = `AudioInProgress.md`.
- § 17.1 Tests prioritaires : `PlaytestInProgress.md` (PT0-PT8).
- **Après le prototype, ne se prennent pas** : § 13.4 à § 13.7 les quatre autres factions, § 12.3 Événements futurs (D209), § 14.3 Battlefield, Domination, Reliques, Scénario (D416), § 9.9 Progression hors partie, Diplomatie V2 (D432), moral de groupe à états (§ 7.2).

**Non vérifié dans ce relevé** : le C++ de `Source\...\Map\HOLTerrainRules.h` pour la forêt et le gué (B4, seul le script a été lu) ; le rattrapage d'XP sur un héros tué (D05) et l'XP pendant la récupération (D06) ; les alertes mineures de D202 (population pleine, recherche terminée) ; la fiche d'un héros adverse (D203) ; l'ordre des étapes F3-F6 avant F7 ; § 9.5, § 9.6, § 10, § 11, § 12 et § 13 jugés au seul état de leur journal, sans relecture du GDD ; aucun worktree n'a été fouillé (journaux de la copie principale et `SecondaryCentersInProgress.md` de Haldir seulement).
