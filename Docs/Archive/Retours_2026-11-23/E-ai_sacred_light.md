# [RÉPONDU : OK] E-ai_sacred_light : l'IA de l'Aube lance elle-même la Lumière sacrée sur ses soldats blessés (anneau doré, Honneur 100 -> 75)

STATUT : VU À L'ÉCRAN
GESTE: interne (c'est l'IA qui joue ; le scénario pose les armées et remplit la jauge de l'IA par hol.AI.HonorSet)
SON: non entendu
branches: main
Reponse: OK (utilisateur, 2026-11-23 15:59)

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`.

Pré-vol (2026-11-23 15:45, agent RECETTE-1123pa, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, main b5a285b) : `preflight.ps1 ai_sacred_light` PASS (environ 55 s, 2 PNG).

À FAIRE et VOIR :
1. Rien à cliquer : regarde l'écran. Devant le forum rouge de l'IA, ses soldats se rassemblent (aucun combat).
2. Ensuite 8 Hommes d'armes bleus (à toi) arrivent sur eux : mêlée. Dès qu'un soldat de l'IA est blessé, un anneau doré (deux cercles) se pose sur la mêlée.
3. Au journal : « AI slot 1 step N: powers think ... decision cast=true » puis « Sacred Light cast on 1 wounded unit(s) » et « Honor now 75 ».

PNG OUVERTS et vus :
- ai_sacred_light_before : forum rouge, 5-6 figures rouges devant la porte et un groupe de 6 rouges plus bas ; aucun combat, aucun anneau (la case de faction à gauche est celle du joueur : 0 / 100, « ressource insuffisante »).
- ai_sacred_light_after : mêlée rouges / bleus au milieu de l'écran, anneau doré (2 cercles) centré sur la mêlée ; Population 13.
Journal : step 39 `wounded 1 of 7, decision cast=true row 0, nearest wounded 253 cm, range 800 cm` ; `Sacred Light put down by slot 1, Honor now 75` ; `Sacred Light cast on 1 wounded unit(s)` ; avant le lancer `Honor 100` (HonorSet).
NON VÉRIFIÉ : points de vie du blessé avant / après (ni la guérison) ; le Chevalier vertueux n'est pas distinguable sur le PNG ; l'IA joue avec une jauge posée par cheat (HonorSet), pas gagnée en jeu ; son non entendu.

Commande complète (règle 13, 2026-11-14) :
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 ai_sacred_light -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
