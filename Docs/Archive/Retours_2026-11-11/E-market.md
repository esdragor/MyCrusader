# [À ESSAYER] E-market : sélectionne le Marché, achète du bois (Z) puis vends du bois (S) et regarde les stocks et les cours de la grille changer

STATUT : VU À L'ÉCRAN (les trois PNG ouverts et vus par l'agent sur le binaire de main 39ba5ec, build PASS 461 s, 2026-11-11)
GESTE: interne   (le scénario sélectionne le Marché par `hol.Select.Click 0.5 0.5` puis déclenche les échanges par code de scénario, pas par une vraie touche Z / S au clavier du système)
SON: sans objet   (lancé sans son)
branches: aucune   (visual contient main 39ba5ec)

Pré-vol (2026-11-11 17:17, agent Sauron-R3, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron) : `preflight.ps1 market -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (28 s, 3 PNG, aucun WARN : le scénario appelle maintenant `hol.HUD.Report 1` et `hol.Test.Sanity`, audit passé).
Écarts de la fiche précédente levés (vérifiés dans le code, ce que le jeu affiche est la vérité) : (1) le stock affiché est 2200 de chaque = 200 de départ (`UHOLMatchSettings::StartingResources`) + 2000 de `hol.Cheat.Resources 2000` ; le `.look` disait 2000, corrigé ; (2) l'or reçu +58 est juste : l'achat monte le cours du bois de 3 points (80 -> 83), `SaleGold(83, 30 %) = 83 x 70 / 100 = 58` (56 au cours de départ 80) ; le `.look` le dit.
PNG OUVERTS et vus (dossier C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- market_grid.png : le Marché (bloc bleu) sélectionné, « Marché — Vous — Points de vie 1500/1500 » ; grille en bas à droite : ligne du haut A Z E Nourriture / Bois / Pierre « 100 or = 96 / 96 / 64 », ligne du dessous Q S D « 100 = 56 or / 56 or / 84 or » ; stocks en haut : 2200 de chaque.
- market_bought.png : Or 2100, Bois 2296 (100 or = 96 bois), la case Acheter bois affiche « 100 or = 92 » (le cours est monté) ; la case Vendre bois est trop petite pour être lue avec certitude (58 attendu).
- market_sold.png : Bois 2196 (-100), Or 2158 (+58) ; les cases de bois sont revenues à « 100 or = 96 » et « 100 = 56 or ».
NON VÉRIFIÉ : les vraies touches A Z E Q S D au clavier ne sont pas rejouées ; l'info-bulle de la case (phrase complète et cours) n'a pas de capture ; les chiffres de la grille sont très petits à cette résolution (la lecture de « 58 » dans la case de market_bought n'est pas certaine, la valeur vient du calcul et de l'or reçu à l'écran) ; le goût n'est pas jugé.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 market -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron -Hidden`
Durée : environ 1 min (Entrée à chaque temps). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.