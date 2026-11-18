# SAURON-1117a : S1f, bibliotheques de lecture Relay, HeroAbility, Deploy

Commit 823943c sur visual (worktree HeroesOfLiberty-Sauron) : "S1f: read libraries for Relay, HeroAbility, Deploy". Branche prete, C++ NON COMPILE, aucun build, aucun quick_check (consigne). Aucun fichier existant modifie.

## Bibliotheques (C++ : regles qui restent en C++ car lues par Mass / serveur ; D836, D839)
- Simulation\HOLRelayReadLibrary.h/.cpp (95 l.) : script `HOLRelayRead::FillPercent(Owner, Taker, Held, HoldSteps)`, `MakeView(Location, Radius, Owner, Taker, Held, HoldSteps)`. FHOLRelayState (non USTRUCT) passe en 3 int ; FHOLRelayView (USTRUCT() sans BlueprintType, defini dans un fichier existant) remplace par le USTRUCT neuf FHOLRelayReadView (X, Y, Radius, Owner, Taker, FillPercent, Description = Describe()).
- Simulation\HOLHeroAbilityReadLibrary.h/.cpp (106 l.) : `HOLHeroAbilityRead::` GetUnlockLevel, IsUnlocked, GetCooldownMs, CanCast, CanPlantBanner, IsWithinAura, Tick (valeur en entree, nouvelle valeur en retour), GetDamageTakenPercent, GetCloseRanksSpeedPercent, GetReducedDamage, GetHealRatePercent, + GetCloseRanksDurationMs / GetBannerDurationMs / GetMoraleReductionCapPercent (constantes lues par les tests). EHOLHeroAbility est deja un UENUM(BlueprintType).
- Siege\HOLDeployReadLibrary.h/.cpp (252 l.) : `HOLDeployRead::` PhaseAt, DefaultProfile, CanFire, CanMove, CanAct, CanAim, Shortened, DeployDelayMs, PackDelayMs, Advance, Decide, Toggle, Apply, GetTooClose. Types neufs : UENUM EHOLDeployPhase / EHOLDeployOrder / EHOLDeployAction (meme ordre que le C++, conversion par static_cast), USTRUCT FHOLDeployProfile / FHOLDeployState / FHOLDeployDecision.
- Total 453 lignes C++ ajoutees (6 fichiers, commentaires inclus). Wrappers purs, aucune regle copiee.
- Script\Tests\HOLS1fLibraries_Test.as (neuf) : 3 tests de fumee (Test_RelayReadLibrarySmoke, Test_HeroAbilityReadLibrarySmoke, Test_DeployReadLibrarySmoke), chacun sur des vecteurs lus des tests C++ (Ring, Unlock/Cast/Damage, WhoCanDoWhat). Ne compile qu'apres le build ; le chargement des scripts sera rouge d'ici la (attendu, comme S1d).

## Non exposables / ecarts
- FHOLRelayState, FHOLRelayView, HOLDeploy::FProfile/FState/FDecision et les enums du namespace : non exposables tels quels (struct nues ou USTRUCT sans BlueprintType, dans des fichiers que je ne devais pas modifier) -> jumeaux USTRUCT/UENUM minimaux dans les nouvelles bibliotheques (liste ci-dessus). HOLHeroAbility::Tick(int32&) : reference non exposable, remplace par une valeur retournee.
- Constantes constexpr (AuraRadius, BannerPlantRange, etc.) non exposees, sauf les 3 lues par les tests.
- Le test Deploy C++ utilise des helpers (StateOf, MsUntilPhaseChange) : a reecrire en script au port.

## Champs
BUILDS_DEMANDES: 0. TESTS_CPP_RETIRES: 0. TESTS_SCRIPT_AJOUTES: 3 (fumee). LIGNES_CPP_SUPPRIMEES: 0. LIGNES_MIGREES_PAR_LOT: 0 (453 l. C++ ajoutees). TYPES_A_EXPOSER: aucun de plus.

## NON VERIFIE
Rien n'est compile : generated.h, noms de classes/UFUNCTION (`Tick` dans une UCLASS statique, FVector2D en parametre), binding Angelscript des USTRUCT/UENUM neufs (namespace attendu HOLRelayRead / HOLHeroAbilityRead / HOLDeployRead d'apres la convention de S1d), signatures AssertEquals(attendu, reel, msg) du test. Pas de rouge vu (lot sans test porte).

## A lancer par la session principale
`C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\build.ps1 -LiveCoding=0 -Wait` puis quick_check ; ensuite lots suivants : port complet des tests L17-07 (HOLRelayRing_Test.as), L17-08 (HOLHeroAbility_Test.as), L17-10 (HOLDeployRules_Test.as) avec rouge VU, puis retrait des tests C++ et du jumeau HOLRelayRules.as.
Suivi mis a jour : Auto_Journey_File.md (L17-07/08/10 en [~]), AngelscriptInProgress.md, EtapeDefini.md, IN_USE.txt (libre, CPP: oui).