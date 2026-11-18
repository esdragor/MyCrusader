# ARWEN-1118m : palier d'unite, une seule source (etape 3 de ARWEN-1117h)

**NON COMPILE.** Aucun build, aucun quick_check, aucun editeur. Commit 92be52c sur `d751-root` (worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`), branche prete, pas de merge, pas de push. Aucun fichier de `Script\` touche. `IN_USE.txt` du worktree : libre.

## Point 2 : le pont existe-t-il cote client et en solo ? OUI (par lecture)
- `UHOLUnitTierSubsystem : UWorldSubsystem` (`Source\HeroesOfLiberty\Simulation\HOLUnitTierSubsystem.h:18`, `class ... UHOLUnitTierSubsystem : public UWorldSubsystem`).
- `DoesSupportWorldType` = `Game || PIE` (`HOLUnitTierSubsystem.cpp:58-61`), **aucun test de NetMode ni de role**. Un client reseau, le serveur dedie et le solo sont tous des mondes `Game` (ou `PIE`) : le sous-systeme y est cree.
- `ShouldCreateSubsystem` (`HOLUnitTierSubsystem.cpp:48-56`) refuse seulement la classe abstraite C++ et une classe script remplacee par un rechargement (`CLASS_Abstract | CLASS_NewerVersionExists`), meme patron que Garrison / Camp / Chat.
- Le board script est trouve par `Find` (`HOLUnitTierSubsystem.cpp:10-18`) : `World->GetSubsystemArrayCopy<UHOLUnitTierSubsystem>()[0]`, la sous-classe script `UHOLUnitTierBoardScript` (`Script\Units\HOLUnitTierBoardScript.as`) etant la seule non abstraite. Aucune donnee serveur n'est lue par la regle script (`HOLUnitTier::Check` sur une table de noms, `Script\Units\HOLUnitTierRules.as:24-62`).
- Reserve (non prouvee ici, pas de build) : le script Angelscript est compile dans tout processus `-game` (comme le board du chat, qui sert les clients). Le commentaire `Server and solo only` de l'ancien `HOLUnitTierBoardScript.as` n'est qu'un commentaire, pas une garde. Preuve a faire par la session principale : un client reseau qui grise le Man-at-arms au palier 0 (voir ci-dessous).
- Conclusion : la grille a ete branchee sur le pont (cas 3 de la consigne).

## Fait (C++)
| Fichier | Changement |
|---|---|
| `Simulation\HOLUnitTierRules.h` / `.cpp` | **SUPPRIMES** (`git rm`) : `HOLUnitTier::Check`, `HOLUnitTier::EBlock`, `HOLUnitTier::GetMinTier` (65 lignes) |
| `Simulation\HOLUnitTierSubsystem.h/.cpp` | `namespace HOLUnitTierReason { Available 0, NotListed 1, TierTooLow 2, NoBuilding 3, CannotPay 4 }` (constantes dans l'en-tete du pont) ; `static GetMinTier(FName)` (lecture de la colonne MinTier, deplacee ici) ; repli sans board = **refus** `TierTooLow` + `UE_LOG(LogTemp, Error, ...)` **une seule fois** (`static bool bLogged`), modele Garrison / Camp ; `CheckUnitTier_Implementation` par defaut = refus, plus de regle C++ |
| `Simulation\HOLUnitTierRulesLibrary.h/.cpp` | `CheckUnit` **SUPPRIMEE** ; `GetUnitMinTier` GARDEE (nom, signature, namespace script `HOLUnitTierRules` inchanges), appelle `UHOLUnitTierSubsystem::GetMinTier` |
| `HeroesOfLibertyUI\HUD\HOLGameScreen.cpp:2279-2282` | `bTierBlocked = PlayerState && BoundTableState.IsValid() && PlayerController && UHOLUnitTierSubsystem::CheckUnit(PlayerController->GetWorld(), Command.TypeName, tier, true, true) == HOLUnitTierReason::TierTooLow` ; include change |
| `Buildings\HOLProductionSubsystem.cpp:448-450` | `UHOLUnitTierSubsystem::GetMinTier`, `HOLUnitTierReason::TierTooLow` ; include de `HOLUnitTierRules.h` retire |
| `Simulation\HOLUnitTierBridgeTests.cpp` | boucle finale : table de **litteraux calcules a la main** (16 types x 4 paliers : 0 Available, 1 NotListed, 2 TierTooLow ; Trebuchet / Cannon = 1 partout, Mangonel {2,2,0,0}, Arquebusier {2,2,2,0}, tier 1 {2,0,0,0}, tier 0 {0,0,0,0}) ; plus aucune comparaison a une regle C++ |
| `Tests\HOLUnitTierProductionTests.cpp` | include `HOLUnitTierRules.h` retire (il n'appelait rien de `HOLUnitTier::`) |
| `Simulation\HOLUnitTypes.h:413-414` | commentaire seulement |
| `Tests\HOLTierCheatTests.cpp` | rien a changer (n'incluait pas la regle) |

Stat git : 11 fichiers, +75 / -108 ; **lignes C++ supprimees (net de la regle) : 65 lignes (2 fichiers) + ~20 lignes de repli / CheckUnit bibliotheque** ; LIGNES_CPP_SUPPRIMEES ~85, LIGNES_MIGREES_PAR_LOT 0 (le script existait deja).

## ATTENTION : contrat avec SAURON-1118m
`Script\Tests\HOLUnitTierRules_Test.as` appelle encore `HOLUnitTierRules::CheckUnit` (lignes 168, 182-184, 233, 251, 254 ; `GetUnitMinTier` aux lignes 156, 219, 271 reste valide). **Si SAURON ne retire pas ces appels, le script ne compilera plus** (fonction supprimee) et tous les tests script tomberaient. Ses tests d'accord (`...ScriptAndSimulationRulesAgree`, `...AgreesOnEveryRowOfTheTable`, `...SiegeEnginesLikeCpp`) n'ont plus d'objet : a retirer ou a reecrire sur `GetUnitMinTier` + `HOLUnitTier::Check` seuls. A verifier avant le build.

## Suspects de compilation
1. Unity build : aucun nom de namespace anonyme ajoute ; le `struct FExpected` de `HOLUnitTierBridgeTests.cpp` est local a `RunTest` (pas de collision). `HOLUnitTierReason` est dans l'en-tete du pont (namespace global, nom unique, `grep` : aucun doublon).
2. `HOLGameScreen.cpp` : `PlayerController` doit etre le type `AHOLPlayerController*` deja utilise plus haut dans le meme bloc (`!PlayerController ||`, l. 2271) ; `UHOLUnitTierSubsystem` est `HEROESOFLIBERTY_API` (classe), `HOLUnitTierReason` en `constexpr` d'en-tete, donc visible du module UI.
3. `HOLUnitTierSubsystem.cpp` : nouveaux includes `Engine/DataTable.h`, `Simulation/HOLSimulationSettings.h`, `Simulation/HOLUnitTypes.h` (copie de l'ancien `HOLUnitTierRules.cpp`).
4. `FExpected` : initialisation d'agregat `{TEXT("x"), {0,0,0,0}}` d'un tableau `int32[4]` (devrait passer en C++17).
5. Comportement a connaitre : sans board script (aucun script compile), la grille grise TOUTES les unites (refus), et le serveur refuse tout ordre de production d'unite listee dans `Produces` ; c'est voulu (D836 point 3, pas de repli C++), mais un test C++ de production lance sans script rougirait. Les tests de production (`Production.UnitTier`) supposent donc le script charge, comme les autres ponts.

## Pour voir `UnitTier.ServerBoard` rouge (bouchon, dans le worktree, a remettre ensuite)
Dans `Script\Units\HOLUnitTierBoardScript.as`, remplacer le corps de `CheckUnitTier` par `return HOLUnitTier::Available;` (aucune unite refusee) : les assertions 1 (ManAtArms palier 0 => 2), 4 (Mangonel), 5 (DamnedWarrior), `NotListed`, `NoBuilding`, `CannotPay` et la table litterale passent au rouge. Bouchon inverse (verifie la table) : `return HOLUnitTier::TierTooLow;`. Commande : `HeroesOfLiberty.UnitTier.ServerBoard` (automation) ou `quick_check -Parts tests`, apres `Tools\build.ps1 -LiveCoding=0 -Wait`.

## Verification d'un client reseau (a faire apres build)
Preuve voulue pour le point 2 : sur un client reseau (pas le serveur), le bouton Man-at-arms de la grille est grise a palier 0 et pret a palier 1 (`hol.Cheat.Tier`). Si le sous-systeme n'existait pas cote client, TOUTES les unites seraient grisees (repli = refus + une ligne `UnitTier: no script board` dans le journal client) : c'est le signal d'alarme.

## Champs
BUILDS_DEMANDES: 1 (lot C++ : pont, grille, tests, suppression) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; TYPES_A_EXPOSER: aucun ; LAUNCHCHECK_DEBUT: non fait (pas de build) ; TEST_ROUGE: non vu (non compile) ; PROCESSUS_DETACHES: 0.
