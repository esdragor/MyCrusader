# GLORFINDEL-1124f : F3 / D900, PendingCasts hors global (script seul)

APPELS_DECLARES: ~14 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 12:40 / voir fin (<= 12:56)
PREMIER_BUILD: 12:42 (en file derriere HeroesOfLiberty-Sauron)   BUILD: voir RESULTAT   LAUNCHCHECK_DEBUT: binaire du worktree perime (dll 11:10 < source 12:42), build lance
TEST_ROUGE: sans objet (correctif de compilation du commit c74f5bf ; tests 66e0be0 inchanges)   TEST_ROUGE_SEPARE: oui (66e0be0 puis c74f5bf)
COMMITS: 1 (849c057, 12:42) + merge main 2e2b9e5 (main 25e29ab) ; merge_main_fait: oui

CAUSE: `TArray<FHOLPendingCast> PendingCasts;` etait un global de namespace dans Script\Factions\HOLFactionPowerServer.as : un global script ne peut pas etre mutable.
CORRECTIF (modele UHOLFollowBookHolder, Script\Orders\HOLFollowOrders.as:25) : `class UHOLFactionPendingHolder : UScriptWorldSubsystem { UPROPERTY() TArray<FHOLPendingCast> Pending; }` declare dans HOLFactionPowerServer.as ; BeginApproach appelle SetPending(UHOLFactionPendingHolder::Get().Pending, Order) ; TickPendingCasts lit Holder = UHOLFactionPendingHolder::Get() (garde nullptr) et utilise Holder.Pending. Les 3 tests (HOLFactionPowerApproachCast_Test.as) utilisent des tableaux locaux : inchanges. Aucun C++.

TESTS: voir RESULTAT ci-dessous. TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun   QUESTIONS: aucune neuve (Retours\Q-lumiere-sacree-approche.md de 1124e reste)
RESTE: crochet C++ d'annulation sur nouvel ordre du heros ; chaine TryCastPower non testee ; HOLPlayerOrders::Move "NOT CALLED BY THE GAME YET" a verifier (voir 1124e).
NON VERIFIE: voir RESULTAT ; DIAGNOSTICS_AS relus par la compilation des scripts seulement ; Get() d'un UScriptWorldSubsystem hors monde de jeu (retourne nullptr ? garde en place dans Tick, pas dans BeginApproach).
ANOMALIES_PROCESSUS: build en file derriere Sauron (attente > 3 min).

RESULTAT: BUILD NON TERMINE a 13:05 (arret dur 12:56 depasse en attendant ; build.ps1 lance 12:42, en file derriere Sauron jusqu'a 12:46, UBT en compilation depuis 12:46:34 : binaire du worktree encore 11:10). NON COMPILE : scripts compiled et les 3 tests F3 NON VERIFIES. Commande pour la session principale : C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\build.ps1 -LiveCoding=0 -Wait puis C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Tests\quick_check.ps1 -Parts tests. PROCESSUS_DETACHES: seulement le build du worktree (PID cmd 8440 / dotnet 26360), non arrete (L25-4). GIT_STATUS_PRINCIPALE: inchange hors docs voulus (Docs\Rapports\GLORFINDEL-1124f.md ajoute).
