# [À ESSAYER] E-signals_minimap_alt : Alt + clic gauche sur la MINI-CARTE pose le même signal qu'au sol (D860)

STATUT : VU À L'ÉCRAN
GESTE: interne
SON: non entendu
branches: aucune

Pré-vol (2026-11-20 12:55, session principale, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, main 991a8bc) : `preflight.ps1 signals_minimap_alt -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (34 s, 3 PNG).
PNG OUVERTS et vus :
- signals_minimap_alt : après Alt + clic sur la mini-carte, la ligne « Vous : Signal » au fil et un carré blanc sur la mini-carte au point cliqué.
NON VÉRIFIÉ : Alt est tenu par une touche interne (`hol.UI.Key LeftAlt`) et le clic par `hol.HUD.Click` : jamais avec la vraie souris ; Alt + glisser sur la mini-carte ne doit rien poser (vu à la lecture seulement).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 signals_minimap_alt -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Gestes : quand le scénario est prêt, garder Alt enfoncé et cliquer (gauche) sur la mini-carte : le signal apparaît au fil, sur la mini-carte et au sol à cet endroit ; sans Alt, le clic déplace seulement la caméra.
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
Reponse: REGLAGE : même chose : taille divisée par 3 (marque du signal sur la mini-carte), pas de nouvel essai (2026-11-20 ~13:05)
