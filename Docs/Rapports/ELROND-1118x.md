# ELROND-1118x : salon en script, journal identique au C++ (NON COMPILÉ)

NON COMPILÉ, NON RÉJOUÉ (consigne : aucun build). Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche tiers.
Impact joueur : aucun changement visible ; seul le texte de journal d'un geste du salon (couleur, équipe, faction) change pour redevenir celui du C++.

## 1. Liste de contrôle : lignes attendues par `Tools\Tests\lobby_match.ps1`

Séquences (waitlog) :
- Alice / Bob, partie normale (`$Pick`, ligne 142) : `waitlog : 2 players (`, `Lobby: colour <N> accepted`, `Lobby: ready on accepted`.
- `$ToSmall` (146) / `$ToMedium` (149-150) : `map Plain Large teams together 1:`, `settings Plain size Small|Medium teams together 1 nomad 0 regicide 0 catastrophe 0 sacred sites 0 accepted`.
- `-Team` (151-154) : `Lobby: team 1 accepted`, `Lobby: slot 2 ai accepted`, `Lobby: ai 2 team 1 accepted`, `Lobby: slot 3 ai accepted`, `Lobby: ai 3 team 2 accepted` ; Bob `Lobby: team 2 accepted`.
- `-AI` (206-212) : `Lobby: colour N accepted`, `slot 1 ai accepted`, `Lobby: ready on accepted`, `1/1 ready`, `Lobby: start sent`, `Lobby: joining match`.
- `-Faction` (275-294) : `Lobby: faction BlackLegions accepted`, `faction Templars accepted`, `slot 2 ai accepted`, `ai 2 faction BlackLegions accepted`, `Lobby: colour 3 accepted`, `Lobby: ready on accepted`, `2/2 ready`, `Lobby: start sent`; Bob : `Lobby: factions Templars,DawnOrder,BlackLegions`.
Vérifications finales (Select-String) : `Lobby: colour N accepted`, `Lobby: ready on accepted`, `Lobby: joining match ... as <Who>`, `Lobby: start accepted (HTTP 202)`, `slot 1 ai accepted`, ligne de salon `N players (...), you X, leader Y, k/n ready, launch shown|hidden, slots c<N>r a<n> x ...` (base, `HOLNewGameScreen.cpp:678`), `Lobby: faction BlackLegions|Templars accepted`, `ai 2 faction BlackLegions accepted`, `Lobby: factions ...`, et pour Bob (faction) AUCUNE ligne `Lobby: faction ` (ligne 322).

## 2. Table des gestes (cpp `HOLNewGameScreen.cpp` -> script `HOLNewGameScreen.as`)

| Geste | Journal C++ (What) | Script aujourd'hui | Écart |
|---|---|---|---|
| Carte / taille / équipes ensemble / brouillard / mode / options | `settings <carte> size <clé> teams together %d nomad.. sacred sites` (SendSettings, cpp:1074) | `SendSettings()` de la base (as:317, 339, 356, 374, 388, 401) | aucun (même fonction) |
| Brouillard / mode | `New game: fog X` / `New game: mode X` (cpp:1035, 1046) | `LogNewGame(FogLine/ModeLine)` as:372, 386 | aucun (déjà fait en 1118v) |
| Place ouvrir / fermer | `slot N open|closed` (cpp:1250) | `SendSlotOpen` (cpp:1876) : même texte | aucun |
| Place IA | `slot N ai` (SendSlotAI, cpp:1099) | `SendSlotAIChoice(-1,-1,"")` -> SendSlotAI | aucun |
| IA couleur / équipe / faction | `ai N colour X` / `ai N team X` / `ai N faction X` | `SendSlotAIChoice` -> SendSlotAI | aucun |
| Ma couleur | `colour N` (cpp:1356) | `SendMyChoice(N,-1,-1,-1,"")` : `me colour N team -1 ready -1 spectator -1 faction ` | ÉCART (corrigé) |
| Mon équipe | `team N` (cpp:1124) | `SendMyChoice(-1,N,...)` : `me colour -1 team N ...` | ÉCART (corrigé) |
| Ma faction | `faction X` (cpp:1331) | `SendMyChoice(...,X)` : `me colour -1 team -1 ready -1 spectator -1 faction X` | ÉCART (corrigé) |
| Prêt | `ready on|off` (ToggleReady, cpp:1142) | câblé par la base (cpp:474), non rebranché par le script | aucun |
| Spectateur | `spectator on|off` (ToggleSpectator, cpp:1158) | câblé par la base (cpp:478) | aucun |
| Lancement | `start sent` / `start accepted` / `joining match` | `Launch` de la base (cpp:470) | aucun |
| Réponse du gestionnaire | `Lobby: <What> accepted` + `ShowLobby` (HandleWritten, cpp:1210) | même HandleWritten, `BP_ShowLobby` du script ; lignes `Lobby: factions` / `Lobby: L1 ...` écrites par la base après (cpp:678-700) | aucun |

## 3. Cause et correction

Cause unique, explique (a) ET (b) : `SendMyChoice` (base) composait toujours `me colour %d team %d ready %d spectator %d faction %s`, donc `Lobby: me colour 2 team -1 ... accepted` au lieu de `Lobby: colour 2 accepted` : `$Pick` (`waitlog Lobby: colour N accepted`) expirait en partie normale (FAIL 404 : pas d'avancée, Alice puis Bob bloqués), et `Lobby: faction X accepted` en -Faction.

Correction (C++ de liaison, D839) : `Source\HeroesOfLibertyUI\Screens\HOLNewGameScreen.cpp`, `SendMyChoice` (vers 1859-1885) : si UN SEUL champ est donné (colour >= 0, team >= 0, ready >= 0, spectator >= 0 ou faction non vide), `What` est le texte du geste C++ (`colour N`, `team N`, `ready on|off`, `spectator on|off`, `faction X`) ; plusieurs champs : l'ancienne ligne `me ...`. Signature et appels du script inchangés (`HOLNewGameScreen.as:473, 492, 508`), donc aucune modification script. Pas de requête ni de ligne en double : `UpdateMe` reçoit les mêmes arguments qu'avant (-1 = non modifié, comme `TOptional` vide du cpp).

## 4. Comportements relus (par lecture, aucun écart)
- Ordre : couleur/faction = `SetConfigSlot` + `Refresh` puis envoi (cpp:1347-1357, as:485-492) ; équipe = écriture sans Refresh puis envoi (cpp:1116, as:502-508) ; carte = Refresh puis SendSettings (as:313-317) ; taille lobby = `Find(Value)` côté C++ vs `Index >= 1 && Index < Places.Num()` côté script (équivalents : None = 0 refusé).
- Valeurs non modifiées : -1 -> `TOptional` vide (cpp:1868-1873), comme les `{}` du geste C++.
- Place IA testée avant le client (`LobbyStateAction`, rules.as:206) comme cpp:1242 ; faction : garde `LobbyLineEditable` = `SlotIndex == FindYourSlot || CanEditAISlot` (cpp:1316, as:461) ; faction hors liste refusée avec Refresh.
- Rafraîchissement après réponse : HandleWritten -> ShowLobby (base) -> `BP_ShowLobby` du script ; grisés : dans `ShowLobbyLine` / `ShowLobby` du script (non retouchés ; test de salon à rejouer pour confirmer).

## 5. Suspects de compilation
- Opérateur ternaire enchaîné sur `FString` (`FString::Printf(...) : ... : ...`) : tous les bras sont des `FString` (Printf renvoie FString), pas de `TEXT` nu comme bras ; devrait compiler. Seul suspect.

## 6. Non établi
- Rejeu non fait : que le test de salon passe en partie normale, `-Team`, `-AI`, `-Faction` est à vérifier par la session principale (le journal d'Alice lu date de 21:24, ancien).
- Grisés / `Lobby: factions` côté Bob : relus seulement via la base ; non confrontés au script `ShowLobbyLine`.
