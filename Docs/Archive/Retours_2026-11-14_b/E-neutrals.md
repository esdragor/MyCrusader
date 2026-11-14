# [À ESSAYER] E-neutrals : un camp de 3 Lanciers neutres, dans une clairière, charge mes 3 Hommes d'armes, est détruit, et mon héros gagne 100 XP

STATUT : VU À L'ÉCRAN (2026-11-14 10:56, agent SAURON-1114a, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, branche visual = main 89c5de3 + docs, build de sync PASS 10:55)
GESTE: interne      (Q = `hol.Select.Grid 4` puis `hol.Select.LeftClickGround -300 0` : commandes qui passent par les gestionnaires d'entrée, mais pas une vraie touche ni une vraie souris ; le vrai geste se joue dans `neutrals_qground`). C'est un essai de comportement du camp (charge, destruction, XP), pas de geste.
SON: non entendu    (-nosound, rien entendu par l'agent)
branches: visual

Pré-vol (2026-11-14 10:55-10:56, SAURON-1114a) : `preflight.ps1 neutrals -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (65 s, 3 PNG) : journal sans avertissement ni erreur, aucun panneau qui en couvre un autre, audit Sanity PASS (il était ROUGE le 2026-11-11 : spin de mêlée ; le scénario joue 3 contre 3 depuis Celeborn).
PNG OUVERTS et vus (Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- neutrals_camp : sol nu, aucun arbre autour du camp (clairière `hol.Decor.ClearNear 0 0 2500`) ; 2 Lanciers gris visibles au centre en haut (le 3e se confond avec un voisin à ce cadrage), mes 3 bleus groupés en bas ; personne ne se bat : conforme.
- neutrals_fight : les 3 Lanciers gris courent sur mes 3 Hommes d'armes bleus (anneaux de sélection, barres de vie vertes, ligne d'ordre rouge, grille Q Attaque / S Arrêt / V Tenir affichée) : conforme.
- neutrals_cleared : les 3 Lanciers gris à terre, lances tombées, mes 3 bleus debout : conforme.
- Cosmétique : le panneau « Scénario neutrals en préparation… » reste en haut sur les 3 captures (défaut connu).
NON VÉRIFIÉ : la souris et le clavier physiques ; le son ; les 100 XP à l'écran (journal seulement : « Camp 1 reward: slot 0 +100 XP ») ; 3 silhouettes grises distinctes sur neutrals_camp ; le BUG de l'utilisateur du 2026-11-11 (« le bras qui ne tient pas la lance est dans son corps ») : à cette distance de caméra, pas visible sur les PNG (prise de lance B, D787 / D795, dans main) ; un seul pré-vol (pas de série ab_matrix).
Question en attente : le camp caché dans le bosquet (`Q-neutrals-camp-cache`, non tranchée ici ; fiche introuvable dans Retours\ le 2026-11-14).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 neutrals -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
