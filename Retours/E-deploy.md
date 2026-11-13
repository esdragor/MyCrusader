# E-deploy : la Mangonnelle (substitut du Trebuchet) se monte et se demonte par la touche D (case D : Monter / Demonter)

STATUT : LOGIQUE SEULEMENT (pre-vol FAIL : NE PAS JOUER)
GESTE: interne      (hol.Select.Grid 6, equivalent de la touche D, pas une vraie touche)
SON: non entendu    (-nosound)
branches: tiers

Pre-vol (2026-11-13 12:41, agent ELROND-1113, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, tiers 64ac8e2, binaire du 11:09 a jour) :
`preflight.ps1 deploy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` = FAIL (6), 125 s.
- OK : la Mangonnelle nait avec le fragment (hol.Siege.Deployable 1), la grille dit `D=Deploy`, cellule « Monter » ; `Order: 1 units deploy` apparait apres hol.Select.Grid 6.
- FAIL : 10 s apres l'ordre, hol.HUD.Report dit encore `D=Deploy` (timeout a l'etape 35, `waitlog D=Pack`) ; aucune ligne `Mangonel hits a building`. La phase ne passe pas a Deploying/Deployed en partie reelle, alors que le test HeroesOfLiberty.Siege.Deploy.PlayerButton est vert.
- Captures : seule deploy_packed est produite (NON OUVERTE, faute d'appels) ; deploy_deployed, deploy_fire, deploy_packed_again absentes.
- WARN : la ligne `hol.Test.Sanity` du .txt n'est pas atteinte (le scenario s'arrete avant).

Pistes pour l'agent suivant (non verifiees) : (1) la Mangonnelle est a ~5250 cm de la Maison, au-dela de la portee max proposee (5000 cm) : l'acquisition automatique ecrit PendingOrder 1 (attaque) que Decide traduit en marche (March), ou l'ordre de marche note PendingOrder 2, et l'un ou l'autre annule le Toggle ; essayer la Maison a 4000 ; (2) ajouter une ligne de journal au HOLDeployProcessor (phase avant / apres, ordre decide) pour voir ce qui se passe ; (3) verifier que RequestToggleDeploy -> ServerToggleDeploy -> HOLPlayerOrders::ToggleDeploy atteint bien la file en solo (aucune ligne de journal aujourd'hui).

NON VERIFIE : tout ce qui suit l'ordre ; aucun PNG ouvert ; aucun client reseau.

Ne pas lancer tant que le pre-vol n'est pas PASS.
