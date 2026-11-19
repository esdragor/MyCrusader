# SAURON-1119ch : classement de HOLStartingHeroTests.cpp (9 tests), worktree HeroesOfLiberty-Sauron, branche starting-hero-world (de integ-20)

FAIT : branche `starting-hero-world` creee depuis integ-20 ; AUCUN fichier de code ecrit, AUCUN commit (rien de portable avec les enveloppes existantes ; pas de test force). IN_USE.txt remis a "libre".

## Classement (9 tests)
| # | Test (HeroesOfLiberty.Match.StartingHero.*) | Exige | Classe |
|---|---|---|---|
| 1 | Location (l.56-102) | FHOLStartRules::GetHeroLocation / GetKingLocation / GetStartingPeasantLocations, rayons des lignes Hero/King/Peasant de UnitTypesTable, UHOLMatchSettings::StartingPeasants. Pas de Mass. | ENVELOPPE A AJOUTER |
| 2 | Count (l.104-169) | ShouldSpawnHero(Config, Slot, bSetting), FHOLMatchConfig::FromJson / MakeDefault, HOLTestHooks::MakeScenarioConfig, bStartingHero du CDO, lecture de Tools/Scenarios/*.json (FindFiles + LoadFileToString). Pas de Mass. | ENVELOPPE A AJOUTER |
| 3 | Spawn (l.171-240) | SpawnKing/SpawnStartingPeasants/SpawnStartingHero + `Units->ServeBirthRequests()` + GetUnitView | MASS, reste C++ (D854) |
| 4 | AtPoint (l.242+) | ServeBirthRequests (l.280) | MASS |
| 5 | NomadSeat (l.311+) | ServeBirthRequests (l.342) | MASS |
| 6 | NomadScatter (l.400+) | ServeBirthRequests (l.439) | MASS |
| 7 | NomadWholeMap (l.512+) | ServeBirthRequests (l.554) | MASS |
| 8 | NomadFirstForum (l.599) | grille des batisseurs + Place | DEJA COUVERT (Test_CGNomadAges, HOLCommandGridKeys_Test.as:247) |
| 9 | NomadReplicated (l.682) | idem, ApplyNomadSeats | DEJA COUVERT |

Chiffres : portables avec les enveloppes existantes = 0 ; avec enveloppe a ajouter = 2 (Location, Count) ; Mass = 5 (Spawn, AtPoint, NomadSeat, NomadScatter, NomadWholeMap) ; deja couverts = 2.

## Pourquoi 0 avec les existantes
- UHOLTestWorldLibrary (HOLTestWorldLibrary.h) n'a aucune fonction de regle de depart (monde, batiments, etat de partie, production, IA, victoire seulement). Location et Count n'ont d'ailleurs pas besoin de monde : ce sont des regles pures de FHOLStartRules, donc il leur faut des bibliotheques *Read*, pas l'enveloppe monde.
- Bibliotheques script existantes : HOLStartingPeasantsRead = GetKingLocation + GetStartingPeasantLocations seulement (HOLStartingPeasantsReadLibrary.h:20-26) ; HOLStartRulesRead = GetMissing + ShouldStart seulement ; HOLMatchConfigRead (Describe, MakeDefaultJson, SetSlotJson...) ne donne ni ShouldSpawnHero ni la lecture scenario. Le morceau « King + Peasants » de Location est deja porte (Test_StartingPeasantsLocations, HOLStartRules_Test.as:46) ; il manque le heros.
- Aucun rayon de corps (FHOLUnitTypeRow::Radius) n'est lisible en script : FHOLSiegeUnitFigures (AreaRadius, SightRadius) et FHOLAITypeFigures n'ont pas Radius.

## TYPES_A_EXPOSER (signatures proposees, C++ de la session principale, lot de wrappers sans regle)
Dans UHOLStartingPeasantsReadLibrary (namespace HOLStartingPeasantsRead) :
- `static FVector GetHeroLocation(const FVector& CentreLocation, float HalfBody)` -> FHOLStartRules::GetHeroLocation
- `static float GetUnitBodyRadius(FName TypeName)` -> Row->Radius du UnitTypesTable, -1 si ligne absente (comme GetRadius du test)
- `static FName GetHeroTypeName()`, `GetKingTypeName()`, `GetPeasantTypeName()` -> FHOLStartRules::HeroTypeName / KingTypeName / PeasantTypeName
- `static int32 GetSettingStartingPeasants()`, `static bool GetSettingStartingHero()` -> CDO de UHOLMatchSettings
Dans UHOLStartRulesReadLibrary (namespace HOLStartRulesRead) :
- `static bool ShouldSpawnHeroJson(const FString& ConfigJson, int32 Slot, bool bSetting)` -> FromJson puis FHOLStartRules::ShouldSpawnHero (false si JSON illisible)
- `static bool ScenarioConfigJson(const FString& ScenarioJson, FString& OutConfigJson)` -> HOLTestHooks::MakeScenarioConfig puis ToJson ; plus `static bool ScenarioHeroCount(const FString& Json)` si on prefere un compte
- `static TArray<FString> ListScenarioFiles()` (Tools/Scenarios/*.json, noms seuls) et `static bool ReadScenarioFile(const FString& Name, FString& OutJson)` (le script n'a pas de lecteur de fichier connu).
Avec ceux-ci, Location et Count se portent entierement (memes vecteurs : demi-corps 600/800, 8 forums a 15000 cm, Num dans {rule, 1, 3, 8}, rayon heros 72, hero Z = centre Z, determinisme ; Count : menu 1, lobby 2, slot 2 ferme 0, regle off 0, scenarios sans "startingHero" 0, VALUE true/1 -> 2, false/0 -> 0).

## Falsification (rouge vu)
Aucun test neuf ecrit : sans objet.

## NON VERIFIE
Rien d'execute (consigne). Le classement vient de la lecture des fichiers (NomadFirstForum / NomadReplicated classes « couverts » sur la foi de l'enonce, leur corps l.599-775 non relu ; AtPoint a Scatter/WholeMap classes Mass par leur appel ServeBirthRequests, corps non lus en entier).

## Champs
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 ; EDITS_REFUSES: 0 ; TESTS_TAUTOLOGIQUES: 0 ; PROCESSUS_DETACHES: aucun lance.