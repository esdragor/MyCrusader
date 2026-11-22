# ELROND-1122h : hol.Select.RightClickGround X Y

NON COMPILÉ (aucun build lancé, consigne de l'énoncé).

APPELS_DECLARES: ~16 (arrêt à 25)
HEURE_DEBUT/HEURE_FIN: 17:55 / ~17:58
PREMIER_BUILD: sans build   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (aucun build)
TEST_ROUGE: 70f77dd (bouchon ScriptRightClickAt vide + commande) ; assertit l'effet : non, il n'y a pas de test C++ : la fonction ouvre un contrôleur local avec viewport (ProjectGroundPoint, GetCursorFraction, CVarTestCursor) et n'est pas testable sans lui. Le rouge est le scénario : `waitlog Scripted right click` (absent avec le bouchon) ; l'effet réel (unité qui marche) est prouvé par deploy / order_lines_blocks après bascule.
TEST_ROUGE_SEPARE: oui (70f77dd bouchon seul, 0c8d5eb la fonction)
COMMITS: 2, dernier 17:57, merge_main_fait: non (merge integ-51 fait en entrée : 7152ed8, sans conflit)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
FAIT: commande hol.Select.RightClickGround <X> <Y> (HOLPlayerController.cpp, à côté de LeftClickGround) : projette le point sol en écran (ProjectGroundPoint), avertissement « the point is not on the screen » sinon ; HOLPlayerController::ScriptRightClickAt(const FVector2D&) (.h déclaré après ScriptLeftClick) : CVarTestCursor posée, journal « Scripted right click at (x, y), armed attack-move: ..., placing: ... », HandleOrder() puis HandleOrderCompleted() (les MÊMES gestionnaires que les liaisons Order Started / Completed, l. 842-843), curseur restauré. Nom distinct de ScriptRightClick() existant (HandleOrder seul, appui) pour ne pas surcharger.
Comportement identique à la souris : HandleOrder quitte la pose de bâtiment (EndPlacement + âge choisi), l'attaque armée, la bannière, la visée de faction SANS ordonner ; seul un clic droit sans mode armé donne l'ordre au relâchement (OrderAt, ou OrderLineAt si glissé : ici jamais, appui et relâchement au même point). Gel (pause) et spectateur : ignoré comme la souris.
RESTE: build C++ (Tools\build.ps1 -LiveCoding=0), puis la session suivante bascule deploy et order_lines_blocks sur `hol.Select.RightClickGround`.
NON VÉRIFIÉ: tout (non compilé). RELECTURE_CPP: signature .h / .cpp identiques ; ProjectGroundPoint et GetLocalControllers déjà utilisés par LeftClickGround juste au-dessus (mêmes portées) ; HandleOrder / HandleOrderCompleted privés mais appelés par un membre ; CVarTestCursor même espace de noms que ScriptLeftClick ; fins de ligne CRLF conservées, pas de BOM. Ce que le verbe ne prouve pas : que l'ordre est exécuté (seul le déplacement le dit), que la caméra est dans l'état supposé (pan/zoom change la projection : replacer avant), qu'un clic sur une unité ou un bâtiment (cible d'attaque, suivi, garnison) se comporte comme à la souris au sol (OrderAt lit ce qui est sous le curseur : un point sol sous une unité visée devient un ordre sur elle) ; l'ordre en ligne (glisser) n'est pas couvert.
PROCESSUS_DETACHES: voir stray_processes ci-dessous
ANOMALIES_PROCESSUS: aucune vue
GIT_STATUS_PRINCIPALE: docs seulement (EtapeDefini.md, Auto_Journey_File.md, ce rapport) : oui
