# SOCLE-1116a — socle de l'étape 1, S1b + S1c réduits à D836 (worktree Celeborn, branche hero-j8-close)

```
APPELS_DECLARES: 43 (arrêt à 45 vus)
HEURE_DEBUT/HEURE_FIN: 2026-11-16 21:30 / 21:58
PREMIER_BUILD: 21:35 (en file derrière Arwen, PASS 21:48, 632 s, « BUILD LENT »)   BUILD: PASS   LAUNCHCHECK_DEBUT: 21:30 vide
TEST_ROUGE: vu a 21:50 : tests 1352/1358 (les 4 tests neufs rouges, une valeur attendue faussée dans chacun, + 2 Nomad D831) ; vert 21:56 : 1356/1358 ; commit vert unique 2ff1ac3 (test + liaisons) ; assertit l'effet : oui (valeurs calculées par la règle C++ via la bibliothèque)
COMMITS: 2, dernier à 21:58 (2ff1ac3 socle ; 3527dee merge main), merge_main_fait: oui (conflit de docs AngelscriptInProgress.md seul, les deux sections gardées)
TESTS: build PASS ; quick_check tests 1356/1358 (seuls rouges : Match.StartingHero.NomadFirstForum et .NomadReplicated, D831, signalés « REGRESSION » car absents de known_reds.txt de ce worktree avant le merge) ; net : toutes les parties PASS (diplomacy, siege, market, 2 clients...) ; netsig3 PASS ; TESTS_TAUTOLOGIQUES: 0 (Test_CoreTypesVisibleFromScript est un test de visibilité, pas de règle)
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
 - S1b : FHOLResources (USTRUCT BlueprintType, 4 champs BlueprintReadWrite) ; FHOLMarketRow (BlueprintType, 10 champs). FHOLTerrainZone NON fait (D836 (1)).
 - S1c Siege : UHOLSiegeCheckLibrary neuf (HOLSiegeCheck::Check, FindEngine, DescribeBlock + GetTier, InPrototype, GetHealthStat, GetDamageStat ; numéros du script).
 - S1c Diplomacy : HOLDiplomacyReadLibrary étendue : FHOLDiplomacyStances (nom distinct du FHOLDiplomacyBook du script) + GetBook (livre répliqué), NewBook, NewTeamBook, SetStance, SetVision, StanceOf, Relation, IsAlly, IsAdversary, FiresAtOnce, SharesVision, CanLeaveRampart.
 - S1c UnitAbility : UHOLUnitAbilityReadLibrary neuf (type + 7 constantes + CanCastRage, GetRageHitCost, GetRageIntervalMs).
 - Tests : Script\Tests\HOLSocleLibraries_Test.as (4 tests).
 - RESTE : 3 lignes `[ ]` ajoutées dans la file (MIG-S1c-Siege, -UnitAbility, -Diplomacy : redirection IA/interface puis retrait des copies script) ; S1d NON préparé (consigne 15 min) ; S1a (menu) non touché.
BUILDS_DEMANDES: 1
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 4 (Test_SiegeCheckLibraryVectors, Test_DiplomacyReadLibraryVectors, Test_UnitAbilityReadLibraryAgreesWithTable, Test_CoreTypesVisibleFromScript)
LIGNES_CPP_SUPPRIMEES: 0 (C++ : 8 fichiers, +400 / -17)
LIGNES_MIGREES_PAR_LOT: 0
TYPES_A_EXPOSER: fiches de changement HOLDiplomacy (FStanceChange bAnnounce/bFormed/bBroke/Before/After, FAlliedVictory) si l'interface en a besoin ; Siege TakesUnitsOn/IsInArea/HealthWithTechnology (hors HOLSiegeRules.h)
NON VÉRIFIÉ: le tip après merge (3527dee) n'est PAS compilé ni testé ici : main y apporte du C++ (HOLChatSubsystem, HOLGameMenuScreen, HOLPlayerController...), Get-StaleBinaryProblem le signale ; aucun fichier C++ commun avec mon lot, mais l'agent de fusion doit compiler. Les vecteurs de Diplomacy.Rules qui lisent les fiches de changement (bFormed, bAnnounce, Before/After) ne sont pas repris (non exposées). Les assertions rampart reprennent les lignes 119-129 du test C++ (lignes 120-121 et 127 supposées : alliance 0-1 puis rupture 1->0).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: build en file derrière Arwen puis « BUILD LENT » 632 s (machine chargée) ; consigne du coordinateur reçue à 21:3x (15 min max) : pas de S1d.
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, M Auto_Journey_Jour.md, M EtapeDefini.md, ?? Docs/Priorite_Socle_Factions_2026-11-16.md (+ ce rapport) ; inchangée hors docs voulus : oui (Auto_Journey_Jour.md et Priorite_* n'étaient pas de moi)
```

Branche : PRÊTE au sens du commit 2ff1ac3 (build PASS + tests,net verts hors 2 rouges Nomad D831). Le merge 3527dee demande une recompilation (C++ de main).
