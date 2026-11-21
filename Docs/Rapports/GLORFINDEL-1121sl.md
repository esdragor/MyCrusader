# GLORFINDEL-1121sl (2026-11-21, 20:51 -> 20:54)

NON COMPILE, aucun build, aucun quick_check. Worktree HeroesOfLiberty-Glorfindel, branche ability-columns.
APPELS_DECLARES: 7
HEURE_DEBUT/HEURE_FIN: 20:51 / 20:54
PREMIER_BUILD: sans build (lot C++ de la session principale)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (NON COMPILE)
TEST_ROUGE: ad8f212 (assertit l'effet : oui) ; rouge NON VU a l'execution (stub C++ NativeManualOrder/BP_ManualOrder sans override script = no-op)
COMMITS: 2 (ad8f212 test + stub C++, f92b072 override + branchements), dernier a 20:53, merge_main_fait: non (DLL pas compilee)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ : HOLFollowBridge.h/.cpp, HOLPlayerController.cpp)
TESTS_SCRIPT_AJOUTES: 1 (Test_FollowBridgeManualOrderEndsFollowOfThoseUnits)
TYPES_A_EXPOSER: aucun
DIAGNOSTICS_AS: non releves

## Rouges attendus nom par nom
Sur ad8f212 : Test_FollowBridgeManualOrderEndsFollowOfThoseUnits ROUGE (assert "unit 1 no longer follows after its manual order" : le script n'a pas l'override, BP_ManualOrder ne fait rien ; les deux premiers asserts passent apres le build de 5508632). Les deux tests de 1121sk restent verts attendus sur f92b072. Sur f92b072 : vert attendu.
Commande : Tools\build.ps1 -LiveCoding=0 dans le worktree Glorfindel puis quick_check -Parts tests.

## Fait
- HOLFollowBridge.h/.cpp : NativeManualOrder(TConstArrayView<uint32>) (rien si vide ou sans classe script) + BP_ManualOrder(TArray<int32> par valeur).
- HOLFollowBridge.as : override ManualOrder -> HOLFollowOrders::OnManualOrder(Holder.Book, UnitId) pour chaque id.
- HOLPlayerController.cpp : OrderAt, en tete du bloc `if (!Own.IsEmpty())` (apres les branches chantier/ferme/source, avant attaque/soin/Suivre/sol), un appel NativeManualOrder(Own) : tout clic droit (sol, attaque unit, batiment ennemi, soin) termine le suivi, la branche Suivre recommence ensuite le sien (Begin remplace l'etat). Grille Stop/Hold : appel avant RequestStopUnits.
- RELECTURE_CPP : Own = TArray<uint32> -> TConstArrayView<uint32> implicite ; BP_ManualOrder par valeur comme BP_RightClickOnUnit ; override AS `TArray<int> UnitIds` ; OnManualOrder(FHOLFollowBook&, int) conforme (HOLFollowOrders.as:64) ; State.bActive=false => StepBodyguard rend false (HOLFollowRules.as:35) => Step retire le suiveur des 4 tableaux.

## NON VERIFIE
- Tout (rien compile). Le test suppose States[i].bActive lisible depuis le script (champ declare FHOLFollowRules.as:16).
- Les clics qui rendent la main avant le bloc Own (chantier a aider, ferme/source) ne terminent pas le suivi : un ordre manuel de recolte/construction laisse le Follow actif. A trancher ou etendre.
- Autres chemins d'ordre non branches : RequestMove des raccourcis/minicarte, attaque-deplacement (armee), patrouille, deploiement, capacites. Un seul appel par chemin d'ordre serait mieux place dans les Request* (RequestMoveUnits, RequestAttackUnit...) : non fait (fichier HOLPlayerController.cpp limite a OrderAt et Stop).
- bPlayerOrderActive reste false dans Step.

ANOMALIES_PROCESSUS: aucune
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel