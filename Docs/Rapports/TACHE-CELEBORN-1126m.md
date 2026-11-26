# TACHE-CELEBORN-1126m : D921 (la porte rembourse) puis G10 (portes, restes)

Écrit par PREP-1126m le 2026-11-26 d'après main 5dc2318 (grep et `git merge-base` du jour). Worktree : `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`, branche neuve du socle partant de main (art-lab-1125ar est finie et dans main) ; séance art parallèle possible : écrire `IN_USE.txt` (`CPP: non`) et céder au premier commit art. Lis `Tools\AutoJourney\brief_commun.md` et `agent_rules.md`.

## Joueur (une ligne)
Quand le joueur pose une porte sur sa palissade finie, il récupère une part du prix des pièces que la porte remplace (aujourd'hui elles disparaissent sans rien rendre).

## Relevé vérifié (ne pas refaire)
- Retrait par la porte DÉJÀ branché dans main : `ServerPlaceWallLine` (`Script\Net\HOLRpcPlayer.as:1613`, boucle `OrderRemoveOwnBuilding` sur `GateRetireIndices`) ET `ServerPlaceBuilding` (`HOLRpcPlayer.as:1533-1555`, `GatePoseRetireIndices`, CELEBORN-1125f). Règles pures : `Script\Buildings\HOLWallLineRules.as` `GateReplacedCells` :513, `GateRetiredCells`, `GateRetireIndices` :543, `GatePoseRetireIndices` :560 ; tests `Script\Tests\HOLWallLineRules_Test.as` :763, :798, :825, :858, :896.
- Prix par sous-case (D915) DÉJÀ branché : `CostOfLine` :90, `LinePercent()` :99 (cvar `hol.Wall.SubCellCostPercent` :21, défaut 25) passé à `OrderPlaceBuilding` (`HOLRpcPlayer.as:1621`). Palissade = 5 Bois, porte = 10 Bois (`Tools\Data\BuildingTypes.csv:18-19`).
- `hol.Wall.GateRefundPercent` n'existe nulle part (grep Script + Source : 0 résultat).
- **Le crédit est du C++ (L29-2, chaîne suivie jusqu'au premier `.cpp`)** : `HOLRpcBridge::OrderRemoveOwnBuilding` = `UHOLRpcBridgeLibrary::OrderRemoveOwnBuilding` (`Source\HeroesOfLiberty\Net\HOLScriptRpcPlayerActor.cpp:496`) appelle `UHOLConstructionSubsystem::DestroyBuilding` (`HOLConstructionSubsystem.cpp:667`) : aucun crédit ; aucune API script ne crédite le stock d'un joueur (seul `CheatResources`, `HOLScriptRpcPlayerActor.h:106`, triche). La ligne File l. 14 dit « sans C++ » : c'est faux pour le crédit seul ; **la règle est script, la liaison est C++** et va au lot C++ n°3 (`TACHE-CPP3-1126m.md`, point D921). Tu ne compiles rien.

## Tâche 1 : D921, règle script + cvar + test rouge (sans C++)
Lis d'abord `Retours\Q-porte-remplacement-remboursement.md` (ou sa copie dans `Docs\Archive\Retours_*`) et `FortificationsInProgress.md:303` : le texte exact de la réponse B fait foi (« une part du prix des palissades retirées, p. ex. la part de sous-case de D915, 25 % »). Défaut codé : la part est `hol.Wall.GateRefundPercent` (défaut 25, 0 = ancien comportement) du prix d'une case de 4 m de chaque pièce retirée, arrondie vers le haut sur le TOTAL (même arrondi que `CostOfLine`). Si le texte permet deux lectures (part du prix payé ou part du prix d'une case), pose UNE question `Retours\Q-porte-remboursement-base-1126m.md` `[À RÉPONDRE]` (A/B/Autre, recommandation, défaut codé) et applique le défaut ; au plus 2 questions.

1. **Premier Edit** : `Script\Tests\HOLWallLineRules_Test.as`, fin du fichier (après `Test_IsolatedGatePoseRetiresTheFinishedPalisadesUnderIt` :896) : `Test_GateRefundIsAShareOfTheRetiredPalisades` (3 pièces palissade finies sous une porte, `GateRefundOf` avec `Percent` 25 doit valoir 4 Bois : 3 x 5 x 25 / 100 = 3,75 arrondi à 4 ; `Percent` 0 = 0 ; liste vide = 0 ; une pièce d'un autre camp ou de pierre n'ajoute rien). La valeur attendue vient du calcul de la règle sur une vraie liste de pièces (`GateRetireIndices` + prix de la table), jamais recopiée.
2. Bouchon dans `HOLWallLineRules.as` (après `GatePoseRetireIndices`, :560-582) : `int GateRefundOf(const TArray<FName>& RetiredTypes, int Percent)` qui renvoie 0 ; ajouter `CVar_WallGateRefundPercent("hol.Wall.GateRefundPercent", 25)` à côté de :21 ; accesseur `GateRefundPercent()` borné à 0 pour une valeur négative. Lancer le test : rouge VU (compte N-1/N), commit 1 = bouchon + test (`Agent: CELEBORN-1126m`).
3. Correctif : `GateRefundOf` somme le prix de chaque type retiré (lire la table via l'accesseur de prix déjà utilisé par les tests de `CostOfLine`, Grep `CostOfTile` / `PricePerTile`), applique `Percent`, arrondit sur le total ; commit 2.
4. **Branchement (C++ requis, NE PAS faire ici)** : dans les deux boucles de retrait (`HOLRpcPlayer.as:1551` et `:1613`), le crédit se fait par une liaison C++ neuve ; l'agent du lot C++ n°3 la pose. Toi : écris dans `FortificationsInProgress.md` la signature attendue (`OrderRemoveOwnBuildingRefund(AActor* RpcActor, AHOLBuilding* Building, int32 RefundPercent)` ou un crédit `FHOLResources` par slot) et ajoute `TYPES_A_EXPOSER: bridge crédit` au rapport.

## Tâche 2 (si 10 min et 15 appels restent) : G10 portes, restes sans C++
Fait dans main : règle, retrait à la pose seule et en ligne (rouge 4ba5ec2, correctif a99f9ad ; 250e976 ; CELEBORN-1125f). Reste de la ligne File l. 109 :
- Empreinte de la porte ORIENTÉE : **C++** (hors tâche, lot C++ de la séance, à signaler).
- `Placeholders.txt` : la ligne de la porte existe (:78, « Porte de palissade », « Porte de bois [à venir] » :82) ; relire, ne rien ajouter si D921 ne crée pas de provisoire.
- Rejouer le scénario `wall_gate_replace` (`Tools\Scenarios\wall_gate_replace.txt`, dans main) : `preflight.ps1 wall_gate_replace`, ouvrir CHAQUE PNG du `.look` et nommer l'objet vu ; si PASS, fiche `Retours\E-wall_gate_replace.md` (gabarit `E_template.md`, `GESTE:`, `SON:`, `branches:`) ; sinon « LOGIQUE SEULEMENT ».

## Limites
- Aucun C++, aucun build, aucun `quick_check` (la session principale lance `quick_check -Parts tests`).
- Fichiers pris par un autre agent de la vague : voir `Auto_Journey_Jour.md` (L21-2) ; ne touche pas `HOLPlayerController.cpp`.
- Journal : `FortificationsInProgress.md` ; ta ligne `EtapeDefini.md` ; ta ligne du File (l. 14 : ajouter « crédit = C++, lot n°3 »).
- Rapport : `Docs\Rapports\CELEBORN-1126m.md` (13 champs).
