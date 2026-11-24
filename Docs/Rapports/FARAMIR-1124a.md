# FARAMIR-1124a : D900 le héros s'approche, puis lance (zone-effect, sans C++)

Impact joueur : pas encore visible. La règle de calcul de la marche existe et est testée ; le héros ne marche pas encore, car le branchement dans `TryCastPower` attend une liaison C++.

APPELS_DECLARES: 18 (dont 1 Edit refusé « not read »)
HEURE_DEBUT/HEURE_FIN: 10:50:00 / 10:54:40
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: bf056c6 (bouchon + test, branche zone-effect), assertit l'effet : oui (le point d'arrêt et le besoin de marche sont CALCULÉS par la règle, 850 cm et 1150 cm de reste) ; rouge NON VU à l'exécution (pas de quick_check dans l'agent). TEST_ROUGE_SEPARE: oui
COMMITS: 3 (bf056c6 test + bouchon, e40e40c règle, ec9537b journal), dernier à 10:54, merge_main_fait: non (worktree déjà à main 173d31b, rien à fusionner)
TESTS: aucun lancé ; commande pour la session principale : `Tools\quick_check.ps1 -Parts tests` DU worktree Faramir (attendu : bf056c6 rouge sur 2 tests sur 3 : `FactionApproachBeyondRangeWalksToRangeMinusMargin` et `FactionApproachDiagonalStopsOnTheLine` ; `InRangeNeedsNoWalk` passe avec le bouchon ; e40e40c tout vert). TESTS_TAUTOLOGIQUES: 0. TESTS_SCRIPT_AJOUTES: 3 (FactionApproachBeyondRangeWalksToRangeMinusMargin, FactionApproachInRangeNeedsNoWalk, FactionApproachDiagonalStopsOnTheLine). BUILDS_DEMANDES: 0 pour ce commit (le branchement en demandera 1, voir TYPES_A_EXPOSER).
ESSAIS: aucun
QUESTIONS: aucune (les valeurs non tranchées restent des défauts codés : arrêt à Range - 50 cm FAIT ; cvar `hol.Faction.ApproachPays` défaut = payer à l'arrivée, À FAIRE avec le branchement)
TYPES_A_EXPOSER: `UHOLHeroLibrary::GetHeroUnitId(WorldContext, Slot)` (id d'unité du héros du slot, -1 sans héros vivant) : sans lui, aucun ordre de marche ne peut viser le héros depuis `TryCastPower` (`HOLHero::GetHeroLocation` est la seule lecture du héros en script ; `HOLAISummary` laisse le héros de côté, AIBrain.as:164 ; la sélection du joueur ne contient pas forcément le héros). Même fichier `HOLHeroLibrary.cpp/.h` que GLORFINDEL-1124a (E5) : à grouper dans SON lot C++ (un seul build). Alternative sans C++ à trancher par l'utilisateur : le client envoie l'id du héros dans `ServerCastFactionPower` (le serveur le revérifie ?) : moins sûr, non retenu.

FAIT :
- `Script\Factions\HOLFactionPowerServer.as` : struct `FHOLFactionApproach {bNeedsWalk, StopPoint}` + `HOLFactionPowerServer::ApproachPointOf(Origin, Point, RangeCm)` (distance <= portée : pas de marche ; sinon arrêt à (Range - 50) cm du point sur la ligne héros-point).
- `Script\Tests\HOLFactionPowerApproach_Test.as` (neuf) : 3 tests de la règle (2000 cm -> arrêt (850, 0) ; 900 cm -> pas de marche ; diagonale 3:4 -> arrêt à 1150 cm du point, sur la ligne).
- Journal `AubeInProgress.md` (worktree, ec9537b), ligne `EtapeDefini.md`, sous-ligne de la ligne 27 d'`Auto_Journey_File.md`.

RESTE (suite de la file, ordre) :
1. Liaison C++ `GetHeroUnitId` (lot C++ de la séance, avec E5).
2. Branchement dans `TryCastPower` (HOLFactionPowerServer.as:~57, avant `DecideAimed` qui refuse hors portée) : hors portée -> `HOLPlayerOrders::Move(Commands, Slot, [HeroId], StopPoint, false, FVector2D(), false)` (HOLPlayerOrders.as:24) et file d'attente du lancer { Slot, PowerIndex, Point } ; à l'arrivée (héros à <= 1200 cm du point, mesuré par `HeroPlaceOf` à chaque tick de la boucle serveur) : `DecideAimed` + `PutDown` (Honneur et recharge écrits UNE fois, à ce moment) ; un nouvel ordre du joueur (Move, Attack...) annule le lancer en attente ; cvar `hol.Faction.ApproachPays` (défaut = payer à l'arrivée ; autre valeur = payer à l'ordre).
3. Test de chaîne : visée à 2000 cm -> aucun refus, un `Move` émis, zone posée au point cliqué après l'arrivée, Honneur déduit une fois (double de commande à créer ; aucun test existant de `TryCastPower` : modèle `HOLFactionPowerAimed_Test.as`).
4. Suite de l'énoncé : S2-c « Lumière sacrée appelle `ClearMalusAround` » (`PutDown` :123).
- Fichiers pris par d'autres : `HOLHeroLibrary.cpp/.h` (E5, pour la liaison), `HOLFactionAim.as` et `HOLGameScreenScript.as` (E1), `HOLRpcPlayer.as` `ServerCastFactionPower` :1064 (à moi seule, E3 ne touche que `ServerPlaceWallLine`). Marque de la position d'arrêt à l'écran : pas dans ma tâche (E1 laisse la couleur « hors portée »).

NON VÉRIFIÉ : rien n'a été compilé ni exécuté : syntaxe Angelscript des deux fichiers (struct globale avant le `namespace`, `Math::Square`, `Math::Sqrt`, `Math::Abs`, `Math::Max` sur `double`) ; `DIAGNOSTICS_AS`: non relevés (pas de serveur de langage lu). Les attendus du rouge (2 sur 3) sont déduits du bouchon, pas vus.
PROCESSUS_DETACHES: aucun processus détaché pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
ANOMALIES_PROCESSUS: aucune ; 1 `Edit` refusé « not read » (remplacé par un remplacement exact PowerShell, BOM gardé) ; 1 `git commit` lancé hors worktree (cwd réinitialisé) : sans effet (rien d'indexé dans la copie principale), refait avec `git -C`.
GIT_STATUS_PRINCIPALE: ` M Auto_Journey_File.md`, ` M Docs/SessionEnd.txt`, ` M EtapeDefini.md`, ` M Placeholders.txt`, ` M Retours/E-tower_wood.md`, `?? Docs/Rapports/*`, `?? Retours/Q-grille-fine-structure.md` : docs seulement (mes seules modifications : EtapeDefini.md et Auto_Journey_File.md) ; aucun fichier de code touché dans la copie principale : oui.
