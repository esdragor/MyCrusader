# FARAMIR-1120pe : deux scenarios d'essai prepares, NON JOUES

APPELS_DECLARES: ~19 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 16:06 / 16:10 (Get-Date PowerShell)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenarios seulement) ; assertit l'effet : non
COMMITS: 1 (70f7754 sur zone-effect, 16:10), merge_main_fait: non
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: wall_line_long : preflight NON LANCE, aucun PNG, LOGIQUE SEULEMENT, NON JOUABLE ; virtuous_knight : preflight NON LANCE, aucun PNG, LOGIQUE SEULEMENT.
QUESTIONS: aucune

## Fichiers (Tools\Scenarios\ de C:\Users\baudo\Documents\HeroesOfLiberty-Faramir)
wall_line_long.txt / .look / .json ; virtuous_knight.txt / .look / .json (json = copie de deploy.json : Plain, slot 0 humain, slot 1 IA ; factions par defaut).
Chaque .look commence par `# branches: wall-line` ou `# branches: tool-known-reds`.

## Trouvailles qui bloquent l'essai (a lire avant de planifier)
1. wall_line_long : AUCUN TYPE DE MUR n'existe. `HOLWallLine::WallTypes()` (Script\Buildings\HOLWallLineRules.as:193, branche wall-line) renvoie une liste vide ; BuildingTypes.csv (15 lignes) n'a aucun mur ; BeginWallDrag sort donc toujours faux. Le nom `Wall` du scenario est PROVISOIRE. Sans type de mur, ni l'essai ni les fantomes verts ne peuvent apparaitre.
2. GESTE : aucune commande de glisser reel. `hol.Mouse.Click` = deplacement + appui + relache d'un coup (HOLUIMouse.cpp:260) ; `hol.Mouse.Hover` seulement deplacement. Le scenario emploie `hol.Mouse.Drag X1 Y1 X2 Y2 [Frames]` (inexistante) : a ajouter au lot C++ (HOLUIMouse.cpp : appui en X1 Y1, N images de deplacement, relache en X2 Y2 ; chemin HandleSelectPressed / HandleSelectCompleted, HOLPlayerController.cpp:4917 et 4975). Le scenario est donc `GESTE: reel` dans l'intention mais ne se lance pas aujourd'hui. TYPES_A_EXPOSER / commande a ajouter : hol.Mouse.Drag.
3. Deblocage des murs : seule piste = `hol.Cheat.Technology Masonry` (Technologies.csv:19, Stats WallTowerHealth) ; aucune commande F5 ni scenario `wall*`/`fortif*` existant. A confirmer.
4. Taille en pixels : NON MESUREE. Connu seulement : un fantome = une boite plate par case de 4 m, cote 92 % d'une case (HOLWallGhostDrawer.as:100, Side = Tile/100*0.92 en echelle d'un cube de 100 cm). Les pixels se relevent sur la capture.
5. virtuous_knight, journal exact (HOLCombatProcessor.cpp de tool-known-reds, 3165-3180) : le coup sur l'allie s'ecrit `Combat: step <N> #<id ennemi> <Type> hits #<id allie> <Type allie> for <Dealt>, <PV allie> HP left` avec PV allie = avant - Kept ; la part du Chevalier (ApplyDamageRedirect, ~l.750) RETIRE des PV SANS aucune ligne de journal : la baisse ne se voit qu'au panneau (clic sur le Chevalier + hol.HUD.Report). Pourcentage et rayon lus dans UnitTypes.csv (DamageRedirectPercent / Radius), non recopies.

## Commandes preflight exactes (a lancer par la session principale, worktree contenant la branche)
wall_line_long (apres merge de wall-line, type de mur et hol.Mouse.Drag, build fait) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\preflight.ps1 wall_line_long -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
virtuous_knight (apres merge de tool-known-reds, build fait) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\preflight.ps1 virtuous_knight -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
Rejeu agent : `play.ps1 <nom> -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`. Puis `session_gate.ps1 -Phase Essay -Root <meme worktree> -Essay <nom>` (branches du .look).

## NON VERIFIE
Tout : aucun lancement. virtuous_knight : format `E=VirtuousKnight` de hol.HUD.Report suppose d'apres `A=Spearman enabled` ; `hol.Select.ClickUnit 0` est un index suppose ; `waitunits 5` et `hol.Units.SpawnHere 3 Spearman 1 120` copies du modele legions_roster / deploy. Viewport 1920x1080 suppose pour les pixels du glisser. Les deux ont des `waitkey` (3 min : preflight les passe seul en -unattended). Le geste de la touche E (hol.UI.Key E 0 0.1) n'est pas dans virtuous_knight : la grille est seulement montree (geste interne pour le spawn).

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
ANOMALIES_PROCESSUS: aucune ; IN_USE.txt ecrit `libre` (CPP: non) a la sortie
GIT_STATUS_PRINCIPALE: non releve (aucune ecriture dans la copie principale hors ce rapport)
