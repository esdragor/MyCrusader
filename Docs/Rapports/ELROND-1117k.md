# ELROND-1117k : MIG-C-PetitesRegles (HeroXP + Monk), branche tiers, NON BUILDE

Commits (tiers, pas de push, pas de fusion) : 7e55694 (lot C++ : `Source\HeroesOfLiberty\Simulation\HOLSmallRulesReadLibrary.h/.cpp`, `UHOLHeroXPReadLibrary` -> `HOLHeroXPRead`, `UHOLHealReadLibrary` -> `HOLHealRead`), aa6fac7 (scripts `HOLHeroXP_Test.as`, `HOLMonk_Test.as`, ne compilent qu'apres le build). `git merge main` fait en premier. Aucun test C++ supprime.

BUILDS_DEMANDES: 1 (lot C++ ci-dessus). TESTS_CPP_RETIRES: 0. TESTS_SCRIPT_AJOUTES: 8 (Test_HeroXPSources, _Tiers, _KillSource, _Levels, _Stats ; Test_MonkHealRules, _HealOrderWalk, _CostDiscount). LIGNES_CPP_SUPPRIMEES: 0. TEST_ROUGE: pas encore (a faire apres build : fausser 120 en 121 dans Test_HeroXPSources et 6 en 7 dans Test_MonkHealRules, quick_check N-2/N, puis vert).
TYPES_A_EXPOSER: aucun nouveau (source XP = int, EOrderWalk = int ; FKillFacts/GetKillXP, FRiseTracker, FHOLHeroSheet non exposes).

## Registre (nom C++ -> test script / a exposer)

| Test C++ | Test script | Etat |
|---|---|---|
| HeroXP.Sources | Test_HeroXPSources | porte, rouge a voir |
| HeroXP.Tiers | Test_HeroXPTiers (+ GetForumLives expose) | porte |
| HeroXP.KillSource | Test_HeroXPKillSource | porte |
| HeroXP.Levels | Test_HeroXPLevels | porte |
| HeroXP.Stats | Test_HeroXPStats | porte |
| HeroXP.Chain | (aucun) | RESTE C++ : table d'unites reelle + HOLCombat::GetDamage/GetMaxHealth + HOLHeroSheet::Make ; a exposer : GetDamage/GetMaxHealth niveau, HOLHeroSheet::Make |
| Monk.Table | (aucun) | RESTE C++ : lit la DataTable UnitTypes (Monk/Spearman, HealRate/HealRange) ; a exposer : lecture d'une ligne de la table d'unites |
| Monk.HealRules | Test_MonkHealRules | porte (candidats = tableaux paralleles) |
| Monk.HealOrderWalk | Test_MonkHealOrderWalk | porte (HealRange 800 en constante ; verifie dans Monk.Table C++) |
| Monk.RiseTracker | (aucun) | RESTE C++ : FRiseTracker a etat (non expose) |
| Monk.CostDiscount | Test_MonkCostDiscount | porte |

Retrait C++ : HOLHeroXPTests.cpp garde Chain, HOLMonkTests.cpp garde Table et RiseTracker : retirer seulement Sources/Tiers/KillSource/Levels/Stats, HealRules/HealOrderWalk/CostDiscount, apres rouge puis vert. Pas Movement (HOLGroupMoveRules + HOLUnitPicking) : non commence (temps).

## Commande de build (session principale)
`C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\build.ps1 -LiveCoding=0 -Wait` puis `quick_check` (parties tests script). Risques a la compilation script : float32 vs double (ChooseTarget passe des `0.0` en float), `Healths[1] = 100.0` sur TArray<float32>, tableaux plats (pas de litteral).
