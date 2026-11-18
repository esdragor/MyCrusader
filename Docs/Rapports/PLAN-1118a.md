# PLAN-1118a : retrait des RPC C++ du contrôleur déjà réécrites en script (lecture seule, aucun build, aucun design tranché)

Sources lues : PLAN-MIG2-S3-1117, ELROND-1117q, HALDIR-1117p / 1117n, GLORFINDEL-1117e (bibliothèques de lecture, sans lien avec les RPC), journaux REPRISE 5 et 6.
Code lu : `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` (`HOLPlayerController.h` 1025 l., `.cpp` ~6775 l., `Script\Net\HOLRpcPlayer.as` 857 l., `Net\HOLScriptRpcPlayerActor.h/.cpp` 188 + 368 l.).
Mesures : grep de chaque nom `Server*` / `Client*` / `Request*` dans tout le dépôt (hors commentaires pour les appels). 57 `UFUNCTION(Server|Client)` dans le `.h`.

## 0. Fait structurant (change tous les chiffres)

Les RPC script appellent le pont, et le pont appelle `AHOLPlayerController::Server<X>_Implementation` : le CORPS de la règle reste en C++ (une seule source, D836).
Retirer « la RPC C++ » = enlever `UFUNCTION(Server)` (2 lignes dans le `.h`), la branche de repli `ServerX(...)` dans `Request<X>` et la cvar ; le corps `_Implementation` devient une méthode
ordinaire appelée par le pont (donc ~0 ligne de règle supprimée). Aucun test C++ n'appelle une RPC : tous les `Tests\*.cpp` n'en parlent qu'en commentaire (Chat, Signal, HeroReturn, DamnedWarrior, MarketRules).
Les appels C++ passent tous par `Request<X>` et par les aides `HOLRpcScriptRoute_Haldir1117n` (l.61, l.5221-5342) et `HOLRpcScriptRoute_Elrond1117q` (l.4749-4804) : il n'y a PAS d'appel direct caché (vérifié l.3954, 3996, 4234, 2977, 3448).
Le script appelle les RPC en retour par `Request<X>` (UI : `HOLGameScreenScript.as`).

## 1. Tableau des RPC C++ restantes (57)

Légende : défaut = cvar `hol.Rpc.Script*` ; net = vu dans `quick_check -Parts net,netsig3 -RpcScript` ou `pc_routes(_seats)_run.ps1` (REPRISE 5-6). « Request » = appelants C++ de `Request<X>` hors contrôleur.

| Lot | RPC | Script | Défaut | Preuve réseau | Appelants (C++ / script / tests / scénarios) | À supprimer |
|---|---|---|---|---|---|---|
| Économie | QueueProduction, Tribute, MarketTrade | oui | 1 | OUI (RpcEconomy : file, tribut, échange) | route `RouteQueueProduction/Tribute/MarketTrade` (l.3954, 3996, 4195, 4219, 4234, 4248) ; script `HOLRpcPlayer.as` + `HOLRpcEconomy_Test` ; C++ `HOLMarketRulesTests` : commentaire | décl. `.h` (x3), repli `ServerX` dans Request, 3 fonctions Route, cvar `ScriptEconomy` |
| Économie | CancelProduction | oui | 1 | NON (« file, tribut, échange » seulement) | `HOLGameScreen.cpp` (Request) ; `HOLRpcEconomy_Test` | idem |
| Match | RequestPause, RequestResume | oui | 1 | OUI (RpcMatch pause, reprise ; `rpc_routes_run` Pause) | `HOLGameMenuScreen.cpp/.h` ; `HOLDisconnectTests` appelle `AHOLGameState::RequestPause` (autre fonction, intacte) ; `HOLGameScreenScript.as` (Resume) | décl., repli, `CallN` pause/reprise, cvar `ScriptMatch` (partagée avec Surrender) |
| Match | Surrender | oui | 1 | NON (aucune ligne RpcMatch d'abandon ; `lobby_match.ps1` x2 + `quick_check` l.3030 lisent « Victory: surrender » côté C++) | `HOLGameMenuScreen.cpp` | idem + adapter `lobby_match.ps1` |
| Spécialisation | StartSpecialization, CancelSpecialization | oui | 1 | OUI (RpcSpec démarrer, annuler ; `pc_routes_seats_run`) | `HOLCommandGrid.cpp/.h` (commentaire), `HOLSpecializationBoardScript.as`, `quick_check` l.7257 | décl. x2, repli x2, cvar `ScriptSpecialization` |
| Duel | DuelChallenge | oui | 1 | OUI avec 2 sièges (témoin cvar 0 : rien) | script UI | décl., repli, cvar `ScriptDuel` |
| Duel | DuelAnswer | oui | 1 | NON (ProofRoute bit 1 = défi seul) | `HOLGameScreenScript.as` | idem |
| Héros | ReturnHero | oui | 1 | OUI avec siège (RpcHero: return received, bit 8) | `HOLGameScreenScript.as` | décl., repli, cvar `ScriptHero` |
| Héros | HeroTalent, WatchUnit | oui | 1 | NON | UI script (Talent x2) ; WatchUnit : `CallN` l.3448 | idem |
| Héros | HeroAbility, CastRage | oui | 1 | NON ; PAS de route C++ (l.5395, 5402 appellent la RPC C++ directe, le script ne les reçoit jamais) | aucun appelant externe | route à ajouter AVANT retrait |
| Diplomatie | DiplomacyStance | oui | 1 | PARTIEL (pilote `rpc_diplomacy_run` : reçue, « refused (1) » faute de siège sur Plain ; effet avec siège jamais vu) | `HOLGameScreenScript.as` x2 -> `RequestStance` ; `rpc_diplomacy_run.ps1` | décl., `RequestStance` l.5700, cvar `ScriptDiplomacy` |
| Diplomatie | DiplomacyVision | NON | - | - | `HOLGameScreenScript.as` (`RequestShareVision`) | à MIGRER d'abord (pas de script) |
| Signal | PostSignal | oui | 1 | OUI (RpcSignal reçu serveur) | `HOLSignalBoardScript.as`, `quick_check` l.7243 | décl., repli, cvar `ScriptSignal` |
| Chat | PostChat | oui | 1 | OUI (RpcChat reçu serveur) | `HOLChatBoardScript.as`, `quick_check` l.8610 | décl., repli, cvar `ScriptChat` |
| Signal / Chat | ClientReceiveSignal, ClientReceiveChat | oui | 1 (cvar du serveur) | PARTIEL (ligne « received » client non citée ; spectateur non vu) | `HOLGameState.h`, `HOLSignalInput.h`, 2 tests C++ (commentaires), `quick_check` x4 | décl. x2 + repli dans les 2 `_Implementation` des Post |
| Triches | CheatResources | oui | 1 | OUI avec siège (RpcCheat: resources received, bit 16) + refus | `HOLCheatPanel.cpp` x5 | décl., `CallCheat`, cvar |
| Triches A | FastBuild, Population, Technology, Tier, UnitSpeed, GatherSpeed (6) | oui | 0 | NON | `HOLCheatPanel.cpp` (FastBuild, Population, UnitSpeed, GatherSpeed) | décl. x6, repli, 6 cvars |
| Triches B | Wound, DamageBuilding, SpawnBuilding, Defeat, FallForum, SkipForum, HoldPresence (7) | oui | 0 | NON | `HOLBuildingCrowdTest.cpp` (Dev : DamageBuilding, SpawnBuilding, AttackBuilding/Move) | décl. x7, repli, 7 cvars |
| Ordres L2 | Move, AttackUnit, HealUnit, AttackBuilding, AttackMove, Stop, ToggleDeploy, MoveInLine, Gather, PlaceBuilding, BuildUnits, DropOff, Garrison, UngarrisonAll, CancelBuilding, SetRallyPoint (16) | NON | - | - | `HOLMinimapView`, `HOLCrossingTest`, `HOLBuildingCrowdTest`, `quick_check` (HealUnit), `HOLHealProcessor.h` | rien (pas de script) |
| Charges L5 | RequestMatchStats, ReceiveMatchStats, MatchStatsRefused, ReceiveFogMemory, ShowDepositPopups (5) | NON | - | - | `HOLMatchStatsSubsystem`, `HeroesOfLibertyGameModeBase`, `HOLGatherProcessor.cpp` (par pas), `quick_check` | rien |

Hors des 57 : `ServerExecRPC` (surcharge moteur) et `ClientSetHUD` : à garder.
Décompte : 35 en script (Stance 1, Duel 2, Match 3, Spec 2, Héros 5, Triches 14, Signal/Chat 4, Économie 4) ; 22 sans script (Vision 1, L2 16, L5 5).

## 2. Classement

- **SÛR à retirer maintenant (12)** : QueueProduction, Tribute, MarketTrade, RequestPause, RequestResume, StartSpecialization, CancelSpecialization, DuelChallenge, ReturnHero, CheatResources, PostSignal, PostChat (défaut 1 + ligne réseau vue + route C++ existante + cvar). Condition commune : le repli devient un journal d'erreur, pas un silence (Q-t6-repli-sans-script).
- **À PROUVER d'abord (23)** : CancelProduction, Surrender, DuelAnswer, HeroTalent, WatchUnit, HeroAbility et CastRage (route C++ en plus), DiplomacyStance (effet avec siège), 13 triches (défaut 0), ClientReceiveSignal et ClientReceiveChat (client + spectateur). Preuve = un scénario avec sièges + cheats actifs, rouge (cvar 0 ou client sans appel) puis vert.
- **NE PAS retirer (22)** : ordres L2 (un clic = une RPC, 16 RPC sans script, la liaison `HOLPlayerOrders` n'existe pas, latence jamais mesurée, ordres de la simulation Mass : Q-t6-ordres-L2) ; L5 (réplication : structures non `BlueprintType`, `ClientShowDepositPopups` appelée par pas par `HOLGatherProcessor`, non fiable par choix) ; DiplomacyVision (pas de script : à migrer, 1 petit lot, puis à prouver).

## 3. Énoncés d'agent (un build au total : lot C++ de la séance, lancé par la session principale ; worktree dédié, `EtapeDefini.md` et journal à chaque pas)

**Lot A - Économie 3 + Match 2 (5 RPC)** : (1) Fichiers : `HOLPlayerController.h/.cpp`, `Script\Net\HOLRpcPlayer.as`, `Net\HOLScriptRpcPlayerActor.*`, `Tools\Tests\quick_check.ps1`. (2) Test rouge d'abord : une porte `quick_check` (grep) qui exige l'absence de `UFUNCTION(Server...)` sur ces 5 noms dans le `.h` ; rouge VU maintenant. (3) Ordre : `Request<X>` n'appelle plus que la route (repli = `UE_LOG Error` + rien), puis décl. `.h`, puis `_Implementation` -> méthode `Do<X>` appelée par le pont, puis retrait des gardes `CVar` du script pour ces RPC. (4) Ne pas toucher Surrender ni les cvars partagées (`ScriptMatch`). (5) Compiler : un build `Tools\build.ps1 -LiveCoding=0 -Wait` (session principale), puis `quick_check -Parts tests` et `-Parts net,netsig3 -RpcScript` (RpcEconomy, RpcMatch). (6) Rendre : lignes retirées, journal, Placeholders non concerné.

**Lot B - Spécialisation 2 + DuelChallenge + ReturnHero (4 RPC)** : (1) mêmes fichiers + `Tools\Scenarios\rpc_routes_seats_run.ps1`. (2) Rouge : porte grep sur les 4 noms. (3) Ordre : routes seules, décl., `Do<X>`, garde script ; la cvar `ScriptSpecialization` disparaît seulement si les 2 Spec sont les seules ; `ScriptDuel` et `ScriptHero` restent tant que DuelAnswer et les autres Héros ne sont pas prouvés. (4) Le témoin rouge « cvar 0 : rien » de `rpc_routes_seats_run` change : voir Q-t6-temoin-cvar. (5) Compiler : même build que le lot A. (6) Rendre : idem.

**Lot C - PostSignal, PostChat, CheatResources (3 RPC)** : (1) mêmes fichiers + `HOLCheatPanel.cpp` (lecture seule, ne change pas). (2) Rouge : porte grep. (3) Ordre : `RequestSignal`, `RequestChat`, `RequestCheatResources` : route seule ; retirer les 3 décl. ; NE PAS retirer les 2 relais Client des `_Implementation` (repli sur la RPC C++ des ClientReceive : lot « À prouver P4 »). (4) Un spectateur doit encore parler (D376) : garder la porte `ChatRefusal`. (5) Compiler : même build ; `quick_check` lignes SG2 et A4 (chat). (6) Rendre : idem.

**P1 - preuves Héros + Diplomatie avec siège (script + scénario, sans C++ sauf routes HeroAbility / CastRage)** : (1) `pc_routes_seats_run.ps1` : ajouter ProofRoute bits Talent, Watch, Answer, Stance, CancelProduction. (2) Rouge : témoin cvar 0 ou client muet : aucune ligne serveur. (3) Vert : lignes `RpcHero`, `RpcDuel`, `RpcDiplomacy`, `RpcEconomy` avec slot >= 0. (4) HeroAbility / CastRage : router `Request*` dans le lot C++ (2 x 3 lignes). (5) Compiler : build du lot C++ seulement s'il y a des routes à ajouter. (6) Sortie : tableau preuve oui/non.
**P2 - 13 triches** : scénario serveur cheats actifs + 13 cvars à 1 (serveur et client), une ligne `RpcCheat: <nom> received` par triche, rouge = cheats inactifs (refus R3.5) ; ensuite lot de retrait comme A.
**P3 - Surrender + Match** : adapter `lobby_match.ps1` aux lignes `RpcMatch`, rouge cvar 0, vert cvar 1 ; **P4 - ClientReceive** : essai 2 clients + 1 spectateur, lire « received » côté client et spectateur.

## 4. Risques réseau

- **Client sans script** (script non chargé, compilation script en échec, build ancien) : `Call*` ne trouve pas la fonction, retourne faux ; aujourd'hui repli sur la RPC C++, après retrait RIEN ne part : action muette sans erreur (déjà noté PLAN-MIG2-S3 §6). Mitigation : repli = journal d'erreur + porte `quick_check` « script chargé » avant `waitplayers`.
- **Serveur dédié** : l'acteur script est créé dans `PostLogin` (`HeroesOfLibertyGameModeBase.cpp:293`, journal « RpcPlayer: no script class derived ... » l.131 du pont) ; si `Script\Net` n'est pas chargé (jamais dans `Dev`), toutes les RPC retirées meurent. Un client et un serveur de versions différentes sont de toute façon incompatibles (indices de RPC), le retrait n'ajoute pas de cas.
- **Spectateur** : sans siège, `GetSenderSlot` = -1 : refus voulu (Diplomatie, triches ; le C++ laissait passer `hol.Cheat.Tier -2`, écart noté par ELROND-1117q) ; Signal et Chat doivent rester permis (D376). Création de l'acteur script pour un spectateur : non vue, à prouver (P4).
- **Ordre entre canaux** : RPC du contrôleur et RPC de l'acteur script sont sur deux acteurs ; l'ordre « sélection puis ordre » n'est pas garanti entre eux : sans objet pour les 12 sûrs, bloquant pour L2.
- **Reconnexion / changement de propriétaire** : l'acteur n'est recréé que par `PostLogin` ; essai de reconnexion absent.
- **Fichier unique** : `HOLPlayerController.cpp`/`.h` et `HOLRpcPlayer.as` sont édités par tous les lots (conflits déjà vus à la fusion 1117d) : lots A, B, C en SÉRIE sur un seul worktree.

## 5. Lignes supprimables (estimation, non compilé)

- 12 RPC SÛRES : décl. 2 l. + repli et ligne de route 3-4 l. par RPC = ~65 l. `.h`+`.cpp`, moins ~12 l. de méthode `Do<X>` ajoutée = ~53 net ; cvars/gardes script : ~12 x 3 + 8 déclarations = ~45 l. script ; aides `Call*`/`Route*` : inchangées (restent la route).
- 23 À PROUVER : ~115 l. C++ net (23 x 5) + ~80 l. script (gardes + 20 cvars) ; fusion des 3 aides (`CallN`, `CallEx`, `CallCheat` : ~190 l. en 3 namespaces) en une : ~90 l. gagnées, à faire en fin de chantier.
- Total C++ : ~53 (maintenant) + ~115 (après preuves) + ~90 (fusion des aides) = ~260 l. sur 6775 + 1025 ; script : ~125 l. Les corps des règles restent (~900 l. d'`_Implementation`), car le pont les appelle : les gagner demanderait de les porter en script (Q-t6-ordres-L2, Q2).
- Non supprimables : 22 RPC (L2, L5, Vision) + `ServerExecRPC`.
