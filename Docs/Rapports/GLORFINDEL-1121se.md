# GLORFINDEL-1121se (2026-11-21, 19:37 -> ~19:41)

NON COMPILE : script seul, aucun build, aucun quick_check, aucun test lance. Le C++ de 1121sd (UHOLUnitScriptLibrary) reste lui aussi non compile.

APPELS_DECLARES: ~18 vus (limite brief 25 ; premier Edit/ecriture au 7e appel)
HEURE_DEBUT/HEURE_FIN: 19:37 / 19:40
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: df819f8 (3 tests + stub PickEnemyInRange qui renvoie -1), assertit l'effet : oui (id choisi, Attack mise dans la file par Step). Rouge NON VU a l'execution.
COMMITS: 2 (df819f8 test + stub, ca47be0 regle), dernier a 19:39, merge_main_fait: non (branche ability-columns, tip de 1121sd ; DLL du worktree non verifiee, la session principale synchronise)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (les ids attendus sont calcules par PickEnemyInRange a partir de positions, le 3e test passe par Step -> HOLPlayerOrders -> file d'ordres)
TESTS_SCRIPT_AJOUTES: 3 (Test_FollowOrdersPickEnemyInRangeTakesTheNearest, Test_FollowOrdersPickEnemyInRangeNoneInRange, Test_FollowOrdersBodyguardAttacksThePickedEnemy), Script\Orders\HOLFollowOrders_Test.as
BUILDS_DEMANDES: 0 de plus (le lot C++ de 1121sd suffit : `Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree Glorfindel, puis `quick_check -Parts tests`)
TYPES_A_EXPOSER: HOLUnitScript::GetEnemiesNear(WorldContext, int UnitId, float Radius, TArray<int>& OutIds, TArray<FVector>& OutLocations) (ennemis vivants de l'unite, par HOLUnitView / HOLCombat) ; HOLUnitScript::GetAttackRange(WorldContext, int UnitId) -> float (portee du type, cm). Ces deux lectures alimentent PickEnemyInRange puis Step (bEnemyInRange / EnemyId).
ESSAIS: aucun   QUESTIONS: aucune

## Decision GDD citee
D818 (Ordres Patrouille, Suivre et focus, § 7.3, 2026-11-15) Q2 = B : Suivre un ALLIE = garde du corps (reste pres de lui et attaque les ennemis a PORTEE) ; Suivre un ENNEMI = l'attaquer (identique a Attaque). HOLFollowRules::Decide prenait deja bEnemyInRange ; ce qui manquait en regle pure est le choix de l'ennemi a portee.

## Rouges attendus nom par nom
- Sur df819f8 (stub -1) : ROUGE Test_FollowOrdersPickEnemyInRangeTakesTheNearest (attend 34, recoit -1) ; ROUGE Test_FollowOrdersBodyguardAttacksThePickedEnemy (0 ordre au lieu de 1 : Enemy = -1, le garde du corps n'attaque pas) ; VERT par construction Test_FollowOrdersPickEnemyInRangeNoneInRange (le stub renvoie -1 : ce test garde la regle contre un correctif qui renverrait un id hors portee, il ne prouve pas le rouge).
- Sur ca47be0 (tip) : les 3 VERTS ; les cinq Test_FollowOrders* de 1121sd inchanges ; Test_FollowOrdersReadUnitUnknownIdIsGone reste dependant du lot C++ (HOLUnitScript, non compile).

## FAIT
- Script\Orders\HOLFollowOrders.as : HOLFollowOrders::PickEnemyInRange(UnitLocation, Range, EnemyIds, EnemyLocations) : distance <= Range, le plus proche gagne, -1 si aucun ou liste vide, listes de tailles inegales : le plus court borne.
- Journal OrdersPatrolInProgress.md l.38, EtapeDefini.md l.26, Auto_Journey_File.md l.130.

## RESTE
1. Lot C++ : GetEnemiesNear + GetAttackRange (voir TYPES_A_EXPOSER), puis compiler avec le lot de 1121sd.
2. Morceau 3 (pas d'ordre en cours) et morceau 4 (clic droit sur unite dans HOLPlayerController, relecture seule).
3. Verifier les noms des tests a l'execution : le dernier test construit un Step avec un allie a 100 cm (dans FollowDistance 300) : Decide doit rendre None sans ennemi (lu dans HOLFollowRules.as l.33-43, a confirmer au vert).

## NON VERIFIE
Compilation script (Math::Min sur int dans le namespace, `const float Distance` comparee a un float BestDistance : doubles en script), execution des 3 tests, signature `const TArray<FVector>&` acceptee. DIAGNOSTICS_AS: non releves (serveur de langage non consulte). Fin de ligne : les deux fichiers .as ont ete ecrits avec les fins de ligne detectees ; git a annonce « LF will be replaced by CRLF » (fichier a fins mixtes possible, eol_check.ps1 -Fix si un Edit multi-lignes refuse).
PORTEE : Range est mesuree entre les deux positions (centres), sans rayon des unites : le C++ GetAttackRange devra dire s'il y ajoute un rayon (a fixer avec HOLCombat).

IN_USE.txt : non touche (aucun build). Placeholders.txt : aucun provisoire ajoute ou retire.
PROCESSUS_DETACHES: voir sortie de stray_processes en dernier geste (aucun lance par moi)
ANOMALIES_PROCESSUS: 3 ecritures PowerShell refusees ou a refaire (alias `rd` de ma fonction, 2 regex d'ancrage : « return -1; » existe 2 fois dans le fichier)
GIT_STATUS_PRINCIPALE: Auto_Journey_File.md, EtapeDefini.md, OrdersPatrolInProgress.md modifies par moi (docs voulus), plus les fichiers deja modifies/non suivis par d'autres sessions ; aucun fichier de code touche dans la copie principale.
