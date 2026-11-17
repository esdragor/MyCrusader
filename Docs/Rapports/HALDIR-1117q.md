# HALDIR-1117q : MIG2-S3 lot L3 Economie (4 RPC)  -- NON COMPILE (C++ et script jamais compiles, aucun test lance)

APPELS_DECLARES: ~24 vus (arret a 25 ; arret dur 19:40 ; dont 1 appel perdu : un `python3` lance par erreur, reste en tache de fond bewl7q583, a arreter)
HEURE_DEBUT/HEURE_FIN: 19:24 / 19:29 (Get-Date)
PREMIER_BUILD: sans build (consigne)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: aucun (pas d'execution) ; assertit l'effet : non (fonctions pures de refus, comme 1117p)
COMMITS: 1 (5cda2e5 sur maps, 19:29), merge_main_fait: non (DLL perimee ; maps etait a jour de main a389ca2 a l'entree)
TESTS: aucun lance ; 4 tests script ajoutes dans Script\Tests\HOLRpcEconomy_Test.as : Test_RpcQueueProductionRefusal, Test_RpcCancelProductionRefusal, Test_RpcTributeRefusal, Test_RpcMarketTradeRefusal ; TESTS_TAUTOLOGIQUES: 0 declare (valeurs calculees par les fonctions testees)
ESSAIS: aucun
QUESTIONS: aucune (defaut hol.Rpc.ScriptEconomy = 0)
BUILDS_DEMANDES: 1 (lot C++ : `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\build.ps1 -LiveCoding=0 -Wait` par la session principale, puis `quick_check -Parts tests`) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 4 ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~0 (liaison) ; TYPES_A_EXPOSER: aucun (AHOLBuilding* et FName passes tels quels, voir NON VERIFIE)

FAIT (4 RPC, etat "ecrit, non compile") :
- Script\Net\HOLRpcPlayer.as : cvar hol.Rpc.ScriptEconomy (defaut 0, une seule pour les 4) ; fonctions pures dans HOLRpc:: EconomySeatRefusal(slot, bHasBuilding) (1 pas de siege, 2 pas de batiment), QueueProductionRefusal, CancelProductionRefusal(+ 3 index < 0), TributeRefusal(+ 3 receveur hors 0..7, 4 ressource < 0, 5 montant <= 0), MarketTradeRefusal(+ 3 ressource < 0, 4 lots <= 0) ; RPC script ServerQueueProduction(AHOLBuilding, FName), ServerCancelProduction(AHOLBuilding, int), ServerTribute(AHOLBuilding, int x3), ServerMarketTrade(AHOLBuilding, int, int, bool). Journal "RpcEconomy: ...". Le script ne reproduit PAS les regles de production : IssueProductionOrder (via l'_Implementation C++) decide proprietaire, type, file, stock, Marche, lots.
- Pont (Net\HOLScriptRpcPlayerActor.h/.cpp, UHOLRpcBridgeLibrary) : QueueProduction, CancelProduction, Tribute, MarketTrade (AActor* RpcActor, AHOLBuilding* Building, FName / int32 / bool) -> _Implementation du controleur proprietaire (friend deja present). Declaration `class AHOLBuilding;` ajoutee. Les types ont ete passes tels quels (pas d'index) : un UObject et un FName sont passables en script/UFUNCTION.
- HOLPlayerController.cpp : CallEx devient CallFull (+ UObject* Object, const FName* Name ; remplit FObjectPropertyBase et FNameProperty), CallEx reste l'enrobage (routes 1117n/o/p inchangees). Nouvelles routes RouteQueueProduction / RouteCancelProduction / RouteTribute / RouteMarketTrade (declarees en tete de fichier) ; les 7 sites d'appel (3 Queue, 1 Cancel, 2 Tribute, 1 Market) font `if (!Route...(this, ...)) { ServerX(...); }` : cvar 0 ou pas d'acteur script = RPC C++ comme avant. Aucune RPC C++ supprimee.
- Marche : pas de UHOLMarketSubsystem dans ce worktree (non cherche par grep dedie) ; les 4 RPC passent quand meme car le corps C++ appelle IssueProductionOrder (Trade) ; la regle HOLMarket::Apply de GALADRIEL-1117f n'est pas touchee.
- IN_USE.txt : seulement "libre" a la sortie (l'entree OCCUPE n'a pas ete ecrite : oubli).

RESTE / A FAIRE PAR LA SESSION PRINCIPALE :
- Build + `quick_check -Parts tests`. Rouge a faire : corps de QueueProductionRefusal / CancelProductionRefusal / TributeRefusal / MarketTradeRefusal -> `return 0;` (et EconomySeatRefusal), puis restaurer.
- Essai 2 clients, hol.Rpc.ScriptEconomy=1 : lire "RpcEconomy: queue received by the server: slot N type X" puis la ligne C++ de production ; verifier que le batiment arrive (net id) et que l'ordre est empile.
- EtapeDefini.md : non touche (aucune ligne ecrite par manque d'appels) ; a ajouter par la session principale.

NON VERIFIE : tout le C++ (compilation : FObjectPropertyBase::SetObjectPropertyValue(ContainerPtrToValuePtr<void>), FNameProperty, `const FName TypeName` dont on prend `&`, ordre CPF_Parm) ; le script (Angelscript : parametre `AHOLBuilding Building` d'un UFUNCTION(Server), `Building != nullptr`, `FName` dans f"{TypeName}", bool dans f-string) ; qu'un AHOLBuilding soit replique par net id dans une RPC d'acteur script (hypothese plan section 6) ; les bornes ajoutees (receveur 0..7, montant > 0, lots > 0, index >= 0) : elles refusent plus tot que le C++ mais la production refuserait de toute facon (a confirmer si ApplyTribute accepte un montant <= 0 pour un cas voulu) ; un `python3` lance par erreur (tache de fond bewl7q583) a arreter.
PROCESSUS_DETACHES: non lance (stray_processes.ps1 non appele faute d'appels) ; voir le python3 ci-dessus (hors worktree)
ANOMALIES_PROCESSUS: le python3 interactif lance par erreur (REPL bloque, timeout 120 s, passe en fond)
GIT_STATUS_PRINCIPALE: ` M EtapeDefini.md`, `?? Docs/Priorite_Socle_Factions_2026-11-16.md`, `?? Docs/Rapports/CELEBORN-1117q.md`, `?? Docs/Rapports/ELROND-1117q.md` (EtapeDefini.md modifie par d'autres sessions, pas par moi) : aucun code touche dans la copie principale : oui
