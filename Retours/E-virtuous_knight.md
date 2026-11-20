# [À ESSAYER] E-virtuous_knight : la case E de la Caserne Aube est le Chevalier Vertueux (active) ; un Chevalier en garde voit ses PV baisser quand un allié proche est frappé, sans qu'aucun ennemi ne le frappe

STATUT : VU À L'ÉCRAN
GESTE: interne      (sélection, garde, apparition des unités = commandes `hol.*` ; aucune vraie touche E ni vrai clic : l'essai reste LOGIQUE SEULEMENT pour le geste tant que la ligne dit interne)
SON: non entendu
branches: main f46dc2d (contient tool-known-reds a97f228 / 3bdad5f)

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`.

Pré-vol (2026-11-21 00:13, agent RECETTE-1120sb, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, main f46dc2d) : `preflight.ps1 virtuous_knight -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (21 s, 3 PNG).
PNG OUVERTS et vus :
- virtuous_knight_grid : Caserne Aube sélectionnée, case E « Chevalier Vertueux » active (même teinte que A et Z, pas grisée) ; journal `grid A=Halberdier enabled Z=ManAtArms enabled E=VirtuousKnight enabled`.
- virtuous_knight_before : panneau « Chevalier Vertueux », PV 155/155, anneau blanc de 8 m autour du Chevalier (bleu), allié à côté dans l'anneau, 3 ennemis rouges de l'autre côté de l'allié.
- virtuous_knight_hp : même Chevalier, même place (en garde, il n'a pas bougé), PV 148/155 ; journal : 0 ligne `hits #13` (le Chevalier est #13), les coups du journal visent l'allié (#14) : la baisse vient de la redirection D864/D865.
NON VÉRIFIÉ : le son (-nosound) ; une vraie touche E / un vrai clic sur le Chevalier (sélection par `hol.Select.Type`) ; le pourcentage exact redirigé (le Chevalier n'a aucune ligne de journal pour la part encaissée) ; que la baisse de 7 PV vienne bien de la redirection et pas d'une autre source (aucune autre cause trouvée au journal).

Commande complète (règle 13, 2026-11-14) :
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 virtuous_knight -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
