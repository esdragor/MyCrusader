# MIGRATION-1118a (worktree HeroesOfLiberty-Elrond, branche tiers)
- TESTS_CPP_RETIRES: 14 : Victory.Rules.OneVsOne/Teams/FreeForAll/SingleCamp/AllDefeated, Victory.Forum.Rules, Victory.Forum.Hold.Rules, Match.Factions.Tables, Gather.Load/Deposit/Choice/Massif/NearbyMassif/Spots
- LIGNES_CPP_SUPPRIMEES: 911 (Victory 259, Factions 105, Gather 547)
- Helpers retires: Victory::Mask ; Factions::LoadData ; Gather::GatherSteps, MakeSource, JoinIndices ; includes HOLVictoryRules.h (Victory), Misc/FileHelper.h + Misc/Paths.h (Factions). Gardes (encore utilises): MakeConfig/FOccupied/Human/AI, TestStepSeconds, FFixtureTable et tables fixtures.
- Suspects de compilation: HOLVictoryTests.cpp n'inclut plus HOLVictoryRules.h (HOLGameState.h l'inclut, aucun symbole direct restant) ; sinon rien. BUILDS_DEMANDES: 0. Pas de build fait : la session principale compile (build.ps1 -LiveCoding=0 -Wait puis quick_check).
- Sha : voir git log -1 sur tiers.
