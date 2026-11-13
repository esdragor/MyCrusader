# E-spear_arm : la prise de lance B (D787, mains echangees) du Lancier en melee, bleu contre neutre gris

STATUT : LOGIQUE SEULEMENT   (pas liste pret : le point du .look n'est pas jugeable a l'oeil sur les PNG, voir plus bas)
GESTE: réel      (Q puis vrai clic gauche sur le sol derriere le Lancier neutre, `hol.Select.Grid 4` + `hol.Select.LeftClickGround 400 0`)
SON: non entendu   (-nosound ; et sans objet pour ce point : c'est une pose)
branches: hero-j8-close

Pourquoi LOGIQUE SEULEMENT : preflight PASS et les 5 PNG ouverts, MAIS la camera de jeu ne descend pas sous 15 m
(`AHOLRTSCamera::MinHeight` 1500 cm) : un Lancier fait ~70 px, on ne peut pas voir si les mains sont sur la hampe ni si un
avant-bras traverse le torse. Planche agrandie x3 des 4 captures de melee (hors jeu, scratchpad de l'agent) : aucune traversee
VISIBLE, mais chez le Lancier BLEU le bras gauche parait leve au-dessus de la tete dans les 4 instants et la lance part en
diagonale de la tete a derriere la hanche droite (pas « a l'horizontale a hauteur de taille ») : douteux, NON TRANCHE.
Le Lancier gris neutre tient sa lance a la taille, mains pres du fanion : plausible.

Pre-vol (2026-11-13 13:41, agent CELEBORN-1113d, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, branche
hero-j8-close 3553fba) : `preflight.ps1 spear_arm` PASS (check 37 s, 5 PNG, 0 attente morte).
PNG OUVERTS et vus :
- spear_arm_idle : les deux Lanciers dans l'image a 8 m, debout ; le bleu lance droite, le gris lance tenue en biais.
- spear_arm_fight1..4 : les deux en melee, PV du gris qui baissent ; silhouettes trop petites pour juger les mains (voir plus haut).
NON VÉRIFIÉ : la prise elle-meme (mains sur la hampe, bras hors du tronc) a l'oeil en jeu ; elle n'est mesuree que hors jeu
(`arm_body_check.py spear_attack` vert : gauche -2,3 cm, droite 0,0 cm). Le son. Le Lancier gris ne frappe jamais en retour dans
le journal du scenario (seul #2 bleu frappe #1 gris) : a regarder.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 spear_arm -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`
Duree : environ 1 min (zoomer a la molette au plus pres sur les deux Lanciers). Reponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
