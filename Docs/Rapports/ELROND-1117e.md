APPELS_DECLARES: 22 (arret a 45 vus)
HEURE_DEBUT/HEURE_FIN: 09:08:12 / 09:13
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 276010a (commit vert unique) ; rouge vu a 09:12 : Test_FactionTechnologyGrid, Category attendue 4 faussee en 5 -> Result=Fail ; remis a 4 -> Result=Success. Assertit l'effet : oui (valeurs lues dans la vraie table par HOLAITechnology::ReadAITechnologyTable).
COMMITS: 1, dernier a 09:12 (276010a sur tiers), merge_main_fait: non (branche = main 962c37d)
TESTS: Automation RunTests Angelscript.UnitTests.Tests.HOLFactionTechnology_Test (moteur "UE 5.8 HOL", pas l'AS_3) : 1/1 vert apres rouge ; quick_check non lance. TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
FAIT: Script\Tests\HOLFactionTechnology_Test.as (Test_FactionTechnologyGrid) : table chargee, TemperedHalberds = ligne Barracks, categorie 4, tier 1, Barracks et Forge non vides. Ligne E3-06 de la file passee a [~], ligne EtapeDefini ajoutee.
RESTE: le filtre par faction (Dawn/Legions sur Barracks, meme liste Forge, IsTechnologyOpenTo) : C++ non expose, FHOLAITechnology n'a pas de colonne Faction. Test C++ NON supprime.
TYPES_A_EXPOSER: HOLFactionRules::IsTechnologyOpenTo(Technology, FactionKey) ; UHOLTechnologySettings::GetBuildingTechnologies(Building, FactionKey, Out) (ou une colonne Faction dans FHOLAITechnology). Meme besoin pour E3-09 FactionNames (HOLFactionRules::GetTable().Overrides).
BUILDS_DEMANDES: 0   TESTS_CPP_RETIRES: 0   TESTS_SCRIPT_AJOUTES: 1 (Test_FactionTechnologyGrid)   LIGNES_CPP_SUPPRIMEES: 0   LIGNES_MIGREES_PAR_LOT: ~15 sur 47
NON VERIFIE: quick_check complet (total de tests non recompte) ; l'echec du rouge n'affiche pas le message d'assertion dans le log (seulement "Error: ::Test_FactionTechnologyGrid"). Piege : l'EngineAssociation du .uproject est "UE 5.8 HOL" ; prendre la 1re entree du registre (AS_3) donne "game module could not be found" (BuildId).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: Auto_Journey_File.md (ma ligne), Auto_Journey_Jour.md, Docs/Priorite_Socle_Factions (preexistants), rapports d'autres agents ; aucun code : oui
