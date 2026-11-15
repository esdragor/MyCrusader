# [À ESSAYER] E-audio_compare_buildings_barracks : écouter les 10 sons de sélection de la Caserne (voix d'hommes d'armes, lames, tour 5 enregistré) et dire une lettre

STATUT : LOGIQUE SEULEMENT   (son jamais entendu par l'agent : c'est ton oreille qui décide)
GESTE: interne
SON: non entendu
branches: d751-root

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`.

Pré-vol (2026-11-15 matin, agent ARWEN-1115a, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, d751-root) : `preflight.ps1 audio_compare_buildings_barracks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (1 PNG).
PNG OUVERTS et vus :
- audio_compare_buildings_barracks_start : le panneau du tour 5 de la Caserne lisible (ouvert par ARWEN-1115a).
NON VÉRIFIÉ : aucun son entendu (-nosound) ; le niveau à l'oreille ; les voix passent par un passe-bas (centroïde <= 800 Hz) et peuvent sembler étouffées ; J (fourreau) est le plus saturé (drive 7,5).

Lettres (Tools/Audio/make_round5_candidates.py, liste Building.Barracks de HOLAudioCandidates.as) :
A s_voice5_yell_a, B s_voice5_yell_b, C s_voice5_yell_c, D s_voice5_yell_d (cris) ; E s_voice5_grunt_a, F s_voice5_grunt_b (grognements) ; G s_voice5_crowd_a, H s_voice5_crowd_b (foule) ; I s_voice5_sword_draw (lame tirée) ; J s_voice5_sword_sheath (lame rengainée).
Tour 4 synthétique refusé (D801 : voix distordues) ; ici de vrais enregistrements coupés à 0,6-0,8 s et adoucis.

Commande complète (règle 13, 2026-11-14), depuis Arwen (les sons du tour 5 ne sont que sur ce disque) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_barracks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min. Entrée = variante suivante, Backspace = précédente. Réponse attendue : une lettre A-J (ou plusieurs), « aucune, dix autres », ou `REGLAGE: ...` (par exemple « voix trop étouffées »).
