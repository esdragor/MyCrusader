# E-forge : la Forge recherche Lames affûtées I (palier 1), 13,2 de dégâts au lieu de 12 — ÉCHEC VISUEL (grille non rafraîchie au palier 1), voir BUG-forge-tier1-grid.md

STATUT : preflight PASS (check 21 s, chaîne waitlog complète, 4 PNG ouverts et vus) ; le check FAIL du pas 41 (`waitlog Selection: Forge`) est CORRIGÉ et rejoué : il passe. Mais le point « forge_tier1 » du .look n'est pas satisfait à l'écran : NON listé [À ESSAYER]. GESTE : Forge posée par `hol.Cheat.SpawnBuilding`, sélection `hol.Select.Category Technological`, recherche par `hol.Select.Grid 8` (internes, pas de clic).

Pré-vol (2026-11-05 21:18, agent d'intégration n°2, worktree HeroesOfLiberty-Elrond = main eb7b5fe, SANS build) : `preflight.ps1 forge -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (journal sans warning/erreur, HUD sans recouvrement sur 2 rapports, audit Sanity OK).

PNG OUVERTS et vus (+ un recadrage agrandi de la rangée W X C V) :
- forge_grid : Forge bleue sélectionnée, rangée W « Lames affûtées », X « Lames affûtées II », C « Empennage », V « Empennage II », toutes grisées au palier 0 : conforme.
- forge_tier1 : « Paladin-Commandant : niveau 3 », « Palier 1 atteint », mais la rangée est IDENTIQUE à celle du palier 0 (W et C grisés, rien de « prêt ») ET l'infobulle jaune « Palier supérieur requis » est encore affichée : le .look attend « W Lames affûtées I prête (non grisée), le refus du palier a disparu » : NON conforme.
- forge_queued : Nourriture 5050 (-150), Or 5100 (-100) : le coût est pris ; un panneau de file « …ames affûtées » (nom coupé à gauche) avec barre de progression ; l'infobulle « Palier supérieur requis » est TOUJOURS là, par-dessus ; la case C « Empennage » est maintenant éclaircie (prête), W grisée (en file) ; Homme d'armes et Lanciers près de la Forge.
- forge_after : recherche finie (file vide), C toujours claire, W grisée ; l'infobulle a disparu.

Défauts : (1) la grille ne se rafraîchit pas au passage au palier 1 (W et C restent grisés alors que la recherche est ensuite acceptée par `hol.Select.Grid 8`) ; (2) l'infobulle « Palier supérieur requis » reste affichée après le palier atteint et pendant la recherche ; (3) le nom dans le panneau de file est tronqué (« ames affûtées »). Détail dans BUG-forge-tier1-grid.md.

NON VÉRIFIÉ : les dégâts 12 puis 13,2 et le 4 des Lanciers (lignes `Spearman for 12,` / `13.2,` / `ManAtArms for 4` : le `waitlog` du scénario les a attendues et la chaîne est allée jusqu'à « ready », donc elles sont au journal, mais je n'ai pas relu les valeurs) ; le clic réel sur W ; la touche F3 (le scénario sélectionne par commande).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 forge -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 15 s (aucune attente). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
