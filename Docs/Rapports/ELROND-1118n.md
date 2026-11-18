# ELROND-1118n (MIG2-S4, reprise 12 du 2026-11-18)

**NON COMPILE, AUCUN test lance.** Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche tiers, base main 4a231ac. Arret a ~19:39 (appels ~21 vus).

## Commits (anglais, sans Co-Authored-By, pas de push)
- `dbc27d7` : liaisons C++ (HOLTechnologyCheckLibrary.h/.cpp) + `Script\Tests\HOLTechnologyTable_Test.as` (8 tests).
- `c035a8f` : `git rm Source\HeroesOfLiberty\Tests\HOLTechnologyTableTests.cpp` (266 lignes), commit SEPARE : a annuler (`git revert c035a8f`) si le build ou le test script echoue.

## Table assertion C++ -> test script
Playable (l. 47) :
| Assertion C++ | Test script |
|---|---|
| 17 jouables | Test_TechnologyTable_Playable (`AssertEquals(17, Playable.Num())`, GetPlayableTechnologies) |
| 4 "Soon" : dans la table, batiment existe, non jouables | idem (ReadAITechnologyTable + GetResearchQueueSize >= 0 + IsPlayable false) |
| chaque attendu jouable / chaque jouable attendu (liste de 17 noms) | idem (Contains dans les deux sens) |
| file de 5 par batiment de recherche | idem (GetResearchQueueSize == 5) |

Rules (l. 110) :
| Assertion C++ | Test script |
|---|---|
| 14 vecteurs Check (Ready, tier, II sans I, batiment, payer, queued, researched, NotImplemented x2, Strict x2, MinHeroLevel 0) | DEJA : Test_TechnologyRulesVectors (HOLTechnologyRules_Test.as), verifie vecteur par vecteur |
| DescribeBlock(NotImplemented) == "soon" | Test_TechnologyTable_DescribeBlock (neuf) |
| MakeCost : cout = ligne, 600 pas, 0 population | Test_TechnologyTable_ResearchQueue (GetResearchCost, cout 100/100/0/0 en litteral) |
| file de 5, 6e refusee, IsQueued, 5 couts pris, Cancel rend, plus en file | idem (HOLProductionRead::TryEnqueue/Cancel existants + IsQueued neuf) |
| recherche sans population, Advance 0 puis 600 | idem (HOLProductionRead::Advance) |
| GetBonusPercent (aucun, Plough I/II total, bois, Pics pierre/or, inconnu) | Test_TechnologyTable_Bonus |
| Brouette carry/speed, ScaleByPercent x6 | Test_TechnologyTable_Bonus |
| GetAttackStat x4, Lames I/II, Empennage | Test_TechnologyTable_AttackLines |
| GetArmorStat x9, ArmorCoversAttack x4, Maille II 2000 (separe en 20 et 0) | Test_TechnologyTable_ArmorLines |
| FHOLTechnologyState Has/Add/Done.Num | Test_TechnologyTable_State (StateHas/StateAdd) |

Tout est couvert : les deux tests C++ retires. Aucun besoin de monde ni de Mass (tables + regles pures + file de production deja liee en script).

## Liaisons ajoutees (raison C++ : liaison a exposer ; wrappers de HOLTechnology::, aucune regle changee)
Dans `UHOLTechnologyCheckLibrary` (namespace script `HOLTechnologyCheck`, existait deja) : DescribeBlock(int), IsPlayable(FName), GetPlayableTechnologies(), GetResearchQueueSize(FName), GetResearchCost(FName, float, FHOLResources&, int&, int&), GetBonusPercent(TArray<FName>, FName), ScaleByPercent, GetAttackStat(int), GetArmorStat(int mask), ArmorCoversAttack(int), IsQueued(FHOLProductionQueueState, FName), StateHas, StateAdd. Le .h inclut `Buildings/HOLProductionReadLibrary.h` (FHOLProductionQueueState). TYPES_A_EXPOSER: aucun autre.

## Chiffres
BUILDS_DEMANDES: 1 (lot C++ de la liaison, par la session principale) ; TESTS_CPP_RETIRES: 2 (HeroesOfLiberty.Technologies.Playable, HeroesOfLiberty.Technologies.Rules) ; TESTS_SCRIPT_AJOUTES: 8 (Test_TechnologyTable_Playable, _DescribeBlock, _ResearchQueue, _Bonus, _AttackLines, _ArmorLines, _State + Test_TechnologyRulesVectors deja la) ; LIGNES_CPP_SUPPRIMEES: 266 ; LIGNES_MIGREES_PAR_LOT: ~266 ; TESTS_TAUTOLOGIQUES: 0 (valeurs de pourcentage / cout / file en litteral, calculees par le code teste). Le total de quick_check passe de -2 C++ a +7 script.

## A lancer (session principale)
1. Build : `<worktree>\Tools\build.ps1 -LiveCoding=0 -Wait`. 2. `quick_check -Parts tests` (les 7 `Test_TechnologyTable_*` doivent apparaitre ; le compte total des tests ne doit pas baisser : -2 +7). 3. Si rouge de compilation script : voir suspects.

## Mutations pour voir rouge (une a la fois, dans le worktree, a remettre)
- Playable : `Tools\Data\Technologies.csv` ligne `PloughI` : `Implemented` True -> False => `Test_TechnologyTable_Playable` rouge (16 au lieu de 17, PloughI non jouable). Ou `Tools\Data\BuildingTypes.csv` : MaxQueue de `Farm` 5 -> 4.
- Rules/DescribeBlock : `Source\HeroesOfLiberty\Technologies\HOLTechnologyRules.cpp` DescribeBlock(NotImplemented) "soon" -> "soon!" => `_DescribeBlock` rouge.
- ResearchQueue : `Technologies.csv` PloughI ResearchTime 30 -> 31 => 620 pas, rouge ; ou Cost Food 100 -> 101.
- Bonus : `Technologies.csv` PloughII Percent 20 -> 25 => `_Bonus` rouge ; ScaleByPercent (HOLTechnologyRules.cpp) : changer l'arrondi.
- AttackLines : MeleeAttackStat `TEXT("MeleeAttack")` -> `TEXT("MeleeAttackX")` dans HOLTechnologyRules.h (ligne `inline const FName MeleeAttackStat`).
- ArmorLines : HOLTechnologyRules.cpp l. 146 `ForgeInfantryArmorCategories` : retirer `EHOLArmorCategory::Ranged` => "archers: Maille" rouge.
- State : FHOLTechnologyState::Add (HOLTechnologyRules.cpp) : toujours `return true` => "added once" rouge.
- Pour les vecteurs Check : mutations deja donnees par ELROND-1116m (Test_TechnologyRulesVectors).
Aucune de ces mutations n'a ete vue (rien compile) : rouge a voir apres build AVANT de garder le commit c035a8f.

## Suspects de compilation
1. `Source\...\HOLTechnologyCheckLibrary.cpp` : `UHOLBuildingSettings::FindType` suppose public (HOLBuildingTypes.h l. 283) ; `UHOLTechnologySettings::FindTechnology/GetTable` (HOLTechnologyTypes.h l. 97-100) supposes publics ; `Building->MaxQueue` (champ de FHOLBuildingTypeRow) ; `FHOLProductionQueueState::Types` lisible. Si FindType est protected : passer par UHOLBuildingTypeLibrary::FindRow.
2. Script : `Soon[Index].ToString()` (FName), `Playable.Contains(FName)`, `n"None"` == NAME_None, `0.05f` litteral, `UPARAM(ref) TArray<FName>&` de StateAdd, refs `FHOLResources&`/`int&` en sortie de GetResearchCost (si AS refuse un ref de sortie non marque, ajouter UPARAM(ref)).
3. Script : `T.AssertEquals("soon", Word, ...)` (FString) ; les resultats de methodes de librairie sont affectes a des variables avant AssertEquals/AssertTrue la ou c'est une methode de classe (les statiques de namespace ont ete laissees en ligne).
4. Fins de ligne : fichiers ecrits en LF (git avertit LF->CRLF), sans effet sur le build ; `eol_check.ps1 -Fix` si besoin.
5. Build unity : aucun namespace anonyme ajoute.
6. Le C++ retire definissait aussi un helper `UE::HeroesOfLiberty::Tests::TechnologyTable::Import` : plus utilise ailleurs (grep fait, aucune autre reference dans le code ; seules des mentions de journaux).

## Docs non touches (consigne)
EtapeDefini.md, Auto_Journey_File.md (ligne MIG2-S4 : a cocher en partie : Technology fait cote HOLTechnologyTableTests), `*InProgress.md`, Placeholders.txt (aucun provisoire ajoute). IN_USE.txt du worktree : "libre".
