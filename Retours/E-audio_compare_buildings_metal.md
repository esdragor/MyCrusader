# [NE PAS JOUER] E-audio_compare_buildings_metal : écoute le son de SELECTION du batiment de METAL (Forge) : sons GRAVES, SOURDS, COURTS et DOUX, pas usants ; tu réponds par cue, la lettre que tu préfères

STATUT : NE PAS JOUER (LOGIQUE SEULEMENT) ; cause : sons candidats jamais commités (seulement dans Arwen) et binaire de la copie jouée périmé (diagnostic 3b94faf) ; redonner quand la copie choisie est recompilée et que preflight.ps1 audio_compare_buildings_metal -Root <cette copie> complet PASS
GESTE: interne      (les sons sont joués par la commande hol.Audio.Candidate du scénario, pas par un vrai clic ; seule la touche Entrée est la tienne)
SON: non entendu   (-nosound côté agent ; passe à « entendu » quand quelqu'un a écouté)
branches: heal-sleep   (le scénario et les candidats viennent du worktree Arwen ; candidats JAMAIS commités, décision B de l'utilisateur)

Réponses : remplace le _ par la lettre préférée (A à J), plusieurs lettres possibles, ou « aucune, dix autres » et dis ce que tu aimes.
Metal (Forge) [Building.Metal] = _

Lettres (le panneau affiche le nom du cue a cote de la lettre) :
Building.Metal : A s_metal2_anvil_low, B s_metal2_plate_deep, C s_metal2_pot_deep, D s_metal2_pot_three, E s_metal2_tin_deep, F s_metal2_latch_deep, G s_metal2_click_deep, H s_metal2_mallet_soft, I s_metal2_gong_deep, J s_metal2_double

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_metal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min (10 sons, Entrée = variante suivante ; le panneau dit « <Cue> <lettre> (<fichier>) vient de jouer »).
Réponse attendue : une lettre par cue, `OK`, `BUG: ...` ou `REGLAGE: ...`.

Reponse: les sons ne se sont jamais lances, 2026-11-11 soir

## STATUT 2026-11-12 (FARAMIR-1112a) : LOGIQUE SEULEMENT, NE PAS JOUER
Cause (FARAMIR-1112a, 2026-11-12, journal HeroesOfLiberty-Haldir\Saved\Logs\HeroesOfLiberty-backup-2026.11.11-18.18.22.log) : meme cause que _stone : lance depuis Haldir, candidats de son seulement dans Arwen (jamais commites) ; preflight durci (42e5e41). binaire de main (copie principale, dll 2026-11-11 11:06) PERIME : 9 commits C++ depuis (rage L4, Monter/Demonter, camps) ; au lancement Angelscript: Error (Namespace 'HOLAIRageOrder' doesn't exist, UHOLCampSubsystem::GetNumCamps) et le jeu reste a l'image 0 (rejoue par FARAMIR-1112a, play.ps1 -Hidden depuis C:\Users\baudo). Arwen (dll 11-11 13:43) et Faramir (dll 11-11 16:02) sont aussi plus vieux que leur dernier commit C++ : recompiler la copie choisie avant de donner la commande.
Commande a redonner seulement apres : fog fusionnee dans main, copie choisie recompilee, puis preflight.ps1 <nom> -Root <cette copie> PASS (le preflight verifie maintenant le binaire, le joueur nomme et les fichiers non commites).
