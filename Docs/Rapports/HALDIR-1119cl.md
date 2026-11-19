# HALDIR-1119cl : rangee de recherche, une case par ligne (D856)

NON COMPILE, NON TESTE (aucun build, aucun quick_check, aucun editeur : consigne). Branche `forge-lines` (de `integ-21`), commit 30632dc, non pousse, non fusionne.

## Impact joueur
Forge : W = Lames affutees (mele), X = Empennage (distance), C = Maille (armure d'infanterie), V = Barde (armure de cavalerie). Chaque case montre le prochain niveau a rechercher de sa ligne (I, puis II une fois I fait) ; une ligne finie garde son dernier niveau (affiche fait). Avant : W Lames I, X Lames II, C Empennage I, V Empennage II, et les deux armures sans case.

Autres batiments dont la rangee change (Requires non vide dans Technologies.csv) :
- LumberCamp : DoubleAxes I <- II (avant 2 cases W X, maintenant 1 case W)
- MiningCamp : MinerPicks I <- II (idem, 1 case W)
- Farm : Plough I <- II, Wheelbarrow I <- II (avant W X C V = Plough I, II, Wheelbarrow I, II ; maintenant W = Plough, X = Wheelbarrow, C et V vides)
Les batiments a technologies sans Requires (MainCentre, Academy, SiegeWorkshop, Sanctuary, Barracks, Chapel) ne changent pas.

## Mise en oeuvre (CPP: oui, liaison seule, aucune regle en C++)
- Regle (script, source unique) : `Script\Player\HOLCommandGridRules.as`, nouvelle fonction `GetResearchLineCells(Researches, Researched)` : lignes = technologies sans Requires, dans l'ordre de la table ; pour chacune on avance tant que le niveau montre est dans `Researched` et qu'une technologie le a pour Requires ; `GetCommand` lit `ResearchCells[Cell - 8]` (cap a NumCells). Les niveaux ne sont pas supposes dans l'ordre de la table.
- Liaison C++ : champ `Researched` (TArray<FName>) dans `FHOLGridContext` (HOLCommandGrid.h) et `FHOLGridContextView` (HOLCommandGridReadLibrary.h) + copie dans les deux conversions (HOLCommandGridBoard.cpp:64, HOLCommandGridReadLibrary.cpp:27) ; rempli dans `HOLPlayerController.cpp` aux deux endroits qui remplissent `Researches` (batiment seul et groupe) par `HOLPlayerState->GetTechnologies().Done` ; lecture `HOLCommandGridRead::GetTechnologyRequires(FName)` (FindTechnology, None si inconnue, sans log).
- Code qui supposait "case 8+i = Researches[i]" : seule la regle ; le test `Test_CGSWCommandCells` (Sweeps_Test, ligne ~254) reste valable en testant `Researches.Contains(Cmd.TypeName)` ; HUD/touches/Describe lisent `Command.TypeName` de la case, pas l'index (grep `Researches[`, `ResearchFirstCell` : rien d'autre).
- `Researched` ne contient que ce qui est recherche (pas la file) : une technologie en file reste montree sur sa case (bloc "deja en file" de CheckResearch), comme avant.

## Tests (script)
- `Test_CGSWResearchRowFits` (HOLCommandGridSweeps_Test.as) adapte a D856 : nombre de LIGNES <= 4 par batiment ; cellules de recherche = nombre de lignes ; ATTEIGNABILITE : on recherche ce que montrent les cases, tour apres tour, et chaque technologie du batiment doit etre vue sur une case. Retiree de `Tools\Tests\known_reds.txt` ; `known_reds_test.ps1` : tout PASS.
- `Test_CGForgeLines` (neuf, valeurs en clair) : rien -> MeleeAttackI, RangedAttackI, InfantryArmorI, CavalryArmorI ; MeleeAttackI faite -> W = MeleeAttackII, autres inchanges ; les deux niveaux de mele -> W reste MeleeAttackII ; InfantryArmorI faite -> C = InfantryArmorII ; pas de 5e case.
- Attentes mises a jour (D856) : `Test_CGResearch` (HOLCommandGrid_Test.as, Farm : W = PloughI, X = WheelbarrowI, C V vides), `Test_CGSWCommandCells`.
- Tests C++ `Research` : aucune lecture de `Researches` dans `Source\...\Tests` (grep).

## ROUGE A VOIR (je ne peux pas executer)
Dans `GetResearchLineCells`, remplacer `if (!Researched.Contains(Shown)) { break; }` par `if (true) { break; }` (ou supprimer l'avance) : `Test_CGForgeLines` rouge a l'etape "MeleeAttackI researched" (W = MeleeAttackI au lieu de MeleeAttackII) et `Test_CGSWResearchRowFits` rouge (MeleeAttackII jamais montree). Remettre apres.

## A lancer par la session principale
`Tools\build.ps1 -LiveCoding=0 -Wait` dans `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` (lot C++ : 5 fichiers), puis `quick_check -Parts tests` (CommandGrid*). Risques non verifies : syntaxe Angelscript (ternaire sur n"" evite, `FName NoName` compare avec `!=`/`==`, `Seen.AddUnique`, param `const FHOLGridContextView&` dans l'aide de test) ; si `HOLPlayerState` non visible dans la branche groupe du controleur (il est declare avant le `if`, ligne ~2210).

## Champs
BUILDS_DEMANDES: 1 (lot C++ de 5 fichiers, par la session principale) ; TESTS_SCRIPT_AJOUTES: 1 (Test_CGForgeLines) ; TESTS_CPP_RETIRES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 ; PROCESSUS_DETACHES: 0 (aucun lance) ; NON VERIFIE : tout (rien execute).
Appels vus : ~14. Lance 18:2x, fini 18:2x.
