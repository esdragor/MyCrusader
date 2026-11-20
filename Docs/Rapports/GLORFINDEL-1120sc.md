# GLORFINDEL-1120sc (chat Prive Tab Tab + marque)
APPELS_DECLARES: ~14 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 23:55 / ~00:00
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 15c46b4 (stub ChosenTarget=-1 + Test_ChatInputScreenClickedPlayerThenTabTabIsPrivate) ; assertit l'effet : oui (canal Prive, marque = nom, cible = slot) ; rouge NON VU a l'execution (aucun quick_check lance) ; le rouge et le vert sont en deux commits (consigne)
COMMITS: 3 (15c46b4 rouge, 7b9f5b4 correctif, docs), merge_main_fait: non necessaire (a jour avec c57e12c)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (ChosenTarget calcule la cible, canal/marque/cible via PressEnter/PressTab/MarkLine/TargetSlot)
ESSAIS: aucun rejoue (consigne)
QUESTIONS: aucune
CAUSE: la regle pure (HOLChatInput.as PressTab/NextChannel) est correcte avec cible. Deux causes plausibles, une seule confirmable sans jeu :
 - MARQUE INVISIBLE (probable, corrige) : HOLChatInputScreen.as Build : UBorder au pinceau blanc par defaut + UCommonTextBlock sans style = texte clair sur blanc ; correctif = SetBrushColor sombre + SetStyle(UHOLTextStyleHUDLabel).
 - PRIVE NON ATTEINT (NON CONFIRME) : le contexte (ChosenSlot) n'est pousse que par Refresh (0.25 s) ; correctif defensif : RefreshChatInput au clic (HOLGameScreenScript.as ~948) et avant ChatInput.KeyPressed (~2542), ChosenTarget extrait. Tab n'est PAS lu par le polling C++ (HOLScriptScreen.cpp:1086-1099 : T, Enter, CtrlD) : il arrive seulement par OnUserMovedFocus de la boite ; si le journal rejoue montre 'ChatInput: Tab (open True) ally True chosen 1 -> channel X' avec X != Private, la cause est le Tab Slate (perdu/double), pas la cible. Pas de cause C++ identifiee.
FAIT/RESTE: fait test+correctif+diagnostics (log Tab, Describe '(ally X, private target N)') ; reste : compiler, quick_check -Parts script, rejouer chat_input, lire la ligne 'ChatInput: Tab'.
NON VERIFIE: tout (aucun build, aucun test, aucun essai) ; que la marque devienne visible ; que ChosenSlot etait vraiment -1 au Tab.
DIAGNOSTICS_AS: non releves (aucune lecture du serveur de langage) ; verifier compilation : Panel.SetBrushColor, MarkText.SetStyle(UHOLTextStyleHUDLabel), TArray<int>.Contains, f-string avec bool.
PROCESSUS_DETACHES: voir stray_processes ci-dessous
GIT_STATUS_PRINCIPALE: non releve (docs seulement : EtapeDefini.md, ce rapport)