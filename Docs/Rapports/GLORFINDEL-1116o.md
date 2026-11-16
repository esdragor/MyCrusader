# GLORFINDEL-1116o : migration, MIG-P1d (RPC Server / Client en script avec Iris), 2026-11-16 soir

APPELS_DECLARES: 22 (arrêt avancé par la consigne « 15 min max, avant 21:55 »)
HEURE_DEBUT/HEURE_FIN: ~21:31 / 21:44
PREMIER_BUILD: sans C++   BUILD: aucun   LAUNCHCHECK_DEBUT: ~21:33 vide (DLL à jour sur try-as-rpc)
TEST_ROUGE: non atteint (la sonde ne naît pas, la RPC n'est jamais appelée) ; assertit l'effet : sans objet
COMMITS: 1 (2f2c732 sur try-as-rpc, jetable, JAMAIS fusionnée), dernier à 21:42 ; rien sur net-playtest (710decf intacte) ; merge_main_fait: non (rien à fusionner, branche jetable)
TESTS: une sonde serveur dédié + 2 clients (scratchpad rpc_probe_run2.ps1, 21:35-21:41) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune

## Réponse MIG-P1d : NON PROUVÉ

Une RPC Server / Client écrite en script passe-t-elle (Iris, serveur dédié + 2 clients) ? **NON PROUVÉ** : la sonde n'a pas pu naître sans C++.

- Modif sur try-as-rpc (2f2c732) : le spawner `UHOLScriptRpcProbeSpawner : UScriptWorldSubsystem` journalise son 1er Tick (`RpcProbe: spawner ticks (game modes=…, IsServer=…)`) et teste le serveur par la présence d'un `AGameModeBase` au lieu de `System::IsServer()`.
- Journal du serveur (`-server -nullrhi`, /Game/Maps/Plain, Iris actif) :
  - `[2026.11.16-20.36.05:965][  0]LogWorld: Bringing World /Game/Maps/Plain.Plain up for play (max tick rate 30)`
  - `[2026.11.16-20.36.08:310][  1]LogHOLMatch: AI brain HOLAIBrain awake, net mode DedicatedServer, thinking slots []` (un sous-système script **sous-classe d'un pont C++** naît bien)
  - **aucune** ligne `RpcProbe: spawner ticks`, ni `RpcProbe: the server spawned`, ni `received by the server` jusqu'à 20:39:36 UTC (3 min 30, client connecté).
- Sortie de la sonde : `server spawner ticks: False 21:37:18` / `server spawned: False 21:38:49` / `red client sees probe: False 21:39:30` / `RED CHECK 21:39:36: … received by the server present = False`.
- Cause : une sous-classe script directe d'`UScriptWorldSubsystem` n'est pas créée ou ne tique pas sur le serveur dédié (Initialize non journalisé : lequel des deux NON VÉRIFIÉ). La 1re sonde (1116n) avait le même défaut, masqué par `System::IsServer()`.
- Ce qu'il faut en C++ (ligne MIG-P1d-C++ ajoutée dans « Prochaine séance de migration » de la file) : un point d'entrée serveur, pont C++ (UWorldSubsystem ou GameMode PostLogin), qui cherche la classe script `AHOLScriptRpcProbe` comme `UHOLScriptScreen::FindScriptClass` et la crée par contrôleur ; puis rejouer la sonde (rouge `hol.Dev.RpcProbeSend 0`, vert `1`).

FAIT / RESTE:
- FAIT : diagnostic de la non-naissance de la sonde (2f2c732, try-as-rpc) ; journaux ChatInProgress.md et AngelscriptInProgress.md sur try-as-rpc (NON fusionnés) ; ligne MIG-P1d de la file, ligne MIG-P1d-C++ en Prochaine séance, ligne d'EtapeDefini.md.
- RESTE : MIG-P1d-C++ (point d'entrée serveur en C++) puis la RPC elle-même.
NON VÉRIFIÉ: la RPC Server/Client en script (jamais appelée) ; si le sous-système script pur n'est pas créé ou seulement pas tiqué sur -server ; s'il naît sur un client `-game` (non regardé) ; le lanceur rpc_probe_run2.ps1 est dans le scratchpad (non durable) : à recopier dans Tools si on le garde.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: (1) arrêt avancé par la consigne utilisateur « 15 min max » (pas de 2e essai : log d'Initialize, client -game) ; (2) le 2e client de 1116n entrait en spectateur (pas de -MatchConfig à 2 humains) : sans effet ici, la sonde n'existait pas. EDITS_REFUSES: 0.
GIT_STATUS_PRINCIPALE:
 M Auto_Journey.md
 M Auto_Journey_File.md
 M Auto_Journey_Jour.md
 M CLAUDE.md
 M EtapeDefini.md
 M GDD.md
 M Tools/AutoJourney/brief_commun.md
?? Docs/Priorite_Socle_Factions_2026-11-16.md
?? Docs/Rapports/ELROND-1116n.md
?? Docs/Rapports/HALDIR-1116n.md
?? Docs/Rapports/SAURON-1116o.md
(inchangée hors docs voulus : oui, mes lignes d'Auto_Journey_File.md et d'EtapeDefini.md seulement)

## Champs de migration
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 0
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: 0 (sonde jetable +10 lignes de script sur try-as-rpc)
RPC EN SCRIPT: NON PROUVÉ
TYPES_A_EXPOSER: un point d'entrée serveur en C++ qui crée une classe script (voir MIG-P1d-C++)
