# GLORFINDEL-1117c : lot L17-05, port script des tests AlertRing et TerrainSources du minimap

Commit net-playtest (voir git log, `L17-05: script port of the minimap alert ring and terrain sources tests`). Sans C++, aucun build, branche prete. Merge main non fait (inutile : net-playtest = main 2dc0add).

## Fait
- Ajout de 2 tests a Script\Tests\HOLMinimapRules_Test.as (fichier DEJA existant : 8 tests de HOLMinimapRules.as conserves ; j'avais d'abord ecrase le fichier, corrige avant le commit final par reprise du contenu de HEAD~1 + ajout). Test_MinimapAlertRing (HOLMinimapAlertRingTests.cpp : 5 diametres, periode 0, taille < 8, age negatif, 3 cvars, decroissance stricte sur 4 pas) ; Test_MinimapTerrainSources (HOLMinimapTerrainTests.cpp : lecture double, gathered from, used up, another used up, standing again, source ajoutee, source ailleurs, vide stable). Tous les vecteurs du C++ sont portes.
- Rouge VU 00:59 : valeur attendue du « half a period » faussee 16 -> 17 dans mon worktree : tests 1377/1380, seul Test_MinimapAlertRing en plus des 2 rouges D831. Remise a 16. Vert : 1386/1388 (seuls rouges : NomadFirstForum, NomadReplicated, D831). Le total ne baisse pas.
- Detail : le smoke HOLS1hLibraries_Test.as reste (2 tests), il double un sous-ensemble.

## Non porte (la bibliotheque ne l'expose pas)
- Rien de ce que les tests C++ verifient. Seuls ecarts : FHOLSource construit par la bibliotheque (TypeIndex 0 force, Resource/Radius/OwnerSlot non lisibles) ; TestNearlyEqual remplace par une aide script MinimapNear ; IsFinite remplace par x == x et |x| < 1e30 ; la valeur int64 de la signature n'est comparee qu'en egalite.

## Tests C++ retirables (non supprimes)
- HeroesOfLiberty.UI.Minimap.AlertRing (HOLMinimapAlertRingTests.cpp) -> Angelscript.UnitTests.Tests.HOLMinimapRules_Test.Test_MinimapAlertRing
- HeroesOfLiberty.UI.Minimap.TerrainSources (HOLMinimapTerrainTests.cpp) -> Angelscript.UnitTests.Tests.HOLMinimapRules_Test.Test_MinimapTerrainSources

## Champs
- BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 2 (Test_MinimapAlertRing, Test_MinimapTerrainSources) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 97 (42 + 55 C++ portes, retrait au lot FIN) ; TYPES_A_EXPOSER: aucun nouveau (FHOLSource contourne)
- TEST_ROUGE: vu a 00:59 : 1377/1380, Test_MinimapAlertRing ; TESTS_TAUTOLOGIQUES: 0 ; EDITS_REFUSES: 0
- NON VERIFIE : rien (quick_check tests vert a 01:04, 1386/1388)
- PROCESSUS_DETACHES: voir stray_processes (lance en dernier)