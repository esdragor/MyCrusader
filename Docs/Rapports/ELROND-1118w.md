# ELROND-1118w : écran Nouvelle partie en script, écart « slot 5 stays closed » (2026-11-18)

Impact joueur : aucun visible (le slot 5 restait bien fermé), mais le contrôle `menu fr` du menu rouge et trois lignes de journal des scénarios fog / faction manquaient.

## Ligne attendue par le contrôle
`quick_check.ps1:3602` : `$Sizes | Select-String -SimpleMatch "New game: slot 5 stays closed, past the 4 places of the size"`.
Séquence jouée (`quick_check.ps1:698-701`, menu fr) : `hol.UI.Choose SizeDropdown 1` (solo à 3 joueurs), `hol.UI.Choose SlotState5 1`, frames. Le contrôle ne lit PAS un état de widget : seulement la ligne de journal.

## Écart
- C++ `HOLNewGameScreen.cpp:1259-1266` (HandleStateChosen, solo) : slot >= places => `UE_LOG(LogHOLUI, Display, "New game: slot %d stays closed, past the %d places of the size")` puis Refresh, retour.
- Script avant correctif : `HOLNewGameScreen.as` OnSlotStateChosen déléguait à `HOLNewGameRules::ApplyStateChosen` (qui renvoie false pour slot >= places) puis Refresh : même effet sur la configuration, mais AUCUNE ligne de journal. Même classe d'oubli pour trois autres UE_LOG de gestes : `New game: fog X` (cpp:1035), `New game: mode X` (cpp:1046), `New game: slot N faction X` (cpp:1340, lu par `faction_shots.ps1:53` ; fog par `fog_shots.ps1:48`).
- Refresh (cpp:848-986) n'est pas dupliqué : le script appelle le Refresh C++ de la base. Donc les effets de Refresh sur les lignes (liste d'état désactivée si SlotIndex >= NumPlaces à chaque Refresh, faction/couleur désactivées et cachées si non occupé, niveau d'IA, équipe, nom) sont identiques par construction. Point 5 : aucun autre écart trouvé de ce côté ; seul écart supplémentaire = les 3 lignes de journal ci-dessus, corrigées. Dans le lobby sans client, le C++ logge la ligne faction (else de `bLobby ? GetClient() : nullptr`) ; le script le fait désormais aussi (`bInLobby && HasClient()` sinon log).

## Correction (commit f4a755a sur tiers, 2 fichiers : HOLNewGameScreen.as, HOLNewGameScreen_Test.as)
- `namespace HOLNewGameLog` (StaysClosedLine, FogLine, ModeLine, FactionLine) : les textes exacts du C++.
- Dans la classe : `LastLogLine` + `LogNewGame(Line)` (Log(n"LogHOLUI", ...) et mémorise, un test script ne lit pas le journal).
- OnSlotStateChosen : `if (SlotIndex >= NumPlaces) { LogNewGame(StaysClosedLine); Refresh(); return; }` avant ApplyStateChosen. OnFogChosen / OnModeChosen : log après SetConfigGlobals. OnSlotFactionChosen : log en solo (et lobby sans client).
- Règle pure `HOLNewGameRules.as` non touchée (la faute n'y était pas).

## Tests ajoutés (HOLNewGameScreen_Test.as)
- `Test_NewGameSoloSlotPastThePlacesStaysClosed` : Medium par `SizeDropdown.Choose(2)`, slot 3 actif / slot 5 grisé (IsWidgetEnabled), `SlotState5.Choose(1)`, GetDropdownIndex = 0, `Script.LastLogLine` == la ligne attendue. Rouge avant correctif : LastLogLine vaut "" (le correctif non câblé ne l'écrit pas). Mutation de rouge : fausser « 4 places » en « 5 places » dans le test, ou retirer le `LogNewGame` d'OnSlotStateChosen.
- `Test_NewGameSoloGesturesWriteTheirJournalLine` : fog 1 => "fog Explored", mode 1 => "mode Diplomacy", faction slot 0 => commence par "New game: slot 0 faction ".
- `Test_NewGameJournalLines` : textes purs (dont fog clampé).

## Ce qui n'a pas pu être lancé
- Un `quick_check -Parts tests` a été lancé (1 des 4 essais, 18 s) : il échoue AVANT mes tests, pour une raison étrangère : le binaire du worktree est périmé. `Get-StaleBinaryProblem` : UnrealEditor-HeroesOfLiberty.dll (20:57:23) plus vieux que HOLMatchStatsReadLibrary.h (21:23:57) et UnrealEditor-HeroesOfLibertyUI.dll (20:57:37) plus vieux que HOLNewGameScreen.h (21:35:20). Erreurs de script : « No matching signatures to HasClient() / CanEditLobbyAISlot / GetNewGameFactionName / GetNewGameLobbyLineName / IsLobbyYouLeader... » sur des lignes qui existaient avant mon édition (HOLNewGameScreen.as:418 etc.), le C++ du worktree (140c344) n'est pas compilé dans le binaire. Aucun build possible dans cette tâche.
- Donc rouge non vu, vert non vu, compte non connu. À faire par la session principale : `Tools\build.ps1 -LiveCoding=0 -Wait` dans HeroesOfLiberty-Elrond, puis `quick_check -Parts tests` (attendu : 3 tests de plus), puis `-Parts menu` (le contrôle « slot 5 past the 4 places of Medium stays closed » doit passer).
- Points d'incertitude de compilation script non vérifiés : `Math::Clamp(int,int,int)`, `FString.StartsWith`, `Log(n"LogHOLUI", string)` (niveau Log, pas Display : le fichier de journal le contient aussi).
