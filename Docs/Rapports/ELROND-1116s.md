# ELROND-1116s : bascule Relay, copie C++ de Step et TerritoryXP supprimee

Commit 6bed6a3 sur tiers (« Relay: script is the only source for Step and TerritoryXP, C++ copy removed »), NON COMPILE, merge main non fait. C++ : oui (suppression du doublon C++ ; D836 point 3 : script source ; Relay.ServerBoard vert prouve par bouchon A FAIRE par la session principale).

- BUILDS_DEMANDES: 0. TESTS_CPP_RETIRES: 0 (HOLRelayRingTests.cpp GARDE en entier : FillPercent et MakeView sont encore appeles par HOLRelaySubsystem::PublishViews, la vue est repliquee). TESTS_SCRIPT_RETIRES: 1 (Test_RelayScriptAndSimulationRulesAgree, 55 296 cas, comparait la copie supprimee). TESTS_SCRIPT_REDUITS: 1 (Test_RelayConstantsAndXPAgreeWithTheSimulation devenu Test_RelayConstantsAgreeWithTheSimulation : ne garde que HoldSeconds / DecaySteps vs constantes C++ encore lues par le sous-systeme ; la part XP retiree). TESTS_SCRIPT_AJOUTES: 0. quick_check : total -1 (le test d'accord retire, exige par la tache).
- LIGNES_CPP_SUPPRIMEES: 152 (net des ajouts : 114 ; les ajouts sont surtout des commentaires et LogNoBoard 11 l.). Tests script : -94 l. +3. LIGNES_MIGREES_PAR_LOT: 0. TYPES_A_EXPOSER: aucun.

## REGISTRE (nom C++ -> nom script ; Grep Source\ et Script\ : aucun appelant C++ restant)
- HOLRelay::Step (HOLRelayRules.cpp, ~55 l.) -> HOLRelay::Step / StepWith (Script\Relays\HOLRelayRules.as), appele par UHOLRelayBoard -> HOLRelayBoardScript.as RelayStep.
- HOLRelay::TerritoryXP (HOLRelayRules.cpp, ~8 l.) -> HOLRelay::TerritoryXP / TerritoryXPAt (script), via RelayTerritoryXP.
- HOLRelay::RelayXPPerMinute, CentreXPPerMinute (HOLRelayRules.h) -> HOLRelay::RelayXPPerMinute / CentreXPPerMinute (script).
- UHOLRelayRulesLibrary::StepRelay, TerritoryXPStep, GetRelayXPPerMinute, GetCentreXPPerMinute (HOLRelayLibrary.h/.cpp) -> retires (seuls appelants : les 2 tests d'accord, retires/reduits).
- GARDES : FHOLRelayState, DefaultHoldSeconds, DefaultDecaySteps (HOLRelaySubsystem.cpp les lit), FillPercent, MakeView (replication), HOLRelayLibrary : GetHoldSeconds, GetDecaySteps (test d'accord reduit), ReadAIRelays, GetAIRelayMode, FHOLAIRelay.
- Pont (HOLRelayBoard.cpp) : plus de repli C++. Sans board script : StepForRelay rend l'etat inchange, TerritoryXPFor rend 0 (carry inchange), UE_LOG(LogTemp, Error) une seule fois ; defauts RelayStep_Implementation / RelayTerritoryXP_Implementation inertes de meme. Test Relay.ServerBoard (HOLRelayBridgeTests.cpp) : comparaisons a la copie C++ remplacees par valeurs attendues a la main (taker 0, held 1 ; 1 XP en 30 pas).
- Commentaires mis a jour : HOLRelayRules.h, HOLRelayLibrary.h, HOLRelayBoard.h, HOLRelaySubsystem.h, HOLSiteRules.h. Commentaires script citant la copie C++ (HOLRelayRules.as l.105-106, 129, HOLSiteRules.as) non retouches (hors budget ; FillPercent reste bien le jumeau C++ de la ligne 105).

## A lancer par la session principale
1. `Tools\build.ps1 -LiveCoding=0 -Wait` dans C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, puis `quick_check -Parts Relay,Script`.
2. TEST_ROUGE : bouchon dans HOLRelayBoardScript.as (`return State;` dans RelayStep) -> Relay.ServerBoard rouge, puis la vraie regle -> vert. Verifier aussi Relays.Ring (inchange).
- NON VERIFIE : compilation C++ (LogNoBoard, UE_LOG avec CoreMinimal ; ToState/ToBoard encore utilises), compilation script (suppression du test d'accord, renommage), tous les tests. Risque : un reference externe (docs) au nom Test_RelayConstantsAndXPAgreeWithTheSimulation.
- PROCESSUS_DETACHES: 0. EDITS_REFUSES: 0 (3 premiers remplacements refuses par erreur de ma chaine, refaits). IN_USE.txt : libre.
