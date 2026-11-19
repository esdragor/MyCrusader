# BILAN-CPP-1119-suite : corps lus, jugement fonction par fonction (lecture seule, HALDIR-1119co)

Code : `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel`, branche integ-22 (lecture seule, aucun build, aucune édition).
Suite de `BILAN-CPP-1119.md` (§ 3, 4, « non mesuré »). LU = compté ou lu dans le corps aujourd'hui (lignes = plage entre deux débuts de fonction). ESTIMÉ = jugement, ou fonction non lue. Les corps ont été lus par extraits ciblés (les `if` de règle, les appels, pas chaque ligne de journal).

## 1. Les trois fichiers « non jugés »

### 1a. `Player\HOLSelectionMarkers.cpp` (1 453 l.) : rendu pur, 0 décision de jeu (LU)

| Classe | Lignes | Fonctions |
|---|---|---|
| Mass / par pas | 0 | aucune (un `Tick` par image, 39 l., ramasse les `Views`) |
| Réplication | 0 | aucune |
| Rendu / composants | ~1 380 (ESTIMÉ, fichier moins le reste) | `UpdateRings` 129, `UpdateAuraRings` 108, `UpdateDuelRings` 83, `UpdateHealthBars` 95, `UpdateForumRings` 63, `UpdateRelayRings` 77, `UpdateOrderLines` 127, `UpdateGhost` 66, `UpdateFlag` 44, `UpdateCryFlashes` 43, `UpdateHeroFlashes` 25, `UpdateSignalMarks` 38, `UpdateFrame` 31, constructeur / `Init` / `AddFlag` 111, `Describe*` 26 : transformations, instances, couleurs, culling caméra, tracé de lignes au sol |
| Liaison | 0 | (le fichier lit le contrôleur et l'état, il n'est pas exposé au script) |
| RÈGLE OU DÉCISION | ~70 (ESTIMÉ) | voir ci-dessous |

Les seuls fragments qui ressemblent à une règle :
- `ShowsHealthBar` (920-924, 5 l., LU) : barre de vie montrée si sélectionné ou blessé. Décision d'affichage, risque VISUEL seul.
- `UpdateRings` (494-540 et suite, LU sur 47 l.) : l'aperçu de formation du glisser droit (`HOLFormation::GetRowPriority`, `ColumnSpacingInDiameters`) refait côté client ce que fait le serveur ; la règle est dans `HOLFormationRules.cpp` (appelée par Mass), ici seule la présentation. Risque VISUEL.
- `UpdateForumRings` (1020-1082, LU) : `HOLForumRules::GetRing` / `GetRingColor` sont déjà des règles lues ; ici on ne fait que tracer.

Jugement : reste en C++ (rendu par image, instances du moteur, équivalent de « Slate / composants de base » de D839). Migrer = risque VISUEL et coût par image en script, gain nul. Aucune fonction à migrer.

### 1b. `Simulation\HOLHeroSubsystem.cpp` (791 l.) : sous-système par pas ; ~190 l. de gardes, déjà délégué au script pour le fond

| Classe | Lignes | Fonctions |
|---|---|---|
| Mass / par pas (LU) | ~212 | `OnStepEnded` 90 (consomme les files d'ordres : retour du héros, capacités, talents, duels ; chaque verdict est rendu par le script : `Return->BuyNow`, `Return->DecideNow`, `Duels->ChallengeNow/AnswerNow/TickNow`), `TickAbilities` 15, `UpdateSheets` 42, `ClearDuelCircles` 65 (boucle sur toutes les unités visibles à chaque pas, ordre de marche hors du cercle, `HOLDuelClear`) |
| Réplication / copies (LU) | ~77 | `CopyDuelView` 29, `WriteCopies` 14, `RewriteCopies` 10, `FindPlayerState` 18, `GetSheets` 6 : les fiches sont copiées dans l'état du joueur (répliqué) |
| Rendu / Slate | 0 | |
| Liaison | ~57 | `Initialize`, `BindSteps`, `OnWorldBeginPlay`, `Find...` (195-242) |
| RÈGLE OU DÉCISION | ~190 brut (94 LU, 96 ESTIMÉ non lues) | ci-dessous |

Fonctions de la dernière classe (toutes `NM_Client` -> `return false`, donc serveur seul) :
- `SetReturnReady` 21 l. (LU) : passe Mort -> PrêtAuRetour. Décide : porte d'état (2 `if`).
- `RequestReturn` 25 l. (LU) : refuse si retour déjà en attente, camp vaincu, état non mort, forum principal absent ou à terre ; fait naître le héros. Décide : ~10 l. de portes, la naissance est un appel C++ (`SpawnStartingHero`).
- `BuyReturn` 26 l. (LU) : or suffisant ? naissance d'abord, puis débit. Décide : ~12 l. ; le prix vient déjà du script.
- `CastCloseRanks` 22 l. (LU) : `HOLHeroAbility::CanCast` (règle, fichier à part), puis écrit refroidissement, cris, fiche. Décide : 3 l. ; le reste est l'écriture d'état.
- `PlantBanner` 29, `ConsumeCloseRanks` 10 (appelée par un processeur, donc par pas), `GetBanner` 10, `GetCooldownMs` 14, `ResetAbilities` 13, `SetReadyStep` 11, `GetStepsFromNow` 7, `ApplyLevel` 16, `AddXP` 13, `SetXP` 11, `PickTalent` 18, `ResetTalents` 14 : ~166 l., ESTIMÉ, non lues (les nombres de règle sont dans `HOLHeroAbility` / `HOLHeroXP`, autres fichiers).

Vraie décision encore en C++ (LU) : ~35 l. de portes (retour, achat, cri). Risque de migration : RÉSEAU (fiches répliquées, ordre dans le pas, ces fonctions sont appelées depuis `OnStepEnded` et depuis des processeurs Mass : il faudrait des liaisons dans les deux sens). Jugement : le fond (prix, décision de retour, duels) est déjà au script ; ce qui reste est la tuyauterie du pas. Ne vaut pas.

### 1c. `HeroesOfLibertyGameModeBase.cpp` (1 043 l.) : crochets moteur + démarrage ; une seule vraie règle, `GetNomadGroupPoints`

| Classe | Lignes | Fonctions |
|---|---|---|
| Mass / par pas | 0 | |
| Réplication | 0 (tout passe par `AHOLGameState` / `PlayerState`) | |
| Plomberie moteur (crochets `AGameModeBase`) | ~440 (LU, plages) | `InitGame` 57, `PreLogin` 44, `InitNewPlayer` 17, `PostLogin` 104, `Logout` 47, `StartPlay` 40, `InitGameState` 12, `AllowPausing` 15, `SendStartedIfAllHere` 30, `SendEmptyIfNobodyLeft` 19, `UpdateAbsences` 17, `OnMatchOver` 10, constructeur 6, `AssignBuildingOwners` 25 : branchent des sous-systèmes ; ce sont des surcharges du moteur, qui restent en C++ par nature |
| Application d'un verdict / orchestration | ~330 | `PostLogin` : le siège est décidé par `FindSlotForLogin` (GameState) ou `MatchConfig.FindSlotForName`, ici seulement spectateur / remplacement / stock / mémoire du brouillard ; `UpdateStartWait` 70 : `FHOLStartRules::GetMissing` / `ShouldStart` décident, ici on les applique ; `SpawnSlotStart` 80, `SpawnMainCentres` 25, `SpawnAIPlayerStates` 52, `Spawn*At` ~70 : les nombres (`GetNumStartingPeasants`, `ShouldSpawnKing`, `ShouldSpawnHero`) viennent de `FHOLStartRules` |
| Liaison | 0 | |
| RÈGLE OU DÉCISION | ~160 (119 LU + ~40 ESTIMÉ) | ci-dessous |

- `GetNomadGroupPoints` (espace anonyme, 712-830 = ~119 l., LU) : choisit les points de départ des groupes d'un siège nomade : au hasard mais identiques pour un même siège, sur le navmesh, à distance mutuelle, loin des autres départs et des sites actifs ; se replie en rangée si aucun lieu libre. C'est une DÉCISION de jeu (placement) entièrement en C++. Dépend de : `TActorIterator<AHOLStartSlot>`, `UHOLSiteSubsystem`, navmesh, `AHOLMapBounds`, deux cvars, `NomadFarEnough` / `NomadSiteDistance` (déjà script). Risque : RÉSEAU / partie (position de départ des unités du mode nomade, une fois par partie, serveur seul) : un écart casserait « même lieu pour même siège ». Liaison à écrire : projection sur le navmesh, énumération des départs, bornes de carte (ESTIMÉ ~70 l.).
- `UpdateStartWait` : ~20 l. de logique propre (listes attendus / présents, ouverture de la fenêtre d'absence) autour de `FHOLStartRules` ; `SpawnSlotStart` : ~20 l. de branche nomade / normal (ESTIMÉ). Pas de quoi migrer.
- `AllowPausing` : appelle `FHOLCheatRules::IsPauseAllowed`, 1 `if`.

Jugement : D839 garde les crochets moteur ; la seule règle est le placement nomade (119 l.), net ESTIMÉ -119 + 70 = -49 pour un mode nomade encore provisoire (cvars « provisional », D797). Ne vaut pas un build.

## 2. `HOLGameState.cpp` (1 823 l., 99 fonctions) : plafond brut 1 485 l. ; vraie décision ~120 l. (LU / ESTIMÉ mêlés)

Corps lus : `HandleForumFall`, `HandleKingDeath`, `AdvanceForums`, `SkipForumPhase`, `SetForumPresenceOverride`, `DefeatSlot`, `EvaluateMatchEnd`, `ApplyPendingDefeats`, `CollapseSlot`, `GiveSlotStock`, `DefeatExpiredAbsences`, `RequestPause`, `ApplyStance`, `ApplyVision`, `ApplyDiplomacyOrders`. Non lus : `InitVictory`, `RequestResume`, les getters de présence, les 23 fonctions d'absence autres que celles ci-dessus.

| Famille | Plafond (bilan) | Décision encore prise en C++ | Verdict script appliqué / application d'état | Répl. / plomberie | Détail lu |
|---|---|---|---|---|---|
| Forum | 425 | ~35 (LU ~30, ESTIMÉ 5) | ~190 | ~200 | `AdvanceForums` 99 l. : les phases sont décidées par `HOLForumRules::Step` (fichier de règle, aussi appelé par la vision, par pas) ; ici ~12 l. de décision (`Lives <= 0` -> défaite en attente, `Standing` -> `RestoreHealth`), le reste est journal (~25), `SetForumState` / `ForceNetUpdate` (~15), évènements. `HandleForumFall` 44 : porte `AskFallGate` (script), `HOLForumRules::Fall`, remboursement de file, `Lives <= 0` : ~6 l. de décision. `HandleKingDeath` 25 : 0 décision (porte script). `SkipForumPhase` 29 : ~3 (outil de dev). `SetForumPresenceOverride` 30 : ~12 (choix d'un camp attaquant, outil de dev) |
| Fin de partie | 243 | ~21 (LU) | ~170 | ~50 | `EvaluateMatchEnd` 58 : ~15 l. de préparation (équipes ramenées à 0 en diplomatie, `HOLDiplomacy::AlliedVictory` calculé en C++) puis `AskMatchEnd` rend le verdict ; `DefeatSlot` 49 : porte `AskDefeatGate`, puis application (SlotDefeats, IA, stats, évènement, `CollapseSlot`) : 0 décision ; `CollapseSlot` 39 : application (tue unités, détruit bâtiments, solde à zéro) ; `ApplyPendingDefeats` 23 : ~6 (forum avant roi) ; `InitVictory` 39 non lue |
| Absence / reconnexion | 498 | ~55 (S4, cité : `Docs\Rapports\S4-PLAN-1119.md`) | ~250 | ~190 | S4 ABANDONNÉE : ~55 l. de décision sur ~480, net C++ positif. Relu : `GiveSlotStock` 58 (`GetStockOnTaking` en règle ; ~8 l. de branche robot / reprise), `DefeatExpiredAbsences` 36 (~8 l. : fin de fenêtre « l'IA tient » ou défaite). Concordant |
| Diplomatie | 164 | ~5 (LU) | ~100 | ~60 | `ApplyStance` 37 : porte `AskStanceGate`, règle `HOLDiplomacy::SetStance` (fichier à part), puis annonce ; les seules décisions sont les REPLIS C++ quand la planche manque (`Gate >= 0 ? ... : (!HasAuthority() ...)`, ligne 1683, et `EvaluateEnd >= 0 ? ... : Change.bFormed`, ligne 1706) : ~3 l. ; `ApplyVision` : ~2 l. de repli ; `ApplyDiplomacyOrders` 22 : répartiteur |
| Pause | 116 | ~2 (LU) | ~80 | ~35 | `RequestPause` 32 : `AskCanPause` + `AskPausesLeftAfterPause` rendent tout ; ici écriture de `PausedBySlot`, `PausesLeft`, `ForceNetUpdate` ; 1 entrée calculée (`NM_Standalone`). `RequestResume` non lue |
| Marché | 39 | 0 | | 39 | stockage |
| Réplication (`OnRep`, `GetLifetime...`) | 155 | 0 | | 155 | reste (D839) |
| Total | 1 640 (recoupements) | ~118 | ~790 | ~730 | décision ~7 % du plafond de 1 485 |

Réponse nette : sur les 1 485 l. du plafond, ~120 l. sont de la décision (dont ~55 pour l'absence déjà jugée non rentable). Le reste applique un verdict de planche déjà rendu (`Ask*`) ou écrit des champs répliqués. Les seules décisions simples à retirer sans liaison sont les ~5 l. de repli de la diplomatie (une suppression, mais qui change le comportement « planche absente » : refus au lieu de repli).

## 3. Les 51 `Request<X>` de `HOLPlayerController.cpp` : relais, pas de règle

Mesure (LU) : 51 fonctions, 524 l. au total (plage), médiane ~10 l., maximum 17 (`RequestPanelTribute`, `RequestCastRage`), 14 fonctions de 12 l. ou plus.

Lues (7) : `RequestProduction`, `RequestAttackMove`, `RequestDropOffUnits`, `RequestCheatUnitSpeed`, `RequestShareVision`, `RequestPanelTribute` (+ début de `RequestReturnHero`). Constat : chacune est un relais client -> RPC du type
`if (!HOLRpcScriptRoute_Haldir1117n::RouteX(this, ...)) { LogNotSent(...); }`, avec 1 à 2 gardes côté client (`IsSpectator()`, `UnitIds.Num() > 0`, `Building != null`). Le RPC est dans l'acteur RPC du script (D845). La seule logique est `RequestPanelTribute` : une boucle de 6 l. qui retrouve le Marché fini du joueur pour y attacher l'ordre (aide côté client, sans autorité).

Jugement : zéro décision d'autorité. Corrige le bilan 1119 qui comptait les `Request<X>` comme « règle de jeu encore en C++ » : ce sont la liaison elle-même (appelés par l'entrée, la grille, les widgets). Les migrer ne gagne rien (le script ne peut pas appeler un relais vers lui-même sans le reposer en C++).

`Do<X>` hors triche (29 fonctions, 437 l. LU) : 3 lues (`DoPostChat` 51, `DoMoveUnits`, `DoSetRallyPoint` 22) : elles identifient le siège de l'appelant et passent à la règle (`HOLPlayerOrders::Move`, file de production) ou à une planche déjà script (porte du chat). Les plus épaisses : `DoPostChat` 51, `DoPostSignal` 42, `DoHealUnit` 42 (135 l., ESTIMÉ ~40 de décision). Le reste ~300 l. : quelques l. de décision chacune (ESTIMÉ ~30 au total). Non lues : les 26 autres.

## 4. VERDICT

| Famille | Décision réellement migrable (LU) | Liaison à écrire (ESTIMÉ) | Net | Risque joueur | Avis |
|---|---|---|---|---|---|
| `HOLSelectionMarkers.cpp` | 0 | 0 | 0 | visuel | ne vaut pas |
| `HOLHeroSubsystem.cpp` | ~35 (portes retour / achat / cri) | ~40 | -0 à +5 | réseau (fiches, ordre dans le pas) | ne vaut pas |
| `HOLGameModeBase.cpp` (placement nomade) | 119 | ~70 | -49 | réseau / partie (mode nomade) | ne vaut pas |
| GameState : forum | ~35 | ~25 | -10 | réseau | ne vaut pas |
| GameState : fin de partie | ~21 | ~15 | -6 | réseau | ne vaut pas |
| GameState : absence (S4) | ~55 | > 55 | positif | réseau | ne vaut pas (S4 abandonnée) |
| GameState : diplomatie (repli) | ~5 | 0 | -5 | réseau (change « planche absente ») | à la rigueur, avec un build déjà ouvert |
| GameState : pause | ~2 | 0 | -2 | réseau | ne vaut pas |
| `Request<X>` (51) | 0 | 0 | 0 | réseau | ne vaut pas (déjà des relais) |
| `Do<X>` de jeu (29) | ~70 (ESTIMÉ) | ~70 | ~0 | réseau | ne vaut pas |
| Risque nul, non relus aujourd'hui (repris du bilan 1119 / plans) : `DoCheat*` ~265 brut, `RefreshGrid` -140 net, `Select.*` -60 net, caméra / placement -90 net | ESTIMÉ | ESTIMÉ | ~-455 (plans, non vérifiés) | nul à visuel | seuls candidats raisonnables, mais dev ou présentation |

Total : si l'on faisait tout ce qui « vaut la peine » parmi ce que j'ai lu, il partirait ~0 à 60 lignes NETTES de C++ (sur 119 510, < 0,05 %) ; en y ajoutant les familles à risque nul des plans non relues aujourd'hui (ESTIMÉ), ~450 à 500 lignes nettes au plus (~0,4 %). L'étape 5 est finie pour ce qui décide : ce qui reste est de l'application de verdicts, de la réplication, de la plomberie moteur ou du rendu.

## 5. Ce que je n'ai pas lu

- Corps non lus : `InitVictory`, `RequestResume`, getters de présence du forum, 23 des 29 `Do<X>` hors triche et les 14 `DoCheat*`, 44 des 51 `Request<X>` (jugés par 7 lus), `PlantBanner` en entier et les 11 petites fonctions de capacités / XP du héros, `UpdateDuelRings` / `UpdateRelayRings` / `UpdateGhost` de `HOLSelectionMarkers` (jugés par leurs voisins et leurs appels), `SpawnSlotStart` après la branche nomade.
- Les règles appelées (`HOLForumRules`, `HOLDiplomacy`, `FHOLStartRules`, `FHOLDisconnectRules`, `HOLHeroAbility`) ne sont pas relues : ce sont des fichiers à part, comptés au § 3c du bilan 1119.
- Les nombres « ~ » de décision sont ceux des lignes de gardes que j'ai lues ; pour les fonctions non lues, ESTIMÉ.
- Aucun appelant recompté, aucun test exécuté, aucun binaire vérifié.
