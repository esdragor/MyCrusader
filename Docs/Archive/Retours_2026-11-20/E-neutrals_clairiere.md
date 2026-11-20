# [À ESSAYER] E-neutrals_clairiere : le camp neutre du centre (3 Lanciers gris) est dégagé de tout arbre sur 25 m (hol.Decor.ClearNear 0 0 2500 Trees), avant puis après

STATUT : VU À L'ÉCRAN
GESTE: interne
SON: non entendu
branches: main

Pré-vol (2026-11-20 16:49, agent RECETTE-1120pa, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, main 9dc5e75) : `preflight.ps1 neutrals_clairiere -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (environ 30 s, 3 PNG).
PNG OUVERTS et vus :
- neutrals_clairiere_avant : bosquet d'une quinzaine d'arbres à tronc et cime ronde sur le camp, qui cache en grande partie les 3 Lanciers gris ; 3 Hommes d'armes bleus en bas de l'image (pas à gauche).
- neutrals_clairiere_large : même cadre, bosquet disparu, les 3 Lanciers gris sont à découvert sur herbe nue ; reste un buisson isolé en bas à droite, à la limite des 25 m, et le cercle de zone du camp.
- neutrals_clairiere_camp : gros plan, les 3 Lanciers gris groupés sur herbe dégagée, aucun arbre entre la caméra et eux.
NON VÉRIFIÉ : le scénario est entièrement automatique (aucune touche ni aucun clic de l'utilisateur) ; le buisson en bas à droite de `_large` est à la limite du rayon, non confirmé comme hors rayon ; aucune partie naturelle (état injecté) ; le panneau de notes cache le haut de l'image ; son non entendu.

Commande complète (règle 13, 2026-11-14) :
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 neutrals_clairiere -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 1 min. Gestes : aucun, regarde l'avant (bosquet), puis l'après (clairière), puis le gros plan. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Reponse: OK
