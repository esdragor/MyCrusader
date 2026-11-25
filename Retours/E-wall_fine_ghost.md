# [À ESSAYER] E-wall_fine_ghost : tracer une palissade a la souris sur la grille fine (SubDivision 4) et voir les fantomes d'1 m pendant le glisser, puis les fondations d'1 m apres le relache

STATUT : VU À L'ÉCRAN (rendu du dessinateur de fantomes HOLWallGhostDrawer.as ; voir la ligne GESTE)
GESTE: interne      (selection du Paysan et touches A puis D par hol.Select.ClickUnit / hol.Select.Grid ; le glisser lui-meme est reel : hol.Mouse.Drag, appui, un deplacement par image, relache. Par la regle L7-6 l'essai reste donc LOGIQUE SEULEMENT pour le geste de selection)
SON: sans objet
branches: walls-g8-1125m

Pré-vol de la version FINALE (2026-11-25 11:19, agent CELEBORN-1125mg, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, tip 759463d + 328ad0d, main non fusionne) : `preflight.ps1 wall_fine_ghost` PASS (35 s, 2 PNG, aucun FAIL ; WARN seulement : frames 40 / 70 et pas de HUD overlap) ; avant, PASS 11:13 sur la 1re version.
PNG OUVERTS et vus (relance `play.ps1 -Hidden` 11:16, 1280x720) :
- wall_fine_ghost_during : file de petits fantomes verts plats, un par sous-case de 1 m (~26 px), serres, de l'appui (gauche) au curseur (cadre filaire a droite) ; aucun fantome rouge ; journal `Wall ghost: cells=31 valid=true`.
- wall_fine_ghost_after : les fantomes ont disparu, 31 fondations de palissade (Bois 20200 -> 20045 = 31 x 5) ; journal `place wall line received by the server`.
NON VÉRIFIÉ : le son ; le geste de selection est interne. ANOMALIE CONFIRMEE sur le preflight final (PNG wall_fine_ghost_after ouvert) : les fondations apres relache font ~4 m de large pour un pas d'1 m (treillis qui se chevauchent ; le fantome fait 1 m) ; ligne dans Placeholders.txt et ligne [ ] (C++) dans Auto_Journey_File.md (Celeborn) ; non tranche. Premiere version du scenario (focus -17400 0) : le glisser traversait le MainCentre a 1280x720, corrige en focus -19400 0.

Commande complète : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 wall_fine_ghost -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
