# [À ESSAYER après correction du build] E-market : sélectionne le Marché, achète du bois (Z) puis vends du bois (S) et regarde les stocks et les cours de la grille changer

STATUT : VU À L'ÉCRAN (les trois PNG ouverts et vus par l'agent ; binaire Editor du 2026-11-11 14:26, voir NON VÉRIFIÉ)
GESTE: interne   (le scénario sélectionne le Marché par `hol.Select.Click 0.5 0.5` puis déclenche les échanges par code de scénario, pas par une vraie touche Z / S au clavier du système)
SON: sans objet   (lancé sans son)
branches: aucune   (visual = main 521adcb)

Pré-vol (2026-11-11 15:20, agent Sauron-F, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, main 521adcb) : `preflight.ps1 market -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (22 s, 3 PNG ; deux WARN : le scénario n'appelle ni `hol.HUD.Report` ni `hol.Test.Sanity`, donc ni chevauchement de panneaux ni audit Sanity vérifiés).
PNG OUVERTS et vus (dossier C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- market_grid.png : le Marché (bloc bleu) sélectionné, « Marché — Vous — Points de vie 1500/1500 » ; grille en bas à droite : ligne du haut A Z E Nourriture / Bois / Pierre « 100 or = 96 / 96 / 64 », ligne du dessous Q S D « 100 = 56 or / 56 or / 84 or » ; stocks en haut : 2200 de chaque.
- market_bought.png : Or 2100, Bois 2296 (achat de 100 or = 96 bois), la case Acheter bois affiche « 100 or = 92 » (plus petit : le cours est monté) ; le panneau du scénario est revenu au-dessus.
- market_sold.png : Bois 2196 (−100), Or 2158 (+58) ; la case Vendre bois reste lisible (« 100 = 56 or »).
Écarts à signaler (pas jugés) : (1) le fichier `.look` dit « stock de 2000 » alors que la barre affiche 2200 de chaque ; (2) l'or reçu à la vente (+58) est lu à l'œil comme différent du 56 affiché avant (les chiffres de la grille sont très petits à cette résolution, lecture incertaine) ; (3) la grille est lisible mais les cases sont petites.
NON VÉRIFIÉ : binaire PÉRIMÉ (le build de main 521adcb échoue : HOLCombatRules.h(403) C4459 « Margin ») ; captures du binaire du 14:26 ; les vraies touches A Z E Q S D au clavier ne sont pas rejouées ; l'info-bulle de la case (phrase complète et cours) n'a pas de capture ; les WARN HUD/Sanity ne sont pas levés.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 market -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron -Hidden`
Durée : environ 1 min (Entrée à chaque temps). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
