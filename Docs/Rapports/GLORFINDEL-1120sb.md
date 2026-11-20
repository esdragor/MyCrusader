# GLORFINDEL-1120sb : integ-33, module Angelscript qui ne compile plus

Joueur : le chat du jeu (2e Entree) reste reparable ; sans ce correctif aucun script ne se charge (0/0 tests).

ERREUR : Script\UI\HOLChatInputScreen.as:186:4, "No matching signatures to 'WidgetBlueprint::SetFocusToGameViewport()'", puis "Hot reload failed due to script compile errors. Keeping all old script code." (Integrate_integ-33.log, ligne `[KO] scripts compiled`). Commit fautif : 3579205 (ability-columns, Glorfindel). Les tests Siege.Deploy rouges dans la meme sortie suivent du meme echec (anciens scripts gardes).

CAUSE : le plugin Angelscript retire le suffixe `BlueprintLibrary` du nom de classe (Source\AngelscriptCode\Public\AngelscriptSettings.h:131-139, Private\Binds\Helper_FunctionSignature.h:138-190). `UWidgetBlueprintLibrary` devient `Widget::` ; `WidgetBlueprint::` n'est que `UWidgetBlueprintStatics` (WidgetBlueprintStatics.h : CreateWidget seul). Le namespace WidgetBlueprint existe donc, mais pas la fonction : "No matching signatures". La fonction C++ existe : WidgetBlueprintLibrary.h:68 `static void SetFocusToGameViewport();` (BlueprintCallable, sans parametre).

CORRECTIF : 728ee41 sur ability-columns (worktree Glorfindel) : `Widget::SetFocusToGameViewport();` (HOLChatInputScreen.as:187), compteur FocusReturns garde, test inchange. Commit anglais sans Co-Authored-By, pas de push ni de fusion.

SYMBOLE DEJA EMPLOYE ? NON : aucun usage de `Widget::...` ni d'un autre moyen de rendre le focus au jeu dans Script\ ou Source\ (seul SetInputMode(FInputModeGameAndUI) en C++, HOLPlayerController.cpp:787, non appelable depuis le script). Le nom est derive du code du plugin, pas d'un usage existant : a confirmer a la compilation.

NON VERIFIE : compilation du module apres correctif ; effet reel du focus (a rejouer chat_input seul) ; d'autres erreurs de compilation qui seraient masquees derriere celle-ci (Galadriel fd9a795/ae69298 non examines : le log ne signale que cette erreur).
DIAGNOSTICS_AS : non relus (aucun serveur de langage dans cet agent).
ANOMALIES : le test Test_ChatInputScreenCloseAfterSendReturnsFocusToGame passe par Screen != nullptr ; si Screen est nul dans le test, FocusReturns monte sans appel reel (comportement existant, non modifie).
Appels : ~20 (arret dur 23:38 respecte, fin 23:26). Aucun build, quick_check ni editeur.
