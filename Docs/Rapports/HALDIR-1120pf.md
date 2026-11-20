NON COMPILE (C++ touche, aucun build, aucun quick_check, aucun editeur lance)

Impact joueur : aucun en jeu ; outil de dev : le scenario neutrals_clairiere devrait enfin montrer le camp sans l'arbre-source de bois devant.

APPELS_DECLARES: ~18 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 16:14 / 16:18 (Get-Date PowerShell)
PREMIER_BUILD: sans build (NON COMPILE)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build demande)
TEST_ROUGE: b74710e (stub qui ignore l'argument + test C++ HeroesOfLiberty.Dev.DecorClearSources) ; assertit l'effet : oui (le registre UHOLSourceSubsystem : source de bois proche debout sans argument, plus debout avec ; arbre lointain et buisson de baies restent) ; rouge NON VU (pas de build)
COMMITS: 3 (b74710e rouge, b806260 corps, 079cb7c merge main) sur wall-line, merge_main_fait: oui (conflit FortificationsInProgress.md : deux lignes d'ajout gardees)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (la valeur assertie est calculee par le registre : IsStanding apres RemoveSource)
ESSAIS: aucun (neutrals_clairiere non rejoue : C++ non compile) ; LOGIQUE SEULEMENT
QUESTIONS: aucune
FAIT / RESTE:
- HOLDecorClear.h/.cpp : FDecorCount::Sources ; CountNear / ClearNear prennent bSources (defaut false) ; WoodSourcesWithin (sources debout, Resource == Wood, dans le rayon) ; ClearNear les retire par UHOLSourceSubsystem::RemoveSource (comme une source epuisee : affichage, navmesh, vue, clients ; serveur ou solo seulement).
- Commande : hol.Decor.ClearNear X Y [R] [Trees|Sources] (mot reconnu a toute place apres Y, insensible a la casse) ; le journal dit maintenant « took N grove actors, N scenery instances and N wood sources ; N left » (le prefixe « Decor.ClearNear 0 0 2500: took » des waitlog est inchange).
- RAYON : le C++ NE PEUT PAS appeler la regle script HOLCampClearing (pas de liaison pour une fonction de namespace .as) : le rayon est le R que passe le scenario (2500, meme nombre que HOLCampClearing::RadiusCm()), dit en commentaire dans HOLDecorClear.cpp et HOLCampClearing.as. Pas de jumeau de la regle : seul le nombre est repete dans la commande du scenario.
- neutrals_clairiere.txt passe Trees (commande, notes, en-tete).
- Autres appelants verifies par grep (Tools\Scenarios\*.txt, Source\) : neutrals, neutrals_3v3, neutrals_minimap, neutrals_spread, spear_arm, keys_rocks_compare, rocks_compare, rocks_compare2, resources_compare : aucun n'a le mot Trees, comportement inchange ; test existant HeroesOfLiberty.Dev.DecorClear : Total() = Actors + Instances + Sources, Sources = 0 sans argument.
- RESTE : compiler (lot C++), quick_check -Parts tests (rouge a prouver : HeroesOfLiberty.Dev.DecorClearSources sur b74710e), rejouer play.ps1 neutrals_clairiere -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir et ouvrir les PNG (l'arbre devant le camp doit avoir disparu).
NON VERIFIE: tout le C++ (compilation de WoodSourcesWithin, World.GetSubsystem<> sur const UWorld&, include HOLSourceSubsystem.h) ; le test (OnWorldBeginPlay du registre sur un monde nu : meme schema que HOLAIBrainTests.cpp:154, autorite supposee vraie en standalone) ; que l'arbre du scenario soit bien Resource == Wood standing (FHOLSource) et non un decor ; l'effet visuel (RemoveSource -> OnSourceUsedUp cache les instances) ; diagnostics Angelscript non relus (seul un commentaire change dans HOLCampClearing.as).
PROCESSUS_DETACHES: voir la sortie finale de stray_processes.ps1 collee ci-dessous
ANOMALIES_PROCESSUS: aucune vue ; IN_USE.txt non pose a l'entree (j'ai ecrit « libre ... CPP: oui » a la sortie seulement)
GIT_STATUS_PRINCIPALE: non releve ; seuls des docs de suivi y sont ecrits (EtapeDefini.md, Auto_Journey_File.md, ce rapport)
