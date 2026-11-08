# [À ESSAYER] E-fog_trees : un bosquet coupé pendant que tu ne le vois pas reste dessiné dans le gris, puis disparaît quand tes soldats reviennent

STATUT : VU À L'ÉCRAN (3 captures ; le grésillement du gris reste NON VÉRIFIÉ, voir plus bas)
GESTE: réel   (Q = hol.Select.Grid 4 puis vrai clic gauche hol.Select.LeftClickGround, modèle neutrals_qground ; les deux déplacements des 4 Hommes d'armes passent par là, plus par hol.Select.OrderNear)
SON: sans objet
branches: fog

Pré-vol (2026-11-08 10:43, agent Faramir-13, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, main c158640) : `preflight.ps1 fog_trees -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` PASS (47 s, 3 PNG, aucun recouvrement de widgets, audit Sanity passé).
PNG OUVERTS et vus :
- fogtrees_a : bosquet de 10 arbres dessiné en vert clair, dans la zone vue, 4 Hommes d'armes en bas.
- fogtrees_b : soldats hors champ, le bosquet est resté dessiné mais assombri (gris de l'exploré), alors qu'il a été coupé.
- fogtrees_c : soldats revenus, plus aucun arbre.
NON VÉRIFIÉ : le grésillement du gris (arbres, sol) à l'œil sur écran réel : jamais reproduit en mesure, images fixes seulement ; la liste « Brouillard » du menu n'est lue que par la 2e note (à faire à la main) ; un arbre coupé chez un client en réseau.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 fog_trees -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
Durée : environ 1 min. Réponse attendue : `OK` ou `BUG: le gris grésille encore, à tel endroit`.


integre sur main fc601b7 (fusion groupee n1, 2026-11-08 11:38) : session_gate -Phase Essay PASS ; preflight fog_trees PASS sur ce build (Sauron), PNG fogtrees_c ouvert : soldats revenus, plus aucun arbre, brouillard noir autour.
Reponse: BUG: le scenario reste bloque en preparation (utilisateur, 2026-11-08 ~11:55, lance par play.ps1 fog_trees -Root ...Sauron). A rejouer par le meme geste que l'utilisateur (play.ps1, pas preflight).

corrige, rejoue (hors fenetre) 2026-11-08 12:2x par Elrond-5 : cause = le clic au sol venait 20 puis 60 images apres hol.Camera.FocusNear ; dans le jeu du joueur la camera derive (souris au bord = defilement, touche tenue), 'hol.Select.LeftClickGround: the point is not on the screen' (journal de 11:48), le waitlog 'Scripted left click at' ne vient jamais, panneau bloque sur 'en preparation'. check/preflight tournent sans defilement ni curseur donc ne le voyaient pas. Correctif : fog_trees pose la camera 2 images avant chaque clic ; preflight.ps1 FAIL 'pan exposure' (PanExposure.ps1, pan_exposure_test.ps1, rouge e5e156b). preflight PASS, PNG fogtrees_a (arbres vus clairs), _b (bosquet coupe reste assombri, soldats hors champ), _c (plus d'arbres, soldats revenus) OUVERTS et vus. NON VERIFIE : reproduction exacte dans le jeu fenetre de l'utilisateur (curseur en haut non reproduit chez moi, bas = derive vue) ; a rejouer par play.ps1 -Root ...Sauron ; neutrals_qground et diplomacy_qground_watch ont le meme risque (clic 125 images apres la camera), non corriges.
