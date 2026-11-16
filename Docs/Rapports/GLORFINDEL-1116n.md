# GLORFINDEL-1116n : migration, pilote 1 (le chat), reprise de 1116m, 2026-11-16 soir

APPELS_DECLARES: 46 (arrêt dur à 45 vus ; ce dernier appel groupe clôture, journal, commit, rapport)
HEURE_DEBUT/HEURE_FIN: 21:08 / 21:31
PREMIER_BUILD: aucun (C++ déjà compilé par 1116m)   BUILD: PASS (20:36, 1116m ; aucun build demandé)   LAUNCHCHECK_DEBUT: 21:08 vide (DLL à jour)
TEST_ROUGE: vu à 21:11 : quick_check -Parts tests 1354/1357, KO HeroesOfLiberty.Chat.ServerBoard (bouchon HOLChatBoardScript.as : tout passe, personne ne lit ; 4 assertions KO : Allies de Bob vers Alice, taux gardé, 201 car., 3e dans la fenêtre) ; vert dans 0a3f09a (le rouge seul n'est pas commité). Assertit l'effet : oui (réponses et destinataires calculés par la vraie chaîne C++ -> script).
COMMITS: 4 (0a3f09a P1b, 7f98164 docs, 715158a sonde sur try-as-rpc seulement, 710decf docs), dernier à 21:31 ; merge_main_fait: oui (Merge made by the 'ort' strategy.
 Auto_Journey_File.md    | 2 +-
 Auto_Journey_Jour.md    | 6 ++++++
 Auto_Journey_Retours.md | 2 +-
 EtapeDefini.md          | 1 +
 4 files changed, 9 insertions(+), 2 deletions(-))
TESTS: 21:18 quick_check -Parts tests,net : tests 1355/1357 (seuls Match.StartingHero.NomadFirstForum et .NomadReplicated, D831 connus), toutes les parties net PASS, netsig3 PASS 53 s. P1c : Chat.ServerBoard seul, MaxTextLength = 1 -> Fail 21:19 sans build ; remis 200 -> Success 21:19:49. TESTS_TAUTOLOGIQUES: 0.
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
- FAIT MIG-P1b 0a3f09a : Script\Chat\HOLChatBoardScript.as (sous-classe du pont UHOLChatSubsystem) ; le WIP 645fc25 devient un lot vert ; Docs\Patches\HOLChatBoardScript.as.txt supprimé (le patch D806 l'était déjà).
- FAIT MIG-P1c : une règle de la garde changée dans le .as change le comportement sans build ; attentes de netsig3 inchangées (elles couvrent déjà le script : answer 2 et 3 exactes, « sent to 2 seats » impossible sans board) ; quick_check.ps1 non touché.
- NON CONCLU MIG-P1d : sonde 715158a sur try-as-rpc (jamais fusionnée). Réponse OUI/NON : AUCUNE pour l'instant.
- **net-playtest PRÊTE pour la fusion** (710decf ; tests,net verts hors les 2 rouges D831 venus de main).
- RESTE P1d : voir les lignes ci-dessous ; si le serveur ne dit pas « the server spawned », le sous-système script ne tique pas sur le serveur dédié : créer la sonde autrement (acteur script placé par le GameMode, ou BeginPlay du sous-système).
NON VÉRIFIÉ: la RPC Server/Client en script avec Iris (P1d) ; le chat joué à la main (seulement netsig3) ; le merge main final n'a pas été retesté s'il a apporté du code (sortie ci-dessus).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: (1) -ExecCmds=hol.Dev.RpcProbe (commande console script) jamais exécuté sur UnrealEditor -server (aucune trace dans le journal). (2) Première sonde arrêtée de force (Stop-Process de mes 3 éditeurs RpcSrv/RpcCli) pour relancer. (3) Budget de 45 appels atteint pendant P1d. EDITS_REFUSES: 0.
GIT_STATUS_PRINCIPALE:
 M Auto_Journey.md
 M Auto_Journey_File.md
 M Auto_Journey_Jour.md
 M EtapeDefini.md
 M GDD.md
 M Tools/AutoJourney/brief_commun.md
?? Docs/Priorite_Socle_Factions_2026-11-16.md
?? Docs/Rapports/PLAN-1116-E3.md
?? Docs/Rapports/SAURON-1116n.md
?? Retours/Q-migration-r9.md

(inchangée hors docs voulus : oui, Auto_Journey_File.md et EtapeDefini.md = mes lignes)

## Champs de migration
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 2 (HeroesOfLiberty.Chat.Route, HeroesOfLiberty.Chat.Gate ; commités dans 645fc25, validés par 0a3f09a)
TESTS_SCRIPT_AJOUTES: 0 ce soir par moi (4 par 1116m dans a9e8811) ; 1 test C++ du pont rendu vert (Chat.ServerBoard)
LIGNES_CPP_SUPPRIMEES: 293 (HOLChatRules.cpp 133 + .h 69, HOLPlayerController.cpp 16, HOLChatTransportTests.cpp 75) contre 259 ajoutées : -34 nettes
LIGNES_MIGREES_PAR_LOT: P1b +26 lignes de script (HOLChatBoardScript.as)
BUILD POUR CHANGER UNE RÈGLE: NON (P1c)
RPC EN SCRIPT: NON CONCLU
TYPES_A_EXPOSER: aucun

## P1d : sortie de la sonde (21:27)
start 21:27:08
server up: True 21:27:33
red client sees probe: False 21:29:39
RED CHECK 21:29:47: server line 'received by the server' present = False
green client pong: False 21:31:49
=====SERVER
=====RED CLIENT
=====GREEN CLIENT
LogCsvProfiler: Display: Metadata set : commandline="" C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\HeroesOfLiberty.uproject 127.0.0.1:7987 -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0
LogInit: Command Line: 127.0.0.1:7987 -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=RpcCli2 -abslog=C:\Users\baudo\AppData\Local\Temp\claude\82f06dab6ac779ae\611

[exited with code 0]

Journal du serveur (rpc_server.log, scratchpad, lignes RpcProbe / Join) :
[2026.11.16-20.27.57:004][600]LogNet: Join succeeded: Player
[2026.11.16-20.30.04:433][360]LogNet: Join succeeded: Player

Journal du client rouge :
LogPluginManager: Mounting Engine plugin PluginBrowser
LogPluginManager: Mounting Engine plugin ContentBrowserClassDataSource
LogPluginManager: Mounting Engine plugin ContentBrowserAssetDataSource
LogPluginManager: Mounting Engine plugin ContentBrowserFileDataSource
[2026.11.16-20.27.48:285][  0]LogModuleManager: InternalLoadLibrary: 'ContentBrowser' ('C:/UE/UE_5.8_HOL/Engine/Binaries/Win64/UnrealEditor-ContentBrowser.dll')
[2026.11.16-20.27.48:286][  0]LogModuleManager: InternalLoadLibrary: 'ContentBrowserData' ('C:/UE/UE_5.8_HOL/Engine/Binaries/Win64/UnrealEditor-ContentBrowserData.dll')
