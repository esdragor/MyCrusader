# ELROND-1119c : scenario de preuve H-3 + greps de controle (worktree HeroesOfLiberty-Elrond, branche tiers)

Impact joueur : la barre du haut (ressources, population, inactifs) et les boutons Menu / Statistiques passes au script ont maintenant un scenario de preuve avant / apres et un essai de 3 min.

## 1. Scenario (commits ae6f006, b3065e4, 65da018 sur tiers ; seuls Tools\Scenarios\ecran_hud_hautdroite.txt / .look / .json)
- .json = copie de income_tooltip.json (Plain, toi + 1 IA, 5 Paysans, hero).
- Etat fixe : IA figee (hol.AI.ThinkSteps 100000), combat coupe, langue fr, 3 Paysans poses (idle, aucun revenu), camera FocusSlot own + 3000 cm, curseur pose en 960,600 avant (a).
- 5 etapes, waitkey avant chaque capture + notereset en francais : hud_haut_depart, hud_haut_triche (hol.Cheat.Wood 1000 + hol.Cheat.Gold 250 ; vrai nom : les cheats AJOUTENT, il n'y a pas de "set" ; HOLPlayerController.cpp:1890-1915), hud_haut_inactifs (hol.HUD.Click IdleWorkers, waitlog "Next idle worker"), hud_haut_survol (hol.Mouse.Hover WoodAmount, infobulle "+0 bois/min"), hud_haut_menu (hol.HUD.Click MenuButton, waitlog "Game menu: opened") puis Echap (hol.UI.Key Escape) et hud_haut_menu_ferme. Une ligne .look par capture, objet nomme.
- Pas de commande de pause utilisable : hol.Match.Pause existe mais pose une surcouche de pause (change le HUD) ; remplace par des Paysans immobiles (aucun revenu, montants fixes).
- Ce qui peut differer d'un passage a l'autre (comparaison pixel) : (1) phase d'animation de repos des 3 Paysans et de l'ouvrier selectionne sur hud_haut_inactifs / _menu / _menu_ferme (arriere-plan, pas la barre : comparer le rectangle de la barre du haut seul pour une preuve stricte) ; (2) valeurs de depart des ressources (fixes par la config, identiques entre binaires) ; (3) position exacte de l'infobulle si la fenetre n'est pas 1920x1080 (r.SetRes 1920x1080w est force) ; (4) le panneau du scenario (notes identiques entre binaires, donc ok).
- Pas de preflight lance (aucun binaire / lancement autorise : la session principale compile ce worktree).

## 2. Grep de controle des symboles retires par d5c333f (Source\ + Script\, hors HOLTopBar.as qui est le nouveau code)
Aucun appelant des symboles suivants : RefreshStatsButton, HandleMenuClicked, HandleStatsClicked, HandleIdleWorkersClicked, IdleWorkersName, MenuButtonName, StatsButtonName, IdleWorkersWidth, IdleWorkersBox, membres Food/Wood/Stone/GoldAmount (en tant que membres C++), MenuButton, StatsButton (C++).
Restes (tous vivants, voulus par le message du commit : Shown* pour Describe) :
- HOLGameScreen.cpp:1212 RefreshTopBar(PlayerState) ; :1255 definition (ne remplit plus que ShownResources / Population / Gatherers lignes 1264-1267) ; .h:295.
- HOLGameScreen.cpp:629, 1241 RefreshIdleWorkers() ; :1993 definition ; .h:326 ; .h:559 ShownIdleWorkers (lu en :2975 Describe).
- HOLGameScreen.cpp:2907 / .h:553-556 ShownResources, ShownPopulationUsed/Cap, ShownGatherers (lus par Describe).
- HOLGameScreen.cpp:1773 : chaines "FoodAmount".."GoldAmount" : cherchees dans ScriptScreen->WidgetTree (HoverIncome), c'est le nom des widgets du SCRIPT (HOLTopBar.as:188...) : bon, pas un reste.
- Commentaires seulement : HOLGameScreen.h:93, HOLUIHUD.cpp:27 et :216 (la chaine d'aide de hol.HUD.Click ne cite pas MenuButton / StatsButton : a completer a l'occasion), HOLUIMouse.h:16.
- HOLPlayerController.cpp:3580-3611 / .h:223,870 : IdleWorkers = membre du CONTROLEUR (autre chose), toujours utilise ; HOLTestSequence.cpp:359 le lit.
Rien a corriger d'urgence ; seul point a decider : RefreshTopBar/RefreshIdleWorkers sont des coquilles qui ne servent plus qu'a Describe.

## 3. hol.UI.Focus StatsButton (quick_check.ps1:768) : CERTAIN, rien a faire
HOLUIOpen.cpp:150-171 cherche le widget dans GetActiveModal() puis GetActiveMenu() du HOLRootLayout, jamais dans l'arbre du HUD de jeu. Au moment de quick_check.ps1:768 on est apres `waitlog End banner: Victory` + `waitui` : le menu actif est la banniere de fin (HOLEndBannerScreen.cpp:46 PushMenu), dont le script cree n"StatsButton" (Script\UI\HOLEndBannerScreen.as:29). Le commentaire de la ligne 767 dit d'ailleurs "from the banner's Statistiques". Le StatsButton du HUD (HOLTopBar.as:334) n'est pas dans la couche menu : il n'etait ni trouve avant d5c333f ni vise ici. Aucune correction.

## Etat
IN_USE.txt : libre, CPP: non. Pas de push, pas de merge. EtapeDefini.md / Auto_Journey_* / Placeholders / Retours non touches.
Commande de preuve pour la session principale : Tools\Scenarios\preflight.ps1 ecran_hud_hautdroite, puis play.ps1 ecran_hud_hautdroite -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond sur le binaire du worktree, puis la meme chose sur un binaire de main, comparer les 6 PNG.
NON VERIFIE : syntaxe des verbes (hol.Mouse.Hover 960 600 5, notereset, waitlog "Camera above..." deux fois, hol.UI.Key Escape ferme bien le menu de partie), tout le scenario n'a jamais tourne.
