# DIAG-1124a (2026-11-24, 11:29 -> ~11:37)

NON COMPILE (aucun build lance, Sauron intouche ; correctif de test commite, pas compile).

APPELS_DECLARES: ~14 vus
HEURE_DEBUT/HEURE_FIN: 11:29:17 / ~11:37
PREMIER_BUILD: sans C++ (diagnostic)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (correctif de test)   TEST_ROUGE_SEPARE: sans objet
COMMITS: 1 (5088e41 sur ability-columns, worktree HeroesOfLiberty-Glorfindel), merge_main_fait: non

## (1) HeroesOfLiberty.Hero.GetHeroLocationStep : defaut du HARNAIS, pas du correctif
- Le log Integrate_integ-64.log ne contient QUE le nom du test en echec (lignes 35 et 108), aucun message : le texte exact de l'echec n'est pas lisible ; le verdict vient du code.
- Preuve par le code : `UHOLSimulationSubsystem::GetInterpolationAlpha()` renvoie 1 quand `StepSeconds` vaut 0 (HOLSimulationSubsystem.h:65). `StepSeconds` n'est lu des reglages que dans `OnWorldBeginPlay` (HOLSimulationSubsystem.cpp:240) ; le monde de test (`CreateTestWorld` : InitializeActorsForPlay, pas de begin play ; le commentaire de l'en-tete ligne 107 dit « no pipeline, no begin play ») ne le fait jamais, donc alpha = 1 et `MakeUnitView` (HOLUnitSubsystem.cpp:1182) rend Lerp(prev, step, 1) = la place du pas.
- Donc la PRECONDITION du test (« la vue est a plus d'1 uu de la place du pas ») echoue avec l'ancien code ET avec le correctif abe5629 : le rouge du 3dda65b etait un faux rouge de harnais, et le vert ne pouvait pas venir. GLORFINDEL-1124a l'avait dit (« si l'alpha vaut 1, la precondition echoue ») ; le correctif lui-meme (GetHeroLocation lit le `FTransformFragment` de l'entite, garde client NM_Client) est correct a la lecture.
- Aucun crochet de test ne permet de poser alpha (Accumulator / StepSeconds prives ; `RunStepsForTests` exige l'EntityManager, donc un begin play).
- Correctif (5088e41, HOLHeroPlaceTests.cpp seul) : la precondition n'est plus une assertion ; si la vue egale la place du pas, `AddInfo` dit « alpha 1 du harnais ». Les assertions d'effet restent (le heros vivant est lu, X=1000, Y=2000 = transform de l'entite, autre slot : false, monde nul : false).
- CONSEQUENCE HONNETE : avec alpha = 1, le test ne peut plus etre ROUGE sur l'ancien code (vue = pas) ; il garde la valeur d'une regression de garde et de lecture du transform, pas celle de la distinction vue/pas. Pour retrouver un vrai rouge : un crochet C++ dans le lot groupe, p. ex. `UHOLSimulationSubsystem::SetInterpolationForTests(StepSeconds, Accumulator)` sous `WITH_DEV_AUTOMATION_TESTS` (HOLSimulationSubsystem.h, a cote de `AdvanceStepIndexForTests`), puis le test pose alpha = 0.5 et remet l'assertion de precondition. Hors de mes fichiers autorises : A SIGNALER pour le lot C++.
- A lancer par la session principale (un seul appel) : build du worktree Glorfindel puis `quick_check.ps1` du worktree `-Parts tests` ; attendu : HeroesOfLiberty.Hero.GetHeroLocationStep PASS (avec la ligne Info « harness alpha is 1 »). Aucun rouge nouveau attendu.

## (2) HeroesOfLiberty.Morale.ProcessorRunsBeforeCombat : PAS tautologique, et le rouge A ETE VU
- Contrairement a l'enonce : dans Integrate_integ-64.log ligne 35 (passe rouge seul, 6 rouges appliques sur 54f022c), la liste `failed:` contient `HeroesOfLiberty.Morale.ProcessorRunsBeforeCombat` ; a la passe verts (ligne 108) il n'est plus en echec (seul GetHeroLocationStep l'est). Donc rouge VU sur le rouge seul (2198/2207 avec ce test dedans), puis vert apres le correctif 63c4562 (swap Combat/Morale l.41/43 de DefaultGame.ini).
- Ce qu'il lit : `GetDefault<UHOLSimulationSettings>()->SimulationProcessors` (HOLMoraleOrderTests.cpp), la liste chargee depuis l'ini, c'est-a-dire exactement la liste que `UHOLSimulationSubsystem::OnWorldBeginPlay` parcourt (HOLSimulationSubsystem.cpp:247). Il ne recopie aucune constante : modifier l'ini change son resultat (prouve par le rouge/vert ci-dessus).
- Limite (pas un defaut de tautologie) : il verifie l'ordre de la LISTE CONFIGUREE, pas l'ordre d'execution resolu par Mass (`StepPipeline` est « in dependency order » ; HOLMoraleProcessor.cpp:151 a deja `ExecuteBefore HOLCombatProcessor`). Comme GALADRIEL-1124a l'ecrit, l'ordre reel etait donc deja bon avant le correctif : le test garde la liste ini alignee sur le plan, il ne prouve pas un effet de jeu. Un test d'effet exigerait de lire `StepPipeline.Processors` apres begin play (monde de test sans begin play : meme limite que (1)).
- Verdict : OK, aucune ligne `[ ]` ajoutee a Auto_Journey_File.md.

TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (Morale verifie ci-dessus)
ESSAIS: sans objet   QUESTIONS: aucune
NON VERIFIE: le message d'echec exact de GetHeroLocationStep (absent du log ; verdict par lecture du code, pas par execution) ; la compilation de 5088e41 (NON COMPILE, RELECTURE_CPP : AddInfo est une methode de FAutomationTestBase, aucun include ajoute, bloc WITH_DEV_AUTOMATION_TESTS inchange) ; que le test passe.
PROCESSUS_DETACHES: aucun lance par moi (aucun processus demarre ; un `python3` heredoc bloque a ete arrete par TaskStop)
ANOMALIES_PROCESSUS: le worktree Glorfindel porte ` M AubeInProgress.md` non commite (pas de moi, non touche) ; Git Bash `date` diverge de PowerShell (11:31 vs 11:35).
GIT_STATUS_PRINCIPALE: non modifie par moi (seul ce rapport ajoute)
