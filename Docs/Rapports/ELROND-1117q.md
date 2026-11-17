# ELROND-1117q : MIG2-S3 L4 Cheats groupe A (7 RPC) -- NON COMPILE (C++ et script jamais compiles, aucun test lance)

APPELS_DECLARES: ~18 (arret a 25 vus ; arret dur 19:40)
HEURE_DEBUT/HEURE_FIN: 19:24 / 19:28
PREMIER_BUILD: sans build (consigne)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: aucun (pas d'execution) ; assertit l'effet : non (fonctions pures de refus ; le rouge se fait en remplacant chaque corps par `return 0;`)
COMMITS: 1 (66a9806 sur tiers, 19:27), merge_main_fait: non (tiers deja a jour de main a389ca2 ; DLL non reconstruite)
TESTS: aucun lance ; 4 tests script ajoutes dans Script\Tests\HOLRpcCheatsA_Test.as : Test_RpcCheatSeatRefusal (Resources / FastBuild / Population), Test_RpcCheatTierRefusal, Test_RpcCheatTechnologyRefusal, Test_RpcCheatScaleRefusal (UnitSpeed / GatherSpeed ; l'infini est CALCULE 1e300*1e300) ; TESTS_TAUTOLOGIQUES: 0 declare
ESSAIS: aucun
QUESTIONS: aucune (defauts hol.Rpc.ScriptCheat<Nom> = 0, les 7 cvars)
BUILDS_DEMANDES: 1 (lot C++ : `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\build.ps1 -LiveCoding=0 -Wait` par la session principale, a grouper avec les lots de CELEBORN-1117q et HALDIR-1117q) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 4 ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~0 (liaison + garde de refus) ; TYPES_A_EXPOSER: aucun (FHOLResources passe en 4 ints, FName et double natifs)

FAIT (tout "ecrit, non compile"; commit 66a9806, 4 fichiers de code, +336 lignes, aucune ligne existante reecrite hors 7 corps Request* qui gagnent un bloc de route) :
- Script\Net\HOLRpcPlayer.as (blocs `CHEATS A`) : 7 cvars hol.Rpc.ScriptCheat{Resources,FastBuild,Population,Technology,Tier,UnitSpeed,GatherSpeed} (apres HOLRpcSlots) ; fonctions pures HOLRpc::CheatRefusal(bAllowed, slot) (1 cheats off R3.5, 2 pas de siege), CheatTierRefusal (3 cible hors -2..7), CheatTechnologyRefusal (3 = None), CheatScaleRefusal (3 echelle non finie) ; 7 RPC UFUNCTION(Server) ServerCheat* (insertees avant le bloc T5 de HALDIR-1117p) : garde cvar, slot de l'expediteur (GetSenderSlot, D288), HOLRpcBridge::CheatRefused (journal C++ "Cheat refused: ...") puis le pont.
- Pont (Net\HOLScriptRpcPlayerActor.h/.cpp, UHOLRpcBridgeLibrary, section `CHEATS A` avant le bloc 1117p) : CheatRefused(RpcActor, What) (appelle IsCheatRefused du controleur proprietaire, vrai si pas de controleur), CheatResources(Food, Wood, Stone, Gold) (reconstruit FHOLResources), CheatFastBuild, CheatPopulation, CheatTechnology(FName), CheatTier, CheatUnitSpeed / CheatGatherSpeed (double -> float) : chacun appelle le _Implementation du controleur (une seule source : stock, cvars, tier, plafond, journal).
- HOLPlayerController.cpp : namespace autonome HOLRpcScriptRoute_Elrond1117q::CallCheat (pose avant RequestCheatResources car le helper de 1117n est defini plus bas ; remplit bool/int, double/float, FName), et dans RequestCheatResources / FastBuild / Technology / Tier / Population / UnitSpeed / GatherSpeed : route vers le script si la cvar vaut 1, sinon la RPC C++ comme avant. Aucune RPC C++ supprimee.

RESTE / A FAIRE PAR LA SESSION PRINCIPALE :
- Build groupe puis `quick_check -Parts tests` ; rouge VU : corps de CheatRefusal / CheatTierRefusal / CheatTechnologyRefusal / CheatScaleRefusal -> `return 0;`, puis restaurer (vert).
- Essai : serveur avec cheats actifs + hol.Rpc.ScriptCheat<Nom>=1 sur serveur ET client (ex. `hol.Cheat.Resources` du client) : lire "Cheat: slot N resources, now ..." ; avec cheats inactifs : "Cheat refused: resources from ..." puis "RpcCheat: resources refused (1)".
- Groupe B (Wound, DamageBuilding, SpawnBuilding, Defeat, FallForum, SkipForum, HoldPresence) : CELEBORN-1117q ; le plan en liste 14 au total.

NON VERIFIE : tout (compilation C++ : acces a IsCheatRefused et aux 7 _Implementation par friend, FHOLResources(int,int,int,int) public, FDoubleProperty / FFloatProperty / FNameProperty dans CallCheat, TArrayView<const int32>(&Places, 1) sur un parametre const ; compilation Angelscript : `n"None"` comparaison d'FName, UFUNCTION(Server) avec FName et `float` (double) en parametre, `const float Huge = 1e300`, FName en parametre de fonction de namespace) ; que le parametre `float` d'une RPC script soit bien un FDoubleProperty et non un FFloatProperty (les deux sont geres) ; que la cvar du client soit lue a l'appel (la route est cote client, la garde dans le corps Server lit celle du serveur : les deux doivent valoir 1) ; l'ordre CPF_Parm (Tier: Tier puis Slot) ; un spectateur ne peut plus lancer un cheat par la route script (le C++ le laissait faire pour hol.Cheat.Tier -2) : ecart voulu par la consigne (slot de l'expediteur), a confirmer. Les tests sont des fonctions pures : l'effet de bout en bout (stock recu, cvar posee) n'est pas teste.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: ` M EtapeDefini.md` (ma ligne 8) et `?? Docs/Priorite_Socle_Factions_2026-11-16.md` (deja la au depart) : inchangee hors docs voulus : oui
