# CELEBORN-1119a : NG-6b, retrait du C++ mort de l'ecran Nouvelle partie

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, branche hero-j8-close, commit 0d8f26b (base main 20a5e2f). NON COMPILE, aucun quick_check, aucun play.ps1. Branche prete, merge main non fait (DLL perimee).

Impact joueur : aucun attendu ; l'ecran Nouvelle partie reste celui du script (valide le 2026-11-19 00:40), seul le C++ mort disparait.

## Retire (HOLNewGameScreen.cpp 1906 -> 1413 = -493 ; .h 425 -> 400 = -25)
- BuildLayout : toute la mise en page C++ (ex-243-352, ~120 lignes) ; reste `BP_BuildNewGame();`.
- ShowLobbyWidgets (ex-711-807, ~97 lignes) et sa branche dans ShowLobby (ShowLobby appelle toujours BP_ShowLobby()).
- Handle{Map,Size,TeamsTogether,Fog,Mode,Option}Chosen (ex-995-1073), HandleTeamChosen, HandleStateChosen, HandleFactionChosen, HandleColorChosen, FindFreeColor.
- Abonnements `if (!bScriptOwnsGestures) ...OnSelectionChanged` de NativeOnInitialized (les 4 boucles par slot et les 6 listes) ; le bloc FindWidget (ex-`if (GetClass() != StaticClass())`) est maintenant inconditionnel.
- Proprietes bScriptOwnsGestures et bScriptOwnsLobbyDisplay (.h) ; leurs 2 lignes `default` et 2 commentaires dans Script\UI\HOLNewGameScreen.as (sinon le script ne compile plus).
- Repli C++ de Open : sans classe script, Open journalise `LogHOLUI: Error: New game screen: the script class UHOLNewGameScreenScript is not loaded...` et rend nullptr (tous les appelants testent ou ignorent le retour : HOLMenuPlayerController.cpp:84, HOLMultiplayerScreen.cpp:542, HOLTitleScreen.cpp:141, HOLUIOpen.cpp:62/125).

## Gardé et pourquoi
- Toute la base appelee par le script : widgets, FindScriptClass, GetNewGame*/AddNewGame*, BP_BuildNewGame, Refresh, SetConfig*, Send*, ShowError, Launch, EnterLobby/ReadLobby/ShowLobby/JoinMatch, ToggleReady/ToggleSpectator, GetColumns, focus.
- Gardes internes encore appeles : ChooseSize (SetConfig, ChooseNewGameSize), IsColorTaken (Refresh), UpdateFactionLists (NativeOnActivated, Refresh), SendSlotAI (SendSlotAIChoice), StartMatch (Launch), HandleWritten, SendSettings (UFUNCTION du script).
- Espace de noms anonyme : aucun helper ne tombe a un seul usage (comptes verifies), donc aucun retire. Aucun LOCTEXT deplace ni supprime hors du code retire : les cles Heading, Map, Fog... restent dans GetNewGameText ; les LOCTEXT du BuildLayout retire (Player, Level, Faction, Colour, Team, You, Mode, Nomad, ...) sont aussi utilises par GetNewGameText/GetNewGameOptions (a verifier au build : voir suspects).
- Le test C++ NewGame.SacredSitesBox (HOLNewGameSitesBoxTests.cpp) : NON porte en script, car il joue EnterLobby avec un FHOLLobby (impossible a construire depuis le script, cf. en-tete de HOLNewGameScreen_Test.as) ; NON supprime ; ADAPTE : il construisait le C++ de la classe de base (NewObject<UHOLNewGameScreen>), qui n'a plus de mise en page ; il prend maintenant `FindScriptClass()` (echoue avec un message si null) et le reste est inchange.
- Lignes de journal « New game: ... » / « Lobby: ... » : intactes (ecrites par le script ou la base, non touchees).
- UCLASS reste concrete (non Abstract) : le test et le script en derivent ; la rendre Abstract est un second temps possible.

## Table symbole -> appelants (grep Source + Script + Tools du worktree)
| Symbole | Appelants restants | Decision |
|---|---|---|
| BuildLayout (C++) | UHOLScreen (Initialize) | corps reduit a BP_BuildNewGame |
| ShowLobbyWidgets | ShowLobby (branche flag, flag toujours vrai en script) | retire |
| Handle*Chosen (11) | abonnements `!bScriptOwnsGestures` seulement (jamais vrais en script) ; aucun autre fichier | retires |
| FindFreeColor | HandleStateChosen seulement | retire |
| bScriptOwnsGestures / bScriptOwnsLobbyDisplay | script (2 `default`) + cpp | retires partout |
| IsColorTaken | Refresh | garde |
| UpdateFactionLists | NativeOnActivated, Refresh | garde |
| SendSlotAI | SendSlotAIChoice (UFUNCTION script) | garde |
| ChooseSize | SetConfig, ChooseNewGameSize | garde |
| StartMatch, HandleWritten, SendSettings | Launch, Send*, script | gardes |
| Open (repli StaticClass) | HOLTitleScreen, HOLMenuPlayerController, HOLMultiplayerScreen, HOLUIOpen ; aucun chemin (spectateur, lobby, test) ne dependait du repli sauf le test SacredSitesBox, qui n'utilisait pas Open | repli retire |
| NewObject<UHOLNewGameScreen> | HOLNewGameSitesBoxTests.cpp seul | passe sur la classe script |

## Suspects de compilation
1. `HOLNewGameScreen.cpp` : includes devenus inutiles (USizeBox, etc.) : sans effet ; verifier qu'aucun helper de l'espace anonyme ne soit devenu inutilise (warning C4505 possible, pas une erreur).
2. `HOLNewGameScreen.cpp` ShowLobby : `bLeader`/`bSpectator` restent utilises dans le LOG (lignes ~499-500) ; si un warning variable inutilisee apparait, c'est ici.
3. Test SacredSitesBox : `NewObject<UHOLNewGameScreen>(GetTransientPackage(), ScriptClass)` avec une classe script dans un test C++ (comme le fait deja HOLScreenTestLibrary::CreateScreenForTest) ; si la classe script est absente dans le contexte du test, le test echoue par TestNotNull. Verifier qu'il passe ET le faire rougir une fois (ex. mettre `GetSiteCount >= 99`).
4. Script\UI\HOLNewGameScreen.as : le script doit compiler sans les deux `default` ; erreur a chercher : « bScriptOwns... not declared ».
5. Mise a jour Placeholders.txt : non touche (aucun provisoire ajoute ni retire).

## Commandes de validation (session principale)
1. `powershell -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait`
2. `Tools\quick_check.ps1 -Parts tests,menu,lobby` (le total ne doit pas baisser ; NewGame.SacredSitesBox vert ; rouge a faire une fois).
3. `Tools\Scenarios\preflight.ps1 ecran_nouvelle_partie` : les 5 captures IDENTIQUES AU PIXEL a celles d'avant.
4. Ouvrir le menu Jouer et un lobby (lobby_match) : lignes « Lobby: ... » inchangees.

## Champs
BUILDS_DEMANDES: 1 (lot C++ de la session principale) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: 518 (cpp 493, h 25) ; LIGNES_MIGREES_PAR_LOT: 0 (code deja migre en NG-3..5) ; TYPES_A_EXPOSER: aucun ; NON VERIFIE : tout (compilation, tests, captures).
