NON COMPILÉ

# ELROND-1120b : D861 (a) engin emballé + ordre d'attaque

Joueur : un engin de siège démonté qui reçoit un ordre d'attaque sur une cible à portée doit se monter puis tirer ; l'hypothèse du lot (portée de montage 5000 cm plus courte que la portée de combat) est FAUSSE pour la Mangonnelle, donc la cause réelle de (a) reste à prouver à l'exécution.

APPELS_DECLARES: ~22 (arrêt à 25)
HEURE_DEBUT/HEURE_FIN: 09:08 / ~09:12 (Get-Date)
PREMIER_BUILD: sans build (NON COMPILÉ, demandé)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (aucun build dans l'agent)
TEST_ROUGE: 13ee758 (deux tests, antérieur au correctif d498a8d) ; assertit l'effet : oui (phase Deployed après DeployMs + PV de la maison / de l'unité qui baissent). Rouge NON VU (rien lancé), et le test « 2500 cm » peut être DÉJÀ VERT (voir NON VÉRIFIÉ).
COMMITS: 2 (13ee758 tests, d498a8d correctif), derniers 09:10 et 09:11, merge_main_fait: non (branche = main 901e391 + d9d930b + 78be07e)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (le test à 2500 cm passe par la vraie chaîne file -> ordre -> combat -> déploiement ; la variante « note à la main » n'est PAS une chaîne réelle, voir ci-dessous)
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (le lot C++ de la séance, avec 78be07e : HOLDeployProcessor.cpp, HOLUnitOrderProcessor.cpp, HOLDeployButtonTests.cpp ; commande `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\build.ps1 -LiveCoding=0 -Wait`, puis tests `HeroesOfLiberty.Siege.Deploy.AttackWhilePackedDeploysThenFires`, `HeroesOfLiberty.Siege.Deploy.AtTheCombatReachButBeyondDeployRange`, et la régression `HeroesOfLiberty.Siege.Deploy`)
TESTS_SCRIPT_AJOUTES: 0 (tests Mass : C++, D838)
TYPES_A_EXPOSER: aucun

## FAIT / RESTE
- Lecture : `UnitTypes.csv` Mangonel : `AttackRange` = `BuildingAttackRange` = 3000 cm. `HOLCombatProcessor.cpp:2480-2483` (bâtiment) et `:3012-3015` (unité) n'écrivent la note `PendingOrder = 1` + distance QU'une fois « à portée de combat » (donc distance <= ~3000 + rayons, 2D). `HOLDeploy::DefaultProfile()` : Min 1200, Max 5000. Donc pour la Mangonnelle, `Decide(Attack, Packed)` ne rend jamais `March` sur une note du combat (3000 < 5000) : la bande « entre MaxRangeCm et la portée de combat » est VIDE. L'hypothèse du lot n'explique pas le bug de la Mangonnelle.
- Le harnais de test n'a pas de mouvement (« no movement runs here ») : une cible à 4000 cm (au-delà des 3000 de portée) n'est jamais atteinte, ce serait un test rouge à jamais. J'ai donc remplacé 4000 par 2500 cm (dedans pour le combat et pour Decide).
- `HeroesOfLiberty.Siege.Deploy.AttackWhilePackedDeploysThenFires` (HOLDeployButtonTests.cpp) : vraie chaîne (file `HOLPlayerOrders::AttackBuilding` puis `Attack`, processeurs ordre / déploiement / combat) ; maison de slot 3 à 2500 cm, puis Lancier (Hold) de slot 3 à 2500 cm ; asserte `Deployed` après `DeployMs + 20` pas ET PV de la cible qui baissent (<= 10 s).
- `HeroesOfLiberty.Siege.Deploy.AtTheCombatReachButBeyondDeployRange` : la note du combat écrite à la main (PendingOrder 1, distance `MaxRangeCm + 1000` -> doit finir `Deployed` ; distance `MinRangeCm - 100` -> doit rester `Packed`, garde de non-régression de `TooClose`). Seul cas rouge garanti avec le code d'avant ; il teste le processeur, pas la chaîne complète (limite assumée).
- Correctif (d498a8d, `HOLDeployProcessor.cpp`) : pour `PendingOrder == 1` la distance donnée à `Decide` est `Min(PendingDistanceCm, Profile.MaxRangeCm)` : le verdict « à portée » du combat n'est plus contredit par la portée de visée de montage ; le bout proche (`MinRangeCm`) est inchangé. `Decide` n'est PAS modifié.
- Jumeau script : `Script\Tests\HOLDeployRules_Test.as` ne change pas ; l'en-tête de `HOLDeployRules.h` dit que la règle n'a qu'une source C++ lue par `HOLDeployRead` (ce `.as` est son test, pas un jumeau de règle) : je ne tranche rien.
- RESTE : compiler et lancer les deux tests. Si `AttackWhilePackedDeploysThenFires` est DÉJÀ VERT à 2500 cm, la cause de D861 (a) est ailleurs. Pistes lues (non prouvées) : (1) une cible plus proche que 1200 cm donne `Refuse/TooClose` (l'engin ne fait rien, aucune marche arrière) ; (2) le joueur clique plus loin que la portée de combat : l'engin marche (comportement attendu) puis doit se monter à l'arrivée, à vérifier en match avec le mouvement réel ; (3) pendant `Deploying`, un ordre de marche ouvert qui n'est pas l'approche du combat note `PendingOrder = 2` (`HOLDeployProcessor.cpp` boucle, `Orders[It].Phase != None && !Combats[It].bApproaching`) et annulerait le montage via `Decide(Move, Deploying)` = `CancelDeploy` ; lire la ligne `Deploy: #id phase a -> b (order, action, reason, distance, walk)` du journal du scénario `deploy` (clic droit sur sol, puis clic droit sur Maison) pour trancher.

## NON VÉRIFIÉ
- Rien compilé ni lancé. Les symboles des tests (`HOLBuildingTypeNames::House`, `AHOLBuilding::Setup/GetHealth`, `HOLPlayerOrders::AttackBuilding`) sont recopiés de `HOLDeployInMatchTests.cpp`, jamais compilés dans ce fichier ; deux includes ajoutés (`Buildings/HOLBuilding.h`, `Buildings/HOLBuildingTypes.h`).
- `Slack = 20` pas et la distance de 2500 cm pour le bâtiment (la portée se mesure à la boîte du mur) : valeurs raisonnées, pas essayées.
- Le test Lancier (Hold) suppose que le Lancier ne s'approche pas (Hold) et ne tue pas la Mangonnelle en 10 s.

## PROCESSUS_DETACHES / ANOMALIES
PROCESSUS_DETACHES: voir la fin de ce fichier (stray_processes lancé en dernier geste).
ANOMALIES_PROCESSUS: aucune connue ; les deux premiers Edit (includes puis bloc de tests) ont passé, un message système a signalé que HOLDeployButtonTests.cpp avait changé sur disque (c'était mon remplacement du nom du second test par PowerShell). EDITS_REFUSES: 0.
