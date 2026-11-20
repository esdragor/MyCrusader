APPELS_DECLARES: 24 (arrêt à 25 vus ; fixé par l'énoncé)
HEURE_DEBUT/HEURE_FIN: 09:07 / 09:10 (Get-Date PowerShell)
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun build demandé)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aa1eae8 (Test_QueuePanelNoteDoesNotCoverTheBar, stub BarRightEdge = largeur de ligne) ; assertit l'effet : non (règle géométrique pure : bord droit de la barre calculé par la règle que Build utilise, pas de widget réel) ; rouge NON VU à l'exécution (aucun quick_check)
COMMITS: 2 (aa1eae8 test seul, 3ba1ebe correctif), dernier à 09:08, merge_main_fait: non (aucune consigne ; branche déjà avec 79a6c27 et 851188e intacts)
TESTS: aucun lancé (pas de build) ; TESTS_TAUTOLOGIQUES: 0 (la valeur assertie vient de BarRightEdge / NoteLeftEdge, non recopiée) ; TESTS_SCRIPT_AJOUTES: 1 (Test_QueuePanelNoteDoesNotCoverTheBar)
ESSAIS: aucun (pas de rendu, LOGIQUE SEULEMENT)
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (le lot C++ de la séance, voir RESTE)
TYPES_A_EXPOSER: aucun nouveau ; UHOLScriptScreen::AddThreadLine(FText, float32 Seconds) (surcharge ou paramètre)

FAIT / RESTE:
- Où vit la ligne : le texte et la règle de transition sont en script (HOLAlertRules.as ShouldAlertPopulationFull / PopulationFullText ; appel HOLGameScreenScript.as:1536-1537 AddThreadLine). La DURÉE est en C++ : FHOLForumThread::LineSeconds = 8.0 (static constexpr, Source\HeroesOfLiberty\Match\HOLForumMessages.h:234, Expire + GetOpacity), état du fil C++ (MessageThread), le script ne fait que dessiner (HOLMessageFeed.as). AddThreadLine (HOLGameScreen.cpp:603) ne prend aucune durée.
- C++ À GROUPER : durée par ligne (champ FHOLForumMessage::LineSeconds, 0 = 8 s ; Expire et GetOpacity la lisent), AddThreadLine(Line, Seconds) exposé au script (UHOLScriptScreen.cpp:471), puis script : HOLAlertRules::PopulationLineSeconds() = 15.0 passé en HOLGameScreenScript.as:1537. Le test d'effet à écrire alors (C++ ou script) : ajouter la ligne, avancer le temps à 14 s : la ligne est là ; à 15,5 s : elle a disparu.
- Son : Screen.PlayAlertSound() existe (HOLAlertBanner.as:126, FHOLForumAlert::SoundPath) mais je n'ai pas tranché si c'est le « son court D804 B » ; non branché (HOLGameScreenScript.as:1537 intact).
- FAIT (script) : la note « Population pleine » de la file ne recouvre plus la barre. HOLQueuePanel.as : la ligne de statut est un UHorizontalBox (barre Fill 1.0, note Auto avec 8 px à gauche, alignée à droite) au lieu d'un UOverlay ; règles HOLQueuePanel::NoteGap / BarRightEdge / NoteLeftEdge. HOLGameScreenScript.as non touché.
NON VÉRIFIÉ: rien compilé : FSlateChildSize (champs SizeRule / Value) et UHorizontalBoxSlot.SetSize ne sont pas vus compilés en Angelscript ; si « SetSize » échoue, remplacer par une marge droite fixe de la barre. Rouge non vu à l'exécution. Rendu à l'écran non vu (la barre pleine se rétrécit-elle à gauche de la note, la hauteur de la ligne 18 px garde-t-elle la note lisible). Le test est géométrique, pas un test de widgets : il ne prouve pas la mise en page réelle.
PROCESSUS_DETACHES: voir la sortie stray_processes (lancé en dernier geste, aucun éditeur lancé par moi)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non relu (docs seulement édités : Auto_Journey_File.md, AlertsMinorInProgress.md, EtapeDefini.md ; aucun code dans la copie principale)
