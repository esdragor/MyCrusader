# GLORFINDEL-1124a (D891, place du heros lue au pas de simulation)

APPELS_DECLARES: 24 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 10:49:57 / voir ligne BUILD
PREMIER_BUILD: 10:54   BUILD: voir "BUILD_RESULTAT" ci-dessous (lance en `-Wait`, passe en arriere-plan apres 180 s)   LAUNCHCHECK_DEBUT: non lance (build du lot juste apres, binaire refait)
TEST_ROUGE: 3dda65b (Source\HeroesOfLiberty\Tests\HOLHeroPlaceTests.cpp, `HeroesOfLiberty.Hero.GetHeroLocationStep`) + assertit l'effet : oui (la valeur lue est celle du transform du pas, (1000,2000), contre une vue a (900,1900))   TEST_ROUGE_SEPARE: oui (commit du test seul ; le code actuel sert de bouchon : le test compile sans rien d'autre). ROUGE NON VU A L'EXECUTION (aucun quick_check lance : session principale).
COMMITS: 2 (3dda65b test 10:52, abe5629 correctif ~10:53), merge_main_fait: non necessaire (worktree = 65000ec, merge de main)
TESTS: aucun lance (build seul) ; TESTS_TAUTOLOGIQUES: 0 (la sortie est calculee par GetHeroLocation sur un vrai monde de test + SpawnTestHero, pas recopiee)
BUILDS_DEMANDES: 1   TYPES_A_EXPOSER: aucun   RELECTURE_CPP: voir ci-dessous
ESSAIS: aucun
QUESTIONS: aucune

## FAIT
- `UHOLHeroLibrary::GetHeroLocation` (HOLHeroLibrary.cpp:402) : garde `NM_Client` -> false (comme GetAuthorityHeroes) ; FindHeroUnit + `GetUnitView` gardes seulement pour vivant / hors garnison (Health > 0) ; la POSITION vient de `FTransformFragment` de l'entite (`Units->FindEntity(HeroId)`), le pas, plus la vue interpolee. Includes ajoutes : MassCommonFragments.h, MassEntitySubsystem.h. Commentaire du .h mis a jour.
- Test C++ (neuf, HOLHeroPlaceTests.cpp) : pas de hero -> false et origine rendue ; hero ne en (1000,2000), previous transform a (900,1900), precondition « la vue differe du pas » assertee, GetHeroLocation = (1000,2000), autre slot false, monde nul false.

## RELECTURE_CPP (a regarder a la compilation / au 1er run)
1. `Units->FindEntity` est suppose public (HOLUnitSubsystem.h:135) ; sinon erreur de compilation a cette ligne.
2. `Entities->GetEntityManager()` const : `GetFragmentDataPtr<FTransformFragment>(Handle)` existe en const (meme usage que MakeUnitView, HOLUnitSubsystem.cpp:1170).
3. Test : `Units->FindEntity` et `GetMutableEntityManager()` ; `FTransformFragment::SetTransform` / `GetTransform` (Mass). Si l'alpha du monde de test vaut 1, la vue = le pas et la PRECONDITION du test echoue (faux rouge de harnais, pas du code) ; dans ce cas poser `Previous` ET alpha, ou lire le rouge sur le message.
4. Le garde client n'est pas teste (pas de NetDriver dans le monde de test) : NON VERIFIE.

## RESTE / ANOMALIES
- `UHOLHeroSubsystem::PlantBanner` (HOLHeroSubsystem.cpp:710) lit encore la position par `GetUnitView` (interpolee) pour la portee de 16 m : meme defaut, hors de mon enonce (HOLHeroSubsystem non liste) ; a traiter dans un lot suivant.
- Si la sortie reste fausse apres le correctif (« banner at (1599,-2) refused » du 23/11 15:48) : non teste ici ; `CastOriginOf` non touchee.
- Brief « premier Edit avant le 8e appel » : premier ecrit au 13e appel (lectures necessaires pour trouver l'accesseur de position et le fragment Previous).

NON VERIFIE: compilation, rouge a l'execution, vert apres correctif, garde client, valeur de l'alpha du monde de test.
PROCESSUS_DETACHES: build `Tools\build.ps1 -LiveCoding=0 -Wait` laisse en arriere-plan (tache b5nvwg5f4), non arrete
ANOMALIES_PROCESSUS: l'appel build a ete passe en arriere-plan par l'outil apres 180 s
GIT_STATUS_PRINCIPALE: non lance
BUILD_RESULTAT: en cours au moment du rapport (lance 10:54:03) ; sortie : C:\Users\baudo\AppData\Local\Temp\claude\82f06dab6ac779ae\0584c8ca-a9ce-4837-a887-ba2185dcd361\tasks\b5nvwg5f4.output
