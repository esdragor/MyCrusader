# SAURON-1119s : lot C2 de l'etape 5 (commandes console du controleur, familles hero / duel / diplomatie / signal / chat / specialisation)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `envelope`, commit `3f55560` (un seul commit pour les 11 commandes, pas un par famille : budget d'appels). Branche prete, pas de push, merge main non fait. TOUT NON COMPILE.

BUILDS_DEMANDES: 0 (le lot est du script + retrait C++ : la session principale compile avec son build groupe ; le retrait de 164 lignes C++ exige ce build)
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 6 (dans `Script\Player\HOLControllerCommandsC2_Test.as`) : Test_ControllerCommandsC2EachNameOnceAndRouted, Test_ControllerCommandsC2HeroAndDuel, Test_ControllerCommandsC2DiplomacyStance, Test_ControllerCommandsC2SignalAndChat, Test_ControllerCommandsC2Specialization (5 fonctions de test ; 6e = la fonction d'aide C2Args non testee, a ne pas compter : donc 5 tests)
LIGNES_CPP_SUPPRIMEES: 164 (HOLPlayerController.cpp, deletions seules, `git diff --stat`)
LIGNES_MIGREES_PAR_LOT: ~164 C++ -> ~230 script (HOLControllerCommandsC2.as) + ~120 script de test
TYPES_A_EXPOSER: aucun pour les 11 migrees ; pour le reste voir ci-dessous
TEST_ROUGE: NON VU (rien compile). Valeur a fausser : dans `HOLControllerCommandsC2.as`, `StanceOf`, faire renvoyer 2 au lieu de 1 pour "neutral" -> `Test_ControllerCommandsC2DiplomacyStance` echoue sur "neutral is stance 1". Second rouge possible : `ChannelOf`, "allies" -> 0 echoue sur "channel allies is 1".
EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: non lance (aucun processus demarre)

## Commandes migrees (11, memes noms `hol.*`)
hol.Hero.Return, hol.Hero.Cast, hol.Duel.Challenge, hol.Duel.Accept, hol.Duel.Refuse, hol.Diplomacy.Stance, hol.Diplomacy.Vision, hol.Signal.Post, hol.Chat.Say, hol.Spec.Start, hol.Spec.Cancel.
Fichiers : `Script\Player\HOLControllerCommandsC2.as` (namespace `HOLControllerCommandsC2`, reutilise `HOLControllerCommands::ParseInt/ParseReal` et la structure `FHOLCheatPlan` de C1) et `HOLControllerCommandsC2_Test.as`. Chaque commande appelle `AHOLRpcPlayer.Server<X>` de chaque controleur local, comme les `Request<X>` C++ (CallN / CallEx).

## Restent en C++ (liaison manquante)
- hol.Hero.Rage : `Controller->GetOwnSelectedUnits()` -> `HOLControllerRead::OwnSelectedUnitIds(AHOLPlayerController*, TArray<int32>& Out)` (puis `ServerCastRage`).
- hol.Market.Trade, hol.Tribute, hol.Prod.Rally, hol.Prod.Queue (Prod.Cancel : selection du controleur aussi, `RequestCancelProduction` lit `GetOwnSelection`) : `SetSelection(FindOwnBuilding(Type))` + `GetOwnSelection` -> `HOLControllerRead::SelectOwnBuilding(AHOLPlayerController*, FName Type, AHOLBuilding& Out)` et `SelectedBuilding(AHOLPlayerController*, AHOLBuilding& Out)` ; les `ServerMarketTrade / ServerTribute / ServerQueueProduction / ServerCancelProduction / ServerSetRallyPoint` existent deja.
- hol.Signal.Report, hol.Chat.Report : `GetNumReceivedSignals()` / `GetNumReceivedChats()` + slot du `PlayerState` -> `HOLControllerRead::ReceivedCounts(AHOLPlayerController*, int& Signals, int& Chats, int& Slot)`.
- hol.Spec.Report : lecture de `FHOLSpecializationView` de chaque `AHOLPlayerState` -> une fonction de description C++ (ou exposer la structure).
- hol.Diplomacy.Force / ForceVision / Report, hol.Tribute.Notice, hol.Market.Report, hol.Prod.Report : lisent / appellent `AHOLGameState` (`ApplyStance`, `ApplyVision`, `ReportDiplomacy`, `NoteTributeReceived`, `GetMarketBook`) ; fichier `HOLGameState.*` interdit pour moi ; signatures a exposer : `static bool ApplyStance(int From, int To, int Stance)`, `ApplyVision(int From, int To, bool bShare)`, `ReportDiplomacy()`, `NoteTributeReceived(int Giver, int Resource, int Arrives, int Tax)`.
- hol.Spawn.Here : deja migre au lot C1 (hol.Units.SpawnHere), non touche.

## Differences connues (a decider)
1. Niveau de journal : les lignes "Usage" et "unknown" etaient `UE_LOG Warning` : le script appelle `Warning(<texte>)` (niveau conserve, texte identique ; categorie `LogHOLMatch` perdue, le `Warning` script n'a pas d'argument de categorie, a verifier au 1er lancement). La ligne "RpcRoute: X not sent" etait `Error` ; le script n'a que `Log` (Display) dans l'usage vu : elle est dans UNE fonction `LogNotSent` (`HOLControllerCommandsC2.as`) a changer si un appel d'erreur existe. Meme ecart que C1, signale ici pour ne pas passer inapercu.
2. Texte : les usages de hol.Hero.Cast et hol.Duel.Challenge n'ont pas le prefixe "Usage: " (identique au C++, volontaire).
3. Spectateur pur : le C++ `Request<X>` revenait en silence (`IsSpectator()`, non appelable du script). Le script envoie le RPC et le serveur refuse (`HOLRpc::SeatRefusal`, ligne "refused" dans le journal) : une ligne de journal de plus pour un spectateur, aucun effet de jeu. hol.Chat.Say n'avait pas ce garde (identique).
4. Les textes d'aide de la console sont perdus (comme C1).
5. Ordre de coherence a verifier au build : `EHOLHeroAbility::Banner` / `::CloseRanks` (noms deduits de la signature du RPC et du C++, valeurs 1 et 0 non relues dans l'en-tete) et `Warning(FString)` (utilise ailleurs dans `HOLGameScreenScript.as:1168`).

## NON VERIFIE
Compilation script et C++, les 5 tests, le rouge, le comportement en jeu (aucun jeu lance), la valeur numerique de `EHOLHeroAbility`, la compilation de `Cast<AHOLRpcPlayer>(HOLRpcBridge::FindPlayerActor(...))` (copie du patron C1).

## Commande a lancer par la session principale
Le build groupe habituel, puis `quick_check -Parts` des tests script (Script\Player) ; scenarios qui utilisent ces commandes : verifier par grep `hol.Hero.Cast|hol.Duel|hol.Diplomacy.Stance|hol.Signal.Post|hol.Chat.Say|hol.Spec` dans `Tools\Scenarios` qu'aucun ne lit une ligne "Usage" de niveau Display.
