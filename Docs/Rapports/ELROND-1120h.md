ELROND-1120h : NON COMPILE (C++ : ordre "cette unite sort" + occupants de garnison lisibles du client). Branche integ-25 (integ-28 fusionnee en premier geste).

APPELS_DECLARES: ~30 (au-dela de 25 : le gros du travail etait de lire le code C++ avant d'ecrire)
HEURE_DEBUT/HEURE_FIN: 11:38 / 11:5x
PREMIER_BUILD: sans build   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: f4cf3ca (test Mass HeroesOfLiberty.Garrison.UngarrisonUnit + stub) ; assertit l'effet : oui (unite hors du batiment, de nouveau selectionnable, l'autre reste dedans, liste des occupants) ; rouge NON VU
COMMITS: 2 (f4cf3ca stub+test, ae74671 correctif), dernier 11:48, merge_main_fait: non (integ-28 fusionnee)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ : Tools\build.ps1 -LiveCoding=0 -Wait sur le worktree Elrond, puis quick_check -Parts cpp avec le filtre HeroesOfLiberty.Garrison ; attendu : Garrison.UngarrisonUnit rouge sur f4cf3ca, vert sur ae74671 ; les autres Garrison.* inchanges)
TESTS_SCRIPT_AJOUTES: 0 (l'accesseur est teste dans le test Mass : un AHOLBuilding ne se fait pas naitre en script)
TYPES_A_EXPOSER: AHOLBuilding::GetGarrisonOccupants(TArray<int32>&) (UFUNCTION BlueprintCallable, membre : le script l'appelle sur le batiment, pas de WorldContext) ; UHOLRpcBridgeLibrary::OrderUngarrisonUnit(AActor*, int) (UFUNCTION statique, comme OrderUngarrisonAll). AHOLPlayerController::RequestUngarrisonUnit(uint32) n'est PAS une UFUNCTION : le widget script passe par le RPC ServerOrderUngarrisonUnit via le controleur (a exposer comme RequestUngarrisonAll si le widget l'appelle directement).

FAIT:
- HOLPlayerOrders::UngarrisonUnit(UWorld&, Slot, UnitId) : refuse inconnu / sur le terrain / batiment d'un autre joueur ; sinon UHOLCommandSubsystem::IssueUngarrisonUnitOrder (FHOLGarrisonOrder::ExitUnitId, Building non utilise) ; au pas, UHOLGarrisonSubsystem::ApplyOrders prend le batiment de l'unite (garnison), verifie le proprietaire (D374) et appelle EjectUnit (nouveau ; EjectAll le reutilise, meme point de sortie GetBirthLocation).
- Chaine client : RequestUngarrisonUnit -> HOLRpcScriptRoute::RouteUngarrisonUnit (CallFull, 1 int) -> RPC script ServerOrderUngarrisonUnit(int UnitId) (Script\Net\HOLRpcPlayer.as, refus HOLRpc::UnitOrderRefusal) -> UHOLRpcBridgeLibrary::OrderUngarrisonUnit -> DoUngarrisonUnit.
- Lecture client : RIEN n'etait replique par batiment (le client connait seulement bGarrisoned par unite, GarrisonedWireValue, sans le batiment). Ajout de AHOLBuilding::GarrisonOccupants (UPROPERTY Replicated, COND_OwnerOnly comme ProductionQueue), tenu par UHOLUnitSubsystem::SetGarrison (retrait chez l'ancien batiment, ajout chez le nouveau) et par la destruction d'une unite en garnison ; accesseur UFUNCTION GetGarrisonOccupants(TArray<int32>&). COUT : un int32 par occupant, envoye au proprietaire seulement, a chaque entree/sortie (ForceNetUpdate) ; rien aux autres joueurs.
- Ecart a la consigne : pas d'identifiant entier de batiment dans le code (les ordres passent l'acteur) : l'accesseur est un membre du batiment, pas une fonction (BuildingId, Out).
RESTE: build + tests ; widget (Script\UI, HOLGarrisonView deja la) qui lit GetGarrisonOccupants du batiment selectionne et envoie l'ordre ; exposer l'appel du controleur ; test script de refus RPC si on veut (HOLRpcOrders_Test.as). Auto_Journey_File.md : aucune ligne ELROND-1120h trouvee, pas de ligne modifiee.
NON VERIFIE: tout (compilation, UHT sur la nouvelle UPROPERTY Replicated TArray et le UFUNCTION const a parametre sortant, rouge/vert du test, CallFull remplissant un parametre int du RPC script, FHOLGarrisonOrder initialise par agregat dans IssueGarrisonOrder avec un 5e champ par defaut, fins de ligne du test (converti en CRLF)).
PROCESSUS_DETACHES: voir stray_processes en dernier geste
ANOMALIES_PROCESSUS: premiere passe de patch lancee deux fois (doublon de IssueUngarrisonUnitOrder dans HOLCommandSubsystem.cpp, corrige avant commit)
GIT_STATUS_PRINCIPALE: non relu (docs seuls ecrits : FortificationsInProgress.md)