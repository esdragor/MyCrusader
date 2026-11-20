# GALADRIEL-1120a : D860 H3 (Alt + clic gauche sur la minicarte pose le signal), script seul

APPELS_DECLARES: ~28 (au-dela des 25 demandes : relectures du chemin C++ Alt+clic et du RPC avant d'ecrire ; arret net apres le correctif)
HEURE_DEBUT/HEURE_FIN: 09:03 / 09:06
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build, script seul)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 79a6c27 (test + stub : ClickAction(bRight,bHasOrderable,bAlt) ignore bAlt, ClickSignal jamais rendu) ; assertit l'effet : non pour le test de regle (regle pure, calculee par le code) ; l'effet GameScreen est le scenario signals_minimap_alt (ci-dessous). Rouge NON VU a l'execution.
COMMITS: 2 (79a6c27 test, 851188e correctif), dernier 09:06, merge_main_fait: non (worktree = main 901e391, rien a fusionner)
TESTS: aucun lance ; TESTS_SCRIPT_AJOUTES: 1 (Test_MinimapClickActionAltPlacesSignal, dans Script\Tests\HOLMinimapRules_Test.as ; les 4 assertions de Test_MinimapClickAction passent a 3 arguments) ; TESTS_TAUTOLOGIQUES: 0 ; BUILDS_DEMANDES: 0 ; TYPES_A_EXPOSER: aucun
ESSAIS: signals_minimap_alt (Tools\Scenarios\signals_minimap_alt.txt) : preflight NON lance, pas de .look, pas de PNG vus : LOGIQUE SEULEMENT. A lancer : quick_check -Parts tests, puis play.ps1 signals_minimap_alt -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel (apres preflight).
QUESTIONS: aucune
FAIT / RESTE:
- HOLMinimapRules.as : `ClickSignal = 3` ; `ClickAction(bRight, bHasOrderable, bAlt)` : gauche + Alt = ClickSignal (avec ou sans selection), gauche = camera, droite (Alt ou non) = deplacement ou rien.
- HOLGameScreenScript.as `HandleMinimapPressed` : Alt lu par `Cast<AHOLPlayerController>(GetOwningPlayer()).IsInputKeyDown(EKeys::LeftAlt/RightAlt)` (UFUNCTION BlueprintCallable, PlayerController.h:1580 de UE 5.4 ; `EKeys::` n'etait utilise nulle part dans Script\ : non verifie a la compilation) ; poste `HOLSignal::Neutral` par `AHOLRpcPlayer.ServerPostSignal(Kind, FVector(x, y, 0))` via `HOLRpcBridge::FindPlayerActor` (meme chemin que hol.Signal.Post, HOLControllerCommandsC2.as:171-198). `RequestSignal` du controleur n'est PAS une UFUNCTION (HOLPlayerController.h:557), donc non appelable du script : le RPC script est le plus petit chemin, aucun C++ requis.
- Alt + glisser (bDrag) : aucun signal, `MinimapLastClick = "nothing"`. Le signal est pose au seul appui (bDrag faux).
- Signal du sol = type neutre pour un clic bref (HOLPlayerController.cpp:4803, KindOfAltClick) : la minicarte pose donc toujours Neutral (pas de geste de glisser pour les 5 autres types sur la minicarte).
- RESTE : H2 (animation), D833 (Population pleine 15 s + son court) NON FAIT (hors appels), .look du scenario, preflight.
NON VERIFIE: compilation du script (EKeys::LeftAlt, IsInputKeyDown, AHOLRpcPlayer depuis Script\UI, HOLSignal::Neutral visible depuis HOLGameScreenScript.as), rouge/vert a l'execution, scenario jamais lance ; que hol.UI.Key LeftAlt tenu 3 s soit lu par IsInputKeyDown quand hol.HUD.Click passe par le script (le scenario signals_altclick le fait pour le sol, pas pour la minicarte) ; l'ordre ServerPostSignal depuis un client distant.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune ; depassement du plafond d'appels (28 vs 25).
GIT_STATUS_PRINCIPALE: M EtapeDefini.md, M Placeholders.txt (pas de moi), M SignalsInProgress.md, ?? 5 docs non suivis deja presents au depart : inchange hors docs voulus : oui
