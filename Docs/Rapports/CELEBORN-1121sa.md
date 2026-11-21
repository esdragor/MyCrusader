# CELEBORN-1121sa : moral, sous-etape 4 (premier morceau : le fragment sur les archetypes d'unite)

NON COMPILE (aucun build, aucun quick_check, consigne). Impact joueur : aucun a l'ecran ; chaque unite porte desormais l'emplacement des effets de moral (vide), ce que le processeur et le combat liront ensuite.

APPELS_DECLARES: 17 (arret a 25 vus ; premier Edit au 8e appel : le test, par PowerShell sur chaine exacte, L15-3)
HEURE_DEBUT/HEURE_FIN: 19:12:25 / 19:15:19 (arret dur 19:26 respecte)
PREMIER_BUILD: sans build (NON COMPILE, demande de l'enonce)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de compilation par l'agent ; DLL du worktree perimee d'apres l'enonce)
TEST_ROUGE: fd6027d (test seul, avant le correctif 7a7172b) ; rouge NON VU a l'execution ; assertit l'effet : oui (une unite nee par la vraie chaine RequestUnits -> ServeBirthRequests porte le fragment, vide, et une pose par la regle est relue sur l'entite avec une cle d'empreinte non nulle)
COMMITS: 2 (fd6027d test, 7a7172b correctif), dernier a 19:14, merge_main_fait: non necessaire (integ-41 = main 6b226f8)
TESTS: aucun lance. TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (aucun scenario, DLL perimee)
QUESTIONS: aucune
FAIT / RESTE:
- FAIT : `BuildContext.AddFragment<FHOLMoraleFragment>()` dans UHOLUnitTrait::BuildTemplate (Simulation\HOLUnitTrait.cpp) : l'archetype de CHAQUE type d'unite (une configuration par type, HOLUnitSubsystem.cpp:507), donc aussi les copies recues du reseau (HOLUnitReceptionSubsystem) et les gabarits de corps (HOLCorpse) qui reprennent le gabarit.
- FAIT : test C++ HeroesOfLiberty.Morale.UnitEntityCarriesTheFragment (Tests\HOLMoraleRulesTests.cpp) : ManAtArms et Peasant nes dans un monde de test, fragment present et vide, pose par HOLMorale::Apply relue sur l'entite (Total(Taken) = 25, FingerprintKey != 0). Meme gabarit de monde que HOLDamnedWarriorTests.cpp (CreateWorld, RequestUnits, ServeBirthRequests, FindEntity).
- FAIT : un `#endif#endif // WITH_DEV_AUTOMATION_TESTS` en fin de HOLMoraleRulesTests.cpp (avertissement C4067 possible) remplace par un seul `#endif // WITH_DEV_AUTOMATION_TESTS`.
- Empreinte : la cle est 0 pour un fragment vide (HOLSimulationSubsystem.cpp:167-174), donc ajouter le fragment a tous les archetypes ne change pas le hash d'etape tant qu'aucun effet n'est pose (determinisme inchange).
- RESTE : processeur UHOLMoraleProcessor (ligne ajoutee a Auto_Journey_File.md, apres la ligne D866) ; mesure de taille a 2000 unites (D190) ; Peasants : le fragment est present mais la regle les exclut par les destinataires (D705), pas par l'archetype (choix de forme, signale).
RELECTURE_CPP: .generated.h dernier include de HOLSimulationFragments.h (ligne 11, apres HOLMoraleRules.h) : oui ; FHOLMoraleFragment est un struct, `State` public : oui ; bouchon retire (HOLMorale::FingerprintKey est la vraie fonction FNV, plus de `return 0` bouchon) : oui ; accolades de case : aucun switch touche ; nouveaux includes du test (Engine/Engine.h, Engine/World.h, MassEntitySubsystem.h, Simulation/HOLUnitSubsystem.h) : presents.
NON VERIFIE: tout (compilation, rouge puis vert) ; le test cree un monde (UWorld::CreateWorld + InitializeActorsForPlay) comme le test du Guerrier Damne, qui force hol.Combat.Enabled = 1 : ce test ne le force pas (la naissance n'en depend pas, a confirmer) ; que `Request.OwnerPlayer = 0`, `FVector(0,0,0)` et le type `Peasant` existent dans la table (Peasant lu de CLAUDE/GDD, `ManAtArms` vu dans le test du Guerrier) ; `for (const TCHAR* TypeName : { TEXT("ManAtArms"), TEXT("Peasant") })` (std::initializer_list de const TCHAR*, a confirmer au build).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune ; anomalie de fichier : `#endif#endif` (voir plus haut), corrigee.
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, Auto_Journey_Jour.md, EtapeDefini.md, GDD.md, MoraleInProgress.md ; ?? Docs/Concept_Faction_Theatre_2026-11-18.md, HSR_Aha_Allegresse_Nihilux.md, Priorite_Socle_Factions_2026-11-16.md, Rapports/{ARWEN,HALDIR,SAURON}-1121sa.md, ETAPE0-1121s.md ; hors docs voulus inchange : oui (mes seules modifs : MoraleInProgress.md, EtapeDefini.md, Auto_Journey_File.md, ce rapport).

## Commande de build et tests (session principale)
1. `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait` (UN appel bloquant ; Celeborn porte aussi tout le C++ de la sous-etape 1 a 3 non compile : HOLMoraleRules, HOLMoraleLibrary, fragment).
2. `quick_check -Parts tests` depuis le worktree (ou le filtre `HeroesOfLiberty.Morale`).
3. Comptes attendus. Sur le tip fd6027d (test seul, build prealable) : 1 rouge, `HeroesOfLiberty.Morale.UnitEntityCarriesTheFragment` ("ManAtArms carries FHOLMoraleFragment" puis idem Peasant). Sur 7a7172b : 0 rouge parmi les 7 tests `HeroesOfLiberty.Morale.*` (CapsAndOffset, TableRowsThroughTheRule, ApplyNotCumulated, AttenuationAndImmunity, TickClearRemove, FragmentPosesRefreshesExpiresRemoves, UnitEntityCarriesTheFragment). Le rouge sur fd6027d demande de compiler ce tip seul (git checkout fd6027d dans le worktree, build, tests, puis retour sur integ-41) ; sinon se contenter du vert au tip.
4. Si le test de monde echoue pour une autre raison que le fragment (monde sans table des unites chargee), comparer a HeroesOfLiberty.Combat.DamnedWarrior.Table / .Rage du meme build.
