# [À ESSAYER] E-audio_compare_buildings_stable : écouter les 10 sons de sélection de l'Écurie (vrai cheval enregistré, tour 5) et dire une lettre

STATUT : LOGIQUE SEULEMENT   (son jamais entendu par l'agent : c'est ton oreille qui décide)
GESTE: interne
SON: non entendu
branches: d751-root

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`.

Pré-vol (2026-11-15 09:40, agent ARWEN-1115b, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, d751-root 442b7de avec main fusionné) : `preflight.ps1 audio_compare_buildings_stable -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (37 s, 1 PNG ; 1 WARN commun : pas de hol.HUD.Report).
PNG OUVERTS et vus :
- audio_compare_buildings_stable_start : le panneau « Scénario audio_compare_buildings_stable » lisible au-dessus de l'Écurie : « Écoute le son de SÉLECTION de l'ÉCURIE, tour 5 : un vrai CHEVAL (hennissements, ébrouements, pas, trot), ENREGISTRÉ », 10 sons A à J, Entrée = variante suivante.
NON VÉRIFIÉ : aucun son entendu (-nosound) ; le niveau à l'oreille ; la licence des sons A-H (Red Library, marquage CC0 d'archive.org, voir Retours\Q-audio-licences-tour5.md).

Lettres (Tools/Audio/make_round5_candidates.py, liste Building.Stable de HOLAudioCandidates.as) :
A s_horse5_whinny_a, B s_horse5_whinny_b, C s_horse5_whinny_c (hennissements) ; D s_horse5_snort_a, E s_horse5_snort_b (ébrouements) ; F s_horse5_vocal_a, G s_horse5_vocal_b (ébrouement + voix) ; H s_horse5_walk (pas sur sol dur) ; I s_horse5_trot_a, J s_horse5_trot_b (trot).
Tour 4 synthétique refusé (D801 : « trop arcade, mais garder l'idée du cheval ») ; ici de vrais enregistrements coupés à 0,7-0,8 s et adoucis (passe-bas, centroïde <= 800 Hz).

Commande complète (règle 13, 2026-11-14), depuis Arwen (les sons du tour 5 ne sont que sur ce disque) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_stable -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min. Entrée = variante suivante, Backspace = précédente. Réponse attendue : une lettre A-J (ou plusieurs), « aucune, dix autres », ou `REGLAGE: ...`.
