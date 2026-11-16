# ARWEN-1116p - MIG-S1c-Siege (redirection vers HOLSiegeCheck, retrait de la copie script)

APPELS_DECLARES: 15
HEURE_DEBUT/HEURE_FIN: 22:21 / 22:30
BUILDS_DEMANDES: 0 (sans C++, DLL du worktree a jour avec le socle)
TEST_ROUGE: vu a 22:26 : tests 1345/1348 (Test_SiegeEngineTiers, valeur attendue 1 faussee en 9) ; vert 22:29 : 1346/1348 (seuls rouges : Match.StartingHero.NomadFirstForum et .NomadReplicated, D831, rouges connus)
TESTS_TAUTOLOGIQUES: 0
COMMITS: fc908b6 sur d751-root (branche prete, jamais pousse, merge main non fait)
FAIT:
 - Appelants de la copie script (Grep Script\ et Source\) : HOLAISiege.as (2 appels HOLSiege::Check), HOLSiegeRules_Test.as, HOLUnitTierRules_Test.as (Check, TierOf), HOLSocleLibraries_Test.as (boucle d'accord). Source\ : seulement des commentaires.
 - HOLAISiege.as : HOLSiegeCheck::Check(int(EHOLSiegeEngine::X), ...).
 - HOLSiegeRules.as : retires TierOf, InPrototype, Check, FindEngine, DescribeBlock, HealthStatOf, DamageStatOf (tous exposes par HOLSiegeCheck).
 - RESTENT (non exposes par HOLSiegeCheck) : enum EHOLSiegeEngine, constantes HOLSiege::Available.. et EngineCount/EngineAt (numerotation), PrototypeEngines, OpenedEngines, IsAvailable (redirigees sur HOLSiegeCheck::InPrototype/GetTier/Check), HealthWithTechnology, AttackWithTechnology, TakesUnitsOn, IsInArea.
TESTS_SCRIPT_RETIRES: 0 retire ; tous retargetes sur la bibliotheque (le total ne baisse pas) : Test_SiegeEngineTiers, Test_SiegePrototypeSet, Test_SiegeTierOrder, Test_SiegeOpenedByTier, Test_SiegeCheckOneReason, Test_SiegeCheckReasonOrder, Test_SiegeIsAvailableAgreesWithCheck, Test_SiegeFindEngine, Test_SiegeTechnologies (HOLSiegeCheck::), Test_UnitTierAgreesWithTheSiegeRule (lit HOLSiegeCheck). Test_SiegeCheckLibraryVectors : la boucle d'accord "copie C++ / copie script" est remplacee par l'ordre des parties de la bibliotheque (InPrototype, GetTier) ; couvert aussi par HOLSiegeTests.cpp (C++ intact).
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 0
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: ~95 lignes de script retirees (HOLSiegeRules.as)
TYPES_A_EXPOSER: TakesUnitsOn, IsInArea, HealthWithTechnology, AttackWithTechnology (hors HOLSiegeRules.h), restent script
DOCS: Auto_Journey_File.md (ligne [x] + sha), EtapeDefini.md (ligne 65), AngelscriptInProgress.md, IN_USE.txt (CPP: non)
NON VERIFIE: net et jeu non lances (script seul, quick_check -Parts tests une fois en vert + une fois en rouge voulu) ; Placeholders.txt non touche (aucun provisoire ajoute/retire).