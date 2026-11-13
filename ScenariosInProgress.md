# Scénarios de test — avancement

Demande de l'utilisateur (2026-10-20) : pouvoir tester vite avec des situations toutes faites. Pour tester l'attaque des
soldats, il ne doit plus avoir à créer un Paysan, construire une Caserne, recruter, faire apparaître des ennemis : le
jeu s'ouvre directement sur une carte avec des soldats et des ennemis.

## Choix de l'utilisateur

- **Lancement** : un script (`Tools/Scenarios/play.ps1 <nom>`) ouvre le jeu directement dans la situation ; en cours de
  partie, `hol.Scenario <nom>` la recharge. Pas d'écran dans le menu.
- **Ennemis** : selon le scénario (immobiles, ou qui chargent) ; pas d'IA, c'est un ordre donné au départ.
- **Quels scénarios** : pas de liste d'avance. **Chaque essai demandé à l'utilisateur définit son scénario** : on
  l'écrit avec l'essai et on lance le jeu avec.

## Conception

- Un scénario = `Tools/Scenarios/<nom>.txt` (étapes, mêmes verbes que `hol.Test.Run` : commandes console, `wait`,
  `waitlog`, `waitunits`…) et, s'il le faut, `Tools/Scenarios/<nom>.json` (configuration de partie, comme
  `-MatchConfig=` : carte, emplacements). Sans `.json`, configuration par défaut (un humain en slot 0).
- `hol.Scenario <nom>` : charge la configuration, ouvre la carte, attend qu'elle soit prête, joue les étapes. Sans
  argument : liste les scénarios. Une étape qui n'arrive pas **ne quitte pas** le jeu (contrairement aux contrôles
  scriptés) : message d'erreur à l'écran et dans le journal.
- **Touche « suite »** (choix de l'utilisateur, 2026-10-22, chantier D345) : `waitkey [texte]` attend que
  l'utilisateur appuie sur **Entrée** (pas avec Alt), sans délai maximal, une ligne bleu clair dans le panneau ; elle
  passe seule après 1 s sous `hol.Test.Run`, `-unattended`, les tests d'automatisation ou `hol.Scenario.AutoContinue 1`
  (pour répéter un scénario hors écran). Mettre un `waitkey` avant chaque étape d'un scénario à plusieurs temps, et
  remettre la caméra sur la cible juste avant chaque ordre ou apparition (`OrderNear` exige le point à l'écran,
  `SpawnHere` fait naître au centre de l'écran, la caméra peut glisser pendant une attente).
- Code en fichiers nouveaux (`Dev/`), hors des fichiers modifiés dans `HeroesOfLiberty-J5c` (J7b.5b en cours) : fusion sans
  conflit.

## Étapes

| Étape | État |
|---|---|
| SC.1 Mécanisme (`hol.Scenario`, `play.ps1`, README) + premier scénario `order_lines` (essai des lignes de J7b.5b) | **fait** (2026-10-20, `main`, rien de commité) |
| SC.2 Essai de l'utilisateur, commit | **fait** (2026-10-20, validé : « tout est bon ») |

## Journal

### 2026-10-20

- Demande et choix de l'utilisateur (ci-dessus). Briques déjà là : `hol.Test.Run`, `hol.UI.NewGame.Launch`,
  `-MatchConfig=`, `hol.Units.Spawn` / `SpawnHere`, `hol.Build.PlaceNear [enemy]`, `hol.Build.Instant`,
  `hol.Cheat.Resources`, `hol.Camera.FocusSlot` / `Focus`. SC.1 lancée en sous-agent.
- **SC.1 faite** (sous-agent arrêté à 15 min / ~100k tokens, code écrit et compilé ; vérifications faites par la
  session principale). Fichiers : `Dev/HOLTestSequence.h` (nouveau : `HOLTestSequence::Run(Owner, Steps, Options)`,
  `FRunOptions { bQuitOnTimeout, OnDone }`, `LoadSteps`), `Dev/HOLTestSequence.cpp` (une course par propriétaire,
  « Test » et « Scenario » ; verbe `note` ; `hol.Test.Run` inchangé, sauf qu'une nouvelle course du même propriétaire
  arrête l'ancienne), `Dev/HOLScenario.cpp` (nouveau, `hol.Scenario [Name]` : depuis le menu, chemin du bouton
  Nouvelle partie `hol.UI.NewGame.Launch - 0` ; en partie, `open <carte>` ; attend « Camera above the start of slot »),
  `Tools/Scenarios/play.ps1`, `README.md`, `order_lines.txt` / `.json` (Plain, slot 0 humain, slot 1 IA).
- Essai sans fenêtre (`-RenderOffScreen`), scénario puis rechargement : prêt en **12 s** chaque fois, carte rechargée
  (LoadMap), **25 unités** les deux fois (5 Paysans + 6 Lanciers + 6 Hommes d'armes du slot 0, 8 Lanciers du slot 1),
  Caserne finie. Corrigé après la 1re capture : l'armée naissait **sur les rochers** devant le centre (2 unités hors
  navmesh) → avancée à 40 m du centre, ennemis à 70 m ; population **17/10** → `hol.Cheat.Population 165` (17/175) ;
  caméra reculée (4600) ; les notes d'un passage restaient sous celles du rechargement →
  `ClearOnScreenDebugMessages()` au début (**pas encore compilé** : Live Coding du jeu de `HeroesOfLiberty-J5c` ouvert,
  il bloque toute compilation du projet, même depuis un autre worktree).
- `quick_check` rapide **PASS** (46 s, tests 82/82) avec le lanceur modifié.
- Note : sur `main`, les lignes colorées de J7b.5b ne sont pas encore là (non commitées, `HeroesOfLiberty-J5c`) ; l'essai
  valide le mécanisme. Une fois commité sur `main`, `HeroesOfLiberty-J5c` le récupère sans conflit (fichiers nouveaux).
- **Essai de l'utilisateur (1)** : le scénario tourne jusqu'au bout chez lui (journal : 3 notes, prêt en 11 s), mais il
  **ne voit ni « prêt » ni les notes**. Cause : messages de débogage d'Unreal, police fixe minuscule sur son écran
  **3840 × 2160**, en haut à gauche sous le panneau des triches, 30 s alors que le jeu met ~30 s à s'ouvrir.
  → **SC.1b** (sous-agent) : **panneau du scénario** dans l'interface (en haut au centre, titre + état + notes, reste
  jusqu'à fermeture « × » ou au scénario suivant), branché par `HOLTestHooks` (le cœur ne dépend pas de l'interface,
  D247), hors des fichiers modifiés dans `HeroesOfLiberty-J5c`.
- **SC.1b faite** (sous-agent arrêté à 15 min / ~120k tokens ; dernière retouche compilée et vérifiée par la session
  principale). `HOLTestHooks::FScenarioDisplay` (nom, état en préparation / prêt / erreur, notes, fermé) +
  `OnScenarioChanged` / `NotifyScenarioChanged` (repli sur les messages de débogage sans interface : serveur dédié) ;
  `HUD/HOLScenarioPanel.h/.cpp` (nouveau, comme `HOLCheatPanel` : titre + état, notes en style Body, « × »), hébergé par
  `AHOLGameHUD` (développement seulement, Z 2), placé sous la barre du haut de l'écran de jeu (`GetTopBar()`, sans
  modifier `HOLGameScreen`). Journal : `Scenario panel: scenario X ready, 3 notes`. Lignes lues par les scripts
  inchangées.
- **Live Coding** : un jeu ouvert (n'importe quel worktree, même moteur) fait refuser toute compilation par UBT
  (« Unable to build while Live Coding is active »). Contournements : compiler avec **`-NoHotReloadFromIDE`** (la
  garde ne sert à rien entre worktrees : DLL distinctes) ; lancer les jeux avec **`-LiveCoding=0`** (`play.ps1` le
  fait ; à ajouter à `quick_check.ps1` quand `HeroesOfLiberty-J5c` aura commité ses changements de ce fichier).
- Preuve sans fenêtre en **3840 × 2160** : panneau lisible en haut au centre, 3 notes une seule fois après
  rechargement, « prêt ». Cadrage repris : ennemis à **22 m** (6200 ; 15 m au moins entre les bords des groupes),
  caméra 4700 et un cran de molette en arrière : armée et ennemis entre le panneau du scénario et celui de la sélection.
  `quick_check` rapide **PASS** (31 s, 82/82). Jeu relancé pour l'essai (2).
- **Essai (2) validé** par l'utilisateur : « tout est bon ». `quick_check -Full` **PASS** (545 s, toutes les parties,
  82/82). Commit sur `main` des seuls fichiers du chantier (les .md GDD / Journey / Animations modifiés par une autre
  session restent hors du commit). **Chantier clos.** Suite : chaque essai demandé vient avec son scénario ;
  `-LiveCoding=0` à ajouter à `quick_check.ps1` après le commit de `HeroesOfLiberty-J5c` ; consigner la décision au GDD
  (numéro libre suivant) quand le GDD n'aura plus de changements en cours d'une autre session.
- **Revue de 9debb86** (collée par l'utilisateur) : deux défauts, aucun bloquant.
  1. **Corrigé** (non commité) : `AHOLGameHUD::EndPlay` laissait `HOLTestHooks::OnScenarioChanged` posé : revenu au
     menu, une étape d'un scénario échouée avant l'ouverture de la carte n'affichait rien (le repli sur la ligne de
     débogage ne joue que sans crochet). Le HUD efface maintenant le crochet en fin de partie, seulement s'il est
     celui qui l'a posé (`ScenarioHookOwner`, deux joueurs locaux dans l'éditeur). Compilé ; scénario + rechargement :
     panneau « prêt, 3 notes » les deux fois ; `quick_check` rapide PASS (24 s, 83/83, avec les lignes D328 de
     c2b44e4 désormais sur `main`).
  2. **Laissé tel quel** (avis de la revue) : jeu fermé pendant qu'un scénario se prépare → le destructeur statique de
     la course appelle `Finish()` (retrait du journal et du minuteur) très tard, quand `GLog` et `FTSTicker` ne sont plus
     garantis. Sans effet constaté ; à corriger si un plantage à la fermeture apparaît (piste : tout arrêter sur
     `FCoreDelegates::OnEnginePreExit`).

### 2026-10-21
- **Scénario sans `.json` : BattleMap au lieu de Plain** (défaut vu par la session du retardataire, `EtapeDefini.md`) :
  corrigé dans `HeroesOfLiberty-J5c`, branche `scenario-default-plain` (non commité). Cause : sans `.json`, `hol.Scenario`
  prenait `FHOLMatchConfig::MakeDefault()`, qui nomme `/Game/Level/BattleMap` ; le repli sur Plain ne jouait que pour
  une carte vide. Depuis le menu, l'écran Nouvelle partie, qui ne propose pas BattleMap, lançait sa première carte
  (Plain) et `waitmap BattleMap` expirait. Maintenant : sans `.json`, Plain (comme le dit le README). Compilé (110 s) ;
  scénario temporaire sans `.json` lancé en `-RenderOffScreen` : `Scenario nojson_check: /Game/Maps/Plain.Plain`,
  `New game: launching /Game/Maps/Plain.Plain`, « prêt » en 18 s. À savoir : avec `-unattended`, le jeu s'arrête au
  démarrage (rien du scénario au journal) ; lancer comme `quick_check`, sans lui.
- **Décision consignée au GDD** (mycrusader-82, à la demande de l'utilisateur) : **D341** (§ 21, datée du 2026-10-20),
  paragraphe « Scénarios d'essai » à la fin du § 16.6 ; numéro suivant D342 (§ 22). Écartés notés : écran dans le menu,
  liste fixée d'avance, ennemis menés par l'IA.
- **Bouton « Relancer »** (demande de l'utilisateur, `EtapeDefini.md`, Outils d'essai ; mycrusader-82,
  `HeroesOfLiberty-J5c`, branche `scenario-restart` depuis `main` 61893ff, rien de commité) : dans l'en-tête du panneau du
  scénario, à gauche du « × », même style ; un clic fait `hol.Scenario <nom>` (étapes relues dans le fichier, carte
  rechargée). `hol.HUD.Click` connaît `ScenarioRestart` et `ScenarioClose`. Compilé ; vérifié sans fenêtre
  (`-RenderOffScreen`) avec un scénario temporaire qui clique sur son propre bouton : 3 relances de suite, carte
  rechargée, notes remises à zéro, « prêt » à chaque fois (scénario supprimé ensuite) ; `quick_check` rapide **PASS**
  (38 s, 90/90). Essai de l'utilisateur lancé sur `formation_one_kind` (`-Root ..\HeroesOfLiberty-J5c`).
- **Essai validé** par l'utilisateur (« c'est bon ») : commit **c446d99** sur `scenario-restart`, `main` avancé dessus
  (sans fusion, même point de départ). Ligne retirée d'`EtapeDefini.md`. `HeroesOfLiberty-J5c` libéré.

### 2026-10-30
- Scénario `tab` (Galadriel, branche `galadriel-tab`, d4fadd0 ; ordres et onglet de Tab, `InterfaceInProgress.md`) : 9 unités sélectionnées au cadre, cibles variées autour, deux joueurs (`tab.json`). Lancé hors écran : « prêt », 3 notes, sélection de 9. Essai de l'utilisateur à faire.
- Scénario `surplus` (Celeborn, branche `hero-j8-close`, clôture de J8 ; `surplus.txt` / `surplus.json`) : un héros ennemi niveau 4 encerclé par 6 Hommes d'armes du joueur, 8 renforts avec clic droit sur lui (`hol.Select.ClickUnit -2 order enemy`), un Homme d'armes ennemi libre 9 m derrière ; scène à ~46 m du héros du joueur (au nord-est, la carte Plain s'arrête à X -20000), pour essayer `hol.Combat.SurplusReassign` 1 / 0. Lancé hors écran : prêt, 5 notes ; mesure A/B hors écran (`hol.Combat.Report` à +3 s et +7 s après l'ordre) : avec 1, 4 des 8 renforts sur l'Homme d'armes libre, avec 0 tous sur le héros dont certains inactifs. Piège noté : une étape placée avant les `waitlog Starting hero` les fait manquer (le journal d'avant n'est pas relu) ; l'IA du slot 1 garde la main sur ses Paysans (celui du premier essai est parti vers son camp), d'où un Homme d'armes comme ennemi libre. Essai de l'utilisateur à faire (ligne dans `Auto_Journey_Retours.md`).

### 2026-11-09 (Sauron-E, sans C++, branche visual)
- session_gate -Phase Essay : seule la ligne branches: compte (plus de scan de la prose), -Essay accepte le nom du scenario (fiche retrouvee par play.ps1 <scenario>) ; test rouge 031b28d, vert 15da447, gate_test.ps1 tout PASS ; passage de bout en bout sur les 12 fiches E-audio_compare_* : 0 FAIL (restent seulement les vrais FAIL : net-playtest et tiers absents de Sauron).
- Essais ratés du 2026-10-31 : restent neutrals et fog_trees (les autres refaits). fog_trees : preflight PASS (50 s), fogtrees_a/b/c vus, Retours\E-fog_trees.md [A ESSAYER], GESTE reel, gresillement NON VERIFIE. neutrals : scenario corrige (d5df96a : Q + clic, capture cleared apres 150 images, camp dezoome), preflight PASS, mais le camp reste cache par les arbres sur neutrals_camp : Retours\E-neutrals.md LOGIQUE SEULEMENT + Q-neutrals-camp-cache.md (defaut : rien change). Suite : appliquer la reponse (poser le camp hors du bosquet), repreflight, PNG, passer E-neutrals en [A ESSAYER].
- Sauron-4 (2026-11-09 16:5x, sans C++, branche visual) : neutrals, reponse C (clairiere) IMPOSSIBLE par commande de test : les arbres du bosquet central de Plain sont du decor, pas des sources (hol.Sources.Nearest 0 0 = mines a 120 m ; TakeNear x16 a vide 8 mines, aucun arbre). Repli B essaye (hol.Camera.Focus -300 0 + hol.Camera.Zoom 6, capture neutrals_camp_close, puis Zoom -6) : les Lanciers gris se voient entre les troncs (1 a 2 a moitie caches) ; 2 preflights de suite FAIL a l'audit Sanity (spin de melee, unites #8 puis #9, D641 connu) alors que le preflight sans le plan serre passait : cause non isolee, scenario remis a l'etat commite (git checkout). Question Retours\Q-neutrals-trees-decor.md (A commande de decor C++, B plan serre, C camp deplace ; recommandation C). E-neutrals reste LOGIQUE SEULEMENT. Aucun commit de code.
- 2026-11-09 17:5x : réponse utilisateur Q-neutrals-camp-cache = C (retirer les arbres autour du camp, 16:44), IMPOSSIBLE par commande de test (arbres du bosquet = décor) -> remplacée par Q-neutrals-trees-decor = A (commande de décor C++ pour retirer les arbres autour du camp, prochaine séance) ; fiche Q-neutrals-camp-cache supprimée, Q-neutrals-trees-decor et E-neutrals gardées jusqu'au lot C++.
- Elrond-22 (2026-11-10 10:05-10:25, sans C++, branche tiers = main 644b196) : arquebusier, note 5 ajoutee a Tools\Scenarios\arquebusier.txt (explique le duel laisse jouer : 155 -> 117 -> 79 -> 41 en 3 tirs, l'Homme d'armes court a 7 m/s et frappe l'Arquebusier de 70 PV avant le 4e tir ; equilibre, pas un bug ; chiffres a l'utilisateur, Retours\Q-arquebusier-valeurs.md, aucune valeur modifiee). Binaire periome : build de rattrapage PASS (397 s). Preflight arquebusier PASS (31 s), arq_before (155/155) et arq_hit (117/155, l'Arquebusier deja au contact) rouverts et vus, VU A L'ECRAN ; E-arquebusier.md mis a jour (GESTE reel par ClickUnit, SON non entendu). quick_check -Parts values : PASS (plus de rouge, rien dans known_reds.txt). Suite : l'utilisateur joue arquebusier (play.ps1 avec -Root), repond OK/BUG/REGLAGE.

### 2026-11-12 (FARAMIR-1112a, sans C++, branche fog)
- Scenarios jamais demarres chez l'utilisateur (2026-11-11 soir), causes : disconnect_1v1 = lance sans -PlayerName (spectateur, etape 5) ; audio_compare_buildings_stone/_metal = lances depuis Haldir, candidats seulement dans Arwen (Â« candidate missing Â», ligne Display que le preflight ne lisait pas) ; legions_roster_gesture = Faramir avance sur main a 18:01 (C++ de 17:01-17:17) sans recompiler, etape 41 jamais venue (deduit, journal ecrase). En plus : binaire de la copie principale perime (11-11 11:06, Angelscript: Error au demarrage, jeu bloque a l'image 0 en -Hidden) ; Arwen et Faramir aussi plus vieux que leur dernier commit C++.
- Test rouge 323b73a (launch_check_test.ps1), vert 42e5e41 : LaunchCheck.ps1 (Get-ScenarioPlayerName, Get-LaunchProblems, Get-UncommittedDependencies, Get-LogLaunchProblems) ; play.ps1 et check.ps1 prennent le premier siege Humain nomme du .json ; preflight section 6 Â« user's launch Â» : FAIL si play.ps1 ne nomme pas le joueur, fichiers non commites (-AllowUncommitted = WARN, -Root impose), binaire plus vieux que le dernier commit C++ de -Root, Â« candidate missing Â» / step timeout / spectateur au journal.
- Q-play-hidden-wait repondue par l'utilisateur (2026-11-11 soir) : A maintenant (-Hidden attend au plus -TimeoutSec puis ferme, deja en place), C plus tard (verbe quit de scenario, C++ de HOLTestSequence : a faire par un agent C++) ; fiche archivee dans Docs\Archive\Retours_2026-11-12.
- Suite : preflight complet (pas lance par cet agent, budget d'appels) sur une copie recompilee ; verbe quit (C++) ; ESSAIS-A-JOUER inchange (aucun preflight PASS sur le lancement reel).

### 2026-11-12 (FARAMIR-1112b, branche fog, build de Faramir lance a 10:53)
- Faille trouvee dans la section 6 de 42e5e41 : le binaire etait compare a la DATE du dernier commit C++. Un dll compile avant le commit du meme code (compiler, tester, committer : le cas courant) etait dit perime, et une recompilation sans changement ne relie pas le dll : FAIL pour toujours. Test rouge 87c4682 (launch_check_test cas 4 : vraie copie git jetable, vrai dll, vrais commits ; cas 5 : fichiers non commites sur un vrai git status), vert fff9792 : Get-StaleBinaryProblem (LaunchCheck.ps1) compare le dll au fichier C++ le plus recent de la copie (Source, Plugins ; comme UBT ; une fusion ou une avance rapide ecrit ses fichiers a son heure) ; preflight FAIL aussi sans dll.
- Cas reels prouves sans rien laisser dans l'arbre : Faramir avant build = FAIL (dll 11-11 16:02 < Source\HeroesOfLibertyUI\HUD\HOLGameScreen.cpp 11-11 18:01:51, l'heure de l'avance rapide) ; fichier non suivi Tools\Scenarios\legions_roster_gesture.probe_uncommitted = FAIL « 1 uncommitted file(s) », supprime ensuite (git status vide).
- NON FAIT faute de build (build de Faramir lance a 10:53 par build.ps1 -Wait, encore en file a 12:38, laisse tourner, ni tue ni relance) : preflight COMPLET legions_roster_gesture et disconnect_1v1 sur binaire frais, PNG du .look, etape 41 de legions jamais vue, section 6 PASS sur le cas sain de bout en bout (seuls ses FAIL sont prouves, par le test et les deux cas reels ci-dessus). Fiches E-legions_roster_gesture et E-disconnect_1v1 laissees en [NE PAS JOUER] par la cloture.
- Trouve par la recette (a corriger, test rouge d'abord dans launch_check_test.ps1) : Get-StaleBinaryProblem ne regarde que Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll ; le module Source\HeroesOfLibertyUI a sa propre dll (UnrealEditor-HeroesOfLibertyUI.dll) : comparer chaque dll de module a la source la plus recente de SON dossier Source\<module>.
- Etat : main = 4851635 contient fog jusqu'a fff9792. Suite pour un agent neuf : verifier que le build est fini (dll plus recentes que Source), corriger le cas des deux dll, puis preflight.ps1 legions_roster_gesture puis disconnect_1v1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir en entier (play.ps1 : -Name positionnel, -Root, -Hidden, -PlayerName, -TimeoutSec, -NoWait, -DryRun).

### 2026-11-13 (FARAMIR-1113, sans C++, branche fog, 10:53-11:2x)
- legions_roster_gesture : preflight COMPLET PASS sur binaire frais (103 s, sequence 188 etapes done, etape 41 venue), 9 PNG vus en 3 planches : noms FR/EN/JA corrects, Guerrier damne / Damned Warrior / JA traduits (defaut du 11-11 corrige), Building attack en anglais. Retours\E-legions_roster_gesture.md -> [A ESSAYER] VU A L'ECRAN (GESTE interne : objet = affichage).
- disconnect_1v1 : session_gate -Phase Essay FAIL (net-playtest absente de Faramir) : PAS joue ; etape 5 non reverifiee ; a faire depuis un worktree qui contient net-playtest ou apres sa fusion.
- L12-1c (ligne 101) : session_gate Launch WARN memoire virtuelle libre < -MinFreeVirtualGB (8) et WARN cl.exe d'un autre moteur (-EngineRoot, ancetres) ; build.ps1 attend la memoire (max 1800 s, puis compile) et l'ecrit (console + colonne load de BuildQueue.csv, en-tete migre une fois). Tests rouges gate_test.ps1 + build_queue_test.ps1 [m] (commit rouge 'red tests for L12-1c'), verts.
- Ligne 98 : LaunchCheck Get-StaleBinaryProblem compare chaque dll de module a son dossier Source\<module> (UI a sa dll). Rouge 565e864 (launch_check_test cas 6), vert. Faramir reel : frais.
- RESTE pour un agent neuf : lignes 103 (preflight en-tete section 6, gate Close ?? de Retours), 99 (eol_check si git ls-files echoue), 100 (L12-2 FAIL > 5 commits Source de retard) ; build.ps1 -MaxWaitMin (ligne 102 hors tache).

### 2026-11-13 (FARAMIR-1113b, sans C++, branche fog, 12:27-12:5x)
- Preflights NON relances : legions_roster_gesture deja PASS a 10:53 sur la meme dll (aucun changement Source/Content depuis 25f5094, dll 11-12 14:04) ; disconnect_1v1 : net-playtest (a859c73, 12:25) ni dans fog ni dans main, E-disconnect_1v1 reste [NE PAS JOUER].
- Ligne 103 (L12-3) : build.ps1 -MaxWaitMin N (0 = sans limite) : au-dela de N min en file, sort sans compiler (\"wait exceeded (attente depassee)\", ticket retire, CSV TIMEOUT, exit 5). Rouge 85aa788 (build_queue_test [t]), vert 224e89a, build_queue_test complet PASS (166 s). Lignes 98 et 101 cochees (faites par FARAMIR-1113).
- Ligne 99 : eol_check sort 2 et ecrit ERREUR si git ls-files echoue ; gate Close dit WARN \"fins de ligne non controlees\" (jamais PASS). Rouge bae86ec (gate_test, cas .git casse). Ligne 100 : en-tete preflight section 6 ; gate Close WARN des fichiers ?? de Retours\ de la copie principale. Bout en bout : eol_check code 2 / 0, gate Close WARN EolBroken, PASS EolClean, WARN Retours/E-deploy.md non suivi.
- NON VERIFIE : gate_test.ps1 complet (> 10 min, lance en fond avant le correctif, sortie jamais lue) ; le nouveau cas ligne 99 de gate_test pas rejoue dans gate_test (seulement le meme scenario a la main). Reste : ligne 102 (L12-2), lancer gate_test complet.

### 2026-11-13 (FARAMIR-INTEG soir, integrateur, sans C++, branche fog = main 8d13f46, 16:25-)
- Taches : L85 preflights talents, fog_orders, map_eldorado ; L45 legions_warrior ; L46 quick_check -Parts build (build hud) ; L41 note. Arret dur 19:45.
- 16:25 : memoire virtuelle libre 2,6 Go (< 8), 4 editeurs ProjectShiva (autre projet), aucun de Faramir.
- Memoire remontee a 9-12 Go (session_gate PASS 9 Go a 16:26) : preflights joues, un editeur a la fois, caches.
- talents : preflight PASS (117 s), 7 PNG ouverts et vus (talents_points/window/first/second/dawn/closed/cry) conformes au .look ; ecarts mineurs : le panneau dit « en preparation... » sur toutes les captures (le journal dit ready), recharge du cri 55 (look : 56), population 11/10 sur talents_cry. VU A L'ECRAN.
- fog_orders : preflight PASS (38 s), fogord_a/b vus : ligne jaune et points nets, sans strie, brouillard en place 3 s plus tard ; NB les points d'ordre sont au bord du noir plutot que dedans. VU A L'ECRAN.
- map_eldorado : preflight PASS (25 s), eldorado_army (10 Hommes d'armes bleus devant le forum, rien ne se bat) et eldorado_middle (amas d'or dans la couronne de marais) vus ; ECART au look « rien d'autre » : un objet rond gris a pointe orange a gauche de l'amas + deux cercles fins aux bords gauche/droit, non identifies. VU A L'ECRAN avec cet ecart.
- legions_warrior (L45) : 1er preflight PASS mais legions_warrior_after FAUX (rage encore en cours, « Rage 6 s », teinte presente) : `frames 600` = ~4 s en cache (~150 img/s). Correctif scenario `wait 11` (a633b97), 2e preflight PASS (43 s, -AllowUncommitted avant commit), 3 PNG vus : bouton Rage actif ; teinte VIOLETTE (bleu equipe + rouge) plutot que « rouge sombre », case E grisee « Rage 10 s » ; apres : teinte partie, ennemi mort, case grisee « Rage 29 s » = la recharge de 40 s compte depuis le LANCEMENT (pas depuis la fin de la rage) : constat pour l'utilisateur, rien tranche. VU A L'ECRAN.
- L41 (note) : talents_heal_cell (« ++ » dore au-dessus de la fenetre Arbre de talents) NON rejoue ici (pas dans la file de cet agent) ; reste question de couche au proprietaire du rendu des marques de soin.
- L46 : quick_check -Parts build lance 16:34 (memoire 7,2 Go), resultat ci-dessous.
- Appels : 42 a 16:40 (arret a 45).

### 2026-11-13 (FARAMIR-OUTILS soir, outils AutoJourney, sans C++, branche fog, 16:40-, arret dur 19:45)
- Taches : (1) L13-2 plafond C++ de `session_gate -Phase Launch` (ne compter que tickets vivants + IN_USE.txt OCCUPE avec `CPP: oui`) ; (2) L87 FAIL > 5 commits Source de retard sur la dll ; (3) L88-L91 de la file.
- 16:40 : git merge main (avance, 4 fiches E-*), IN_USE.txt OCCUPE + `CPP: non`.
- 16:45 : tests rouges ecrits dans `Tools\AutoJourney\gate_test.ps1` : L13-2 (texte citant C++ / libre + CPP: oui / OCCUPE + CPP: non = rien), L87 (5 vs 6 commits Source apres la dll, vrai depot jetable), L88/L89 (agent_rules + modele d'enonce citent `CPP: oui`, `LAUNCHCHECK_DEBUT`, `TESTS_TAUTOLOGIQUES`), L90 (Close WARN si EtapeDefini dit « pas dans main » pour une branche deja dans main, `-EtapeFile`).
- Appels vus : ||||| ||||| ||||| ||||| |||| (24 a 16:47)
- 17:01 : run rouge de gate_test (16:44-17:01, ~17 min : chaque -Phase Launch relance no_window_test) : 10 FAIL attendus = capMix (format OCCUPE + CPP: oui), L13-2 (4 agents comptes dont U3 « Pris ... C++ » et U6 « Haldir ... fichiers C++ »), L87 (6 commits), 7 controles de docs ; L90 deja vert (correctif ecrit pendant le run). Commit rouge b9e6c03.
- 17:02 : correctif 97a0622 : session_gate Launch (IN_USE.txt compte seulement si 1re ligne OCCUPE ET ligne `CPP: oui` ; L87 `git log --since=@<dll> HEAD -- Source` > 5 = FAIL, dll principale UnrealEditor-HeroesOfLiberty.dll sinon la plus recente) ; Close L90 (-EtapeFile, SHAs cites tous ancetres de main, sinon branche du 1er mot entre parentheses) ; agent_rules.md + Auto_Journey.md (marqueur CPP, LAUNCHCHECK_DEBUT, TESTS_TAUTOLOGIQUES, docs dans le commit de fusion).
- 17:02 : 2e run de gate_test lance (resultat NON LU par cet agent, arret a 45 appels) : PROCHAINE ETAPE agent neuf = `powershell -NoProfile -ExecutionPolicy Bypass -File Tools\AutoJourney\gate_test.ps1` (~17 min) doit finir `=> tout PASS` ; puis L91 (build.ps1 « BUILD LENT » : BuildQueue.csv n'a PAS de colonne nombre d'actions, il faut d'abord la relever dans la sortie UBT `[N/M]`).
- Appels vus : 46 a 17:05 (arret).

### 2026-11-13 (FARAMIR-OUTILS-R, reprise, sans C++, branche fog, 17:06-, arret dur 19:45)
- 17:06 : gate_test.ps1 relance en arriere-plan (sortie scratchpad gate_test_run.txt), attente de la notification de fin.
- 17:08 : L91 test rouge d996822 (`Tools\Tests\build_queue_test.ps1 -Only s` : CSV factice normal 1-2 s / merge 100 s, build factice 6 s : normal = BUILD LENT console + colonne load, merge = rien, CSV vide = rien) : 2 FAIL attendus.
- 17:10 : correctif f4ace98 : `build.ps1` `Get-SlowVerdict` (mediane des PASS du meme `priority`, >= 3 lignes, duree > 3 x mediane => « build : BUILD LENT (...) » + ajout a la colonne `load`, schema du CSV inchange) ; `status.ps1` liste les BUILD LENT des 2 dernieres heures. `-Only s` 7/7 PASS.
- Constat sur le vrai CSV (lecture seule) : mediane normal 90 s (326 PASS), 67 auraient ete marques (20 %) ; merge 111 s, 6/44. Pas de compte d'actions UBT : question `Retours\Q-build-lent-seuil.md` (A minimal en place / B compter `[n/N]` du build.log / C seuil 3e quartile), recommandation B.
- Appels vus : ||||| ||||| ||||| ||||| (20 a 17:11)
- 17:14 : L90 (jamais vu rouge : correctif ecrit pendant le 1er run) : test de mutation (script jetable scratchpad `l90_mutation.ps1`, copies mutantes de session_gate.ps1) : la 1re assertion etait trop large (satisfaite par une autre ligne) => resserree sur `commits .*AgentX` + 3e assertion ligne sans SHA (`branche featx`, AgentW) dans gate_test.ps1. Resultat : reel VERT ; mutant SHA ignores ROUGE ; mutant branche off ROUGE ; mutant regle retiree ROUGE. Le run de gate_test lance a 17:06 teste l'ANCIENNE version de ces assertions : a relancer une fois apres sa fin.
- Appels vus : 34 a 17:15

- 17:45 : 2e run gate_test (assertions L90 durcies) : => tout PASS
- 17:30 : REPONSE UTILISATEUR Q-build-lent-seuil = B. ARRET (55 appels vus, seuil 45 depasse : le trait 34 a 17:15 sous-comptait). B NON FAIT. PROCHAINE ETAPE agent neuf (sans C++) : (1) test rouge dans Tools\Tests\build_queue_test.ps1 cas [s] : fake.ps1 ecrit des lignes [1/N] ... [N/N] (variable HOL_FAKE_ACTIONS) ; CSV factice avec des PASS normal de 2 actions/1-2 s ET de 200 actions/100 s (load = 'vm 10 GB; actions N') ; build 6 s a 2 actions => BUILD LENT, build 6 s a 200 actions => rien ; (2) Tools\build.ps1 : N = max des [n/N] du build.log (0 si aucun), LoadText += '; actions N' ; Get-SlowVerdict ne garde que les PASS du meme priority dont 'actions M' est proche (defaut propose M entre N/2 et 2N, N=0 => M=0 ; >= 3 lignes sinon pas de verdict ; anciennes lignes sans 'actions' ignorees) ; (3) build_queue_test complet PASS, commit, L91 + EtapeDefini. La largeur 'proche' (x2) n'est pas tranchee par l'utilisateur.
