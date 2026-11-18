# [À ESSAYER] E-hud-bannieres : les bannières « Bob : connexion perdue — m:ss » et « Bob : de retour » du HUD sont dessinées par le script et s'affichent comme avant

STATUT : VU À L'ÉCRAN
GESTE: interne   (le scénario ferme lui-même son panneau et simule le départ puis le retour de « Bob » par la commande hol.Net.FakeAbsent ; aucun vrai joueur ; l'utilisateur fera ses propres gestes ensuite)
SON: sans objet
branches: galadriel-tab

Panneau H-1 du HUD (étape 4 de la migration) : les bannières d'attente (« En attente de Bob (m:ss) ») et d'absence (« Bob : connexion perdue — m:ss », « Bob : de retour ») passent du C++ au script (`Script\UI\HOLWaitBanner.as`, crochet dans `HOLGameScreenScript.as`). Ce scénario ne montre que la bannière d'ABSENCE : l'attente exige un vrai lobby nommé (voir NON VÉRIFIÉ).

Pré-vol (2026-11-18 23:07, agent GALADRIEL-1118v, worktree d'essai C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, galadriel-tab, scénario commité 269d572) : `preflight.ps1 ecran_hud_bannieres -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` PASS (36 s, 4 PNG) ; panneaux du HUD sans recouvrement, sanity audit passé. Binaire à jour : tests 1548/1548, quick_check net PASS à 22:59.
PNG OUVERTS et vus (APRÈS = script, Galadriel ; AVANT = HUD C++ du worktree Haldir, branche maps, même scénario, 1920 x 1080, pré-vol 23:06) :
- hud_banniere_absence : bandeau sombre sous la barre du haut, centré, « Bob : connexion perdue — 1:30 », même place, même taille, même fond, même police, même texte que le C++ ; ressources, fil de messages, mini-carte inchangés. Les trois nuages blancs derrière sont du décor, au même endroit avant et après.
- hud_banniere_retour : « Bob : de retour », même bandeau resserré autour du texte, même place avant et après.
- hud_banniere_aucune et hud_banniere_fin (APRÈS seulement ouverts) : aucun bandeau ; sur « aucune » seul le petit panneau du scénario (« en préparation… ») est sous la barre du haut.
Journal, mot pour mot identique avant et après : `Absence banner: shown, Bob : connexion perdue — 1:30`, `Absence banner: shown, Bob : de retour`, `Absence banner: hidden` (tiret cadratin intact dans le journal du script).
NON VÉRIFIÉ : la bannière d'ATTENTE d'une vraie partie réseau (« En attente de Bob (1:5x) »), vue seulement par `quick_check` partie net (capture qcnet_2_wait), pas de scénario solo ; le salon : l'absence d'un vrai joueur (lobby_match, deux scénarios rejoin) ; le compte à rebours qui baisse à l'écran (une seule valeur vue, 1:30) ; les captures AVANT « aucune » et « fin » (non ouvertes) ; la bannière sous une vraie souris (clic au travers).
À noter, sans rapport avec les bannières : dans les captures AVANT (Haldir) le panneau des Triches est replié (« Triches + »), dans les captures APRÈS (Galadriel) il est déplié (« Triches - ») ; le diff galadriel-tab contre maps ne touche que les bannières, donc réglage propre à chaque worktree à confirmer.

Geste (2 min) :
1. Lance la commande ci-dessous ; le scénario te demande d'appuyer sur Entrée, puis ferme lui-même son panneau.
2. Regarde sous la barre du haut, au centre : « Bob : connexion perdue — 1:30 » et le compte à rebours qui baisse d'une seconde par seconde.
3. Quelques secondes plus tard : « Bob : de retour » pendant 5 s, puis le bandeau disparaît.
4. Ensuite, dans la console : `hol.Net.FakeAbsent Alice 30` (bandeau 30 s), `hol.Net.FakeAbsent` seul l'efface ; clique le sol sous le bandeau : la souris passe à travers.

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle, jamais « même commande avec… ».
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 ecran_hud_bannieres -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
