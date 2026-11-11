# Valeurs orphelines, 2026-11-11 (Arwen, d751-root, sans C++)

Outil : `Tools/Data/value_consumers.py` (test `value_consumers_test.py`, branché dans `quick_check -Parts values`). Rien n'est corrigé ici, tout est listé.

## 1. Valeur du registre sans consommateur (`--check`) : AUCUNE

Les 171 lignes `code`/`table` (hors « pas codé », `gdd`, `ignore`, tables entières `*`) de `Tools/Data/ValueSheet.csv` ont toutes au moins un lecteur
dans `Source/` ou `Script/` (constante : son nom apparaît ailleurs que dans sa définition ; colonne de table : le mot existe dans le code).
`Tools/Data/orphan_values_known.txt` est donc vide. Un nouvel orphelin fait échouer `values` (`DEAD <id>`), une entrée connue devenue lue aussi (`STALE <id>`).
Limite : méthode textuelle (un nom utilisé seulement dans une chaîne construite ressortirait « mort » : le lister dans le fichier avec la raison ; un nom très courant ne ressort jamais).

## 2. L'inverse : constantes de fichiers que le registre cite, sans ligne au registre (`--uncovered`, rapport seulement)

89 constantes numériques de fichiers déjà cités. La plupart ne se règlent pas (valeurs d'énumération `Kind*`, `Apply*`, `Ok`, `None`, octets de brouillard,
tailles de tableaux). **Candidats à régler** (à trancher par la session principale : ligne au registre ou `note.<Fichier>`) :
`StalledSeconds`, `StrandedSeconds` (HOLAIOpening.as) ; `StepsPerMinute` (HOLRelayRules.as) ; `SymmetryTolerance`, `MinSites`, `MaxSites`, `ZoneMargin`
(HOLSiteLayout.h, HOLRelayLayout.h) ; `Lot`, `BookSize` (HOLMarketRules.h) ; `MaxHeight`, `DefaultHeight`, `ZoomStep` (HOLRTSCamera.h) ;
`MinWalkRateShare`, `MaxWalkRateShare` (HOLUnitAnimation.h) ; `DyingSteps`, `DefaultWindUpPercent`, `AttackMoveRadius`, `MaxAttackers`, `BuildingApproachGap`,
`BuildingWallGap`, `BuildingCloseInProgress`, `BuildingCloseInStopMs`, `Spots`, `Angle` (HOLCombatRules.h).

Liste complète (fichier : noms), produite par `python Tools/Data/value_consumers.py --uncovered` :

- `Script/AI/HOLAIArmy.as` : NeutralOwnerSlot (1092)
- `Script/AI/HOLAIOpening.as` : StalledSeconds (244), StrandedSeconds (338)
- `Script/Biomes/HOLBiomeRules.as` : FreshGrass (88), DrySteppe (89), Autumn (90), DarkForest (91), Snow (92), Count (93), MapDefault (96), KindFlower (99), KindTuft (100), KindBush (101), KindDryGrass (102), KindFallenLeaves (103), KindFern (104), KindFrozenTuft (105), KindBareBush (106), KindCount (107)
- `Script/Economy/HOLTributeRules.as` : Ok (24), NoMarket (25), Self (26), NoRecipient (27), NotEnough (28), BadAmount (29), BadResource (30)
- `Script/Hero/HOLHeroReturn.as` : PriceNotReady (17), PlaceMainForum (20), PlaceNone (21)
- `Script/Morale/HOLMoraleRules.as` : LeverDealt (15), LeverRate (17), LeverCount (18), ApplyNothing (24), ApplyPosed (25), ApplyRefreshed (26), ApplyBlockedByImmunity (27), Kept (118)
- `Script/Relays/HOLRelayRules.as` : None (15), StepsPerMinute (108)
- `Script/Sites/HOLSiteRules.as` : None (19)
- `Source/HeroesOfLiberty/Map/HOLRelayLayout.h` : SymmetryTolerance (53)
- `Source/HeroesOfLiberty/Map/HOLSiteLayout.h` : SymmetryTolerance (53), MinSites (55), MaxSites (56), ZoneMargin (58)
- `Source/HeroesOfLiberty/Map/HOLVisionRules.h` : UnexploredByte (299), ExploredByte (300), VisibleByte (301)
- `Source/HeroesOfLiberty/Match/HOLDiplomacyRules.h` : Enemy (17), Neutral (18), Ally (19)
- `Source/HeroesOfLiberty/Match/HOLMarketRules.h` : Lot (28), TradableResources (30), BookSize (32)
- `Source/HeroesOfLiberty/Player/HOLRTSCamera.h` : MaxHeight (67), DefaultHeight (68), ZoomStep (70)
- `Source/HeroesOfLiberty/Presentation/HOLUnitAnimation.h` : MinWalkRateShare (80), MaxWalkRateShare (81)
- `Source/HeroesOfLiberty/Simulation/HOLCombatRules.h` : DyingSteps (19), DefaultWindUpPercent (80), AttackMoveRadius (139), MaxAttackers (145), BuildingApproachGap (243), BuildingWallGap (251), BuildingCloseInProgress (256), BuildingCloseInStopMs (257), Spots (300), Angle (303), Spots (328), Angle (333), RangedReach (695), DefaultArrowSpeed (698), DefaultAidRadius (707), LastAttackerMemoryMs (718), NeutralAssistPursuit (728)
- `Source/HeroesOfLiberty/Simulation/HOLGatherRules.h` : CellSize (68), NoRoomSourceDistance (186), NearbyMassifDistance (192), MassifStartWeight (200), ContactGap (203), DepositGap (209)
- `Source/HeroesOfLiberty/Simulation/HOLHeroAbilityRules.h` : ShieldsUpArmorBonus (45), HealersHandsHealPercent (48), CloseRanksSpeedPercent (57)
- `Source/HeroesOfLiberty/Simulation/HOLHeroLibrary.cpp` : Index (376)
- `Source/HeroesOfLiberty/Simulation/HOLRelayRules.h` : DefaultDecaySteps (29), RelayXPPerMinute (30), CentreXPPerMinute (31)

