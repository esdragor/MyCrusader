# FARAMIR-1120pb (zone-effect, sans C++, alertes mineures D804 B, partie script)

Impact joueur : un bâtiment fini ou une recherche finie ajoute une ligne discrète au fil de messages (8 s), joue le ping d'alerte provisoire et clignote sur la mini-carte (lieu connu seulement), une ligne par type et par 30 s. NON COMPILÉ, non vu à l'écran.

APPELS_DECLARES: 14 (arrêt à 25 vus)
HEURE_DEBUT/HEURE_FIN: 15:46 / 15:49 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: f311fd1 (test seul, stub `HOLAlertRules::MinorStep` vide), assertit l'effet : oui (les alertes calculées par `MinorStep` à partir de compteurs lus). Rouge NON VU à l'exécution (aucun quick_check demandé).
COMMITS: 3 (f311fd1 test rouge, 9173e34 correctif + branchement, 6fe4378 journal), dernier à 15:48, merge_main_fait: oui (déjà à jour sur 4374ef7)
TESTS: aucun lancé ; TESTS_SCRIPT_AJOUTES: 2 (Test_AlertMinorStepFromCounters, Test_AlertMinorStepNoReplay, dans `Script\UI\HOLAlertRules_Test.as`) ; TESTS_TAUTOLOGIQUES: 0
BUILDS_DEMANDES: 0 (à lancer par la session principale : `quick_check -Parts tests` sur le worktree Faramir ; rouges attendus au premier commit f311fd1 : Test_AlertMinorStepFromCounters, Test_AlertMinorStepNoReplay)
TYPES_A_EXPOSER: aucun nouveau ; manque côté C++ déjà noté : lieu de la recherche (`OutLastResearchBuildingLocation` reste (0,0), champ répliqué ou RPC), sons par type (assets, `FHOLMinorAlert.SoundKey` réservé).
ESSAIS: aucun (pas de scénario ; un scénario d'essai exige un build).
QUESTIONS: aucune (le design est celui de D804 B).
DIAGNOSTICS_AS: NON VÉRIFIÉ (pas d'éditeur autorisé)

FAIT :
- `Script\UI\HOLAlertRules.as` : `FHOLMinorWatch`, `FHOLMinorStep`, `HOLAlertRules::MinorStep(Watch, Buildings, Researches, Now)` : 1re lecture = base, compteur qui monte = événement, `MinorAlert` par type, horloge propre à chaque type.
- `Script\UI\HOLGameScreenScript.as` : `RefreshMinorAlerts()` appelée à chaque Refresh juste après `RefreshPopulationAlert()`, lit `GetMinorEventsNow` (rien pour un spectateur), `ShowMinorAlert` : `AddThreadLine(texte, 8 s)`, `PlayAlertSound()` (ping 1 kHz provisoire, déjà dans Placeholders.txt l. 198), ping mini-carte par `FHOLMinimapAlert(X, Y)` ajouté à `MinimapAlerts` si le lieu est connu (|X| ou |Y| > 1).
- Journal `AlertsMinorInProgress.md` (worktree, 6fe4378), ligne d'`EtapeDefini.md` l. 29, ligne de file (l. 102 au moment de l'écriture).
RESTE : sons propres par type ; lieu de la recherche (C++) ; « attaque ailleurs » hors vue (détection des PV en baisse hors de la vue, branchement de `ShouldAlertAttackedElsewhere`) ; build + quick_check.

NON VÉRIFIÉ : compilation des scripts (aucun éditeur) ; type de l'argument `Seconds` de `AddThreadLine` (déclaré `double` dans `HOLScriptScreen.h:304`, passage d'un `float` script = double) ; `GetMinorEventsNow` appelé depuis le script avec `FVector2D&` et `int&` ; unités de `FHOLMinimapAlert` (X, Y en monde, comme `Detect`) ; affichage réel.
ANOMALIES_PROCESSUS: une erreur de ma part, corrigée : `Auto_Journey_File.md` avait glissé de 2 lignes entre ma lecture et mon `edit_line` (autre session) ; j'avais remplacé l'indice par ligne lue au lieu de la ligne trouvée, ce qui a écrasé la ligne alertes mineures par la copie d'une autre ligne ; restaurée dans la minute depuis la sortie de lecture de 15:46 (ligne 102, relue : texte d'origine + ma note). À vérifier d'un coup d'oeil par la session principale (`git diff Auto_Journey_File.md`, la ligne « lignes de mouvement » l. 100 doit rester seule).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
GIT_STATUS_PRINCIPALE: docs seuls modifiés (EtapeDefini.md, Auto_Journey_File.md) : oui, hors les autres sessions
