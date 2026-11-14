# [À ESSAYER] E-audio_compare_buildings_barracks : TOI SEUL peux juger ces sons (l'agent joue sans son) ; écoute les 10 sons du 4e tour (VOIX d'hommes d'armes, synthétiques) pour la SÉLECTION de la CASERNE et choisis la lettre du meilleur

STATUT : VU À L'ÉCRAN pour l'affichage ; le SON reste à ton oreille (SON: non entendu)
GESTE: interne      (les sons sont joués par la commande `hol.Audio.Candidate` du scénario, pas par un vrai clic sur la Caserne ; seule la touche Entrée est la tienne)
SON: non entendu    (l'agent lance avec -nosound : il ne PEUT pas entendre ; passe à « entendu » quand tu as écouté)
branches: d751-root   (les candidats du 4e tour et leur liste HOLAudioCandidates.as ne sont que dans d751-root, worktree Arwen ; pas dans main)

Pré-vol (2026-11-14 13:10, ARWEN-1114d, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche d751-root 0a4ebc4, build PASS 13:05, Get-StaleBinaryProblem vide) : 30 sons du 4e tour importés (éditeur caché, 0 erreur) ; `preflight.ps1 audio_compare_buildings_barracks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (37 s, journal sans erreur ; 1 WARN : pas de hol.HUD.Report).
PNG OUVERT et vu : `Saved_Tests\Screenshots\WindowsEditor\HOL\audio_compare_buildings_barracks_start.png` : panneau « Scénario audio_compare_buildings_barracks », texte juste (CASERNE, tour 4, VOIX d'hommes d'armes, 10 sons, dix variantes A à J, réécoute `hol.Audio.Candidate Building.Barracks C`) ; château, héros, 5 soldats bleus.
Niveau (check_audio_soft.py) : tous au-dessus du plancher -18 dB au-dessus de 200 Hz, centre spectral sous 800 Hz (voix assombries au 4e tour : pente plus forte, passe-bas 1,2 kHz).
NON VÉRIFIÉ : aucun des 10 sons n'a été entendu par personne (-nosound) ; voix SYNTHÉTIQUES (aucun enregistrement libre de voix dans MyCrusader_Audio) : elles peuvent sonner robotiques ; le volume réel à ton oreille.

Sons candidats exclus de git, présents SEULEMENT sur le disque d'Arwen : lance la commande telle quelle, avec `-Root ...HeroesOfLiberty-Arwen`.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_barracks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min (3 min au plus).
Gestes : 1. attends le panneau « Écoute le son de SÉLECTION de la CASERNE » ; 2. Entrée = son A (le panneau dit la lettre et le fichier) ; 3. Entrée = son suivant jusqu'à J ; 4. après J, Entrée = fin. Réécouter : touche ² puis `hol.Audio.Candidate Building.Barracks C`, Entrée.
Réponse attendue : `OK` (dis la lettre), `BUG: ...` ou `REGLAGE: ...` (plusieurs lettres, ou « aucune, dix autres » en disant ce que tu aimes).
Caserne [Building.Barracks] = _

Lettres : A s_voice_hah, B s_voice_ho, C s_voice_hey, D s_voice_hup, E s_voice_duo, F s_voice_trio, G s_voice_grunt, H s_voice_murmur, I s_voice_whisper, J s_voice_order

## Historique
- 2026-11-14 (D798) : « le reste n'est pas intéressant » : 4e tour d'une autre nature (voix pour la Caserne).
- 2026-11-14 13:10 (ARWEN-1114d) : synthèse corrigée (312287a), importée, pré-vol PASS, PNG vu : [À ESSAYER] depuis Arwen.
