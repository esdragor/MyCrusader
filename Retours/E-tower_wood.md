# [À ESSAYER] E-tower_wood : la Tour de guet en bois lâche des flèches sur 3 Lanciers rouges à 12 m (petit trait orange + pointe sombre, planté près du Lancier de gauche)

STATUT : LOGIQUE SEULEMENT   (flèche vue sur PNG, mais geste interne et son non entendu : jamais listé prêt)
GESTE: interne
SON: non entendu
branches: integ-41

Pré-vol (2026-11-23 10:50, agent CELEBORN-1123h, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, commit 4d61f5d) : preflight.ps1 tower_wood check PASS (40 s, 6 PNG) ; le FAIL global n'était que « 2 fichiers non commités » (corrigé par le commit 4d61f5d).
PNG OUVERTS et vus :
- tower_wood_during : 3 Lanciers rouges au centre, petit trait orange à gauche de la tête du Lancier de gauche + hampe sombre : la flèche, minuscule à Zoom 2 (~20 px)
- tower_wood_during2 / during3 / during4 : le trait orange suit le Lancier de gauche qui avance vers la tour (la flèche suit sa cible ; ArrowReport : en vol 0, lancées 1, atterries 1)
- tower_wood_after : une flèche (hampe + pointe sombre, trait orange) à gauche d'un Lancier près de la tour ; barres de vie non lisibles
- tower_wood_before : NON ouvert
NON VÉRIFIÉ : flèche EN VOL (elle ne vit que 6 pas = 0,3 s à 12 m, la capture tombe après l'impact) ; PV entamés lisibles ; geste réel ; son ; que le worktree Sauron contient integ-41.

Commande complète : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 tower_wood -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
(Le Root Sauron doit contenir integ-41 : sinon jouer avec -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn.)
Durée : environ 1 min. Réponse attendue : OK, BUG: ... ou REGLAGE: ....