# GALADRIEL-1119d : PopulationFullThread rouge apres H-8

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, branche galadriel-tab (base 049b7c8).

## Symptome
`HeroesOfLiberty.UI.Alerts.PopulationFullThread` (HOLPopulationAlertThreadTests.cpp:74) : 1 ligne au lieu de 2 au pas de 135 s. Les 4 premieres verifications passent.

## Hypotheses testees
1. Crochet `AgePanels.Refresh()` (Refresh du script) leve ou consomme une ligne : crochet retire (edition temporaire), quick_check -Parts tests : TOUJOURS rouge (meme message). Hypothese ecartee.
2. `AgePanels.Build` leve et coupe la suite d'OnInitialized : essai 2 INVALIDE (j'ai edite des scripts pendant le run : « RefreshAges does not exist in superclass », 1586/1588). Non rejoue : la cause trouvee par lecture est confirmee par le vert ci-dessous. Aucune exception de script dans les journaux.
3. Lecture du code : le C++ retire n'ecrivait aucune ligne de fil ; aucun `ShowMessage`/`Thread` dans le retrait. Ecartee.

## Cause
H-8 (83da6fa) a remplace dans `UHOLGameScreen::RefreshGrid` l'appel `RefreshAges(PlayerState)` (panneaux d'ages seuls) par `ScriptScreen->RefreshNow()`, le Refresh COMPLET du script (idem dans NativeTick pour le palier et l'age choisi). `RefreshGrid` est declenche par le changement de population (`Player->SetPopulation`). Le Refresh complet contient `RefreshPopulationAlert`, qui lit l'heure reelle du HUD. Dans le test, `SetPopulation(10,10)` precede la pose de `World->RealTimeSeconds` : l'alerte est donc evaluee a l'heure PERIMEE (120 s : 19 s apres la 1re ligne, sous les 30 s) , ne part pas, mais pose `Watch.bWasFull = true` ; au vrai pas (135 s) `DescribeScript` voit « deja pleine » : pas de 2e ligne. Dans le jeu l'heure est courante, donc pas de defaut visible, mais un Refresh complet a chaque changement de grille etait aussi plus lourd que le RefreshAges d'origine.

## Correction (sans toucher au test)
- `UHOLScriptScreen::RefreshAges()` (BlueprintImplementableEvent, HOLScriptScreen.h) surcharge dans HOLGameScreenScript.as : `AgePanels.Refresh()` seul.
- HOLGameScreen.cpp : RefreshGrid, le palier et l'age choisi appellent `RefreshAges()` au lieu de `RefreshNow()` ; `RefreshNow` reste pour `DescribeScript` (hol.HUD.Report).
- Defaut voisin vu : `ShownTier` n'etait plus ecrit apres le retrait (la condition de NativeTick `GetTier != ShownTier` restait vraie a chaque image) : il est maintenant ecrit au moment de l'appel.

## Resultat
Commit 3076130 (galadriel-tab, anglais, sans push ni merge). Build PASS (54 s), quick_check -Parts tests : 1588/1588 PASS (131 s). Le test C++ est inchange.
Reste : scenario de preuve de H-8 (captures avant/apres des panneaux d'ages) NON rejoue ici ; le test d'ages en script n'a pas de cas sur RefreshAges (appel C++ seul). NON VERIFIE : l'essai 1 ecarte seulement le crochet Refresh, pas la sequence complete (cause confirmee par le correctif, pas par un essai d'isolement a part).
