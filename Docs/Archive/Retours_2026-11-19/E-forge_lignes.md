# [JOUÉ : OK, 2026-11-19 ~20:10] E-forge_lignes : la Forge montre 4 cases (Lames affûtées, Empennage, Maille, Barde), chaque case passe au niveau II après recherche ; la Ferme en montre 2, le Camp de bûcherons 1 (D856)

STATUT : VU À L'ÉCRAN
GESTE: réel pour la recherche (vrai clic de souris sur la case W) ; interne pour la sélection des bâtiments (commandes hol.Select.*) et leur pose (triche)
SON: sans objet
branches: aucune (tout est dans main 13a286d depuis le 2026-11-19 19:27 : integ-22 = retrait du natif de la grille + correction D856 + scénario)

Pré-vol (2026-11-19 19:29, session principale, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main 13a286d, binaire recompilé 19:28) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\preflight.ps1 forge_lignes -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (41 s, 4 PNG, audit Sanity passé). REJOUÉ 20:04 sur main dc993f6 (Elrond resynchronisé et recompilé 20:03 : seuls des tests ont changé) : PASS (34 s), gate Essay sans FAIL, capture forge_niveau_2 revue (identique).
PNG OUVERTS et vus (19:31) :
- forge_4_lignes : Forge sélectionnée, rangée du bas de la grille : W « Lames affûtées », X « Empennage », C « Maille », V « Barde », les 4 allumées, aucune « II ».
- forge_niveau_2 : après le clic sur W, la case W montre « Lames affûtées II » grisée ; son infobulle dit « Palier supérieur requis » (300 nourriture, 200 or) ; X, C, V inchangées ; 150 nourriture et 100 or payés pour le niveau I.
- ferme_2_lignes : Ferme sélectionnée (puces « Tous (2) », « Centre prin. », « Ferme (1) ») : W « Charrue », X « Brouette », C et V vides.
- camp_1_ligne : Camp de bûcherons sélectionné : W « Haches doubles » seule, X, C, V vides.
NON VÉRIFIÉ : le Camp de mineurs (même règle, non montré) ; le niveau II réellement recherché au palier 2 (ici le palier 1 est donné par triche, la case II reste grisée) ; une ligne dont tous les niveaux sont faits (case « faite ») ; le clic sur X, C, V.
Défauts connus du scénario (pas du jeu) : l'infobulle de la case W reste affichée aux étapes 2 à 4 tant que ta souris ne bouge pas (elle est restée sur la case) ; le Camp de bûcherons est posé au milieu de quelques arbres.

À confirmer en répondant : D856 a été écrite pour TOUT bâtiment de recherche, donc elle change aussi la Ferme (2 cases au lieu de 4 : Charrue, Brouette) et les Camps de bûcherons et de mineurs (1 case au lieu de 2). Ta réponse ne portait que sur la Forge : dis-le si tu ne voulais que la Forge.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 forge_lignes -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 3 min (Entrée à chaque étape : 1 la Forge, 2 le niveau II après le clic sur W fait par le scénario, 3 la Ferme, 4 le Camp de bûcherons).
Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`. À regarder, en bas à gauche, la rangée du bas de la grille (touches W X C V) : (1) 4 cases à la Forge, aucune « II » au départ ; (2) W devient « Lames affûtées II », grisée (palier 2 requis), les 3 autres inchangées ; (3) Ferme : Charrue et Brouette seulement ; (4) Camp de bûcherons : Haches doubles seule.

Reponse: OK (2026-11-19 ~20:10, dans la conversation : « forge_lignes : OK »). L'essai montrait aussi la Ferme (2 cases) et le Camp de bûcherons (1 case) : aucune réserve de l'utilisateur sur ce périmètre (lecture de la session principale : D856 reste écrite pour tout bâtiment de recherche ; l'utilisateur ne l'a pas dit en toutes lettres).
