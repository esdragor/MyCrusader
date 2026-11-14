# [À ESSAYER] E-nomad_start : une partie Nomade commence sans aucun bâtiment ; tes Paysans naissent DISPERSÉS en groupes, ton Roi avec l'un, ton héros avec un autre ; ton premier Forum se construit dès l'âge I (C) pour 150 bois + 400 pierre

STATUT : VU À L'ÉCRAN (réserve : `preflight` sort FAIL (1) pour la SEULE raison connue `?? Content/Maps/Crown.umap` non suivi dans le worktree Haldir ; tout le reste OK, check PASS)
GESTE: réel      (partie 4 : sélection des Paysans `hol.Select.Type Peasant`, touche de l'âge I `hol.Select.Grid 0`, touche C `hol.Select.Grid 10`, vrai clic gauche `hol.Select.LeftClickGround` ; parties 1-3 : on regarde seulement)
SON: non entendu      (`-nosound` ; l'essai ne porte pas sur un son)
branches: maps

Rejoué 2026-11-14 12:01 (agent HALDIR-1114d, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branche maps e02b67b ; départ dispersé d2c3ab6, test NomadScatter 63eefff, tests 1257/1257) : `preflight.ps1 nomad_start -Root <worktree>` : check PASS (28 s, 5 PNG à 1920x1080), aucun avertissement du journal, « HUD overlap: none », « Sanity: OK (39 steps, 14 units followed) » ; seul FAIL = Crown.umap non suivi (connu).
Cause du délai de 300 s du `play.ps1 -Hidden` de 11:48 (journal `Saved\Logs\HeroesOfLiberty.log` du worktree) : « Test sequence: timeout at step 9: waitlog Nomad slot 0: no main centre, its units at ( » : le code écrit maintenant « its units dispersed around ( » (d2c3ab6) ; la ligne suivante (« Starting Peasants of slot 0: 5 ») aurait aussi bloqué (une ligne par groupe : « 1 in front of its main forum » x 5). Scénario corrigé (44092b6, d78d739, e02b67b) : waitlog des nouvelles lignes, `r.SetRes 1920x1080w` APRÈS ces waitlog (waitlog ne lit que les lignes écrites depuis la dernière commande), vue large à 7000 cm, deux gros plans, `quit` final gardé. Aucun changement de code.
Journal exact vu (scenario_nomad_start.log) : « Nomad slot 0: no main centre, its units dispersed around (-14800, 0) », « Regicide: king of slot 0 spawned at (-12696, 4832) », « Nomad slot 0: group 0 (1 Peasants) at (-12246, 4832) », group 1 (-17327, 1800), group 2 (-8975, -1206), group 3 (-15142, -1611), group 4 (-17803, -4087), « Starting hero of slot 0 spawned at (-17777, 2100) » ; « Building report: 0 buildings » ; « Selection report: selected 5 units (own): Peasant x5 » ; « Construction: slot 0 placed MainCentre foundation at (-14800, -3600), paid F 0 W 150 S 400 G 0 (F 200 W 200 S 200 G 200) ». Mêmes places aux 3 passages (graine fixe par siège).
PNG OUVERTS et vus :
- nomad_start_wide (caméra 7000 cm) : aucun bâtiment à moi ; 5 points bleus à 5 endroits différents, plusieurs dizaines de mètres entre eux : un Paysan seul en haut au centre, un seul à gauche contre la forêt, un seul en bas à gauche (étiré à l'horizontale, bord de l'image), le Roi (point doré de la couronne) avec un Paysan à droite, le héros (blanc et or) avec un Paysan en bas au centre dans un cercle pâle. Unités petites (15 à 25 px). Barre : Nourriture 200, Bois 350, Pierre 600, Or 200, Population 5 / 0, Inactifs : 5.
- nomad_start_king (1500 cm) : le Roi (grosse couronne dorée) et un Paysan bleu collés l'un à l'autre, seuls dans le cadre ; un bloc d'or (jaune et brun) en haut, à moitié sous le panneau du scénario.
- nomad_start_hero (1500 cm) : le héros (tabard blanc à soleil doré, bouclier, lance) et un Paysan à environ 2 m, sans le Roi, au centre d'un grand cercle pâle.
- nomad_start_grid : « Paysan (5) », « 5 inactifs » ; page I de la grille : C « Centre principal » active à l'âge I, à côté de Maison, Camp de bûcherons, Camp de mineurs, Moulin, Caserne, Champ de tir, Ferme, Sanctuaire de l'Aube. Les 5 Paysans des 5 groupes sont donc sélectionnés ensemble.
- nomad_start_forum (3000 cm) : la fondation du Forum = grand cadre bleu clair en fil de fer (cadre fantôme, provisoire déjà listé) au centre gauche, une forêt juste à sa droite ; barre : Bois 200, Pierre 200 (150 bois + 400 pierre payés) ; « 5 en marche », Inactifs : 0 ; la case C est grisée.
NON VÉRIFIÉ :
- Fondation posée à (-14800, -3600) pour un clic visé à (-14800, -2500) : 11 m d'écart (accrochage de la pose ou visée du clic, non cherché) ; on ne voit pas si elle touche la forêt.
- La case C reste listée (grisée) après la pose (le test dit qu'elle quitte la liste : rafraîchissement non vu) ; l'infobulle (pas de survol) ; en réseau, un joueur distant ne voit pas la case (lu dans le code, `Retours\Q-nomad-net.md`).
- Le cercle pâle autour du héros n'est pas identifié ; le Paysan étiré en bas à gauche de la vue large (déformation du bord ou autre) non vérifié de près.
- « Endroits aléatoires de la carte » (D207) : le code disperse dans un disque de 60 m autour du départ (réglage provisoire `hol.Nomad.ScatterRadiusCm`), non tranché.
- Le journal dit encore « Starting Peasants of slot 0: 1 in front of its main forum » pour un Nomade sans forum (texte trompeur, aucun effet).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 nomad_start -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Historique :
- 2026-11-13 22:30 (HALDIR-1113m) puis recette 3 23:05 : départ Nomade sans bâtiment, Roi + 5 Paysans + héros en rangée au point du départ, VU À L'ÉCRAN.
- Réponse de l'utilisateur 2026-11-14 : « BUG: je ne pouvais pas construire de forum, je n'avais pas l'option ; il faut que je puisse le construire exceptionnellement alors que normalement c'est palier 1 ; et en Nomade tout le monde devrait être dispersé, pas au même endroit. »
- 2026-11-14 11:23 (HALDIR-1114b) : point 1 corrigé, case C et paiement 150/400 vus, fondation cachée par un buisson (caméra à 1500 cm).
- 2026-11-14 12:01 (HALDIR-1114d) : point 2 (départ dispersé, d2c3ab6) vu ci-dessus, fondation visible.
