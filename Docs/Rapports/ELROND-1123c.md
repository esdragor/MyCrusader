# ELROND-1123c : S4-b (garnison de l'IA), integ-25, script seul

APPELS_DECLARES: ~28 vus (hors seuil de 25 : trois attentes de quick_check ont coute des appels) ; premier Edit apres le 8e appel (non respecte)
HEURE_DEBUT/HEURE_FIN: 09:21 / ~09:33
PREMIER_BUILD: sans C++   BUILD: sans C++   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 4e15858 (Test_AIGarrisonsAttackedTower + bouchon HOLAIGarrisonCall::Plan qui renvoie vide), vu a 09:27 : quick_check tests 2099/2100 (REGRESSION, `Test_AIGarrisonsAttackedTower`) ; assertit l'effet : oui (unites appelees, jamais l'ennemi, jamais deux tours pour une unite, tour calme et IA Easy : personne)   TEST_ROUGE_SEPARE: oui
COMMITS: 2 (4e15858 rouge, 33bcecc correctif), dernier a ~09:31, merge_main_fait: non (pas demande)
TESTS: quick_check -Parts tests. Rouge : 2099/2100 (216 s). Vert : relance 09:27 -> 09:31, code de sortie 0 mais SORTIE VIDE (le pipe `Select-Object -Last 12` n'a rien rendu) : compte vert NON VU. TESTS_TAUTOLOGIQUES: 0 (la liste des unites vient de HOLAIGarrison::Entrants / TowerCalls / HOLGarrison::AdmittedWith)
ESSAIS: aucun
QUESTIONS: aucune (Retours\Q-garrison-ia.md existe deja, valeurs par defaut = celles de HOLAIGarrisonRules.as)
FAIT: `Script\AI\HOLAIGarrisonCall.as` : `HOLAIGarrisonCall::Plan(Rule, Level, Towers, UnitIds, UnitSlots, UnitKinds, UnitPlaces, Radius)` -> `FHOLAIGarrisonCall[]` (tour + ids), une unite entree dans une tour n'est plus offerte aux suivantes. Test `Script\AI\HOLAIGarrisonCall_Test.as`.
RESTE: appel dans `HOLAIBrain.as` : NON FAIT, faute de donnees. Ordre de garnison de l'IA : `HOLPlayerOrders::Garrison` / `HOLCommandScript::IssueGarrison` existent en script (Commands\) mais exigent un `UHOLCommandSubsystem` que le cerveau n'a pas ; l'IA n'a pas de `HOLAIGarrisonOrder::` (patron : `HOLAIRageOrder::IssueAIRageOrder`, `HOLAIRageOrders.h`). TYPES_A_EXPOSER : (1) dans `FHOLAIBuilding` (`AI\HOLAISummary.h:101`) : places de garnison (GarrisonSlots), occupants, secondes depuis le premier et le dernier coup recu (aucun lecteur de coups sur AHOLBuilding trouve) ; (2) dans le resume des unites : slot, genre de garnison (HOLGarrison::Kind*), position, id ; (3) un ordre `IssueAIGarrisonOrder(PlayerIndex, UnitIds, Building)` sur le patron de l'ordre de rage. Aussi : aucune tour dans BuildingTypes.csv (S3-a Celeborn, integ-41) : sans elle, aucun batiment de l'IA ne prend de garnison.
NON VÉRIFIÉ: le vert (sortie vide, voir TESTS) ; DIAGNOSTICS_AS non relus (serveur de langage non consulte) ; Grep des parties aiduel / aiserver de quick_check sur les fichiers touches non fait (nouveaux fichiers, aucun appelant, donc aucune partie ne les charge hors `tests`) ; Placeholders.txt et AIInProgress.md / LegionsInProgress.md non mis a jour (budget) .
PROCESSUS_DETACHES: voir la sortie de stray_processes en dernier geste (dans la reponse finale)
ANOMALIES_PROCESSUS: quick_check a ete deplace en arriere-plan apres 180 s (le blocage depasse la limite d'un appel) ; sortie vide au second passage ; premier Edit tardif (8e appel passe) ; arret a 09:35 respecte
GIT_STATUS_PRINCIPALE: non releve ; la copie principale n'a recu que les lignes de EtapeDefini.md et Auto_Journey_File.md : oui
