# ARWEN-1126m5 (15:37 - 15:44) : regression du vrai clic gauche sur le Mangonneau, cause NON trouvee, correctif non livre

APPELS_DECLARES: ~42 (arret anticipe : plus de piste lisible sans journal C++)
HEURE_DEBUT/HEURE_FIN: 15:37 / 15:44
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 281f3f7 (scenario Tools/Scenarios/arwen_pick.txt/.json, diagnostic) ; vu a 15:41 ; assertit l'effet : oui (la selection du Mangonneau) ; le rouge de reference reste deploy_far (timeout etape 22 waitlog Selection: Mangonel, ARWEN-1126m4)   TEST_ROUGE_SEPARE: non (pas de correctif, donc pas de 2e commit)
COMMITS: 1 (281f3f7, 15:43), merge_main_fait: non
TESTS: `play.ps1 arwen_pick -Hidden -Root ...HeroesOfLiberty-Arwen` (56 s). Journal : apres `hol.Mouse.Click 533 300` (vrai evenement Slate, "on SViewport") `Selection report: selected none` ; apres `hol.Select.Click 0.5 0.5` (SelectAt direct) `Selection: Mangonel #13 of slot 0 (own)` ; `hol.Select.ClickUnit -1 slot=0` idem. TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (diagnostic, pas un essai joueur)
QUESTIONS: aucune
FAIT:
- Preuve que le PICKING n'est pas en cause : SelectAt(0.5, 0.5) et ClickUnit selectionnent le Mangonneau dans le meme scenario, meme camera, meme point. Le defaut est sur le chemin du VRAI clic : Slate (HOLUIMouse.cpp:84-85 ProcessMouseButtonDown/Up) -> Enhanced Input IA_Select (HOLPlayerController.cpp:864-868, Started = HandleSelectStarted, Completed = HandleSelectCompleted).
- Ecartes par lecture (git diff 8d79aaa..77b857a) : GatherPickCandidates / PickAt (aucun changement ; MeshScale 4.5 -> art asset change la hauteur du corps mais le clic est au pied, et SelectAt le trouve) ; HandleSelectStarted / HandleSelectCompleted (inchanges) ; GetCursorFraction (inchange, play -Hidden met TestCursor 0.5 0.5) ; modes armes (FactionAimIndex vaut -1 par defaut, .h:984 ; IsAttackMoveArmed = FactionAimIndex == 100) ; HOLGameScreen.cpp (seul le marche) ; HOLScriptScreen (seul LastBuildingType).
- Aucun commit fautif designe : la bissection par `git log` sur SelectAt ne donne rien (SelectAt n'a PAS change depuis 8d79aaa).
RESTE (pour un agent neuf) :
1. Prouver si HandleSelectStarted est appele : ajouter UE_LOG en tete de HandleSelectStarted et HandleSelectCompleted (C++ : lot C++, HOLPlayerController.cpp:5196 et :5272 environ) ; sans ca, impossible de dire si c'est l'action IA_Select qui ne se declenche pas (mapping, Started+Completed dans la meme image) ou un early-return (IsInputFrozen, IsPlacing, FactionAimIndex >= 0, Alt tenu, bSelectPressed faux car GetCursorFraction faux).
2. Pistes sans C++ a essayer avant : rejouer arwen_pick avec `hol.UI.Key` / un `hol.Mouse.Down` + `frames 2` + `hol.Mouse.Up` (HOLUIMouse.cpp:98-110 a un chemin Down/Up separe) pour voir si Started+Completed dans la meme image est le piege (le delta de timing viendrait d'un changement d'ordre des Tick depuis AR6-d / HOLCombatSoundSubsystem) ; ArtistBinaries/Win64/*.dll et Config/DefaultGame.ini ont change depuis 8d79aaa (non lus : verifier que le binaire du worktree correspond a son Source).
3. Correctif probable C++ (3f : la chaine passe par HOLPlayerController.cpp, premier .cpp) : a decrire apres le log du point 1.
NON VERIFIE: quel handler s'arrete ; que deploy.txt echoue pareil ; le lien avec le bug utilisateur du 11-25 ; le contenu de DA_Art_Mangonel.uasset (binaire).
PROCESSUS_DETACHES: aucun
ANOMALIES_PROCESSUS: aucune ; arwen_pick.look supprime avant commit (copie sans contenu)
GIT_STATUS_PRINCIPALE: M ArtistesInProgress.md, Auto_Journey_Jour.md, EtapeDefini.md, Docs/Rapports/AFTER-GALADRIEL-1126m.txt ; ?? .vscode/, Docs/Rapports : docs deja modifies avant moi : oui
