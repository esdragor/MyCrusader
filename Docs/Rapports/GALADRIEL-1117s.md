# GALADRIEL-1117s : liaison HOLVictoryRules et port des 5 tests Victory.Rules

NON COMPILÉ (aucun build, aucun quick_check). Commit f253888 sur galadriel-tab (worktree HeroesOfLiberty-Galadriel), `git merge main` fait en premier geste (3 fichiers de docs), rien poussé.

## Fonctions C++ ajoutées (UHOLVictoryReadLibrary, Source\HeroesOfLiberty\Match\HOLVictoryReadLibrary.h/.cpp)
Namespace script attendu : `HOLVictoryRead`. Pas de nouveau USTRUCT. Les slots sont donnés comme `States` (0 fermé, 1 humain, 2 IA) et `Teams` (par index de slot), complétés à 8 slots (fermé, équipe 0), comme `MakeConfig` du test C++ (même schéma que `UHOLMatchConfigReadLibrary::DrawRandomColors`). FHOLMatchConfig n'est pas ré-exposé.
- `static bool Evaluate(const TArray<int32>& States, const TArray<int32>& Teams, int32 DefeatedMask, int32& OutWinnerMask, TArray<int32>& OutWinners)` : retourne `bOver`, sort `WinnerMask` et `GetWinners()` de `HOLVictoryRules::Evaluate`.
- `static int32 GetCamp(States, Teams, int32 Slot)` : `HOLVictoryRules::GetCamp` (-1 = slot fermé).
- `static bool AreHostile(States, Teams, int32 SlotA, int32 SlotB)` : `HOLVictoryRules::AreHostile`.

## Tests script ajoutés (Script\Tests\HOLVictory_Test.as, Test_VictoryForumLives conservé)
Test_VictoryRulesOneVsOne, Test_VictoryRulesTeams, Test_VictoryRulesFreeForAll, Test_VictoryRulesSingleCamp, Test_VictoryRulesAllDefeated (mêmes vecteurs que le C++, gagnants et masque vérifiés), plus Test_VictoryRulesCampsAndHostility (AreHostile, absent du C++ : 2 slots équipe 0 hostiles, même équipe 2 amis et même camp, équipes 1 et 2 hostiles). Aucun test tautologique : les valeurs viennent du calcul C++.
Aides de fichier (hors Test_) : VictoryList, VictoryMask, VictoryOver, constantes VictoryHuman / VictoryAI.

## Suspects de compilation
- C++ : `HOLVictoryRules::GetCamp` / `AreHostile` / `Evaluate` sont `HEROESOFLIBERTY_API` (OK pour la liaison) ; `Slots[i].Team` supposé int32 (le test C++ l'assigne ainsi) ; `static_cast<uint8>(DefeatedMask)` tronque au-delà de 8 bits (voulu).
- Script : paramètres `TArray<int32>&` de sortie (OutWinners) et `int32&` : liaison attendue comme `HOLMatchConfigRead::Describe` ; `const int` globales de fichier ; `1 << Slots[i]` sur int ; `const FString&` en paramètre avec concaténation de littéraux `Label + "..."` ; paramètre `FUnitTest& T` passé à une fonction d'aide non-Test_ ; `T.AssertEquals(int, int, FString)` comme dans HOLMatchConfig_Test.as.

## Commande de test (session principale)
Build groupé (`Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree HeroesOfLiberty-Galadriel), puis `quick_check -Parts Script` (les 6 nouveaux tests doivent apparaître). Rouge à voir : fausser une valeur attendue (p. ex. le gagnant de Test_VictoryRulesOneVsOne), puis vert ; ensuite seulement retirer les 5 tests C++ Victory.Rules de HOLVictoryTests.cpp (FHOLVictoryOneVsOneTest, TeamsTest, FreeForAllTest, SingleCampTest, AllDefeatedTest ; l'aide MakeConfig/Mask reste utilisée par GameState et Collapse).

## Champs
- BUILDS_DEMANDES: 0 (le lot C++ de liaison est à compiler par la session principale) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 6 (noms ci-dessus) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~60 lignes de test C++ portées
- TYPES_A_EXPOSER: reste HOLForumRules (FHOLForumState, Fall, Advance, Step, Raze...) pour Forum.Rules
- TEST_ROUGE: non vu (pas de build) ; TESTS_TAUTOLOGIQUES: 0
- EtapeDefini.md non touché (consigne) ; IN_USE.txt : OCCUPE à l'entrée (CPP: oui), libre à la sortie.
- NON VÉRIFIÉ : compilation C++ et script, noms exposés au script (HOLVictoryRead), rouge puis vert.
