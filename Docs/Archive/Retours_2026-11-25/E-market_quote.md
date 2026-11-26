# [JOUÉ OK] E-market_quote : les cases du Marché annoncent ce que l'échange rapporte vraiment (devis calculé par le script), et Q remet la croix rouge

STATUT : VU À L'ÉCRAN (marché) ; croix rouge de Q : LOGIQUE SEULEMENT (test HeroesOfLiberty.Grid.ArmedModeCursor, pas de capture du curseur)
GESTE: interne      (sélection par hol.Select.Click, touches Z / S par hol.UI.Key)
SON: sans objet
branches: market-quote-1125s

Pré-vol (2026-11-25 15:3x, CLAUDE-1125s, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, branche market-quote-1125s 1e73a88 sur main 78375b3) : `preflight.ps1 market -TestMap` PASS (4 PNG). Sans `-TestMap` : FAIL seulement sur « market.json has no mapSize » (D820, antérieur au lot).
PNG OUVERTS et vus :
- market_grid : Marché sélectionné, A Z E = +96 / +96 / +64, Q S D = +56 / +56 / +84, stock 2200 partout.
- market_bought : après Z (acheter du bois), or 2100, bois 2296 ; la case Acheter bois dit +92 (cours 83), Vendre bois +58.
- market_sold : après S (vendre du bois), or 2158 = +58, exactement ce que la case annonçait ; bois 2196 ; cases revenues à +96 / +56.
- market_1080 : en 1920x1080, la grille entière lisible, rien de coupé.
NON VÉRIFIÉ : la croix rouge de Q à l'écran (curseur logiciel non capturé) ; le désarmement de Q quand la sélection perd ses soldats ; hol.Select.Banner.

Ce qui change pour le joueur : rien de visible sur le Marché si tout va bien (mêmes chiffres qu'avant : le devis vient maintenant de la même règle que le serveur, plus de copie C++). Ce qui revient : depuis ce matin, la touche de l'attaque-déplacement ne montrait plus la croix rouge en attendant le point ; elle revient.

Commande complète (jouable tant que le worktree Faramir est sur market-quote-1125s ; après l'intégration, la même avec -Root C:\Users\baudo\Documents\MyCrusader) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 market -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
À faire : lire les chiffres des cases, acheter puis vendre du bois (Z puis S) et vérifier que l'or reçu est celui de la case ; puis sélectionner le héros ou des soldats, cliquer la case Attaque-déplacement de la grille : le curseur doit devenir une croix rouge jusqu'au clic au sol ou Échap.
Durée : environ 3 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Reponse: OK (utilisateur, 2026-11-25, essai joue sur la branche market-quote-1125s, worktree Faramir) : devis du Marche et croix rouge de Q.
