# REVUE-1122 : relecture d'integ-52 (edc631a) contre main

Lecture seule, `git diff main...integ-52 -- Source Script` (35 fichiers). Rien de compilé ni lancé : tout est « vérifié par lecture » sauf mention « supposé ».
Impact joueur en une ligne : rien de bloquant pour fusionner main en local ; deux défauts gênants (suivi qui écrase d'autres ordres et coûte cher par image, opacité de furtivité qui reste d'une partie à l'autre) et un risque réseau à confirmer.

## Bloquant fusion

Aucun trouvé. Le C++ est sans effet de bord sans classe script (valeurs neutres), les colonnes de BuildingTypes sont ajoutées en fin de ligne.

## Gênant

1. **Opacité de furtivité jamais remise à 1.0 d'une partie à l'autre** (vérifié).
   `Source/HeroesOfLiberty/Presentation/HOLUnitOpacity.cpp:9-13` : la table est un `static TMap` du processus. `ClearStealthOpacities` n'est appelée nulle part hors tests (git grep). `NextUnitId` repart à 1 par monde (`HOLUnitSubsystem.h:265`). `Script/Fog/HOLStealthOpacityApply.as` (`Last` est neuf à chaque monde) n'écrit que les CHANGEMENTS : une unité à 1.0 hors zone n'est jamais réécrite.
   Scénario : partie 1 sur Forest, l'unité #37 finit dans un bosquet (0.5) ; retour au menu sans quitter le jeu, partie 2 : l'unité #37 (alliée, ennemie ou neutre, la table ne connaît pas le propriétaire) est dessinée à moitié transparente tant que rien ne la remet à 1.0. Même chose sur une carte sans zone (`HOLStealthFilter.as` Tick sort dès `Zones.Num() == 0`, donc aucun nettoyage).
   Correctif : appeler `ClearStealthOpacities` à l'installation de `UHOLStealthInstaller` (avant le premier Tick installé) et à sa fin de monde.
   Même cause, effet mineur : une unité morte ou garnisonnée dans une zone garde son 0.5 dans la table et dans `Last` (les identifiants ne sont pas réutilisés dans un monde, donc inoffensif dans la partie).

2. **Suivi : un Move est émis à CHAQUE Tick et par suiveur, avec une lecture de toutes les unités par suiveur** (vérifié pour le mécanisme, coût supposé non mesuré).
   `HOLPlayerController.cpp` (Tick, fin) appelle `NativeStep` à chaque image ; `Script/Orders/HOLFollowBridge.as` `Step` boucle sur chaque suiveur ; `HOLFollowOrders.as` `Step` renvoie `HOLPlayerOrders::Move(...)` sans mémoire (seul l'Attack est dédoublonné par `LastAttackIds`) tant que le suiveur est à plus de 300 cm. Chaque Move passe par `UHOLCommandSubsystem::IssueMoveOrder` (projection navigable + annulation du soin, `HOLCommandSubsystem.cpp:194-208`).
   De plus `PickEnemyForUnit` -> `UHOLUnitScriptLibrary::GetEnemiesNear` (`HOLUnitScriptLibrary.cpp:34-62`) fait un `HOLUnitView::Gather` complet de toutes les unités du monde PAR suiveur et PAR image, et `GetAttackRange` fait un `LoadSynchronous` + `FindRow` par suiveur et par image.
   Scénario : 100 suiveurs, 3 000 unités, 60 images/s : 100 Gather de 3 000 vues et 100 Move par image. Le scénario `follow_crowd` (Tools/Scenarios) est là pour le mesurer, pas encore de chiffre lu.
   Correctif : cadence de 4 à 5 Hz comme le filtre de furtivité, ne renvoyer un Move que si la cible a bougé de plus de FollowDistance depuis le dernier Move, un seul Gather partagé par pas.

3. **Suivi : les ordres qui ne passent pas par les cinq `Request*` accrochés ne mettent pas fin au suivi** (vérifié par lecture, comportement supposé).
   `NativeManualOrder` n'est ajouté que dans RequestMoveUnits / AttackUnit / HealUnit / AttackMove / Stop / MoveInLine (+ le début de `OrderAt` pour le clic droit). Les autres routes (`RequestGatherUnits` l.3202, `RequestBuildUnits` l.3479, `RequestPlaceBuilding` avec bâtisseurs l.3403, `RequestDropOffUnits` l.3497, `RequestGarrisonUnits` l.3514, `RequestAttackBuilding` l.4815, ToggleDeploy l.3047, capacités de héros) n'y touchent pas ; `HOLFollowRules.as` annonce pourtant « patrol / hold / another follow » comme ordres manuels.
   Scénario : les unités suivent un allié, le joueur les envoie construire par le bouton ou la touche (pas le clic droit) : au Tick suivant le Move du suivi (non mis en file) remplace l'ordre de construction dès que l'allié s'éloigne de plus de 300 cm. Le clic droit direct sur une source ou un bâtiment est lui couvert par l'appel en tête de `OrderAt`.
   Correctif : appeler `NativeManualOrder` dans un point commun (par ex. à la porte du sous-système de commandes pour tout ordre de joueur hors suivi) plutôt que fonction par fonction.

4. **Suivi sur un client réseau : ordres posés dans le sous-système de commandes LOCAL, sans RPC** (supposé : l'exécution de la file côté client n'a pas été lue).
   `HOLFollowBridge.as` `Step` : `HOLCommandScript::GetCommands()` puis `HOLPlayerOrders::Move/Attack` (`HOLCommandScriptLibrary.cpp:64-66`, `139-146`) ; tous les autres ordres du joueur passent par `HOLRpcScriptRoute_Haldir1117n::Route*` (RPC vers le serveur). `NativeStep` est appelé pour tout contrôleur local, donc aussi un client.
   Scénario : un client pur ordonne à ses unités de suivre un allié : la liste du suivi est locale, les Move sont émis dans un monde client qui ne simule pas (« les unités affichées sur un client », `HOLUnitScriptLibrary.cpp:16`) : les unités ne bougent pas, ou le serveur et le client divergent. Solo et hôte d'une partie en écoute fonctionnent. À confirmer par une partie à deux machines.

5. **Soin : le clic droit sur une unité de la sélection elle-même, avec un Moine dans la sélection, devient un ordre de soin** (vérifié).
   `HOLPlayerController.cpp:~1497-1508` : `FindAllyAt(ViewportFraction, TConstArrayView<uint32>())` n'exclut pas la sélection (le commentaire de Follow le dit, pas celui du soin) et `bSkipOwn` est faux.
   Scénario : Moine + 5 soldats sélectionnés, clic droit sur le sol là où le corps d'un soldat sélectionné est dessiné (déplacement court) : `RequestHealUnit` sur ce soldat, les autres « marchent là » vers un point qui est un des leurs, au lieu du Move au point cliqué. Avant 1122e ce chemin n'existait pas (la cible venait de `FindTargetAt`, hostiles seulement). Même branche prioritaire sur : dépôt (`RequestDropOffUnits`) et garnison si une unité alliée est dessinée devant le bâtiment, et sur le Suivi pour toute sélection contenant un Moine (un coéquipier ne peut pas être suivi avec un Moine sélectionné).
   Correctif : passer la sélection en `Excluded` aussi pour le soin ; décider l'ordre de priorité Soin / Suivi.

## Mineur

6. `FindScriptClass` (`HOLFollowBridge.cpp:10-27`) : sans classe script (script non compilé, test), `Cached` reste vide et le `TObjectIterator<UClass>` complet est relancé à CHAQUE appel, donc à chaque Tick du contrôleur (`NativeStep`) et à chaque `Request*`. Neutre en jeu normal (classe trouvée, mise en cache). Correctif : mémoriser aussi l'échec (par ex. jusqu'au prochain hot reload).
7. Suivi et Maj : `OrderAt` ignore `bShift` pour le suivi (`NativeRightClickOnUnit` sans paramètre de file) : Maj + clic droit sur un allié démarre un suivi qui remplace la file, au lieu de s'ajouter comme le soin ou le déplacement.
8. `PickFollowAt` avec `hol.Follow.OwnUnits 0` (`HOLUnitPicking.cpp`) : une unité à soi dessinée devant un coéquipier est sautée et le coéquipier DERRIÈRE est pris ; le joueur voulait un déplacement. Intentionnel d'après le commentaire, mais surprenant.
9. Furtivité et suivi : `GetEnemiesNear` (`HOLUnitScriptLibrary.cpp:44-60`) ne filtre pas par vision : un garde du corps peut recevoir un Attack sur un ennemi masqué par la furtivité ou le brouillard à portée (+120 cm). Supposé : la porte de commande n'a pas été lue pour voir si elle refuse une cible non vue.
10. `HOLStealthFilter.as` : le commentaire de `HOLStealthOpacityApply.as` dit « 20 fois par seconde » alors que la cadence est 4 Hz (`OwnOpacityPeriodSeconds`) ; `HOLFollowOrders.as` (en-tête) dit toujours « Not wired to the game yet » et « NOT COMPILED » ; `HOLTopBar.as` dit « not wired by this commit » alors que `HOLGameScreenScript.as` l'appelle. Trompeur pour la relecture, sans effet.
11. `HOLGameScreenScript.as` (+10 lignes) appelle `TopBar.SetBannerRect` à chaque `Refresh` : 8 `GetWidgetBox` et 8 `SetPadding` par appel, même sans changement. Coût léger, à garder en tête si Refresh est par image (non vérifié).
12. Suivi mutuel A suit B et B suit A (ou boucle A, B, C) : pas de boucle infinie (chacun s'arrête à 300 cm), mais personne n'avance ensuite. Aucun effet de coût.

## Par sujet demandé

- (1) Clic droit : ordre d'attaque (hostile, avec marge) avant soin avant suivi avant bâtiment ennemi, dépôt, garnison, source, déplacement. Mort : `HOLUnitView::Gather/Find` excluent les unités mourantes par défaut, donc pas de cible morte. Brouillard et furtivité : `GatherPickCandidates` filtre par `Vision->IsUnitShown`, donc le client ne peut pas CLIQUER une unité non vue (l'autorité côté serveur d'un RPC forgé n'est pas dans ce diff). Garnison : une unité garnisonnée n'est pas dans `Gather`, donc pas cible. Défauts : 5, 7, 8. `ScriptRightClickAt` et la commande console : aucun défaut (même schéma que `ScriptLeftClick`).
- (2) Pont : `FollowBridge` est un `UPROPERTY(Transient)` du contrôleur, créé avec le contrôleur comme Outer : durée de vie correcte ; hot reload géré (classe `CLASS_NewerVersionExists` ignorée et pont remplacé). Un seul contrôleur local par monde en pratique ; liste du suivi dans un sous-système de monde, donc remise à zéro à chaque carte. Suiveur mort ou cible morte : `ReadUnit` faux, entrée retirée en marche arrière. Défauts : 2, 3, 4, 6.
- (3) Furtivité : `TakeOpacitySnapshot` copie la table une fois par exécution sur le fil de jeu (copie vide quand aucune zone) : correct. Identifiants : `UnitId` de simulation, converti en `int32` des deux côtés. Slot local : `GetFirstPlayerController()->PlayerState` ; sur un serveur dédié renvoie `INDEX_NONE` et la passe sort. Cadence 4 Hz et sortie immédiate sans zone : correct. Défaut : 1.
- (4) Morale : faction par emplacement lue une fois par pas (tableau de `MaxSlots`), partie sans configuration = `Dawn` (`HOLFactionRules.cpp:374-383`), emplacement hors tableau = Dawn : aucun défaut.
- (5) `HOLTopBar` / `SetBannerRect` : logique pure de décalage correcte (pas de bandeau = 0) ; la position gauche du tooltip est approchée par le centre de la cellule, dit dans le commentaire. Seul le point 11.
- (6) `FHOLBuildingTypeRow` : quatre colonnes `BuildingAttack*` ajoutées en fin de structure, « données seulement, lues par rien » (attendu) ; `BuildingTypes.uasset` et le CSV changent ensemble. Aucun lecteur dans integ-52 : confirmé.

## Règles de CLAUDE.md

- Une règle, une source : la règle « une unité de la sélection n'est pas cible de suivi » existe deux fois : `HOLFollowOrders.as` `ClassifyRightClick` (SelectedIds.Contains) et le C++ `PickAllyAt(... Excluded)` ; de même `bTargetIsAlly` est passé constant `true` par le C++ alors que le script le classe. Jumeau partiel, mineur. `PickAllyAt/PickFollowAt` (qui est suivable) est une règle écrite en C++ sans jumeau script : à justifier (clic, Slate) ou migrer.
- Tests neufs en C++ : `HOLAllyPickingTests`, `HOLFollowBridgeTests`, `HOLFollowOwnUnitsTests`, `HOLStealthOwnUnitsTests`, `HOLUnitOpacityDrawTests`, `HOLDeployButtonTests`, `HOLMoraleRulesTests`. Seul le dernier relève de D838 (simulation Mass) ; les autres testent du C++ neuf justifié (liaison, picking, dessin) : acceptable, à ranger sous « C++ par exception » dans le lot C++ de la séance.

## NON VÉRIFIÉ

Tout comportement d'exécution (aucun build, aucun lancement) ; le coût réel du suivi à 100 unités ; l'exécution de la file de commandes côté client réseau (point 4) ; la porte serveur contre un RPC forgé ; la lecture de l'indice 4 par le matériau `M_UnitPlayerColor` (asset binaire hors diff) ; la fréquence réelle de `Refresh` de l'écran de jeu ; `HOLUnitView::Find` et `IsUnitShown` lus seulement par leurs signatures et commentaires ; les fichiers de test `.as` et C++ (non lus en détail), HOLCheatPanel, HOLDeployButton, scénarios et outils PowerShell (hors périmètre demandé).
