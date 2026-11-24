# [À ESSAYER] E-tower_wood_garrison : un clic droit sur la Tour de guet en bois fait entrer les 3 Lanciers sélectionnés dans la tour (ils disparaissent de l'écran, la sélection se vide)

STATUT : VU À L'ÉCRAN   (avant / après vus sur 2 PNG, FARAMIR-1124sc 2026-11-24 19:40 ; son non entendu)
GESTE: réel (vrai clic droit sur la tour via `hol.Select.RightClickGround` ; la tour elle-même est posée par `hol.Cheat.SpawnBuilding`, pas par le geste de pose, couvert par l'essai tower_wood)
SON: non entendu
branches: faramir-1124sc (eb7e87a, scénario tower_wood_garrison, worktree HeroesOfLiberty-Faramir ; sur main e86dca3, aucun code touché)

Pré-vol (2026-11-24 19:40, agent FARAMIR-1124sc, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, branche faramir-1124sc, commit eb7e87a sur main e86dca3) : `preflight.ps1 tower_wood_garrison -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir -AllowUncommitted` PASS (28 s, 2 PNG, binaire à jour). Journal : `Garrison: slot 0 puts 3 of 3 units in TowerWood` après le vrai clic droit.
PNG OUVERTS et vus :
- tower_wood_garrison_before : la Tour de bois du joueur au centre bas de l'écran, 3 Lanciers bleus sélectionnés juste au nord, panneau « Lancier (3) » en bas à gauche
- tower_wood_garrison_after : les 3 Lanciers ont disparu de l'écran (entrés dans la garnison), plus de cercles de sélection ni de panneau « Lancier (3) » (sélection vide)
NON VÉRIFIÉ : son ; barres de vie ; tour posée par cheat (pas par le geste de pose) ; essai rejoué sur la DLL de main après fusion de la branche ; sortie de la garnison (clic droit hors tour) et tir depuis la garnison non jouées.

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle.
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 tower_wood_garrison -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
