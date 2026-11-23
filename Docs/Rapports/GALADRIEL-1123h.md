# GALADRIEL-1123h : D889 « Bientôt » EN sur deux lignes (NON COMPILÉ)

APPELS_DECLARES: ~25 (arrêt à 25 vus)
HEURE_DEBUT/HEURE_FIN: 10:26:24 / ~10:34
PREMIER_BUILD: demandé 10:28:50, EN FILE derrière le build de HeroesOfLiberty-Faramir (non démarré à 10:32)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: binaires périmés (UnrealEditor-HeroesOfLiberty.dll 2026-11-22T22:27, ...UI.dll 2026-11-22T18:56) : build nécessaire
TEST_ROUGE: rouge préexistant (ligne D889 d'Auto_Journey_File : StatusFit 55,8 px pour 47), NON revu à l'exécution ce jour (pas de build) + « assertit l'effet : oui » (lignes comptées, largeur de la ligne la plus large mesurée par la case, contenu lu)   TEST_ROUGE_SEPARE: non (un seul commit c1ba30b : un seul build possible dans le temps, pas de bouchon)
COMMITS: 1 (c1ba30b, 10:28), merge_main_fait: oui (merge --no-edit main OK en entrée ; MoraleInProgress.md modifié dans le worktree à l'entrée, non touché, non commité)
TESTS: aucun lancé (pas de build) ; TESTS_TAUTOLOGIQUES: 0 (Width et lignes calculées par la case : MeasureStatusWidth, BreakStatusLines, ComputeStatusFit)
ESSAIS: aucun (pas de build) ; tech_soon FR/EN/JA 1920 et 1280 à faire
QUESTIONS: aucune
FAIT: HOLGridCell.cpp/.h : UHOLGridCell::BreakStatusLines (coupe au espace le plus proche du milieu), GetStatusLineCount, SetStatus coupe en 2 lignes si la largeur à une ligne > DefaultCellSize - 2*StatusSidePad (72), ComputeNameZone(..., StatusLines = 1) (hauteur d'état = lignes x ligne), MeasureStatusWidth = ligne la plus large, DescribeContent lit ShownStatus (texte d'origine). Test StatusFit étendu (1 ligne FR, 2 lignes EN, contenu, ajustement sous 47 px, échelle > borne, zone de nom, BreakStatusLines).
RESTE: `Tools\build.ps1 -LiveCoding=0 -Wait` dans HeroesOfLiberty-Galadriel ; `Tools\Tests\quick_check.ps1 -Parts tests` (rouge à voir si on veut : le test seul n'a pas de bouchon) ; `Tools\Scenarios\preflight.ps1 tech_soon` en FR/EN/JA 1920 et 1280, PNG ouverts ; puis intégrer la branche. Risque à vérifier : en l'absence de moteur de rendu l'estimation (5,5 px par lettre) donne 60,5 < 72 pour « Coming soon » : la coupe n'a lieu qu'avec un renderer (cas de l'éditeur réel, mesuré 93 px).
NON VÉRIFIÉ: tout (code jamais compilé, test jamais lancé, aucun PNG).
PROCESSUS_DETACHES: voir réponse finale
ANOMALIES_PROCESSUS: build en file derrière Faramir (position 1) ; le build lancé en arrière-plan (task bnv6nhh6l) meurt si l'agent finit : à relancer par la session principale.
GIT_STATUS_PRINCIPALE: non relevé
