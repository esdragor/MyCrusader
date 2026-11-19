# SAURON-1119e : HUD H-7, liaison + bascule de la file de production (2026-11-19)

Impact joueur : aucun changement visible attendu (file, barre de tete, note, clic annuler : memes noms, memes textes, meme place). NON COMPILE, NON LANCE. Binaire perime : la session principale compile apres.

## Commits (branche envelope, worktree HeroesOfLiberty-Sauron)
1. 0f9a3fb LIAISON : UHOLScriptScreen GetQueuePanel, GetQueueNow (delegue a UHOLGameScreen::ReadQueue, nouveau : RefreshQueue + UpdateHead en une lecture, dans HOLGameScreen.cpp car FindUnitType / LocalFactionKey y sont locaux), SetQueueReport (UHOLGameScreen::QueueReport, defaut "0 []"), GetAccentColor, CancelProduction(int) (choix : membre de UHOLScriptScreen plutot que de rendre RequestCancelProduction UFUNCTION : pas de toucher au controleur), SetQueueLive + evenement QueueFrameTick, RequestRefresh (C++ seul). HOLQueuePanel.as corrige : UHOLScriptScreen::SizedBar n'existe pas (SizedBar est une methode de la classe script HOLGameScreenScript) : boite construite sur place, hauteur 6 seulement comme le C++ (pas de BarWidth) ; ClickItem passe par Screen.CancelProduction ; Refresh appelle Screen.SetQueueLive(bQueueShown) ; methode FrameTick().
2. dc8540a BASCULE : HOLGameScreenScript.as : membre QueueView, Build apres PlayerTable (GetQueuePanel), Refresh, ClickWidget (apres TopBar), QueueFrameTick override. HOLGameScreen.cpp : plus de construction (QueueColumn..QueueHint, 42 lignes), plus de QueueItems/OnClicked, plus de UpdateHead par image, plus de RefreshQueue/UpdateHead dans Refresh (remplaces par ScriptScreen->RequestRefresh() : le script relit a l'image suivante au lieu d'attendre 250 ms), plus de boucle de clic QueueItems dans ClickWidget, Describe : "Queue = QueueReport". QueuePanel reste cree vide, meme slot, Collapsed.
3. RETRAIT : NON FAIT (limite d'appels). Reste a retirer apres grep : .cpp RefreshQueue, UpdateHead, HandleQueueItemClicked, QueueItemName, QueueItemWidth (usage unique restant : constexpr ligne 249), ; .h NumQueueItems, QueueItems, QueueProgress, QueueNote, EHeadState, HeadState, HeadProgress, ShownQueueTypes, bQueueShown, RefreshQueue, UpdateHead, HandleQueueItemClicked. GARDER QueueItemHeight. Le grep montre que seuls ces corps lisent encore ces symboles (RefreshQueue/UpdateHead/HandleQueueItemClicked sans appelant) ; HeadState/HeadProgress ne sont lus ni par la note Population maximale H-2 ni par IsProductionHeldForRoom (boucle propre sur les batiments) : seul Describe les lisait, remplace.

## Choix
- Cadence : le C++ appelle QueueFrameTick() dans NativeTick SEULEMENT quand bQueueLive (SetQueueLive(bQueueShown) ecrit par le script a chaque Refresh) ; meme procede que bMessageLive / FrameTick du message court. Le script y refait Refresh() (libelles reposes seulement si la signature change). Hors file : Refresh a 4 Hz + RequestRefresh (RefreshLeft = 0) a chaque Refresh C++ evenementiel.
- Journal : UHOLGameMenuScreen::MenuLog(ClickLine) inchange.
- Ligne queue N de hol.HUD.Report : mot pour mot via le script (ReportLine -> SetQueueReport -> Describe, seulement quand bGridShown comme avant).

## Table outils -> lignes (a jour)
quick_check 1379/1711 (hol.HUD.Click QueueItem2 / QueueItem0) : ClickWidget script -> QueueView.ClickName ; 3696 / 3700 / 6376 (queue N) : QueueReport ; alert_population.txt (note) : QueueNote script (HOLQueuePanelText.PopulationFull) ; journal HUD: queue item N clicked (cancel) : MenuLog (aucun outil ne le lit).

## Suspects de compilation
- TArray<bool>& en sortie d'UFUNCTION ; float& (float32 en script, fait) ; TArray<FString> sorties ; UFUNCTION BlueprintImplementableEvent QueueFrameTick surcharge oid QueueFrameTick() sans parametre dans le script ; GetOwnSelection() sur controleur const (comme Refresh) ; UHOLProductionSubsystem::GetBuildSteps / GetDisplayStep / HOLProduction::GetProgress / HOLCommandGrid::IsPopulationBlocked appeles dans ReadQueue const (memes inclusions que l'ancien UpdateHead, meme fichier) ; HOLScriptScreen.cpp : AHOLPlayerController::RequestCancelProduction public (include Player/HOLPlayerController.h deja la).
- Le .as : Screen.ConstructWidget(USizeBox...) ; la barre par MakeBar a un fond sans coins a comparer (fond 0.08/0.08/0.1 rond 2 du C++ remplace par le style de MakeBar, identique sauf remplissage blanc teinte par Accent).
- Mon incident : un chemin relatif a ete ecrit dans la copie principale (cwd .NET) : 2 fichiers de Source touches puis RESTAURES (git checkout) et le patch rejoue dans le worktree ; main est propre (verifie git status Source).

## Commandes de validation (session principale)
Build du worktree puis quick_check -Parts building_queue,hud (QueueItem0..4, queue N) ; lobby_match ; scenario de preuve avant/apres : non ecrit (Tools\Scenarios\building_queue.txt non lu).

## Champs
BUILDS_DEMANDES: 1 (lot C++ du worktree) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: ~92 (commit 2, hors retrait des corps) ; LIGNES_CPP_AJOUTEES: ~180 (liaison + ReadQueue) ; TEST_ROUGE: non vu (pas de build) ; PROCESSUS_DETACHES: aucun.