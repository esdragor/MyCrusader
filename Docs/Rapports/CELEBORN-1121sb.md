# CELEBORN-1121sb : moral, sous-etape 4, UHOLMoraleProcessor, tranche 1 (la regle Tick)

NON COMPILE (aucun build, aucun quick_check, consigne). Impact joueur : aucun a l'ecran tant que le processeur n'est pas inscrit dans Config\DefaultGame.ini ; ensuite, les effets de moral temporaires d'une unite s'epuisent au pas fixe et finissent par disparaitre (les auras restent).

APPELS_DECLARES: 18 (arret a 25 vus ; premier Edit/Write au 8e appel, un cran en retard sur la consigne)
HEURE_DEBUT/HEURE_FIN: 19:18:28 / 19:21:20 (arret dur 19:32 respecte)
PREMIER_BUILD: sans build (NON COMPILE, demande de l'enonce)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de compilation par l'agent)
TEST_ROUGE: 9757c95 (test + bouchon seul, avant le correctif 9c2145b) ; rouge NON VU a l'execution ; assertit l'effet : oui (le fragment de l'entite nee par la vraie chaine RequestUnits -> ServeBirthRequests est relu apres chaque pas du processeur lance par UE::Mass::Executor::Run ; valeurs attendues CALCULEES par HOLMorale::Tick sur une copie, jamais tapees)
COMMITS: 2 (9757c95 test + bouchon, 9c2145b correctif), dernier a 19:20, merge_main_fait: non (integ-41 = main 6b226f8 + les deux commits CELEBORN-1121sa ; integ-42 en cours d'integration)
TESTS: aucun lance. TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ de la session principale, avec le fragment de 1121sa)   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun

FAIT / RESTE:
- FAIT : Simulation\HOLMoraleProcessor.h/.cpp neufs (UHOLMoraleProcessor : Server + Standalone, groupe Tasks, ExecuteBefore HOLCombatProcessor, requete FHOLMoraleFragment ReadWrite, bAutoRegisterWithProcessingPhases = false comme le soin). Execute : StepMs = HOLCombat::ToMilliseconds(DeltaTime), puis HOLMorale::Tick(Fragment.State, StepMs) sur chaque unite.
- FAIT : test C++ HeroesOfLiberty.Morale.ProcessorTicksTheFragment (Tests\HOLMoraleRulesTests.cpp) : un ManAtArms ne, effet temporaire 1000 ms (IdA) + aura (IdB) poses par HOLMorale::Apply, un pas -> duree de l'effet = celle de la copie ticquee, aura inchangee ; puis assez de pas pour l'expiration -> IdA parti, IdB reste, Total(LeverDealt) = 10.
- Commit 1 = test + bouchon (Execute vide) ; commit 2 = corps. Le rouge attendu au tip 9757c95 : 1 rouge, `HeroesOfLiberty.Morale.ProcessorTicksTheFragment` (assertions « one step: the temporary effect lost one step of time », « one step: the time left is less than it was », « the temporary effect has ended » ; les deux autres passent avec le bouchon). Au tip 9c2145b : 0 rouge parmi les 8 tests HeroesOfLiberty.Morale.*.
- RESTE 1 (bloquant pour le jeu) : inscrire `+SimulationProcessors=/Script/HeroesOfLiberty.HOLMoraleProcessor` dans Config\DefaultGame.ini (le soin y est a la ligne 42 ; HOLSimulationSubsystem.cpp:246 lit cette liste). Fichier hors de ma liste : non touche.
- RESTE 2 : tranches suivantes : ClearAuras puis auras reposees dans la portee, destinataires (D705), lecture des leviers par le combat (HOLCombatProcessor.cpp, interdit ici), mesure de taille a 2000 unites (D190).
RELECTURE_CPP (L20-2) : .generated.h dernier include du .h : oui ; UCLASS + GENERATED_BODY + public/protected/private : oui ; includes du .cpp (MassCommonTypes.h, MassExecutionContext.h, HOLCombatRules.h pour ToMilliseconds, HOLMoraleRules.h, HOLSimulationFragments.h pour FHOLMoraleFragment) : presents ; signature de ForEachEntityChunk(Context, lambda) et ConfigureQueries identiques au soin : oui ; accolades : relues ; includes du test (MassExecutor.h, MassProcessingContext.h, HOLCombatRules.h, HOLMoraleProcessor.h, HOLSimulationSettings.h, HOLSimulationSubsystem.h) ajoutes : oui ; aucun switch.
NON VERIFIE: tout (compilation, rouge puis vert) ; `Chunk.GetMutableFragmentView<FHOLMoraleFragment>()` renvoie bien un TArrayView ; que Apply pose IdA en emplacement 0 et IdB en 1 (le test relit Effects[0] et Effects[1] apres un pas : si Apply trie autrement, l'assertion « aura inchangee » lit le mauvais emplacement) ; que le test ne demande pas de table des unites ou de reglages (CallInitialize(Simulation, EntityManager) comme le test du Guerrier Damne) ; ExecuteBefore sur un nom de processeur absent de l'executeur du test (inoffensif attendu, le test lance le processeur seul) ; l'ordre ExecuteBefore(Combat) n'a pas ete confronte au graphe de dependances de la simulation (cycle impossible a ma connaissance : le combat n'est pas dit « apres » le moral).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune ; IN_USE.txt non mis a jour (consigne de budget, aucun processus lance) ; avertissement git LF -> CRLF sur les deux nouveaux fichiers (normal).
GIT_STATUS_PRINCIPALE: docs seulement (Auto_Journey_File.md, EtapeDefini.md, MoraleInProgress.md par moi ; le reste est d'autres sessions) : oui.

## Commande de build et tests (session principale)
1. `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait` (un fichier source neuf : un build complet ; apres l'ajout de la ligne de DefaultGame.ini si la session principale la fait).
2. `quick_check -Parts tests` ou le filtre `HeroesOfLiberty.Morale`.
