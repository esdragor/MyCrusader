# CELEBORN-1119cc : port script de HOLBuildingTests.cpp (Foundation, Selection, Damage)

NON COMPILE. Aucun build, aucun quick_check, aucun editeur lance. Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`, branche `test-world-7`.

## Commit
- `6ac4a89` "Tests: script port of the component-free building tests (Foundation, Selection, Damage) with thin HOLTestWorld wrappers, not compiled" (3 fichiers, +336), pose sur a41c7b8.
- Fichiers : `Source\HeroesOfLiberty\Tests\HOLTestWorldLibrary.h/.cpp` (C++ de liaison, D839 : liaison a exposer), `Script\Tests\HOLBuildingWorld_Test.as` (neuf). `HOLBuilding_Test.as` et `HOLBuildingTests.cpp` non touches (AUCUN test C++ retire).
- BUILDS_DEMANDES: 1 (lot C++ de liaison : `Tools\build.ps1 -LiveCoding=0 -Wait` par la session principale, puis quick_check `-Parts tests`). TESTS_CPP_RETIRES: 0. TESTS_SCRIPT_AJOUTES: 3 (Test_BuildingFoundation, Test_BuildingSelection, Test_BuildingDamage). LIGNES_CPP_SUPPRIMEES: 0.

## Liaison ajoutee (UHOLTestWorldLibrary, toutes enveloppes minces vers la vraie methode d'AHOLBuilding, corps sous WITH_DEV_AUTOMATION_TESTS quand ils touchent aux composants / au monde)
Aucune methode d'AHOLBuilding n'est UFUNCTION (seules 7 UFUNCTION de rappel, lignes 378-396 de HOLBuilding.h), d'ou les enveloppes :
- `void SetBuildingConstructionState(AHOLBuilding*, EHOLConstructionState)` -> SetConstructionState
- `void SetBuildingConstructionWork(AHOLBuilding*, int64)` -> SetConstructionWork
- `void ApplyBuildingAppearance(AHOLBuilding*)` -> ApplyPendingAppearance
- `int32 GetBuildingHealth(const AHOLBuilding*)` -> GetHealth ; `bool IsBuildingHurt` -> IsHurt ; `bool IsBuildingFoundation` -> IsFoundation ; `bool IsBuildingFinished` -> IsFinished ; `bool HasBuildingWorkStarted` -> HasWorkStarted ; `bool IsBuildingSeenHere` -> IsSeenHere
- `bool IsBuildingAlive(const AHOLBuilding*)` -> IsValid
- `int32 CountBuildingMeshPieces(const AHOLBuilding*)` -> GetComponents<UStaticMeshComponent>().Num()
- `int32 DamageBuildingByAttack(AHOLBuilding*, int32 Damage, bool bRanged)` -> UHOLConstructionSubsystem::DamageBuilding(*B, Damage, Ranged|Melee)
- `int32 GetBuildingTypeMaxHealth(FName)`, `int32 GetBuildingTypeRangedArmor(FName)` -> UHOLBuildingSettings::FindType(...)->MaxHealth / RangedArmor (-1 sans ligne)
- `AActor* TraceSelectionRayDown(UHOLTestWorldHandle*, double X, double Y)` -> World->LineTraceSingleByChannel(HOLCollision::Selection) de (X,Y,5000) vers le bas 10000, retourne l'acteur touche ou nullptr
- Includes ajoutes au .cpp (bloc WITH_DEV) : `Components/StaticMeshComponent.h`, `HOLCollision.h`. GetBuildingConstructionWork / GetBuildingTotalWork existaient deja.
- Le .cpp a ete complete par ajout en fin de fichier, fins de ligne CRLF verifiees (0 LF isole).

## Tests script, assertions, test C++ couvert
Test_BuildingFoundation (couvre `Buildings.Foundation`, partie sans controleur) :
- monde pret ; batiment d'un autre joueur spawn (House, owner 1, Foundation) ; IsSeenHere faux ; 0 piece ; le rayon de selection en (100,100) ne touche rien (3 assertions du bloc "another player's foundation" du C++) ; IsFoundation && !IsFinished && !HasWorkStarted ("a foundation" du C++, la lecture ne depend pas du proprietaire).
- Ajouts hors C++ (etat du jeu lu tel quel) : apres SetConstructionState(UnderConstruction) plus fondation ni fini ; TotalWork > 0 ; travail pose = travail lu (Total/2) ; apres Finished, IsFinished vrai.
Test_BuildingSelection (couvre `Buildings.Selection`, 2/2 assertions) : MainCentre fini ; le rayon en (0,0) touche le batiment ; en (2000,0) rien.
Test_BuildingDamage (couvre `Buildings.Damage`, partie sans composants) : ligne House de la table (MaxHealth > 0) ; RangedArmor == 40 ; fondation nue : 1 PV ; a moitie construit : MaxHealth/2 (MaxHealth lu de la table, Total/2 lu du jeu) ; 50 a distance -> 10 infliges (10 en clair comme le C++) ; IsHurt ; fini : MaxHealth - 10 ; coup de 100000 en melee -> MaxHealth - 10 infliges ; batiment detruit (IsBuildingAlive faux).
Un seul spawn : le C++ spawne SpawnActor puis Setup(Type, Owner, Colour, State) ; SpawnBuilding fait Setup(Type, Owner, Owner, State) a l'origine (couleur = proprietaire, aucune assertion portee ne la lit).

## Assertions non portees
- Foundation, batiment du joueur lui-meme (SetOwner(APlayerController local) puis SetAsLocalPlayerController) : non porte : controleur local non expose / pas d'enveloppe de creation. Entrainent : IsSeenHere vrai, 13 pieces d'un fantome, 1 piece non visible, emprise visible, hauteur = House->Height, 0 piece a la couleur pleine du proprietaire, le rayon touche le fantome, 13 pieces / 5 pieces de couleur / Max.Z <= FoundationStakeHeight + 1 d'un chantier, moitie de travail = moitie de hauteur, fini : 4 pieces, hauteur entiere. Raison : bornes des composants (Bounds, IsVisible, materiaux) non exposees.
- UBoxComponent::CanEverAffectNavigation (Foundation x3, Damage x2) : non porte : composant non expose.
- Damage, gravats (TActorIterator<AHOLRubble> : type "House", LifeSpan ~10 s, aucun clic repondu, 1 gravat) : non porte : AHOLRubble sans enveloppe (a ajouter au lot suivant : CountRubbles + GetRubbleTypeName + GetRubbleLifeSpan, header Buildings/HOLRubble.h).
- Foundation, "TestNotNull house in the table" : porte en Damage seulement (MaxHealth > 0) ; la ligne n'est pas exposee.

## Tests de HOLBuildingTests.cpp restants (10 tests)
- Portes en partie (C++ reste en place) : Foundation, Damage. Porte entierement : Selection (a retirer apres rouge puis vert).
- A porter plus tard (si la liaison suit) : Foundation (partie proprietaire + bornes), Damage (gravats, navmesh).
- Restent C++ (GetComponents, maillages instancies, materiaux, bornes) : Appearance, ForumColour, ArcheryRange, FarmField, FarmClick, Walkable, FarmShape. Raison : ils lisent les composants / couleurs / formes. (`Buildings.Types` deja migre dans HOLBuilding_Test.as.)

## FALSIFICATION (rouge a voir, sans rebuild apres le build du lot)
Une fois la liaison compilee, un seul changement par essai dans `Script\Tests\HOLBuildingWorld_Test.as` (hot reload ou rechargement des scripts) :
- ligne 130 : `T.AssertEquals(10, Dealt, ...)` -> `11` : attendu rouge `Test_BuildingDamage` (le jeu rend 50 - 40 = 10).
- ligne 119 : `AssertEquals(40, Armor, ...)` -> `41` : rouge `Test_BuildingDamage`.
- ligne 122 : `AssertEquals(1, Health, ...)` -> `2` : rouge `Test_BuildingDamage`.
- ligne 97 : `T.AssertTrue(Beside == nullptr, ...)` -> `T.AssertTrue(Beside != nullptr, ...)` : rouge `Test_BuildingSelection`.
Remettre la valeur puis revoir le vert avant tout retrait de test C++.

## NON VERIFIE
- RIEN n'est compile : le C++ de liaison (signatures, includes : HOLConstructionSubsystem.h pour EHOLAttackType / DamageBuilding, StaticMeshComponent.h, HOLCollision.h), la visibilite de EHOLConstructionState en parametre de UFUNCTION (deja utilise par SpawnBuilding), le script entier.
- Hypotheses non verifiees : `AHOLBuilding::GetHealth/IsSeenHere/HasWorkStarted` sont const et publics (les en-tetes le disent) ; `Cast<AHOLBuilding>(Hit)` accepte un `AActor` en script ; `Hit == nullptr` sur AActor ; `int64 / 2` et comparaison `int64 == int64` en script ; `AActor Hit` retourne par une UFUNCTION a retour `AActor*` ; `IsValid` apres Destroy via le pointeur script (la destruction par DamageBuilding peut annuler la reference script avant, auquel cas IsBuildingAlive(null) renvoie quand meme faux : correct).
- SpawnBuilding(...,Foundation) : suppose que Setup en monde de test donne les memes etats que SpawnActor + Setup du C++ ; IsSeenHere faux sans controleur local suppose (c'est ce que le C++ asserte pour "Other").
- Le "Damage" script spawne a l'origine ; le C++ aussi (SpawnActor sans transform).
- Test du rayon en (0,0) : suppose la collision du monde de test identique a celle du C++ apres ApplyPendingAppearance.

## Anomalies
- Aucune sur le code. Le `.cpp` de la bibliotheque a ete signale modifie sur disque pendant mon travail (mon ajout) : rien d'autre vu. Fichiers non suivis `Tools\Scenarios\c28_*` et `c31_*` non touches.
- Commande a lancer par la session principale : `powershell -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait` puis `quick_check -Parts tests` (worktree Celeborn).
