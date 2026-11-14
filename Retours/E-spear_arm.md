# [À ESSAYER] E-spear_arm : la prise de lance B (D787 / D795, mains echangees) du Lancier en melee, bleu contre neutre gris

STATUT : VU À L'ÉCRAN   (melee et lance tenue a la taille vues sur les 5 PNG ; les mains elles-memes ne sont pas jugeables a 15 m : c'est ce que l'utilisateur regarde au zoom)
GESTE: interne   (Q puis clic sur le sol par commandes `hol.Select.Grid 4` + `hol.Select.LeftClickGround 400 0`)
SON: non entendu   (-nosound ; sans objet pour ce point : c'est une pose)
branches: hero-j8-close

Ce qu'il faut regarder : chez les deux Lanciers (bleu et gris neutre), le bras qui ne pousse pas la lance ne doit pas entrer
dans le torse ni sortir du dos ; la lance est tenue des deux mains, main gauche devant, main droite derriere a la hanche droite.
Hors jeu (outil `arm_body_check.py`, 2026-11-14) : spear_attack gauche -2,3 cm / droite 0,0 cm, spear_fall vert (D795 : la
premiere image de la chute reposee en prise B, commit 3349950, rebake A_Spearman_Fall). La chute n'apparait pas dans ce
scenario (personne ne meurt pendant les captures) ; elle n'est vue que sur la planche hors jeu
`C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Saved\c1114b\spear_fall_B.png` (bras hors du tronc, lance lachee vers l'avant).

Pré-vol (2026-11-14 10:55, agent CELEBORN-1114b, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, branche
hero-j8-close 7781a65, APRES le rebake de la chute) : `preflight.ps1 spear_arm -Root <worktree>` PASS (check 31 s, 5 PNG, 0 attente morte).
PNG OUVERTS et vus :
- spear_arm_idle : les deux Lanciers a 8 m, debout ; le bleu lance droite, le gris lance tenue en biais.
- spear_arm_fight1..4 : les deux en melee, lance en diagonale a la taille, tenue des deux mains ; aucun bras ne sort visiblement
  du dos ; le bleu passe de 80 a 72 PV a fight4 (le gris riposte) ; le bras gauche du bleu parait haut vu de dos (effet de camera, D795).
NON VÉRIFIÉ : les mains exactement sur la hampe a l'oeil en jeu (camera min 15 m, un Lancier ~70 px) ; la chute en jeu ; le son ;
le geste reel (touche et clic simules par commandes).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 spear_arm -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`
Durée : environ 1 min (zoomer a la molette au plus pres sur les deux Lanciers). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
