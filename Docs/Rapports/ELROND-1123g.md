# ELROND-1123g (2026-11-23, integ-25, C++ liaisons D839)

Joueur : une selection avec un Moine affichera enfin la cellule Sceau de lumiere ; l'IA reconnait la cavalerie pour la garnison et sait depuis quand un batiment est frappe.

## Fait (commits sur integ-25, rouge = bouchon + test dans le meme commit, jamais lance : un seul build)
- B2 : `HOLCommandGrid::NoteSelectedUnitType(Context, TypeName)` (HOLCommandGrid.h/.cpp) appelee par le controleur (HOLPlayerController.cpp ~2467) pour chaque unite selectionnee ; DamnedWarrior -> bRageUnit, Monk -> bMonkUnit. Rouge 1cc0fc2, vert e166452. Test `HeroesOfLiberty.Player.CommandGrid.SelectionKinds` (Tests/HOLBindingsLotTests.cpp) : monde reel, Monk / DamnedWarrior / Spearman nes dans la table d'unites, HOLUnitView::Find, selection mixte. La boucle du controleur elle-meme n'est pas exercee (pas de PlayerController en test) : NON VERIFIE.
- G1 : `FHOLUnitView::bCavalry` (HOLUnitView.h), rempli depuis la categorie d'armure Cavalry dans HOLUnitSubsystem.cpp (solo/serveur) et HOLUnitReceptionSubsystem.cpp (client) ; HOLAISummary.cpp:61 le lit. Rouge e8aa0ac, vert a1cddb2. Test `HeroesOfLiberty.AI.Summary.GarrisonKindCavalry` (cavalier = 2, engin = 3, + LightCavalry nee en monde -> bCavalry vrai, Spearman faux ; couvre aussi la demande G3 siege). Heros : hors de l'armee de l'IA, non testable ici.
- Lecteur coups : `AHOLBuilding::LastHitTime` existait deja (HOLBuilding.cpp:329, mis a jour dans TakeDamagePoints cote serveur, getter GetLastHitTime) : aucun champ serveur neuf ; expose au resume par `FHOLAIBuilding::SecondsSinceHit` (HOLAISummary.h, rempli dans GatherBuildings, -1 si jamais touche). Rouge df27846, vert 476b896. Test `HeroesOfLiberty.AI.Summary.SecondsSinceHit` (deux Houses en monde, une touchee par TakeDamagePoints(1)). Pas de changement de AHOLBuilding.h (en-tete central intact).

## BUILD
Un seul build `Tools\build.ps1 -LiveCoding=0 -Wait` lance 10:31 (avant la limite 10:36). Resultat : voir fin de ce fichier.

## Tests
Non lances si le build a fini apres 10:42 (arret dur) : commande a lancer : `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Tests\quick_check.ps1 -Parts tests` (filtres : Player.CommandGrid.SelectionKinds, AI.Summary.GarrisonKindCavalry, AI.Summary.SecondsSinceHit).

## RELECTURE_CPP (L20-2)
Diff relu : NoteSelectedUnitType declaree API et definie dans le namespace HOLCommandGrid ; GatherBuildings utilise World.GetTimeSeconds() (UWorld const) ; deux seuls sites remplissent FHOLUnitView (HOLUnitSubsystem.cpp, HOLUnitReceptionSubsystem.cpp) ; FHOLAIBuilding est un USTRUCT ScriptReadWrite : le champ est visible du script sans autre liaison. Risque : la ligne LightCavalry doit porter la categorie Cavalry dans la table (le test le dira).

## NON VERIFIE
Compilation (voir ci-dessous), tests, rouges jamais vus, regle script qui lit SecondsSinceHit (HOLAIGarrisonCall.as, hors tache), cellule LightSeal dans HOLCommandGridRules.as.

## Resultat BUILD (10:44, arret dur atteint)
Build lance 10:31, toujours en cours a 10:44 (log Saved\Logs\build.log, [7/20] Module.HeroesOfLiberty.1.cpp), AUCUN resultat vu : compilation et tests NON VERIFIES. Le build n'a PAS ete tue (stray_processes.ps1 -Stop non lance pour ne pas le perdre ; IN_USE.txt du worktree laisse OCCUPE). A faire : attendre Result: Succeeded/Failed dans build.log (ou la sortie de la tache), puis quick_check -Parts tests, puis liberer IN_USE.txt et lancer stray_processes.ps1 -Stop.
