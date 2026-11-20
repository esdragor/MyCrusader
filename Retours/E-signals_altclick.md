# [À ESSAYER] E-signals_altclick : la marque du signal (Alt + clic gauche au sol) pulse maintenant, au sol et sur la mini-carte (D860)

STATUT : VU À L'ÉCRAN
GESTE: interne
SON: non entendu
branches: aucune

Pré-vol (2026-11-20 12:54, session principale, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, main 991a8bc) : `preflight.ps1 signals_altclick -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (41 s, 4 PNG).
PNG OUVERTS et vus :
- signals_altclick_pulse_a / pulse_b (0,3 s d'écart) : le losange blanc au sol change de taille (environ 140 puis 190 px), la ligne « Vous : Signal » au fil.
NON VÉRIFIÉ : la pulsation de la mini-carte n'a qu'une capture fixe (carré blanc) ; bornes 0,8 à 1,4 et 3 cycles provisoires (cvars `hol.Signal.PulseMin`, `PulseMax`, `MarkCycles` ; `hol.Signal.MarkStyle 0` = l'ancienne marque fixe).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 signals_altclick -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Gestes : quand le scénario est prêt, Alt + clic gauche au sol ailleurs : regarder la marque battre, au sol et sur la mini-carte, puis s'effacer après 5 s.
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...` (amplitude, vitesse).
Reponse:
