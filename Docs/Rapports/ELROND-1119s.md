# ELROND-1119s - correcteur de script, tests monde de test (lot 1)

Worktree HeroesOfLiberty-Elrond, branche test-world, tip 0e0cb1a (inchange, aucune correction necessaire).

- Passage 1 (10:00 -> 10:02:56) : tests 1622/1622 PASS des le premier essai (attendu 1622). Aucune erreur Angelscript.
- ROUGE VU (10:03:10 -> 10:05:43) : HOLBuildingRegistry_Test.as `AssertEquals(3, Walk.Num(), "three of this world")` -> 4 ; HOLTierCheat_Test.as `AssertEquals(0, Tier, "without the command...")` -> 7. Resultat : tests 1620/1622 FAIL, les 2 tests Angelscript.UnitTests.Tests.HOLBuildingRegistry_Test.Test_BuildingRegistry et HOLTierCheat_Test.Test_TierCheat en echec.
- Valeurs remises par git checkout (fichiers identiques a 0e0cb1a), relance (10:08:00) : 1622/1622 PASS.
- Tests C++ d'origine toujours verts dans le journal : HeroesOfLiberty.Buildings.Registry (Success), HeroesOfLiberty.Match.TierCheat (Success).
- Corrections : 0, donc aucun commit. Aucun blocage C++, aucun plantage de l'editeur (creation/destruction de monde OK).
- Anomalie mineure : un Set-Content a reecrit les fins de ligne avant la remise ; annule par git checkout, working tree propre.
- stray_processes.ps1 -Stop lance en dernier geste.
