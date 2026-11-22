# CELEBORN-1122f : F8 T2, premiere fleche de batiment. NON COMPILE

Joueur : une Tour de guet en bois FINIE tire une premiere fleche sur l'ennemi le plus proche a portee (rien d'autre ne change : une seule fleche par batiment, tant que la cadence T4 n'existe pas).
Technique : `UHOLCombatProcessor::ReleaseBuildingArrows`, appelee dans `Execute` juste avant l'atterrissage des fleches ; reutilise `StepBuildings`, `Arrows`, `FArrow` et l'atterrissage existant.

APPELS_DECLARES: 17 (arret a 25 vus ; le seuil se juge sur `tool_uses` de la session principale)
HEURE_DEBUT/HEURE_FIN: 17:45:40 / 17:50 (Get-Date)
PREMIER_BUILD: sans build (consigne : NE COMPILE PAS)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build, consigne)
TEST_ROUGE: 999abc5 (bouchon qui compile + test `HeroesOfLiberty.Combat.BuildingShot`, meme commit), rouge NON VU a l'execution ; assertit l'effet : oui (PV perdus de la cible + fleches lachees)   TEST_ROUGE_SEPARE: oui (999abc5 rouge, d9e41cc vert, 5d6a6de include)
TEST_ROUGE attendu : sur 999abc5 le cas « ennemi a portee » echoue (0 fleche, 0 PV perdu) ; les 3 contre-cas (hors portee, allie, fondation) passent deja (le bouchon ne tire rien).
COMMITS: 3 sur integ-41 (999abc5, d9e41cc, 5d6a6de), dernier 17:49 ; merge_main_fait: non (consigne : pas de fusion ; DLL non compilee)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (la perte de PV et le nombre de fleches viennent de la vraie chaine : processeur de combat reel pas a pas, atterrissage existant ; la borne haute de PV lit la colonne BuildingAttack de la ligne copiee)
ESSAIS: aucun
QUESTIONS: aucune (defauts pris, liste plus bas)
BUILDS_DEMANDES: 1 (lot C++ : `Tools\build.ps1 -LiveCoding=0` dans C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, puis `quick_check -Parts tests`, filtre `HeroesOfLiberty.Combat.BuildingShot`)

## Fait
- `Source\HeroesOfLiberty\Simulation\HOLCombatProcessor.h` : declaration `void ReleaseBuildingArrows(const UWorld&, int64 StepIndex, int32 StepMs)` et `TSet<uint32> BuildingsShot`.
- `Source\HeroesOfLiberty\Simulation\HOLCombatProcessor.cpp` : corps l.838-900 environ ; appel `ReleaseBuildingArrows(*World, StepIndex, StepMs);` l.~1415 juste avant la boucle d'atterrissage « The arrows that land this step » ; include `Buildings/HOLBuildingTypes.h`.
- Regle : pour chaque `FStepBuilding` : ligne `GetType()` avec `BuildingAttack > 0` et `BuildingFiresEmpty`, `!bGone`, `IsFinished()`, pas deja dans `BuildingsShot` ; cible = parmi `Fighters` vivants, camp different (`Candidate.Camp != Step.Camp`) et, en Diplomatie, `StepCamps.FiresAtOnce(proprietaire, victime)` (meme test que `IsAutoFoe` des archers : allie et neutre jamais), a portee `HOLCombat::IsInBuildingShotRange(Step.Box, pos, rayon, BuildingAttackRange)` ; le plus petit ecart de bord, egalite au plus petit `UnitId`. Une `FArrow` : `Target/TargetId` de l'unite, `Shooter` vide, `ShooterId` = identifiant du batiment, `ShooterType = NAME_None`, `ShooterSlot`, `ShooterCamp`, `Damage = float(BuildingAttack)`, `ArrivalStep = StepIndex + GetArrowFlightSteps` (jamais dans le pas meme), `++TotalArrowsReleased`, `BuildingsShot.Add(id)`.
- Degats : la voie d'atterrissage existante des fleches (`Arrow.Damage` plat, reduit seulement par le levier moral `GetDamageTakenPercent` deja lu). T2 : la CIBLE PERD DES PV (pas de crédit, pas d'armure : T3). L'atterrissage gere un `Shooter` vide (`FindFighter` rend INDEX_NONE, gardes `Shooter != INDEX_NONE` l.1475-1486 lus).
- Aucun tir deux fois dans le meme pas : un seul `Arrows.AddDefaulted` par batiment et par pas, et `BuildingsShot` empeche toute 2e fleche jamais.
- Aucune unite ni batiment sans les colonnes touche : la boucle ne lit que `Row->BuildingAttack <= 0` (un entier) puis `continue` ; sortie anticipee `StepBuildings.IsEmpty() || Fighters.IsEmpty()` ; la boucle sur `Fighters` n'est atteinte que par un batiment fini, arme, qui n'a pas encore tire. Les 16 autres lignes (attaque 0) coutent une lecture d'entier par pas et par batiment (acteurs : peu).
- Test neuf `Source\HeroesOfLiberty\Tests\HOLBuildingShotTests.cpp` : un monde par scenario (banc du Shelter de `HOLGarrisonOrderTests.cpp`), ligne copiee de la Maison avec 5 / 1.625 / 3000 / vrai sous le nom `BuildingShotTestTower` ; Spearman en Hold, 10 s de pas : (1) tour finie, ennemi (proprietaire 3) a 1000 cm du bord : 1 fleche, PV perdus > 0 et <= BuildingAttack ; (2) ennemi a 4500 cm du bord : 0 fleche, 0 PV ; (3) allie (proprietaire 0) : 0 / 0 ; (4) fondation (`EHOLConstructionState::Foundation`) : 0 / 0.
- Docs : FortificationsInProgress.md l.246, EtapeDefini.md l.35, Auto_Journey_File.md l.37 (ma ligne), IN_USE.txt libre (worktree).

## Grep L22-3 (TowerWood)
`Tools\Tests\quick_check.ps1` l.4268 : seule occurrence = la chaine du rapport HUD (`F=TowerWood greyed`), sans lien avec le tir ; `Tools\Scenarios\` : aucune occurrence. Rien a ajuster.

## Defauts pris (non ajoutes a la donnee), a confirmer par l'utilisateur
1. Un batiment ne tire qu'UNE fleche en tout (pas de cadence) : provisoire jusqu'a T4 ; le `TSet BuildingsShot` ne se purge pas (T4 le remplace).
2. Cible : unites vivantes seulement (pas de batiments, pas de heros ecartes) ; hostilite = camp different, et en Diplomatie `FiresAtOnce` : un Neutre (camp neutre, `NeutralOwner`) n'est PAS vise en Diplomatie ; hors Diplomatie un proprietaire d'un autre camp quelconque est vise (comme les archers). Un neutre n'est pas teste.
3. Priorite : le plus proche du bord, egalite plus petit UnitId ; cible rechoisie a chaque tir (il n'y en a qu'un).
4. Un seul tireur par tour : flechage par occupant = 1 (non lu, `GarrisonSlots` ignore) ; la tour tire vide (`BuildingFiresEmpty` requis pour ce premier jet, comme 1122d).
5. Degat plat `BuildingAttack` (sans armure : T3), sans bonus de technologie, sans credit (`Shooter` vide : ni `DamageDealt`, ni `LastAttacker`, ni XP).
6. Pas de brouillard (T5), pas d'affichage de la fleche (T7 : un client ne voit pas la fleche, seul le serveur / solo applique le degat), pas d'`UHOLArrowSubsystem::Launch`.
7. Hauteur ignoree ; la fleche part du centre du corps (sans le toit).
8. Une fleche en vol atterrit meme si la tour meurt entre-temps (comme les archers).

## RELECTURE_CPP (L20-2)
Relu apres ecriture (`git diff` + lecture des lignes) : `FArrow` est dans la section `protected` de la classe, la fonction est membre (acces OK) ; `FBox2D::GetCenter()` rend `FVector2D` ; `FMath::Clamp` sur doubles (`FBox2D::Min/Max` en double UE5) ; `GetArrowFlightMilliseconds(const FVector2D&, const FVector2D&, float)` et `GetArrowFlightSteps(int32, int32)` declares dans `HOLCombatRules.h` l.784-807 ; `StepIndex` et `StepMs` definis l.860-861 avant l'appel (l.~1415) ; `Candidate.Constants->Radius` (float, l.343 du cpp) ; `FHOLBuildingTypeRow::BuildingAttack` (int32), `BuildingAttackRange` (float), `BuildingFiresEmpty` (bool) lus dans `HOLBuildingTypes.h` l.174-186 ; include `Buildings/HOLBuildingTypes.h` ajoute (5d6a6de) car `GetType()` n'est lu que par pointeur ailleurs dans ce fichier ; fins de ligne CRLF conservees (cpp, h), test converti en CRLF ; le test emprunte la forme exacte du Shelter (`CallInitialize`, `AdvanceStepIndexForTests`, `ServeBirthRequests`, `Executor::Run`).
Sources de risque de compilation restantes : `Tower->Setup(type, 0, 0, State)` (signature l.71 de `HOLBuilding.h` lue, 4 parametres) ; `Table->RemoveRow(FName)` (API UE standard, non vue dans ce depot) ; `TestEqual(const TCHAR*, int64, int64)` avec `static_cast<int64>` ; `UWorld::CreateWorld` non detruit (`DestroyWorldContext` seulement, comme les tests existants).
Aucun jumeau script (D836/D839 : processeur Mass).

## Reste
- Build puis `quick_check -Parts tests` : rouge attendu sur 999abc5 (cas « a portee »), vert sur d9e41cc.
- T3 (armure de tir + credit au proprietaire, `Shooter` absent), T4 (cadence + purge de `BuildingsShot`), T5 (brouillard), T6 (garnison), T7 (affichage), T8 (cout, scenario).

## NON VERIFIE
Compilation, tous les tests, le rouge et le vert a l'execution ; que le Spearman a l'identifiant multiple de 4 est bien servi par la recherche (copie du banc Shelter) ; que `GetDamageTakenPercent` vaut 0 par defaut (borne haute du test : `HealthLost <= BuildingAttack`) ; la hostilite proprietaire 0 contre 3 hors Diplomatie (copiee du Shelter, Mangonel 3 contre 0) ; que `GetBodyBounds()` d'une Maison finie est non vide ; `Footprint` du Spearman (Radius) ; l'etat `.uasset` de BuildingTypes ne sert pas ici (le test ajoute sa propre ligne).

PROCESSUS_DETACHES: aucun (stray_processes.ps1 -Root HeroesOfLiberty-Celeborn -Stop : « aucun processus detache »)
ANOMALIES_PROCESSUS: aucune ; ecart d'horloge : mon `IN_USE.txt` d'entree portait « 17:52 » (heure estimee a tort), l'heure reelle (Get-Date) etait 17:46
GIT_STATUS_PRINCIPALE: inchangee hors docs voulus : oui (FortificationsInProgress.md, EtapeDefini.md, Auto_Journey_File.md modifies par ma ligne ; le reste etait deja la)
