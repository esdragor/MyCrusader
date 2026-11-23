# VERDICT-1123pa (lecture seule, 2026-11-23, main a984c61)

Impact joueur : (1) la palissade tracee en ligne pose les fondations mais aucun Paysan ne part les construire ; (2) la Tour de guet en bois garnie tire 1 fleche par volee, comme videe (aucune regle tranchee ne dit le contraire).

## BUG 1 : wall_line_long, pas de construction au relache

VERDICT : BUG CONFIRME (l'ordre de construction n'existe pas sur le chemin de la ligne). Ce n'est pas un oubli de scenario : c'est ecrit dans le code.

(a) Batiment SEUL
- Client : `Source\HeroesOfLiberty\Player\HOLPlayerController.cpp:3392` `RequestPlaceBuilding(...)` ; `:3401-3416` : `Builders = GetCommandUnits()` (Paysans selectionnes, `:3406`), log "Placement: N units sent to build it" (`:3410`), puis `RoutePlaceBuilding(this, TypeName, Point, Builders, bQueue)` (`:3414`, route C++ `:4323` `CallFull(... "ServerPlaceBuilding" ... &Ids)`).
- Serveur : `Script\Net\HOLRpcPlayer.as:1500-1512` `ServerPlaceBuilding(TypeName, Point, TArray<uint32> UnitIds, bQueue)` -> `HOLRpcBridge::OrderPlaceBuilding(this, TypeName, X, Y, Z, HOLRpc::OrderIds(UnitIds), bQueue)` (`:1511`). Les ids des Paysans voyagent avec la pose : c'est la que l'ordre de construire est emis.

(b) LIGNE
- Client : `HOLPlayerController.cpp:3457` `EndWallDrag()` -> `:3487` `RequestPlaceWallLine(PlacementType, Start, Ground, bKeep)` ; `:3495-3502` : n'appelle que `NativeManualOrder(GetCommandUnits())` (fin du Follow, `:3499`) puis `RoutePlaceWallLine(this, TypeName, Start, End, bQueue)` (`:3500`, route `:4326-4329`) : AUCUN id de Paysan envoye. Le RPC `ServerPlaceWallLine(FName, Start, End, bool bQueue)` n'a pas de parametre `UnitIds`.
- Serveur : `HOLRpcPlayer.as:1518-1546`. Le commentaire `:1515` le dit : "NO builder is sent". `:1540` `TArray<int32> NoBuilders;` puis `:1544` `HOLRpcBridge::OrderPlaceBuilding(this, TypeName, Point.X, Point.Y, Point.Z, NoBuilders, bQueue)` pour chaque cellule : fondations posees, 0 constructeur.
- Le journal `Placement: N units sent to build it` n'existe pas sur le chemin de la ligne (EndWallDrag ne l'ecrit pas) : confirmation par le code.

(c) Ou ajouter l'ordre : C++ ET script (petit).
- Il faut que le client envoie ses Paysans : (1) C++ `HOLPlayerController.cpp:3495-3500` (`RequestPlaceWallLine` : lire `GetCommandUnits()`, le passer a la route) et la route `:4326-4329` (ajouter `&Ids` comme `:4323`) + `OnOrderGiven.Broadcast` comme `:3412` ; (2) script `HOLRpcPlayer.as:1518` ajouter `TArray<uint32> UnitIds`, et `:1540-1545` remplacer `NoBuilders` par `HOLRpc::OrderIds(UnitIds)` (+ refus pur si 0 ids ? non : une ligne sans Paysan reste permise, comme un batiment seul sans selection, a verifier dans `HOLRpc::PlaceBuildingRefusal`). Le script seul ne suffit pas : le serveur ne connait pas la selection du client.
- Le C++ est justifie : signature du RPC repliquee (reseau) = exception permise du CLAUDE.md.

(d) Regle voulue pour N fondations et M Paysans
- GDD:3348 (decision de pose F-Fortifications, point (3)) : "Pose par tracé en ligne, au glisser, sur la grille de 4 m ... ; coût payé segment par segment, plusieurs Paysans en parallèle." -> c'est la seule phrase trouvee ; elle ne dit pas comment N fondations sont reparties sur M Paysans (file, partage, ordre). Numero de decision non releve (ligne 3348 du GDD).
- Batiment seul : `bQueue` = Shift (`:3391` "Shift: next one", `:3410` ", queued") ; l'ordre de repartition dans `OrderPlaceBuilding` n'a pas ete lu. Je ne tranche pas : question a poser (Q: tous les Paysans sur la 1re fondation puis file, ou repartition 1 Paysan par fondation ?). Proposition de depart, non tranchee : meme `UnitIds` pour chaque cellule, 1re cellule non queue, suivantes queue (file Shift), pour que la bande enchaine dans l'ordre du trace.

LIGNE DE FILE :
- [ ] FIX wall_line_long (construction au relache) : C++ `HOLPlayerController.cpp:3495-3502` + route `:4326-4329` (envoyer `GetCommandUnits()` + `OnOrderGiven`) ; script `Script\Net\HOLRpcPlayer.as:1518-1545` (param `TArray<uint32> UnitIds`, remplacer `NoBuilders`) ; C++ pour la signature RPC repliquee (reseau), script pour le serveur. Test rouge `HOLWallLine_BuildersGetOrder` (script, `Script\Tests\HOLWallLineRules_Test.as` ou `HOLConstruction_Test.as`) : "ServerPlaceWallLine avec 2 Paysans et une ligne de 3 cellules : 3 fondations ET les 2 Paysans ont un ordre de construction (sur la 1re fondation, les autres en file)". Regle N/M a trancher avant (GDD:3348 ne la dit pas).
- Fichiers touches : `Source\HeroesOfLiberty\Player\HOLPlayerController.cpp` (+ `.h` si la signature de `RequestPlaceWallLine` change), `Script\Net\HOLRpcPlayer.as`, un test script. PAS HOLCombatProcessor.cpp.

## BUG 2 : ai_garrison, tour de guet en bois : 1 fleche au lieu de 1 + occupants

VERDICT : REGLE PROPOSEE MAIS NON TRANCHEE (donc le code est conforme a ce qui est decide) ; ce que l'utilisateur attend ("(1 + occupants) fleches") est exactement la proposition de D884, jamais confirmee. Pas un bug de code au sens strict : un point de regle ouvert + un branchement a faire quand il sera tranche.

(a) Regle ecrite
- GDD.md:3726 (ligne), **D884** (Tour de guet en bois TowerWood, tir, complete D878 ; 2026-11-22, reponse a `Q-towerwood-tir`) : "par flèche, **5 dégâts, cadence 1,625 s, portée 3000** ; la tour **tire même vide**. ... Le nombre de flèches par occupant (proposé : 1 + 1 par occupant) reste **à confirmer à part**." Fiche : `Docs\Archive\Retours_2026-11-22\Q-towerwood-tir.md`.
- GDD.md:3396 : la technologie "Levée de la garnison" = "plus de flèches par occupant (départ +50 %, arrondi) pour le forum, les centres secondaires et les Bastions seulement (écartés : toutes les tours, cadence)" : ne concerne PAS la tour de guet en bois.
- D878 (texte non relu dans cette passe : non etabli).

(b) Code
- `Source\HeroesOfLiberty\Simulation\HOLCombatProcessor.cpp:840-845` : `GetBuildingArrowCount(const int32 Occupants) { (void)Occupants; // D884 : nombre par occupant à confirmer, 1 tant que non tranché. return 1; }`. Le nombre d'occupants est calcule (`:870-883` `CountOccupants` via `GetGarrisonUnitIds`), passe (`:946-951`), puis ignore. Boucle de tir `:951-952` `ArrowCount = Max(1, GetBuildingArrowCount(Occupants))` / `for Shot < ArrowCount` : prete a recevoir un compte > 1 ; `RecordArrowShot(..., ArrowCount)` `:981`.
- Le point de branchement est donc UNIQUE : `:840-845`.
- `Tools\Data\BuildingTypes.csv:17` TowerWood : GarrisonSlots 3, BuildingAttack 5, BuildingAttackInterval 1.625, BuildingAttackRange 3000, BuildingFiresEmpty True (colonnes d'apres l'en-tete `:1`). Aucune colonne "fleches par occupant".

(c) Tests
- `Source\HeroesOfLiberty\Tests\HOLBuildingShotTests.cpp:431-433` : commentaire explicite "The number of arrows per shot is ONE in every case (GetBuildingArrowCount, D884 ...): the test pins no other number." Le test F8 T6 couvre des occupants (`Options.Occupants` `:87`, spawn `:237-255`) mais seulement pour FiresEmpty true/false et le depart des occupants ; AUCUN test n'affirme le nombre de fleches avec occupants.

(d) Scenario `Tools\Scenarios\ai_garrison.txt`
- Tour a (2800, 2600), 8 Hommes d'armes lances en attaque-deplacement vers elle (`AttackMoveNear 2800 2600`), 4 Archers IA apparaissent pres de la tour, 3 entrent (3 places, scenario en-tete `:7`). Portee de tir 3000 (30 m, CSV) : les Hommes d'armes sont des cibles a portee des qu'ils approchent ; la tour tire meme vide (D884, FiresEmpty True). La fonction `HOLCombat::IsInBuildingShotRange` est appelee `:922` (`Step.Box`, rayon de la cible) ; une portee MINIMALE n'est pas visible dans ce qui a ete lu (non etabli : corps de la fonction non lu). Le scenario ne mesure pas les fleches (il attend `Garrison: slot 1 puts`, pas un compteur de tirs) : preuve visuelle seulement.
- Comme la tour tire meme vide et que l'ennemi (Hommes d'armes) est a portee, elle a tres probablement tire des fleches ; l'utilisateur en a vu une a la fois (1 par volee, cadence 1,625 s), conforme au code actuel.

(e) Verdict : le code tire 1 fleche par volee quel que soit le nombre d'occupants ; c'est le comportement DOCUMENTE (D884 + test `:431-433`). La regle "1 + 1 par occupant" est seulement PROPOSEE. Ce n'est pas un bug a corriger sans decision : QUESTION a poser a l'utilisateur ("1 + 1 par occupant", confirmer), puis brancher le compte.

LIGNE DE FILE (apres confirmation de la regle) :
- [ ] D884 fleches par occupant : script impossible, c'est du C++ de simulation Mass (code par entite et par pas, test Mass reste en C++ : D838) : `HOLCombatProcessor.cpp:840-845` `GetBuildingArrowCount` -> `1 + Occupants` (si TowerWood seul : lire une colonne `ArrowsPerOccupant` du CSV, defaut 0 pour le forum/centres qui ont la tech D-Levee). Test rouge `HeroesOfLiberty.BuildingShot.ArrowsPerOccupant` dans `Source\HeroesOfLiberty\Tests\HOLBuildingShotTests.cpp` : "tour TowerWood avec 3 occupants : la 1re volee lache 4 fleches (RecordArrowShot ArrowCount == 4), 0 occupant : 1 fleche, et les fleches partent toutes au meme pas" (+ remplacer le commentaire `:431-433`).
- Fichiers touches : `Source\HeroesOfLiberty\Simulation\HOLCombatProcessor.cpp` (CONFLIT possible avec la branche zone-effect de Faramir : toucher seulement les lignes 840-845), `Source\HeroesOfLiberty\Tests\HOLBuildingShotTests.cpp`, `GDD.md` (D884 une fois confirmee), eventuellement `Tools\Data\BuildingTypes.csv`.

## Non etabli
- Texte de D878 ; numero de la decision de GDD:3348 (pose en ligne) ; comment `HOLRpcBridge::OrderPlaceBuilding` repartit les Paysans quand `bQueue` est vrai/faux (corps non lu) ; corps de `HOLCombat::IsInBuildingShotRange` (portee minimale) ; `HOLRpc::PlaceBuildingRefusal` pour 0 constructeur ; si la tour a tire dans la session de l'utilisateur (aucun journal lu).
