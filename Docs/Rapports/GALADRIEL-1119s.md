# GALADRIEL-1119s : port script de HOLBuildingRegistryTests, HOLTierCheatTests, HOLHeroAbilityOrderTests (reprise 14, 2026-11-19)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` (branche `port-tests`, HEAD `44395a9`). Aucun commit, aucun fichier ecrit dans le worktree : AUCUN des trois fichiers n'est portable sans liaison neuve (regle : pas de C++ ecrit par moi). Resultat utile = la liste exacte des liaisons manquantes.

BUILDS_DEMANDES: 0 / TESTS_CPP_RETIRES: 0 / TESTS_SCRIPT_AJOUTES: 0 / LIGNES_CPP_SUPPRIMEES: 0 / LIGNES_MIGREES_PAR_LOT: 0 / TESTS_TAUTOLOGIQUES: 0 (aucun test ecrit plutot qu'un test tautologique).

## Constat commun (verifie par lecture des en-tetes et grep de `Script\`)
- Aucun test script ne cree de monde : `Script\**\*_Test.as` n'utilisent que `FUnitTest`, des tables (`HOLBuildingType::FindRow`) et le sous-systeme d'ordres SANS monde (`HOLCommandScript::MakeDetachedForTest()`, `HOLCommandLibrary_Test.as`). Le seul `SpawnActor` du script est celui d'un acteur script en jeu (`HOLBiomeApplier.as:99`).
- `AHOLBuilding::Setup`, `AHOLGameState::GetTier / SetTierOverride / GetTierOverride`, `FHOLBuildingIterator`, `UHOLCommandSubsystem::IssueHeroAbilityOrder / ConsumeHeroAbilityOrders`, les sous-systemes hero / simulation / unites : aucun n'est `UFUNCTION` ni dans une `U...ScriptLibrary` (seule `UHOLCommandScriptLibrary` existe et n'a pas d'ordre de capacite de heros).

## Table C++ -> script

### 1. HOLBuildingRegistryTests.cpp (98 l.) : 1 test C++ (`HeroesOfLiberty.Buildings.Registry`), 0 porte
| Cas C++ (TestEqual / TestTrue) | Script | Raison |
|---|---|---|
| no building: none found | laisse en C++ | monde + iterateur |
| four buildings spawned | laisse en C++ | monde + SpawnActor + Setup |
| the same buildings in the same order as TActorIterator | laisse en C++ | iterateur + TActorIterator |
| three of this world | laisse en C++ | idem |
| the other world's alone | laisse en C++ | deux mondes |
| after a destroy: as TActorIterator / the two left | laisse en C++ | iterateur |
| a new one comes last / and TActorIterator agrees | laisse en C++ | iterateur |
| a loop that destroys sees all three / none left / the other world untouched | laisse en C++ | iterateur qui detruit pendant la marche |

Liaisons manquantes (toutes en C++, a ecrire par la session principale, une seule classe d'aide de test) :
- `UFUNCTION(ScriptCallable) static UWorld* UHOLTestWorldLibrary::CreateTestWorld()` et `static void DestroyTestWorld(UWorld* World)` (corps du `MakeWorld` / `DestroyWorld` du test C++ : `UWorld::CreateWorld(Game, false)`, `GEngine->CreateNewWorldContext`, `InitializeActorsForPlay`, puis `DestroyWorldContext` + `DestroyWorld`).
- `static AHOLBuilding* UHOLTestWorldLibrary::SpawnBuilding(UWorld* World, FName TypeName, int32 Owner)` (SpawnActor + `Setup(TypeName, Owner, Owner)`, car `Setup` n'est pas appelable du script).
- `static TArray<AHOLBuilding*> UHOLBuildingRegistryLibrary::GetBuildingsByRegistry(const UWorld* World)` (enveloppe `FHOLBuildingIterator`) et `GetBuildingsByActors(const UWorld* World)` (enveloppe `TActorIterator<AHOLBuilding>`, la reference de l'accord). Le cas « une boucle qui detruit ce qu'elle parcourt » est une propriete de l'ITERATEUR C++ (il doit survivre a `Destroy()` pendant la marche) : un `TArray` rendu d'un coup ne la teste pas ; ce cas doit rester en C++ meme avec ces liaisons (ou exposer `FHOLBuildingIterator` en type valeur script avec `IsValid()/Next()/Get()`).
- Nom du futur fichier script : `Script\Tests\HOLBuildingRegistry_Test.as`.

### 2. HOLTierCheatTests.cpp (48 l.) : 1 test C++ (`HeroesOfLiberty.Match.TierCheat`), 0 porte
| Cas C++ | Script | Raison |
|---|---|---|
| game state spawned | laisse | monde + AHOLGameState |
| without the command the tier is the hero's (0) | laisse | `GetTier` non lie |
| a Ram refused at tier 0 (`HOLSiege::Check == TierTooLow`) | laisse | la regle `Check` est DEJA couverte en script (`HOLSiegeRules_Test.as`, `HOLSiegeCheck::Check`) ; seule l'entree `GetTier` manque |
| forced tier 3 read though hero level 1 / Ram no longer refused | laisse | `SetTierOverride` non lie |
| other slots untouched / report no override (-1) | laisse | `GetTier`, `GetTierOverride` |
| tier 9 held to 3 / forced 1 read / 0 forced is an override / -1 off / slot 99 changes nothing | laisse | `SetTierOverride` (bornes dans le C++ de l'etat) |
| TierOverrides is a replicated property (CPF_Net) | laisse | reflexion `FindPropertyByName` + `CPF_Net` non exposee |

Liaisons manquantes :
- `AHOLGameState` : `UFUNCTION(ScriptCallable) int32 GetTier(int32 Slot) const`, `void SetTierOverride(int32 Slot, int32 Tier)`, `int32 GetTierOverride(int32 Slot) const` (`Match\HOLGameState.h:308-312`), ou une `UHOLGameStateReadLibrary` statique equivalente.
- Un moyen d'obtenir un `AHOLGameState` sans partie : le `CreateTestWorld()` ci-dessus + `static AHOLGameState* SpawnGameState(UWorld*)`, ou mieux un `MakeDetachedForTest()` comme celui des ordres.
- `static bool UHOLGameStateReadLibrary::IsTierOverridesReplicated()` (le cas CPF_Net) ; sinon ce cas reste en C++.
- Fichier script futur : `Script\Tests\HOLTierCheat_Test.as`.

### 3. HOLHeroAbilityOrderTests.cpp (92 l.) : 1 test C++ (`HeroesOfLiberty.HeroAbility.Orders`), 0 porte
| Cas C++ | Script | Raison |
|---|---|---|
| test world ready / slot 0's hero alive | laisse en C++ | Mass : `UHOLUnitSubsystem::RequestUnits / ServeBirthRequests` fait naitre un vrai heros (D838) |
| the cry cast: one cry in the public sheet / ready again 1200 steps / cry pending (ConsumeCloseRanks) | laisse en C++ | `UHOLHeroSubsystem` (BindSteps, GetSheets, ConsumeCloseRanks) + `UHOLSimulationSubsystem::GetOnStepEnded().Broadcast` + Mass |
| ordered again during its cool-down: still one cry / none pending | laisse en C++ | idem |
| no banner at level 1 / level 4: standing 900 steps / place in sheet / ready 2400 / aura no cool-down | laisse en C++ | idem + `SetXP` |
| orders of no player's slot dropped (`INDEX_NONE`, `MaxSlots` -> `Left.Num() == 0`) | PORTABLE des qu'une liaison existe, sur un sous-systeme detache | `IssueHeroAbilityOrder` / `ConsumeHeroAbilityOrders` non lies |

Liaisons manquantes (seul le dernier cas est portable ; le reste est la simulation Mass : reste en C++, D838) :
- `UHOLCommandScriptLibrary` : `static void IssueHeroAbility(UHOLCommandSubsystem* Commands, int32 PlayerIndex, EHOLHeroAbility Ability, const FVector& Location)` et `static TArray<FHOLScriptOrderRead> ConsumeHeroAbilities(UHOLCommandSubsystem* Commands)` (meme patron que `IssueRage` / `ConsumeRages`, `HOLCommandScriptLibrary.h`). `EHOLHeroAbility` est deja un `UENUM(BlueprintType)`. Attention : le cas dit « slot hors table -> rien en file » ; le `MakeDetachedForTest()` ne refuse pas les slots defaites (commentaire de `HOLCommandLibrary_Test.as`) mais le refus INDEX_NONE / MaxSlots est une borne de la file, il devrait donc tenir sans monde (a verifier a la compilation).
- Fichier script futur : `Script\Commands\HOLHeroAbilityOrder_Test.as` (ou `Script\Tests\`).

## Resume des liaisons (a ecrire en un lot C++, un build)
1. `UHOLTestWorldLibrary` : `CreateTestWorld`, `DestroyTestWorld`, `SpawnBuilding`, `SpawnGameState`.
2. `UHOLBuildingRegistryLibrary` : `GetBuildingsByRegistry`, `GetBuildingsByActors` (+ eventuel itérateur valeur).
3. `AHOLGameState` : `GetTier`, `SetTierOverride`, `GetTierOverride` en `ScriptCallable`, + `IsTierOverridesReplicated`.
4. `UHOLCommandScriptLibrary` : `IssueHeroAbility`, `ConsumeHeroAbilities`.

## NON VERIFIE
Rien n'a ete compile ni execute (interdit). Les affirmations « non lie » viennent de la lecture de `Source\HeroesOfLiberty\{Buildings,Match,Commands,Simulation}\*.h` et d'un grep de `Script\*.as` (CreateWorld, SpawnActor, AHOLGameState, AHOLBuilding, GetSubsystem, IssueHeroAbility) ; je n'ai pas verifie `AngelscriptManager` pour une liaison generique de `UWorld` (rien dans les scripts existants n'en utilise). Si le plugin lie `AHOLBuilding` par `USTRUCT/UCLASS` automatiquement, seules les UFUNCTION comptent et elles manquent.

## Anomalies
Aucune. Arret : 10 appels d'outils environ, sous la limite.
