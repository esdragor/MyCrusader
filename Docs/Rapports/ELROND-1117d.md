# ELROND-1117d : L17-12 PARTIEL (HOLProductionTests.cpp -> Script\Tests\HOLProduction_Test.as)

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, branche tiers (base main 2dc0add), branche prete, merge main non fait (inutile : DLL a jour, pas de build).
Commit : voir `git log -1` de tiers (message "Port the read-only production rules tests to script (L17-12 partial)...").

- BUILDS_DEMANDES: 0
- TESTS_SCRIPT_AJOUTES: 3 (Test_Production_PopulationCap, Test_Production_Progress, Test_Production_WaitingForRoom)
- TESTS_CPP_RETIRES: 0 (HOLProductionTests.cpp garde Queue, Advance, Tables)
- LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~7 assertions (6 vecteurs)
- TYPES_A_EXPOSER: FHOLProductionItem (USTRUCT non expose), FHOLProductionCost (struct simple), TFunctionRef (Advance), FHOLResources en entree de TryEnqueue/Cancel
- TEST_ROUGE: vu a 00:59 : 0.5f faussé en 0.6f dans Test_Production_Progress, tests 1386/1389 (REGRESSION Angelscript.UnitTests.Tests.HOLProduction_Test.Test_Production_Progress) ; remis, vert 1387/1389 (00:57 puis 01:01), seuls rouges NomadFirstForum et NomadReplicated (D831). TESTS_TAUTOLOGIQUES: 0 (les valeurs passent par la bibliotheque C++ reelle).
- quick_check -Parts tests : 2 fois vert hors rouges connus. EDITS_REFUSES: 0. PROCESSUS_DETACHES: non lance (aucun processus ouvert par moi).

## Porte (script, memes vecteurs, depart a l'etape 10, 4 pas)
GetPopulationCap(10)=10 ; (400)=175 ; GetProgress(10,12,4)=0.5 ; GetProgress(15,100,4)=1 (plafonne) ; IsWaitingForRoom(10,13,4)=false ; (10,14,4)=true.

## Reste en C++ (HOLProductionTests.cpp)
- Production.Queue (13 assertions) : TryEnqueue (acceptation, coût pris, refus NotEnoughResources, QueueFull), Cancel (remboursement, index hors file) : FHOLProductionItem/Cost/Resources non exposes.
- Production.Advance : tout ce qui passe par Advance (StartStep de la tete, FreePopulation, Done, file vide, 2 unites sorties, "the next one starts at once", "the unit out takes its place") et les lectures de Queue[n].IsStarted/StartStep ; GetStartedPopulation n'est pas teste ici mais reste non expose.
- Production.Tables : toutes les assertions (DataTable UnitTypes, FHOLUnitTypeRow, UHOLBuildingSettings, UHOLMatchSettings) : donnees du moteur, pas des regles pures.

## Assertions C++ redondantes avec le script (a retirer plus tard, non retirees ici)
Dans FHOLProductionAdvanceTest : "half done two steps later" (GetProgress 12.0 = 0.5), "not waiting for room before 100 %" (IsWaitingForRoom 13.0), "it waits at 100 % for a place" (IsWaitingForRoom 14.0), "progress is capped" (GetProgress 100.0 = 1), "population cap: what the buildings give" (10), "population cap: 175 at most (D03)" (400). Le C++ passe un FHOLProductionItem, le script les memes valeurs plates.
Note : Test_ProductionReadLibrarySmoke (HOLS1gLibraries_Test.as, S1g) couvre deja 5 de ces 6 vecteurs ; le nouveau fichier ajoute le plafonnement a 100 % et les separe en tests nommes.

## Docs mises a jour
Auto_Journey_File.md (ligne L17-12), EtapeDefini.md (1 ligne), AngelscriptInProgress.md (1 ligne), IN_USE.txt (libre, CPP: non).
NON VERIFIE : rien de notable ; PlaceholdersInProgress non touche (aucun provisoire).