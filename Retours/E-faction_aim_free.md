# [À ESSAYER] E-faction_aim_free : tu vises la Lumière sacrée à la souris et un cercle de 6 m te montre où elle tombera (vert à portée, orange au-delà)

STATUT : VU À L'ÉCRAN (recette indépendante HALDIR-1124l, 2026-11-24 12:08, sur main 9d40517 fusionné et compilé : PASS ; l'aperçu de visée est vu sur 2 PNG)
GESTE: réel      (vrai clic sur la case Lumière sacrée hol.Mouse.Click FactionBox0, souris déplacée hol.Mouse.Hover X Y, vrai clic au sol hol.Mouse.Click ; événements Slate sur le curseur virtuel de Slate, pas le curseur du bureau)
SON: non entendu   (-nosound ; aucun son neuf dans cet essai)
branches: main

Recette HALDIR-1124l (2026-11-24 12:07, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branche integ-58 = main 9d40517 fusionné, build PASS 92 s) : `preflight.ps1 faction_aim_free -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` PASS (56 s, 4 PNG, aucun recouvrement de HUD), 0 erreur au journal. PNG OUVERTS et vus par l'agent de recette :
- faction_aim_free_ready : PASS. Le château, le héros et 5 Hommes d'armes devant lui, anneau crème de 12 m autour du héros, panneau Lumière sacrée 100 / 100 « prêt », PV héros 180 / 450 ; aucun cercle de visée (rien d'armé).
- faction_aim_free_aim : PASS. Case armée : cercle VERT de 6 m posé sur le sol devant le château, autour des soldats, centre à l'intérieur de l'anneau de portée ; pas de zone dorée, Honneur 100 / 100.
- faction_aim_free_aim_far : PASS. Curseur à droite, centre hors de l'anneau crème : le cercle est ORANGE ; toujours pas de zone dorée, Honneur 100 / 100.
- faction_aim_free_after : PASS. Arc/cercle DORÉ au point cliqué, Honneur 85 / 100, case « recharge 18 s », PV héros 245 / 450 (monté).
NON VÉRIFIÉ : (1) le cercle suit la VRAIE souris du bureau (le pré-vol passe par le curseur virtuel ; chemin DeprojectMousePositionToWorld jamais exécuté avec une vraie souris) ; (2) portée mesurée depuis le héros (HOLHero::GetHeroLocation) sur un CLIENT réseau non vérifiée ; (3) le cercle est un cercle de débogage (placeholder, ligne de Placeholders.txt), partie masquée par le toit du château, absent en Shipping ; (4) tests Test_FactionAimPreviewColoursOutOfReach non lancés (quick_check à la charge de la session principale) ; (5) le refus silencieux hors portée au clic n'est pas capturé par PNG.

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle.
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 faction_aim_free -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
(le worktree Haldir contient main 9d40517 : le correctif est dans main 9d40517, compilé)
Ce que tu fais : clique la case Lumière sacrée, déplace la souris : un cercle vert (à portée, 12 m du héros) ou orange (hors de portée) la suit ; clique le sol pour lancer. Trois lancers, Entrée = suivant.
Durée : environ 3 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Recette HALDIR-1124m (2026-11-24, dans main 25e29ab, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, build PASS) : PASS (4 PNG vus : cercle de visee VERT a portee puis ORANGE hors portee, zone doree + Honneur 84/100 apres le lancer)
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 faction_aim_free -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
