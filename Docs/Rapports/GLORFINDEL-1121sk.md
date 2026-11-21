# GLORFINDEL-1121sk (2026-11-21, 20:49 -> ~20:53)

NON COMPILE, aucun build, aucun quick_check. Worktree HeroesOfLiberty-Glorfindel, branche ability-columns.
APPELS_DECLARES: ~12 ; BUILDS_DEMANDES: 1 (lot C++) ; TESTS_SCRIPT_AJOUTES: 2 (Test_FollowBridgeRightClickOnAllyStartsFollow, Test_FollowBridgeEnemyOrOwnSelectionIsNotFollow)
TYPES_A_EXPOSER: aucun nouveau (UHOLFollowBridge est le C++ neuf).
DIAGNOSTICS_AS: non releves (serveur de langage non consulte).

## Commits
- 2136ad3 (test rouge) : Source\HeroesOfLiberty\Player\HOLFollowBridge.h/.cpp (enveloppe C++, calque HOLWallLineBridge ; sans classe script Get() = nullptr, les appels rendent false / rien) + Script\Orders\HOLFollowBridge_Test.as.
- 5508632 : Script\Orders\HOLFollowBridge.as (UHOLFollowBridgeScript : RightClickOnUnit = HOLFollowOrders::ApplyRightClick(Holder.Book, ...) == Follow ; Step = StepUnit pour chaque suiveur, parcours a l'envers, suiveur fini retire des 4 tableaux du livre) + HOLPlayerController.cpp : include, OrderAt (apres le bloc soin Moine : allie non neutre sous le curseur -> NativeRightClickOnUnit, log "Order: %d units follow", OnOrderGiven, return ; sinon chemin sol inchange ; ennemi garde RequestAttackUnit), Tick (IsLocalController() && HOLPlayerState -> NativeStep).

## Rouges attendus nom par nom
Sur 2136ad3 (pas de classe script) : Test_FollowBridgeRightClickOnAllyStartsFollow ROUGE (Bridge nul, assert 1 ; retour avant les autres) ; Test_FollowBridgeEnemyOrOwnSelectionIsNotFollow ROUGE (Bridge nul). Sur 5508632 : les deux verts attendus.
Commande (session principale) : Tools\build.ps1 -LiveCoding=0 (HOLFollowBridge.*, HOLPlayerController.cpp) puis quick_check -Parts tests.

## RELECTURE_CPP / appels script contre signature
- BP_RightClickOnUnit : TArray<int32> par valeur (comme DragCells du modele, evite l'inconnue const& des overrides) ; override AS `TArray<int> Selected` par valeur. ApplyRightClick(FHOLFollowBook&, const TArray<int>&, bool, int, bool) OK ; StepUnit(UObject WorldContext = Controller, UHOLCommandSubsystem, int, FHOLFollowBook&, int, bool) OK ; HOLCommandScript::GetCommands() sans argument (HOLPlayerOrdersBoard.as:100) ; WorldContext explicite (piege 7) respecte.
- Own est TArray<uint32> : copie en TArray<int32>.

## NON VERIFIE / limites
- Tout (rien compile). Le test appelle RightClickOnUnit avec controleur nullptr (non utilise par la regle) ; suppose UHOLFollowBookHolder::Get() non nul dans le monde de test (comme C10a:233).
- bPlayerOrderActive = false : un suiveur peut etre re-ordonne par Follow meme s'il execute un ordre joueur ; GAP : un ordre manuel (Move/Attack du joueur sur un suiveur) n'appelle pas encore HOLFollowOrders::OnManualOrder (a brancher dans les Request* ou OrderAt). Sans cela le Follow continue de re-emettre des Move apres un ordre manuel.
- Un suivi n'est pas arrete si le joueur clique le sol : meme gap.
- Multijoueur client : GetCommands() est le sous-systeme local (comme la table des ordres) : a confirmer.
- EtapeDefini.md : ligne ajoutee. Placeholders : aucun provisoire ajoute.
