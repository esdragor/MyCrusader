# [À ESSAYER] E-audio_compare_buildings_metal : TOI SEUL peux juger ces sons (l'agent joue sans son et ne peut rien entendre) ; écoute les 10 sons de SÉLECTION du bâtiment de MÉTAL (Forge) et choisis la lettre du meilleur

STATUT : VU À L'ÉCRAN pour l'affichage ; le SON reste à ton oreille (SON: non entendu)
Recette 19:44 sur main dc67a16 (worktree Faramir, branche fog, candidats copiés d'Arwen dans Faramir à 19:42, non suivis par git) : `preflight.ps1 audio_compare_buildings_metal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` PASS (28 s ; aucun « introuvable », aucun timeout ; 1 WARN : pas de hol.HUD.Report au moment de la capture). PNG vu : audio_compare_buildings_metal_start = panneau « Scénario audio_compare_buildings_metal », notes justes (SÉLECTION du bâtiment de MÉTAL : Forge ; 10 sons A à J ; Entrée = variante suivante ; réécoute `hol.Audio.Candidate Building.Metal C`) ; château, héros, 5 soldats bleus.
Statut précédent, réparé : logique seulement (recette indépendante 19:3x sur main dc67a16, worktree Faramir : preflight FAIL, « Objet /Game/Audio/Candidates/soft/SW_s_metal2_anvil_low introuvable » puis « timeout at step 18: waitlog candidate played Building.Metal A » : les candidats exclus de git n'existent que sur le disque d'Arwen, l'essai n'est donc PAS intégré et ne se joue que depuis Arwen ; défaut commun avec audio_compare_buildings_stone, ligne [ ] en tête de Auto_Journey_File.md ; PNG _start non ouvert par la recette)
(ancien statut, pré-vol depuis Arwen 17:55 : VU À L'ÉCRAN pour l'affichage (le panneau des notes de l'écoute Forge est vu sur la capture) ; le SON lui-même reste LOGIQUE SEULEMENT (règle L7-6) tant que tu ne l'as pas écouté : c'est précisément ton geste
GESTE: interne      (les sons sont joués par la commande `hol.Audio.Candidate` du scénario, pas par un vrai clic sur la Forge ; seule la touche Entrée est la tienne)
SON: non entendu    (l'agent lance avec -nosound : il ne PEUT pas entendre ; passe à « entendu » quand tu as écouté)
branches: aucune (main)   (le scénario et la commande `hol.Audio.Candidate` sont dans main)

Règle (L7-6) : elle porte ici sur le statut du SON, pas sur l'écran : l'écran est vu, le son attend ton oreille.

Pré-vol (2026-11-13 17:55, agent ARWEN-1113c, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche d751-root = merge de main 93e3d86, binaire frais build PASS 17:48) : `preflight.ps1 audio_compare_buildings_metal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (check, sanity, lancement joueur OK), 1 PNG.
Porte d'essai (2026-11-13 17:59, ARWEN-1113d) : `session_gate.ps1 -Phase Essay -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen -Essay audio_compare_buildings_metal` = 1 FAIL : « 2 fichier(s) de code différent de main (ex. Script/Neutrals/HOLCampMarks.as) : l'essai ne compte pas comme intégré » (correctif du carré de camp 4b32a80/0dc1939 de d751-root, sans rapport avec le son, pas encore dans main : disparaît à la fusion groupée de 20:00) ; PASS worktree propre, PASS binaire 17:48 >= dernier commit de code, PASS branche d751-root contenue.
PNG OUVERTS et vus (ARWEN-1113c puis ARWEN-1113d, 17:5x) :
- `Saved_Tests\Screenshots\WindowsEditor\HOL\audio_compare_buildings_metal_start.png` : panneau « Scénario audio_compare_buildings_metal » en haut, texte juste (MÉTAL : Forge ; 10 sons ; Entrée = variante suivante ; réécoute par console `hol.Audio.Candidate Building.Metal C`) ; château, héros et 5 soldats bleus au centre.
NON VÉRIFIÉ : aucun des 10 sons n'a été entendu par personne (-nosound) ; que chaque son soit bien GRAVE, SOURD, COURT et DOUX à l'oreille (seulement mesuré par `Tools\Audio\check_audio_soft.py`) ; le volume réel ; l'enchaînement des 10 Entrée avec le son audible.

Sons candidats : exclus de git (accord de l'utilisateur du 2026-11-13, ligne `Content/Audio/Candidates/` de `C:\Users\baudo\Documents\MyCrusader\.git\info\exclude`, partagée par tous les worktrees). Emplacement (décision du 2026-11-13 : copie dans le worktree qui joue l'essai) : copiés d'Arwen le 2026-11-13 19:42 dans CE worktree, `C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Content\Audio\Candidates\soft\` (fichiers `SW_s_metal2_*`, non suivis : git status vide). Conséquence : lance la commande telle quelle, avec `-Root ...HeroesOfLiberty-Faramir` ; depuis une copie sans ces fichiers, le jeu dit « candidate missing » et reste muet (cas du 2026-11-11).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 audio_compare_buildings_metal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
Durée : environ 2 min (3 min au plus).
Gestes : 1. attends le panneau « Écoute le son de SÉLECTION du bâtiment de MÉTAL (Forge) » ; 2. Entrée = joue le son A ; le panneau dit « Building.Metal A (s_metal2_anvil_low) vient de jouer » ; 3. Entrée = son suivant (B, C... jusqu'à J) ; 4. après J, Entrée = fin. Pour réécouter une lettre : touche ² (console) puis `hol.Audio.Candidate Building.Metal C` (la lettre voulue), Entrée.
Réponse attendue : `OK` (un son te convient, dis sa lettre), `BUG: ...` (ex. pas de son, panneau faux) ou `REGLAGE: ...` (ex. `REGLAGE: Metal H`, plusieurs lettres possibles, ou « aucune, dix autres » en disant ce que tu aimes).
Métal (Forge) [Building.Metal] = _

Lettres (le panneau affiche le nom du fichier à côté de la lettre) :
Building.Metal : A s_metal2_anvil_low, B s_metal2_plate_deep, C s_metal2_pot_deep, D s_metal2_pot_three, E s_metal2_tin_deep, F s_metal2_latch_deep, G s_metal2_click_deep, H s_metal2_mallet_soft, I s_metal2_gong_deep, J s_metal2_double

## Historique
- 2026-11-11 soir : « les sons ne se sont jamais lancés » : lancé depuis Haldir, sans les candidats (seulement dans Arwen) ; binaires périmés.
- 2026-11-12 (FARAMIR-1112a) : pré-vol durci (42e5e41 : FAIL sur « candidate missing » et sur fichier non commité).
- 2026-11-13 : candidats exclus de git (accord), Arwen recompilé (build PASS 17:48), pré-vol PASS 17:55 : fiche passée de [NE PAS JOUER] à [À ESSAYER] (ARWEN-1113d).
