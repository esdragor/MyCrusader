# PLAN-1118b : migration des 16 RPC d'ordres L2 vers le script d'abord (D845), puis retrait des RPC C++ (lecture seule, rien compilé, aucun design tranché)

Lu (worktree Elrond, tip 7a16bf1) : `Player\HOLPlayerController.h` l.749-810 et `.cpp` (corps l.4278, 4629-4726, 5805-5908), `Commands\HOLPlayerOrders.h` (339 l. de .cpp), `Net\HOLScriptRpcPlayerActor.h` (pont, 188 l.), `Script\Net\HOLRpcPlayer.as` (958 l.), commit df4e6ce (lot A), PLAN-1118a §0-§4.
Noms réels C++ : ServerMoveUnits, StopUnits, MoveUnitsInLine, GatherUnits, DropOffUnits, GarrisonUnits (les 10 autres = nom de la demande). Les 16 sont `UFUNCTION(Server, Reliable)` du contrôleur ; 17 sites d'appel, tous dans `HOLPlayerController.cpp` (`Request<X>` + OrderAt l.6268 + groupe de bâtiments l.2979).

## 1. Les 16 RPC (corps en lignes, commentaires compris)

Tous les corps commencent par `GetHOLPlayerState()` (le slot vient du contrôleur propriétaire, jamais du client, D288) ; l'ordre entre ensuite dans la file `UHOLCommandSubsystem` (traitée au pas par les processeurs Mass : le corps de RPC ne touche jamais une entité Mass). Le plafond de 200 unités et le refus d'un spectateur sont dans `HOLPlayerOrders::IsAccepted`.

| RPC | Paramètres | Corps | Le corps appelle | Types déjà prouvés en RPC script ? |
|---|---|---|---|---|
| Move | ids, point, bQueue | 9 | `HOLPlayerOrders::Move` | ids : oui (CastRage) ; FVector_NetQuantize : oui (HeroAbility) |
| AttackUnit | ids, uint32 cible, bQueue | 8 | `::Attack` | uint32 scalaire : oui (WatchUnit) |
| HealUnit | ids, uint32 cible, bQueue | 41 | `UHOLUnitSubsystem`, `FHOLCampTable::AreHostile`, `UHOLHealProcessor::OrderHeal`, puis `::Move` | idem |
| AttackBuilding | ids, bâtiment, bQueue | 8 | `::AttackBuilding` | AHOLBuilding : oui (économie) |
| AttackMove | ids, point, bQueue | 7 | `::AttackMove` | oui |
| Stop (StopUnits) | ids, bHold | 7 | `::Stop` | oui |
| ToggleDeploy | ids | 7 | `::ToggleDeploy` | oui |
| MoveInLine | ids, 2 points, bWidth, bQueue | 11 | `::Move` (FrontLine = Fin - Début) | 2 FVector_NetQuantize dans UNE RPC : jamais vu |
| Gather (GatherUnits) | ids, int source, bQueue | 9 | `::Gather` (+ `SendNonWorkersTo`) | oui |
| PlaceBuilding | FName, point, ids, bQueue | 9 | `::Place` | FName : oui ; FName + point + tableau ensemble : jamais vu |
| BuildUnits | ids, bâtiment, bQueue | 9 | `::Build` | oui |
| DropOff (DropOffUnits) | ids, bâtiment, point, bQueue | 9 | `::DropOff` | combinaison jamais vue |
| Garrison (GarrisonUnits) | ids, bâtiment | 8 | `::Garrison` (TakesGarrisonOrder relu côté serveur) | oui |
| UngarrisonAll | bâtiment | 7 | `::UngarrisonAll` | oui |
| CancelBuilding | bâtiment | 13 | `UHOLCommandSubsystem::IssueConstructionOrder` (Cancel) | oui |
| SetRallyPoint | bâtiment, point, int source, site, bQueue | 22 | `UHOLSourceSubsystem`, `IssueProductionOrder` (Rally) ; n'appelle PAS `HOLPlayerOrders::SetRally` (doublon existant) | 2 AHOLBuilding + point + int + bool : jamais vu |

Total des corps : ~184 lignes. « ids » = `TArray<uint32>` (~4 octets l'unité, 200 au plus). Aucun test C++ n'appelle une de ces RPC ; les tests (GatherTests, GarrisonOrderTests, AIOrdersTests...) appellent `HOLPlayerOrders::*` : ils ne bougent pas, l'IA non plus (pas de RPC).
Types : tous compilent déjà SEULS en RPC script (T0 de 2026-11-17 et lots A-C). Reste à prouver les COMBINAISONS (5 paramètres, 2 points, 2 bâtiments) : un T0 d'une séance (§5, lot S0), sans C++.

## 2. Ordre entre canaux (PLAN-1118a §4)

- **La sélection est un état CLIENT** (`GetCommandUnits`, `GetOwnSelection`). Chaque ordre transporte sa liste d'ids dans la RPC : aucun des 16 ne lit une sélection serveur. Donc « sélection puis ordre » ne peut pas arriver dans le mauvais ordre : il n'y a pas de sélection envoyée. Seul état serveur de ce genre : `WatchUnit` (déjà en script, hors des 16, sert à l'affichage de la charge).
- **Risque réel = l'ordre entre deux ORDRES** : `bQueue` (Maj) : Move puis Move(queue), Place puis BuildUnits, Stop puis Move. Deux RPC fiables d'un même acteur gardent leur ordre ; deux acteurs (contrôleur et acteur script) : aucune garantie établie (non vérifié avec Iris ici ; PLAN-1118a §4 : « non garanti »). Une migration PAR FAMILLES mêlerait les canaux (Move script, Stop C++) : inversions possibles.
- **Déjà vrai aujourd'hui** : HeroAbility, CastRage, WatchUnit, QueueProduction, CancelProduction, Tribute... sont déjà sur l'acteur script alors que les 16 ordres sont sur le contrôleur (ex. un Move puis CastRage). Migrer les 16 DIMINUE le mélange (les ordres d'unités et de bâtiments passent tous sur le même acteur).
- **Références d'objet** (`AHOLBuilding*`) : un bâtiment inconnu du serveur arrive nul ; tous les corps testent déjà la nullité (aucun changement).
- **SetRallyPoint en rafale** : un clic de groupe envoie UNE RPC PAR bâtiment producteur (l.2979) : la mesure compte N RPC par clic.
- Preuve à faire (§4) : 100 paires Move + Move(queue) puis Stop + Move, comptage des inversions ; en mode mélangé (cvar par famille) pour MONTRER le risque, en mode unique pour le prouver nul.

## 3. À exposer en C++ (UN lot, un build)

1. **Corps `Do<X>` des 16** : `_Implementation` -> méthodes ordinaires du contrôleur (patron de df4e6ce ; `friend UHOLRpcBridgeLibrary` existe) : ~16 x 2 lignes d'en-tête, corps inchangés (~184 l. déplacées, ~0 supprimée).
2. **Pont** `UHOLRpcBridgeLibrary` : 16 `UFUNCTION(BlueprintCallable)` `(AActor* RpcActor, ...)` ; `uint32` n'est pas BlueprintCallable : ids en `TArray<int32>` recopiés (patron CastRage), `FVector` ; ~6 l. cpp + 3 l. .h par fonction = ~145 l. Variante à discuter : une bibliothèque `HOLPlayerOrders` exposée directement (12 fonctions de l'espace de noms + 4 cas de corps du contrôleur) : ~100 l., mais le slot doit alors venir de `GetSenderSlot` côté script.
3. **Appel client** : `CallFull` (l.5236-5300) ne sait passer ni `TArray<uint32>`, ni 2 points, ni 2 objets (c'est pourquoi CastRage « n'est pas routé du C++ ») : une fonction `CallOrder` à paramètres typés (tableau d'ids, 2 points, 2 objets, FName) : ~70 l. ; les 17 sites `Request<X>` passent par elle : ~17 x 2 l. ; repli = `LogNotSent` (patron lot A).
4. **Mesure** (§4) : cvar client `hol.Rpc.OrderScript` (0 = RPC C++ actuelle, 1 = script ; défaut 0 jusqu'au verdict) + 2 lignes de journal par ordre : ~40 l.
Estimation : ~+300 l. C++ (bonne marge : +-30 %), script ~+300 l. (16 RPC de ~14 l. : garde slot >= 0, plafond 200, ids non vides, journal `RpcOrder: <nom> received`, appel du pont ; fonctions de refus testables comme `HOLRpcEconomy_Test`). Retrait final (§5 lot R) : ~-32 l. de déclarations, ~-20 l. de replis, cvar ; les ~184 l. de corps restent (D836 : une source).

## 4. Mesure de la latence d'un clic

**Ce qu'on mesure** : de l'appel client `Request<X>` à l'entrée du corps serveur. Un clic = un `Request<X>` (le geste est en C++ dans les deux modes : la mesure ne dépend pas de l'interface).
- **Témoin** : le MÊME binaire, la cvar `hol.Rpc.OrderScript` basculée par le scénario (0 puis 1, en alternance A/B/A/B pour tuer la dérive de machine) ; la RPC C++ existe encore pendant la mesure, c'est ce qui permet la comparaison avant le retrait.
- **Horodatage** : ligne client `OrderSend kind=Move key=<n> t=<FDateTime::UtcNow en ticks>` à l'entrée de `Request<X>` ; ligne serveur `OrderRecv kind=Move key=<n> t=... step=<n° de pas>` en 1re ligne du corps (C++ : `_Implementation` ; script : corps de la RPC, avant le pont). Client et serveur sont deux processus de la MÊME machine : même horloge, écart lu à la milliseconde. Clé = point cible unique (x = 1000 + n) pour la corrélation sans ajouter de champ à la RPC. Mesure complémentaire serveur seule : durée du corps (`FPlatformTime::Cycles64` avant/après) = le coût CPU propre du script (appel script + pont).
- **Scénario** `Tools\Scenarios\order_latency_run.ps1` (modèle `rpc_routes_seats_run.ps1` : serveur dédié `-nullrhi`, 1 client, 2 sièges) : 200 Move par mode, tailles 1 / 50 / 200 ids, plus 100 SetRally de groupe ; sortie : médiane, p95, max par mode, nombre reçu / envoyé, inversions d'ordre.
- **Mesurable en local** : le coût CPU, la file d'attente jusqu'au prochain tick réseau du serveur, l'ordre, la perte de messages. **Non mesurable** : la gigue et la perte d'un vrai réseau (un `Net PktLag=` de simulation est à vérifier dans cette version du moteur), la charge de 8 joueurs.
- **Critère proposé (à présenter à l'utilisateur)** : écart de MÉDIANE script - C++ inférieur à 1 pas de simulation (durée du pas : à relire, non lue ici), p95 inférieur à 1 pas, 100 % reçu, 0 inversion en mode unique. Hors critère : un écart sous 1 ms est du bruit.

## 5. Lots pour agents Sonnet (15 min, 25 appels chacun ; un seul build, lancé par la session principale ; lots en SÉRIE sur UN worktree : `HOLPlayerController.*` et `HOLRpcPlayer.as` sont partagés)

| Lot | Contenu | C++ ? | Fichiers | Sortie |
|---|---|---|---|---|
| S0 | T0 des combinaisons : 4 RPC script de sonde (2 points ; FName + point + ids ; 2 bâtiments + point + int + bool ; ids + bâtiment + point) qui journalisent | non | `Script\Net\HOLScriptRpcProbe.as`, `rpc_probe_run.ps1` | types OK / KO avant d'écrire le pont |
| C1 | `Do<X>` des 16 + pont 8 premières fonctions (Move, AttackUnit, HealUnit, AttackBuilding, AttackMove, Stop, ToggleDeploy, MoveInLine) | oui | `HOLPlayerController.h/.cpp`, `HOLScriptRpcPlayerActor.h/.cpp` | NON COMPILÉ |
| C2 | pont 8 dernières (Gather ... SetRallyPoint) + `CallOrder` + cvar + lignes `OrderSend` | oui | idem | -> BUILD par la session principale |
| A1 | script : 8 RPC d'ordres d'unités + refus + `HOLRpcOrders_Test.as` (rouge vu) | non | `HOLRpcPlayer.as`, `Script\...\_Test` | tests script verts |
| A2 | script : 8 RPC bâtiments / production + tests | non | idem | idem |
| M1 | scénario `order_latency_run.ps1` + `order_order_run.ps1` (inversions) | non | `Tools\Scenarios\` | commande à lancer |
| P1..P4 | preuve réseau par ordre (4 lots de 4 : `hol.Rpc.ProofRoute` étendu d'un bit par RPC) | non (sauf bits) | `rpc_routes_seats_run.ps1` | tableau preuve oui / non |
| R | retrait : 16 déclarations, replis, cvar ; `Tools\Tests\no_cpp_rpc_gate.ps1` + `quick_check` (16 noms) | oui (petit) | idem | après verdict utilisateur |

Preuve réseau par ordre (P) : 2 sièges (Alice slot 0, Bob slot 1), serveur dédié. EFFET assertés, pas seulement la ligne : Move = l'unité arrive (position), AttackUnit/AttackBuilding = PV qui baissent, Heal = PV de l'allié qui montent, Stop/Hold = vitesse nulle, ToggleDeploy = état monté / démonté, Gather = ressource reçue, Place = fondation présente et coût payé, BuildUnits = progression de construction, DropOff = stock reçu, Garrison / UngarrisonAll = occupants, Cancel = fondation retirée et remboursement, SetRally = point lu sur le bâtiment. Rouge = client qui n'envoie rien : 0 ligne serveur, 0 effet ; ou refus d'un spectateur / d'un siège de l'autre camp (ordre sur l'unité d'autrui sans effet).

## 6. Risques et questions

Risques : (a) ordre entre canaux en migration par familles (§2) ; (b) script non chargé côté client ou serveur : plus de repli, action muette (LogNotSent + porte « script chargé » de quick_check, déjà au lot A) ; (c) coût par clic de la copie `TArray<int32>` et de l'entrée script : mesuré en §4 ; (d) `CallFull` ne sait pas passer ces types (§3) : le lot C2 est le plus gros (~25 appels : le couper en C2a / C2b si besoin) ; (e) HealUnit (41 l.) et SetRally (22 l.) ont de la règle dans le corps : ne pas les réécrire, les déplacer ; (f) clics de l'IA : hors sujet (appel direct, pas de RPC) ; (g) `HOLCrossingTest` et `HOLBuildingCrowdTest` (Dev) appellent `Request*` : ils servent de charge de test gratuite.

**Q1 : le critère de latence.** Contexte : sur la même machine on ne mesure que le CPU, la file du tick réseau et l'ordre, pas un vrai réseau. A) critère §4 (médiane et p95 < 1 pas, 0 inversion) en local seulement. B) A + une passe avec retard réseau simulé (si le moteur le permet) et un siège sur une 2e machine. Autre. Recommandation : A pour décider du retrait, B avant la sortie publique ; défaut codé : cvar `hol.Rpc.OrderScript` = 0 jusqu'au verdict.
**Q2 : migrer les 16 d'un coup ou par familles.** Contexte : les ordres en file (Maj) dépendent de l'ordre d'arrivée. A) les 16 derrière UNE cvar, un seul canal (ordre sûr, preuve plus grosse). B) 2 familles (unités puis bâtiments) avec cvars séparées (lots plus petits, mélange des canaux pendant la transition). Autre. Recommandation : A ; défaut codé : une seule cvar `hol.Rpc.OrderScript`.


## Réponses de l'utilisateur (2026-11-18, D846)
- Q1 critère de latence : A (local seulement, médiane et p95 < 1 pas, 100 % reçu, 0 inversion).
- Q2 bascule : A (une seule cvar hol.Rpc.OrderScript pour les 16 ordres).
