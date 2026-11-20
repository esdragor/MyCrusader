# [À ESSAYER] E-deploy : le Mangonneau obéit au premier clic (D861) : monté + clic droit au sol = il se démonte puis part ; démonté + clic droit sur la Maison = il se monte puis tire

STATUT : VU À L'ÉCRAN
GESTE: interne
SON: non entendu
branches: aucune

Pré-vol (2026-11-20 12:53, session principale, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, main 991a8bc) : `preflight.ps1 deploy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (78 s, 11 PNG).
PNG OUVERTS et vus :
- deploy_attack_mounted : Mangonneau monté (case D « Démonter »), statut « Attaque », ligne rouge vers la Maison ennemie.
- deploy_attack_hit : la Maison ennemie sélectionnée, 465 / 600 PV : touchée par l'engin monté.
- deploy_move_walked : après le clic droit au sol, l'engin s'est démonté (case D « Monter ») et a marché sans second clic.
NON VÉRIFIÉ : les clics du scénario sont des ordres internes (`hol.Select.OrderGround`, `OrderNear`), pas de vraie souris ; aucun projectile n'est dessiné (manque connu) ; la cause de l'ancien comportement (a) n'a pas été prouvée rouge à l'exécution (test vert avant correctif possible).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 deploy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Gestes : regarder ; les étapes 6 à 9 s'affichent dans le panneau du scénario. Pour l'essayer à la main : sélectionner le Mangonneau monté, clic droit au sol (il doit partir seul après 5 s de démontage) ; puis, démonté, clic droit sur la Maison rouge (il doit se monter puis tirer).
Durée : environ 3 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
Reponse: BUG/REGLAGE : « le démontage est instantané, comme le montage ; ça doit prendre du temps (1-2 s à chaque fois) » (2026-11-20 ~13:05)
