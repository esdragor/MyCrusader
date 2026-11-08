# [À ESSAYER] E-unit_tiers : la Caserne grise l'Homme d'armes (case Z) au palier 0, la case devient active au palier 1 et la touche Z forme un Homme d'armes

STATUT : VU À L'ÉCRAN
GESTE: interne      (touche Z via `hol.Select.Grid 1`, commande `hol.*`, pas de clic souris sur la case)
SON: sans objet
branches: tiers

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu` (le geste ici est interne : la partie « touche » est vue à l'écran, pas le clic souris ; à dire à l'utilisateur).

Pré-vol (2026-11-08 11:3x, agent Elrond-16, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main d1ce8fa) : `preflight.ps1 unit_tiers -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (2 PNG).
PNG OUVERTS et vus :
- unit_tiers_grey : la Caserne sélectionnée au palier 0, le Lancier (A) actif, l'Homme d'armes (Z) grisé
- unit_tiers_open : après `hol.Cheat.Tier 1`, la case Z active et un Homme d'armes sorti de la Caserne
NON VÉRIFIÉ : un vrai clic souris sur la case grisée (le scénario appuie sur la touche Z) ; Arbalétrier, Cavalier léger et Moine non joués à l'écran (couverts par les tests `Production.UnitTier` et l'accord script / simulation) ; sans cheat `hol.Cheat.Tier`, la montée au palier 1 par le niveau du héros n'est pas jouée dans cet essai.

Ce que tu dois voir : (1) palier 0, la Caserne est sélectionnée : la case Lancier (A) est active, la case Homme d'armes (Z) est grisée ; appuyer sur Z ne forme rien. (2) Après `hol.Cheat.Tier 1` : la case Z est active ; Z forme un Homme d'armes (2 s pour l'essai). Une Caserne au palier 0 ne forme donc que le Lancier ; l'Homme d'armes, l'Arbalétrier, le Cavalier léger et le Moine demandent le palier 1.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 unit_tiers -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
