# [À ESSAYER] E-audio_compare_buildings_stone : TOI SEUL peux juger ces sons (l'agent joue sans son et ne peut rien entendre) ; écoute les 10 NOUVEAUX sons (3e tour, plus francs et plus forts) de SÉLECTION des bâtiments en PIERRE (Centre principal, Caserne, Champ de tir, Écurie, Atelier de siège) et choisis la lettre du meilleur

STATUT : VU À L'ÉCRAN pour l'affichage ; le SON reste à ton oreille (SON: non entendu)
GESTE: interne      (les sons sont joués par la commande `hol.Audio.Candidate` du scénario, pas par un vrai clic sur un bâtiment ; seule la touche Entrée est la tienne)
SON: non entendu    (l'agent lance avec -nosound : il ne PEUT pas entendre ; passe à « entendu » quand tu as écouté)
branches: d751-root   (les candidats du 3e tour et leur liste HOLAudioCandidates.as ne sont que dans d751-root, worktree Arwen ; pas dans main)

Pré-vol (2026-11-14 11:0x, ARWEN-1114b, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche d751-root d85cb19, binaire à jour : Get-StaleBinaryProblem vide) : 20 sons du 3e tour importés (éditeur caché, 227 sons sous /Game/Audio/Candidates) ; scénario régénéré ; `preflight.ps1 audio_compare_buildings_stone -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (34 s ; 10/10 « candidate played » A à J dans l'ordre ci-dessous, aucun « introuvable », aucun timeout ; 1 WARN : pas de hol.HUD.Report au moment de la capture).
PNG OUVERT et vu : `Saved_Tests\Screenshots\WindowsEditor\HOL\audio_compare_buildings_stone_start.png` : panneau « Scénario audio_compare_buildings_stone » en haut, texte juste (PIERRE : Centre principal, Caserne, Champ de tir, Écurie, Atelier de siège ; 10 sons, 1 cue, dix variantes A à J ; Entrée = variante suivante ; réécoute `hol.Audio.Candidate Building.Stone C`) ; château, héros, 5 soldats bleus.
Porte d'essai (session_gate -Phase Essay, 11:0x) : 1 FAIL attendu « 1 fichier de code différent de main (Script/Audio/HOLAudioCandidates.as) » = la liste du 3e tour, pas encore fusionnée (disparaît à la fusion de d751-root) ; PASS worktree propre ; PASS binaire.
Niveau (mesuré par `Tools\Audio\check_audio_soft.py`, plancher -18 dB au-dessus de 200 Hz ; tour 2 refusé : -20 à -47 dB ; sons élus Bois/Sacré/Marché : -12 à -19) : A -17,6 ; B -17,1 ; C -12,5 ; D -12,9 ; E -12,0 ; F -13,6 ; G -18,0 ; H -12,1 ; I -12,6 ; J -13,7.
NON VÉRIFIÉ : aucun des 10 sons n'a été entendu par personne (-nosound) ; le volume réel à ton oreille (seulement mesuré dans les fichiers) ; l'enchaînement des 10 Entrée avec le son audible.

Sons candidats : exclus de git (`Content/Audio/Candidates/`), présents SEULEMENT sur le disque d'Arwen (décision du 2026-11-13 : copie dans le worktree qui joue l'essai). Lance donc la commande telle quelle, avec `-Root ...HeroesOfLiberty-Arwen` ; depuis une autre copie, le jeu dit « candidate missing » et reste muet.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_stone -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min (3 min au plus).
Gestes : 1. attends le panneau « Écoute le son de SÉLECTION des bâtiments en PIERRE » ; 2. Entrée = joue le son A ; le panneau dit « Building.Stone A (s_stone3_block) vient de jouer » ; 3. Entrée = son suivant (B, C... jusqu'à J) ; 4. après J, Entrée = fin. Pour réécouter une lettre : touche ² (console) puis `hol.Audio.Candidate Building.Stone C` (la lettre voulue), Entrée.
Réponse attendue : `OK` (un son te convient, dis sa lettre), `BUG: ...` (ex. pas de son, panneau faux) ou `REGLAGE: ...` (ex. `REGLAGE: Stone C`, plusieurs lettres possibles, ou « aucune, dix autres » en disant ce que tu aimes).
Pierre (Centre principal, Caserne, Champ de tir, Écurie, Atelier de siège) [Building.Stone] = _

Lettres (le panneau affiche le nom du fichier à côté de la lettre) :
Building.Stone : A s_stone3_block, B s_stone3_mine_firm, C s_stone3_knock, D s_stone3_door_heavy, E s_stone3_punch, F s_stone3_drum_firm, G s_stone3_double, H s_stone3_gate, I s_stone3_mallet, J s_stone3_slab

## Historique
- 2026-11-11 soir : « les sons ne se sont jamais lancés » : lancé depuis Haldir, sans les candidats (seulement dans Arwen) ; binaires périmés.
- 2026-11-12 (FARAMIR-1112a) : pré-vol durci (42e5e41 : FAIL sur « candidate missing » et sur fichier non commité).
- 2026-11-13 : candidats exclus de git (accord), pré-vol PASS depuis Arwen puis depuis Faramir (copie) : [À ESSAYER] (tour 2).
- 2026-11-14 (réponse de l'utilisateur, tour 2) : BUG: aucun des candidats n'est bien ; sons difficiles à entendre, volume trop faible.
- 2026-11-14 10:51 (ARWEN-1114a) : cause du volume mesurée (énergie sous 100 Hz, pic -4 dB, aucun gain) ; rouge ba24081 (plancher audible), 3e tour généré 437ab6a (gain cuit +8 dB et limiteur, sous-graves coupés) ; fiche en [NE PAS JOUER].
- 2026-11-14 11:0x (ARWEN-1114b) : 3e tour importé dans Arwen, scénario régénéré d85cb19, pré-vol PASS, PNG vu : [À ESSAYER] depuis Arwen.
