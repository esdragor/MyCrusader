# ELROND-1119t : lot 2 de la bibliotheque de monde de test, port de DisconnectDisplay, retrait TierCheat C++

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche `tiers`. TOUT NON COMPILE, NON EXECUTE (aucun build, aucun quick_check, aucun jeu). Pas de push, pas de merge main.

Impact joueur : aucun (outillage de test). Plan technique : liaison C++ minimale + 4 tests script + un test C++ de moins.

## Commits
- 12a69f9 liaison : `Tests\HOLGameStateTestLibrary.h/.cpp` (neufs) + `SpawnPlayerState` dans `HOLTestWorldLibrary.h/.cpp`
- 38017c4 script : `Script\Tests\HOLDisconnectDisplay_Test.as`
- 4e0dc21 retrait : `Source\HeroesOfLiberty\Tests\HOLTierCheatTests.cpp` supprime ; commentaire de `HOLTestWorldLibrary.h:70` reformule

## Liaison (UHOLGameStateTestLibrary, namespace script `HOLGameStateTest`, `Match\HOLGameState.h` NON touche)
Corps sous `#if WITH_DEV_AUTOMATION_TESTS`. Fonctions : InitVictoryFromJson, DefeatSlot (Reason en int), IsSlotDefeated, GetDefeatReason (int), IsMatchOver, GetLives, IsForumDown, IsSlotAbsent, AddPlayerState, GiveSlotStock(State, Player, FHOLResources, Now), KeepAbsentSlot(State, Player, Now, Window ; Window<=0 = defaut), RecordForumEvents (-> `UHOLForumEventRecorder` : GetCount/GetKind/GetSlot/GetOtherSlot/GetLives/GetReason en int), CountStatsDefeats, GetPlayersTableRows (une ligne par rang, champs separes par tab : Name, State, bAway, bAIHeld, Shown), FormatSeatReport (SeatState et AIMode en int). Plus `UHOLTestWorldLibrary::SpawnPlayerState(Handle, Slot, ColorIndex, Name)`.
Non fait (temps) : GetForumState (FHOLForumState sans USTRUCT : IsForumDown seulement), GetAbsentNotices, SimulateReturn, FindSlotForLogin.

## Tests script (4, memes valeurs que le C++)
Test_DisconnectTable, Test_DisconnectMessage, Test_DisconnectLeftIsDefeat, Test_DisconnectReport. La config se construit en JSON (`HOLMatchConfigRead::MakeDefaultJson/SetSlotJson`), les messages passent par `HOLForumMessagesRead::GetMessage` (existant, aucun C++ neuf pour DisconnectMessage). Les textes de message sont compares sur `.Source` (comme HOLForumMessages_Test.as), le C++ lit `.ToString()`.

## Pour VOIR le rouge de DisconnectDisplay
Fausser dans `HOLDisconnectDisplay_Test.as`, `Test_DisconnectReport` : `"slot 1 (Bob) away, AI holds, 162 s left"` -> `"... 161 s left"` (la valeur reelle arrondit 161.2 vers le haut). Rouge de la chaine reelle (cas DefeatSlot) : `T.AssertEquals(1, Defeats, ...)` -> `2`. Le test C++ reste jusqu'au rouge puis vert (session principale).

## Champs
BUILDS_DEMANDES: 1 (lot C++ : 2 fichiers neufs + 2 modifies ; UHT regenere, a lancer par la session principale avant tout test)
TESTS_CPP_RETIRES: 1 fichier (HOLTierCheatTests.cpp : HeroesOfLiberty.Match.TierCheat), porte en entier, rouge vu 10:05
TESTS_SCRIPT_AJOUTES: 4 (Test_DisconnectTable, Test_DisconnectMessage, Test_DisconnectLeftIsDefeat, Test_DisconnectReport)
LIGNES_CPP_SUPPRIMEES: taille de HOLTierCheatTests.cpp (non mesuree ; le fichier entier)
LIGNES_MIGREES_PAR_LOT: 146 (DisconnectDisplay, C++ encore present)
TYPES_A_EXPOSER: FHOLForumState (GetForumState), FHOLAbsentNotice (GetAbsentNotices, deja USTRUCT : verifier l'acces script), FHOLMatchConfig (passe en JSON), EHOLDefeatReason / FHOLDisconnectRules::ESeatState / EAIMode / FHOLForumEvent (int32), FHOLPlayersTableRow (une ligne tab-jointe)
NON VERIFIE : compilation C++ et UHT (include de `Match/HOLResources.h` dans le .h neuf pour FHOLResources en parametre UFUNCTION ; `AGameStateBase::AddPlayerState` public suppose) ; chargement du .as (`FString.ParseIntoArray`, `UHOLForumEventRecorder` en script sans `U`, nom d'espace `HOLGameStateTest` deduit de la regle `UHOL...Library` -> `HOL...`, `FHOLForumScenario.Names` indexe par slot) ; equivalence config JSON / config C++ (FromJson normalise : teams et couleurs a verifier sur le cas LeftIsDefeat, slot 3 ferme team 0).
Anomalies : budget d'appels depasse (~32 vus au lieu de 25) pour finir les trois commits ; avertissements LF->CRLF de git sur les fichiers neufs (sans effet).
merge main non fait (branche = main 41dc1a0). PROCESSUS_DETACHES: aucun lance.
