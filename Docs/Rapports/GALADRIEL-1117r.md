# GALADRIEL-1117r : port des tests purs de HOLVictoryTests.cpp

- FAIT : worktree HeroesOfLiberty-Galadriel, branche galadriel-tab, commit 1cfc444 (branche prête, rien poussé, merge main non fait : DLL non reconstruite)
- CONSTAT : les 19 tests de HOLVictoryTests.cpp. Les 5 tests Rules (OneVsOne, Teams, FreeForAll, SingleCamp, AllDefeated) sont purs mais appellent HOLVictoryRules::Evaluate / GetCamp et FHOLMatchConfig, NON exposés au script (HOLCampRules_Test.as:330 le disait déjà). Les autres tests (Forum.*, Collapse, GameState) utilisent HOLForumRules (Fall, Advance, Step, Raze, GetPlayersTable, GetSelectionBlock, GetRing...) non exposé, ou un monde/Mass/acteur (D838).
- TESTS_SCRIPT_AJOUTES: 1 (Test_VictoryForumLives dans Script\Tests\HOLVictory_Test.as : GetLives 0/0, 0/1, 0/2, 0/3, 1/0, via HOLHeroXPRead::GetForumLives ; couvre seulement les 5 premières assertions de Forum.Rules)
- TESTS_CPP_RETIRES: 0. Tests C++ prêts à retirer : AUCUN (FHOLVictoryForumRulesTest garde Fall/Advance/Skip/tuning non exposés).
- TYPES_A_EXPOSER (liaison C++ à faire, lot C++ groupé) : HOLVictoryRules::Evaluate + FHOLVictoryResult (bOver, WinnerMask, GetWinners), HOLVictoryRules::GetCamp / AreHostile ; FHOLMatchConfig (construction de slots State/Team) ; HOLForumRules (FHOLForumState, FHOLForumTuning, Fall, Advance, Step, Raze, Skip, GetRebuildProgress, GetStepsToRebuild, GetHoldProgress, DescribeHold, GetRing, GetSelectionBlock, GetPlayersTable). Avec la liaison Victory (Evaluate/GetCamp/AreHostile), les 5 tests Rules deviennent portables : OneVsOne, Teams, FreeForAll, SingleCamp, AllDefeated.
- BUILDS_DEMANDES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~10
- NON VÉRIFIÉ : compilation du .as (aucun build) ; rouge puis vert non vus ; nom Test_ découvert comme HOLHeroXP_Test.as (supposé).
- A lancer par la session principale : build groupé puis quick_check -Parts Script.
