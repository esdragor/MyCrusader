# [À ESSAYER] E-fog_trees : un bosquet de 10 arbres vu puis coupé hors de ta vue reste dessiné dans le gris, et disparaît quand tes soldats reviennent

STATUT : VU À L'ÉCRAN pour les 3 points statiques (bosquet vu, bosquet coupé gardé dans le gris, bosquet disparu au retour) ; LOGIQUE SEULEMENT pour « le gris grésille-t-il ? » (images fixes seulement, un grésillement ne se voit pas sur un PNG) et pour la liste « Brouillard » du menu Nouvelle partie (2e note du scénario : non capturée).
GESTE: réel      (Q = `hol.Select.Grid 4` puis clic gauche au sol = `hol.Select.LeftClickGround`, par les gestionnaires d'entrée comme `neutrals_qground` ; souris physique non testée)
SON: sans objet
branches: aucune    (seul main compte ; le scénario est dans main, correctif de placement de caméra 115dd98)

Règle (L7-6) : le geste est réel au sens du modèle `neutrals_qground` (touche puis clic par les gestionnaires), la souris physique n'est pas jouée par l'agent.

Pré-vol (2026-11-09 16:31, agent Sauron-E, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, main e9a349a, binaire du 13:20, `session_gate -Phase Essay` : binaire PASS) : `Tools\Scenarios\preflight.ps1 fog_trees -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (50 s, 3 PNG ; aucun HUD qui en recouvre un autre, audit Sanity PASS).
PNG OUVERTS et vus (Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- fogtrees_a : le bosquet de 10 arbres, vert vif, au bord de la zone claire, au fond ; mes 4 Hommes d'armes en bas, le panneau Q Attaque / S Arrêt / V Tenir visible ; noir tout autour, gris de l'exploré en bandes.
- fogtrees_b : les 10 arbres sont toujours là mais ASSOMBRIS (vert foncé = gris de l'exploré), mes soldats ne sont plus dans l'image (ils sont revenus au centre) ; le bosquet a été coupé pendant ce temps (journal `Sources.Report`) et reste dessiné : c'est l'ancien état connu, voulu.
- fogtrees_c : les soldats sont revenus ; les arbres ont disparu, il ne reste que le sol quadrillé ; la bande de gris à cet endroit est vide.
- Cosmétique vu sur les 3 captures : le panneau du scénario affiche « en préparation… » pendant tout le déroulé (même défaut connu sur `qground`).
NON VÉRIFIÉ : le grésillement du gris (sol et arbres) en mouvement, aucune capture ne le montre ; la souris physique ; la liste « Brouillard » du menu (2e note) ; une partie jouée à l'œil dans la vraie fenêtre.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 fog_trees -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...` (dis surtout si le gris des arbres et du sol grésille encore quand les soldats s'éloignent).

Reponse (utilisateur, 2026-11-09 16:44, joue par lui) : OK.
