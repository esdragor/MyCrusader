# ELROND-1120pd (2026-11-20, 15:57 -> 16:02, integ-25, script seul)

NON COMPILE. CETTE BRANCHE NE COMPILE QU'AVEC ability-columns (GLORFINDEL-1120pc, 14b8786 / 65ebacd) : elle utilise FHOLBuildingSelView.GarrisonOccupants / GarrisonOccupantTypes, HOLControllerCommand::RequestUngarrisonUnitOf / RequestUngarrisonAllSelected, qui n'existent PAS dans integ-25. Ne pas la fusionner seule dans main ; l'integration reunit les deux.

APPELS_DECLARES: ~22 (arret a 25 vus ; arret dur 16:12 non atteint)
HEURE_DEBUT/HEURE_FIN: 15:57 / 16:02
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: f4ca160 (stub qui compile + 3 tests) ; assertit l'effet : oui (boutons et chemins d'icone calcules par ButtonsFor / UnitIconPath) ; rouge NON VU a l'execution (pas de quick_check)
COMMITS: 2 (f4ca160 test seul, 56c4496 corps + branchement), dernier 16:00, merge_main_fait: oui (Already up to date)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (ButtonsFor passe par ViewFor / HOLGarrisonView::Build, l'icone vient de UnitIconPath)
TESTS_SCRIPT_AJOUTES: 3 (Test_GarrisonFrameUnitIconPathIsTheTypesTexture, Test_GarrisonFrameOccupantsBecomeButtonsWithTheirTypeIcon, Test_GarrisonFrameButtonsTolerateAShortTypeListAndAnotherPlayersBuilding)
BUILDS_DEMANDES: 0 ; TYPES_A_EXPOSER: aucun de plus (voir les noms de Glorfindel)
ESSAIS: aucun (rien a jouer avant fusion + build)
QUESTIONS: aucune

## Rouges attendus (bouchons de f4ca160)
Les 3 tests ci-dessus : UnitIconPath renvoie "" et ButtonsFor renvoie un tableau vide jusqu'a 56c4496. Tests "Short types / other player" : le sous-test "another player's building: 0 bouton" passe deja avec le bouchon, les autres sont rouges.

## FAIT
- HOLGarrisonFrameRules.as : struct FHOLGarrisonButtonView ; UnitIconPath(TypeKey) = /Game/UI/Icons/Units/T_Unit_<Type>.T_Unit_<Type> ("" si cle vide) ; ButtonsFor(View, OccupantTypes) (types en parallele, absent = pas d'icone, libelle = id) ; UngarrisonAllLabel() = "Sortir tous".
- HOLGarrisonFrame.as : Refresh(occupants, types, slots, bOwn, bFinished) ; bouton = UImage (texture trouvee par LoadObject, mise en cache par type, nullptr memorise) ou, sans texture, le libelle (nom du type) ; bouton Sortir tous ; Hide() ; clics -> SendLastOrder -> HOLControllerCommand::RequestUngarrisonUnitOf / RequestUngarrisonAllSelected via Cast<AHOLPlayerController>(Screen.GetOwningPlayer()).
- HOLSelection.as : membre Garrison (UHOLGarrisonFrame), construit a la fin de Build sous la ligne des vies, Refresh(View.GarrisonOccupants, View.GarrisonOccupantTypes, HOLGarrison::SlotsOf(FName(View.TypeKey)), View.bOwn, View.bFinished) dans RefreshBuilding, Hide() des que le chemin n'est plus 1 (batiment seul).
- D874 : la file de production (HOLQueuePanel.as) n'a AUCUNE icone (elle montre des noms) ; aucune texture d'unite n'existe (Content\UI\Icons : Ally, Aura, Banner, CloseRanks, Enemy, HeroEmblem + Talents). Donc chemin d'icone cree ici, repli sur le nom du type, ligne ajoutee a Placeholders.txt.
- Docs : FortificationsInProgress.md (l. 235), EtapeDefini.md (l. 9), Auto_Journey_File.md (l. 88), Placeholders.txt (l. 203), IN_USE.txt libre (CPP: non).

## RESTE
Fusion integ-25 + ability-columns, build C++ groupe, quick_check -Parts tests, case de grille Sortir tous (lot D868, ici un bouton de la ligne), textures T_Unit_*, essai a l'ecran.

## NON VERIFIE
Tout : rien compile, rien lance. Points a risque : (1) View.TypeKey de FHOLBuildingSelView est-il bien le nom de ligne de BuildingTypes.csv attendu par HOLGarrison::SlotsOf(FName) (non lu) ; (2) TMap.Find(Key, OutValue) avec valeur nullptr, SetBrushFromTexture, UHorizontalBox dans un UButton : signatures Angelscript supposees d'apres HOLGameScreenScript.as ; (3) LoadObject sur un chemin absent peut journaliser un avertissement une fois par type (cache) ; (4) DIAGNOSTICS_AS : getDiagnostics VS Code renvoie [] pour HOLGarrisonFrame.as mais le fichier n'est probablement pas analyse (worktree non ouvert) : pas une preuve.

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: hors docs voulus inchange : oui (seuls des .md / .txt de suivi touches ; relance du statut sans fichier de code)
