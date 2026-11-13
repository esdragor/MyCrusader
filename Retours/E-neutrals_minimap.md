# [À ESSAYER] E-neutrals_minimap : un camp neutre vu par mes soldats apparaît sur la minicarte en carré ROUGE, puis GRIS une fois nettoyé ; un camp jamais vu n'y est pas

STATUT : VU À L'ÉCRAN (le passage ROUGE puis GRIS du carré est visible sur les 2 PNG de la recette 19:3x, preflight PASS sur main dc67a16, worktree Faramir) ; RÉSERVE gardée : sur la capture 1280x720 le carré mesure ~8 px effectifs, rouge saumon : un REGLAGE de taille est possible (`Size = 12.0`, Script/Neutrals/HOLCampMarks.as, une ligne). Correction 19:4x (FARAMIR) : le correctif 12 px 0dc1939 EST dans main (`Size = 12.0` dans HOLCampMarks.as de main) ; le FAIL « d751-root ABSENTE » du gate vient du tip de d751-root, qui a bougé pour un autre travail (« Faction choice » 7371e9f..4043c18, sans rapport) : d'où `branches: aucune (main)`. Lecture de la recette, gardée pour mémoire : le PNG ne montre pas ce que le .look promet : agrandi x8, le carré fait 8 px à l'écran (et non 12), rouge saumon plutôt que saturé, et ce sont les points BLEUS de mes soldats qui sont dedans, pas les 3 points gris du camp ; le passage rouge -> gris, lui, se voit. Cause : session_gate Essay 19:38 = FAIL « branche d751-root ABSENTE » : le correctif 12 px (4b32a80 / 0dc1939) n'est PAS dans main dc67a16, main montre l'ancien carré ; à rejouer sur main après la fusion de d751-root (et vérifier alors les 12 px à l'écran))
(ancien statut : VU À L'ÉCRAN, pré-vol PASS complet le 2026-11-13 17:53, 2 PNG ouverts et vus par l'agent)
GESTE: interne      (hol.Units.Move ; l'essai montre un affichage, pas un geste)
SON: sans objet
branches: aucune (main)

D789 (2026-11-12) : rien au survol d'un camp sur la minicarte. Vérifié : aucun survol / infobulle dans `Script/Neutrals/` (le point n'a pas de survol).

Correctif du 2026-11-13 (ARWEN-1113c, d751-root) : le carré faisait 7 px et se perdait sous les points de 3 px des gardes du camp ; il fait maintenant 12 px (deux fois un bâtiment). Test rouge 4b32a80 (`Test_CampMarkReadsAtAGlance` : >= 12 px, rouge saturé, distinct du gris), correctif 0dc1939 (`Script/Neutrals/HOLCampMarks.as`, `Size = 12.0` : une ligne à changer si la taille ne te plaît pas).

Pré-vol (2026-11-13 17:53, agent ARWEN-1113c, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, d751-root b5be3af = main 27c5547 + correctif, build PASS 558 s) : `preflight.ps1 neutrals_minimap -Root <worktree>` PASS (check 43 s, 6 s d'attente explicite, 0 waitkey, 2 PNG). WARN : pas de `hol.HUD.Report`, pas de `hol.Test.Sanity`. `quick_check -Parts tests` 1222/1222 PASS.
PNG OUVERTS et vus (capture 1280x720, sans agrandissement) :
- neutrals_minimap_alive : sur la minicarte (en bas à droite), dans le cadre de la caméra, un petit carré ROUGE net avec le point de mes soldats dedans ; rien vers le coin haut-droit (camp lointain 9000, 9000, jamais vu).
- neutrals_minimap_cleared : au même endroit, le carré est GRIS (nettement pas rouge), le point de mes soldats dedans ; toujours rien pour le camp lointain.
NON VÉRIFIÉ : client en réseau (réplication OwnerOnly, `net` non rejoué ce soir) ; la lisibilité à l'œil de l'utilisateur sur son écran (sur la capture 1280x720 le carré reste petit, environ 8 px : un REGLAGE de taille est possible) ; un point rougeâtre minuscule dans la tache de vision du bas de la minicarte, présent sur les deux captures, non identifié (pas à l'emplacement du camp lointain).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 neutrals_minimap -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
