# CELEBORN-1116t : redirection de Diplomacy vers HOLDiplomacyRead (script seul)

Branche hero-j8-close, commit 0b54124 (sans C++, sans build, Source\ non touche). Lance 23:35, fini 23:43.

FAIT
- HOLDiplomacyPanel.as, HOLGameScreenScript.as, HOLDiplomacyPanel_Test.as, HOLSignalRules_Test.as : HOLDiplomacy::Enemy/Neutral/Ally -> HOLDiplomacyRead::GetStanceEnemy()/GetStanceNeutral()/GetStanceAlly().
- HOLDiplomacyRules_Test.as : 14 tests + la 1re moitie de Test_DiplomacyRulesVectors passes sur FHOLDiplomacyStances, HOLDiplomacyRead (ChangeStance/ChangeVision, fiches FHOLDiplomacyStanceChange/VisionChange, GetResult*). Helpers LibHumansBook/LibMakeAllies/LibDiploVecAllies. Les tests Garrison, AlliedVictory, AlliedVictoryAIsAndTeams et la 2e moitie des vecteurs (nouveau test Test_DiplomacyRulesVectorsGarrisonVictory) restent sur HOLDiplomacyRules.as.
- Le sens de HOLDiplomacyRules.as n'est pas change.

NON SUPPRIME : HOLDiplomacyRules.as et son test. Appelants restants (Grep) : seulement HOLDiplomacyRules_Test.as, pour CanGarrison, DoorsOpenFor, AlliedVictory (FHOLAlliedVictory) ; InMatch n'a plus d'appelant mais reste copie d'une ligne dans HOLSignalRules.as (commentaires). A exposer a HOLDiplomacyRead au prochain lot C++ : CanGarrison, DoorsOpenFor, AlliedVictory + fiche, InMatch ; ensuite git rm des 2 fichiers (le test de comparaison du socle existe : HOLSocleLibraries_Test.as).

ROUGE : 23:37 valeur faussee (Second.bFormed AssertTrue -> AssertFalse dans Test_DiplomacyAllyNeedsBoth, sur la bibliotheque) : tests 1361/1364 (Test_DiplomacyAllyNeedsBoth + 2 Nomad). Remise : tests 1362/1364, seuls rouges Nomad D831. Les scripts compilent sans build neuf (DLL a jour).

CHAMPS
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 1 (Test_DiplomacyRulesVectorsGarrisonVictory, scission de Test_DiplomacyRulesVectors, total 1364 inchange) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~340 lignes de script redirigees ; TYPES_A_EXPOSER: FHOLAlliedVictory + AlliedVictory, CanGarrison, DoorsOpenFor, InMatch ; TEST_ROUGE: vu a 23:37 : 1361/1364 ; TESTS_TAUTOLOGIQUES: 0 ; EDITS_REFUSES: 1 (edit_line -After mal ecrit, refait) ; PROCESSUS_DETACHES: aucun lance par moi.
NON VERIFIE : Placeholders.txt non touche (aucun provisoire ajoute ni retire) ; "Fichiers de developpement" d'EtapeDefini.md non touche.