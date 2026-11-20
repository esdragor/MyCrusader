# GLORFINDEL-1120g : saisie du chat (D807 Q-chat-ui = A), regle pure en script, 2026-11-20

Impact joueur : Entree ouvrira un champ de chat sur Allies (Tous sans allie), Tab basculera Allies > Tous > prive ; la REGLE est livree et testee, pas encore branchee a l'ecran (le joueur ne voit rien de neuf).

NON COMPILE (aucun build, aucun quick_check lance ; test jamais execute) : le rouge est a faire voir par la session principale.

APPELS_DECLARES: 13 (arret a 25 vus par moi)
HEURE_DEBUT/HEURE_FIN: 11:06:18 / ~11:12 (arret dur 11:25)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 012b424 (test + stub, avant le correctif c67c16d) + assertit l'effet : oui (chaque test finit dans HOLChat::Recipients : les slots qui liraient le message). Rouge NON VU a l'execution (pas de quick_check dans l'agent) : lancer `quick_check -Parts tests` sur 012b424 puis c67c16d ; attendu : 7 tests Test_ChatInput* rouges sur 012b424 (le stub ne fait rien : PressEnter = None, Tab/Escape = false, TargetSlot = slot choisi), verts sur c67c16d.
COMMITS: 2 (012b424 test + stub, c67c16d correctif), dernier a ~11:10, merge_main_fait: non (premier geste : `git merge integ-27` fait, avance propre sur la branche ability-columns)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (les lecteurs sont calcules par HOLChat::Recipients, jamais recopies)
TESTS_SCRIPT_AJOUTES: 7 (Script\UI\HOLChatInput_Test.as) : Test_ChatInputEnterOpensOnAlliesThenSends, Test_ChatInputWithoutAllyOpensOnAll, Test_ChatInputTabCyclesAlliesAllPrivate, Test_ChatInputPrivateOnlyWithAChosenPlayer, Test_ChatInputEscapeClosesWithoutSending, Test_ChatInputEnterOnABlankFieldDismisses, Test_ChatInputLostAudienceDoesNotWidenSilently
BUILDS_DEMANDES: 0 pour ce lot (un seul si un C++ est retenu, voir ci-dessous)
TYPES_A_EXPOSER: voir section Branchement (RequestChat non UFUNCTION ; capture de touches a verifier)
ESSAIS: aucun (aucune interface visible)
QUESTIONS: aucune (les 2 choix ci-dessous sont codes par defaut, une ligne chacun)

## Fichiers (worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, branche ability-columns)
- Script\UI\HOLChatInput.as : `struct FHOLChatInputState { bool bOpen; int Channel; }` et namespace `HOLChatInput` : `PressEnter(State, Text, bHasAlly, bHasPrivateTarget)` -> None / Opened / Send / Dismissed ; `PressTab(State, bHasAlly, bHasPrivateTarget)` ; `PressEscape(State)` ; `TargetSlot(State, ChosenSlot)` (slot choisi seulement sur Private, sinon -1). S'appuie sur `HOLChat::OpeningChannel` et `HOLChat::NextChannel` (deja dans Script\Chat\HOLChatRules.as, 1116a), donc une seule source pour l'ordre des canaux.
- Script\UI\HOLChatInput_Test.as : 7 tests, helpers `ChatInput*` (noms uniques, un global n'est pas visible d'un autre fichier).
- Dossier Script\UI (pas Dev/Editor/Examples) : charge sur client.

## Deux choix de l'agent (pas de design tranche, reversibles en une ligne)
1. Entree sur un champ ouvert VIDE ou blanc : ferme sans envoyer (Dismissed). Alternative : laisser ouvert. Ligne : le `if (Text.TrimStartAndEnd().IsEmpty())` de PressEnter.
2. Canal sans lecteur au moment d'Entree (l'allie est mort, le joueur choisi en Diplomatie a change) : Entree N'ENVOIE PAS, bascule sur Tous et laisse le champ ouvert (retour None) : le public ne s'elargit jamais sans que le joueur le voie (un texte prive ne part jamais en Tous d'un seul Entree). Ligne : le bloc `State.Channel = HOLChat::All; return None;`.

## Faisabilite du branchement : EN SCRIPT, avec deux inconnues a sonder (probablement 0 C++, 1 petit C++ au pire)
Constats lus (pas d'execution) :
- Aucun champ de saisie de chat n'existe : `grep Chat` dans Script\UI\ ne trouve rien, le chat n'a que `Script\Chat\HOLChatBoardScript.as` (pont serveur) et la ligne du fil (HOLMessageFeed).
- Un champ de texte est du C++ deja expose : `UEditableTextBox` est utilise par `HOLMultiplayerScreen.h:195` (NameBox) ; un `UEditableTextBox` se construit donc depuis le script (meme motif que les autres panneaux de HOLGameScreenScript.as) et `OnTextCommitted` (ETextCommit::OnEnter / OnCleared pour Echap) est un delegue dynamique lisible en script.
- ENVOI : `AHOLPlayerController::RequestChat(int32 Channel, const FString& Text, int32 TargetSlot)` (HOLPlayerController.h:579) est une methode C++ SANS UFUNCTION : inappelable du script telle quelle. Deux voies SANS C++ : (a) la commande de dev existante `hol.Chat.Say <all|allies|private> [<slot>] <texte>` par `Gameplay::ExecuteConsoleCommand` (chemin ServerPostChat -> pont HOLChatBoardScript, deja prouve par netsig3), (b) un RPC script `UFUNCTION(Server)` posant le meme appel (le portage des RPC en script est fait : Script\Net\HOLRpcPlayer.as contient deja le chat, voir HOLRpcSignalChat_Test). A confirmer en lisant HOLRpcPlayer.as : si une `ServerPostChat` script y est appelable, c'est la voie propre (zero C++). Sinon : exposer `RequestChat` en `UFUNCTION(BlueprintCallable)` = 1 ligne C++ dans le lot groupe.
- CAPTURE DES TOUCHES : la partie risquee. (1) OUVRIR sur Entree en jeu : Entree n'est pas liee dans le script de ce que j'ai lu ; il faut un point d'ecoute (action d'entree du controleur ou `OnPreviewKeyDown` d'un widget plein ecran du HUD). (2) TAB dans un champ : Slate traite Tab comme navigation de focus avant le widget ; il faut intercepter Tab au niveau du widget parent (`OnPreviewKeyDown` d'un `UUserWidget` script, retourner Handled) ; (3) FOCUS : `SetKeyboardFocus` du champ a l'ouverture et retour du focus au jeu a la fermeture (sinon les raccourcis du jeu restent bloques ou, a l'inverse, tapent dans le champ). Le seul `NativeOnKeyDown` C++ du dossier UI est `HOLDropdown.h:151` ; rien d'expose pour le HUD. Je n'ai PAS pu verifier si `UHOLScriptScreen` laisse passer les touches.
- Verdict : script d'abord (nouveau widget `Script\UI\HOLChatInputScreen.as`, sous-classe de UserWidget, jamais dans HOLGameScreenScript.as qui appartient a un autre agent : la regle pure se branche par `FHOLChatInputState` en membre et `HOLChatInput::Press*`). C++ seulement si la sonde montre que Tab/Entree n'arrivent pas au script : alors un petit relais Slate (`FInputPreProcessor` ou `NativeOnPreviewKeyDown` d'un widget de base) = lot C++ groupe, un build.
- Branchement minimal propose pour le prochain tour (script, ~1 appel de test) : (1) `HOLChatInputScreen.as` : construit Border + UEditableTextBox + TextBlock du canal (marque « (Allies) » / « (Tous) » / « (Prive) » deja codee dans la ligne de reception ; libelles : GDD, jamais inventes) ; (2) `OnPreviewKeyDown` : Entree -> `HOLChatInput::PressEnter(State, Box.Text, bHasAlly, bHasTarget)` puis, sur `Send`, envoi par la voie (a) ou (b) avec `State.Channel` et `HOLChatInput::TargetSlot(State, ChosenSlot)` ; Tab -> `PressTab` ; Echap -> `PressEscape` ; (3) `bHasAlly` = au moins un autre siege vivant en alliance mutuelle (meme lecture que `HOLChat::MatesFromTeams` / diplomatie), `ChosenSlot` = joueur choisi du panneau de Diplomatie (D387) : a lire dans le script de la diplomatie ; (4) scenario `chat_input` (touche reelle Entree, texte, Tab, Entree, ligne du fil du recepteur) = essai avec geste reel.

FAIT / RESTE
- FAIT : regle pure + 7 tests (2 commits), journal ChatInProgress.md, ligne d'Auto_Journey_File.md (marquee `[~]`), ligne d'EtapeDefini.md (ajoutee en fin), IN_USE.txt.
- RESTE : lancer `quick_check -Parts tests` (rouge sur 012b424, vert sur c67c16d) ; le branchement (script, sonde Tab/Entree/focus, voie d'envoi) ; scenario `chat_input`.

NON VERIFIE : tout a l'execution : la compilation de HOLChatInput.as et du test (formes copiees de HOLShortMessage.as / HOLChatRules_Test.as : struct a champs initialises, ref `FHOLChatInputState&`, `FString::TrimStartAndEnd().IsEmpty()`, `AssertEquals(int,int,...)` vu en usage ailleurs), le rouge du stub, la faisabilite de Tab/Entree/focus en script, l'appelabilite d'un envoi par RPC script (HOLRpcPlayer.as non lu).
PROCESSUS_DETACHES: voir derniere ligne ci-dessous (stray_processes lance en dernier geste)
ANOMALIES_PROCESSUS: aucune vue ; aucun processus lance par moi
GIT_STATUS_PRINCIPALE: hors docs voulus, inchangee : oui (modifies : Auto_Journey_File.md, EtapeDefini.md, ChatInProgress.md par moi ; le reste etait deja la : AubeInProgress, FortificationsInProgress, MoraleInProgress, SpecializationInProgress, NeutralsInProgress, et les Docs/Rapports/Retours non suivis d'autres agents)
