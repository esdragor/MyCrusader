# HALDIR-1119aa : une ligne de journal par pont de planche script

Branche `boards-log` (depuis `integ-16`), worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`. NON COMPILE, NON LANCE.
Commits : a155f61 (C++), puis le commit des outils de test (voir `git log boards-log`).

## Impact joueur
Aucun changement de regle. Si une planche script ne naît pas, le journal du serveur ou du client le dit (Warning) au lieu de retomber sur le C++ en silence.

## Ponts trouves (14, tous `UCLASS(Abstract)`, `UWorldSubsystem`, `BlueprintNativeEvent` rendant "pas de reponse")
Chaque pont est Abstract : SEUL le script en fait naître une instance. `Find(World)` = `World->GetSubsystemArrayCopy<Pont>()` puis premier element (null si aucune instance) ; `Ask*` rend -1 / repli C++ si null. Aucun pont ne cree la sous-classe lui-meme : le moteur la cree via la collection de sous-systemes si la classe script existe.
Donc le pont ne peut pas ecrire "ABSENT" lui-meme (il n'existe pas) : la ligne est ecrite par une fonction commune, appelee depuis un sous-systeme concret toujours present.

| Pont (nom dans la ligne) | Classe C++ | Classe script |
|---|---|---|
| MatchFlow | UHOLMatchFlowSubsystem | UHOLMatchFlowBoardScript |
| Market | UHOLMarketSubsystem | UHOLMarketBoardScript |
| Construction | UHOLConstructionBoard | UHOLConstructionBoardScript |
| PlayerOrders | UHOLPlayerOrdersBoard | UHOLPlayerOrdersBoardScript |
| Production | UHOLProductionBoard | UHOLProductionBoardScript |
| CommandGrid | UHOLCommandGridBoard | UHOLCommandGridBoardScript |
| UnitTier | UHOLUnitTierSubsystem | UHOLUnitTierBoardScript |
| Camp | UHOLCampBoard | UHOLCampBoardScript |
| Garrison | UHOLGarrisonBoard | UHOLGarrisonBoardScript |
| Relay | UHOLRelayBoard | UHOLRelayBoardScript |
| Site | UHOLSiteBoardSubsystem | UHOLSiteBoardScript |
| Chat | UHOLChatSubsystem | UHOLChatBoardScript |
| Signal | UHOLSignalSubsystem | UHOLSignalBoardScript |
| Specialization | UHOLSpecializationSubsystem | UHOLSpecializationBoardScript |

Les 7 de l'enonce + 7 trouvees par Grep (Camp, Garrison, Relay, Site, Chat, Signal, Specialization), meme defaut.

## Code
- `Source\HeroesOfLiberty\Match\HOLBoardLog.h/.cpp` (neufs) : `HOLBoardLog::LogWorldBoards(UWorld&)`, une fonction modele `Report<TPont>` pour les 14.
- `Simulation\HOLSimulationSubsystem.cpp` : appel dans `OnWorldBeginPlay`, AVANT le `return` du client (serveur, client, solo).
- Monde de jeu : `World.IsGameWorld()` ET `World.GetGameInstance() != nullptr` (les mondes de test C++ de `HOLTestWorldLibrary.cpp` sont `EWorldType::Game` mais sans GameInstance et n'appellent pas `Simulation::OnWorldBeginPlay`). Une fois par monde (ensemble de `TWeakObjectPtr<UWorld>`).
- Categorie `LogHOLMatch`.

## Lignes exactes
Display (planche presente) : `LogHOLMatch: Display: Board: <Pont> script <NomUClass>` (NomUClass = `GetName()` de la classe script, probablement sans le prefixe U : ex. `HOLMatchFlowBoardScript`, a verifier au 1er lancement).
Warning (absente) : `LogHOLMatch: Warning: Board: <Pont> ABSENT (C++ fallback)`.
Exemples : `Board: MatchFlow script ...`, `Board: Market script ...`, `Board: Construction script ...`, `Board: PlayerOrders script ...`, `Board: Production script ...`, `Board: CommandGrid script ...`, `Board: UnitTier script ...`, `Board: Camp ...`, `Board: Garrison ...`, `Board: Relay ...`, `Board: Site ...`, `Board: Chat ...`, `Board: Signal ...`, `Board: Specialization ...`.

## Client
Tous ces sous-systemes ont `DoesSupportWorldType` Game/PIE sans condition de mode reseau : la planche script est donc creee aussi sur un client (et en solo) quand la classe script est chargee (le script se charge sur le client ; non vu directement). `UHOLSimulationSubsystem::OnWorldBeginPlay` tourne sur le client (il y retourne apres le test `NM_Client`) : la ligne y est ecrite aussi. A confirmer au premier lancement dans un journal client.

## Controles ajoutes
- `Tools\Tests\lobby_match.ps1` : liste `$BoardBridges` avant `$Failure`, deux `elseif` apres "no server log of the match" dans la partie AI (serveur dedie cuisine du gestionnaire) : une ligne `LogHOLMatch: Display: Board: <pont> script \S+` pour chacun des 14, et aucune `Board: \S+ ABSENT`. Les autres cas du fichier (Dave, Faction, etc.) n'ont pas ete touches.
- `Tools\Tests\quick_check.ps1` (partie net, juste apres `server: no match or network error`, ligne ~6806 ; journal `$NetServerLog` = `quick_net_server.log`) : deux controles `server: every script board is there` et `server: no script board ABSENT`. Note : le controle voisin `ServerErrors` (`LogHOLMatch: (Error|Warning)`) aurait deja fait echouer sur un ABSENT (Warning LogHOLMatch).
- Les deux fichiers analysent sans erreur (Parser PowerShell), BOM et CRLF conserves.

## NON VERIFIE
Compilation (ni build ni quick_check, consigne). Format exact du niveau Display dans `server.log` du gestionnaire (suppose `LogHOLMatch: Display:` comme `LogHOLUI: Display:` vu dans les motifs voisins). `GetSubsystemArrayCopy<T>` en `const UWorld&` (meme appel que les `Find`). `Board->GetClass() != TBridge::StaticClass()` valide pour des classes script (non-natives).

## Commande a lancer (session principale)
`Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree Haldir, puis `Tools\Tests\quick_check.ps1 -Parts lobby,net` (lobby = lobby_match AI). Pour voir l'ABSENT : cuisiner/lancer un serveur avec un dossier script saute ou renommer un `.as` de planche, la ligne `ABSENT` doit apparaitre et le controle rougir (rouge a voir).

BUILDS_DEMANDES: 1 (lot C++ unique) | TESTS_CPP_RETIRES: 0 | TESTS_SCRIPT_AJOUTES: 0 | LIGNES_CPP_SUPPRIMEES: 0 | LIGNES_MIGREES_PAR_LOT: 0 | TYPES_A_EXPOSER: aucun | PROCESSUS_DETACHES: 0 (rien lance) | merge main non fait
