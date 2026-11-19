# [RÉPONDU] E-hud-message-court : la note « Population maximale — construisez des maisons » et le message court du HUD sont dessinés par le script et s'affichent comme avant

STATUT : VU À L'ÉCRAN
GESTE: interne   (le scénario clique lui-même le forum et appuie lui-même sur A ; l'utilisateur fera de vrais gestes ensuite)
SON: sans objet
branches: net-playtest

Panneau H-2 du HUD (D851, étape 4 de la migration) : le message court (texte qui apparaît 3 s puis s'efface en 0,5 s) et la note de production retenue faute de place passent du C++ au script (`Script\UI\HOLShortMessage.as`). Le fondu par image reste porté par le C++ : le script n'est appelé qu'aux changements, et à chaque image seulement pendant qu'un message est affiché.

Pré-vol (2026-11-18 22:56, session principale, worktree d'essai C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, net-playtest 8a5f28c, main 7d4ed6b) : `preflight.ps1 alert_population -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` PASS (69 s, 1 PNG).
Sur ce worktree : build PASS, tests 1550/1550 (rouge vu 23:01 sur les 7 tests du panneau : 1543/1550, une valeur faussée par test, remise), smoke, camera 2p / 8p, production PASS.
PNG OUVERTS et vus (APRÈS = script, comparé à AVANT = HUD C++ du worktree Haldir, même scénario, même résolution 1920 x 1080, 22:58) :
- alert_population : la note jaune « Population maximale — construisez des maisons » au-dessus du panneau de production, même place, même texte, même couleur ; barre des ressources 10 / 10 ; « Population pleine » au fil en haut à droite ; le 2e Paysan à 100 % ne sort pas ; identique à l'œil à l'écran C++.
Journal : la ligne `HUD message: PopulationFull "Population maximale — construisez des maisons"` est écrite par le script, mot pour mot celle du C++ (niveau Log au lieu de Display).
NON VÉRIFIÉ : l'effacement en fondu d'un message court ordinaire (« Pas assez de bois », file pleine), vu seulement par les tests de règle, pas à l'écran ; un message qui en remplace un autre ; la note peut apparaître jusqu'à 0,25 s plus tard qu'avant (lecture au rafraîchissement du HUD) ; `hud_rects_game_test.ps1` non lancé ; en réseau.
À noter, identique avant et après (pas une régression) : dans le panneau de production, le texte « Population pleine » chevauche la barre pleine (ligne de file déjà ouverte, D833).

Geste (3 min) :
1. Lance la commande ci-dessous, appuie sur Entrée quand le scénario le demande : il clique ton forum et commande deux Paysans.
2. Regarde : la note « Population maximale — construisez des maisons » apparaît au-dessus de la file de production quand le 2e Paysan est à 100 % et ne sort pas.
3. Fais toi-même un ordre refusé (par exemple commande un bâtiment sans assez de bois, ou remplis la file) : le message court apparaît, reste environ 3 secondes, puis s'efface en fondu.
4. Tue un Paysan ou construis une maison : le Paysan à 100 % sort aussitôt et la note disparaît.

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle, jamais « même commande avec… ».
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 alert_population -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel`
Durée : environ 3 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
Reponse: OK (utilisateur, 2026-11-19 ~00:40, dans la conversation ; essai joué sur le worktree d'essai ; bascule fusionnée ensuite, voir Auto_Journey_Jour.md « Agents du 2026-11-19 »)
