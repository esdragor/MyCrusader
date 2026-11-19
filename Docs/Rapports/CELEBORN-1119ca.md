# CELEBORN-1119ca (reprise 16, vague 16-B) : port script de HOLAIBuildOrdersTests.cpp

NON COMPILE, NON TESTE : aucun build, aucun quick_check, aucun editeur (regle de seance).

HEURE_DEBUT/HEURE_FIN: 16:18 / ~16:23 ; ~17 appels vus
COMMIT: d2546f4 (branche test-world-6, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn), parent ad16f0b (aussi non compile). Non fusionne, non pousse.

## Fonctions de liaison ajoutees (UHOLTestWorldLibrary, namespace script HOLTestWorld ; corps sous WITH_DEV_AUTOMATION_TESTS ; D839 : liaison a exposer)
Enveloppes minces vers la vraie chaine, le monde de test comme contexte de monde (le contexte du script n'atteint pas le monde de test) :
- `int32 CheckAIPlacement(UHOLTestWorldHandle*, int32 Slot, FName TypeName, FVector Point)` -> UHOLAIOrderLibrary::CheckAIPlacement
- `bool FindAIPlacement(UHOLTestWorldHandle*, int32 Slot, FName TypeName, FVector Centre, int32 MaxRings, FVector& OutPoint)`
- `bool FindAIFarmBlock(UHOLTestWorldHandle*, int32 Slot, FVector Centre, int32 MaxRings, FVector& OutMill, TArray<FVector>& OutFarms)`
- `bool IssueAIPlaceOrder(UHOLTestWorldHandle*, int32 Slot, FName TypeName, FVector Point)` (UnitIds {} , bQueue faux, comme le test C++)
- `bool IssueAISetRallyOrder(UHOLTestWorldHandle*, int32 Slot, FVector BuildingLocation, FVector Point)`
- `int32 GetPlacementRefusalBuilding()` -> static_cast<int32>(EHOLPlacementRefusal::Building) (enum non expose ; la valeur vient de l'enum du jeu)
- `bool FarmBlockOverlaps(FVector Mill, const TArray<FVector>& Farms)` -> SnapFootprint des lignes Mill/Farm + HOLConstruction::Overlaps sur toutes les paires (code du test C++ deplace tel quel)
- `bool PayBuildingCost(FName TypeName, const FHOLResources& Stock, FHOLResources& OutAfter)` -> HOLConstruction::TryPay(copie du stock, Type->Cost) ; faux sans ligne
- `bool HasBuildingRallyPoint(const AHOLBuilding*)`, `FVector GetBuildingRallyPoint(const AHOLBuilding*)`

## Tests script : Script\Tests\HOLAIBuildOrders_Test.as (NEUF)
Monde = equivalent de FWorld : GameState (SpawnGameState 1 humain + 1 IA), SpawnAIPlayerStates (JSON slot 0 Human, slot 1 AI, reste ferme), centre owner 1 en (0,0), centre owner 0 en (-20000,0), Init Source/Construction/Production. Echec de monde = "test world ready" faux.
- `Test_AIBuildPlacement` (= HeroesOfLiberty.AI.Build.Placement) : House sur le centre refuse = valeur de l'enum Building ; MainCentre = -1 ; spot House trouve, passe CheckAIPlacement (0), a plus de 1 cm du centre ; MaxRings -1 : rien ; bloc de ferme trouve, 8 fermes, moulin et chaque ferme passent (0), aucun chevauchement des neuf.
- `Test_AIBuildPlace` (= ...Place) : refus sur le centre et sans slot (-1) avant la file ; sur le spot libre : accepte, 0 House avant le pas ; apres 1 pas : 1 House du slot 1 ; stock = stock avant moins le cout de la ligne House (calcule par le jeu, PayBuildingCost) ; 2e spot different ; stock mis a zero : un 2e ordre, apres 1 pas toujours 1 House.
- `Test_AIBuildProduction` (= ...Production) : centre du slot 0 refuse, point vide refuse, Peasant au centre accepte ; file du centre 0 avant le pas, 1 apres ; Food a baisse ; rally refuse sans batiment ; rally accepte ; apres 1 pas HasRallyPoint et point a moins de 1 cm de (3000,1000).
Chaque test detruit son monde en sortie.

## Assertions non portees
- `TestNotNull(the House row)` : la ligne n'est pas exposee ; couvert par PayBuildingCost vrai ("the stock covered it").
- Ordre de creation : le C++ cree les centres AVANT InitVictory, le script cree le GameState (InitVictory) d'abord. Aucune regle n'en depend a ma connaissance (non verifie a l'execution).
- Les comparaisons `ToString()` de FHOLResources deviennent des comparaisons des 4 champs Food/Wood/Stone/Gold (SameRes) : si FHOLResources porte un autre champ, il n'est pas compare.

## FALSIFICATION pour le rouge a voir (sans rebuild, une valeur attendue)
- Script\Tests\HOLAIBuildOrders_Test.as ligne 129 : `T.AssertEquals(1, ...` -> `T.AssertEquals(2, ...` : Test_AIBuildPlace rouge.
- ligne 92 : `T.AssertEquals(8, NumFarms` -> `9` : Test_AIBuildPlacement rouge.
- ligne 163 : `AssertEquals(1, GetProductionQueueCount` -> `2` : Test_AIBuildProduction rouge.
Remettre ensuite les valeurs. (Une falsification de regle script n'est pas possible : les regles testees sont en C++, UHOLAIOrderLibrary / UHOLConstructionSubsystem.)

## Commandes pour la session principale
1. Lot C++ (ce commit + ad16f0b dans le meme build) : `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait`
2. Puis quick_check des parties de tests script (nom de la partie selon l'habitude : les Test_* du dossier Script\Tests) ; chercher `Test_AIBuildPlacement`, `Test_AIBuildPlace`, `Test_AIBuildProduction` dans le journal.
3. Rouge (falsification ci-dessus), puis vert, puis seulement alors retrait des 3 tests C++ de HOLAIBuildOrdersTests.cpp (227 lignes, fichier entier).

## HOLUnitTierProductionTests.cpp : VERIFICATION demandee (rien ecrit)
- Oui, la production finie fait naitre l'unite par Mass : `HOLProductionSubsystem.cpp:652` (`Units->RequestUnits(Request)`, dans AdvanceQueues, ligne 564).
- MAIS le test `HeroesOfLiberty.Production.UnitTier` ne finit jamais une production : il ne diffuse que OnStepStarted (RunStep 1, 2, 3) et lit le nombre d'elements des files (Test.Queues, GetProductionQueue().Num()) et les ressources payees ; UHOLUnitSubsystem n'y sert qu'a FindTypeRow (cout). Donc, pour CE test, pas de naissance Mass : portable apres verdict de la session principale, besoins probables : Barracks/range/stable/chapel/workshop, GetTier (et le cheat de palier), cout d'une unite (FindTypeRow : GetProductionCostResources existe), IssueAI*/commandes. Verdict de portabilite : probable PORTABLE, a confirmer en lisant le monde (lignes 60-135 du .cpp : slots d'IA, tiers). Non commence.

## Champs migration
BUILDS_DEMANDES: 1 (lot C++ : ce commit + ad16f0b)
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 3 (Test_AIBuildPlacement, Test_AIBuildPlace, Test_AIBuildProduction)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: 227 (HOLAIBuildOrdersTests.cpp, 3 tests) ; lot de liaison : +9 fonctions
TYPES_A_EXPOSER: aucun nouveau (EHOLPlacementRefusal reste cache : valeur lue par GetPlacementRefusalBuilding)
TESTS_TAUTOLOGIQUES: 0 (les valeurs attendues viennent du jeu : enum de refus, cout de la ligne House, vraie chaine d'ordres et de pas)

## NON VERIFIE (rien n'est compile)
- C++ : le .cpp et le .h n'ont jamais ete compiles. Risques : (a) `UHOLAIOrderLibrary::IssueAIPlaceOrder(World, ..., {})` : `{}` pour `const TArray<int32>&` comme dans le test C++, devrait passer ; (b) `UHOLBuildingSettings::FindType`, `HOLBuildingTypeNames::Mill/Farm`, `SnapFootprint`, `HOLConstruction::Overlaps/TryPay` : includes deja dans le .cpp (HOLConstructionSubsystem.h, HOLConstructionRules.h, HOLBuildingTypes.h via le .h) mais jamais verifies ensemble ; (c) EHOLPlacementRefusal vient de HOLConstructionRules.h (inclus seulement sous #if, la fonction a un repli).
- Script : `FVector&`, `TArray<FVector>&` en UFUNCTION doivent etre vus comme parametres `&out` ; `AHOLBuilding&out` en parametre d'une fonction script (modele repo : `bool&out` dans HOLVolcanoAltarSquad_Test.as) ; `Farms[Index]` ; `Math::Sqrt` (modele repo ok) ; un `AHOLBuilding Centre;` declare sans valeur puis rempli par &out.
- Comportement : SpawnGameState(1,1) fait InitVictory avec la config par defaut avant SpawnAIPlayerStates avec MakeDefaultJson + SetSlotJson (parametres 3 a 7 copies du modele HOLDisconnect_Test.as : etat 0 ferme, 1 humain, 2 IA ; couleur=slot ; equipe 0). Si InitVictory refuse la config ou si SpawnAIPlayerStates n'engendre pas l'etat du slot 1, `test world ready` echoue (CountPlayerStatesOfSlot(1)==1 : si InitVictory a deja cree l'etat IA, SpawnAIPlayerStates en ferait un 2e et ce compte serait 2 : A VERIFIER EN PREMIER ; le C++ appelait InitVictory puis SpawnAIPlayerStates, comme ici, avec le meme resultat attendu 1 etat par slot).
- Les sets de ressources du slot 1 : le slot 1 doit avoir le stock de depart (StartingResources) pour que la maison soit payable.
- Pas de registre : je suppose que tout `void Test_*(FUnitTest&)` des fichiers de Script\Tests est decouvert (modele HOLDisconnect2_Test.as).

## Anomalies
- Fichiers non suivis Tools\Scenarios\c28_*, c31_* laisses intacts. Le message de commit a ete tronque par rtk dans le journal git (sujet complet : "Script port of the 3 AI build orders tests (HOLAIBuildOrders_Test.as) + 9 HOLTestWorld wrappers for the AI order library (not compiled)").
- PROCESSUS_DETACHES: aucun lance par moi (stray_processes non execute, aucun processus demarre).
