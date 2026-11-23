# SAURON-1123pa : S3-b, cause du curseur sans sol en fin de glisser de mur

APPELS_DECLARES: ~24 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 15:09 / ~15:20
PREMIER_BUILD: 15:10   BUILD: PASS (HeroesOfLibertyEditor Development, Sauron, 135 s, build lent 4 actions)   LAUNCHCHECK_DEBUT: binaire plus vieux que HOLPlayerController.cpp (11:45 < 15:10, ma propre edition) : build lance aussitot
TEST_ROUGE: sans objet (trace de mesure ; le `waitlog Drag end: foreground=` de wall_line_long.txt est le critere) ; assertit l'effet : non / TEST_ROUGE_SEPARE: non (mesure, pas de correctif)
COMMITS: 2 (8679ccd trace C++ + waitlog ; ca5b3ed scenario), dernier 15:15, merge_main_fait: oui (tip 3ddc875 contient main fc41498)
TESTS: wall_line_long rejoue 2 fois en -Hidden ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun liste ; wall_line_long reste LOGIQUE SEULEMENT (pas de preflight/PNG ouverts)
QUESTIONS: aucune
BUILDS_DEMANDES: 0 de plus   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun

## Fait
- 8679ccd : cvar `hol.Mouse.LogDragEnd` (0 par defaut) + une ligne `Drag end: foreground=.. viewport=.. size=.. mouse=.. (x, y) testCursor='..' fraction=.. ground=.. lastShown=..` dans `AHOLPlayerController::EndWallDrag` (HOLPlayerController.cpp, cvar pres de CVarTestCursor). Aucun changement de comportement, aucun en-tete. `GetCursorGround` : valeur de retour gardee dans `bGroundFound`, meme appel qu'avant.
- ca5b3ed : wall_line_long.txt active la cvar, attend la ligne (`waitlog Drag end: foreground=`), et (apres le 1er resultat) rend le curseur a la vraie souris.

## CONCLUSION (par le journal, 2 lancements)
Lancement 1 (scenario inchange, play.ps1 -Hidden) :
`Drag end: foreground=1 viewport=1 size=1280x720 mouse=1 (1700.0, 540.0) testCursor='0.5 0.5' fraction=1 (0.5000, 0.5000) ground=1 (-16400, 0) lastShown=(-16400, 0)` puis une ligne de (-16400,0) a (-16400,0), refusee.
- IsForegroundWindow = OUI au relache, meme cache (-RenderOffScreen). L'hypothese d'ARWEN-1123g (fenetre non au premier plan) est REFUTEE pour play.ps1 -Hidden.
- GetMousePosition = lue (1700, 540) : hol.Mouse.Drag injecte bien la souris.
- La vraie cause : `play.ps1 -Hidden` ajoute `hol.Camera.TestCursor 0.5 0.5` (play.ps1:66), et GetCursorFraction rend ce curseur de test AVANT de lire la souris : fraction constante 0.5,0.5, donc le glisser ne bouge jamais, depart = fin = centre ecran. (Le sol existe : ground=1.)
- Second defaut du scenario : le viewport cache fait 1280x720 ; les pixels 200..1700 du scenario (1920x1080) sortent de l'ecran (fraction 1.33 > 1, faux).
Lancement 2 (`hol.Camera.TestCursor off` + `hol.Mouse.Drag 133 360 1133 360 90`) :
`Drag end: foreground=1 ... mouse=1 (1133.0, 360.0) testCursor='off' fraction=1 (0.8852, 0.5000) ground=1 (-16400, 1881) lastShown=(-16400, 1839)` ; ligne (-16400,-1892) -> (-16400,1881) (~94 cases, plus de plafond) : le glisser fonctionne, le sol est trouve. MAIS le serveur la refuse : `place wall line refused (3) ... cell (-42, -2)` (aucune fondation) : la ligne part a x=-16400, sur le MainCentre du joueur (-16400,0), case de depart bloquee.

## Un vrai joueur peut-il le rencontrer ?
- TestCursor : non (vide chez un joueur ; seuls play.ps1 -Hidden et les scenarios le posent). Le defaut est donc un ARTEFACT DE SCENARIO.
- Fenetre au second plan : le journal ne l'a pas observe (foreground=1 toujours). Cas theorique restant : Alt-Tab pendant un glisser -> IsForegroundWindow faux -> GetCursorFraction faux au relache ; depuis ARWEN-1123f (68c86d0) la ligne s'arrete alors au dernier sol montre (WallDragEnd) : pas de pose d'UNE fondation. Non mesure ici (pas de test d'Alt-Tab possible en -Hidden).

## Lignes [ ] proposees (non ajoutees a la file)
- [ ] (S3-b, scenario) wall_line_long : partir de l'ecart du MainCentre (la 1re case de la ligne (-42,-2) est refusee : point d'appui a refaire, p.ex. x=-14000) ; refus de TOUTE la ligne sur une seule case bloquee : regle voulue (D867) ou a ouvrir ? (le serveur refuse la ligne entiere, `refused (3)`) ;
- [ ] (outil) play.ps1 -Hidden : ne pas poser TestCursor 0.5 0.5 quand le scenario utilise hol.Mouse.Drag, ou le scenario le met `off` (fait dans wall_line_long) ; autres scenarios a hol.Mouse.Drag sous -Hidden : meme piege.
- [ ] (facultatif, comportement) GetCursorFraction : aucun correctif necessaire pour -Hidden (foreground=1).

## RESTE / NON VERIFIE
- NON VERIFIE : Alt-Tab reel pendant un glisser ; aucune capture (PNG non ouverts) ; E-wall_line_long reste LOGIQUE SEULEMENT.
- Refus (3) de la ligne a cause de la case de depart : cause du code 3 non lue au code (pas de temps).
- DIAGNOSTICS_AS: sans objet (aucun .as touche).
- RELECTURE_CPP : relu, `bGroundFound` utilise, cvar dans le namespace UE::HeroesOfLiberty::Player (comme CVarTestCursor), `%s` sur un FString temporaire valide, build PASS.
