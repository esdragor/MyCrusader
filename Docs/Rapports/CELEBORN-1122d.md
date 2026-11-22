# CELEBORN-1122d : plan du tir de bâtiment (F8, D884), LECTURE SEULE. NON COMPILÉ, NON LANCÉ

Joueur : la Tour de guet en bois tire seule sur les ennemis qu'il voit à 3000 cm de son bord (5 dégâts, 1,625 s), même vide.
Technique : un seul bloc dans `UHOLCombatProcessor::Execute` qui réutilise la liste `StepBuildings`, le tableau `Arrows` et le projectile existants.

APPELS: 12 (plafond 20). HEURE: 17:36 à 17:40. Code lu : `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn` (branche integ-41), sans le modifier.
Tout ce qui est écrit ci-dessous vient d'un Grep ou d'une lecture de lignes citées ; ce qui n'a pas été lu est marqué NON VÉRIFIÉ.
Chemins `S\` = `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Source\HeroesOfLiberty\`.

## 0. État des lieux (faits lus)
- **Aucun bâtiment ne tire aujourd'hui.** `BuildingAttack` (int), `BuildingAttackInterval`, `BuildingAttackRange`, `BuildingFiresEmpty` sont dans `FHOLBuildingTypeRow` (`S\Buildings\HOLBuildingTypes.h:174-186`) ; aucun lecteur (la seule autre occurrence du mot `BuildingAttack` est celle des UNITÉS : `Constants.BuildingAttack`, `HOLCombatProcessor.cpp:2341`, `:2545-2575`, un autre struct). Le grep des sources ne donne aucun lecteur des 4 champs de ligne de bâtiment.
- `Tools\Data\BuildingTypes.csv` de ce worktree : ligne `TowerWood` = Footprint 800x800, Height 800, MaxHealth 800, RangedArmor 40, **Mesh vide**, BuildGridSlot 7, Age 0, Category Military, Walkable False, Shape BlockAndTower, **SightRadius 4800**, **GarrisonSlots 3**, BuildingAttack 5, Interval 1.625, Range 3000, FiresEmpty True. Les 16 autres lignes : attaque 0 / 0 / 0 / False. Le `.uasset` de ce worktree est-il régénéré (6656131 est dans integ-50) : NON VÉRIFIÉ. Un commentaire `HOLGarrisonSubsystem.cpp:55` dit encore « GarrisonSlots 0 pour tous » : périmé ou non, NON VÉRIFIÉ.
- Le tir des archers vit dans `S\Simulation\HOLCombatProcessor.cpp` : relâche l.3085-3150 (`Arrows.AddDefaulted_GetRef`, `HOLCombat::GetArrowFlightMilliseconds/GetArrowFlightSteps`, `HOLCombatRules.h:766-772`), relâche contre un bâtiment l.2545-2600 (modèle du « tir vers une boîte »), atterrissage au début du pas l.1406-1530 (`Arrows[]` triés par ordre de relâche, `ArrivalStep`), struct `FArrow` `HOLCombatProcessor.h:117-137`, tableau `Arrows` l.139.
- Les bâtiments sont des **acteurs répliqués** (AHOLBuilding), pas des entités Mass : le processeur les collecte chaque pas dans `StepBuildings` (`HOLCombatProcessor.cpp:1372-1392`, exclut `IsFoundation()` et `GetHealth() <= 0`, `Step.Box = GetBodyBounds()`, `Step.OwnerPlayer`, `Step.Camp`).
- Garnison : une unité garnie porte `FHOLGarrisonedTag`, exclue de la requête du combat (`HOLCombatProcessor.cpp:271-272`), donc jamais cible ni tireur. Les occupants côté serveur : `UHOLUnitSubsystem::GetGarrisonUnitIds(const AHOLBuilding*, TArray<uint32>&)` (`S\Simulation\HOLUnitSubsystem.h:181-182`) ; côté client propriétaire seulement : `AHOLBuilding::GetGarrisonOccupants` (`S\Buildings\HOLBuilding.h:134-138`). Éjection à la destruction : `UHOLConstructionSubsystem::DestroyBuilding` l.645 appelle `EjectAll` l.709 (déjà là).
- Camps : `FHOLCampTable` (`S\Match\HOLVictoryRules.h:29-75`) : `AreHostile` (adversaire), `FiresAtOnce` (tire seul : en Diplomatie seulement l'état Enemy ; un Neutre ne reçoit pas de tir), `Relation` (propriétaire hors slots et camps neutres : Enemy). Le processeur a `StepCamps` (l.912) et `Step.Camp`. Les camps neutres : unités du propriétaire `FHOLMatchConfig::NeutralOwner`, atterrissage d'une flèche note le coup par `HOLCamps::NoteHit` seulement si `Shooter != INDEX_NONE` (l.1456-1458).
- Brouillard : `UHOLVisionSubsystem::IsVisible(Slot, Location)` (`S\Map\HOLVisionSubsystem.h:149`), utilisé dans le combat par le lambda `IsSeenBy` l.854-862 (Vision mise à nullptr si brouillard désactivé ou non démarré). Le bâtiment est un observateur : `SightRadius` 4800 > portée 3000, donc une cible à 3000 est vue par la tour elle-même (`HOLVisionSubsystem.cpp:276-281`).
- Affichage : `UHOLArrowSubsystem::Launch(FHOLArrowLaunch{From, To, Target, FlightMs, ShooterType})` (`S\Presentation\HOLArrowSubsystem.h:15-44`) ; `GetProjectileMesh(ShooterType)` cherche la ligne dans la table des UNITÉS, sinon **SM_Arrow** (l.62, `HOLArrowSubsystem.cpp:68-80`) : un nom de tour donne donc SM_Arrow sans rien faire. Le client déduit ses flèches de l'état des unités tireuses (`S\Net\HOLNetUnitDisplay.cpp:67-170`) : **une tour n'est pas une unité, aucune flèche de tour ne s'afficherait chez un client** ; les dégâts, eux, arrivent par la santé répliquée des unités.
- Modèles à imiter pour les tests : `S\Tests\HOLGarrisonOrderTests.cpp:696-770` (Shelter : table copiée `AddRow`, vrai `UHOLCombatProcessor`, `RunStep`, `Place` d'unités sur un identifiant multiple de 4, `Tower->Setup(type, owner, color)`), `S\Tests\HOLCombatTests.cpp` (Projectiles l.506, Cadence l.638, Damage l.298 ; compteurs `GetTotalArrowsReleased/Landed/Lost`, `HOLCombatProcessor.h:55-57`).

## 1. Règle générale de placement (D836 / D839, une seule source)
- **Tout le tir de bâtiment en C++, dans `UHOLCombatProcessor::Execute`** : le processeur seul possède `Fighters`, `Arrows`, `StepCamps`, `Vision` (locaux ou privés) ; un second processeur obligerait à exposer ces états (deuxième source). Raison dite (D839) : code appelé par pas qui lit les unités du pas, et projectile commun avec le combat.
- **Géométrie et dégât** = règles pures C++ inline dans `HOLCombatRules.h` (comme `IsInRange` l.110) : appelées par cible par pas.
- **Nombre de flèches par occupant** : pas de règle écrite ici. Tant que non confirmé : une fonction unique `HOLCombat::GetBuildingArrowCount(Occupants)` qui rend 1, un commentaire « D884 : à confirmer », et AUCUN test qui fige un nombre autre que 1. Quand l'utilisateur tranche, la formule se met en script (modèle `HOLGarrisonBoard`, `S\Simulation\HOLGarrisonBoard.h:11-47` : pont mince, la source est le script) ou reste en une ligne C++ : choix de la session, pas d'une copie des deux.

## 2. Tâches (15 min chacune : UN test rouge, UN correctif). Ordre 1 à 8.

### T1 : portée depuis le bord du bâtiment (règle pure, C++)
- Lire : `HOLCombatRules.h:100-135` (`GetEdgeDistance`, `IsInRange`), `HOLCombatProcessor.cpp:2547-2570` (la même pince `Clamp(position, Box.Min, Box.Max)` pour viser le « point le plus proche du mur »), `HOLCombatRules.cpp` (corps des fonctions déclarées).
- Modèle : `IsInRange`. Test rouge : nouveau test simple `HeroesOfLiberty.Combat.BuildingShotRange` dans `HOLCombatTests.cpp` (patron l.298) : boîte 800x800 centrée en (0,0), unité de rayon 40 à (400+3000+40, 0) : dans la portée 3000 ; 1 cm de plus : hors portée ; unité à l'intérieur de la boîte : dans la portée. Rouge : la fonction `HOLCombat::IsInBuildingShotRange(const FBox2D&, const FVector2D&, float UnitRadius, float Range)` n'existe pas (échec de compilation du test) puis, bouchon `return false`, rouge par assertion.
- Correctif : la fonction (pince de la position de l'unité dans la boîte, distance au point pincé moins le rayon de l'unité, `<= Range`). Pas de racine inutile (comparer les carrés, le coût est par cible par pas).
- Réseau : aucun. Coût : O(1) par candidat. C++ : OUI (règle pure par entité et par pas, D839).

### T2 : première flèche sur l'ennemi le plus proche, seulement un ennemi, seulement fini
- Lire : `HOLCombatProcessor.h:140-162` (`FStepBuilding`), `HOLCombatProcessor.cpp:1372-1392` (collecte), `:911-912` (`CampTable`), `:854-862` (`IsSeenBy`), `:3085-3150` (relâche de flèche à imiter), `HOLArrowSubsystem.h:15-44`.
- Modèle : la relâche de l'archer (l.3113-3140). Champs de `FArrow` à utiliser : `Shooter` laissé vide (pas d'entité), `ShooterId = Building->GetUniqueID()`, `ShooterType = NAME_None`, `ShooterSlot = owner`, `ShooterCamp`, `Damage = float(Row->BuildingAttack)`, `Target`/`TargetId` de l'unité visée, `ReleaseStep`, `ArrivalStep`.
- Correctif : dans `Execute`, après la collecte de `StepBuildings` et avant la boucle des unités, un bloc : pour chaque bâtiment `IsFinished()` (l'itérateur `FHOLBuildingIterator` ne filtre que Foundation et PV, donc **tester `IsFinished()` ici**) dont la ligne a `BuildingAttack > 0` et (`BuildingFiresEmpty` pour ce premier jet), chercher parmi `Fighters` l'ennemi vivant de plus petite distance de bord (T1) avec `StepCamps.FiresAtOnce(owner, victimOwner)` ; départage : plus petit `UnitId` (déterminisme) ; relâcher une flèche ; `++TotalArrowsReleased`. Pas de cadence encore (une seule flèche : le test s'arrête après la première).
- Test rouge : banc de `HOLGarrisonOrderTests.cpp:696-770`, ligne copiée de TowerWood posée avec `AddRow`, une tour finie (propriétaire 0), un Archer ennemi (propriétaire 1) à 2000 cm de son bord, un Archer allié (propriétaire 0) à 1500 cm : après quelques pas `Combat->GetTotalArrowsReleased() == 1` ET la flèche vise l'ennemi (lire `Arrows` par un accesseur de test ou vérifier la santé de l'ennemi seulement en T3 ; ici : `TotalArrowsReleased` + log `hol.Combat.LogHits`). Contre-test dans le même banc : tour en fondation (`SetConstructionState(Foundation)`) : 0 flèche. Le rouge est net : aujourd'hui `TotalArrowsReleased` reste 0.
- Réseau : le serveur (ou le solo) décide ; ce processeur ne tourne pas sur le client (NON VÉRIFIÉ : l'absence d'exécution côté client n'a pas été relue). Rien n'est répliqué par T2.
- Coût : par pas, une boucle sur les bâtiments déjà collectés (peu : actors) ; la recherche O(F) sur `Fighters` n'a lieu que prête au tir (T4), sinon aucune.
- Sélection : voir plus bas « Questions ». C++ : OUI.

### T3 : la flèche touche, avec l'armure, et le crédit va au joueur
- Lire : `HOLCombatProcessor.cpp:1406-1530` (atterrissage), `HOLCombatRules.cpp` (`GetDamage`, `ResolveArrow`), `Kill(...)` (signature NON VÉRIFIÉE : doit accepter `ShooterId` sans pointeur d'unité), `FindFighter` sur un `FMassEntityHandle` vide (NON VÉRIFIÉ : l'atterrissage l.1448 et l.1455 suppose `INDEX_NONE` pour un tireur absent ; le code garde `Shooter != INDEX_NONE`, donc plausible).
- Modèle : atterrissage existant ; armure : `GetDamage(attaquant, victime, niveau, armure talent, ...)` prend les constantes d'une UNITÉ tireuse, pas utilisables par une tour : nouvelle règle pure `HOLCombat::GetBuildingShotDamage(float Base, const FHOLUnitTypeConstants& Victim, ...)` qui applique la même formule d'armure de tir (D345) sans le niveau du tireur. Bonus de technologie du propriétaire (`RangedAttackPercent[Slot]`, l.941) : question non tranchée, ne pas l'appliquer en v1.
- Test rouge : `HeroesOfLiberty.Combat.BuildingShotDamage` : après `ArrivalStep`, la santé de l'ennemi a baissé exactement de la valeur de la règle ; `TotalArrowsLanded == 1`. Rouge : sans le correctif l'arrivée n'applique pas la valeur d'armure (ou est perdue si l'atterrissage lit un tireur absent).
- Réseau : la santé de l'unité se réplique par le canal normal des unités (`HOLUnitReplicationSubsystem`) ; rien de neuf. C++ : OUI (atterrissage et règle appelée par flèche).

### T4 : cadence 1,625 s, une cadence par tour
- Lire : `HOLCombatRules.h:80-95` (`StartAttack`, `TickAttack`), `HOLCombatRules.h` `ToMilliseconds` (cherche), `HOLCombatProcessor.cpp:2545-2560` (usage).
- Modèle : `Combat.CooldownMs` des unités. Stockage : **`TMap<uint32 BuildingId, int32 CooldownMs>` dans le processeur** (pas dans l'acteur : évite de toucher `HOLBuilding.h`, donc un rebuild de moins ; purge des tours disparues au pas) ; itérer dans l'ordre de `StepBuildings` (déterminisme NON VÉRIFIÉ : l'ordre de `FHOLBuildingIterator` n'est pas prouvé stable, trier par `Step.Id` si doute).
- Correctif : intervalle `HOLCombat::ToMilliseconds(Row->BuildingAttackInterval)` (1625 ms), `StartAttack` quand une cible existe, `TickAttack` à chaque pas, relâche à l'instant du geste ; le compte à rebours continue sans cible (pas de rafale au retour d'une cible : à confirmer).
- Test rouge : un ennemi immobile à portée pendant 5 s de pas simulés : `TotalArrowsReleased` = valeur attendue calculée par la même formule du test à partir de la ligne de table (pas une constante magique) ; deux tours : exactement le double (une cadence par tour, pas partagée). Rouge : T2 ne tire qu'une fois.
- Réseau : rien. Coût : la recherche O(F) sur `Fighters` seulement quand le compte à rebours arrive à 0 et que la cible est perdue ; une tour tire environ toutes les 1,6 s. Réutiliser le pas de recherche des unités (`bSearchStep`, identifiant multiple de 4, l.2341) si on veut étaler ; une grille spatiale d'unités existe-t-elle : NON VÉRIFIÉ (l.1584-1700 non lues). C++ : OUI.

### T5 : la tour ne tire que sur ce que son propriétaire voit
- Lire : `HOLCombatProcessor.cpp:854-862` (`IsSeenBy` prend un `FFighter` chercheur : ajouter une variante qui prend le slot du propriétaire du bâtiment), `HOLVisionSubsystem.h:104,149`, et le test existant du brouillard du combat (F5, FogInProgress.md point 3 ; grep `IsVisible` dans `S\Tests\` : NON VÉRIFIÉ, non fait).
- Modèle : `IsSeenBy` + le banc de ce test du brouillard.
- Correctif : filtrer les candidats par `Vision->IsVisible(OwnerPlayer, Fighters[i].Location)` (Vision nullptr si brouillard éteint, comme l.856-858). La tour éclaire elle-même (SightRadius 4800 > 3000) : la vue est vraie sans autre observateur dans le banc.
- Test rouge : brouillard actif, l'ennemi à portée mais hors de la vue de la tour (par exemple tour de fondation ou son propriétaire n'a pas de source de vue sur la cellule) : 0 flèche ; la cible se voit : 1 flèche. Si le banc de vue est trop lourd, régler le `SightRadius` de la ligne copiée à 0 dans le test : la tour ne voit rien. Rouge : T2-T4 tirent sans regarder le brouillard.
- Réseau : la vue est du serveur ; rien de répliqué. C++ : OUI (même appel que le combat).

### T6 : tire vide ou garnie (FiresEmpty), nombre de flèches (valeur provisoire 1)
- Lire : `HOLUnitSubsystem.h:174-182`, `HOLGarrisonOrderTests.cpp:696-770` (banc avec `GarrisonSlots = 2`), `HOLGarrisonSubsystem.cpp:94` (`EjectAll`).
- Correctif : une tour tire si `Row->BuildingFiresEmpty || occupants > 0` ; nombre d'occupants lu par `Units->GetGarrisonUnitIds(Building, Ids).Num()` ; flèches par tir = `HOLCombat::GetBuildingArrowCount(occupants)` (rend 1 ; voir §1).
- Test rouge : ligne copiée avec `BuildingFiresEmpty = false` : 0 flèche sans occupant ; après `UHOLUnitSubsystem::SetGarrison` d'un Archer allié : ≥ 1 flèche. Deuxième test `FiresEmpty = true` : tire sans occupant (c'est TowerWood). Aucun test n'affirme le nombre d'occupants multiplié tant que D884 ne le dit pas.
- Réseau : l'occupant vient de la liste serveur ; le client propriétaire reçoit la liste répliquée, rien de plus. Coût : un appel par tir, pas par pas. C++ : OUI (lecture Mass / sous-système) ; formule en C++ ou script plus tard.

### T7 : affichage de la flèche (solo, hôte) et mort de la tour
- Lire : `HOLArrowSubsystem.h:15-44,62`, `HOLArrowSubsystem.cpp:68-80,171` ; `HOLCombatTests.cpp:506-560` (patron Projectiles : `GetLaunchedCount`).
- Correctif : à la relâche, `FHOLArrowLaunch` avec `From` = centre de la boîte + `Height` du type (le toit, pas le sol : 800 cm), `To` = position de la cible, `Target` = l'entité visée, `FlightMs`, `ShooterType = NAME_None` (donc SM_Arrow). Purge de la table de cadence quand la tour disparaît.
- Test rouge : `Arrow display GetLaunchedCount()` augmente d'une flèche par tir de tour ; une tour détruite (`UHOLConstructionSubsystem::DestroyBuilding`) ne tire plus aux pas suivants, ses flèches déjà en vol atterrissent comme celles d'un archer mort (« whatever the shooter does meanwhile », l.3109) : à confirmer par l'utilisateur ; les occupants sont éjectés (déjà : `EjectAll`).
- Réseau (T7b, tâche suivante) : le client n'a pas la flèche de tour (voir §0) : champ répliqué sur `AHOLBuilding` (par exemple un compteur de tirs et l'identifiant de la cible, réplication à tous : le filtre de brouillard des bâtiments s'applique déjà aux acteurs, `HOLVisionSubsystem.h:173-187`) et `OnRep` qui appelle `Launch` ; test `HOLNetTests.cpp`. C++ : OUI.

### T8 : coût par pas, scénario `tower_wood`, documents
- Lire : `S\StressTest\HOLStressTestSubsystem.cpp` (modèle du coût), `Tools\Scenarios\` (aucun scénario TowerWood : le grep de 1122b le dit), `HOLCommandGrid_Test` l.524-532 (case F de TowerWood).
- Correctif : un compteur de coût (tours tirant, recherches par pas) lisible par un test ; scénario `tower_wood` (une tour contre une escouade, avec et sans garnison, FortificationsInProgress.md l.130-137 de F8) ; mise à jour de `Placeholders.txt`, `FortificationsInProgress.md`, `EtapeDefini.md`.
- Test rouge : un banc de 200 unités et 10 tours : le coût de recherche par pas reste sous le budget fixé par la session (valeur à poser : non décidée ici). C++ : OUI pour le compteur, non pour le scénario (script / données).

## 3. Réponses directes
- **Quels bâtiments tirent déjà ?** Aucun (aucun lecteur des 4 colonnes de ligne ; seuls les UNITÉS ont un tir de bâtiment).
- **Le bâtiment en chantier doit-il tirer ?** Non, recommandé : tester `IsFinished()` (la collecte `StepBuildings` ne retire que la fondation, l.1380) ; à confirmer par l'utilisateur (c'est une décision de conception, pas dans D884).
- **Ordre de priorité de cible (à confirmer)** : le plus proche du bord (T1), départage au plus petit identifiant ; garder la cible tant qu'elle est vivante, vue, à portée (sinon nouvelle recherche) : proposé, non tranché.
- **Réseau** : le serveur (ou le solo) décide tout dans le combat ; les dégâts passent par la santé répliquée des unités ; seule la flèche affichée chez un client demande un champ neuf (T7b).

## 4. Questions de design non tranchées (sans les poser)
1. Nombre de flèches par occupant (D884 : à confirmer ; proposé 1 + 1 par occupant) et si le type d'occupant compte (Archer, Arbalétrier, Lancier, Héros).
2. Cible : héros ? engins de siège ? bâtiments ennemis ? unités des camps neutres (propriétaire `NeutralOwner` : `Relation` rend Enemy, donc la tour les viserait) ? unités en fuite ?
3. Priorité : le plus proche, le plus faible, tireurs d'abord, garder ou changer la cible à chaque flèche ?
4. Portée depuis le bord du corps (retenu ici) ou depuis le centre ; hauteur (D693 : bonus de hauteur pour la tour : sur la portée ou les dégâts ?).
5. Bonus de technologie du propriétaire sur le tir de tour ; XP et statistiques de la partie (`HOLMatchStats`) : à qui va la mort.
6. Un coup d'une tour sur une unité neutre réveille-t-il son camp (`HOLCamps::NoteHit` n'est appelé qu'avec un tireur-unité).
7. Tour en chantier (voir §3) ; tour abîmée tire-t-elle moins ; une flèche en vol à la mort de la tour.
8. Compte à rebours à vide : le temps passé sans cible compte-t-il (rafale au retour) ou repart-il à zéro.
9. Le lancement de flèche est-il visible d'un joueur qui ne voit pas la tour (brouillard) ; le « tire plus avec ses occupants » du F8 est-il en flèches ou en cadence.
10. Tir allié : une tour sur une unité d'un joueur en Neutre diplomatique (`FiresAtOnce` dit non) : à confirmer ; tir sur une unité alliée écartée.

## 5. Provisoire d'affichage (Placeholders.txt, chemin absolu `C:\Users\baudo\Documents\MyCrusader\Placeholders.txt`)
- l.131 « Flèche (SM_Arrow, mesh fait par script) [style] » existe déjà : la flèche de tour réutilise ce provisoire (aucune ligne neuve pour le mesh).
- Ligne à AJOUTER avec T7 : « Tir de la Tour de guet en bois : la flèche part du centre du toit (hauteur du type), sans meurtrière, sans animation d'archer, sans fumée de poudre, sans son [forme] » ; et, avec T7b, « flèche de tour chez un client : tirée à la réplication, pas à l'instant du tir [forme] ».
- l.74 « Tour de guet en bois [à venir] » (colonne Mesh de la ligne vide) : reste tant que le maillage n'existe pas.

## NON VÉRIFIÉ
Compilation, tous les tests, le rouge de chaque tâche (aucun build). Non lus : `HOLCombatRules.cpp` (corps de `GetDamage`, `ResolveArrow`, `StartAttack`), signature de `Kill`, comportement de `FindFighter` sur une poignée vide, les l.1584-1700 (recherche des unités : existence d'une grille spatiale), l'ordre de `FHOLBuildingIterator`, le banc du test de brouillard F5, les valeurs de `EHOLConstructionState`, l'état du `.uasset` de BuildingTypes dans integ-41, l'exécution du combat sur un client. La ligne TowerWood (GarrisonSlots 3) vient du CSV, pas du `.uasset`.
