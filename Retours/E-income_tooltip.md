# [À ESSAYER] Infobulle de revenu (barre du haut)
GESTE: réel (hol.Mouse.Hover : curseur Slate immobile, l'infobulle est celle que Slate affiche)
SON: non entendu (-nosound)
branches: specialization-trial
Statut : VU À L'ÉCRAN (4 PNG ouverts : FR bois, FR or, EN, JA), preflight PASS.
recette: RECETTE-1121sb PASS sur main a7833ea (Sauron) : 4 PNG ouverts (FR bois +377 bois/min, FR or +0 or/min, EN +377 wood/min, JA +377 木材/分), infobulle fond sombre bord dore, preflight PASS. Reste : Inactifs : 0 non traduit en EN/JA (hors essai).
Ce que tu dois voir : survole le montant de bois / d'or de la barre du haut : panneau HUD fond sombre, bord doré, police ~14 px : "+N bois/min", "+0 or/min", puis "+N wood/min" (EN) et "+N 木材/分" (JA) après changement de langue en direct.
Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 income_tooltip -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron