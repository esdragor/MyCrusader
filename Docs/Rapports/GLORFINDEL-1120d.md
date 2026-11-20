# GLORFINDEL-1120d : AX-3, edit_line.ps1 (sans C++, outil)

APPELS_DECLARES: 23 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 10:06 / 10:12
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans C++, aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 53b2f4b (branche ability-columns), vu a 10:09 : `edit_line_test : 4 FAIL` (AX-3a relu, AX-3c relu, AX-3e sortie coupee, AX-3f echec d'ecriture) ; assertit l'effet : oui (octets du fichier, code de sortie)
COMMITS: 2 (test 53b2f4b, correctif 29e415a), dernier a 10:11, merge_main_fait: non (merge integ-26 fait au premier geste)
TESTS: `Tools\Tests\edit_line_test.ps1` : 27 PASS, 0 FAIL, code 0 apres correctif ; TESTS_TAUTOLOGIQUES: 0 (les octets sont calcules par l'outil reel lance en processus)
ESSAIS: aucun (outil de suivi)
QUESTIONS: aucune

FAIT :
- Reproduction (fichiers temporaires du scratchpad, jamais un vrai fichier de suivi) : `-Append` seul, avec `-After`, texte a accents/backticks/`$`/`;`/`|`/`&`/3000 caracteres, appel par `powershell -File`, `-Command`, `cmd /c`, Git Bash, `rtk`, redirection `*>`, `2>&1`, `-DryRun` : TOUJOURS code 0. Le 255 vu ce matin n'a donc PAS ete reproduit tel quel.
- Reproduit en revanche (rouge) : `| Select-Object -First 1` coupe le pipeline AVANT l'ecriture (le script imprimait avant d'ecrire) : fichier non modifie, code -1/255. Une sortie qui n'arrive pas au lecteur ne devait jamais toucher l'ecriture ni le code.
- (b) `-Match` sur DEUX lignes : deja refuse sans ecrire (test AX-3b, vert avant et apres). (c) `-Match` dont le texte n'est que dans la ligne voisine : seule cette ligne change (AX-3c, vert avant et apres) ; l'ecrasement de D858 par la ligne CELEBORN (SAURON-1120d, "decalage d'index") n'a pas ete reproduit, le code d'index lu est correct sur les fichiers CRLF, LF et mixtes testes.
- Correctif (`Tools\AutoJourney\edit_line.ps1`, une seule source) : (1) l'ecriture precede toute impression, impressions via `Say` non fatal : une sortie coupee ne perd plus l'edition ni ne change le code ; (2) la ligne ecrite est RELUE dans le fichier et affichee (`relu : ...`) ; (3) si les octets relus ou la ligne i relue different : original restaure, code 3 ; (4) echec d'ecriture ou erreur inattendue : `edit_line : ECHEC (...)`, code 2 (trap), jamais 255 ; (5) succes = 0 ; fins de ligne et BOM conserves (inchange, tests 1/2/5 verts) ; `Parser::ParseFile` : 0 erreur ; l'outil reste ASCII sans BOM comme avant.
- Dogfood : la ligne AX-3 de `Auto_Journey_File.md` cochee par le nouvel outil (code 0, ligne relue affichee).

RESTE : `agent_rules.md` (section Outils L14) ne cite pas encore les codes 2/3 ni la ligne `relu :` (l'en-tete du script le dit) ; a ajouter par la session principale. Si un 255 reapparait avec la version corrigee, noter la commande exacte et le shell d'appel : la cause n'est pas dans le script.
NON VERIFIE : l'origine exacte du 255 du matin (non reproduit : hypothese = sortie ou pipeline coupe cote harness pendant l'impression, que le correctif rend sans effet sur l'ecriture et le code) ; la cause du decalage SAURON-1120d ; l'outil de la copie principale (`Tools\AutoJourney`) reste l'ancien tant que `ability-columns` n'est pas fusionnee.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs de suivi et rapports modifies/non suivis par d'autres sessions (AubeInProgress, AudioBuildings, Auto_Journey_File dont ma ligne AX-3, DuelInProgress, EtapeDefini, Placeholders, rapports) ; aucun fichier de code : oui
