# [À ESSAYER] Infobulle de revenu (barre du haut)
GESTE: réel (hol.Mouse.Hover : curseur Slate immobile, l'infobulle est celle que Slate affiche)
SON: non entendu (-nosound)
branches: specialization-trial
Statut : VU À L'ÉCRAN (4 PNG ouverts : FR bois, FR or, EN, JA), preflight PASS.
Ce que tu dois voir : survole le montant de bois / d'or de la barre du haut : panneau HUD fond sombre, bord doré, police ~14 px : "+N bois/min", "+0 or/min", puis "+N wood/min" (EN) et "+N 木材/分" (JA) après changement de langue en direct.
Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 income_tooltip -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel