# GALADRIEL-1117o : MIG-C-CommandGrid, 3e lot (tests script portes, NON COMPILE)

**NON COMPILE** : aucun build, aucun quick_check, aucun editeur. Le script n'a jamais ete charge par le plugin Angelscript ; la bibliotheque C++ (555ca10) non plus.

APPELS_DECLARES: ~12 (arret a 25 vus ; heure d'arret 18:30 non atteinte)
HEURE_DEBUT/HEURE_FIN: 18:07 / ~18:12 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun (script seul, non compile) ; assertit l'effet : oui (valeurs calculees par les regles C++ via la bibliotheque, aucune recopiee)
COMMITS: 1 (320a97e sur galadriel-tab, ~18:10), merge_main_fait: non (DLL perimee, la session principale synchronise)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 0 (le lot C++ groupe de 1117n reste a compiler une fois)
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 5 + 1 demi (Test_CGUnitOrders, Test_CGHeroAbilities, Test_CGAges, Test_CGMouseCursor, Test_CGResearch ; Test_CGTables complete : Siege workshop + Chapel) -> 15 sur 15
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: ~420 lignes de script ajoutees dans Script\Tests\HOLCommandGrid_Test.as
TYPES_A_EXPOSER: aucun pour les 15 tests ; FHOLSpecializationView (SetSpecialization, GetOpenSpecializationTier) reste non expose (aucun test C++ ne le lit). Aucune assertion n'a exige un type absent.

## Fait (commit 320a97e)
Script\Tests\HOLCommandGrid_Test.as, memes vecteurs que HOLCommandGridTests.cpp L538-924, un par un :
- Tables : 2e moitie (SiegeWorkshop : A Ram, Z Mangonel, reste vide ; Chapel : A Monk, reste vide) via GetProducesOf.
- UnitOrders (Q S V, Soldiers / Peasants / Mixed / Enemy / Barracks fini ou chantier), HeroAbilities (cellules par numero d'aptitude, niveau 1 puis 4, recharge 42 s : HeroCooldownSeconds rempli a 3 entrees avant d'ecrire l'index), Ages (Nomad : BuildableAlwaysOpen = 4 false puis true sur la derniere entree), MouseCursor (via GetCursorCrosshairs / GetCursorDefault), Research (CheckResearch avec FHOLTechFactsView, blocs compares aux GetBlock*, Describe pour les deux textes, boucle ImprovedAlarmBell / TemperedHalberds).
- Aides : CGNamesEqual, CGKeyIs, CGKeyBuild (comparaison de FHOLGridKeyResultView champ a champ, TypeName vide pour Key{Target, Age}).
- Pieges du brief respectes : resultat de chaque appel assigne a une variable locale avant AssertEquals quand c'est une comparaison de blocs / curseurs ; aucun conteneur imbrique ; pas de constante partagee entre fichiers.

## Reste (session principale)
1. Un build groupe du C++ de 1117n (suspects : `Out.HeroCooldownSeconds[Ability]`, `static_cast<HOLTechnology::EBlock>(uint8)`, `Row->Cost.Food`, `EMouseCursor::Crosshairs` via ICursor.h, include `Technologies/HOLTechnologyTypes.h`).
2. `quick_check -Parts Script` : si un Test_CG* ne compile pas, suspects : `FName NoName;` (CGKeyIs), `float == 1.0` (GetAgePanelOpacity renvoie un float C++), `FHOLResources` passe par valeur a une fonction de bibliotheque (const ref), comparaison `bool == bool` (`bReached == (Age == 0)`).
3. Rouge vu (fausser une valeur, ex. `GetMouseCursor` attendu), puis vert, puis retrait des 5 tests C++ portes (UnitOrders, HeroAbilities, Ages, MouseCursor, Research) et de la 2e moitie de Tables.

## NON VERIFIE
Tout : compilation script et C++, noms exposes (HOLCommandGridRead::*), valeurs des vecteurs (transcrites a la main depuis le C++).

PROCESSUS_DETACHES: aucun (aucun processus lance)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non relevé ; aucun code touche dans la copie principale (seuls EtapeDefini.md, Auto_Journey_File.md, ce rapport).
