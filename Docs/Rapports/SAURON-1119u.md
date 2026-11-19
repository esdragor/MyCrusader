# SAURON-1119u : liaisons C++ pour finir le lot C2 et le journal par categorie

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `envelope`, commit `8125088` (4 fichiers neufs, aucun fichier existant touche, ni `Script\`, ni `HOLPlayerController.*`). Branche prete, pas de push, pas de merge. TOUT NON COMPILE.

BUILDS_DEMANDES: 1 (lot C++ : la session principale compile avec son build groupe ; raison D839 = liaison a exposer)
TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 (aucun test : rien ne compile, les tests de l'agent de script suivant couvriront)
LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 ; TEST_ROUGE: sans objet (liaisons seules)
TYPES_A_EXPOSER: `UHOLScriptLogLibrary`, `UHOLControllerCommandLibrary` (faits ici) ; PROCESSUS_DETACHES: non lance ; EDITS_REFUSES: 0

## 1. Journal par categorie : `Source\HeroesOfLiberty\Commands\HOLScriptLogLibrary.h/.cpp`
Recherche prealable : aucun `*Library.cpp` expose ne fournit une ecriture generique (les `UE_LOG` des bibliotheques sont des lignes figees : Volcano, Biome, ScreenTest, TestWorld). Donc neuf.
Categories du jeu (`HeroesOfLibertyLog.h`, module HeroesOfLiberty) : LogHOLSimulation, LogHOLPresentation, LogHOLCommands, LogHOLStressTest, LogHOLMatch, LogHOLTest, LogHOLNet. `LogHOLUI` est dans le module UI (non joignable d'ici), les `DEFINE_LOG_CATEGORY_STATIC` (LogHOLSettings, LogHOLVolcano, LogHOLTestWorld, LogHOLScreenTest) sont locales a leur fichier : inaccessibles. Nom exact attendu : `LogHOLMatch`.

Namespace script probable `HOLScriptLog` (la classe `UHOLScriptLogLibrary` perd « U » et « Library ») :
- `HOLScriptLog::LogDisplay(FName Category, FString Line)` -> bool : `UE_LOG(Category, Display, "%s")`. Debloque : les lignes Display (restent comme avant).
- `HOLScriptLog::LogWarning(FName, FString)` -> bool : remplace `Warning(...)` du script ; debloque les lignes « Usage: ... » et « unknown ... » `LogHOLMatch` Warning de C1 et C2 (ecart n°1 de SAURON-1119s).
- `HOLScriptLog::LogError(FName, FString)` -> bool : remplace `LogNotSent` de `HOLControllerCommandsC2.as` (ligne « RpcRoute: X not sent », Error).
- `HOLScriptLog::IsKnownCategory(FName)` -> bool.
Categorie inconnue : une ligne Warning `LogHOLMatch` (« ScriptLog: unknown log category ... ») et retour false. Pas de niveau Fatal. Le fichier/ligne affiches par le moteur sont ceux de la bibliotheque (inoffensif pour les greps sur le texte).
Implementation : `FMsg::Logf(__FILE__, __LINE__, Category.GetCategoryName(), Verbosity, "%s", *Line)` sous `#if !NO_LOGGING` et `IsSuppressed` (meme sortie qu'`UE_LOG`).

## 2. Acces controleur : `Source\HeroesOfLiberty\Player\HOLControllerCommandLibrary.h/.cpp`
Classe `UHOLControllerCommandLibrary`, namespace script probable `HOLControllerCommand` (convention de `UHOLControllerReadLibrary` -> `HOLControllerRead`). Attention : les noms `HOLControllerRead::OwnSelectedUnitIds / SelectOwnBuilding / SelectedBuilding / ReceivedCounts` cites par SAURON-1119s sont ici sous `HOLControllerCommand::` (bibliotheque neuve demandee). Boucle sur les controleurs locaux : reste au script (`GetLocalControllers` est un helper local au fichier du controleur). Toutes les `Request<X>` sont publiques et appelees telles quelles (garde spectateur, lecture de selection : identiques au C++).

| Fonction script | Debloque | Equivalent C++ |
|---|---|---|
| `HOLControllerCommand::OwnSelectedUnitIds(Controller, TArray<int>& Out)` | hol.Hero.Rage (si le script veut les ids) | `GetOwnSelectedUnits()` |
| `RequestCastRageOfSelection(Controller)` | hol.Hero.Rage (preferee) | `RequestCastRage(GetOwnSelectedUnits())` |
| `HasOwnSelection(Controller)` | Prod.* | `GetOwnSelection() != nullptr` |
| `SelectOwnBuilding(Controller, FName Type)` -> bool trouve | hol.Market.Trade et hol.Tribute (`"Market"`), hol.Prod.Queue (2e argument) | `SetSelection(FindOwnBuilding(Type))` (null efface, comme le C++) |
| `EnsureOwnSelection(Controller, FName Fallback)` -> bool | hol.Prod.Rally, hol.Prod.Queue (`MainCentre`) | `if(!GetOwnSelection()) SetSelection(FindOwnBuilding(Fallback))` |
| `RequestRallyPointAt(Controller, double X, double Y)` | hol.Prod.Rally | `RequestRallyPoint(FVector(X,Y,0))` |
| `RequestProduction(Controller, FName Type)` | hol.Prod.Queue (defaut `Peasant`) | `RequestProduction` |
| `RequestCancelProduction(Controller, int Index)` | hol.Prod.Cancel (selection du controleur lue en C++) | `RequestCancelProduction` |
| `RequestMarketTrade(Controller, int Resource, int Lots, bool bBuy)` | hol.Market.Trade (food 0, wood 1, stone 2) | `RequestMarketTrade` |
| `RequestTribute(Controller, int Receiver, int Resource, int Amount)` | hol.Tribute (gold = 3) | `RequestTribute` |
| `ReceivedCounts(Controller, int& Signals, int& Chats, int& Slot)` | hol.Signal.Report, hol.Chat.Report (Slot = -1 sans player state) | `GetNumReceived*`, `GetHOLPlayerState` |
| `DescribeSpecializationStates(Controller)` -> TArray<FString> | hol.Spec.Report (une ligne par player state, texte exact) | boucle `GetSpecialization()` |
| `DescribeProductionReport(Controller)` -> FString | hol.Prod.Report (ligne complete « Production report: ... ») | corps de la commande copie |
| `DescribeMarketReport(WorldContext)` -> TArray<FString> | hol.Market.Report (lignes dans l'ordre ; « no game state » seule sinon) | corps copie |
| `ForceStance(WorldContext, From, To, Stance)` -> bool GameState present | hol.Diplomacy.Force (0 enemy, 1 neutral, 2 ally ; defaut du C++ = enemy) | `ApplyStance` |
| `ForceVision(WorldContext, From, To, bool bShare)` -> bool | hol.Diplomacy.ForceVision | `ApplyVision` |
| `ReportDiplomacy(WorldContext)` -> bool | hol.Diplomacy.Report | `ReportDiplomacy()` |
| `NoteTributeNotice(WorldContext, Giver, Resource, Arrives, Tax)` -> bool | hol.Tribute.Notice (ressource inconnue = 3 en C++, a faire cote script) | `NoteTributeReceived` (receveur = 1er controleur local) |

Les fonctions a `WorldContext` s'appellent du script sans passer le monde si le plugin gere `meta=WorldContext` (sinon passer un acteur). Quand une fonction renvoie false pour « pas de GameState / joueur », le script ecrit la ligne « Usage: ... » avec `HOLScriptLog::LogWarning(n"LogHOLMatch", ...)`.

## Reste (non fait)
- Aucun specificateur ajoute dans `HOLPlayerController.h` ; toutes les methodes utilisees sont publiques (verifie : lignes 214, 245, 247, 386-395, 448, 466, 529, 560-565). `IsSpectator()` (ligne 605) est public : non expose ici, un `HasSpectatorSeat` pourra suivre si l'ecart n°3 de SAURON-1119s doit etre ferme (garde `Request<X>` deja en C++ pour celles que j'enveloppe).
- `FindOwnBuilding` de HOLPlayerController.cpp est local au fichier : sa boucle est dupliquee (6 lignes) dans le `.cpp` neuf ; a supprimer quand le C++ des commandes sera retire (la regle reste unique cote script + cette bibliotheque).
- Les commandes C++ ne sont PAS retirees ici (HOLPlayerController.cpp interdit) : retrait par l'agent de script suivant, apres les tests.
- Ecart n°5 de 1119s (`EHOLHeroAbility::Banner/CloseRanks`, valeurs) non regarde.

## NON VERIFIE
Compilation C++ (4 fichiers) et UHT. Points a surveiller au build : `FMsg::Logf` / `FLogCategoryBase::IsSuppressed` ; `HOLProduction::GetProgress` (inclus via `Buildings/HOLProductionRules.h`, copie du code du controleur) ; `FHOLProductionItem`, `HOLMarket::FBook`, `FHOLSpecializationView` (inclus par `HOLGameState.h` / `HOLPlayerState.h` : si absent, ajouter `Match/HOLMarketRules.h`, `Match/HOLSpecializationView.h`) ; `Controller->GetOwnSelectedUnits()` renvoie `TArray<uint32>` (cast int32). Nom script reel des classes et resolution de `WorldContext` par le plugin Angelscript.

## Commande a lancer par la session principale
`Tools\build.ps1 -LiveCoding=0 -Wait` dans `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` (lot C++ groupe avec les autres), puis verifier au demarrage que `HOLScriptLog::LogWarning` et `HOLControllerCommand::RequestProduction` existent cote script.

## Anomalies
- `EtapeDefini.md` / `Auto_Journey_File.md` non touches (consigne) ; `IN_USE.txt` non modifie (budget d'appels).
- Un message systeme a signale que `HOLControllerCommandLibrary.cpp` avait change sur disque apres ma lecture : c'est mon `sed` (include `HOLProductionRules.h` ajoute, `const_cast` retire), contenu verifie, committe.
- Avertissements LF -> CRLF de git sur les 4 fichiers : normaux.
