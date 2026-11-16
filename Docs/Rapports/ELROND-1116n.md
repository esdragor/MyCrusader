# ELROND-1116n : D836 point 6 (palier 0) fait, point 2 (Technologies) non commencé (worktree HeroesOfLiberty-Elrond, branche `tiers`, sans C++)

APPELS_DECLARES: 14 (arrêt avancé par la consigne « 15 min au plus » du coordinateur, reçue à 21:35)
HEURE_DEBUT/HEURE_FIN: 21:29 / 21:37
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun build, `Source\` non touché)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: vu à 21:32 : quick_check tests 1351/1354, rouge nommé `Angelscript.UnitTests.Tests.HOLUnitTierRules_Test.Test_UnitTierAgreesOnEveryRowOfTheTable` (+ les 2 rouges connus D831) ; rouge non commité (même tour, le classifieur refuse le rouge seul), vert commité bd64095 ; assertit l'effet : oui (la raison du script est comparée à celle de la vraie bibliothèque C++ `HOLUnitTierRules::CheckUnit` sur la colonne MinTier, pour les 16 lignes)
TEST_ROUGE_VU: oui
COMMITS: 3 (bd64095 correctif + test, 84962a1 journal, cb9f1e5 merge main), dernier à 21:36, merge_main_fait: oui
TESTS: quick_check -Parts tests : rouge 1351/1354 (21:32), vert 1352/1354 (21:35) ; seuls rouges = `Match.StartingHero.NomadFirstForum` et `.NomadReplicated` (D831, rouge connu ; quick_check les dit « REGRESSION : pas dans known_reds.txt », ils n'y sont pas listés) ; total 1354 inchangé ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun ; LOGIQUE SEULEMENT
QUESTIONS: aucune
EDITS_REFUSES: 0
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 0 (5 assertions ajoutées dans Test_UnitTierAgreesOnEveryRowOfTheTable)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: 0 (lot d'accord : +3 lignes dans `Script\Units\HOLUnitTierRules.as`)
TYPES_A_EXPOSER: aucun

## Fait (D836 point 6)
- `Script\Units\HOLUnitTierRules.as` : `HOLUnitTier::MinTierOf` donne 0 au Hero, au King, à LightInfantry et à LightInfantryNanite, comme la colonne MinTier de `Tools\Data\UnitTypes.csv` ; en-tête mis à jour (seul un type hors table est encore NotListed).
- `Script\Tests\HOLUnitTierRules_Test.as`, test `Test_UnitTierAgreesOnEveryRowOfTheTable` : Listed 12 -> 16, LeftOut 4 -> 0, 4 assertions « palier 0 » + 1 assertion « le héros au palier 0 : même raison que le C++ ». Commentaire mis à jour.
- Effet en jeu : aucun aujourd'hui. `Script\AI\HOLAIArmy.as` l. 610 lisait -1 pour ces types, il lit maintenant 0 ; `> Tier` reste faux dans les deux cas.

## Registre
| Test | Statut |
|---|---|
| Test_UnitTierAgreesOnEveryRowOfTheTable | modifié (assertion NotListed -> palier 0), vu rouge puis vert |
| Tests retirés / ajoutés / redirigés | aucun |
| Lignes script supprimées | 0 |

## Non fait (D836 point 2, Technologies)
Non commencé : la consigne de l'utilisateur relayée à 21:35 limite chaque agent à 15 min. Le geste exact figure dans la ligne `[ ]` MIG-CPP-Technology d'`Auto_Journey_File.md` :
- appelants à rediriger vers `HOLTechnologyCheck::Check` (même signature que `CheckImplemented`) : `HOLAITech.as` l. 144 et `HOLForge_Test.as` l. 69-75 et 92 ;
- garder les constantes de raison 0-8 (la bibliothèque C++ n'expose que `Check`) ;
- rediriger les 7 tests de règle et les vecteurs vers le C++ ;
- retirer `Test_TechnologyScriptAndSimulationRulesAgree`.

FAIT / RESTE: D836 point 6 fait (bd64095). Branche `tiers` à jour avec main (cb9f1e5) : **branche prête**. IN_USE libre. Reste : MIG-CPP-Technology (ligne `[ ]` précise) ; bascule UnitTier vers le script après le socle (ligne `[ ]` mise à jour).
NON VÉRIFIÉ: aucun quick_check après le dernier `git merge main` (cb9f1e5). Le merge a apporté au moins `Retours/Q-migration-r9.md` ; je n'ai pas vérifié qu'il n'apportait que des docs. Le message exact des assertions rouges n'a pas été lu, seulement le nom du test.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: M Auto_Journey.md, M Auto_Journey_File.md (mes 2 lignes + autres), M Auto_Journey_Jour.md, M EtapeDefini.md (ma ligne + autres), M Tools/AutoJourney/brief_commun.md, ?? Docs/Priorite_Socle_Factions_2026-11-16.md (+ ce rapport) ; inchangée hors docs voulus : oui
