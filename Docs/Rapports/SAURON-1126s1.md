# SAURON-1126s1 : AR6-d, sons restants (ordre, fondation, salve de tour, coup chez un client distant, fin de construction)

HEURE_DEBUT/HEURE_FIN: 10:2x / 10:43
BUILD: PASS (2 builds : rouge 90 s, vert 65 s)   LAUNCHCHECK_DEBUT: binaire périmé (sous-système réécrit) -> build d'abord
TEST_ROUGE: a07d25d (bouchons RaiseOrder / RaisePlacement / GetFirstUnitType, OrderEventFor / OrderSlotFor, LastBuildingType non rempli) ; assertit l'effet : oui (compteurs du sous-système sur le vrai processeur de combat : salves de tour contre flèches libérées ; ordre / pose comptés par type ; type du bâtiment fini lu par ObserveMinorEvents)   TEST_ROUGE_SEPARE: oui ; vu : quick_check tests 2323/2328 (les 4 tests neufs + Test_MinorEventsBuildingFinished étendu)
COMMITS: 5 (rouge a07d25d, vert 23d2e99, docs 735bc80, scénario a35559e, rapport), merge_main_fait: inutile (main db72de8 déjà dans la branche)
TESTS: quick_check -Parts tests 2328/2328 PASS (vert) ; quick_check -Parts net,build PASS (407 s, net 2 clients / siege / signals 3 clients compris)
ESSAI: Tools\Scenarios\order_sound (play.ps1 -Hidden, 23 s) : vrai clic droit -> « Order: 1 units to (-13400, -1004) » -> « Order sound: Move ManAtArms » (même image)
FAIT:
- UHOLCombatSoundSubsystem : OnOrderSound (Kind, TypeName, Point), RaiseOrder / RaisePlacement / GetFirstUnitType, GetEventCountFor(Type, Slot).
- AHOLPlayerController : RaiseOrder à côté des 13 repères d'ordre d'unités (OrderAt, AttackMoveAt, OrderLineAt ; Heal / Follow = Move, DropOff = Gather) ; RaisePlacement dans RequestPlaceBuilding / RequestPlaceWallLine (une fois par ligne). Pas les points de ralliement.
- Salve de tour : HOLCombatProcessor (une fois par salve, au toit), client AHOLBuilding::ShowArrowShot.
- Client distant : UHOLNetUnitDisplayProcessor rejoue la cadence de toute unité Attacking, son à chaque coup rejoué, flèches seulement pour un tireur ; bRequiresGameThreadExecution = true ; HOLInterpolatedInstancesProcessor lit la cadence rejouée pour les tireurs seulement (geste de mêlée chez un client inchangé).
- Fin de construction : FHOLMinorEventsWatch.LastBuildingType, UHOLScriptScreen::GetLastFinishedBuildingType ; HOLGameScreenScript.RefreshMinorAlerts joue le Construction de la fiche à chaque bâtiment fini (compteur propre), sans ping provisoire par-dessus.
- Script : HOLArtSound::OrderEventFor / OrderSlotFor ; HandleOrderSound (journal « Order sound: <Kind> <Type> ») ; ARTISTES.md tableau « quand chaque emplacement sonne » + note agents 13.
NON VÉRIFIÉ: aucun son entendu (aucun DA_Art n'a de son ; les 5 sons d'ordre et BuildingDone de la table sont muets par choix de l'utilisateur) ; son de coup chez un client distant et salve de tour côté client non observés en partie réseau (seulement net,build verts, sans régression) ; fin de construction non jouée en partie.
ANOMALIES_PROCESSUS: une sortie de quick_check tronquée par cut (liste des échecs lue par le compte 2328 - 2323 = 5 tests neufs/étendus).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
