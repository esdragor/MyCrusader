# ARWEN-1116m — séance de migration D830/D835 (2026-11-16 soir), worktree Arwen, branche `d751-root`

Lots : MIG-J-Camp, MIG-J-UnitAbility, MIG-J-Diplomacy (sans C++, aucun build, `Source\` non touché).

```
APPELS_DECLARES: 42 (arrêt à 45 vus par moi)
HEURE_DEBUT/HEURE_FIN: 20:11 / 20:32
PREMIER_BUILD: sans C++   BUILD: sans objet (aucun build demandé)   LAUNCHCHECK_DEBUT: sans C++ (DLL à jour d'après l'énoncé)
TEST_ROUGE: Camp vu à 20:18 (tests 1326/1330 : Test_CampOwner + Test_CampRulesVectors, valeurs faussées 9 et 37) ; UnitAbility vu à 20:20 (unit_ability_accord_test FAIL (1) : MinHp 31 dans une copie de HOLUnitAbilityTable.as) ; Diplomacy vu a 20:27 (tests 1328/1331 : Test_DiplomacyRulesVectors, gagnants faux "0,1,3") ; assertit l'effet : oui (états, parts d'XP, gagnants calculés par les règles)
COMMITS: 4 (5ef9f10, 8fe93e7, 72560c2, 617fe7a docs) + merge 6a4afa1, dernier a 20:31, merge_main_fait: oui (6a4afa1 : main n'apportait que des docs, pas de retest)
TESTS: quick_check -Parts tests : rouge Camp 1326/1330, vert Camp 1328/1330 (20:23) ; -Parts tests,values : rouge Diplomacy 1328/1331 + values PASS (20:27, avec unit_ability_accord_test) ; vert final 1329/1331 (20:31) ; les 2 rouges restants = HeroesOfLiberty.Match.StartingHero.NomadFirstForum, .NomadReplicated (rouges de main, D831) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (lots de règles pures)
QUESTIONS: aucune (le sens des jumeaux est écrit en lignes de bascule, R9)
FAIT / RESTE: voir plus bas
NON VÉRIFIÉ: voir plus bas
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Arwen
ANOMALIES_PROCESSUS: aucune (aucun editeur lance hors quick_check masque)
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, EtapeDefini.md (mes lignes) ; Auto_Journey_Jour.md, ?? autres rapports = autres agents ; inchange hors docs voulus : oui
```

Champs de migration :
```
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0 (aucun : la suppression fait partie de la bascule)
TESTS_SCRIPT_AJOUTES: 3 tests script (Test_CampRulesVectors, Test_CampOwner, Test_DiplomacyRulesVectors) + 1 accord python (unit_ability_accord_test.py)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: Camp 112 l. de test script (135 l. de test C++ portées) ; UnitAbility 99 l. python + 3 l. quick_check (accord de 9 champs, 45 l. C++ couvertes) ; Diplomacy 163 l. de test script (162 l. de test C++ portées)
DUREE_PAR_LOT: Camp 20:11-20:24 (13 min, dont 8 min de quick_check) ; UnitAbility 20:19-20:27 (8 min, en parallèle de la vérif Camp) ; Diplomacy 20:24-20:31 (7 min, dont 7 min de quick_check rouge + vert)
TYPES_A_EXPOSER: Camp (sens B) : HOLCampRules::FSettings, FMember, FIntruder, FStep, FHit, FShare, EAggro, ECredit, EState (aucun n'est USTRUCT/UENUM) ; UnitAbility : aucun en sens A ; en lecture script des constantes C++ : une bibliothèque BlueprintPure (RageDurationMs...) ; Diplomacy : HOLDiplomacy::FBook, FStanceChange, FVisionChange, FAlliedVictory, EResult (non USTRUCT) + une bibliothèque pure (NewBook, NewTeamBook, SetStance, SetVision, Relation, IsAdversary, FiresAtOnce, AlliedVictory) pour un accord par bibliothèque ; HOLDiplomacyReadLibrary existe mais lit l'état du monde, pas la règle pure
```

## Registre des tests (nom à nom)

| Jumeau | Test C++ | Test script en regard | Reste en C++ et pourquoi |
|---|---|---|---|
| Camp | `HeroesOfLiberty.Neutrals.CampRules` (HOLCampRulesTests.cpp) | `Test_CampRulesVectors` (neuf, mêmes vecteurs, dont parts 36/64 sur 110 dégâts) ; déjà là : `Test_CampAggroOnSight`, `Test_CampAggroOnAttackOnly`, `Test_CampLeashAndReturn`, `Test_CampIgnoresWhatItCannotReach`, `Test_CampClearedForGood`, `Test_CampRewardByCredit`, `Test_CampRewardTiesAndNoHit`, `Test_CampSettingsAreCoherent` | - |
| Camp | `HeroesOfLiberty.Neutrals.Owner` | `Test_CampOwner` (neuf : chaque emplacement 0..7 attire le camp et touche sa récompense ; NeutralOwnerSlot = 8) | `FHOLMatchConfig::NeutralOwner/IsNeutralOwner`, `HOLVictoryRules::AreHostile`, `HOLCamps::IsPassive/NoteHit` : C++ sans copie script (sous-système, processeur de combat) |
| UnitAbility | (aucun test pur ; `HOLDamnedWarriorTests.cpp` = Mass) | `unit_ability_accord_test.py` (neuf, 9 champs) ; déjà là : `Test_UnitAbilityEffectIsAMoraleEffect`, `Test_UnitAbilityCanCast`, `Test_UnitAbilityHitCostNeverKills`, `Test_UnitAbilityRageActsOnTheRealMoraleChain` | `HOLDamnedWarriorTests.cpp` (chaîne Mass) ; `GetRageIntervalMs` n'a pas de copie script (la cadence passe par HOLMoraleTable Rate) |
| Diplomacy | `HeroesOfLiberty.Diplomacy.Rules` (HOLDiplomacyRulesTests.cpp) | `Test_DiplomacyRulesVectors` (neuf, assertion par assertion) ; déjà là : 14 tests `Test_Diplomacy*` | - |

## FAIT / RESTE
- Fait : 3 jumeaux couverts sans build (5ef9f10, 8fe93e7, 72560c2), 3 lignes de bascule `[ ]` dans « Lot C++ de fin de séance » (sens proposé A/B, non tranché, R9), quick_check.ps1 lance l'accord UnitAbility (partie values).
- Reste : la décision du sens (utilisateur, R9) ; en sens A pour Camp, retirer la copie script sans build ; aucune autre ligne MIG pour Arwen ce soir.
- Branche `d751-root` prête (pas de fusion dans main).

## NON VÉRIFIÉ
- quick_check -Parts net non lancé (aucun code réseau touché).
- Le rouge UnitAbility a été vu sur une COPIE faussée de HOLUnitAbilityTable.as (--root du dossier temporaire) pour ne pas toucher l'arbre pendant un quick_check en cours.
- Les nombres d'appels par appelant viennent de grep (occurrences, pas d'appels à l'exécution) ; la fréquence de HOLHealProcessor/HOLVisionSubsystem n'est pas mesurée.