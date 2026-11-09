# [À ESSAYER] E-unit_stats_attack : clique chaque type d'unité à toi (Bélier, Mangonneau, Lancier, Homme d'armes, Archer, Moine) : le bloc de stats sous les points de vie doit porter la ligne Attaque avec sa valeur (Bélier : « Attaque bâtiments 60 », Moine : aucune ligne Attaque).

STATUT : VU À L'ÉCRAN
GESTE: réel      (clic gauche sur le corps dessiné de chaque unité, hol.Select.ClickUnit ; les unités sont posées par cheat, aucune touche n'est nécessaire pour lire le bloc)
SON: non entendu   (aucun son attendu ici ; lancé sans fenêtre, rien entendu)
branches: aucune      (seul main compte : le binaire de Faramir = main 02ac688, D750 `hol.HUD.UnitStatsBuilding` déjà dedans ; les seuls commits de la branche fog sont des scénarios)

Pré-vol (2026-11-09 09:21, agent Faramir-17, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, main 02ac688) : `preflight.ps1 unit_stats_attack -Root <worktree>` PASS (45 s, 6 PNG). Seul WARN : pas d'audit `Sanity:` (le scénario ne le demande pas).
PNG OUVERTS et vus :
- usa_ram : Bélier à moi, « Attaque bâtiments 60 · Armure mêlée 0 · Armure distance 60 · Armure héros 0 · Vitesse 2,2 m/s » (la valeur 60 est présente)
- usa_mangonel : Mangonneau, « Attaque 45 · Armure mêlée 0 · Armure distance 60 · Vitesse 1,8 m/s »
- usa_spearman : Lancier, « Attaque 8 », 80/80 PV
- usa_manatarms : Homme d'armes, « Attaque 12 », 155/155 PV
- usa_archer : Archer, « Attaque 5 », 70/70 PV
- usa_monk : Moine, aucune ligne Attaque (attendu), 80/80 PV
Panneau du scénario : reste sur « en préparation… » sur les PNG du pré-vol (essai sans fenêtre, le panneau ne passe à « prêt » qu'à la fin ; non vérifié en jeu).
NON VÉRIFIÉ : jamais joué avec une vraie fenêtre ni au son ; le Mangonneau n'affiche qu'« Attaque 45 » (pas de ligne « bâtiments » : à confirmer que c'est voulu, la valeur 45 est la même contre bâtiment) ; la police/lisibilité en 1280x720 seulement.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 unit_stats_attack -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
