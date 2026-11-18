# HALDIR-1117s - UHOLGatherReadLibrary + HOLGather_Test.as

NON COMPILÉ, NON LANCÉ. Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branche maps, commit bb0c3d7 (branche prête, main fusionné en premier geste, sans build).

APPELS_DECLARES: ~17 (arrêt à 45)
HEURE_DEBUT/HEURE_FIN: 23:54 / 23:59 (Get-Date)
PREMIER_BUILD: sans build (consigne)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (DLL périmée sans importance, rien lancé)
TEST_ROUGE: non vu (rien lancé) ; assertit l'effet : oui (valeurs calculées par la vraie chaîne HOLGather::*). À faire : fausser une valeur une fois, rouge, puis vert.
COMMITS: 1 (bb0c3d7, 23:59), merge_main_fait: oui (premier geste)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0
BUILDS_DEMANDES: 1 (lot C++ de la séance, voir commande)
TESTS_SCRIPT_AJOUTES: 7 : Test_Gather_Load, Test_Gather_Deposit, Test_Gather_Choice, Test_Gather_Massif, Test_Gather_NearbyMassif, Test_Gather_Spots, Test_Gather_Sanctuary (Script\Tests\HOLGather_Test.as)
TESTS_CPP_RETIRES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~480 (script) + ~330 (liaison C++)

## Fichiers
- Source\HeroesOfLiberty\Simulation\HOLGatherReadLibrary.h / .cpp (nouveaux ; dossier de HOLGatherRules). Espace de noms script : HOLGatherRead.
- Script\Tests\HOLGather_Test.as (nouveau). IN_USE.txt libre.

## Fonctions C++ ajoutées (UHOLGatherReadLibrary, toutes BlueprintPure, enveloppes sans règle)
Un seul USTRUCT, petit : FHOLGatherLoadScript {Resource, Amount, Progress, ProgressResource}, BlueprintReadWrite (nécessaire : la charge est l'état de la règle ; style fonctionnel, la charge entre par valeur et ressort : aucun paramètre in/out).
GetNoRoomSourceDistance, GetNearbyMassifDistance, StartGathering, GatherSteps (N pas de Gather, sorties : pris, perdu, reste de la source), IsFull, IsFullOf, Deposit (stock en TArray<int32> Food,Wood,Stone,Gold ; sorties : déposé, stock), GetEdgeDistance, IsInContact, GetDepositDistance, IsAtDeposit, GetDepositSpot, FindNearestDeposit (tableaux parallèles), GetSanctuaryFarmPercent, GetSourceSpots, GetStandPoints, IsClearOfSpots, IsSpotTaken (FHOLSpotClaims reconstruit à chaque appel depuis les claims passés ; sorties : claims restants, questions de péremption), GetMassifs, GetLinkCount, GetNearbyMassifCount, SearchMassif (FHOLSourceMassifs reconstruit ; « accepté » = index dans la liste Accepted ; OutTried = ordre essayé ; dépôt optionnel).
Positions en FVector2D, dépôt = centre + demi-étendue + yaw, ressource = numéro (0 Food, 1 Wood, 2 Stone, 3 Gold).

## Suspects de compilation
1. C++ : `OutStock = { Resources.Food, ... }` (TArray<int32> init-list, devrait passer) ; `Claims.Add(uint64, 0, Spot)` ; lambda `TFunctionRef<bool(const FClaim&)>` capturant StaleOwners ; FHOLResources(int,int,int,int) supposé (vu dans le test C++).
2. Liaison script : sorties `int32&` / `TArray<int32>&` / `TArray<FVector2D>&` sur des UFUNCTION BlueprintPure (même schéma que GetKillSource de HOLHeroXPRead) ; paramètres `double` ; FVector2D en paramètre const& et en retour (GetDepositSpot) ; USTRUCT FHOLGatherLoadScript passé en `const&` et rendu par valeur.
3. Script : `TArray<int> X = {1,3,0}` (initialiseur), `Math::Cos/Sqrt/Min/Max/Abs`, `TArray.FindIndex/RemoveAt/Contains`, `TArray<float32>` pour les `TArray<float>` C++ (Yaws, Radii), `FHOLGatherLoadScript.Progress` (float32) comparé à un double, `Load = f(Load, ...)` (même variable en entrée et en sortie : la C++ copie d'abord). Si une erreur de compilation casse le module script : supprimer HOLGather_Test.as, rien ne dépend de lui.
4. Si `Resource` d'un champ de la struct est refusé (nom réservé) : renommer.

## Non porté (reste en C++)
Gather.Tables (tables source et unité : trop lourd, deux blocs ; le bloc MainCentre est déjà dans HOLBuilding_Test.as), VeinSizes, LinkedSource, FarmField, Income, 2e moitié de Sanctuary (monde / Mass, D838). Dans Spots : la sémantique « claims persistants » est rejouée en script (tableaux tenus à la main, les claims lâchés retirés avec RemoveAt) ; c'est équivalent pour les vecteurs du test.

## Tests C++ qui deviendront retirables (après rouge vu puis vert du script)
- HeroesOfLiberty.Gather.Load, .Deposit, .Choice, .Massif, .NearbyMassif, .Spots : COMPLETS -> retirables en entier.
- HeroesOfLiberty.Gather.Sanctuary : PARTIEL (les 6 premières lignes, la règle, sont portées ; la suite monde reste) : ne pas retirer le test, au mieux retirer ces lignes.
- HeroesOfLiberty.Gather.Tables : reste.
- Helpers C++ du fichier devenus inutiles après retrait : GatherSteps, MakeSource, JoinIndices (vérifier que VeinSizes etc. ne les utilisent pas avant de les retirer).

## Commande pour la session principale
Lot C++ groupé : `Tools\build.ps1 -LiveCoding=0 -Wait` sur le worktree Haldir (UHT pour le nouvel USTRUCT), puis `quick_check -Parts script` (ou tests) ; rouge : fausser une valeur (ex. `GetSanctuaryFarmPercent` 15 -> 16 dans Test_Gather_Sanctuary) puis remettre. Sur main : `git merge maps` (fichiers neufs seulement, aucun conflit attendu).

## NON VÉRIFIÉ
Tout : aucun build ni lancement. Durée des tests Massif/NearbyMassif (script, 300 reconstructions de grille par SearchMassif/IsSpotTaken dans la boucle des 300 inactifs, ~45 000 paires) : peut être lente ; si trop lente, réduire à 100 inactifs côté script.

PROCESSUS_DETACHES: aucun (aucun processus lancé ; stray_processes non relancé)
ANOMALIES_PROCESSUS: aucune. Content/Maps/Crown.umap non suivi dans le worktree laissé intact (non cohérent avec la tâche, non commité).
GIT_STATUS_PRINCIPALE: non modifié par moi (seul ce rapport ajouté).
