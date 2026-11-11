# [A ESSAYER, PARTIEL] E-orders_marks_compare : les marques d'ordres au sol (Placeholders.txt : Trajets et marques d'ordres au sol [?]) ; dis aime / pas aime par lettre

STATUT : VU A L'ECRAN pour A, B, C, D et E (A et B recaptures nets le 2026-11-11 14:4x, agent Sauron-R4). Geste interne : la ligne GESTE reste telle quelle. Aucun gout juge par l'agent.
GESTE: interne      (hol.Select.OrderNear / OrderGround / AttackMoveNear ; la touche Entree est le seul geste de l'utilisateur, passee en auto par preflight)
SON: sans objet
branches: visual

Options : A clic droit (marque du clic + ligne jaune + point), B trajet en file (Maj + clic droit), C attaque-deplacement (Q + clic) = scenario orders_marks_compare ; D et E = ligne jaune dans le noir du brouillard, rendu actuel et ancien = scenario fog_orders_compare (reutilise tel quel).
VU (agent) : C (ordermark_C_attackmove) : ligne et point ROUGES (le jaune est celui des deplacements). D (fogcmp_above) : ligne jaune fine avec deux points, deux segments, traverse le noir du brouillard, nette. E (fogcmp_old) : meme ligne mais en pointilles.
VU (agent, corrige) : A : ordermark_A_click = petit anneau blanc au point du clic (il ne dure que 0,25 s, PingSeconds : capture 2 images apres le clic) ; ordermark_A_move = ligne jaune des Hommes d'armes jusqu'au point, point jaune au bout, unites visibles. B : ordermark_B_click = anneau blanc au 2e point ; ordermark_B_queue = DEUX segments jaunes (unites -> point 1 -> point 2), un point jaune a chaque etape. Cause du « pas vu » d'avant : le monde n'est pas dessine pendant les attentes (il faut des `frames` apres un `wait` avant `capture`), des shaders encore en preparation au 1er clic (wait 6 avant), et l'anneau du clic dure moins de 0,25 s (capture 2 images apres, pas 30). Corrige dans le scenario.
Pre-vol : preflight orders_marks_compare PASS (2026-11-11 14:44, agent Sauron-R4, 5 PNG ouverts : A_click, A_move, B_click, B_queue, C), fog_orders_compare PASS (Sauron-R3, apres build PASS 153 s). 
Lancer UNE commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 orders_marks_compare -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron  (puis fog_orders_compare pareil ; sans -Hidden : fenetre, reservee a toi). Duree : environ 1 min chacun.
Reponse attendue : pour A a E `aime` / `pas aime` ou `REGLAGE: ...`. Le choix de la ligne [?] -> [forme]/[style] est le tien, l'agent n'y touche pas.
