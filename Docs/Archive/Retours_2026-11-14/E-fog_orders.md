# [À ESSAYER] E-fog_orders : les lignes d'ordre jaunes restent nettes au-dessus du brouillard et le brouillard reste en place pendant la marche

STATUT : VU À L'ÉCRAN
GESTE: interne (ordres en file donnés par le scénario ; objet = affichage)
SON: sans objet
branches: aucune

Pré-vol (2026-11-13 16:2x-16:33, agent FARAMIR-INTEG intégrateur, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, branche fog = main 8d13f46, dll 11-13 13:27/13:29) : `preflight.ps1 fog_orders -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` PASS (38 s, 2 PNG).
PNG OUVERTS et vus :
- fogord_a : 4 Hommes d'armes, ligne jaune et deux points nets, sans strie noire
- fogord_b : 3 s plus tard le brouillard est toujours là, la zone claire a suivi les hommes, la ligne suit
NON VÉRIFIÉ : les points d'ordre sont au BORD du noir plutôt que dedans (la ligne « dans le noir » n'est pas montrée loin dans le noir) ; Maj + clic droit de l'utilisateur.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 fog_orders -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
