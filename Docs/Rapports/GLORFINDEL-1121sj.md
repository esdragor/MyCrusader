# GLORFINDEL-1121sj (2026-11-21, 20:47 -> 20:49)

NON COMPILE : aucun code ecrit, aucun build, aucun quick_check. ARRET aux 25 appels (28 vus par moi : decouverte trop longue, 0 Edit).

APPELS_DECLARES: 28 vus (depasse l'arret a 25 ; premier Edit jamais atteint : regle "avant le 10e appel" NON appliquee)
HEURE_DEBUT/HEURE_FIN: 20:47 / 20:49
PREMIER_BUILD: sans C++ (rien ecrit)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non fait
TEST_ROUGE: aucun (rien commite par moi) + assertit l'effet : non
COMMITS: 0 (seul le merge integ-44 dans ability-columns : fait, a4f3531 en tip), merge_main_fait: non
TESTS: aucun ; TESTS_TAUTOLOGIQUES: 0
TESTS_SCRIPT_AJOUTES: 0 ; BUILDS_DEMANDES: 0
TYPES_A_EXPOSER: voir PLAN ci-dessous (nouveau UHOLFollowBridge C++)
DIAGNOSTICS_AS: non releves
QUESTIONS: aucune

## FAIT
- `git merge integ-44` dans HeroesOfLiberty-Glorfindel : OK (24 fichiers, tip a4f3531 ; contient d0351d6 UHOLFollowBookHolder, 7d5bf9d tests clic droit).
- Decouverte complete du terrain (ci-dessous), pour un agent neuf.

## TERRAIN (file:ligne dans Glorfindel, apres merge)
- Clic droit reel : `HOLPlayerController.cpp` HandleOrder (5071) arme bOrderPressed ; HandleOrderCompleted (5105) appelle OrderAt / OrderLineAt. `OrderAt(ViewportFraction, bQueue)` = 1364. Dans `if (!Own.IsEmpty())` (1410) : bloc ennemi (1422-1432 : FindTargetAt -> UnderId, bDirect, HOLUnitView::Find(*GetWorld(), UnderId, Under), Camps.AreHostile -> RequestAttackUnit), puis bloc soin Moine (1435-1450, ami non neutre + un soigneur selectionne -> RequestHealUnit). Un ALLIE sans soigneur selectionne tombe aujourd'hui dans le chemin sol (Move). C'est la ou brancher Follow : apres le bloc soin (apres la ligne 1450 `}`), meme condition d'allie (`!AreHostile && Under.OwnerPlayer != FHOLMatchConfig::NeutralOwner`).
- Tick du controleur : `AHOLPlayerController::Tick` 2095 (n'a aujourd'hui que SendWatchedLoad cote serveur).
- Patron C++ -> script existant : `Player/HOLWallLineBridge.h/.cpp` (UCLASS(Abstract) UObject, `Get()` cherche la classe script fille via TObjectIterator + CLASS_NewerVersionExists, evenements `BlueprintImplementableEvent BP_X` ; cote script `Script/Buildings/HOLWallLineBridge.as` : `class UHOLWallLineBridgeScript : UHOLWallLineBridge` avec `UFUNCTION(BlueprintOverride)` sans le prefixe BP_). Hors dossiers Dev/Examples/Editor.
- Script : `HOLFollowOrders.as` : `ApplyRightClick(FHOLFollowBook& Book, const TArray<int>& SelectedIds, bool bOnUnit, int TargetId, bool bTargetIsAlly)` (184, rend EHOLRightClickOrder) ; `StepUnit(UObject WorldContext, UHOLCommandSubsystem Commands, int PlayerIndex, FHOLFollowBook& Book, int UnitId, bool bPlayerOrderActive)` (156, false = cible ou unite disparue : fin du Follow) ; `UHOLFollowBookHolder::Get().Book` (UScriptWorldSubsystem, ligne 25). `HOLControllerCommandsC10a.as:229` RightClickStartsFollow existe deja (console seulement).
- NON TROUVE (a lire en premier par l'agent suivant) : comment obtenir un vrai `UHOLCommandSubsystem` en script (les tests utilisent `HOLCommandScript::MakeDetachedForTest()`, HOLCommandLibrary_Test.as:26) : grep `GetWorldSubsystem|UHOLCommandSubsystem` dans Script\Commands\ ; l'indice du joueur depuis le controleur (GetHOLPlayerState()->GetPlayerIndex() cote C++, a passer en argument).

## PLAN (un lot C++, un build, lance par la session principale)
1. Commit 1 (test rouge, script) dans Script\Player\HOLControllerCommandsC10aFollowBridge_Test.as (ou Script\Orders\HOLFollowBridge_Test.as) : `UHOLFollowBridge::Get()` non nul ; `BP_RightClickOnUnit(controller, selected={1,2}, target=9, bAlly=true)` rend true et remplit `UHOLFollowBookHolder::Get().Book` ; ennemi -> false, livre inchange ; cible dans la selection -> false. Stub qui rend false (compile) pour voir le rouge.
2. C++ neuf : `Source\HeroesOfLiberty\Player\HOLFollowBridge.h/.cpp` calque sur HOLWallLineBridge : `UCLASS(Abstract) class UHOLFollowBridge : UObject` ; `static UHOLFollowBridge* Get();` ; `static bool NativeRightClickOnUnit(AHOLPlayerController*, const TArray<int32>& Selected, int32 TargetId, bool bTargetIsAlly);` (Get()->BP_RightClickOnUnit, false sans classe script) ; `static void NativeStep(AHOLPlayerController*);` ; `UFUNCTION(BlueprintImplementableEvent) bool BP_RightClickOnUnit(AHOLPlayerController* Controller, const TArray<int32>& Selected, int32 TargetId, bool bTargetIsAlly);` et `void BP_Step(AHOLPlayerController* Controller, int32 PlayerIndex);`.
3. Script `Script\Orders\HOLFollowBridge.as` : `class UHOLFollowBridgeScript : UHOLFollowBridge` ; `RightClickOnUnit` = `HOLFollowOrders::ApplyRightClick(UHOLFollowBookHolder::Get().Book, Selected, true, TargetId, bTargetIsAlly) == EHOLRightClickOrder::Follow` ; `Step` = pour chaque UnitIds du livre : `StepUnit(Controller, Commands, PlayerIndex, Book, UnitId, false)`, et si false : retirer l'unite du livre (fonction de fin de follow de HOLFollowOrders / HOLFollowRules a nommer apres lecture). Pas de WorldContext implicite (piege 7) ; un seul index par tour (pas de modification du tableau pendant l'iteration : parcourir a l'envers).
4. Commit 2 : dans `OrderAt`, apres le bloc soin (1450) : si allie sous le curseur et `UHOLFollowBridge::NativeRightClickOnUnit(this, SelectedAsInt32, UnderId, true)` : log `Order: %d units follow #%u ...`, `OnOrderGiven.Broadcast(Under.Location); return;` ; l'ennemi garde RequestAttackUnit (inchange), le sol garde son chemin. Dans `Tick` : `if (IsLocalController() && !IsSpectator()) UHOLFollowBridge::NativeStep(this);` (Gap : bPlayerOrderActive passe a false faute d'API connue ; un ordre manuel du joueur doit appeler HOLFollowOrders::OnManualOrder pour un follower : a brancher dans les Request* ou dans OrderAt avant l'envoi).
5. Build : `Tools\build.ps1 -LiveCoding=0` (lot C++ : HOLFollowBridge.*, HOLPlayerController.cpp), puis `quick_check -Parts tests`.

## Rouges attendus nom par nom
Aucun commit de ma part, donc aucun rouge a voir. Tests existants de 7d5bf9d / e861db4 (Test_ControllerCommandsC10aRightClick*) : verts attendus sur le tip.

## NON VERIFIE
Tout (aucun code ecrit). Le plan suppose que le controleur peut etre passe a un BlueprintImplementableEvent (AHOLPlayerController est UCLASS, donc OK) et que PlayerIndex se lit en C++ via GetHOLPlayerState().

PROCESSUS_DETACHES: aucun (aucun processus lance)
ANOMALIES_PROCESSUS: 28 appels vus > 25 (decouverte trop large avant le 1er Edit) ; premier Edit jamais fait ; EtapeDefini.md et Placeholders.txt non touches. Placeholders : aucun provisoire ajoute.
GIT_STATUS_PRINCIPALE: seul ce rapport ajoute par moi ; aucun fichier de code touche dans la copie principale : oui.
