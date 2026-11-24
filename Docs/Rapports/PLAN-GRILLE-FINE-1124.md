# PLAN-GRILLE-FINE-1124 : grille fine des murs (D898 + D904 + D901), SAURON-1124a, 2026-11-24 10:5x

Docs seules, aucun code, aucune decision tranchee. Lu sur main 173d31b (worktree Sauron). Les valeurs marquees « defaut propose » sont des propositions
a confirmer, pas des choix.

Impact joueur : au lieu de poser des blocs de 4 m, le joueur trace un mur fin (1 m) dont le debut et la fin tombent la ou il clique ; il paie chaque
metre (5 Bois la sous-case de palissade) et une case occupee fait un trou au lieu de tout annuler.

## 1. Decisions deja prises (a integrer telles quelles)
- D898 : une case se divise en 4 sous-cases en longueur ET en largeur (donc 16 par case) ; le plus important est le debut et la fin, le reste est un trace.
- Q1 = A : palissade + mur de bois + mur de pierre sur la grille fine (lignes CSV `Palisade`, `StoneWall` ; `PalisadeGate` : voir point 2).
- D904 : cout = prix actuel PAR SOUS-CASE (palissade 5 Bois, mur de pierre 10 Pierre) ; une ligne de meme longueur coute 4 fois plus.
  Exemple lu : le geste reel de SAURON-1123pb (9 fondations, Bois 20200 -> 20155 = 9 x 5) devient 36 sous-cases = 180 Bois.
- D901 (E3, ligne [ ] du socle, code sur la grille ACTUELLE) : une case bloquee est sautee (trou), les autres sont posees et payees ; toutes bloquees = rien ;
  fantome rouge de la case sautee. Ce plan ne la refait pas : `FHOLWallLinePlan.Skipped` (E3) sert tel quel sur la grille fine.

## 2. Ce que le code fait aujourd'hui (lu)
- `Script\Buildings\HOLWallLineRules.as` est PUR et prend deja `Tile` en parametre : `DragCells`/`Cells` (:14-70, entiers, 8 directions, seuil tan 22,5 = 0,4142,
  diagonale `Steps = (AX+AY+1)/2`), `Validate` (:96-141), `CellOfCoordinate` (floor), `CellToPoint(Cell, Tile, Z)` = centre (Cell+0,5)*Tile (:164),
  `PlanFromRequest` (:182-200, garde `Limit = 100000 * Tile`), `Cost` = cases x prix (:60), `CVar_WallMaxLineCells` (:12, 0 = sans limite, D867).
  Passer Tile = 100 au lieu de 400 donne donc DEJA 4 sous-cases par case : la regle ne change presque pas, c'est le monde autour qui est a 400.
- La grille de 4 m vient de `UHOLBuildingSettings::TileSize = 400` (HOLBuildingTypes.h:259, `GetTileSize` :261). `UHOLConstructionSubsystem::SnapFootprint`
  (HOLConstructionSubsystem.cpp:73-76) = `HOLConstruction::SnapToGrid(Point, Type.Footprint, GetTileSize())` (HOLConstructionRules.cpp:10, CORPS NON LU) ;
  appelants : HOLConstructionSubsystem.cpp:443, HOLPlayerController.cpp:3334 et :3931, HOLControllerCommandLibrary.cpp:803, HOLAIOrders.cpp:179 (IA).
- Lignes de murs dans `Tools\Data\BuildingTypes.csv` : `Palisade` (Footprint 400x400, 400 PV, 5 Bois, 5 s, slot 6, Shape BlockAndTower, Walkable False),
  `PalisadeGate` (400x400, 10 Bois, 8 s, slot 10), `StoneWall` (400x400, 1200 PV, 10 Pierre, 8 s, slot 7, age 1). Pas de colonne de grille par type.
- Serveur : `ServerPlaceWallLine` (Script\Net\HOLRpcPlayer.as:1518-1543) lit `HOLConstructionRead::GetWallLineWorld(MapMin, MapMax, Occupied)` (rend `Tile`, cases
  occupees EN CASES DE 4 M ; HOLConstructionReadLibrary.h:116, CORPS NON LU), appelle `PlanFromRequest`, puis UN `OrderPlaceBuilding` PAR CASE avec
  `BuildersForCell`/`QueueForCell` (ARWEN-1123pe : tous les Paysans sur chaque case, file) : 4 fois plus d'ordres (cases x Paysans).
- Client : le glisser (`BeginWallDrag` HOLPlayerController.cpp:3431, `WallDragCells` :3331/:3443, `WallDragTile` :3440) appelle `UHOLWallLineBridge::NativeDragCells`
  = `HOLWallLineBridge.as:15-19` -> `DragCellsFromPoints(Start, End, Tile, MaxLineCells())` ; le dessin des fantomes par case n'a pas ete lu.
- Navigation : un batiment non `Walkable` bloque le navmesh une fois pose (`HOLConstruction::BlocksNavigation` HOLConstructionRules.cpp:351 ; HOLBuilding.cpp:160).
- IA : aucune pose de mur par l'IA trouvee (le Grep de `Palisade`/`IsWallType` ne touche que Script\Buildings) ; l'IA pose ses batiments par `CheckPlacement` +
  `SnapFootprint` (HOLAIOrders.cpp:179). Cote IA le sujet est l'ATTAQUE des murs et le chemin (point 6).

## 3. Les 6 points a preciser, avec un defaut propose (aucun n'est tranche)
1. Epaisseur et collision. Mur = 1 sous-case = 1 m d'epaisseur sur 1 m de long (lecture de D898 « largeur »). Questions : l'unite passe-t-elle un trou de 1 m
   laisse par une case sautee (D901) ? un navmesh a rayon d'agent > 50 cm ne passe pas. Defaut propose : piece 100x100, non praticable, `BlocksNavigation` par piece,
   un trou d'UNE sous-case reste infranchissable (a mesurer en scenario, etape G6). PV, duree et prix PAR SOUS-CASE : D904 fixe le prix, pas les PV (400 PV par metre au lieu
   de par 4 m = mur 4 fois plus solide) ni la duree (5 s par sous-case = 4 fois plus long a construire) : defaut propose = lignes CSV inchangees, PV/duree par sous-case, a
   confirmer (Q-grille-fine-structure, question B).
2. Portes. `PalisadeGate` = 1 case 400x400 (CSV) ; `GateReplaces` (HOLWallLineRules.as ~:249-255) ne remplace que la palissade du MEME camp. Sur la grille fine la porte
   couvre 4 sous-cases de large : defaut propose = empreinte 400x100 (ou 100x400) orientee comme la ligne, ce qui demande une ROTATION d'empreinte (le fichier la declare
   « hors regle ») donc du C++. Ni l'ouverture ni le passage du proprietaire (`GatePassesUnit`) ne changent.
3. Alignement 8 directions. La regle garde les 8 directions (`Cells`). Constats lus : (a) sur la grille fine la diagonale avance de 1,414 m par sous-case contre 1 m sur un axe, donc
   la diagonale revient MOINS cher au metre (D904 = prix par sous-case) ; (b) deux pieces carrees posees en diagonale ne se touchent que par un coin : une unite peut se
   glisser entre les deux si le navmesh le permet. Defaut propose : on ne change rien en (a), et (b) est mesure (G7) avant de choisir entre « piece de remplissage », pieces
   tournees a 45 degres ou rien.
4. Rendu. Fantomes : un par sous-case (jusqu'a 4 fois plus, ex. 94 cases = 376 fantomes) ; rouge pour la case sautee (D901) ; fondations : `FoundationStakeSize`
   (HOLBuilding.cpp:1178) pose un piquet par fondation, a verifier a 1 m ; maillages : `Mesh` est vide dans le CSV (forme de moteur, ligne Placeholders.txt a mettre a jour a
   chaque changement de forme). Defaut propose : une instance (ISM) pour les fantomes ; rien pour le maillage final.
5. Reseau. Un RPC par ligne (Start, End : inchange) mais le serveur cree 4 fois plus d'acteurs `AHOLBuilding` repliques et 4 fois plus d'ordres (`OrderPlaceBuilding` par
   case et par Paysan). Defaut propose : une pièce = un acteur (reutilise toute la chaine), a MESURER (G9) avec le compteur de cout de la tour (F8 T8) comme modele, et seulement
   alors Q-grille-fine-structure question A (un acteur par pièce ou un segment par ligne). `hol.Wall.MaxLineCells` (0 = sans limite, D867) compte desormais des SOUS-cases : la
   valeur d'un reglage existant change de sens (a dire dans le journal).
6. IA. L'IA ne pose pas de murs aujourd'hui. Reste : (a) le chemin autour d'un mur fin (navmesh reconstruit pour chaque piece : 376 zones sales a regrouper), (b) l'attaque d'un mur
   (choix de la cible parmi 4 fois plus de pieces de 400 PV), (c) `SnapFootprint` appele par l'IA (HOLAIOrders.cpp:179) doit garder la grille de 4 m pour les batiments normaux.
   Defaut propose : rien a coder cote IA avant G8 ; seulement verifier (c) par un test.
7. (point ajoute) Debut ou fin de ligne bloques. D898 dit que le debut et la fin comptent : si la sous-case de debut ou de fin est occupee, D901 la saute comme les autres. Defaut
   propose : saut et fantome rouge (D901), sans decaler la ligne ; a confirmer car D898 insiste sur ces deux cases.

## 4. Decoupe en etapes de 15 min (un test rouge VU a l'execution par etape ; bouchon + test dans le commit 1, correctif dans le commit 2)
Prerequis : E3 (D901, Skipped) fusionnee ; G1 a G3 editent HOLWallLineRules.as et HOLWallLineRules_Test.as, les memes fichiers que E3 : jamais en parallele.
Reglage de retour arriere : cvar `hol.Wall.SubDivision` (defaut 1 = grille actuelle tant que G10 n'est pas valide, 4 apres), une ligne a changer ; les 28 tests actuels de
HOLWallLineRules_Test.as gardent SubDivision = 1.

- G1 (script, 15 min) Pas fin de la regle. Fichiers : Script\Buildings\HOLWallLineRules.as, Script\Tests\HOLWallLineRules_Test.as. Bouchon `PlanFromRequestFine(Slot, Start, End,
  Tile, SubDivision, MapMin, MapMax, OccupiedTiles, MaxCells)` qui rend le plan de grille de 4 m. PREMIER TEST ROUGE : Start (50,50,0), End (1650,50,0), Tile 400, SubDivision 4 ->
  `Plan.Cells.Num() == 17` (bouchon : 5), premiere case (0,0), derniere (16,0) ; `Cost(...)` = 17 x 5 = 85 Bois. Correctif : Pitch = Tile / SubDivision passe a `PlanFromRequest`,
  bornes de la carte multipliees par SubDivision.
- G2 (script, 15 min) Occupation en sous-cases + saut. Meme fichiers. Bouchon `ExpandOccupied(OccupiedTiles, SubDivision)` vide. ROUGE : case de 4 m (2,0) occupee, meme ligne que G1
  -> `Skipped` = les 4 sous-cases (8..11, 0), `Cells.Num() == 13`, cout 65 ; toutes occupees -> `Refusal == Occupied`, 0 case. Correctif : chaque case occupee = SubDivision x SubDivision
  sous-cases. NB : les pieces de mur deja posees (100x100) sont vues par `GetWallLineWorld` comme une case de 4 m : approximation levee en G4 (C++).
- G3 (script, 15 min) Debut et fin sous la souris (D898). Meme fichiers. Bouchon `PieceBox(Cell, Pitch)` qui rend une boite nulle. ROUGE : ligne de G1 -> `PieceBox` de la premiere
  case commence a x = 0, de la derniere finit a x = 1700 ; les boites se touchent sans se chevaucher (somme des largeurs = 17 x 100) ; diagonale (50,50) -> (850,850) : 9 cases, derniere
  case (8,8) ; tir a 30 degres : colle sur l'axe (`DragCells` : 8 directions gardees). Correctif : `PieceBox` = [Cell x Pitch, Cell x Pitch + Pitch].
- G4 (C++, lot C++ groupe, UN build) Monde en sous-cases. Fichiers : Source\HeroesOfLiberty\Buildings\HOLConstructionReadLibrary.h/.cpp (`GetWallLineWorld` avec un pas, occupation
  rendue en sous-cases a partir des boites des batiments ET des pieces de mur), HOLConstructionSubsystem.cpp:73 (`SnapFootprint` : grille par type, pas de 4 m pour les batiments normaux),
  Tools\Data\BuildingTypes.csv (`Palisade`, `StoneWall`, `PalisadeGate` : Footprint 100x100, 400x100 pour la porte) + regeneration du .uasset (editeur, session principale seule).
  ROUGE (script, appelle la liaison exposee) : `SnapFootprint` d'une Palissade en (130, 70) = boite 100..200 x 0..100 ; d'un Entrepot = boite sur la grille de 4 m inchangee ;
  `GetWallLineWorld` avec un mur pose en (0,0) ne rend QUE la sous-case (0,0) occupee. Raison C++ admise : liaison a exposer au script (brief D839).
- G5 (script + 1 appel C++ deja prevu en G4) Serveur : une piece par sous-case, payee. Fichiers : HOLRpcPlayer.as `ServerPlaceWallLine` (:1518-1543, E3 y a touche aussi : relire),
  HOLWallLineRules.as `BuildersForCell`/`QueueForCell`. ROUGE : pure `OrdersForLine(Plan, UnitIds)` -> N pieces x M Paysans ordres (13 x 2 = 26) ; scenario `wall_line_long` au geste reel
  (play.ps1 -Hidden, preflight) : Bois avant - apres = 5 x pieces posees (l'effet), plus le journal `place wall line ... N cell(s)`. Question de charge : G9.
- G6 (C++/Mass, lot C++) Collision et navmesh a 1 m. ROUGE (scenario `wall_fine_block`) : une ligne de 8 sous-cases entre un Paysan et un but : le Paysan n'arrive PAS (position finale
  du meme cote), et avec UNE sous-case sautee le resultat est enregistre et lu au journal (infranchissable ou non : c'est la mesure du point 1, pas un choix).
- G7 (C++/Mass, lot C++) Diagonale. ROUGE (scenario `wall_fine_diagonal`) : diagonale de 8 pieces : l'unite ne traverse pas. Si elle traverse : le plan note la cause et ouvre la question
  (piece de remplissage / rotation 45 degres), sans coder le choix.
- G8 (script) Fantomes et rouge de la case sautee (client). Fichiers : HOLWallLineBridge.as, HOLPlayerController.cpp:3331-3443 (relire : gestion du glisser), le dessin des fantomes (a
  chercher). ROUGE : `DragCells` du pont rend 17 cases pour G1 et la liste des sautees ; le fantome rouge existe pour chaque sous-case sautee (compte lu par l'etat du controleur, pas
  une ligne de journal).
- G9 (C++ mesure, lot C++) Cout par pas. Modele : `BuildingShotCost` (CELEBORN-1123e). ROUGE : 376 pieces posees (le geste de 94 cases), nombre d'acteurs repliques et reconstructions de navmesh
  par pas bornes ; seuil a poser par la session (valeur a regler en test, D904), non decide ici.
- G10 (script + donnees) Portes sur la grille fine et bascule du defaut a 4. Fichiers : HOLWallLineRules.as `GateReplaces`, test. ROUGE : une porte remplace exactement les 4 sous-cases
  alignees de la palissade du meme camp et pas d'autres ; la porte d'un autre camp ne remplace rien. Puis `hol.Wall.SubDivision` = 4 par defaut, Placeholders.txt a jour.

Ordre et dependances : G1 -> G2 -> G3 (script, une seule file de worktree) ; G4 (lot C++) avant G5, G6, G7, G9 ; G8 apres G1 ; G10 apres G4 et G6. Les etapes C++ (G4, G6, G7, G9) se regroupent
en UN lot, UN build lance par la session principale.

## 5. Lignes [ ] pretes pour la File (a coller dans Auto_Journey_File.md par la session principale ; je n'edite pas ce fichier hors de ma ligne D898)
- [ ] (D898 G1, plan PLAN-GRILLE-FINE-1124.md, script, apres E3) pas fin : `PlanFromRequestFine` ; TEST ROUGE : (50,50)->(1650,50), Tile 400, SubDivision 4 -> 17 cases (bouchon 5), cout 85 Bois ; HOLWallLineRules.as + test ; journal FortificationsInProgress.md
- [ ] (D898 G2, script) occupation en sous-cases : case de 4 m (2,0) occupee -> 4 sous-cases sautees, 13 posees, cout 65 ; toutes occupees -> Refusal Occupied
- [ ] (D898 G3, script) debut et fin sous la souris : `PieceBox` ; TEST ROUGE : boites 0..1700, contigues, diagonale (50,50)->(850,850) = 9 cases, derniere (8,8)
- [ ] (D898 G4, lot C++) monde en sous-cases : `GetWallLineWorld` avec un pas, `SnapFootprint` par type, CSV Palisade/StoneWall/PalisadeGate 100x100 (porte 400x100) + regen .uasset ; TEST ROUGE script : SnapFootprint Palissade (130,70) = 100..200 x 0..100, Entrepot inchange
- [ ] (D898 G5) serveur : `ServerPlaceWallLine` une piece par sous-case, `OrdersForLine` ; TEST ROUGE : 13 pieces x 2 Paysans = 26 ordres ; scenario wall_line_long : Bois = 5 x pieces
- [ ] (D898 G6/G7, lot C++, scenarios) collision a 1 m, trou d'une sous-case, diagonale : mesure, pas de choix ; `wall_fine_block`, `wall_fine_diagonal`
- [ ] (D898 G8) fantomes par sous-case et rouge de la sautee (HOLWallLineBridge.as, controleur)
- [ ] (D898 G9, lot C++) cout par pas : 376 pieces, acteurs repliques, navmesh ; seuil a poser par la session
- [ ] (D898 G10) portes sur la grille fine puis `hol.Wall.SubDivision` = 4 par defaut, Placeholders.txt

## 6. Questions ouvertes (2 fichiers au plus : `Retours\Q-grille-fine-structure.md`)
- A : une piece = un acteur (defaut propose, reutilise toute la chaine) ou un segment par ligne (peu d'acteurs, chantier plus gros) ; recommandation : A puis mesure G9.
- B : PV et duree PAR SOUS-CASE : lignes CSV inchangees (mur 4 fois plus solide et 4 fois plus long a batir au metre) ou divises par 4 (meme solidite et meme duree au metre, seul le prix change :
  contredit D904 « ligne 4 fois plus chere » pour le prix, pas pour les PV) ; recommandation : garder les lignes inchangees pour G1 a G5, regler les PV en G9 avec la mesure.
Les points 1, 2, 3, 4, 6, 7 restent des lignes de ce plan avec defaut propose (aucune question).

## 7. NON VERIFIE
Corps de `HOLConstruction::SnapToGrid` (HOLConstructionRules.cpp:10), de `GetWallLineWorld`, du dessin des fantomes, de `OrderPlaceBuilding` / `AskPlace` (file des Paysans) ; aucune des mesures
(trou d'une sous-case, diagonale, cout de 376 pieces) : toutes sont des etapes du plan, pas des constats. Les nombres des tests (17, 13, 85, 65, 26) sont calcules a la main a partir des regles lues,
a recontroler au premier rouge.
