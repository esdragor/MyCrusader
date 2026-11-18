# SAURON-1118m : UnitTier script lit la colonne MinTier (etapes 2 et 4 de ARWEN-1117h)

NON COMPILE, aucun test lance, BUILDS_DEMANDES: 0. Worktree `HeroesOfLiberty-Sauron`, branche `visual`, un commit (voir `git log`), pas de merge, pas de push. Aucun fichier de `Source\` touche, aucun *InProgress.md / EtapeDefini.md / Auto_Journey_File.md touche.

## Fichiers touches
- `Script\Units\HOLUnitTierRules.as` : `MinTierOf` = `HOLUnitTierRules::GetUnitMinTier(FName(UnitType))`, valeur negative -> `UnknownTier` (-1). `HasRowInUnitTypes` supprimee ; `Check` ne teste plus que `MinTier == UnknownTier` -> `NotListed`. En-tete mis a jour. Plus de table typee.
- `Script\Tests\HOLUnitTierRules_Test.as` : voir ci-dessous.
- `IN_USE.txt` (libre).

## Tests retires (3, tous les appels a `HOLUnitTierRules::CheckUnit` du dossier Script\)
`Test_UnitTierScriptAndSimulationRulesAgree`, `Test_UnitTierAgreesOnEveryRowOfTheTable`, `Test_UnitTierSiegeEnginesLikeCpp`. Grep apres lot : plus aucun `CheckUnit(` de la bibliotheque dans `Script\*.as` (reste `CheckUnitTier`, la methode du pont, sans rapport). Reste `GetUnitMinTier` (dans `MinTierOf`, le nouveau test de colonne et le test des engins).

Assertions des tests retires portees ailleurs, en litteraux :
- 16 lignes listees, Hero palier 0 / King / LightInfantry / Nanite palier 0, Hero disponible au palier 0 : `Test_UnitTierColumnOfEveryRow` (Check au palier propre = Available pour chaque ligne).
- Dragon NotListed : `Test_UnitTierColumnOfEveryRow` (et `ReasonOrder`).
- Trebuchet / Cannon : `GetUnitMinTier == -1` et NotListed aux paliers 0..3 : `Test_UnitTierSiegeEnginesNotListed`.

## Tests ajoutes (2)
- `Test_UnitTierColumnOfEveryRow` : les 16 lignes de UnitTypes.csv avec palier attendu litteral (colonne verifiee par PowerShell Import-Csv : LightInfantry 0, LightInfantryNanite 0, Archer 0, Peasant 0, Zombie 0, Spearman 0, ManAtArms 1, DamnedWarrior 1, King 0, Hero 0, Crossbowman 1, Monk 1, Ram 1, Mangonel 2, LightCavalry 1, Arquebusier 3), compare a `MinTierOf` ET a `GetUnitMinTier`, plus Dragon hors table.
- `Test_UnitTierSiegeEnginesNotListed` : Trebuchet / Cannon.

## Tests existants modifies (conserves)
- `Test_UnitTierTable` : Trebuchet / Cannon attendent `UnknownTier` (plus 3).
- `Test_UnitTierAgreesWithTheSiegeRule` : retrait des 2 assertions `GetTier(Trebuchet/Cannon) == MinTierOf` (ces engins n'ont plus de palier colonne).
- `UnitTierTypes()` : Trebuchet et Cannon retires de la liste (sinon `Test_UnitTierMonotone` voit -1 hors 0..3).
- Commentaire d'`OpensSoldiers` : renvoie a `Test_UnitTierSiegeEnginesNotListed`.

## Mutation pour voir le rouge (session principale)
Dans `Test_UnitTierColumnOfEveryRow`, changer `Expected.Add(2)` de `Mangonel` en `Expected.Add(3)` : rouge sur "Mangonel: the column MinTier" (et "library reads the same column", et "comes out at its own tier" ne casse pas). Variante : changer la cellule MinTier de Mangonel dans `Tools\Data\UnitTypes.csv` (2 -> 3). Remettre apres.

## Verifications des appelants de MinTierOf
- `Script\AI\HOLAIArmy.as:610` (`ChooseComposition`) : meme resultat pour toute ligne du CSV. DIFFERENCE : Trebuchet / Cannon donnaient 3, donnent -1 (donc plus filtres par le palier). Aucune occurrence de Trebuchet / Cannon dans `Script\AI\` ; ils ne sont candidats que s'ils ont une ligne CSV (aucune aujourd'hui) -> comportement inchange en pratique. A surveiller quand leurs lignes arriveront.
- Board (`HOLUnitTierBoardScript.as`) : appelle `HOLUnitTier::Check` : inchange (Trebuchet / Cannon deja NotListed).
- Tests : voir ci-dessus ; 13 autres occurrences dans `HOLUnitTierRules_Test.as` toutes relues (Table, Siege, Monotone, nouveaux).

## Suspects de compilation du script
1. `HOLUnitTierRules::GetUnitMinTier(FName(UnitType))` appelee depuis un namespace script `HOLUnitTier` dans un fichier de `Script\Units\` (les tests l'appelaient deja de `Script\Tests\`, meme binding statique de la bibliotheque).
2. `TArray<int>.Add` / `Expected[Row]` : usage standard, mais non compile ici.
3. `Engines[Engine] + " at tier " + Tier + ": not listed"` : concatenation FString + int, deja utilisee dans l'ancien test retire.
4. Contrat ARWEN-1118m : si CheckUnit C++ est supprime et `GetUnitMinTier` garde tel quel, rien d'autre a faire ; sinon (binding non regenere) `Binds.Cache` du worktree contient encore CheckUnit : pas un probleme tant que plus aucun .as ne l'appelle.

## Commande a lancer (session principale)
Build (apres le lot C++ d'ARWEN-1118m), puis `quick_check -Parts tests` : attendre `UnitTier*` verts (Table, OpensSoldiers, ReasonOrder, AgreesWithTheSiegeRule, Monotone, ColumnOfEveryRow, SiegeEnginesNotListed) et `HeroesOfLiberty.UnitTier.ServerBoard`. Le total quick_check baisse de 3 tests d'accord retires + 2 ajoutes (net -1 test script) : attendu, la regle C++ qu'ils comparaient disparait.

TEST_ROUGE: NON vu (non compile). TESTS_TAUTOLOGIQUES: 0 (le palier attendu est litteral, la valeur testee vient du CSV via la bibliotheque).
