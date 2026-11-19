# [RÉPONDU 2026-11-17, OK] E-ecran-reglages : l'écran Réglages (écrit en script) s'ouvre depuis le titre, a ses 4 onglets, et chaque réglage s'applique et se garde

STATUT : VU À L'ÉCRAN
GESTE: réel   (hol.Mouse.Click sur SettingsButton, les 4 onglets et BackButton ; puis l'utilisateur essaie les rotateurs à la main)
SON: non entendu   (réglages Son sans effet audible : le jeu n'a pas encore de sons ; note du scénario)
branches: hero-j8-close

Pré-vol (2026-11-17 11:2x, agent CELEBORN-1117h, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, branche hero-j8-close 8c10fe6) : `preflight.ps1 ecran_reglages -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn` PASS (41 s, 5 PNG).
Correctif du 1117g : le scénario porte maintenant la ligne `# default-settings` ; check.ps1 remet alors la section HOLGameUserSettings du Saved_Tests à ses valeurs par défaut (au lieu de couper le défilement au bord comme pour tous les autres tests). Le .look n'a pas été modifié.
PNG OUVERTS et vus (tous conformes au .look) :
- ecran_reglages_video : onglet Vidéo choisi ; Résolution 1280 x 720, Affichage, Synchronisation verticale Non, Images par seconde Illimitée, Échelle de rendu 90 % à gauche ; Qualité Haute et les 8 groupes (Distance d'affichage ... Terrain) à droite
- ecran_reglages_son : 4 lignes (Volume général, Musique, Effets, Voix) à 100 % et la note en dessous
- ecran_reglages_commandes : Changer les touches grisé, Vitesse de la caméra Normale, Défilement au bord de l'écran Oui, la note
- ecran_reglages_langue : une ligne Langue sur Français
- ecran_reglages_retour : écran titre (Jouer, Multijoueur, Réglages, Quitter) ; le panneau de notes jaune du scénario couvre le haut (sans gravité)
NON VÉRIFIÉ : le sens et l'effet réels de chaque réglage (volume sans son dans le jeu, cadence, vitesse de caméra, résolution), la touche Échap et LB/RB à la manette, la persistance après Retour puis Réglages (jouée par toi seulement).

ATTENTION : play.ps1 avec -Root écrit les réglages dans le worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Saved\Config\WindowsEditor, JAMAIS dans ta copie principale ni tes réglages utilisateur. Ne change ni la langue ni la résolution sans les remettre.

Geste (3 min) :
1. Lance la commande ci-dessous ; le jeu s'ouvre sur le titre, le scénario clique Réglages puis les 4 onglets tout seul (environ 90 s) et revient sur Vidéo.
2. Quand la note demande « Entrée : j'ai essayé » : onglet Son, change un volume ; onglet Vidéo, change les Images par seconde ; onglet Commandes, change la Vitesse de la caméra : chaque valeur doit changer tout de suite.
3. Teste les flèches du clavier, Échap : la navigation entre colonnes marche.
4. Appuie sur Entrée : le scénario clique Retour. Rouvre Réglages à la main : tes valeurs sont gardées.

Commande complète (copiable telle quelle).
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 ecran_reglages -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`
Durée : environ 3 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Reponse: OK (2026-11-17, ~16:2x, essai joué par l'utilisateur ; D840 lot 2). Fusion dans main en cours (fusion groupée n°5, worktree Sauron, branche visual).
