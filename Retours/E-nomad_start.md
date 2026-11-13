# [À ESSAYER] E-nomad_start : une partie Nomade commence sans aucun bâtiment ; ton Roi, tes 5 Paysans et ton héros naissent au point de départ

STATUT : VU À L'ÉCRAN
GESTE: interne      (le scénario lance la partie et place la caméra par des commandes `hol.*` ; aucun clic à rejouer : l'essai est de regarder le départ)
SON: sans objet
branches: aucune (main)

Pré-vol (2026-11-13 22:30, agent Haldir, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branche maps 12d2bab sur main 0c09db6) : `preflight.ps1 nomad_start -Root <worktree> -AllowUncommitted` PASS (check 22 s, 2 PNG). Seul WARN : `Content/Maps/Crown.umap` non suivi dans ce worktree (fichier d'un autre chantier, non lu par cet essai, qui joue Plain) : l'essai ne se joue donc QU'AVEC `-Root` ci-dessous.
Recette indépendante 3 23:05 sur main 091df3c : PASS, PNG vus : nomad_start_wide (aucun bâtiment à moi, 5 Paysans en rangée + Roi couronne dorée + héros blanc et or au centre du cercle pâle ; barre : Nourriture 200, Bois 350, Pierre 600, Or 200, Population 5 / 0, Inactifs : 5), nomad_start_close (les 7 unités entières, aucun bâtiment) ; réserves : deux objets non identifiés à moitié cachés par le HUD dans la vue large (petit bloc bleu et blanc sous « Triches », bloc jaune et brun sous « Diplomatie ») ; la sortie du check montre des lignes « LogAutomationTest: Error: Condition failed » juste avant le scénario (test d'erreur unifiée du moteur UE::UnifiedErrorTest, déjà vu en recette 2, sans lien avec l'essai) ; le jeu se ferme seul (verbe `quit`, étape 27).
PNG OUVERTS et vus :
- nomad_start_wide : vue large au-dessus du départ (-14800, 0) : aucun bâtiment à toi (ni Centre-ville, ni chantier) ; au centre d'un grand cercle pâle, une rangée de 5 Paysans bleus, derrière eux le Roi (couronne dorée) et le héros (blanc et or), environ 20 px de haut chacun ; forêt à gauche, buissons à baies à droite, pierres en haut. Barre du haut : Nourriture 200, Bois 350, Pierre 600, Or 200, Population 0 / 0, Inactifs : 5. Journal : « Building report: 0 buildings », « HUD overlap: none ».
- nomad_start_close : zoom au plus près du jeu (caméra à 1500 cm), centré sur les unités : les 5 Paysans (environ 55 à 65 px), le Roi et le héros (environ 70 à 80 px) entiers, aucun bâtiment dans le cadre.
Journal exact vu (scenario_nomad_start.log) : « Nomad slot 0: no main centre, its units at (-14800, 0) », « Regicide: king of slot 0 spawned at (-15935, 0) », « Starting Peasants of slot 0: 5 in front of its main forum », « Starting hero of slot 0 spawned at (-15935, 300) » (et les mêmes lignes pour l'IA, slot 1, à (15200, 0)).
NON VÉRIFIÉ :
- 100 px par unité demandé : PAS atteint, le zoom le plus proche du jeu est 1500 cm (HOLRTSCamera::MinHeight) : 55 à 80 px sur la capture 1066 x 600.
- (CORRIGÉ 22:52, 2bde715, vérifié sur PNG à 23:00 puis par la recette 3 : barre « Population 5 / 0 ») Le « Population 0 / 0 » venait de `HOLProductionSubsystem.cpp:519` (sortie anticipée sans bâtiment). Le plafond 0 avant le premier forum ou maison est la règle A de `Retours\Q-nomad-population.md` : question ouverte, la barre peut passer en rouge ou alerter (non vérifié).
- Les Paysans naissent en UN seul groupe au point du départ : les groupes séparés (SplitIntoGroups, FarEnough, NomadGroupClear) ne sont pas faits.
- Bouton de spécialisation éteint d'un Nomade sans forum : AUCUNE interface de spécialisation n'existe encore (SP3 pas commencé) ; seule la règle côté serveur refuse le départ (test HeroesOfLiberty.Specialization.NomadForum).
- Le Roi n'est là que parce que le .json coche Régicide (bRegicide) ; le cercle pâle autour du départ n'a pas été identifié.

Lancer UNE commande (22:46 : le scénario et le code sont maintenant DANS main a2465a2, binaire compilé dans le worktree Faramir) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 nomad_start -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
