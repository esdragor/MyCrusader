NON COMPILÉ
# CELEBORN-1119n3 : morceau 8, hol.Cheat.Specialization (branche cheat-specialization, worktree HeroesOfLiberty-Celeborn)

- Commits : ROUGE 37c8e78 (test + plan en bouchon qui refuse toujours), VERT 95b8288 (plan réel + RPC + pont + C++). Rien compilé, aucun test lancé.
- BUILDS_DEMANDES: 1 (lot C++ groupé). TESTS_SCRIPT_AJOUTES: 1 (Test_ControllerCommandsC2CheatSpecialization). TYPES_A_EXPOSER: aucun (pont UHOLRpcBridgeLibrary::CheatSpecialization, UFUNCTION BlueprintCallable comme CheatTier).
- Rouge attendu sur 37c8e78 : Test_ControllerCommandsC2CheatSpecialization échoue (bValid faux, route vide, 0 entier au lieu de 2, paliers 1 et 3 refusés, usage = texte STUB) ; Test_ControllerCommandsC2SpectatorSendsNothing reste vert (plan invalide, continue).
- Vert attendu sur 95b8288 : le même test passe (route ServerCheatSpecialization, entiers 2 et 1, palier 0 et 4 refusés avec « Usage: hol.Cheat.Specialization <Tier 1..3> <Option> », option manquante refusée).
- Lancer : build lot C++ puis quick_check -Parts tests (script) ; essai réel : hol.Cheat.Specialization 1 1 en partie, journal « Cheat: slot N specialization tier 1 option 1 started and ended at once, tier applied 1 ».

## Lignes ajoutées (git diff --stat)
- Script/Player/HOLControllerCommandsC2_Test.as : +28 (test, commit rouge)
- Script/Player/HOLControllerCommandsC2.as : +2 au rouge (bouchon), puis net +19/-2 au vert : branche PlanOf (usage, Args.Num() < 2, palier hors 1..3 refusé, 2 entiers), entrée dans Names(), branche Send ServerCheatSpecialization, fonction HOLCheatSpecializationCommand et const FConsoleCommand HOLCheatSpecializationConsole.
- Script/Net/HOLRpcPlayer.as : +15 (ServerCheatSpecialization(int Tier, int Option), UFUNCTION(Server), garde HOLRpc::CheatRefusal(!HOLRpcBridge::CheatRefused(this, "specialization"), Sender), journal « RpcCheat: specialization ... »).
- Source/HeroesOfLiberty/Net/HOLScriptRpcPlayerActor.h : +4 ; .cpp : +7 (CheatSpecialization, calque de CheatTier).
- Source/HeroesOfLiberty/Player/HOLPlayerController.h : +3 ; .cpp : +31 (DoCheatSpecialization : IsCheatRefused("specialization"), Find, StartNow comme DoStartSpecialization, si réponse != 0 journal + RefreshView + return, sinon AdvanceNow(Slot, 3600000, true), journal, RefreshView).

## Doutes de compilation
1. HOLRpc::CheatRefusal : signature vue (bool, int) par grep, mais appelée ici hors des RPC existants de la même forme (calque de ServerCheatTechnology : OK a priori).
2. AdvanceNow(Slot, 3600000, true) : 3 600 000 ms supposé ≥ tout délai de palier (30/45/60 s au registre) ; je n'ai pas lu AdvanceSpecialization pour savoir si un dépassement est écrêté ou compté tel quel. La réponse « 0 = Ok » est celle de HOLSpecialization::Ok (HOLSpecializationRules.as:48) ; si StartNow rend NoScript (valeur autre que 0) la triche s'arrête au refus.
3. constexpr int32 local dans une fonction #if !UE_BUILD_SHIPPING : standard, pas de souci attendu.
4. Le test garde ses accès Ints[...] par Ints.Num() > 0 ? ... : -99 (accès hors bornes évité au rouge). Angelscript : const FString locale dans un bloc if de PlanOf nommée CheatUsage, const int CheatTier : noms vérifiés distincts des autres locaux du fichier mais non compilés.
5. Pas de piège « Reliable » : UFUNCTION(Server) nu. Les #include du .cpp du contrôleur sont ceux de DoStartSpecialization (même fichier).
6. Le serveur : DoCheatSpecialization tourne sur le serveur via le pont ; la garde IsCheatRefused y est évaluée (comme DoCheatTier).

## Reste
Compiler, lancer le test (rouge sur 37c8e78 puis vert sur 95b8288), essai réel de la commande. EtapeDefini.md et Auto_Journey_File.md : à mettre à jour par la session principale.
