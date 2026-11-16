# GLORFINDEL-1117b : lot L17-05, bibliotheque de lecture du minimap (S1h)

Commit net-playtest 4efe1f6 "S1h: read library for the minimap rules". Merge main non fait. Aucun build lance.

## Fait
- Source\HeroesOfLibertyUI\HUD\HOLMinimapReadLibrary.h/.cpp (35 + 31 lignes C++, module UI) : UHOLMinimapReadLibrary, namespace script HOLMinimapRead.
  - AlertDiameter(Age, Period, Size) : renvoie UHOLMinimapView::AlertDiameter (membre statique, appelable sans instance : AUCUNE extraction, HOLMinimapView.cpp/.h inchanges, donc pas de HOLMinimapRules.h).
  - GetSourcesSignature(TArray<FVector> Locations, TArray<int32> Remaining) -> int64 : construit des FHOLSource (TypeIndex 0) puis appelle HOLMinimapTerrain::GetSourcesSignature ; uint32 rendu en int64 (0..2^32-1).
  - GetConsoleFloat(FString Name) : valeur du cvar flottant, -1 si absent (lecture des hol.Minimap.AlertShrinkSeconds / AlertSize / AlertSeconds, comme le test C++).
- Script\Tests\HOLS1hLibraries_Test.as (neuf) : Test_MinimapAlertRingReadLibrarySmoke (AlertDiameter 3 vecteurs + 3 cvars), Test_MinimapTerrainSignatureReadLibrarySmoke (lecture double, "gathered from : pas de changement", "used up : change"). Ne compile qu'apres le build (la bibliotheque n'existe pas dans la DLL actuelle).

## Types non exposables
- FHOLSource : struct C++ sans USTRUCT, champs sans UPROPERTY -> remplace par deux tableaux paralleles (Locations, Remaining) ; TypeIndex/Resource/Radius/OwnerSlot ne sont pas lisibles du script (non utiles a la regle : IsStanding = TypeIndex valide ET Remaining > 0).
- TConstArrayView<FHOLSource> et uint32 : non exposables tels quels (vue de tableau, entier non signe) -> TArray et int64.
- Le test C++ ne couvre pas "une source ajoutee" ni "ailleurs" dans la fumee : a porter au lot suivant (le vecteur est faisable avec les tableaux paralleles).

## Suivi mis a jour
Auto_Journey_File.md (ligne L17-05), AngelscriptInProgress.md (1 ligne), EtapeDefini.md (1 ligne), IN_USE.txt (libre, CPP: oui).

## Commande pour la session principale
Build puis quick_check (parties : script + tests UI) : `<worktree>\Tools\build.ps1 -LiveCoding=0 -Wait`, ensuite le port complet de AlertRing et TerrainSources dans Script\Tests (rouge vu puis vert) avant retrait des 2 .cpp de test.

## Champs
- BUILDS_DEMANDES: 1 (a lancer par la session principale)
- TESTS_CPP_RETIRES: 0
- TESTS_SCRIPT_AJOUTES: 2 (Test_MinimapAlertRingReadLibrarySmoke, Test_MinimapTerrainSignatureReadLibrarySmoke ; non compiles)
- LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 ; lignes C++ ajoutees: 66
- TYPES_A_EXPOSER: FHOLSource (contourne par tableaux paralleles)
- TEST_ROUGE: sans objet (lot bibliotheque, port apres build)
- NON VERIFIE : compilation C++ (UFUNCTION int64 / const TArray<FVector>& / FString) et script ; noms exacts des methodes de FUnitTest (utilise seulement AssertTrue) ; Math::Abs en script ; nom du namespace HOLMinimapRead (deduit de la convention HOLHUDLayoutRead).
- PROCESSUS_DETACHES: 0