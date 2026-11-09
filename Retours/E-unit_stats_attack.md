# [À ESSAYER] E-unit_stats_attack : clique chaque type d'unité à toi (Bélier, Mangonneau, Lancier, Homme d'armes, Archer, Moine) : le bloc de stats sous les points de vie doit porter la ligne Attaque avec sa valeur (Bélier : « Attaque bâtiments 60 », Moine : aucune ligne Attaque).

STATUT : VU À L'ÉCRAN (objet de l'essai = l'AFFICHAGE du bloc de stats, pas un geste)
GESTE: interne      (le scénario sélectionne par `hol.Select.ClickUnit`, c'est un appel interne et non un vrai clic : à l'utilisateur de cliquer lui-même une unité de chaque type ; les unités sont posées par cheat)
SON: sans objet     (aucun son attendu)
branches: aucune    (seul main compte : D750 `hol.HUD.UnitStatsBuilding` est dans main 60013e8)

Règle (L7-6) : le geste de clic n'est pas jugé ici (interne), seul le contenu du bloc l'est.

Pré-vol (2026-11-09 10:50, agent Elrond-integration, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main 60013e8, rejoué sur le build de main fusionné) : `preflight.ps1 unit_stats_attack -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (25 s, 6 PNG, aucun panneau de HUD ne se couvre). Seul WARN : pas d'audit `Sanity:`.
PNG OUVERTS et vus :
- usa_ram : Bélier « Vous » 400/400, « Attaque bâtiments 60 · Armure mêlée 0 · Armure distance 60 · Armure héros 0 · Vitesse 2,2 m/s »
- usa_mangonel : Mangonneau 200/200, « Attaque 45 · Armure mêlée 0 · Armure distance 60 · Armure héros 0 · Vitesse 1,8 m/s »
- usa_spearman : Lancier 80/80, « Attaque 8 · Armure 0/0/0 · Vitesse 7,8 m/s »
- usa_manatarms : Homme d'armes 155/155, « Attaque 12 · Armure mêlée 4 · Armure distance 4 · Armure héros 2 · Vitesse 7 m/s »
- usa_archer : Archer 70/70, « Attaque 5 · Armures 0 · Vitesse 7,8 m/s »
- usa_monk : Moine 80/80, AUCUNE ligne Attaque (attendu), armures 0, vitesse 7,8 m/s
Constat : le panneau du scénario reste sur « en préparation… » sur tous les PNG alors que le journal dit « ready » (affichage du panneau, sans rapport avec l'objet).
NON VÉRIFIÉ : jamais joué par l'agent avec une vraie fenêtre ni un vrai clic de souris ; le Mangonneau n'affiche qu'« Attaque 45 » (pas de ligne « bâtiments » : à confirmer que c'est voulu) ; lisibilité en 1280x720 seulement.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 unit_stats_attack -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Reponse (utilisateur, 2026-11-09, joue par lui) : OK, MAIS pas d'infobulle au survol (hover) pour les autres stats du bloc (hors la ligne Attaque) -> REGLAGE : ligne [ ] en tete de Auto_Journey_File.md (infobulle de survol pour chaque stat du bloc de stats d'une unite, ex. PV, armures, vitesse, portee ; libelles du GDD).
