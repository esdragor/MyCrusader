# SAURON-1119w : garde spectateur des commandes console C2 (worktree HeroesOfLiberty-Sauron, branche envelope)

Impact joueur : un spectateur qui tape `hol.Hero.Return`, `hol.Duel.*`, `hol.Diplomacy.*`, `hol.Signal.Post` ou `hol.Spec.*` n'envoie plus rien au serveur (comme avant le lot C2), au lieu d'un RPC refus� c�t� serveur avec une ligne de journal.

## Garde d'origine (C++, avant 3f55560)
Le garde n'�tait PAS dans les lambdas des commandes (celles-ci bouclaient sur `GetLocalControllers()` et appelaient `Controller->Request<X>()`, lignes retir�es par 3f55560), il �tait en t�te de chaque `AHOLPlayerController::Request<X>` de `Source/HeroesOfLiberty/Player/HOLPlayerController.cpp` (lignes lues dans `3f55560^`) :
- condition exacte : `AHOLPlayerController::IsSpectator()` (cpp:3599) = `HOLPlayerState && HOLPlayerState->IsOnlyASpectator()` ; `return;` silencieux, AUCUNE ligne de journal (ni LogNotSent) ;
- `RequestReturnHero` cpp:5458 (`if (IsSpectator()) return;`) -> hol.Hero.Return ;
- `RequestHeroAbility` cpp:5482 (`IsSpectator() || Ability == Aura`, Aura inatteignable depuis la console) -> hol.Hero.Cast ;
- `RequestDuelChallenge` cpp:5557, `RequestDuelAnswer` cpp:5568 (`if (!IsSpectator()) {...}`) -> hol.Duel.Challenge / Accept / Refuse ;
- `RequestSignal` cpp:5580 -> hol.Signal.Post ;
- `RequestSpecialization` cpp:5742, `RequestCancelSpecialization` cpp:5754 -> hol.Spec.Start / Cancel ;
- `RequestStance` cpp:5796, `RequestShareVision` cpp:5808 -> hol.Diplomacy.Stance / Vision.
- EXCEPTION : `RequestChat` (cpp:5668-5670) n'avait PAS de garde, commentaire d'origine "A pure spectator talks too (with the spectators, D376): the server decides". `hol.Chat.Say` envoie donc aussi pour un spectateur, avant comme apr�s (l'�nonc� la citait dans la liste : elle n'avait pas ce garde).

## Lot C1 (`HOLControllerCommands.as`)
Pas concern� : les `RequestCheat*` retir�s par 4fbd19d (cheats, victoire) n'avaient aucun `IsSpectator` (appel direct `CallCheat`, refus c�t� serveur par `CheatRefused`). Rien perdu, rien chang�. Les commandes C2b (Hero.Rage, Market, Tribute, Prod.*) appellent les `Request<X>` C++ via la biblioth�que : leurs gardes C++ restent.

## Fait
- d90e6f5 (liaison, NON COMPIL�) : `UHOLControllerCommandLibrary::IsSpectatorController(AHOLPlayerController*)` (.h/.cpp), appelle `Controller->IsSpectator()` (publique, `HOLPlayerController.h:605`), faux sans contr�leur ; rien dans `HOLPlayerController.h`. Script : `HOLControllerCommand::IsSpectatorController`.
- e821e73 (script + test, NON COMPIL�) : `HOLControllerCommandsC2::ShouldSend(bSpectator, Route)` pure (spectateur : seulement `ServerPostChat`), appel�e par `Run` avant `Send` (donc ni envoi ni ligne de journal) ; en-t�te mis � jour ; test `Test_ControllerCommandsC2SpectatorSendsNothing` dans `HOLControllerCommandsC2_Test.as` (joueur envoie tout ; spectateur n'envoie rien sauf chat ; Spec.Start / Cancel en dur).

## Rouge � voir (une fois, puis remettre)
Dans `HOLControllerCommandsC2.as`, `ShouldSend` : remplacer `return Route == "ServerPostChat";` par `return true;` -> `Test_ControllerCommandsC2SpectatorSendsNothing` �choue sur "... a spectator sends nothing". (Noter aussi dans l'en-t�te du fichier de test.)

## � lancer par la session principale
Build C++ (liaison nouvelle, un seul build avec le lot) puis `quick_check -Parts` script C2 ; attendu : total +1 test script (1694 -> 1695 avec les autres lots comme compt�s). Rouge puis vert � constater.

## Champs
BUILDS_DEMANDES: 1 (la liaison est du C++ neuf, � grouper) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 1 (Test_ControllerCommandsC2SpectatorSendsNothing) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 ; TYPES_A_EXPOSER: aucun de plus ; TEST_ROUGE: NON VU (aucun build/jeu, consigne) ; PROCESSUS_DETACHES: aucun lanc� ; merge main non fait.
NON V�RIFI� : compilation C++ et script, le test (ni rouge ni vert), le nom `HOLControllerCommand::IsSpectatorController` c�t� script (d�duit du pr�fixe des appels C2b).
Anomalie : `Docs/Concept...` etc. non touch�s ; l'�nonc� listait `hol.Chat.Say` parmi les gard�es, le C++ d'origine ne la gardait pas.
