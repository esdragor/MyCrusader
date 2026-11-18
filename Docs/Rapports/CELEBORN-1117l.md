# CELEBORN-1117l : MIG-C-ManagerClient (hero-j8-close, worktree HeroesOfLiberty-Celeborn ; git merge main fait en premier, Reglages non touches)
Commits : cd6ce90 (lot C++ : Net/HOLManagerClientReadLibrary.h/.cpp, UHOLManagerClientReadLibrary, namespace script HOLManagerClientRead) ; 37c1e2e (Script/Tests/HOLManagerClient_Test.as, 12 tests). Pas de build, pas de push, pas de fusion, AUCUN test C++ supprime.

## Classement des 4 tests « Lobby » (HOLMapSizeLobbyTests.cpp) : lot ManagerClient (ForumMessages, ManagerClient, JSON), pas MapSize
| Test C++ | Appelle | Sort |
| MapSize.Lobby.Read | FHOLManagerClientRules::ParseLobby/ParseLobbies + FHOLLobby (Size, GetMapSize, GetNumPlaces, CanToggleSlot, GetMatchMode) | porte : Test_ManagerClientMapSizeLobbyRead |
| MapSize.Lobby.Send | FHOLManagerClientRules::MakeSettingsBody | porte : Test_ManagerClientMapSizeLobbySend |
| MapSize.Lobby.ManagerReference, .ManagerAIReference | FHOLMatchConfig::FromJson + fichiers Manager/HOLManager.Tests/Data/*.json + FHOLMapSizeRules ; AUCUN appel a FHOLManagerClientRules | RESTENT C++ (lecture de fichier + FHOLMatchConfig, autre lot : MatchConfig/JSON) |

## FHOLManagerClientRules et le reseau
FHOLManagerClientRules est PUR (Parse*, Make*Body, ChooseLobbyAction, MakeUrl, MakeMatchAddress, GetMatchOrigin, ReadFailure : texte et nombres, aucun HTTP). Le reseau est dans FHOLManagerLink / HOLManagerClient (Slate), qui restent C++ et ne sont pas touches. FHOLReconnect (etat, Tick sur l'horloge du monde) reste C++ : seules FormatLine et GetSecondsLeft de FHOLReconnectRules sont exposees ; la boucle de pas du test Reconnect reste donc en C++.

## Lot C++ (enveloppes seules, ~290 lignes cpp)
JSON du manager en entree, valeurs PLATES en sortie (texte : "true"/"false", chiffres, nom d'enum ; "" si illisible) car FHOLLobby, FHOLLobbySummary, FHOLManagerError, TOptional ne passent pas au script. Lecture : IsStatus/StatusField, GetNumLobbies/LobbiesField/ChooseLobbyAction, IsLobby/LobbyField(Field, SlotIndex)/LobbySlotField, IsJoin/JoinField, IsWatch/WatchField, IsProtocolError/ErrorField, FailureField (ReadFailure), GetNumServers/ServersField. Ecriture : MakeMeBody, MakeJoinBody, MakeSettingsBody, MakeSlotBody, MakeAISlotBody (TOptional -> couples bHas/valeur). Aides : JsonHasField/JsonField/JsonNumFields (lecteur FJsonObject, comme les tests C++), MakeMatchAddress, GetMatchOrigin, MakeUrl, FormatReconnectLine, GetReconnectSecondsLeft.

## Tests script ajoutes (12, memes vecteurs que le C++)
Test_ManagerClientParse, ...LobbyAction, ...Failure, ...Lobby, ...Write, ...AI, ...Match, ...Origin, ...Spectator, ...ReconnectLine, ...MapSizeLobbyRead, ...MapSizeLobbySend (+ aides MC_*). Ecarts assumes : Write et AI construisaient un FHOLLobby a la main -> ici le JSON du meme salon (passe par ParseLobby, donc plus fort) ; « Anonymous » de Write garde le leader (leader null non affirme) ; « Full » de Spectator ferme 6 slots au lieu des 8 (aucun slot Open reste, meme assertion).

## Reste a faire (session principale)
1. BUILD : `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait` (UHT genere HOLManagerClientReadLibrary ; liaison script HOLManagerClientRead::).
2. Compiler les scripts, lancer quick_check -Parts script (attendu +12 tests : Test_ManagerClient*).
3. ROUGE VU puis vert : fausser une valeur (ex. "Salon de Bob" -> "Salon de Bab" dans Test_ManagerClientParse) une fois, voir rouge, remettre ; PUIS seulement retirer HOLManagerClientTests.cpp (9 tests ; Reconnect garde sa boucle) et FHOLMapSizeLobbyReadTest/SendTest.
## NON VERIFIE (aucune compilation) : litteral FString.Replace en script, `"" + int`, double en parametre UFUNCTION, GetNameStringByValue (EHOLAIDifficulty/EHOLMatchMode/EHOLFogMode sont UENUM : verifie dans les en-tetes), ParseLobby avec "leader": null non teste.
BUILDS_DEMANDES: 1 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 12 (+ 2 de lot MapSize Lobby comptes dedans : 12 fonctions Test_ au total) ; LIGNES_CPP_SUPPRIMEES: 0 ; TYPES_A_EXPOSER: UHOLManagerClientReadLibrary ; TEST_ROUGE: non vu (pas de build) ; PROCESSUS_DETACHES: aucun
