NON COMPILÉ (aucun build, aucun test lancé)

APPELS_DECLARES: 22 (arrêt à 25 vus ; seuil 80 jugé sur tool_uses)
HEURE_DEBUT/HEURE_FIN: 09:26 / 09:28 (Get-Date PowerShell)
PREMIER_BUILD: sans C++ (aucun build demandé)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: sans C++ (rien compilé)
TEST_ROUGE: sans objet (les deux tests étaient déjà rouges depuis D858 ; ce commit corrige leur attente) ; assertit l'effet : oui (file de production, prix payé, PV de naissance)
COMMITS: 1 (005c6da, "Tests follow D858: ..."), dernier à 09:28, merge_main_fait: non (sans objet : aucun merge demandé)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (les grilles passent par HOLFactionRules::FilterProduces sur la liste Produces de la caserne, le prix attendu de l'Aube est lu sur la ligne Halberdier de la table, la santé aussi)
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
- Cause : le code C++ est juste. `IsUnitOpenTo` -> `Allows(Row->Faction, clé de la faction du slot)` ; Spearman = Faction Legions, Halberdier = Faction Dawn (UnitTypes.csv de tool-known-reds, 6d4b82b). Seuls les tests attendaient un Lancier pour l'Aube.
- HOLAIOpeningTests.cpp, AI.Army.Production : slot 1 est l'Aube par défaut ; la caserne attend maintenant un Halberdier (1er type du mix ouvert à l'Aube : le script HOLAIArmy.as de 3fc6949 ajoute Halberdier juste après Spearman, Spearman filtré par CandidateFactions). Asserts : type dans FilterProduces(Produces de Barracks, slot 1), jamais Spearman, égal à Halberdier. Include Match/HOLFactionRules.h ajouté.
- HOLFactionRulesTests.cpp, Match.Factions.Roster : l'Aube commande le Halberdier, les Légions le Spearman (SlotSpear[2]) ; grilles de la caserne vérifiées par FilterProduces (Aube : Halberdier oui / Spearman non ; Légions : inverse) ; prix de l'Aube = ligne Halberdier (75/30), Légions 45/15 (override inchangé) et < prix Spearman de la table (60 food) ; naissances : Halberdier de l'Aube avec les PV de sa ligne (100), Spearman des Légions 777 ; durée de pas = max des deux GetCost.
- RESTE : ces tests ne peuvent passer qu'après fusion de la branche tool-known-reds (CSV Halberdier + HOLAIArmy.as) dans le worktree Haldir (le CSV de wall-line est encore sans Halberdier : Barracks.Produces = Spearman, ManAtArms, DamnedWarrior). Lancer ensuite `Tools\build.ps1 -LiveCoding=0 -Wait` puis les deux tests (HeroesOfLiberty.AI.Army.Production, HeroesOfLiberty.Match.Factions.Roster), sans oublier les .uasset régénérés (9c65e7a).
- Autres tests C++ à surveiller (non demandés, non touchés) : Factions.RealChain, Factions.ClientModel, Factions.Preload, AI.Army.Faction utilisent encore Spearman pour le slot Aube (RealChain l. ~192-224 via fixture ; des jalons pourraient devenir rouges, à vérifier à l'exécution).
NON VÉRIFIÉ: tout : syntaxe C++ (noms FHOLBuildingTypeRow, ->Produces en TArray<FName>, UHOLBuildingSettings::FindType, include HOLFactionRules.h dans HOLAIOpeningTests.cpp), valeurs réelles à l'exécution, ordre du mix de l'IA. TestNotEqual sur FName suppose la surcharge disponible.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
ANOMALIES_PROCESSUS: aucune ; IN_USE.txt remis à « libre ».
GIT_STATUS_PRINCIPALE: non vérifié par git status ; docs modifiés par moi : AubeInProgress.md (ligne ajoutée), EtapeDefini.md (ligne ajoutée) ; hors docs voulus inchangé : oui (aucun code écrit dans la copie principale).
