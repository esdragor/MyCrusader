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
- T_Source_GoldVein.png : filon d'or entier (bloc brun + pépites jaunes), sur le sol gris uni (AR5-f, plus d'herbe), mur uni derrière.
- T_Source_Tree.png : arbre entier (tronc + houppier), sur le sol gris uni (AR5-f), mur uni derrière.
- T_Unit_Arquebusier.png : mannequin bleu entier, de dos aux trois quarts.
- T_Unit_Mangonel.png : mannequin entier (forme d'attente, pas encore de modèle).
AR5-f (2026-11-26, CELEBORN-1126i1, branche art-icons2-1126ar, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn) : preflight art_icon_all PASS (57 s, 51/51 PNG) ; ouverts : T_Unit_Spearman (lancier bleu entier, pieds sur le sol gris), T_Unit_Mangonel (mannequin gris entier, forme d'attente), T_Source_Tree, T_Source_GoldVein, T_Source_BerryBush (buisson vert à baies rouges entier), T_Building_Barracks : TOUS sur le sol gris uni, plus d'herbe ; 51 textures réimportées (Content\UI\Icons). Rejouer : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 art_icon_all -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`.
NON VÉRIFIÉ : les autres PNG (non ouverts un à un) ; icône vue dans le cadre de garnison après ce réimport (pas rejoué).

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle, jamais « même commande avec… ».
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 art_icon_all -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel`
Puis ouvrir le dossier C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Saved\ArtIcons et regarder les PNG.
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
