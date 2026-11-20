NON COMPILÉ (rien compilé ni lancé ; la session principale compile).

# GLORFINDEL-1120b : D860 H2, marque du signal au sol qui pulse

APPELS_DECLARES: 10 (arrêt à 45 vus)
HEURE_DEBUT/HEURE_FIN: 09:07 / 09:10 (Get-Date)
PREMIER_BUILD: non lancé (NON COMPILÉ)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (aucun build dans l'agent)
TEST_ROUGE: 39de039 (stub PulseScale = 1.0) + « assertit l'effet : oui » (C++ `Signals.GroundMarkPulse` : deux âges -> deux largeurs d'instance, rapport = règle ; script `Test_SignalPulseScale`). Rouge NON VU à l'exécution (à voir par la session principale).
COMMITS: 2 (39de039 test + stub, 1668abd correctif), derniers à 09:09, merge_main_fait: non (DLL non vérifiée, branche ability-columns, 91c0e1f et 86bca4e intacts)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (valeurs calculées par PulseScale / MarkLifetime / PulseMin / PulseMax)
ESSAIS: aucun
QUESTIONS: aucune (défaut codé : `hol.Signal.MarkStyle` 1 = pulse ; 0 = fixe d'avant)
BUILDS_DEMANDES: 1 (lot C++ du jour : `Tools\build.ps1 -LiveCoding=0 -Wait` sur `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel`, puis `quick_check -Parts tests`)
TESTS_SCRIPT_AJOUTES: 1 (Test_SignalPulseScale dans Script\Tests\HOLSignalPulse_Test.as) + 1 test C++ d'effet (HeroesOfLiberty.Signals.GroundMarkPulse)
TYPES_A_EXPOSER: aucun nouveau type ; 4 fonctions de UHOLSignalInputReadLibrary ajoutées (double PulseScale(double), double MarkLifetime(), double PulseMin(), double PulseMax()) -> script `HOLSignalInputRead::...`

FAIT
- `Player\HOLSignalInput.h/.cpp` : règle `PulseScale(AgeSeconds)` (sinus à nombre entier de cycles : 1 à la naissance et à l'expiration, monte à PulseMax, descend à PulseMin) ; `MarkLifetime()` (= hol.Signal.IconSeconds, 5 s), `PulseMin()` (clamp 0.1..1), `PulseMax()` (>= 1). Cvars : `hol.Signal.MarkStyle` (1 défaut, 0 = fixe), `hol.Signal.PulseMin` 0.8, `hol.Signal.PulseMax` 1.4, `hol.Signal.MarkCycles` 3. Défauts : Scale(0) = 1, Scale(T/4) = 0.8.
- `HOLSignalInputReadLibrary.h/.cpp` : 4 wrappers.
- `HOLSelectionMarkers.cpp` `UpdateSignalMarks` : `Size = BaseSize * HOLSignalInput::PulseScale(Now - Signal.ReceivedAt)` par signal, Now = GetWorld()->GetTimeSeconds() (même horloge que ReceiveSignal / Expire), trait recalculé par signal.
- Tests : script `Test_SignalPulseScale` ; C++ `Signals.GroundMarkPulse` (style 1 : tailles différentes et rapport = PulseScale(T/4)/PulseScale(0) ; style 0 : égales ; restaure le cvar).

RESTE : MINI-CARTE (non touchée, autre agent sur `HOLGameScreenScript.as`) : vers les lignes 1690-1705, remplacer le carré fixe 10 px par `SignalMark.Size = 10.0 * HOLSignalInputRead::PulseScale(Now - Signal.ReceivedAt)` (Now = horloge du jeu, la même que celle donnée à `HOLSignalInputRead::Expire` ; `ReceivedAt` est lisible sur FHOLReceivedSignal) ; recentrer le carré si sa position est son coin haut-gauche (taille variable). Puis compilation, rouge vu sur 39de039, balayage `signals_pulse_compare` (pulse / clignote / anneau) si l'utilisateur veut choisir la forme.

NON VÉRIFIÉ : compilation (signatures `double` en UFUNCTION BlueprintPure, `UE_DOUBLE_PI`, lambda de test avec `Marks` capturé const, inclusion `HAL/IConsoleManager.h`), exécution des deux tests, binding script (`HOLSignalInputRead::PulseScale` pas encore vu par le compilateur de script), rendu visuel de la marque, test existant `Signals.GroundMark` (la taille à l'âge 0 vaut BaseSize, inchangée) ; l'hypothèse que `MarkCycles` reste 3 (le test script suppose que T/4 n'est pas un point où le sinus vaut 0 : avec 2 cycles, Scale(T/4) = Scale(0) = 1).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: aucune (le dernier appel PowerShell a renvoyé le code 255 après avoir tout exécuté ; cause non cherchée)
GIT_STATUS_PRINCIPALE: docs seuls modifiés (SignalsInProgress.md, EtapeDefini.md, Auto_Journey_File.md par moi ; autres lignes d'autres sessions) : oui
