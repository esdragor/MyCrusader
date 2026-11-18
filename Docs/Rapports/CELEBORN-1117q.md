# CELEBORN-1117q : MIG2-S3 T9 lot L4 Cheats, groupe B  -- NON COMPILE (C++ et script jamais compiles, aucun test lance)

APPELS_DECLARES: ~24 (arret a 25 vus ; arret dur 19:40)
HEURE_DEBUT/HEURE_FIN: 19:24 / 19:27 (Get-Date)
PREMIER_BUILD: sans build (consigne)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: aucun (pas d'execution) ; assertit l'effet : non (fonctions pures de validation) ; rouge a faire par la session principale : corps de chaque HOLRpc::Cheat*Refusal -> `return 0;`, quick_check -Parts tests, restaurer
COMMITS: 1 (7abc399 sur hero-j8-close, 19:27), merge_main_fait: non (consigne : ne fusionne rien ; branche deja sur main c43cd17)
TESTS: aucun lance ; 7 tests script ajoutes dans Script\Tests\HOLRpcCheatsB_Test.as : Test_RpcCheatWoundRefusal, Test_RpcCheatDamageBuildingRefusal, Test_RpcCheatSpawnBuildingRefusal, Test_RpcCheatDefeatRefusal, Test_RpcCheatFallForumRefusal, Test_RpcCheatSkipForumRefusal, Test_RpcCheatHoldPresenceRefusal ; TESTS_TAUTOLOGIQUES: 0 declare (valeurs calculees par les fonctions testees)
ESSAIS: aucun
QUESTIONS: aucune (defauts hol.Rpc.ScriptCheat<Nom> = 0 pour les 7)
BUILDS_DEMANDES: 1 (lot C++ : `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait` par la session principale, a fusionner avec les lots ELROND/HALDIR 1117q) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 7 ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~0 (liaison) ; TYPES_A_EXPOSER: aucun de neuf

FAIT (7 RPC, etat "ecrit, non compile", +310 lignes, 4 fichiers + 1 test) :
- Script\Net\HOLRpcPlayer.as : 7 cvars hol.Rpc.ScriptCheatWound / DamageBuilding / SpawnBuilding / Defeat / FallForum / SkipForum / HoldPresence (defaut 0) ; fonctions pures dans le namespace HOLRpc : CheatWoundRefusal(bCheatsAllowed, senderSlot), CheatDamageBuildingRefusal(bCheatsAllowed, amount), CheatSpawnBuildingRefusal(bCheatsAllowed, typeName, slot), CheatSlotRefusal(bCheatsAllowed, slot) (Defeat, FallForum, SkipForum), CheatHoldPresenceRefusal(...) ; RPC script ServerCheatWound(int), ServerCheatDamageBuilding(int Amount, int AttackType, FVector), ServerCheatSpawnBuilding(const FString& TypeName, int Slot, FVector), ServerCheatDefeat / FallForum / SkipForum(int), ServerCheatHoldPresence(int, bool, bool, bool, int). Chaque bloc porte le commentaire `CHEATS B`, insere a des endroits distincts (apres la cvar Chat, avant « The interface's call », avant le bloc T5 Signals).
- Refus : cheats inactifs (R3.5) = HOLRpcBridge::CheatsAllowed(this, "<quoi>") qui appelle l'IsCheatRefused du C++ (journal « Cheat refused: ... » du C++) ; puis bornes (slot de l'expediteur pour Wound, slot 0..255 pour SpawnBuilding, slot 0..7 pour Defeat/FallForum/SkipForum/HoldPresence, quantite negative pour DamageBuilding, nom de type vide, slot d'attaquant hors bornes). Le corps reste le C++ (_Implementation appele par le pont).
- Pont (Net\HOLScriptRpcPlayerActor.h/.cpp, UHOLRpcBridgeLibrary, insere entre les blocs 1117o et 1117p) : CheatsAllowed, CheatWound, CheatDamageBuilding(X,Y,Z,Amount,AttackType int), CheatSpawnBuilding(FString type -> FName), CheatDefeat, CheatFallForum, CheatSkipForum, CheatHoldPresence.
- HOLPlayerController.cpp : 7 routes (cvar 1, sinon C++ comme avant) dans les Request* via HOLRpcScriptRoute_Haldir1117n::CallEx (declaration anticipee ajoutee juste avant RequestCheatWound, car CallEx n'est defini qu'apres ces fonctions) ; SpawnBuilding passe le nom du type comme FString (Text), DamageBuilding/SpawnBuilding le point comme FVector.
- Constat : le C++ ServerCheatHoldPresence_Implementation n'a AUCUN IsCheatRefused (les 6 autres l'ont) ; le chemin script le refuse hors cheat actif (plus strict, a trancher si le C++ doit suivre : Q non ouverte, pas de design tranche, defaut = refuser).

RESTE / A FAIRE PAR LA SESSION PRINCIPALE :
- Build C++ (lot unique) puis `quick_check -Parts tests` ; rouge a faire comme indique a TEST_ROUGE.
- Essai serveur + client, cheats actifs, hol.Rpc.ScriptCheatWound=1 etc. sur client ET serveur (le serveur lit sa propre cvar, comme les autres lots) : lire « RpcCheat: ... refused » hors cheat et « Cheat: slot N wound ... » avec.
- IN_USE.txt : l'entree OCCUPE n'a pas ete ecrite (j'ai ecrit libre a la sortie seulement).

NON VERIFIE : tout (compilation C++ : acces par friend a IsCheatRefused (const, privee) et aux 7 _Implementation, FVector_NetQuantize(FVector), CallEx redeclare dans le namespace avant sa definition ; compilation script : `const FString& TypeName` en UFUNCTION(Server), 5 parametres dont 3 bool, bool en argument de la fonction pure, appel `HOLRpcBridge::CheatsAllowed(this, "wound")` avec litteral converti en FString) ; que le FName du type passe bien en FString (FStrProperty) via CallEx ; que l'ordre des ints de CallEx suive bien les parametres int/bool de l'UFunction script (HoldPresence : Slot, bOff, bAttackers, bDefenders, AttackerSlot) ; que le ProcessEvent d'une RPC Server depuis le client parte au serveur (prouve seulement pour les lots 1117n/o).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune ; incident de script : variable automatique `$args` de PowerShell ecrasee dans mon helper (tableaux vides `{  }`), detecte et corrige avant commit (diff relu) ; backtick-n litteral corrige aussi.
GIT_STATUS_PRINCIPALE:  M EtapeDefini.md (ma ligne 699, voulue) / ?? Docs/Priorite_Socle_Factions_2026-11-16.md (deja la au depart) : inchangee hors docs voulus : oui
