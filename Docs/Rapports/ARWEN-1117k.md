# ARWEN-1117k : MIG-C-Mouvement (Avoid + Sanity), worktree HeroesOfLiberty-Arwen, branche d751-root

Commits : 5f457a2 (lot C++ enveloppes HOLMovementRulesLibrary), ca0b589 (tests script, compilent APRES le build seulement). Aucun test C++ supprime. Pas de push, pas de fusion.
Merge main fait (un INVENTAIRE-JUMEAUX-1117b.md non suivi, identique a main, retire pour que le merge passe). L'ecran Multijoueur de la branche n'est pas touche.

## Decision (D836, D838)
Les regles Avoid sont appelees par entite et par pas (HOLSimulationProcessors.cpp, HOLUnitOrderProcessor.cpp) : le C++ reste la SOURCE. Sanity est lue par l'audit et les memes traces.
Donc : bibliotheque de lecture seulement (enveloppes : copie, un appel, copie), aucun changement de simulation. Classe script `HOLMovementRules::` (UHOLMovementRulesLibrary, ~230 lignes).
Les lambdas du C++ (IsClear) deviennent une table de bool indexee par tour ; les enums des int32.

## Registre (nom du test C++ -> test script / a exposer)
| Test C++ | Test script (Script/Tests) | Envelopes utilisees |
|---|---|---|
| Movement.MarchPairNoTremble | HOLAvoidRules_Test.as Test_AvoidMarchPairNoTremble | NextDetourTurn, TurnWithoutClearWay, TimeToContact, ChooseDetourSide, RemembersMemberDetour, IsMemberDetourOver, Analyse |
| Movement.DetourSide | Test_AvoidDetourSide | ChooseDetourSide |
| Movement.HeadOnAndBlockedPlace | Test_AvoidHeadOnAndBlockedPlace | IsHeadOn, IsPlaceInBody, IsSlotBlockedSeen |
| Movement.DetourTurn | Test_AvoidDetourTurn | NextDetourTurn, EaseBackSteps |
| Movement.CrossingRole | Test_AvoidCrossingRole | ChooseCrossingRole |
| Movement.TimeToContact | Test_AvoidTimeToContact | TimeToContact, TimeNever |
| Movement.NoClearWay | Test_AvoidNoClearWay | TurnWithoutClearWay |
| Movement.DetourFootprint | Test_AvoidDetourFootprint | GetFootprintDistance, LeadsIntoFootprint |
| Movement.LoopPatience | Test_AvoidLoopPatience | StepLoopPatience |
| Sanity.Spin | HOLSanityRules_Test.as Test_SanitySpin | JudgeTrack |
| Sanity.Shake | Test_SanityShake | JudgeTrack |
| Sanity.Motion | Test_SanityMotion | Analyse |
| Sanity.StallFinalLeg | Test_SanityStallFinalLeg | StallLimitSeconds, StepStall |
| Sanity.NoStrike | Test_SanityNoStrike | StepNoStrike |
| Sanity.Colour | Test_SanityColour | FindColourClashes |
| Sanity.Stall (lignes 243-515 du C++) | PAS PORTE (non lu a fond dans le temps) | a exposer : HOLCombat::FBody, HasFreePlaceAround, IsBoxedIn (+ FStall.Step deja la) |

15 des 16 tests ecrits en script, memes vecteurs. Ecarts connus : MarchPair lit en C++ les cvars hol.Units.AvoidTime / AvoidMaxAngle / MarchMemberDetourSteps ; le script en ecrit les defauts livres (0.5, 60 -> 6 tours, 8 ; un FConsoleVariable script CREE une variable, il ne lit pas celle du jeu). Si un defaut change, le test C++ le voit, pas le script.
Fonctions exposees sans test dedie : FStall.bFinalLegRun, HOLSanity::FindColourClashes seul cas, FSanityRules (utilisees par defaut).

## Suite (apres le build)
1. Build : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\build.ps1 -LiveCoding=0 -Wait`
2. quick_check -Parts script : erreurs de compilation script a corriger (non compile ici : AS jamais vu ; points a surveiller : `FAvdMarchWalker&` en parametre inout, `String.StartsWith/Contains`, `Math::RoundToInt(double)`, fonctions declarees apres usage).
3. Rouge VU par test (valeur attendue faussee une fois), puis vert, puis seulement retirer les tests C++ ci-dessus (HOLAvoidTests.cpp 9, HOLSanityTests.cpp 6) ; ne PAS retirer Sanity.Stall.
4. Le total de quick_check ne doit pas baisser.

## Champs
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 15 (9 Avoid + 6 Sanity, noms ci-dessus) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 (bibliotheque de lecture, +373 lignes C++, +712 script) ;
TYPES_A_EXPOSER: HOLCombat::FBody + HasFreePlaceAround + IsBoxedIn (pour Sanity.Stall) ; TEST_ROUGE: non vu (pas de build) ; NON VERIFIE: compilation C++ et script.
