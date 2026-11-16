# HALDIR-1117b : L17-07..11, 15 tests C++ retires (portes en script)

Branche prete : maps, commit 7a7a541 (worktree HeroesOfLiberty-Haldir), NON COMPILE, sans quick_check. Merge main non fait.
BUILDS_DEMANDES: 0 (a lancer par la session principale : build puis quick_check)
TESTS_CPP_RETIRES: 15
TESTS_SCRIPT_AJOUTES: 0
LIGNES_CPP_SUPPRIMEES: ~560 (HOLDeployRulesTests 296, HOLHeroAbilityTests ~100, HOLRelayRingTests ~37, Orders.Chains ~89, Match.Stats.Scores ~22)
TOTAL quick_check ATTENDU : baisse EXACTEMENT de 15 (les tests script restent).

## Registre : test C++ supprime (fichier::nom) -> equivalent script (fichier::nom)
| # | Test C++ supprime | Equivalent script (verifie par le rapport nomme) |
|---|---|---|
| 1 | HOLOrderChainTests.cpp::HeroesOfLiberty.Orders.Chains | HOLOrderChain_Test.as::Test_OrderChains_Group/_Peasants/_Mix/_Rally/_Bound (ELROND-1117c) |
| 2 | HOLMatchStatsTests.cpp::HeroesOfLiberty.Match.Stats.Scores | HOLMatchStats_Test.as::Test_MatchStats_Scores (ELROND-1117c) |
| 3 | HOLRelayRingTests.cpp::HeroesOfLiberty.Relays.Ring | HOLRelayRing_Test.as::Test_RelayRing (SAURON-1117b) |
| 4 | HOLHeroAbilityTests.cpp::HeroesOfLiberty.HeroAbility.Unlock | HOLHeroAbility_Test.as::Test_HeroAbilityUnlock |
| 5 | HOLHeroAbilityTests.cpp::HeroesOfLiberty.HeroAbility.Cast | HOLHeroAbility_Test.as::Test_HeroAbilityCast |
| 6 | HOLHeroAbilityTests.cpp::HeroesOfLiberty.HeroAbility.Damage | HOLHeroAbility_Test.as::Test_HeroAbilityDamage |
| 7 | HOLDeployRulesTests.cpp::Siege.Deploy.WhoCanDoWhat | HOLDeployRules_Test.as::Test_DeployWhoCanDoWhat (ARWEN-1117b) |
| 8 | ...::Siege.Deploy.DelaysAreTheProfiles | Test_DeployDelaysAreTheProfiles |
| 9 | ...::Siege.Deploy.AttackOrderDecisions | Test_DeployAttackOrderDecisions |
| 10 | ...::Siege.Deploy.MinimumRange | Test_DeployMinimumRange |
| 11 | ...::Siege.Deploy.MoveOrderDecisions | Test_DeployMoveOrderDecisions |
| 12 | ...::Siege.Deploy.Button | Test_DeployButton |
| 13 | ...::Siege.Deploy.Counterweight | Test_DeployCounterweight |
| 14 | ...::Siege.Deploy.SequenceFromOrderToShot | Test_DeploySequenceFromOrderToShot |
| 15 | ...::Siege.Deploy.EveryCombination | Test_DeployEveryCombination |

Total : 15. Fichiers supprimes : HOLRelayRingTests.cpp, HOLHeroAbilityTests.cpp, HOLDeployRulesTests.cpp (les trois ne contenaient que ces tests). Conserves : HOLOrderChainTests.cpp (Orders.BuildingQueue + espace de noms Flags), HOLMatchStatsTests.cpp (Rules, Send, Curves).

## Verifications
- Grep : aucun .cpp/.h n'inclut les 3 fichiers supprimes ; seules references = commentaires de doc dans HOLDeployReadLibrary.h, HOLDeployRules.cpp, HOLHeroAbilityReadLibrary.h, HOLRelayReadLibrary.h (« tests portes en script », obsoletes, a mettre a jour a la main plus tard, sans effet compilateur). Les autres tests Siege.Deploy.* (PlayerButton, PressWhileWalking, InMatch, DeployedAutoFire, Processor) sont dans d'autres fichiers, intacts. Pas de liste explicite de fichiers dans Build.cs.
- Les en-tetes Simulation/HOLCombatRules.h et HOLOrderChainRules.h restent inclus par HOLOrderChainTests.cpp (BuildingQueue).
- Equivalence douteuse : aucune (ELROND note seulement que les lignes « nothing done / unchanged » de Scores sont jugees equivalentes ; accepte sur consigne).

## A lancer (session principale)
build.ps1 -LiveCoding=0 -Wait puis quick_check ; verifier : total baisse de 15 exactement (1385 -> 1370 attendu si 1385 avant, les tests script des branches fusionnees comptes a part).
Placeholders.txt : relu, rien de provisoire touche. PROCESSUS_DETACHES: non lance (aucun processus).