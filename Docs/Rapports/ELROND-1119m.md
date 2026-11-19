# ELROND-1119m : HUD H-5b, le FIL DE MESSAGES en script (2026-11-19, reprise 14)

Impact joueur : aucun changement visible attendu (memes lignes, meme place, memes durees 8 s + fondu 0,5 s, meme clic qui centre la camera). Branche tiers du worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, 5 commits, NON COMPILE, NON LANCE (consigne : aucun build).

## Etapes faites (un commit chacune)
1. 941ec53 : Script\UI\HOLMessageFeed.as (namespace HOLMessageFeed = regles pures ; class UHOLMessageFeedPanel = vue) + Script\UI\HOLMessageFeed_Test.as.
2. fd6207e : liaison C++ = membres de UHOLScriptScreen (.h/.cpp) + 3 accesseurs et 1 compteur dans HOLGameScreen.h.
3. 3bdbe77 : crochet HOLGameScreenScript.as (membre MessageFeed, Build apres PlayerTable.Build, Refresh apres PlayerTable.Refresh, override MessageFrameTick, ClickWidget apres AlertBanner) + bascule : HOLGameScreen.cpp ne dessine plus le fil.
4. 0564669 : retrait du C++ mort (FillMessageRow, FocusAlertSlot et sa ligne de journal).
5. a41238e : scenario Tools\Scenarios\ecran_hud_fil.txt + .look + .json (copie de ecran_hud_alerte.json : mapSize Small).

## Conception (une seule source)
- RESTE C++ : la machine a etats du fil (FHOLForumThread : Add, Expire 8 s, 5 lignes, GetOpacity, Describe), la SOURCE des messages (HandleForumEvent, HandleTributeMessage, HandleSignalMessage, HandleHeroReadyMessage, HandleHeroLevelUp, AddThreadLine : ils appellent toujours MessageThread.Add puis RebuildMessages), TickMessages (Expire seulement), DescribeMessages (ligne "HUD messages:"). RebuildMessages = ++MessageRevision. Le conteneur MessagesList reste cree en C++ dans la RightColumn sous PlayersPanel (hote, comme PlayersPanel) ; le script y construit MessageLine0..4 / MessageRow0..4 (memes noms).
- Script = vue + clic + journal : UHOLMessageFeedPanel.Build(Screen), Refresh() (chaque image tant que le fil a une ligne : liste visible, lignes redessinees SEULEMENT quand la revision change, opacite lue chaque image), FillRow (mots en UHOLTextStyleHUDLabel, noms en UHOLTextStyleHUDPlayerName + couleur du joueur apres l'ajout a la rangee, comme FillMessageRow), ClickName(MessageFeed).
- Liaison C++ (UHOLScriptScreen) : UVerticalBox* GetMessagesList() const ; int32 GetMessageState(int32& OutRevision, TArray<bool>& OutAlerts, TArray<int32>& OutSlots, TArray<float>& OutOpacities) const (renvoie le nombre de lignes ; pas de tableau imbrique) ; ool GetMessageLine(int32 Index, TArray<FHOLForumPieceValue>&, TArray<FLinearColor>&) const (SplitNames, meme helper FillMessagePieces que GetAlertBanner, dont la boucle a ete factorisee) ; BindMessageClick(UBorder*) + HandleMessageMouseDown (bouton gauche -> ClickWidget(MessageFeed), comme HandleAlertMouseDown) ; evenement MessageFrameTick(float) appele par NativeTick tant que le fil C++ a une ligne et une fois de plus quand il se vide (bFeedLive). UHOLGameScreen : GetMessageThread(), GetMessageRevision(), GetMessagesList(), membre int32 MessageRevision.
- Clic : la ligne qui porte une alerte est Visible (les autres HitTestInvisible), comme avant ; le script trouve la 1re ligne survolee ET d'alerte (IsHovered) -> FocusCameraOnForum(slot) + ligne de journal. Un clic sans ligne survolee rend false (le clic passe, comme FEventReply(false)).

## Table outil -> texte lu -> ou le script l'ecrit (grep de Tools\ avant retrait)
| Outil | Texte lu / nom de widget | Apres H-5b |
|---|---|---|
| quick_check.ps1:3243-3245 (+ tribute_i18n.txt:9,16,23 waitlog) | HUD messages: (...) | C++ DescribeMessages() inchange (le fil reste l'etat C++) |
| quick_check.ps1:1672-1719, 4261-4289 | HUD message: <Cle> | message COURT (H-2, HOLShortMessage.as), pas le fil : inchange |
| quick_check.ps1:4715, 7189 | Hero level alert: slot N level A -> B (sound) | RESTE C++ dans HandleHeroLevelUp (source du fil) |
| quick_check.ps1:793, forum_messages.txt, quick_check:3240 | Alert: / hol.HUD.Click AlertPanel | banniere H-5a, inchange |
| alert_population.look:2 | HUD messages: ... Population pleine | idem 3243, C++ DescribeMessages() |
| (aucun outil) | HUD: alert of slot N clicked (camera on its forum) : grep vide dans Tools\ (la banniere H-5a l'ecrit deja par MenuLog) | ligne C++ de FocusAlertSlot RETIREE ; le script l'ecrit mot pour mot par UHOLGameMenuScreen::MenuLog (niveau Display) pour le clic de la banniere ET de la ligne du fil |
| (aucun outil) | MessageLine<i>, MessageRow<i>, MessagesList : grep de quick_check.ps1, lobby_match.ps1, Tools\*.ps1, Scenarios\*.txt/.json/.look vide ; hud_rects* : aucun fichier correspondant trouve par le glob | memes noms cree par le script (MessagesList reste C++) |

## Lignes C++ retirees
HOLGameScreen.cpp/.h/HOLScriptScreen : git diff fd6207e HEAD --shortstat = 19 insertions, 104 suppressions (3 fichiers, dont le crochet script) : BuildLayout (boucle des 5 lignes), RebuildMessages reduit a ++MessageRevision, boucle de fondu de TickMessages, HandleMessageLineMouseDown (.cpp + .h), FillMessageRow et son namespace, FocusAlertSlot (.cpp + .h) et sa ligne HUD: alert of slot, membres MessageLines / MessageRows. LIGNES_CPP_SUPPRIMEES ~104 ; C++ ajoute : ~110 lignes de liaison (au titre D839 : liaison a exposer).

## Tests script ajoutes (5, dans HOLMessageFeed_Test.as, jamais compiles)
Test_MessageFeedOnlyAnAlertLineUnderTheMouseTakesTheClick (HoveredAlertLine : survol sans alerte, ligne masquee, 1re gagne, fil vide) ; Test_MessageFeedClickLineIsTheCppOne (ligne de journal mot pour mot + egalite avec HOLAlertBanner::ClickLogLine) ; Test_MessageFeedLinesAndListFollowTheThreadCount (ListShown, LineShown, MaxLines=5) ; Test_MessageFeedRedrawsOnlyWhenTheThreadChanged (NeedsRedraw) ; Test_MessageFeedClicksAndPlaceAreTheCppOnes (LineTakesClick, PieceIsName, TopPadding ; les 3 dernieres assertions LineOpacity/PaddingX/PaddingY sont une garde de recopie des valeurs du C++ : TAUTOLOGIQUE par nature, la preuve du rendu est la capture). Aucun nom d'aide globale partage (toutes les fonctions de test sont Test_MessageFeed*). TESTS_TAUTOLOGIQUES: 1 (le garde de recopie). TESTS_CPP_RETIRES: 0 (les tests C++ de FHOLForumThread restent : la regle reste C++).

## A faire par la session principale (commande)
1. Build : <worktree>\Tools\build.ps1 -LiveCoding=0 -Wait (un build, lot C++ de l'etape 2/3/4), puis quick_check -Parts script (+ les parties hud / forum_messages / tribute_i18n / lobby).
2. Rouges a poser une fois dans le worktree (a remettre) : dans HOLMessageFeed_Test.as, HoveredAlertLine(Hovered, Alerts, 4) attendu 2 -> 3 (doit passer rouge) ; dans HOLMessageFeed.as, NeedsRedraw : != -> == (doit casser Test_MessageFeedRedrawsOnlyWhenTheThreadChanged) ; puis vert.
3. Scenario : play.ps1 ecran_hud_fil -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond AVANT (checkout de 3b52f7a : fil C++) et APRES (cette branche) ; decouper la zone haut-droite de hud_fil_ligne et comparer a l'oeil (libelle visible, couleur du nom, meme noirceur / padding).

## Pieges et suspects de compilation / comportement
- Rien n'est compile. Suspects : (a) TArray<bool>& et TArray<float>& en parametres de sortie d'UFUNCTION (aucun precedent exact ; AgePanels utilise TArray<bool> en script, pas en sortie C++) ; (b) UBorder.IsHovered() jamais appele en script dans le projet ; (c) class UVerticalBox* en retour d'UFUNCTION dans HOLScriptScreen.h (declaration elaboree) ; HOLScriptScreen.cpp n'inclut peut-etre pas Components/VerticalBox.h (non necessaire pour un simple pointeur) ; (d) Piece.SetStyle(UHOLTextStyleHUDPlayerName) / UHOLTextStyleHUDLabel : memes suspects que H-5a (SetStyle direct) ; (e) l'ordre d'appel : le fil C++ fait Expire dans UHOLGameScreen::NativeTick, le script dans UHOLScriptScreen::NativeTick (enfant) : au pire une image de decalage.
- Le panneau d'une ligne est cree par Screen.MakePanel + SetBrushColor(1,1,1,0.8) (meme motif que AlertPanel de H-5a) au lieu de FSlateRoundedBoxBrush(HUDPanel.CopyWithNewOpacity(0.8f), 4.f) : le rendu (rayon 4, couleur) n'est prouve QUE par la capture. SetBrushColor teint un brush deja fait par MakePanel ; si MakePanel ne donne pas le fond arrondi HUDPanel, utiliser Screen.SetRoundedBrush(Line, GetStyleColor(3) alpha 0.8, 4, transparent, 0).
- hol.Mouse.Hover MessageLine0 : syntaxe d'argument NON verifiee dans le scenario (leçon 5 dit que l'outil trouve les widgets de l'arbre du script par nom) ; hol.HUD.Click MessageFeed rend false sans ligne survolee. La ligne vit 8 s : sur un waitkey lent, le pas 2 (survol + clic) peut tomber sur un fil vide.
- Difference voulue : le fondu est lu par le script a chaque image (le C++ le posait a chaque image aussi : meme cadence). Le C++ remettait l'opacite a 1 a chaque reconstruction ; le script applique GetOpacity directement.
- La visibilite de MessagesList (SetShown dans l'ancien RebuildMessages) est maintenant posee par le script a chaque image ; BuildLayout la cree toujours Collapsed.
- ECART NOTE, NON CORRIGE : UHOLForumAlertHolder::ShowEvent passe SeesForum=false (HOLForumMessagesReadLibrary.cpp:160) alors que HandleForumEvent passe GameState->DoesViewerSeeForum(...). Je n'y ai pas touche : la source des messages du fil reste le C++ (HandleForumEvent), le holder n'est utilise par aucun nouveau code.
- Fichiers de suivi NON edites (consigne) : EtapeDefini.md, Auto_Journey_File.md, Auto_Journey_Jour.md, IN_USE.txt, *InProgress.md. Placeholders.txt : aucun provisoire ajoute ou retire.

## Question (aucun design tranche)
Aucune. (Choix a confirmer : les clics du fil passent par ClickWidget(MessageFeed) plutot qu'un nom par ligne ; recommande de garder.)

## Champs
BUILDS_DEMANDES: 1 (lot C++ : etapes 2 a 4 ensemble) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 5 (voir plus haut) ; LIGNES_CPP_SUPPRIMEES: ~104 ; LIGNES_MIGREES_PAR_LOT: ~240 (script 177 + 64 test, C++ de logique du fil retire ~100) ; TYPES_A_EXPOSER: aucun type, 5 fonctions + 1 evenement sur UHOLScriptScreen ; TEST_ROUGE: non vu (rien compile) ; TESTS_TAUTOLOGIQUES: 1 ; PROCESSUS_DETACHES: aucun lance.
NON VERIFIE : compilation C++ et script, les 5 tests, le rendu (libelle, couleur, padding), le survol et le clic, le fondu, le scenario ecran_hud_fil, les lignes de journal sous quick_check.