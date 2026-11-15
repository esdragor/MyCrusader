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

## Recette 2026-11-15 (build fusionné 753fd00)
RECETTE FAIL : les sons du tour 5 de la Caserne sont absents du build fusionné de Faramir, le scénario s'arrête au premier son (RECETTE-1115a, intégrateur indépendant, 2026-11-15 11:24 ; worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, branche fog = main 753fd00, binaire de 11:08).
- `preflight.ps1 audio_compare_buildings_barracks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` : FAIL (3) : check FAIL après 86 s (« a step timed out »), 1 ligne d'erreur au journal, 4 lignes d'asset manquant ; 2 WARN (pas de HUD overlap, pas de Sanity). Les contrôles statiques et « lancement joueur » sont OK (binaire à jour, rien de non commité à signaler : les candidats sont exclus de git). `session_gate.ps1 -Phase Essay -Essay audio_compare_buildings_barracks` : aucun FAIL (le worktree contient le tip de d751-root) : le code y est, pas les sons.
- Journal (Saved_Tests\Logs\scenario_audio_compare_buildings_barracks.log) : « LogUObjectGlobals: Warning: Objet 'Object /Game/Audio/Candidates/soft/SW_s_voice5_yell_a.SW_s_voice5_yell_a' introuvable », « candidate missing Building.Barracks A asset /Game/Audio/Candidates/soft/SW_s_voice5_yell_a.SW_s_voice5_yell_a », puis 60 s plus tard « Test sequence: timeout at step 18: waitlog candidate played Building.Barracks A ».
- Cause constatée (11:29) : `Content\Audio\Candidates` de Faramir = 140 fichiers (dossier soft, le plus récent du 2026-11-08), 0 du tour 5 ; celui d'Arwen = 373 fichiers, dont 30 du tour 5 (10 voice5, 10 shot5, 10 horse5, .uasset du 2026-11-15 09:34) ; aucun n'est suivi par git (exclusion `.git/info/exclude`).
- PNG OUVERT et vu : audio_compare_buildings_barracks_start (pris avant l'échec) : panneau du scénario lisible, « Ecoute le son de SELECTION de la CASERNE, tour 5 : des VOIX d'hommes d'armes et des lames, ENREGISTREES », Entrée = variante suivante ; le panneau couvre le haut du bâtiment du centre, qui a l'aspect du Forum (même image que pour le Champ de tir et l'Écurie) : aucune Caserne distincte vue.
- Commande de la fiche (Arwen) : NON REJOUÉE et à ne pas donner telle quelle : le binaire d'Arwen est périmé (Get-StaleBinaryProblem à 11:18 et 11:29 : dll du 09:11 plus vieille que Source\HeroesOfLiberty\Tests\HOLStartingHeroTests.cpp et Source\HeroesOfLibertyUI\Tests\HOLNewGameSitesBoxTests.cpp de 11:01).
- Commande complète depuis Faramir, valable SEULEMENT après copie des 30 .uasset du tour 5 d'Arwen vers Faramir et un pré-vol PASS (aujourd'hui elle échoue) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 audio_compare_buildings_barracks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
- Rien corrigé ni copié par la recette. Inchangé : GESTE interne, SON non entendu (`-nosound`, aucun son entendu par l'agent).
