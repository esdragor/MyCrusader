# E-arquebusier_gestes : l'Arquebusier a son corps et ses gestes (arme sur l'épaule, tir, rechargement à la baguette, chute)

STATUT : OK (joué par l'utilisateur le 2026-11-14 : « arquebusier_gestes : OK » ; avant : [À ESSAYER] VU À L'ÉCRAN   (pré-vol PASS sur le worktree Sauron, branche visual, commit après 8fd7cef ; 19 PNG recadrés et ouverts)
GESTE: interne      (hol.Select.ClickUnit / hol.Select.OrderNear : appels internes, pas de vrai clic)
SON: non entendu    (-nosound ; l'Arquebusier n'a pas de son propre)
branches: visual (worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, NON fusionné dans main)

Ce qui est à voir (voie A validée le 2026-11-14) : repos et marche avec l'arquebuse SUR L'ÉPAULE et la bandoulière de charges ; au tir : mise en joue, recul violent, arme posée crosse à terre, charge, balle, baguette tirée (bras levé), bourrage, baguette rangée, arme relevée (4 s en tout) ; la chute (celle de l'Arbalétrier, le fusil lui échappe des mains).
VU : le corps de l'Arquebusier (mannequin avec son arme) au repos, en marche, le bras levé pendant la baguette, l'arme relevée à la fin, la chute avec le fusil au sol.
NON VU nettement : la baguette elle-même (fine, vue de dos par la caméra RTS), la fumée et la balle (rien dans le jeu : le projectile reste le carreau SM_Bolt).
Réserves : bras droit à 4 cm dans le tronc à l'image 56 de l'attaque (la main au canon) ; mort = chute de l'Arbalétrier ; corps = mannequin ; test C++ HOLAnimationTests pas encore étendu à l'Arquebusier.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 arquebusier_gestes -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Reponse (2026-11-14) : OK. Suite demandee dans la foulee : (1) corriger le bras droit a 4 cm dans le tronc image 56, (2) fumee de poudre, balle visible et test HOLAnimationTests etendu a l'Arquebusier. (reponse donnee dans la conversation)
