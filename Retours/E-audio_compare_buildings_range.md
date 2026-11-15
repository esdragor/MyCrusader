# [À ESSAYER] E-audio_compare_buildings_range : écouter les 10 sons de sélection du Champ de tir (arc long, arbalète, volée, tour 5 enregistré) et dire une lettre

STATUT : LOGIQUE SEULEMENT   (son jamais entendu par l'agent : c'est ton oreille qui décide)
GESTE: interne
SON: non entendu
branches: d751-root

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`.

Pré-vol (2026-11-15 matin, agent ARWEN-1115a, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, d751-root) : `preflight.ps1 audio_compare_buildings_range -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (1 PNG).
PNG OUVERTS et vus :
- audio_compare_buildings_range_start : le panneau du tour 5 du Champ de tir lisible (ouvert par ARWEN-1115a).
NON VÉRIFIÉ : aucun son entendu (-nosound) ; le niveau à l'oreille ; H (volée, copyc4t) et J (carreau qui touche, spookymodem) sont sous CC-BY 3.0 : crédit à prévoir s'ils sont élus.

Lettres (Tools/Audio/make_round5_candidates.py, liste Building.Range de HOLAudioCandidates.as) :
A s_shot5_longbow_a, B s_shot5_longbow_b (tir d'arc long) ; C s_shot5_longbow_set (flèche posée) ; D s_shot5_nock (encoche) ; E s_shot5_crossbow_a, F s_shot5_crossbow_b (tir d'arbalète) ; G s_shot5_bow (arc) ; H s_shot5_volley_a (archers qui tirent) ; I s_shot5_arrow_pass (carreau qui passe) ; J s_shot5_bolt_hit (tir d'arbalète et impact).
Tour 4 synthétique refusé (D801 : « tirs de vaisseau, notes aléatoires ») ; ici de vrais enregistrements coupés à 0,6-0,8 s et adoucis.

Commande complète (règle 13, 2026-11-14), depuis Arwen (les sons du tour 5 ne sont que sur ce disque) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_range -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min. Entrée = variante suivante, Backspace = précédente. Réponse attendue : une lettre A-J (ou plusieurs), « aucune, dix autres », ou `REGLAGE: ...`.
