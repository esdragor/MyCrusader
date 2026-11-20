# GLORFINDEL-1120sa (2026-11-20 22:56-23:00) : chat_input, la 2e Entree ne rouvre pas le champ

NON COMPILE (script seul, aucun build, aucun quick_check). Rouge NON VU a l'execution.

APPELS_DECLARES: 12 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 22:56:38 / 23:00
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++ (correctif en script, C++ inchange)
TEST_ROUGE: 1a00e0f Test_ChatInputScreenCloseAfterSendReturnsFocusToGame (HOLChatInputScreen_Test.as) + « assertit l'effet : oui, a moitie » (compteur FocusReturns, proxy de l'appel de focus : l'effet reel demande le rejeu chat_input). Rouge attendu : Close() ne rend pas le focus (stub vide), FocusReturns 0 au lieu de 1. Non vu.
COMMITS: 3 (1a00e0f test + stub compilable, 3579205 correctif, journal ChatInProgress), dernier 23:00 ; merge_main_fait: branche deja a jour de main au depart, pas de merge refait
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
FAIT: analyse de la chaine ; test rouge, puis correctif : HOLChatInputScreen::Close() appelle ReturnFocusToGame() -> WidgetBlueprint::SetFocusToGameViewport() ; Close() protege contre Panel nul (test sans widgets).
CAUSE RETENUE (deduction, non prouvee) : apres l'envoi (frame 395) la zone de saisie est seulement repliee et garde le focus Slate ; le viewport perd le clavier ; l'Entree 2 arrive donc comme `Key pressed: Enter (focus: none)`, `down not handled` (RECETTE-1120pb 395-551) et WasInputKeyJustPressed (HOLScriptScreen.cpp:1090) ne se declenche pas. e3d1969 (garde 0,25 -> 0,1 s) ne pouvait rien : le defaut n'est pas dans la garde. HOLGameScreenScript.as non touche (zone Galadriel respectee).
RESTE : compiler les scripts ; quick_check -Parts tests ; rejouer `chat_input` seul (etape 39 `waitlog chat: open`) ; si encore KO : le relais C++ est la piste suivante (lot C++).
NON VÉRIFIÉ : (1) que `WidgetBlueprint::SetFocusToGameViewport` soit lie en Angelscript sous ce nom (UWidgetBlueprintLibrary::SetFocusToGameViewport existe en C++ UMG ; liaison non verifiee, pas de diagnostics AS lus : DIAGNOSTICS_AS non releve) ; (2) la cause (deduite du journal, jamais observee) ; (3) le rouge/vert a l'execution ; (4) le rejeu chat_input. Un echec de compilation de cette ligne bloquerait tout le script : a lire en premier.
ANOMALIES_PROCESSUS: aucune. L'Edit tool n'a pas ete utilise (ecritures par PowerShell, chaine exacte, chemins absolus) ; premier ecrit (IN_USE) a l'appel 8 (L20-8 limite).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
GIT_STATUS_PRINCIPALE: modifie hors docs voulus : non (seuls EtapeDefini.md et Auto_Journey_File.md ajoutes par moi ; les autres fichiers modifies/non suivis ne sont pas de moi)