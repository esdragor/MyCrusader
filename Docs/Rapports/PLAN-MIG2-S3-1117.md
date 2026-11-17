# PLAN-MIG2-S3-1117 : les 57 RPC du contrôleur appelables du script (ELROND-1117h, 2026-11-17)

Lecture seule + plan. Aucun build, aucun design tranché (les questions sont dans `Retours\Q-mig2-s3.md`). Sources : ligne MIG2-S3 de
`Auto_Journey_File.md` (L108), `ANALYSE-1116a.md` (§3 « RPC et réplication », étape 1 « socle de liaisons », risque « RPC en script non prouvées »),
`Source\HeroesOfLiberty\Player\HOLPlayerController.h` (déclarations) et `.cpp` (longueur des `_Implementation`, mesurée en lignes jusqu'à la fonction suivante).

## 1. Ce qui est acquis (P1d prouvé)

- Une `UFUNCTION(Server)` et une `UFUNCTION(Client)` écrites en script passent avec Iris : la sonde `Script\Net\HOLScriptRpcProbe.as` (acteur script
  possédé par le contrôleur, créé par le pont C++ `HOLScriptRpcProbeBridge` dans `PostLogin`) reçoit côté serveur, répond côté client.
- Cause de l'échec antérieur : dossier `Script\Dev` sauté hors éditeur interactif (règle : jamais de code serveur ou de jeu dans `Dev`, `Developers`, `Examples`, `Editor`).
- Donc DEUX lectures de « RPC appelables du script » deviennent possibles (c'est la question Q1) :
  - **A. RPC déclarées dans un acteur script** possédé par le contrôleur (patron de la sonde) : le corps de la RPC est en script ; le contrôleur C++ perd la déclaration.
  - **B. RPC gardées en C++ comme enveloppe d'une ligne** qui appelle le script (pont) ; ou, côté client, **appelables du script** (le script appelle `Controller.ServerX(...)`).
  - **C. Paire générique** (demande client, réponse serveur) étendue par le script (ANALYSE ligne 263) : hors de ce plan, voir Q1.

## 2. Les 57 RPC en lots (51 `Server`, 6 `Client`)

La longueur donnée est celle de l'`_Implementation` C++ (lignes). `ServerExecRPC` (surcharge du moteur, 61 lignes, console serveur) n'est pas dans les 57 : à garder en C++.

| Lot | RPC (nombre) | Nature | Lignes C++ | Difficulté |
|---|---|---|---|---|
| **L1 Diplomatie** | `ServerDiplomacyStance`, `ServerDiplomacyVision` (2) | 2 `int32` + 1 `bool`, la règle est déjà `HOLDiplomacy::SetStance` (C++ seule source, D836) ; le corps empile un ordre dans `UHOLCommandSubsystem::IssueDiplomacyOrder` | 12, 11 | TRIVIAL |
| **L1 Duel** | `ServerDuelChallenge`, `ServerDuelAnswer` (2) | scalaires, règle déjà en script (`Script\Duel\`) | 12, 16 | TRIVIAL |
| **L1 Partie** | `ServerRequestPause`, `ServerRequestResume`, `ServerSurrender` (3) | sans argument ou 1 `bool`, état de partie | 9, 9, 21 | TRIVIAL |
| **L1 Spécialisation** | `ServerStartSpecialization`, `ServerCancelSpecialization` (2) | 2 `int32`, pont déjà existant (1bce302) | 17, 30 | TRIVIAL |
| **L1 Héros** | `ServerReturnHero`, `ServerHeroTalent`, `ServerHeroAbility` (enum + `FVector_NetQuantize`), `ServerCastRage` (`TArray<uint32>`), `ServerWatchUnit` (5) | règles déjà en script (`Script\Hero\`, `Script\Talents\`) sauf rage (C++ seule source, D836) | 29, 37, ~20, 10, 35 | FACILE (types `enum`, `uint32`, vecteur à vérifier) |
| **L1 Signaux et chat** | `ServerPostSignal`, `ClientReceiveSignal`, `ServerPostChat`, `ClientReceiveChat` (4) | la règle du chat est déjà en script (`HOLChatBoardScript.as`) : la RPC reste la seule part C++ ; `FString`, `FVector` | 37, ~47, ~46, ~27 | TRIVIAL (le chat est le candidat naturel) |
| **L2 Ordres du joueur** | `ServerMoveUnits`, `AttackUnit`, `HealUnit`, `AttackBuilding`, `AttackMove`, `StopUnits`, `ToggleDeploy`, `MoveUnitsInLine`, `GatherUnits`, `PlaceBuilding`, `BuildUnits`, `DropOffUnits`, `GarrisonUnits`, `UngarrisonAll`, `CancelBuilding`, `SetRallyPoint` (16) | `TArray<uint32>` + `FVector_NetQuantize` + `AHOLBuilding*` ; le corps appelle `HOLPlayerOrders::<Ordre>(*World, slot, ...)` (C++, règle d'IA0.2) ; ~8 à 20 lignes chacun (HealUnit 17, SetRally ~23) | ~250 en tout | MOYEN : types à exposer, fréquence élevée (un clic = une RPC), `HOLPlayerOrders` est une fonction de namespace invisible du script (ANALYSE §3) : bibliothèque à écrire |
| **L3 Économie** | `ServerQueueProduction`, `ServerCancelProduction`, `ServerMarketTrade`, `ServerTribute` (4) | `AHOLBuilding*`, `FName`, `int32` ; le marché a déjà sa règle en script (`HOLMarket::Apply`, GALADRIEL-1117f, non compilée) | 15, 15, ~17, ~25 | FACILE à MOYEN (dépend de `UHOLMarketSubsystem` du lot C++ de GALADRIEL-1117f) |
| **L4 Cheats** | `ServerCheatResources` (`FHOLResources`, `USTRUCT(BlueprintType)`), `FastBuild`, `Population`, `Technology`, `Tier`, `UnitSpeed`, `GatherSpeed`, `Wound`, `DamageBuilding`, `SpawnBuilding`, `Defeat`, `FallForum`, `SkipForum`, `HoldPresence` (14) | scalaires sauf 3 ; 17 à 41 lignes chacun, refusés hors cheat (R3.5), touchent beaucoup de sous-systèmes | ~330 | FACILE chacun, valeur faible (outils de test) : lot de remplissage |
| **L5 Charges utiles** | `ServerRequestMatchStats`, `ClientReceiveMatchStats` (`FHOLMatchStatsChunk`), `ClientMatchStatsRefused`, `ClientReceiveFogMemory` (`FHOLFogGhostNet`, `TArray<uint32>`), `ClientShowDepositPopups` (`FHOLDepositPopup`, non fiable) (5) | structures `USTRUCT()` sans `BlueprintType` (3 sur 3 : `HOLMatchStats.h:161`, `HOLVisionSubsystem.h:15`, `HOLPlayerController.h:50`) donc invisibles du script ; fiabilité et débit à garder | 33, 8, ~10, ~20, 786 (avec l'écran) | DIFFICILE : liaison de structures d'abord ; candidats à rester en C++ |

Total : 18 (L1) + 16 (L2) + 4 (L3) + 14 (L4) + 5 (L5) = 57.

**Par pas ou Mass** : AUCUNE des 57 RPC n'est appelée par pas ni par entité : ce sont des points d'entrée du joueur (un clic, une touche). Le seul lien avec Mass est
aval : L2 pousse des ordres que la simulation Mass lit (`HOLPlayerOrders`, `UHOLCommandSubsystem`). Le script ne fait que valider et empiler ; l'exécution reste en C++.
Les trois fichiers à `ParallelFor` restent exclus (ANALYSE §6).

## 3. Ce que le lot C++ (une seule fenêtre de build) doit exposer avant les tâches de script

Pas de C++ avant la réponse à Q1. Selon la réponse : (a) `UHOLCommandSubsystem::IssueDiplomacyOrder` + `EHOLDiplomacyOrderKind` (L1 Diplomatie) ; (b) `HOLPlayerOrders::*` par une bibliothèque
(L2) ; (c) `FVector_NetQuantize`, `TArray<uint32>`, `AHOLBuilding*` : à VÉRIFIER dans le script avec un test d'une ligne avant tout (NON VÉRIFIÉ ici) ; (d) un pont générique qui crée, dans `PostLogin`,
tout acteur script dérivé d'une base C++ (généralise `HOLScriptRpcProbeBridge`, aujourd'hui une seule sonde) ; (e) pour L5 : `BlueprintType` sur 3 structures.

## 4. Pilote proposé (sous réserve de Q1 : réponse A ; si B, même RPC, la déclaration reste en C++ et le test est le même)

**`ServerDiplomacyStance(int32 TargetSlot, int32 Stance)`** : la plus petite RPC utile (corps de 12 lignes, 2 `int32`, aucune référence d'acteur, aucun lien avec la simulation autre qu'un ordre empilé),
déjà branchée à une UI en script (`Script\Diplomacy\HOLDiplomacyClick.as` + `HOLDiplomacyPanel.as`) et déjà testée (`HOLDiplomacyRules_Test.as`, 37 Ko) donc le comportement attendu est connu.
Budget : ~100 lignes (60 de script, 40 de test), sans C++ si `IssueDiplomacyOrder` est déjà visible, sinon 1 binding.

1. **Test rouge d'abord, en script** (`Script\Tests\HOLRpcDiplomacy_Test.as`) : sur un monde de test, une fonction script pure `ResolveStanceOrder(senderSlot, targetSlot, stance)` donne l'ordre à empiler (slot de l'émetteur, jamais celui qu'un client prétend : D288 ; spectateur refusé ; hors bornes refusé). Rouge VU : fonction absente ou valeur faussée une fois.
2. **Vert** : l'acteur script par contrôleur (`AHOLScriptRpcActor`-dérivé, patron de la sonde, dossier `Script\Net\`, JAMAIS `Dev`) déclare `UFUNCTION(Server) ServerDiplomacyStance` qui lit le slot par `AHOLPlayerState` du propriétaire, appelle `ResolveStanceOrder`, empile l'ordre. Le journal garde « Diplomacy order: slot N stance S toward slot T queued » (même ligne que le C++).
3. **Preuve réseau** : une variante de `Tools\Scenarios\rpc_probe_run.ps1` (serveur + 2 clients) où le client 2 appelle la RPC par une commande console ; ligne attendue dans le journal du serveur ; rouge (cvar d'envoi à 0 : ligne absente) puis vert.
4. Retrait de `ServerDiplomacyStance` du contrôleur C++ seulement après le vert réseau (même règle que pour les tests : rouge puis vert avant suppression) ; ce retrait est du C++ (lot de la séance).

Mesure à noter : lignes C++ supprimées / script ajoutées, builds demandés, et latence d'un aller-retour (comparer à la RPC C++).

## 5. Tâches par agent de 15 min (une tâche = UN test rouge + UN correctif ; script seul sauf mention)

| # | Tâche | Dépend de |
|---|---|---|
| T0 | Vérifier en script (test d'une ligne) que `FVector_NetQuantize`, `TArray<uint32>`, `AHOLBuilding*`, `FName`, `EHOLHeroAbility` sont des types de paramètre d'une `UFUNCTION(Server)` script ; noter lesquels manquent (`TYPES_A_EXPOSER`) | rien |
| T1 | Pilote §4 étapes 1-2 (test rouge + acteur script de la diplomatie) | Q1, T0 |
| T2 | Pilote §4 étape 3 : `rpc_diplomacy_run.ps1` (3 processus ; lancé par la session principale, pas par l'agent) | T1 |
| T3 | L1 Duel (2 RPC) sur le même acteur, tests rouges ; L1 Partie (3 RPC) | T2 vert |
| T4 | L1 Spécialisation (2) + L1 Héros (5) : un test rouge par RPC, un lot de 2 ou 3 par agent | T2 vert |
| T5 | L1 Signaux et chat (4) : les deux `Client` aussi (la RPC client est appelée par le serveur) | T2 vert |
| T6 | Lot C++ de la séance (UN build, session principale) : pont générique de création d'acteurs script, bibliothèque `HOLPlayerOrders`, retrait des déclarations migrées de L1 | T3-T5 |
| T7 | L2 Ordres, par groupes de 4 RPC (4 agents) : mouvement et attaque, construction, garnison, rassemblement ; mesurer la latence | Q2, T6 |
| T8 | L3 Économie (4 RPC) | T6, lot Marché de GALADRIEL-1117f compilé |
| T9 | L4 Cheats, par groupes de 5 (3 agents) : lot de remplissage | T6 |
| T10 | L5 : seulement si Q2 le décide ; `BlueprintType` sur 3 structures (lot C++) puis 5 RPC | Q2 |

Chaque tâche : test rouge VU, vert, `quick_check -Parts tests` (session principale), `rpc_*_run.ps1` pour toute RPC nouvellement migrée avant retrait du C++. Total estimé : 1 tâche de vérification, 1 pilote en 3 séances d'agent, ~10 agents pour L1, ~6 pour L2 à L4, 2 pour L5 : une séance de 12 à 16 agents si Q1 et Q2 sont répondues.

## 6. Risques

- La RPC `Client` du script est appelée par le serveur : l'acteur script doit être `bAlwaysRelevant`, possédé (pas de RPC Server sans propriétaire).
- Changer le propriétaire ou perdre le pont `PostLogin` (spectateur, reconnexion) : une RPC migrée devient muette sans erreur ; chaque RPC migrée a son essai à 2 clients, et un test par spectateur (L1 Signaux).
- Script sur le chemin d'un clic (L2) : rien n'est mesuré (ANALYSE §6, perf) ; mesurer avant de supprimer le C++.
- Un `.as` dans `Dev`, `Developers`, `Examples` ou `Editor` n'est pas chargé hors éditeur : aucune de ces tâches n'y écrit.

## NON VÉRIFIÉ

Aucun type de paramètre de RPC en script autre que `FString` / `int` ; les longueurs d'`_Implementation` sont des écarts de lignes (fonctions annexes comprises) ; nombre « 57 » recompté à la main (51 `Server` + 6 `Client`, `ServerExecRPC` à part) ; `IssueDiplomacyOrder` visible du script non vérifié.
