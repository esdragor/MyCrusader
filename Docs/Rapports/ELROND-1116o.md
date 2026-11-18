# ELROND-1116o - MIG-CPP-Technology (D836 point 2), worktree HeroesOfLiberty-Elrond, branche tiers, sans C++

HEURE_DEBUT/HEURE_FIN: 21:37 / 21:50
BUILD: NON COMPILE (aucun build, Source\ non touche)   BUILDS_DEMANDES: 0   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: vu a 21:42 : quick_check tests 1350/1353, rouge nomme Angelscript.UnitTests.Tests.HOLTechnologyRules_Test.Test_TechnologyRulesVectors (vecteur "cannot pay" faussé : attendu NoBuilding), remis ensuite ; vert 21:49 : 1351/1353, seuls rouges Match.StartingHero.NomadFirstForum et .NomadReplicated (D831, connus). TESTS_TAUTOLOGIQUES: 0 (les vecteurs passent par la vraie chaine C++ HOLTechnologyCheck::Check)
COMMITS: 8d4180c (sur tiers), merge main fait : oui. Branche prete.
EDITS_REFUSES: 0
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 0 ; RETIRE : Test_TechnologyScriptAndSimulationRulesAgree (accord script/C++ sur 1296 cas, sans objet). Le total passe donc de 1354 a 1353 : baisse voulue par la consigne (le test comparait deux copies dont une n'existe plus) ; les 14 vecteurs de Test_TechnologyRulesVectors (ligne "script" retiree) couvrent la regle C++.
LIGNES_CPP_SUPPRIMEES: 0   LIGNES_MIGREES_PAR_LOT: 0 C++ ; script : -122 / +41 (copie de la regle supprimee)
TYPES_A_EXPOSER: aucun

## Appelants de HOLTechnologyRules::CheckImplemented (avant)
Script\AI\HOLAITech.as l. 144 ; Script\Tests\HOLForge_Test.as l. 69-75, 92 ; Script\Tests\HOLTechnologyRules_Test.as (Test_TechnologyNotImplemented, accord, vecteurs). Source\ : aucun (les .cpp ne citent que HOLTechnologyRules.h, C++, autre chose).
Autres fonctions supprimees de HOLTechnologyRules.as : Check, IsAvailable, CheckQueued (appelants : HOLTechnologyRules_Test.as seulement, redirigés vers HOLTechnologyCheck::Check avec bImplemented true, bQueued false). Gardees : constantes de raison 0-8.

## Fait
HOLAITech.as et HOLForge_Test.as appellent HOLTechnologyCheck::Check (meme signature). HOLTechnologyRules_Test.as : Test_TechnologyAvailable/Hero/BuildingAndCost/Chain/ReasonOrder/Queued/NotImplemented redirigés ; helper TechnologyRulesVector : ligne script retiree ; accord retire.
NON VERIFIE : l'AI en partie reelle (tests seulement) ; aucune partie autre que tests.
PROCESSUS_DETACHES: aucun lance par moi (quick_check termine)
