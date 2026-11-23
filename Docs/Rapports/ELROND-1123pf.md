# ELROND-1123pf (integ-25, 15:44 -> 15:52) : lot C++ de liaisons groupe n2, NON COMPILE

Pour le joueur : le bouton de specialisation lira enfin si le Forum du Nomade est acheve (au lieu de s'allumer a tort) ; l'IA de l'Aube aura un test de banniere possible (heros vivant dans le monde de test).

- Worktree HeroesOfLiberty-Elrond, branche integ-25 (base 203e977, main NON fusionne : DLL 11:43 perimee).
- Commit 1 (tests rouges + bouchons qui compilent) : **c489a56**. Commit 2 (corps reels + remplacement du `true`) : **9d682e0**. Les deux finissent par `Agent: ELROND-1123pf`. NOT COMPILED.

## Liaisons livrees (A, B, C : les trois)
- (A) `UHOLSpecializationReadLibrary::IsForumCompleteForSlot(const UObject* WorldContextObject, int32 Slot)` : BlueprintPure, meta WorldContext, script `HOLSpecializationRead::IsForumCompleteForSlot(Slot)`. NOUVELLE bibliotheque `Source/HeroesOfLiberty/Match/HOLSpecializationReadLibrary.h/.cpp`. Corps : `GEngine->GetWorldFromContextObject(..., ReturnNull)` puis `UHOLSpecializationSubsystem::IsForumComplete(World, Slot)` (le calcul du serveur pour NoForumComplete, aucune regle recopiee). `HOLSpecializationSlot.as` : `ReadOf(View, PlayerTier, Faction, bForumComplete)` (4e parametre, plus de `true` en dur dans ReadOf), `ReadView` passe `HOLSpecializationRead::IsForumCompleteForSlot(Slot)`. Commentaire "PROVISIONAL" retire.
- (B) `UHOLTestWorldLibrary::SpawnTestHero(UHOLTestWorldHandle*, int32 Slot, FVector Location)` -> int32 (id de l'unite, 0 si rien) : script `HOLTestWorld::SpawnTestHero`. `BindSteps()` (idempotent) puis unite `Hero` nee pour le slot par la vraie table d'unites (motif de HOLHeroAbilityMatchTests.cpp). Corps sous `WITH_DEV_AUTOMATION_TESTS`, 0 ailleurs. Niveau 4 ensuite par le `HOLTestWorld::SetHeroXP` existant (`HOLHeroXP::GetLevelXP(4)`).
- (C) `UHOLTestWorldLibrary::SetPlayerSpecialization(AHOLPlayerState*, const FHOLSpecializationView&)` -> bool (ce que rend `SetSpecialization` : vrai si la vue a change) : script `HOLTestWorld::SetPlayerSpecialization`, bibliotheque de TEST seulement, corps sous la garde, false ailleurs.
- En-tetes centraux touches : AUCUN (`HOLGameState.h`, `HOLPlayerState.h` intacts). Touches : `Match/HOLSpecializationReadLibrary.h` (neuf), `Tests/HOLTestWorldLibrary.h` (+ `#include "Match/HOLSpecializationView.h"` avant `.generated.h`, + 2 declarations), cpp : `Match/HOLSpecializationReadLibrary.cpp` (neuf), `Tests/HOLTestWorldLibrary.cpp`, `Tests/HOLBindingsLotTests.cpp`. Script : `Script/UI/HOLSpecializationSlot.as`, `Script/Tests/HOLSpecializationSlot_Test.as`. Note : fichier neuf (.h/.cpp) = UBT le ramasse au build ; si la session principale a un projet genere a la main, regenerer.

## Tests rouges (commit c489a56, rouge a VOIR au lancement ; non compile donc non vu)
C++ (`HOLBindingsLotTests.cpp`, nom automation) :
- `HeroesOfLiberty.Specialization.ForumCompleteForSlot` (siege standard vrai ; siege Nomade sans Forum faux ; apres `NoteForumCompleteNow` vrai ; slot negatif et monde nul faux). Bouchon `true` : rouge au 2e controle.
- `HeroesOfLiberty.TestWorld.SpawnTestHero` (id > 0, c'est le heros du slot via `FindHeroUnit`, `PlantBanner` refusee niveau 1 et a 16,5 m, acceptee a 16 m niveau 4 ; slot -1 : 0). Bouchon 0 : rouge.
- `HeroesOfLiberty.TestWorld.SetPlayerSpecialization` (la vue posee se relit par `GetSpecialization` : PendingTier 1, PendingOption 1, RemainingMs 5000 ; retour vrai ; joueur nul faux). Bouchon false : rouge.
Script : `Test_SpecializationSlotReadOfCarriesTheForumFlag` (HOLSpecializationSlot_Test.as) : `ReadOf(..., false).bForumComplete` faux, `Resolve` -> `OpenTier` 0 sans Forum, > 0 avec. Bouchon (ReadOf ignore le 4e parametre) : rouge. Les 4 appels existants de ReadOf passent `true`.

## RELECTURE_CPP (faite a la main, rien compile)
`.generated.h` dernier include des deux `.h` ; `SetNomadNow`, `NoteForumCompleteNow`, `IsForumComplete` (static), `FindHeroUnit`, `RequestUnits`, `ServeBirthRequests`, `GetAllUnitIds`, `PlantBanner`, `SetXP`, `SetSpecialization`, `GetSpecialization` sont tous `public` ; bouchons retires dans le commit 2 ; pas de `case` ; aucun parametre modifie declare `const` a tort (`const FVector Location` par valeur dans le .cpp, `FVector Location` dans le .h : accepte par UHT) ; corps de test sous `WITH_DEV_AUTOMATION_TESTS` (gardes de la bibliotheque de test existante : declarations hors garde comme leurs voisines, corps dedans ; tests dans le bloc `#if WITH_DEV_AUTOMATION_TESTS` existant de HOLBindingsLotTests.cpp). `HOLTestWorldLibNs::HOLTestWorldLibWorldOf` n'est utilise que dans le `#if`.

## A lancer par la session principale
`Tools\build.ps1 -LiveCoding=0 -Wait` dans HeroesOfLiberty-Elrond (integ-25), puis `quick_check -Parts tests` du worktree ; attendu 2144 + 4 tests (dont 3 C++ si le quick_check les lance ; sinon `Automation RunTests HeroesOfLiberty.Specialization.ForumCompleteForSlot+HeroesOfLiberty.TestWorld.SpawnTestHero+HeroesOfLiberty.TestWorld.SetPlayerSpecialization`). Pour voir le rouge : poser c489a56 seul (checkout) avant 9d682e0.

## Champs
- BUILDS_DEMANDES: 1 (ce lot). TESTS_SCRIPT_AJOUTES: 1 (Test_SpecializationSlotReadOfCarriesTheForumFlag) + 3 C++ (lot de liaisons). TYPES_A_EXPOSER: aucun nouveau.
- TEST_ROUGE: non vu (rien compile) ; TEST_ROUGE_SEPARE: oui (c489a56 puis 9d682e0) ; TESTS_TAUTOLOGIQUES: 0. EDITS_REFUSES: 1 (le `git commit -F -` sous PowerShell 5.1, refait avec `-m`).
- DIAGNOSTICS_AS: NON VERIFIE. PROCESSUS_DETACHES: aucun lance (stray_processes non execute). LAUNCHCHECK_DEBUT: sans objet (pas de build).
- merge main non fait, DLL perimee.

## NON VERIFIE / anomalies
- Rien compile ni execute. Que `HOLSpecializationRead::IsForumCompleteForSlot(Slot)` (WorldContext implicite) s'appelle depuis une fonction de `namespace` du script comme `Gameplay::GetGameState()` le fait deja dans `ReadView` (meme mecanisme suppose). Que `Hero` soit bien enregistre dans `HeroUnitBySlot` a la naissance par `ServeBirthRequests` sans processeur Mass (suppose d'apres HOLHeroAbilityMatchTests.cpp : meme sequence). Que le script du test `Resolve(nullptr, ...)` accepte un `nullptr` pour un `AHOLPlayerController`, et que `OpenTier` d'un etat neuf au palier 1 soit > 0.
- Le test script de `PlantHeroBanner` (GLORFINDEL-1123pb, branche ability-columns) n'est pas dans ce worktree : `SpawnTestHero` est livre, mais le test de la banniere de l'IA attend la fusion des deux branches (le test C++ SpawnTestHero couvre `UHOLHeroSubsystem::PlantBanner`, la fonction que `PlantHeroBanner` appelle).
- EtapeDefini.md : « SP3 : bouton permanent » absent, ma ligne est en FIN DE FICHIER (l. 987), a deplacer a la main sous Elrond. Auto_Journey_File.md : ligne 93 apres « SP3 : bouton permanent ». SpecializationInProgress.md : ligne 137 (fin de fichier).
- Placeholders.txt : aucune ligne `bForumComplete` n'existait (jamais ajoutee par ELROND-1123pd) ; rien a retirer, rien d'ajoute.
- IN_USE.txt : OCCUPE puis libre, a la racine du worktree.
