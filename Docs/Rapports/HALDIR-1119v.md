# HALDIR-1119v : corrections de la bascule du bloc de selection (worktree HeroesOfLiberty-Haldir, branche selection)

Commit : a15d5a4 (HOLGameScreen.cpp seul, 16 + / 5 -).

## Cause commune aux defauts 1 et 2
Les widgets du bloc de selection du script (SelectionName, SelectionStats, SelectionStat0..5) sont CONSTRUITS par l'ecran script (proprietaire = ScriptScreen->WidgetTree)
mais ACCROCHES au SelectionHost de l'ecran de jeu (UHOLScriptScreen::GetSelectionHost, HOLGameScreen.h:129). UWidgetTree::FindWidget part de la racine de l'arbre :
chercher dans ScriptScreen->WidgetTree ne les trouve donc pas (HOLScriptScreen.cpp:549 cherche deja ses cellules d'age dans GameScreen->WidgetTree, pour la meme raison).
Les lectures du texte de stats passaient (miroir SetStatsReportMirror), pas celles qui lisent des widgets.

## Defaut 1 (HoverStat) : corrige
HOLGameScreen.cpp HoverStat (ex-l.941) cherchait SelectionStat<i> dans l'arbre du script -> nullptr -> `no statistic shown`.
Correction : aide FindSelectionWidget (HOLGameScreen.cpp:~918) qui cherche d'abord dans WidgetTree de l'ecran de jeu puis dans celui du script ; utilisee aussi par DescribeStatsTip (SelectionStats, SelectionStat<i>).
Journal obtenu : `LogHOLUI: Display: HUD hover 0: attack: 8` (scenario_ecran_hud_selection_unite.log:2250).

## Defaut 2 (DescribeSelectionName) : corrige
Meme cause (HOLGameScreen.cpp:~1875, SelectionName). Journal : `HUD selection name: Lancier` (l.2232), `Paysan` (l.2276, 2368) ; `none` pour le groupe de 5 paysans (l.2318), comme en C++ (le nom cede la place aux puces).

## Defaut 3 (points « · » decales de 1 a 2 px) : non corrige, cause non trouvee
Compare au C++ d'origine (main:HOLGameScreen.cpp:1663-1680 et 2741-2748) : meme separateur (octets identiques c2 b7, `"   ·   "`), meme regle (separateur colle au texte sauf derniere puce ou mode 2),
meme WrapBox `SelectionStats` avec padding (0,2,0,0), meme style UHOLTextStyleHUD (SetStyle apres construction, MakeText de HOLScriptScreen.cpp:424). Rien ne differe dans le script
(HOLSelectionUnit.as:104, 257-262, 438-448, 530). Piste restante, non verifiee : texte pose en FText::FromString (script) contre FText::AsCultureInvariant / FText::Format (C++), ou
largeur de puce arrondie differemment (decalage sub-pixel de mise en page du WrapBox). 26 px sur 0,006 % de la zone : visuellement negligeable.

## Pre-vol et tests
- build.ps1 : PASS (92 s).
- preflight ecran_hud_selection_unite : `check ecran_hud_selection_unite : PASS (43 s)`, `preflight ecran_hud_selection_unite : FAIL (1)` (le seul FAIL = Crown.umap non suivi, attendu) ; les 5 captures presentes et copiees dans scratchpad\apres\ecran_hud_selection_unite\ (ecrase).
- quick_check -Parts tests : `tests 1640/1640 : PASS`.

## Reste
- Defaut 3 (decalage 1-2 px des points) : a regarder a l'echelle 1 avec un essai FText::AsCultureInvariant cote script si le decalage gene.
- Les commandes Tools\... Click / Mouse / Open (Dev\HOLUI*.cpp) cherchent par `Screen->WidgetTree->FindWidget` : a verifier pour les widgets du bloc de selection si un scenario les nomme (aucun ne l'a fait echouer ici).
