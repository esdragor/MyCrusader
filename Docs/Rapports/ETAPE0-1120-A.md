# ETAPE0-1120-A : D861, D860, lot C++ du socle des factions (lecture seule, main 901e391, 2026-11-20 ~09:00)

Joueur : D861 = un engin de siège obéit au premier clic (monte puis tire, démonte puis marche). D860 = le signal pulse et Alt + clic sur la mini-carte le pose. Lot : les pouvoirs de faction deviennent jouables.
Aucun fichier modifié, aucun build, aucun test. 22 appels d'outil vus par moi. Les causes de D861 sont des HYPOTHESES lues dans le code, non rejouées.

## 1. D861 (BUG, siège)

### Déjà fait dans git (sur main)
Le montage existe : `HOLDeployRules.h/.cpp` (G5, `Decide/Toggle/Apply/Advance`), `HOLDeployProcessor.h/.cpp` (G6 a, cvar `hol.Siege.Deployable`), combat qui laisse la note d'attaque (G6 b/c : `HOLCombatProcessor.cpp:2480-2483` bâtiment, `:3012-3015` unité, ordre `PendingOrder = 1`), bouton D (G6 d, `HOLUnitOrderProcessor.cpp:1508-1566`, `PendingOrder = 3`), phase au fil (G6 e, 58 bits), scénario `deploy` (G6 f). Tests verts connus : `HeroesOfLiberty.Siege.Deploy.Processor / InMatch / PlayerButton / PressWhileWalking / DeployedAutoFire`. Aucun commit n'adresse D861 (git log des 2 fichiers : derniers = terrain 046d758, autofire 73adb69).

### Code réel et causes probables
- (b) engin MONTÉ + déplacement : `HOLUnitOrderProcessor` applique le déplacement tout de suite (`MoveOrder.Phase` posée) ; `HOLDeployProcessor.cpp:80-84` note `PendingOrder = 2` ; `Decide(Move, Deployed)` -> `Pack` (`HOLDeployRules.cpp:105-115`). Pendant `Packing` (5 s) `HOLDeployProcessor.cpp:86-90` met la vitesse à 0 : la marche ne progresse plus, `StuckSteps` monte et `HOLUnitOrderProcessor.cpp:4012-4029` clôt l'ordre (`Order.Phase = None`, l'engin « attend un nouvel ordre »). Hypothèse forte, à prouver par le test. Le même mécanisme explique que `PressWhileWalking` (G6 f) ait dû annuler la marche à la main (`:1522-1550`).
- (a) engin DÉMONTÉ + attaque à portée : la note d'attaque n'est écrite que par le combat une fois « à portée de combat » (`HOLCombatProcessor.cpp:3007-3015`), avec la distance 2D ; `Decide(Attack, Packed)` rend `March` (aucun effet : ni montage, ni marche) si distance > `MaxRangeCm` (5000 cm, `DefaultProfile`) et `Refuse/TooClose` si < 1200 cm. Si la portée de combat de l'engin (Mangonnelle de substitution) dépasse 5000 cm, l'engin est « à portée » pour le combat (il arrête sa marche, `StopWalk`) mais `March` pour le montage : il ne fait rien d'autre que s'arrêter. Piste 2 : la cible est une unité qui bouge (distance changeante). Notes de G6 : « Maison à 5250 cm > portée max 5000 -> l'attaque auto devient March » (Palier3InProgress, entrée 2026-11-12/13), même famille.
- Le script `Script\Commands\HOLPlayerOrders.as` ne décide que l'acceptation et l'entrée dans la file (`ToggleDeploy` `:75-81`) ; il ne touche pas au montage. Pas de changement script attendu.

### Fichiers à toucher et langage
C++ (Mass : processeur par entité et par pas, D839) :
1. `Source\HeroesOfLiberty\Simulation\HOLDeployProcessor.h/.cpp` : l'ordre gardé. Ajouter au fragment un ordre différé (`bMoveKept` + point, ou nature de l'ordre), lu quand la phase passe à `Packed` (b) ; pour (a) : rendre `March` impossible quand le combat se dit à portée (recaler `Decide` sur la portée de combat réelle, ou monter dès que le combat se dit à portée et que la distance >= `MinRangeCm`).
2. `Source\HeroesOfLiberty\Commands\HOLUnitOrderProcessor.cpp` (branche Move/Attack avant écriture de `MoveOrder`, près de `:1508-1566`) : pour une unité portant le fragment en phase `Deploying/Deployed/Packing`, garder l'ordre dans le fragment au lieu de l'appliquer (et ne pas laisser `StuckSteps` le clore).
3. `Source\HeroesOfLiberty\Siege\HOLDeployRules.h/.cpp` seulement si on change `Decide` (même jeu de cas à recopier dans `HOLDeployRules_Test.as` : jumeau déjà existant, D836 impose UNE source : un changement de `Decide` impose de dire lequel des deux survit).
4. `Source\HeroesOfLiberty\Tests\HOLDeployButtonTests.cpp` (ou nouveau `HOLDeployKeptOrderTests.cpp`) : tests (simulation Mass : C++, D838).

### Premier test rouge proposé
- `HeroesOfLiberty.Siege.Deploy.MoveWhileDeployedKeepsOrder` dans `HOLDeployButtonTests.cpp` (modèle `PressWhileWalking` `:209-310`) : Mangonnelle `hol.Siege.Deployable 1`, montée (ToggleDeploy, `DeploySteps + 4` pas), puis `HOLPlayerOrders::Move(*World, 0, Mine, FVector(4000,0,0))`, `PackMs/StepMs + 4` pas ; assertion de l'EFFET : `Transform.X` a avancé d'au moins N cm (ou `MoveOrder.Phase != None` et vitesse de consigne > 0 une fois `Packed`), jamais seulement la phase. Pas d'ajout de mouvement dans ce harnais (« no movement runs here ») : asserter alors `MoveOrder.Phase != None` et `MoveTarget.DesiredSpeed > 0` à `Packed` + 2 pas. Rouge attendu : `Phase == None` après le démontage.
- `HeroesOfLiberty.Siege.Deploy.AttackWhilePackedDeploysThenFires` : engin emballé, cible (bâtiment mur à ~4000 cm, puis variante unité à 4000 cm), `HOLPlayerOrders::Attack/AttackBuilding`, assertion : phase `Deployed` après `DeployMs`, PV de la cible qui baissent. Variante `...AtTheCombatReachButBeyondDeployRange` : distance entre `MaxRangeCm` et portée de combat de la Mangonnelle de substitution (c'est ce cas qui doit se montrer rouge si l'hypothèse (a) est vraie). Premier geste : lancer cette variante seule pour trancher l'hypothèse avant de coder.
- Stub : champ `bMoveKept` présent (compile), non lu.

### Taille et découpe
> 15 min d'agent en un morceau (C++, build + 2 tests rouges + 2 correctifs). Découpe :
- D861-1 (agent C++, 15 min) : (b) test rouge + stub + ordre gardé dans le fragment + relance à `Packed`. 
- D861-2 (agent C++, 15 min) : (a) test des trois distances, diagnostic (journal `Deploy:` ligne `reason`), correctif de `Decide`/combat.
- D861-1 et -2 touchent les mêmes fichiers : séquentiels, ou UN agent pour les deux dans le lot.
- Rejeu par un AUTRE agent (règle 19) sur le build de main fusionné : scénario `deploy` à étendre (clic droit sur sol, puis clic droit sur Maison), `E-deploy` a été archivé le 2026-11-19.

### Conflits
`HOLUnitOrderProcessor.cpp` + `HOLDeployProcessor.cpp` : aucune autre tâche de ce fichier ne les touche, SAUF le morceau 3 du lot si l'on ajoute `IssueFactionPowerOrder` (alors dans `HOLCommandSubsystem.cpp`, pas dans l'order processor : pas de conflit). `HOLCombatProcessor.cpp` : morceau 4 et 10 le touchent ; D861 (a) peut s'en passer si on corrige `Decide`. Eviter (a) dans le combat tant que 4 y écrit.

## 2. D860 (REGLAGE, signal)

### Déjà fait dans git
H1 marque au sol (d4c6155, `Signals.GroundMark`, `HOLSelectionMarkers.cpp:882-920` `UpdateSignalMarks`, taille fixe par cvar `CVarSignalGroundMarkSize`). Carrés de la mini-carte (taille fixe 10 px, `HOLGameScreenScript.as:1690-1705`, SG3). Aucun commit H2/H3 (SignalsInProgress.md `:129-130`).

### H2 animation (sol + mini-carte)
- Une seule source (D836/D839) : le sol est en C++ (`UpdateSignalMarks`, instances par image) ; la mini-carte est en script (`RefreshMinimap`). Règle pure en C++ : `HOLSignalInput::PulseScale(AgeSeconds, ...)` dans `Player\HOLSignalInput.h/.cpp`, exposée au script par `HOLSignalInputReadLibrary.h/.cpp` (la bibliothèque existe déjà : `KindOfAltClick`), lue par `UpdateSignalMarks` (`Size * PulseScale(Now - Signal.ReceivedAt)`, `FHOLReceivedSignal::ReceivedAt` existe, `HOLSignalInput.h:27`) ET par `HOLGameScreenScript.as` (`SignalMark.Size = 10.0 * PulseScale(...)`, ligne ~1695). Forme de l'animation : balayage `signals_pulse_compare` (pulse / clignote / anneau qui s'étend), choix derrière un cvar `hol.Signal.MarkStyle` (D345).
- Test rouge : `Test_SignalPulseScale` dans un `Script\Tests\HOLSignalPulse_Test.as` neuf (script, D838) qui appelle `UHOLSignalInputReadLibrary::PulseScale` : assertions calculées par le code (échelle varie dans la durée de vie : `Scale(0) != Scale(T/4)`, bornes `[min,max]`, revient à 1 à l'expiration). Stub C++ qui compile : `PulseScale` rend toujours 1.0 -> rouge. Test d'EFFET : `HOLSignalGroundMarkTests.cpp` (existe) : deux lectures du transform d'instance à deux âges différents sont différentes (C++, il lit `AHOLSelectionMarkers`).
- Langage : C++ pour la règle + l'appel sol ; script pour l'appel mini-carte et les tests. Taille : 15 min un agent (règle + 2 appels + tests), sans scénario ; scénario de balayage : +1 agent (script/json, pas de C++).

### H3 Alt + clic sur la mini-carte
- Le press de la mini-carte arrive à `HandleMinimapPressed(Position, bRight, bDrag)` (`HOLGameScreenScript.as:1748-1772`), via `UHOLMinimapView::OnPressed` (`HOLMinimapView.cpp:260/272/680`) qui ne transporte pas Alt. Pas de C++ nécessaire : le script lit `PlayerController.IsInputKeyDown(EKeys::LeftAlt/RightAlt)` (UFUNCTION de `APlayerController`, à vérifier à la compilation du script) et poste via le même chemin que `hol.Signal.Post` : `HOLControllerCommandsC2.as:109-113, 197-198` (`Rpc.ServerPostSignal(Kind, FVector(x, y, z))`). Z = foyer de la caméra comme `OrderMove` (`HOLMinimapView.cpp:662`).
- Règle pure : étendre `HOLMinimap::ClickAction(bRight, bSelection)` en `ClickAction(bRight, bSelection, bAlt)` avec une nouvelle valeur `ClickSignal` (`Script\...\HOLMinimapRules.as`, test `HOLMinimapRules_Test.as:52-57`). Test rouge : `Test_MinimapClickActionAltPlacesSignal` (`ClickAction(false, true, true) == ClickSignal`, aussi sans sélection, gauche seulement ; droite+Alt reste un déplacement), stub : `ClickSignal` défini mais jamais rendu. Effet : un test script sur le GameScreen (modèle `ClickMinimapName`, `:1776-1790`) avec nom `MinimapClickAL120x80` -> `Minimap signals: 1` (log existant `:1703`) ou `GetNumReceivedSignals` +1.
- A traiter : un Alt + glisser sur la mini-carte (`bDrag`) ne doit pas poser de signal (`:1752` ignore déjà `bRight && bDrag`, pas `bDrag` gauche : la caméra suit).
- Langage : script seul (0 build). Taille : <= 15 min, un agent.

### Conflits D860
H2 sol (C++ : `HOLSelectionMarkers.cpp`, `HOLSignalInput.*`, `HOLSignalInputReadLibrary.*`) = lot C++. H2 mini-carte et H3 = `HOLGameScreenScript.as` (même fichier que le morceau 6 du lot, voir §3) et `HOLMinimapRules.as` : un seul agent à la fois sur `HOLGameScreenScript.as`. H2 et H3 sont donc séquentiels ou fusionnés dans un agent script.

## 3. Lot C++ du socle des factions (LOT-CPP-socle-factions-1119.md), état réel sur main 901e391

Main contient le lot de la veille (df895b0, merge integ-25) : morceaux 1 et 8 (ELROND-1119n4 = `FactionResource` dans `AHOLGameState`, commits 9a51321/1f7e15b ; la bibliothèque statique `HOLFactionResourceLibrary` a été SUPPRIMEE par 37ef75a : les accesseurs `GetFactionResource/SetFactionResource` du GameState sont déjà appelables par script ; `CheatSpecialization` présent dans `HOLScriptRpcPlayerActor` et `HOLPlayerController`, `HOLControllerCommandsC2.as`, test de liste à 12 noms dcdc596).

Vérifié par grep sur `Source` et `Script` (absent = aucune occurrence) :
| n° | état | constat | C++ ? |
|---|---|---|---|
| 1 | FAIT sauf `PowerCooldownEndMs` | `FactionResource` fait ; `PowerCooldownEndMs` jamais ajouté (ELROND-1119n4 : « PowerCooldownEndMs non fait ») ; pas de bibliothèque statique | un champ réplicat. si on le veut en C++ ; sinon l'état de recharge peut vivre dans le tableau de règles script (à trancher) |
| 2 | ABSENT | aucun `CheatHonor`, aucune commande `hol.Cheat.Honor` | probablement AUCUN C++ : le RPC `ServerCheatHonor` (script, modèle `ServerCheatTier`, `HOLRpcPlayer.as:1014`) peut appeler `GameState.SetFactionResource` (UFUNCTION) sur le serveur ; à vérifier : la garde de triche `IsCheatRefused` est-elle lisible du script ? (`HOLRpc::CheatTierRefusal` est script, d'après le plan). Si oui : script seul, 1 test (modèle `HOLTierCheat_Test.as`) |
| 3 | ABSENT | `FactionPower` n'existe que dans `HOLFactionSlot.as` (vue) ; pas de RPC, pas de `IssueFactionPowerOrder` | RPC script ; C++ seulement si l'appelant est le contrôleur C++ (souris au sol) : sinon bouton script -> RPC -> serveur script -> `HOLZoneEffect...Start` (morceau 4). A lire : où se fait le clic de visée au sol |
| 4 | ABSENT | aucun `ZoneEffect` | C++ (processeur Mass : ralentissement par entité et par pas) : `HOLZoneEffectRules.h`, champ d'unité à côté de `CloseRanksSpeedPercent` (`HOLSimulationFragments.h`), `HOLCombatProcessor.cpp`, empreinte `HOLSimulationSubsystem.cpp:153-155` ; valeurs de D859 (rayon 6 m, durée 10 s, portée 8 m, recharge 40 s, -40 %) |
| 5 | A MOITIE | `UnitAbilities.csv` a déjà `Target,RadiusCm,RangeCm` (b524196/582fadd), générateur corrigé (a0d7c5d) ; ligne `LightSeal` ABSENTE (grep vide) | script/données, 0 build ; valeurs D859 connues : ligne à écrire |
| 6 | ABSENT | `HOLFactionSlot.as` existe (vue, tests) mais n'est appelé par aucune ligne de `HOLGameScreenScript.as` ; `GetAboveHeroBlockPlace` / hôte d'emplacement absents ; `GetBottomLeftPlace` existe (`HOLScriptScreen.cpp:764`) | peut-être script seul (calculer au-dessus du bloc du héros depuis `GetBottomLeftPlace` et l'échelle) ; sinon un accesseur C++ trivial |
| 7 | ABSENT | aucun `SetSeatFaction` ; la faction du siège se lit déjà par `HOLFactionRules::GetSlotFactionKey(World, Slot)` (`HOLFactionRules.h:119`, namespace C++ non UFUNCTION : pont script à vérifier) ; nomades = `NomadSeats` réplicat | à trancher : lecture via pont existant (script seul) ou réplication d'un masque de factions (C++) |
| 8 | FAIT (dans main) | voir ci-dessus | - |
| 9 | ABSENT | aucun `*Moral*` en simulation (seulement `HOLHeroAbility*`) ; le moral M3-M5 n'existe pas | hors lot (gros) |
| 10 | ABSENT | Défi du Chevalier : aucune trace C++ ; valeurs maintenant connues (D859 : 8 s, rayon 8 m, recharge 45 s) mais « cumul de deux Chevaliers » à trancher | C++ (`HOLCombatProcessor.cpp:638-730`), hors du premier build |
| 11 | ABSENT | `Tools\Data\UnitTypes.csv` ne contient aucune ligne `Halberdier` (grep = 0) ; valeurs de D858 connues (75 nourriture + 30 bois, 18 s, 100 PV, vitesse 740, attaque 8, cadence 1,875 s, portée 130, +17 cavalerie, +8 `HeavyInfantry`, remplace le Lancier à l'Aube) | pas de C++ ; ligne CSV + régénération de `Content\Units\UnitTypes.uasset` par l'éditeur (règle 13 : seul, file de build vide) ; `HOLUnitTierRules_Test.as:142-181` (« 16 rows ») à mettre à jour ; IA de l'Aube : `HOLAIArmy.as` |

Questions Q1/Q2 : répondues (D858, D859), leurs fiches sont archivées (`Docs\Archive\Retours_2026-11-19\`), plus rien ne « bloque » les valeurs. Seule question restante du plan : « la jauge entre-t-elle dans `HashUnits` ? » (le plan recommande non, tant qu'elle ne change pas d'unité), et le choix règle-en-C++ du morceau 4.

### Ordre proposé pour ce jour, UN build
Contenu C++ réel du lot restant (peut-être 4 morceaux, pas 8 comme le plan le disait) :
- Lot C++ du jour (un build) : D861 (agent C++ A : `HOLDeployProcessor.*`, `HOLUnitOrderProcessor.cpp`, `HOLDeployButtonTests.cpp`) ; D860 H2 sol (agent C++ B : `HOLSignalInput.*`, `HOLSignalInputReadLibrary.*`, `HOLSelectionMarkers.cpp`, `HOLSignalGroundMarkTests.cpp`) ; morceau 4 (agent C++ C : `HOLZoneEffectRules.*`, `HOLSimulationFragments.h`, `HOLCombatProcessor.cpp`, `HOLSimulationSubsystem.cpp`, tests Mass) ; éventuellement morceau 1 `PowerCooldownEndMs` + 3 si le clic de visée est C++ (agent à ne lancer qu'après lecture : `HOLPlayerController.*`, `HOLScriptRpcPlayerActor.*`). A, B, C ne partagent aucun fichier : 3 agents C++ en parallèle (plafond de la règle 10 = 3). D861 (a) ne doit pas toucher `HOLCombatProcessor.cpp` tant que C y écrit.
- Hors build (script, en parallèle) : morceau 2 (script seul si la garde de triche est lisible), 5 (CSV + ligne `LightSeal`, 0 build), 6 + D860 H2 mini-carte + H3 (tous dans `HOLGameScreenScript.as` : UN agent à la suite, pas deux), 7 (script si pont existant), 11 (CSV + uasset, après le build et seul).
- Conflits de fichiers à retenir : `HOLGameScreenScript.as` = morceau 6 + H2 mini-carte + H3 ; `HOLCombatProcessor.cpp` + `HOLSimulationFragments.h` = morceau 4 + 10 (+ D861 (a) si on y touche) ; `HOLPlayerController.*` + `HOLScriptRpcPlayerActor.*` + `HOLRpcPlayer.as` = morceaux 2, 3 (8 déjà fait) ; `HOLUnitOrderProcessor.cpp` = D861 seul.
- Qui tient dans UN build : D861, D860 H2 sol, 4, (1 PowerCooldown), (3). Pas dans ce build : 9, 10 ; 11 est une régénération d'asset (pas un build).

## NON VERIFIE
- Les deux causes de D861 sont des hypothèses de lecture (aucun test lancé) ; en particulier (a) dépend de la portée de combat de la Mangonnelle de substitution (non lue) contre `MaxRangeCm` 5000.
- `IsInputKeyDown` appelable du script, et la garde de triche lisible du script (morceau 2) : non vérifiés.
- Lignes citées de `Palier3InProgress.md` (entrées du 2026-11-11 à 13) et de `LOT-CPP...` : relues dans ce tour sauf mention ; `HOLUnitOrderProcessor.cpp:4012-4029` lu comme « arrivée ou blocage », la raison exacte de la clôture d'ordre pendant `Packing` non prouvée.
- Morceaux 3, 6, 7 : le geste de visée au sol, l'hôte de place au-dessus du bloc héros et le pont `GetSlotFactionKey` vers le script n'ont pas été lus.
- Valeurs de `HOLDeploy::DefaultProfile` lues dans les notes de G5/G6 (8000/5000 ms, 1200/5000 cm), pas dans le fichier de ce tour.
