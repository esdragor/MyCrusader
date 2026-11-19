# GALADRIEL-1119cj : gardes et messages des ponts (branche bridge-guards, 57a6cd7, NON COMPILE)

NON COMPILE : aucun build ni quick_check (consigne). Commande a lancer par la session principale : `Tools\build.ps1 -LiveCoding=0 -Wait` dans `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, puis quick_check des parties Player/Commands/Match. Branche `bridge-guards` partie de integ-20 (4fcb689), un commit (57a6cd7), pas de push, pas de fusion. Debut 18:09, fin vers 18:13. 6 appels vus.

## Point 1 : acces non garde CONFIRME, garde ajoutee
`HOLPlayerController.cpp` (SelectBuildingCategory, ~1912-1962). La garde de liste vide (`Buildings.IsEmpty()` -> retour, ligne 1912) existe, mais elle ne couvre pas `INDEX_NONE` : `Entries` a la taille de `Buildings` (non vide), `NextInCycle` rend `INDEX_NONE` sans planche (ou une reponse script hors plage), puis `Buildings[Index]` et `Entries[Index]` etaient lus sans test.
- :1952-1953 avant : `const int32 Index = NextInCycle(...); AHOLBuilding* Next = Buildings[Index];`
- apres : ajout de `if (!Buildings.IsValidIndex(Index) || !Entries.IsValidIndex(Index)) { return; }` entre les deux (selection deja posee, pas de cycle ni de camera).

## Point 2 : messages vrais (decision inchangee, textes existants intacts)
- `HOLCommandGrid.cpp` LogNoBoardOnce (:51) : si `!IsInGameThread()` -> `CommandGrid: %s refused, not on the game thread` (puis retour) ; sinon le texte EXACT `CommandGrid: %s refused, no script grid board` (`tier_grid_client_run.ps1` le cherche toujours). Reste : `!GEngine` retombe sur l'ancien texte (cas sans realite pratique).
- `HOLPlayerOrders.cpp` LogBoardAbsent (:49) : si `!IsInGameThread()` -> `PlayerOrders: %s refused, not on the game thread` ; sinon `PlayerOrders: %s refused, script board absent` inchange. Limite : un `World` nul (`Find` nul aussi) reste rapporte comme planche absente, faute de pouvoir le distinguer dans `Verdict` (sa signature prend deja le monde, mais `AskIsAcceptedAnywhere` passe nullptr volontairement).
- `HOLConstructionSubsystem.cpp` Place (:433-487) : ajout de `int32 RawVerdict` (reponse du pont avant repli sur -1, pose dans `default:`) ; si `RawVerdict > 5 || < -1` -> `Construction: Place refused, verdict out of range %d` (une fois, drapeau propre `bLoggedRange`) ; sinon le texte `Construction: Place refused, script board absent` inchange. Decision (refus) inchangee.
- `HOLConstructionSubsystem.cpp` Cancel (:537-549) : le journal d'erreur `Cancel refused, script board absent` (une fois) est inchange, mais `Verdict = 1` (qui faisait suivre la ligne trompeuse `cancel of slot %d ignored: not its building under construction`) est remplace par `UE_LOG(Display, "Construction: cancel of slot %d ignored: no script board"); return;`. Meme comportement (l'ancienne branche retournait deja : `Verdict != 0`), seul le texte change. Le journal `Construction: cancel of slot ... not its building under construction` n'est donc plus emis sans planche : verifier qu'aucun test ne cherche cette ligne dans le cas sans planche (non cherche).
Aucune signature d'`Ask*` changee.

## Point 3 : autres cas B
Faits (garde locale d'une ligne, aucun effet en jeu normal) :
- `HOLGameState.cpp` HandleForumFall (:415), HandleKingDeath (:458), DefeatSlot (:697) : `if (GateAnswer == 0)` -> `if (GateAnswer == 0 || !bSlotValid)`. Avant : si la regle script rendait 1 alors que `bSlotValid` est faux, `ForumStates[Slot]` / `SlotDefeats[Slot]` etaient indexes hors tableau. Apres : refus. Aucun changement quand la regle script refuse deja un emplacement invalide (cas attendu).

Laisses (pas une garde locale, ou sans risque) :
1. Grille, `AskPickEvenly` (`HOLPlayerController.cpp:2709-2710`) : deja gere (nullptr), rien.
2. 17 `Ask*` des ordres : distinguer `-1` (planche absente) d'un refus de regle script cote script demande un changement de contrat (bool/int -> enum) : PROPOSITION, non faite. Verifier aussi le fil des appelants IA/Mass de `HOLPlayerOrders::*` (non lu ; serait A si hors fil de jeu).
3. Production (`AskOrderGate` ... `AskStockAfterCancel`), `AskQueueRefund`, `AskMatchEnd` (largeur de `States`/`Teams` : regle script), `AskCanPause` et autres (message identique pour `-1` de planche absente et reponse hors plage script) : necessitent la lecture des corps ou un enum ; PROPOSITION : un enum `EHOLBridgeAnswer { Absent, OutOfRange, Refused, Accepted }` commun aux ponts, retourne par les `Ask*` a verdict.
4. Construction `AskPlace` : `Find(World)` ne teste pas `IsInGameThread` (construction, production, match flow) alors que grille et ordres le testent : incoherence a trancher (propositions, pas faite : changerait le comportement hors fil de jeu).
5. Gravite C de l'audit (`AskDestroy`, `AskQueueTotal`, `AskStockAfterPlace`, `AskCancelRefund`, `AskRubble`, `AskStanceGate` ...) : rien.

## Champs
APPELS_DECLARES: 6 | BUILDS_DEMANDES: 0 | TEST_ROUGE: sans objet (gardes de liaison, pas de regle de jeu) ; test conseille : un test C++ de `NextInCycle` sans planche appele via `SelectBuildingCategory` (non fait) | TESTS_SCRIPT_AJOUTES: 0 | TESTS_CPP_RETIRES: 0 | LIGNES_CPP_SUPPRIMEES: 5 (remplacees) | PROCESSUS_DETACHES: aucun lance
NON VERIFIE : compilation (l'edition `RawVerdict` dans le `switch` et `IsInGameThread` dans HOLCommandGrid.cpp / HOLPlayerOrders.cpp ne sont pas compiles : include `CoreGlobals` normalement present) ; aucun effet en jeu teste.