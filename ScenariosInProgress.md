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
