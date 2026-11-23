# ELROND-1123pc (integ-25, tip bbd8947) : NON FAIT, plafond de 25 appels atteint en lecture (15:29)

Pour le joueur : rien de change, le bouton de specialisation reste eteint (ReadView vide). Aucun commit, aucun fichier modifie, IN_USE.txt non touche.

## Constats (lecture seule)
- `AHOLPlayerState::GetSpecialization` EST appelable du script dans le worktree : `UFUNCTION(BlueprintPure)` `const FHOLSpecializationView& GetSpecialization() const` (Source/HeroesOfLiberty/Match/HOLPlayerState.h:137-139). Donc la tache est faisable, mais la lecture complete de ReadView demande 3 autres donnees dont l'accessibilite script est a verifier :
  1. Palier de faction : `AHOLGameState::GetTier(int32 Slot)` est `UFUNCTION(BlueprintCallable)` (HOLGameState.h:306-308), OK. Il faut le SLOT.
  2. Slot du joueur local : `AHOLPlayerState::GetPlayerIndex()` n'est PAS une UFUNCTION (HOLPlayerState.h:73). Dans HOLGameScreenScript.as, `RefreshFactionSlot` (l.2163-2189) a `PlayerSlot = GetLocalSlot()` (l.2165) ; `ReadView(AHOLPlayerController)` n'a pas le slot. Piste : changer la signature en `ReadView(AHOLPlayerController Controller, int Slot)` (une ligne dans `RefreshSpecializationSlot` l.2080 + `Resolve`), a dire au rapport avant d'editer HOLGameScreenScript.as. `GetLocalSlot` n'est pas defini dans ce fichier (grep `int GetLocalSlot()` vide : defini dans la base C++ UHOLScriptScreen ou ailleurs, non verifie).
  3. Faction / nombre de voies : `AHOLGameState::GetSlotFaction(int32 Slot)` est `UFUNCTION(BlueprintCallable)` (HOLGameState.h:612-613) ; `HOLSpecialization::OptionCountOf(EHOLFaction)` est une regle script. OK. Le bouton reste cache pour une faction sans jauge via `HOLFactionSlot` (FactionView.bVisible), comportement inchange.
  4. Forum complet : `AHOLGameState::IsSeatNomad` / `GetNomadForumSeats` NE sont PAS des UFUNCTION (HOLGameState.h:321, 338) ; `UHOLSpecializationSubsystem::Find(World)` est un `static` non-UFUNCTION (HOLSpecializationSubsystem.h:35) et `IsForumComplete` aussi (l.41) ; `SeatHasCompleteForum` est un BlueprintNativeEvent surcharge dans HOLSpecializationBoardScript.as:99 mais on ne peut pas obtenir l'instance depuis ReadView sans `Find`. Faute de liaison : `bForumComplete = true` (vrai pour tout siege non nomade ; un Nomade sans Forum acheve verrait le bouton allume a tort, le serveur refuse avec NoForumComplete = 8).

## Plan pour un agent suivant (sans C++ si on accepte bForumComplete = true)
- Commit 1 (rouge) : test dans Script/Tests/HOLSpecializationSlot_Test.as : une fonction pure `HOLSpecializationSlot::ReadOf(FHOLSpecializationView View, int PlayerTier, EHOLFaction Faction)` -> `FHOLSpecializationRead` (OptionCount = OptionCountOf(Faction), bForumComplete = true) avec bouchon vide ; test : voie choisie palier 1 + progression -> vue ; aucune specialisation + palier 1 -> vue allumee « a choisir ». Le test existant `Test_SpecializationSlotWidgetReadIsDarkUntilBoundAndLitWhenInjected` affirme que ReadView sans controleur est sombre : a garder (controleur null -> vide).
- Commit 2 : `ReadView(Controller, Slot)` : `AHOLPlayerState PS = Cast<AHOLPlayerState>(Controller.PlayerState)` ; `AHOLGameState GS = Cast<AHOLGameState>(Gameplay::GetGameState())` ; View = PS.GetSpecialization() ; PlayerTier = GS.GetTier(Slot) ; Faction = EHOLFaction(GS.GetSlotFaction(Slot)) ; OptionCount = OptionCountOf(Faction).
- TYPES_A_EXPOSER (lot C++ groupe, pour le vrai etat « Forum present ») : `AHOLGameState::IsSeatNomad` et `GetNomadForumSeats` (ou un `IsSeatForumComplete(Slot)` UFUNCTION BlueprintPure), et/ou `UHOLSpecializationSubsystem::Find` en UFUNCTION statique ; `AHOLPlayerState::GetPlayerIndex` en UFUNCTION si on veut lire le slot sans passer par l'ecran.

## Champs
- BUILDS_DEMANDES: 0
- TESTS_SCRIPT_AJOUTES: 0
- TYPES_A_EXPOSER: AHOLGameState::IsSeatNomad, AHOLGameState::GetNomadForumSeats (ou IsSeatForumComplete), UHOLSpecializationSubsystem::Find (static), AHOLPlayerState::GetPlayerIndex
- TEST_ROUGE: non vu (aucun test ecrit) ; TEST_ROUGE_SEPARE: sans objet
- TESTS_TAUTOLOGIQUES: 0
- EDITS_REFUSES: 0
- DIAGNOSTICS_AS: sans objet (aucun fichier modifie)
- PROCESSUS_DETACHES: aucun lance
- integ-25 PRETE a bbd8947 : oui, rien n'y a ete ajoute.
- Fichiers de suivi (ligne 89 de la file, EtapeDefini.md, SpecializationInProgress.md) : NON mis a jour (plafond d'appels) ; la ligne 89 reste `[ ]`.
- Anomalie : mon premier Edit est tombe apres le 8e appel (L20-8 manque) : la tache demandait de lire Slot/tier/forum cote client, ce qui a coute 24 appels de lecture.
