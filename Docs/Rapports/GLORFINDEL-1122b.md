# GLORFINDEL-1122b : scénario follow_short (pont de Suivre à l'exécution) : VERT sur les erreurs, SUIVI NON PROUVÉ

APPELS_DECLARES: 25 (arrêt à 25 vus ; premier Edit/Write au 12e appel environ : consigne « avant le 8e » non tenue, 9 appels de lecture)
HEURE_DEBUT/HEURE_FIN: 16:40 / 16:52 (Get-Date PowerShell)
PREMIER_BUILD: sans C++   BUILD: sans build (dll à jour 16:39 selon l'énoncé)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scénario, pas de correctif) ; le rouge du CDO (19 727 erreurs) est celui de L22-5, non rejoué ici   TEST_ROUGE_SEPARE: non (rien à séparer : aucun correctif, HOLFollowBridge.as non touché)
COMMITS: 1 (f1c0ec4 scénario follow_short.txt/.look/.json, 16:44), merge_main_fait: non (non demandé)
TESTS: 1 passage `play.ps1 follow_short -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -TimeoutSec 300` ; TESTS_TAUTOLOGIQUES: 0 ; TESTS_SCRIPT_AJOUTES: 0
ESSAIS: follow_short : preflight NON lancé, PNG start et walk ouverts (cancelled non ouvert) : LOGIQUE SEULEMENT (aucune différence visible, voir plus bas)
QUESTIONS: aucune
BUILDS_DEMANDES: 0   TYPES_A_EXPOSER: aucun

## Geste utilisé
`hol.Select.Army` (4 Hommes d'armes) puis `hol.Select.ClickUnit -1 order` (clic droit sur le dernier id = Paysan #5 allié hors sélection ; journal : « Right click on unit #5 at (0.500, 0.355) » puis « Right click on unit #5: the selected units follow it »). C'est la commande hol.* la plus proche du geste réel, MAIS elle n'entre PAS par `UHOLFollowBridge::NativeRightClickOnUnit` (HOLPlayerController.cpp:1465) : le départ du suivi se fait en script (HOLControllerCommandsC10a.as:229-237, `UHOLFollowBookHolder::Get().Book`). Le pont (instance) n'est donc exercé que par `NativeStep` (Tick du contrôleur) et `NativeManualOrder` (ordre manuel : `hol.Select.OrderNear 1500 0`, journal « Order: 4 units to (-13291, 0) » + « RpcOrder: move received by the server »). `RightClickOnUnit` du pont n'est pas rejoué ici.

## Comptes (journal `Saved\Logs\HeroesOfLiberty.log`, tout le journal du lancement, 2321 lignes, scénario « ready » à 15.42.28 UTC)
Commande : `Select-String -Path $l -Pattern '<motif>' | Measure-Object`
- `Angelscript` : 546 lignes, dont `Angelscript: Error` = 0 (et `LogAngelscript: Error` = 0) ; 276 `Angelscript: Warning` (division entière, `HOLScenarioPanelScript.as`... : bruit de compilation, aucun lié au pont).
- `requires WorldContext` : **0** (L22-5 : 19 727 avec le CDO).
- `Script` : 614 lignes (noms de fichiers `.as` et « Compiling »), aucune erreur.
- `Error:` : 20, toutes `LogAutomationTest: Error: Condition failed` à 15.42.20 (autotests du moteur `UE::UnifiedErrorTest`, au chargement, avant le scénario) : sans rapport.
- Classe script du pont : trouvée ou non, NON DIT par le journal (aucun log de `FindScriptClass`) ; aucune erreur de compilation de `Script\Orders\HOLFollowBridge.as`.

## Ligne « Follow: » attendue
Elle n'existe nulle part dans le code (grep `Follow: ` dans Source et Script : aucune). Lignes de suivi réellement présentes : « Right click on unit #5: the selected units follow it » (script C10a). La ligne C++ « Order: N units follow #id ... » (HOLPlayerController.cpp:1466) n'apparaît pas (autre chemin).

## L'unité suit-elle ? NON PROUVÉ
- Captures follow_short_start / follow_short_walk : IDENTIQUES à l'œil (6 ovales verts à droite = décor, un petit cercle de sélection derrière le château) : les Hommes d'armes sont masqués par le château, aucune distance lisible. Le scénario s'est déroulé en ~3 s réelles (240 images = 1,7 s de jeu), trop court pour une marche mesurable.
- Aucun log de position ni de `Step` du pont : « décroissance de distance » non mesurée. L'ordre manuel annule-t-il le suivi : le journal montre l'ordre de déplacement reçu par le serveur, mais AUCUNE preuve que le livre a été vidé ni que le suivi n'a pas repris (pas de log).
- Le scénario n'a pas de `quit` : le jeu masqué a tourné jusqu'à `-TimeoutSec` (300 s), tué par play.ps1 (normal ici, mais 5 min).

## RESTE (pour un agent neuf)
1. Rejouer avec une prise de vue qui montre les unités (FocusNear sur le groupe, zoom, pas derrière le château : Paysan à (0,-900) est derrière le château du centre) et un log de position (une commande hol.* de position d'unité, ou un `Step` journalisé dans HOLFollowOrders.as) pour la distance décroissante.
2. Ajouter `quit` en fin de scénario.
3. Gestes réel : `hol.Select.OrderNear <dx> <dy>` visant le Paysan pour passer par `NativeRightClickOnUnit` (non essayé).

NON VÉRIFIÉ : le suivi à l'exécution (marche, annulation), la présence de la classe script du pont (aucun log), le rouge CDO (non rejoué), l'ouverture de follow_short_cancelled.png.
DIAGNOSTICS_AS: non relus dans VS Code (aucun .as modifié) ; journal du jeu : 0 erreur Angelscript.
PROCESSUS_DETACHES: voir la sortie de stray_processes en fin de séance (play.ps1 a fermé ses 5 processus : cl, crashpad_handler, powershell, UnrealEditor, zenserver)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non relancé (inchangée par moi hors ce rapport et ma ligne d'EtapeDefini.md : oui)
