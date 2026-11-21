# DIAG-1121se : integ-44, erreurs de compilation Angelscript (worktree HeroesOfLiberty-Sauron)

Heure 20:30 -> 20:37. Correctif : commit d0351d6 sur integ-44 (5 fichiers .as, aucun C++, aucun build).
Journal lu : Saved_Tests\Logs\quick_tests.log (les erreurs sont sur la ligne qui suit "Angelscript: Error:" ; horodatage du log en UTC, 1 h de moins que Get-Date).

## Erreurs (une seule vague, 10 messages sur 5 fichiers)
| # | fichier:ligne | branche d'origine | erreur | correctif |
|---|---|---|---|---|
| 1 | Script/Orders/HOLFollowOrders.as:26 (`FHOLFollowBook PlayerBook;`) | Glorfindel 18d5922 (utilise par Player/HOLControllerCommandsC10a.as:233) | « Global variable 'PlayerBook' must be const. Mutable global variables are not supported » | le livre vit dans un sous-systeme de monde script : `class UHOLFollowBookHolder : UScriptWorldSubsystem { UPROPERTY() FHOLFollowBook Book; }` (HOLFollowOrders.as, hors namespace) ; C10a lit `UHOLFollowBookHolder::Get().Book` (retour false si null). Seule reference de PlayerBook : C10a:233 |
| 2 | Script/Orders/HOLFollowOrders.as:131 et :134 | Glorfindel 18d5922 (liaison HOLUnitScript de 6e42d63) | `GetAttackRange(UObject&, int)` / `GetEnemiesNear(UObject&, ...)` : la liaison script n'a pas le WorldContext (piege 7) | appels sans le contexte ; PickEnemyForUnit(UObject WorldContext, ...) garde sa signature (le parametre n'est plus lu) |
| 3 | Script/Orders/HOLFollowOrders_Test.as:171 et :175 | Glorfindel 5841f40 | idem avec `nullptr` | `GetEnemiesNear(987654, ...)`, `GetAttackRange(987654)` |
| 4 | Script/Fog/HOLStealthOpacityApply.as:11 | Arwen 8f03b93 | « No matching signatures to UHOLUnitOpacityLibrary::SetStealthOpacity » : une UBlueprintFunctionLibrary s'appelle par son nom sans U ni Library (comme HOLUnitScript::, HOLForumMessagesRead::) | `HOLUnitOpacity::SetStealthOpacity(...)` |
| 5 | Script/Fog/HOLUnitOpacity_Test.as (14 appels : lignes 6, 13, 14, 17, 22, 23, 24, 26, 27, 29, 34, 47, 48, 51) | Arwen 8f03b93 | idem (Set/Get/ClearStealthOpacities) | `HOLUnitOpacity::...` |

Pas d'erreur sur `for (int UnitId : SelectedIds)`, `Contains` sur ref const, `OwnSelectedUnitIds`, `Math::Clamp`, double/float32, NewObject/Cast : ces suspects compilent.

## Resultat
quick_check -Parts tests : **2049/2052**. Le module compile (0/0 resolu).

## Rouges restants (3)
1. Angelscript.UnitTests.Orders.HOLFollowOrders_Test.Test_ApplyRightClickOnAllyStartsFollowForTheOthers
2. Angelscript.UnitTests.Player.HOLControllerCommandsC10a_Test.Test_ControllerCommandsC10aRightClickOnAllyStartsFollow
   Lecture : VRAI DEFAUT (contradiction dans la branche Glorfindel, pas l'editeur). `ClassifyRightClick` (HOLFollowOrders.as) renvoie Move quand la cible est dans la selection (`SelectedIds.Contains(TargetId)`), conforme a Test_RightClickOnSelectedAllyIsNotFollow (selection {3,9}, cible 9 -> Move). Mais les deux tests rouges (et les commentaires « ally or own = Follow », « every selected unit but the target ») selectionnent {3,9,4} avec cible 9 et attendent Follow avec 2 suiveurs. Les trois tests ne peuvent pas etre verts ensemble. Choix de design a faire : (A) Follow des autres si la cible est selectionnee, Move seulement quand la cible est la seule unite selectionnee ; il faut alors changer le test « SelectedAlly » en selection {9} ; (B) garder Move et changer les deux tests rouges (cible hors selection). Non tranche ici (design).
3. HeroesOfLiberty.HeroTalent.ShieldsUp (Source/HeroesOfLiberty/Tests/HOLHeroTalentEffectTests.cpp:148) : « no talent: 11 m from the hero, aura, 10.2 » attendu 10.2, obtenu 12.0 ; puis 11.0 au lieu de 9.35. Lecture : VRAI DEFAUT DE TEST C++ (regression de Celeborn 129fa9a). Le combat lit maintenant l'aura seulement par le levier de moral (HOLCombatProcessor.cpp, HOLMorale::TakenPercent), pose par UHOLMoraleProcessor ; Celeborn a ajoute le processeur de moral a HOLHeroAbilityMatchTests.cpp mais pas a HOLHeroTalentEffectTests.cpp, dont le RunStep ne lance que Units + Combat. Correctif C++ (NON FAIT, pas de build) : dans HOLHeroTalentEffectTests.cpp, comme dans HOLHeroAbilityMatchTests.cpp : `#include "Simulation/HOLMoraleProcessor.h"`, `UHOLMoraleProcessor* Morale = NewObject<UHOLMoraleProcessor>(Simulation); Morale->CallInitialize(Simulation, EntityManager.ToSharedRef());` et `UE::Mass::Executor::Run(*Morale, ProcessingContext);` avant le combat dans RunStep. Verifier aussi les autres tests C++ qui font tourner le combat sans le moral.

## Champs agent_rules
APPELS_DECLARES: ~21 | HEURE_DEBUT/HEURE_FIN: 20:30 / 20:37 | PREMIER_BUILD: sans C++ ; BUILD: NON COMPILÉ ; LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 0/0 vu au depart (Integrate_integ-44.log), 2049/2052 apres ; assertit l'effet : sans objet (diagnostic)
COMMITS: 1 (d0351d6, 20:33), merge_main_fait: non | TESTS: quick_check -Parts tests 2049/2052 ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun | QUESTIONS: aucune (la contradiction ClassifyRightClick est decrite ci-dessus pour Glorfindel / la session principale)
NON VÉRIFIÉ: `UHOLFollowBookHolder::Get()` compile mais n'est jamais execute par un test (RightClickStartsFollow n'est couvert que par RightClickUnitOrder avec un livre local) ; DIAGNOSTICS_AS serveur de langage non consulte (journal de l'editeur utilise)
PROCESSUS_DETACHES: aucun lance par moi (quick_check termine) | ANOMALIES_PROCESSUS: aucune | GIT_STATUS_PRINCIPALE: non touche (seul ce rapport ecrit)
