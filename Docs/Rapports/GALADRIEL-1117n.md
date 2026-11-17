# GALADRIEL-1117n : MIG-C-CommandGrid, 2e lot (bibliothèque étendue, tests script NON portés)

**NON COMPILÉ** : aucun build, aucun quick_check, aucun éditeur. Le C++ ajouté n'a jamais vu de compilateur.

APPELS_DECLARES: ~27 (plafond de 25 atteint avant d'écrire le script de test)
HEURE_DEBUT/HEURE_FIN: 18:05 / 18:08 (Get-Date)
PREMIER_BUILD: sans build   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé
TEST_ROUGE: aucun ; assertit l'effet : sans objet (aucun test écrit)
COMMITS: 1 (555ca10 sur galadriel-tab, 18:07), merge_main_fait: non
TESTS: aucun ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ groupé de la session principale)
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 0 (les 5 restants restent à écrire : UnitOrders, HeroAbilities, Ages, MouseCursor, Research, + 2e moitié de Tables)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: 0 (bibliothèque seulement, ~190 lignes ajoutées)
TYPES_A_EXPOSER: restent FHOLSpecializationView (SetSpecialization, GetOpenSpecializationTier) : aucun test C++ ne les lit, donc non exposés.

## Fait (commit 555ca10)
`HOLCommandGridReadLibrary.h/.cpp` étendus :
- `FHOLGridContextView` : BuildableAlwaysOpen (TArray<bool>), Researches, bDeployables, bDeployedAny, bRageUnit, bPrimaryForum, HeroCooldownSeconds (TArray<int32>, index = numéro d'aptitude), Specialization{Tier,PendingOption,RemainingMs}.
- `FHOLGridCommandView` : Ability (int32), CooldownSeconds, bPack, Tier, Option, RemainingMs. `FHOLFeedbackView` : Block (int32).
- Nouveau `FHOLTechFactsView` (HeroTier, bRequiresDone=true, bBuildingFinished, bResearched, bQueued ; défauts = ceux de HOLTechnology::FFacts).
- Statiques : GetAttackMoveCell / GetStopCell / GetHoldCell / GetDeployCell / GetResearchFirstCell / GetNumAges ; GetHeroAbilityAura / CloseRanks / Banner (numéros), GetHeroAbilityCell(int32), GetRageCell, GetSpecializationCell, GetReportName(CommandView) ; IsAgeOpen, GetAgePanelOpacity ; GetMouseCursor (int32) + GetCursorCrosshairs / GetCursorDefault ; GetBuildingResearches, IsTechnologyInTable, IsTechnologyImplemented, GetTechnologyFoodCost, CheckResearch(Technology, Facts, Stock, Queue, Max) -> FHOLFeedbackView ; GetBlockNone / NotImplemented / AlreadyResearched / AlreadyQueued / TierTooLow / RequiresMissing / NoBuilding.
- Les enums non UENUM (EHOLHeroAbility, EBlock, EMouseCursor) passent en int32 + fonctions de constantes ; aucune règle ajoutée.

## Reste (pour l'agent suivant)
1. Écrire dans `Script\Tests\HOLCommandGrid_Test.as` (même fichier, aux mêmes vecteurs que `HOLCommandGridTests.cpp` L577-924) : Test_CGUnitOrders (L577), Test_CGHeroAbilities (L659), Test_CGAges (L716, avec Nomad : BuildableAlwaysOpen.Add(true) sur la dernière entrée, les autres false), Test_CGMouseCursor (L798), Test_CGResearch (L812). Nota : le test C++ `Tables` a une 2e moitié (Siege workshop L538, Chapel L555 : GetProducesOf(n"SiegeWorkshop") donne A Ram, Z Mangonel ; GetProducesOf(n"Chapel") donne A Monk) que le script n'a pas encore pour la grille de production.
2. Les « tests Specialization / report names / Deploy cell » cités dans la tâche n'existent pas dans `HOLCommandGridTests.cpp` (15 tests : MarketGain, Market, Commands, Feedback, Cycle, Categories, Group, Subgroups, Distribution, Tables, UnitOrders, HeroAbilities, Ages, MouseCursor, Research) : seules les liaisons sont exposées.
3. Session principale : build groupé, puis suspects de compilation : `Out.HeroCooldownSeconds[Ability]` (tableau C), `static_cast<HOLTechnology::EBlock>(uint8)`, `Row->Cost.Food` (FHOLTechnologyRow::Cost supposé FHOLResources), `EMouseCursor::Crosshairs` via ICursor.h, include `Technologies/HOLTechnologyTypes.h` (nom supposé : là où vit UHOLTechnologySettings).
4. Docs non mises à jour par moi (EtapeDefini.md, Auto_Journey_File.md, IN_USE.txt) : plafond d'appels atteint.

## NON VÉRIFIÉ
Tout : compilation, noms d'en-têtes, comportements.

PROCESSUS_DETACHES: aucun (aucun processus lancé)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non relevé ; aucun code touché dans la copie principale.
