# [À ESSAYER] E-art_icon : la cabine d'icônes produit une icône PNG par type (51), chaque objet entier devant un mur uni

STATUT : VU À L'ÉCRAN
GESTE: interne      (commande `hol.Art.Icon All` : outil d'artiste, pas un geste de joueur)
SON: sans objet
branches: art-icons-1125ar

Pré-vol (2026-11-25 22:55, agent GLORFINDEL-1125ar6, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, branche art-icons-1125ar 751f761) : `preflight.ps1 art_icon_all -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` PASS (50 s, 1 PNG de point : ai_all) ; journal « Art icon: done count=51 types=51 » ; 51 PNG dans Saved\ArtIcons.
PNG OUVERTS et vus :
- ai_all.png : vue de jeu après la cabine : unités posées en rangées, un filon d'or de la rangée des sources au centre.
- T_Building_Barracks.png : caserne ENTIÈRE (n'est plus coupée), sur un sol gris uni (plus le sol vert), mur uni derrière ; le bleu restant est le toit de la forme d'attente à la couleur du joueur.
- T_Building_MainCentre.png : Forum entier avec ses 4 tours et drapeaux, sol gris uni.
- T_Source_GoldVein.png : filon d'or entier (bloc brun + pépites jaunes), sur l'herbe, mur uni derrière.
- T_Source_Tree.png : arbre entier (tronc + houppier), sur l'herbe, mur uni derrière.
- T_Unit_Arquebusier.png : mannequin bleu entier, de dos aux trois quarts.
- T_Unit_Mangonel.png : mannequin entier (forme d'attente, pas encore de modèle).
NON VÉRIFIÉ : les 44 autres PNG (non ouverts) ; import en Texture2D (pas fait) ; unités et sources encore sur l'herbe (seuls les bâtiments ont le sol gris).

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle, jamais « même commande avec… ».
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 art_icon_all -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel`
Puis ouvrir le dossier C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Saved\ArtIcons et regarder les PNG.
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
