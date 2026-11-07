# [A ESSAYER] Noms des Legions en francais, anglais, japonais (legions_roster_gesture) - VU A L'ECRAN

GESTE: reel
SON: non entendu
branches: galadriel-tab

Commandes (depuis le worktree Galadriel, qui contient la branche) :
powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 legions_roster_gesture -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 legions_roster_gesture_dawn -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel

Statut (2026-11-07, Galadriel-8, 13:0x) : build PASS, tests 1092/1092 (GameNames.LegionsLanguages et Localization.Translations verts), preflight PASS des deux scenarios, TOUS les PNG ouverts et vus : grille de la Caserne = Lancier squelette (A) / Garde noire (Z) ; bloc du bas = Garde noire, Cavalier funeste ; Lancier de l'Aube clique en ennemi = Lancier (exact, fin de ligne) ; Caserne de l'Aube sans grille Legions ; EN : Skeleton Spearman / Black Guard en grille, Bone Ram en selection ; JA : grille et selection en japonais (la Mangonneau et l'Archer ne sont pas en PNG mais leur texte est assertionne par waitlog) ; scenario Aube : Lancier / Homme d'armes (+ Hallebardes trempees), Archer / Arbaletrier, Cavalier leger gardes, et le Lancier des Legions de l'IA clique en ennemi dit Lancier squelette. Le japonais est une PROPOSITION a faire relire (LegionsInProgress.md, defaut = la proposition actuelle).
Pas entendu : aucun son concerne. A jouer a la main : cliquer la Caserne des Legions, A / Z, cliquer l'unite produite, lire le nom ; Parametres > Langue pour en / ja.