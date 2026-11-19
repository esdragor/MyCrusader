# SAURON-1119t : lot G1 (contenu de la grille 3x4, moitié 1) en script, NON COMPILÉ

Joueur : aucun changement (le jeu ne change pas : la règle script n'est branchée nulle part). Technique : règle script + test d'accord contre la bibliothèque C++.

- BRANCHE / WORKTREE : `envelope`, `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, commit `00a2800` (2 fichiers neufs, NON COMPILÉ, non poussé, pas de merge).
- FICHIERS : `Script\Player\HOLCommandGridRules.as` (namespace `HOLCommandGridRules`, 330 l.), `Script\Tests\HOLCommandGridRules_Test.as` (3 tests, 413 l.). `HOLCommandGrid.cpp/.h`, `Test_CG*`, `HOLGameScreen.cpp`, `HOLPlayerController.cpp`, `HOLControllerCommands*.as` NON touchés.
- BUILDS_DEMANDES: 0 (un lot : build + quick_check à lancer par la session principale après le prochain build ; filtre tests script `HOLCommandGridRules` ; le test ne tourne qu'avec la DLL qui porte `UHOLCommandGridReadLibrary` 2e lot et `UHOLHeroAbilityReadLibrary`).
- TESTS_CPP_RETIRES: 0. LIGNES_CPP_SUPPRIMEES: 0.
- TESTS_SCRIPT_AJOUTES: 3 (`Test_CGRAgreementSelections`, `Test_CGRAgreementTables`, `Test_CGRHardValues`) ; noms de test `HeroesOfLiberty.CommandGridRules.AgreementSelections / AgreementTables / HardValues`.
- LIGNES_MIGREES_PAR_LOT: ~200 lignes C++ (cpp 45-237 hors SetSpecialization) -> ~330 l. script (commentaires inclus).
- TEST_ROUGE: NON VU (rien compilé ni lancé). À fausser pour voir le rouge : dans `HOLCommandGridRules.as`, `GetCommand` : `Result.bBuy = Cell < GridColumns;` -> `Cell > GridColumns` (rougissent `AgreementSelections` et `HardValues`) ; ou `HeroAbilityBanner = 2` -> `3` (accord + dur) ; ou `GetRageCell` 2 -> 3 (accord).
- TESTS_TAUTOLOGIQUES: 0 sauf un point : la case de la 1re option de spécialisation lit `HOLCommandGridRead::GetSpecializationCell` des deux côtés (c'est le cvar `hol.UI.SpecializationCell`, un réglage) ; `HardValues` garde des valeurs en dur pour les autres cases.
- EDITS_REFUSES: 0. PROCESSUS_DETACHES: voir plus bas.

## Test d'accord (ce qu'il balaie)
- `AgreementSelections` : 4096 sélections (12 booléens par bits : bâtiment à soi, fini, groupe, forum tombé, Marché, forum principal, bâtisseurs, unités à soi, soldats, rois seuls, héros, rage ; autres faits par restes : déploiement, niveau de héros 1/4, âge atteint 0-3, spécialisation palier/option/ms, Produces 0/2/8 types, Researches 0/2/5, recharges héros dont négative, bâtissables de la table, bAlwaysOpen) x cases -1..12 : `GetCommand`, `GetOrder`, `GetReportName` ; `IsBuildGrid` ; sur les grilles de bâtisseurs : `IsAgeReached`, `IsAgeOpen`, `GetAgePanelOpacity`, `GetAgeBuilding` pour âges -1..4 x cases -1..12.
- `AgreementTables` : grille des bâtisseurs de la table x 4 âges atteints x 6 âges x 14 cases ; chaque type de bâtiment de la table (`WithTableBuildables`) x 8 variantes (site/fini, forum principal, Marché) avec son `GetProducesOf` et son `GetBuildingResearches` ; échanges (4 biens x achat/vente : `GetTradeCell`, `GetTradeGain`, `GetTradeGainText`) ; `GetCategoryRow` x3, `GetHeroAbilityCell` -1..3, `GetRageCell`, curseur (4 combinaisons), constantes.
- `HardValues` : Marché (A achat Food, S vente Wood, R et W vides, cases d'échange, gain texte `+12`, logo Or), chantier (V annule, pas pour un groupe), héros (niv. 1 : cri prêt, bannière grisée ; niv. 4 : recharge 42 s), rage case E, ordres Q S V D (D remballe), rois seuls, spécialisation (A démarre, B en cours annule avec 5000 ms, noms `SpecializeT2A`, `CancelSpecializationT2B`), curseur, âges (opacité 0.55 / 1).

## Fonctions portées (cpp 45-237)
`GetReportName`, `GetCommand`, `GetRageCell`, `GetHeroAbilityCell`, `GetTradeCell`, `GetTradeGain`, `GetTradeGainText`, `GetOrder`, `IsBuildGrid`, `IsAgeReached`, `IsAgeOpen`, `GetAgePanelOpacity`, `GetCategoryRow`, `GetAgeBuilding`, constantes de la grille (accesseurs `GetNumCells()` etc.). `GetMouseCursor` : portée sous la forme `IsAttackCursor(bAttackMoveArmed, bPlacing) -> bool` (le pont fait true = Crosshairs, false = Default).

## Fonctions NON portées
- `SetSpecialization` et `GetOpenSpecializationTier` (cpp 22-43) : dépendent de `FHOLSpecializationView` (non exposé).
- Seconde moitié (lots G2a / G2b) : `PressKey`, `ClickAge`, `FindMissingResource`, `Check*`, `Blocks`, `IsPopulationBlocked`, `Describe`, `GetCellFace`, `IsSelectedByCategory`, `IsRepeatPress`, `NextInCycle`, `GetCycleRank`, `IsAloneInGroup`, `SplitSubgroups`, `GetCommandPriority`, `MergeCommands`, `PickEvenly`.

## TYPES_A_EXPOSER
- `FHOLSpecializationView` (USTRUCT BlueprintType) : pour porter `SetSpecialization` / `GetOpenSpecializationTier`.
- Le cvar `hol.UI.SpecializationCell` est enregistré en C++ (`HOLCommandGrid.cpp:11`) et lu par le script via `HOLCommandGridRead::GetSpecializationCell` : à la suppression du C++, le déclarer côté script (`Console::Variable`) ou le garder dans une liaison de 3 lignes.
- Pour le retrait du C++ (lot ultérieur) : le pont (`UHOLCommandGridSubsystem` ou équivalent) convertit `FHOLGridContext` <-> `FHOLGridContextView` ; `FHOLGridCommandView` rend `Ability` en entier (Aura 0, CloseRanks 1, Banner 2, l'ordre de l'énum) et le script en dépend (testé dans `HardValues` via `GetHeroAbilityCloseRanks/Banner`).

## NON VÉRIFIÉ (rien compilé)
- Tout : syntaxe et API script. Points les plus risqués : `EHOLHeroAbility(Ability)` (cast entier -> enum ; l'en-tête `HOLHeroAbilityReadLibrary.h` dit UENUM exposé, mais `HOLCommandGridReadLibrary.h:166` dit « the enum is not a UENUM » : à trancher à la compilation ; repli : table de déblocage par niveau lue par `HOLHeroAbilityRead::GetUnlockLevel`) ; `HOLHeroAbilityRead::IsUnlocked` (nom de namespace d'après le commentaire de l'en-tête) ; chaînes `f"..."` et `FName(FString)` dans `GetReportName` ; `FString.IsEmpty()`, `Math::IsNearlyEqual` dans le test.
- Durée du balayage (4096 sélections x ~14 cases x 3 fonctions + âges : ordre de 10^5 à 10^6 appels de pont) : peut être lent ; réduire le masque à 10 bits si le test dépasse sa limite.
- Un contexte avec `HeroCooldownSeconds` plus long que 3 : le script lit l'indice demandé (<Num) ; le C++ copie les 3 premiers : identiques pour les indices 1 et 2.
- `ReachedAge` hors 0-3 non balayé.

## Anomalies
- Appels : j'ai dépassé le 8e appel avant d'écrire le premier fichier (lecture du C++, des vues, des tests existants : 16 appels). Pas d'autre écart ; pas de `IN_USE.txt` touché (non demandé).
- Le plan G1-PLAN-1119 suggère `Script\Player\` : utilisé tel quel (le dossier existe).

## Commande à lancer (session principale, après build de l'enveloppe)
`Tools\Tests\quick_check.ps1 -Parts script` (filtre `HOLCommandGridRules`), puis fausser une valeur, relancer, vérifier le rouge, remettre.
