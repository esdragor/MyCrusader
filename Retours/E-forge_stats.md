# [À ESSAYER] E-forge_stats : la Forge (grille du palier), la Caserne des Legions (infobulle 45 / 15) et la ligne de statistiques de l'unite

STATUT : VU À L'ÉCRAN (grille, ligne de stats) ; le survol (infobulle des stats et du Lancier) n'est PAS vu a l'ecran : LOGIQUE SEULEMENT pour les infobulles
GESTE: interne
SON: sans objet

Pré-vol (2026-11-06 11:53, agent Galadriel, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, main adbddad) : preflight.ps1 forge, legions_roster, orge_stats_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel PASS.
PNG OUVERTS et vus :
- forge_tier1 : Forge au palier 1, W Lames affutees I prete (non grisee), X grise ; le toast « Palier supérieur requis » (3 s) est encore la 0,8 s apres le refus.
- forge_stats_before : Homme d'armes : « Attaque 12 · Armure mêlée 4 · Armure distance 4 · Armure héros 2 · Vitesse 7 m/s », sur 3 lignes.
- forge_stats_after : « Attaque 13,2 (+1,2) · Armure mêlée 4 · Armure distance 4 · Armure héros 2 · Vitesse 7 m/s ».
- forge_stats_compare_A / _B : A une ligne, B une ligne par statistique (reponse de l'utilisateur : A).
- legions_roster_barracks : Caserne des Legions : Lancier (A), Homme d'armes (Z), Hallebardes « Bientôt » ; l'infobulle 45 / 15 n'est pas sur la capture, elle est lue au journal (F 45 | W 15) et le cout paye 5200 -> 5155 / 5185.
NON VÉRIFIÉ : le survol reel des stats (Points de vie, Attaque de base, Technologies +10 %, Portée, Armure siège) et du Lancier (45 / 15) ; traductions en/ja ; Caserne (Hallebardes trempées = Bientôt) ; armure sans bonus de technologie.

Lancer UNE commande par essai (3 minutes au plus chacun) :
powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 forge -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 legions_roster -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
Reponse attendue : OK, BUG: ... ou REGLAGE: ... (en particulier : l'infobulle au survol de la ligne de stats).
Reponse (utilisateur 2026-11-06 12:3x) : forge : OK. legions_roster : REGLAGE: seulement le Dread Rider a un nom qui change, les autres unites Legions gardent leur nom d'Aube (noms par faction a cabler, voir ligne [ ] de Auto_Journey_File.md). Infobulle 45/15 et infobulle de stats au survol : pas de retour.

Reponse (utilisateur 2026-11-06 12:3x) : infobulle au survol : OK (fixe). Restent seulement les noms des unites Legions a cabler.
