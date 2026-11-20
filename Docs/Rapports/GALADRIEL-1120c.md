NON COMPILÉ (rien compilé ni lancé ; la session principale compile et lance les tests).

# GALADRIEL-1120c : D860 H2 mini-carte qui pulse, D833 part script

APPELS_DECLARES: ~27 (arrêt visé 25 ; dépassé de peu, l'heure d'arrêt 09:52 n'était pas atteinte)
HEURE_DEBUT/HEURE_FIN: 09:36 / 09:40 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 71fa32f (H2, stub qui garde le carré fixe de 10 px) et cd72b61 (D833, stub PopulationFullLineSeconds = 0) + « assertit l'effet : oui » (la taille de la marque rendue par HOLSignalMarks::Mark change avec l'âge et vaut base x PulseScale, calculée par la règle C++ ; le centre ne bouge pas ; retour à la base à l'expiration). Rouge NON VU à l'exécution.
COMMITS: 4 (71fa32f test H2, 8ea7fff correctif H2, cd72b61 test D833, 8fe8b75 correctif D833), dernier à 09:38, merge_main_fait: non ; `git merge integ-26` fait en premier geste (comme demandé)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (valeurs calculées par HOLSignalInputRead::PulseScale / MarkLifetime ; le test D833 vérifie des relations : > 10 s, < PopulationRepeatSeconds, ShouldAlertPopulationFull n'en relance pas une à la sortie de la ligne)
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 0 (les scripts se compilent au lancement des tests ; `quick_check -Parts tests` par la session principale)
TESTS_SCRIPT_AJOUTES: 2 (Test_MinimapSignalMarkPulses dans Script\UI\HOLSignalMarks_Test.as ; Test_AlertPopulationFullLineDuration dans Script\UI\HOLAlertRules_Test.as)
TYPES_A_EXPOSER: AddThreadLine(const FText&, double Seconds) sur UHOLScriptScreen (wrapper de GLORFINDEL-1120c, absent de cet arbre ; seul AddThreadLine(FText) existe)

FAIT
- H2 mini-carte : nouvelle règle pure `HOLSignalMarks::Mark(Frame, Location, Color, ReceivedAt, Now)` (Script\UI\HOLSignalMarks.as) : `Size = 10 * HOLSignalInputRead::PulseScale(Now - ReceivedAt)`. `RefreshMinimap` (HOLGameScreenScript.as ~l.1690) l'appelle avec `Gameplay::GetTimeSeconds()` (même horloge que Expire). Pas de recentrage à faire : `HOLMinimapView.cpp:162-165` dessine déjà chaque carré centré (`At = Position - Size/2`). Les journaux `Minimap signals: N` et `Minimap signal mark at` sont inchangés. La fonction n'a pas pris `FHOLReceivedSignal` en entrée : ses champs sont BlueprintReadOnly (non construisible en script).
- D833 part script : `HOLAlertRules::PopulationFullLineSeconds = 15.0` (règle de durée testée) ; `RefreshPopulationAlert` appelle `PlayAlertSound()` après la ligne.
- Journaux : SignalsInProgress.md, AlertsMinorInProgress.md, ligne Auto_Journey_File.md. EtapeDefini.md non touché (manque d'appels : à faire par la session principale).

RESTE
- `AddThreadLine(HOLAlertRules::PopulationFullText(), HOLAlertRules::PopulationFullLineSeconds)` : NON fait, la signature à 2 arguments n'existe pas dans l'arbre (grep Source\ : seul `AddThreadLine(const FText&)`). Un TODO commenté marque l'endroit dans HOLGameScreenScript.as (RefreshPopulationAlert). Autre appelant à passer à 15 s : `HOLShortMessage.as:187`.
- Rouge vu à l'exécution des deux tests, puis balayage `signals_pulse_compare` si l'utilisateur veut choisir la forme.

SON (D804 B) : `Screen.PlayAlertSound()` joue `FHOLForumAlert::SoundPath` = `/Engine/EngineSounds/1kSineTonePing` (ping sinus 1 kHz PROVISOIRE, `HOLScriptScreen.cpp:865-868`, le même que l'alerte « attaque ailleurs » ; la voix du héraut doit le remplacer). Ce n'est donc PAS un son choisi mais l'unique son d'alerte existant : à confirmer par la session principale ; ligne Placeholders.txt à vérifier (non touchée). Attention : Q-alerts-minor réponse A disait « ligne silencieuse, pas de ping » (D449), D833 demande le son : appliqué tel que demandé.

NON VÉRIFIÉ : compilation script (`HOLSignalInputRead::PulseScale` vient du lot C++ de Glorfindel, non compilé non plus : si le binaire ne l'a pas, TOUT script qui l'appelle échoue, y compris HOLGameScreenScript.as), exécution des deux tests, `Gameplay::GetTimeSeconds` (double) passé à un `double`, rendu à l'écran du carré pulsant, que `PlayAlertSound` est visible depuis UHOLGameScreenScript (héritage : `class UHOLGameScreenScript : UHOLScriptScreen`, la méthode est sur UHOLScriptScreen, `HOLScriptScreen.h:382`). Les fichiers neufs ont des fins de ligne LF (git avertit « LF will be replaced by CRLF »).
PROCESSUS_DETACHES: non lancé (aucun processus lancé par cet agent)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs seuls modifiés par moi (SignalsInProgress.md, AlertsMinorInProgress.md, Auto_Journey_File.md, ce rapport) : oui
