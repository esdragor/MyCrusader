# [À ESSAYER] E-audio_compare_buildings_range : TOI SEUL peux juger ces sons (l'agent joue sans son) ; écoute les 10 sons du 4e tour (TIRS d'arc, synthétiques) pour la SÉLECTION du CHAMP DE TIR et choisis la lettre du meilleur

STATUT : VU À L'ÉCRAN pour l'affichage ; le SON reste à ton oreille (SON: non entendu)
GESTE: interne      (les sons sont joués par la commande `hol.Audio.Candidate` du scénario, pas par un vrai clic sur le Champ de tir ; seule la touche Entrée est la tienne)
SON: non entendu    (l'agent lance avec -nosound : il ne PEUT pas entendre ; passe à « entendu » quand tu as écouté)
branches: d751-root   (les candidats du 4e tour et leur liste HOLAudioCandidates.as ne sont que dans d751-root, worktree Arwen ; pas dans main)

Pré-vol (2026-11-14 13:10, ARWEN-1114d, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche d751-root 0a4ebc4, build PASS 13:05, Get-StaleBinaryProblem vide) : 30 sons du 4e tour importés (éditeur caché, 0 erreur) ; `preflight.ps1 audio_compare_buildings_range -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (34 s, journal sans erreur ; 1 WARN : pas de hol.HUD.Report).
PNG OUVERT et vu : `Saved_Tests\Screenshots\WindowsEditor\HOL\audio_compare_buildings_range_start.png` : panneau « Scénario audio_compare_buildings_range », texte juste (CHAMP DE TIR, tour 4, TIRS : corde d'arc, sifflement de flèche, impact ; dix variantes A à J ; réécoute `hol.Audio.Candidate Building.Range C`).
Niveau (check_audio_soft.py) : tous au-dessus du plancher -18 dB au-dessus de 200 Hz, centre spectral sous 800 Hz (corde plus douce au 4e tour, passe-bas 1 kHz).
NON VÉRIFIÉ : aucun des 10 sons n'a été entendu (-nosound) ; sons SYNTHÉTIQUES (corde Karplus-Strong, souffle filtré) ; le volume réel à ton oreille. L'ordre exact des lettres est celui de la liste Building.Range de HOLAudioCandidates.as (le panneau affiche le fichier à côté de la lettre).

Sons candidats exclus de git, présents SEULEMENT sur le disque d'Arwen : lance la commande telle quelle, avec `-Root ...HeroesOfLiberty-Arwen`.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_range -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min (3 min au plus).
Gestes : 1. attends le panneau « Écoute le son de SÉLECTION du CHAMP DE TIR » ; 2. Entrée = son A (le panneau dit la lettre et le fichier) ; 3. Entrée = son suivant jusqu'à J ; 4. après J, Entrée = fin. Réécouter : touche ² puis `hol.Audio.Candidate Building.Range C`, Entrée.
Réponse attendue : `OK` (dis la lettre), `BUG: ...` ou `REGLAGE: ...` (plusieurs lettres, ou « aucune, dix autres » en disant ce que tu aimes).
Champ de tir [Building.Range] = _

Sons (s_shot_*) : twang_low, twang, twang_whoosh, whoosh, volley, thunk, twang_thunk, draw, double, crossbow.

## Historique
- 2026-11-14 (D798) : « le reste n'est pas intéressant » : 4e tour d'une autre nature (tirs pour le Champ de tir).
- 2026-11-14 13:10 (ARWEN-1114d) : synthèse corrigée (312287a), importée, pré-vol PASS, PNG vu : [À ESSAYER] depuis Arwen.
