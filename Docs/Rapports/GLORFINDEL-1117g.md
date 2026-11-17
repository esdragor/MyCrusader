
## Suite GLORFINDEL-1117h (2026-11-17, net-playtest) : rouge vu, tests C++ retires

Rouge : une valeur fausse par test (`20`->`21` dans Test_Production_Queue "cost taken when queued", `10`->`11` dans Test_Production_AdvanceQueue "head started", `50`->`51` dans Test_Production_Tables "50 food") ; `Get-StaleBinaryProblem` vide ; quick_check tests 1391/1396 avec les 3 tests script en rouge. Valeurs remises : 1394/1396 (seuls NomadFirstForum / NomadReplicated, connus). Le rouge n'est pas commite.

| Test C++ supprime | Test script | Lignes C++ supprimees |
|---|---|---|
| FHOLProductionQueueTest (HeroesOfLiberty.Production.Queue) | Test_Production_Queue | 27 (l.36-62) |
| FHOLProductionAdvanceTest (HeroesOfLiberty.Production.Advance) | Test_Production_AdvanceQueue + Test_Production_Progress / _WaitingForRoom / _PopulationCap | 43 (l.64-106) |
| FHOLProductionTablesTest (HeroesOfLiberty.Production.Tables) | Test_Production_Tables | 98 (l.108-205) |

Le reste du fichier (includes, namespace Tests::Production : Flags, noms de types, MakeCost) n'etait utilise que par ces 3 tests (aucune autre reference) : `Source\HeroesOfLiberty\Tests\HOLProductionTests.cpp` supprime en entier (207 lignes au total). Total quick_check attendu : -3 tests (a verifier apres compilation par la session principale).
