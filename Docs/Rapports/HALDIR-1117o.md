# HALDIR-1117o : MIG2-S3 T4 lot L1 Specialisation + Heros  -- NON COMPILE (C++ et script jamais compiles, aucun test lance)

APPELS_DECLARES: ~14 (arret a 25 vus ; arret dur 18:36)
HEURE_DEBUT/HEURE_FIN: 18:22 / 18:26
PREMIER_BUILD: sans build (consigne)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: aucun (pas d'execution) ; assertit l'effet : non (fonctions pures de validation)
COMMITS: 1 (0df654a sur maps, 18:25), merge_main_fait: non (DLL perimee)
TESTS: aucun lance ; 3 tests script ajoutes dans Script\Tests\HOLRpcHeroSpec_Test.as : Test_RpcSeatRefusal, Test_RpcHeroAbilityRefusal, Test_RpcRageRefusal ; TESTS_TAUTOLOGIQUES: 0 declare (fonctions de bornes, pas de constante recopiee ; l'enum est celui du C++)
ESSAIS: aucun
QUESTIONS: aucune (defauts hol.Rpc.ScriptSpecialization / ScriptHero = 0)
BUILDS_DEMANDES: 1 (lot C++ : build.ps1 -LiveCoding=0 -Wait par la session principale) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 3 ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~0 (liaison) ; TYPES_A_EXPOSER: aucun de neuf

FAIT (Specialisation 2 RPC + Heros 5 RPC, tout dans l'etat "ecrit, non compile") :
- Script\Net\HOLRpcPlayer.as : cvars hol.Rpc.ScriptSpecialization et hol.Rpc.ScriptHero (defaut 0 : la RPC script ne fait rien) ; fonctions pures HOLRpc::SeatRefusal(slot), HeroAbilityRefusal(slot, EHOLHeroAbility) (2 = Aura/autre, seules CloseRanks et Banner sont lancees, comme le C++), RageRefusal(slot, n) ; RPC script ServerStartSpecialization(int,int), ServerCancelSpecialization(), ServerReturnHero(), ServerHeroTalent(int), ServerHeroAbility(EHOLHeroAbility, FVector_NetQuantize), ServerCastRage(const TArray<uint32>&), ServerWatchUnit(uint32).
- Pont (Net\HOLScriptRpcPlayerActor.h/.cpp, UHOLRpcBridgeLibrary) : StartSpecialization, CancelSpecialization, ReturnHero, HeroTalent, HeroAbility, CastRage (TArray<int32> : uint32 n'est pas BlueprintCallable, recast en uint32), WatchUnit (int32 recast). CHOIX : chaque statique appelle le _Implementation du controleur proprietaire (corps et journal C++ identiques, une seule source) ; pour cela `friend class UHOLRpcBridgeLibrary;` ajoute dans HOLPlayerController.h (apres GENERATED_BODY). Le pont de specialisation de 1bce302 (UHOLSpecializationSubsystem) est reutilise tel quel via ServerStart/CancelSpecialization_Implementation.
- HOLPlayerController.cpp : Call du namespace HOLRpcScriptRoute_Haldir1117n devient CallN(..., TArrayView<const int32>) (remplit bool / int32 / uint32 dans l'ordre des parametres) + Call une-int en enveloppe (les routes de 1117n inchangees) + declaration anticipee de CallN en tete (le watch est avant la definition). Routes ajoutees (cvar 1, sinon C++ comme avant) : RequestSpecialization, RequestCancelSpecialization, RequestReturnHero, RequestHeroTalent, la surveillance de l'unite (ServerWatchUnit). Aucune RPC C++ supprimee.
- eol_check -Fix lance : a retouche 2 fichiers .as hors de ma tache (Neutrals\HOLCampRules_Test.as, Tests\HOLRpcDuelMatch_Test.as, fins de ligne seulement, aucun diff git).

RESTE / A FAIRE PAR LA SESSION PRINCIPALE :
- `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\build.ps1 -LiveCoding=0 -Wait` puis `quick_check -Parts tests`. Rouge a faire : chaque corps de SeatRefusal / HeroAbilityRefusal / RageRefusal -> `return 0;`, puis restaurer.
- NON ROUTES depuis le C++ : ServerHeroAbility (enum + vecteur) et ServerCastRage (tableau) : CallN ne remplit que des entiers. Le script les expose mais RequestHeroAbility / RequestCastRage appellent encore le C++ seulement. A faire si on veut les essayer avec cvar = 1 (FEnumProperty / FStructProperty / FArrayProperty).
- Pas de fonction pure pour WatchUnit ni pour HeroTalent (le C++ ne valide que le slot / la famille est jugee par les regles au pas) : seulement SeatRefusal.
- Essai 2 clients avec hol.Rpc.ScriptSpecialization=1 / ScriptHero=1 : lire "Specialization: slot N starts tier ..." / "Hero return order: slot N queued" / "Hero talent order ...".

NON VERIFIE : tout le C++ (compilation : `friend` sur la classe UCLASS, acces aux _Implementation privees, UFUNCTION a parametres EHOLHeroAbility et FVector_NetQuantize dans une classe de bibliotheque, include Simulation\HOLHeroAbilityRules.h dans le .h du pont) ; le script (compilation Angelscript des 7 RPC et des 3 fonctions) ; que ProcessEvent d'une RPC script emette bien en reseau depuis un client ; que le type de propriete d'un uint32 en RPC script soit FUInt32Property (sinon le watch recevra 0).
PROCESSUS_DETACHES: aucun lance
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non relance ; modifs voulues : EtapeDefini.md, Auto_Journey_File.md (docs), Docs\Rapports\HALDIR-1117o.md
