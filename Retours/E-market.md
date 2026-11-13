# [À ESSAYER] E-market : les cases du Marché disent « +X » suivi du carré de couleur de la ressource reçue (D781), sans « 100 or = X »

STATUT : VU À L'ÉCRAN (le libellé des cases ; l'achat et la vente du scénario passent par la commande interne)
GESTE: interne (le scénario achète et vend par `hol.Select.Grid`, pas par la touche Z / S ; à toi d'essayer A Z E / Q S D et le clic sur les cases)
SON: non entendu (lancé avec -nosound)
branches: galadriel-tab

Ce qui change (D781) : ligne du haut A Z E = Acheter : « +96 » puis le carré de la ressource (rouge nourriture, brun bois, gris pierre) ; ligne du dessous Q S D = Vendre : « +56 » puis le carré doré de l'or. Le nom de la ressource reste en haut de la case (sinon les trois cases Vendre seraient identiques). Le carré tient lieu de logo tant que les icônes de ressources n'existent pas (Placeholders.txt « Icônes des ressources »). L'info-bulle garde la phrase complète pour 100 et le cours.

Pré-vol (2026-11-13 17:25, agent Galadriel, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, branche galadriel-tab b372781 + b711090 sur main 8d13f46) : `preflight.ps1 market` PASS (28 s, 3 PNG).
PNG OUVERTS et vus :
- market_grid : A « Nourriture +96 [carré rouge] », Z « Bois +96 [brun] », E « Pierre +64 [gris] » ; Q « Nourriture +56 [or] », S « Bois +56 [or] », D « Pierre +84 [or] » ; aucun « 100 ».
- market_bought : après l'achat de bois, bois 2296, or 2100 ; Z dit « +92 », S dit « +58 » (cours du bois monté).
- market_sold : après la vente de bois, bois 2196, or 2158 (+58) ; Z et S reviennent à « +96 » / « +56 ».
NON VÉRIFIÉ : vraie touche Z / S et vrai clic sur une case (le scénario passe par la commande interne) ; Maj + case (5 lots) ; lisibilité du carré de couleur à côté du chiffre sur un autre écran ; textes en / ja (« +96 » n'est pas un texte à traduire) ; le message « Shaders en préparation » du moteur passe sous le panneau Triches au premier PNG (moteur, pas le Marché).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 market -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
