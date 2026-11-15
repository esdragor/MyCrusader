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

## Recette 2026-11-15 (build fusionné 753fd00)
RECETTE FAIL : les sons du tour 5 de l'Écurie sont absents du build fusionné de Faramir, le scénario s'arrête au premier son (RECETTE-1115a, intégrateur indépendant, 2026-11-15 11:27 ; worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, branche fog = main 753fd00, binaire de 11:08).
- `preflight.ps1 audio_compare_buildings_stable -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` : FAIL (3) : check FAIL après 80 s (« a step timed out »), 1 ligne d'erreur au journal, 4 lignes d'asset manquant ; 2 WARN (pas de HUD overlap, pas de Sanity). Les contrôles statiques et « lancement joueur » sont OK (binaire à jour ; les candidats sont exclus de git). `session_gate.ps1 -Phase Essay -Essay audio_compare_buildings_stable` : aucun FAIL (le worktree contient le tip de d751-root) : le code y est, pas les sons.
- Journal (Saved_Tests\Logs\scenario_audio_compare_buildings_stable.log) : « LogUObjectGlobals: Warning: Objet 'Object /Game/Audio/Candidates/soft/SW_s_horse5_whinny_a.SW_s_horse5_whinny_a' introuvable », « candidate missing Building.Stable A asset /Game/Audio/Candidates/soft/SW_s_horse5_whinny_a.SW_s_horse5_whinny_a », puis 60 s plus tard « Test sequence: timeout at step 18: waitlog candidate played Building.Stable A ».
- Cause constatée (11:29) : `Content\Audio\Candidates` de Faramir = 140 fichiers (dossier soft, le plus récent du 2026-11-08), 0 du tour 5 ; celui d'Arwen = 373 fichiers, dont 30 du tour 5 (10 voice5, 10 shot5, 10 horse5, .uasset du 2026-11-15 09:34) ; aucun n'est suivi par git (exclusion `.git/info/exclude`).
- PNG OUVERT et vu : audio_compare_buildings_stable_start (pris avant l'échec) : panneau du scénario lisible, « Ecoute le son de SELECTION de l'ECURIE, tour 5 : un vrai CHEVAL (hennissements, ebrouements, pas, trot), ENREGISTRE », Entrée = variante suivante ; le panneau couvre le haut du bâtiment du centre, qui a l'aspect du Forum (même image que pour la Caserne et le Champ de tir) : aucune Écurie distincte vue, alors que le pré-vol d'Arwen écrit « au-dessus de l'Écurie ».
- Commande de la fiche (Arwen) : NON REJOUÉE et à ne pas donner telle quelle : le binaire d'Arwen est périmé (Get-StaleBinaryProblem à 11:18 et 11:29 : dll du 09:11 plus vieille que Source\HeroesOfLiberty\Tests\HOLStartingHeroTests.cpp et Source\HeroesOfLibertyUI\Tests\HOLNewGameSitesBoxTests.cpp de 11:01).
- Commande complète depuis Faramir, valable SEULEMENT après copie des 30 .uasset du tour 5 d'Arwen vers Faramir et un pré-vol PASS (aujourd'hui elle échoue) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 audio_compare_buildings_stable -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
- Rien corrigé ni copié par la recette. Inchangé : GESTE interne, SON non entendu (`-nosound`, aucun son entendu par l'agent).

## Reprise 2026-11-15 (Arwen mis à jour sur main)
Agent ARWEN-1115e, 2026-11-15 11:32-11:45, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche d751-root avancée sur main 3c3150d (deux avances rapides : 5d83ef2 puis 3c3150d, main ayant bougé pendant la reprise), `Tools\build.ps1 -LiveCoding=0 -Wait` PASS (11:35 puis 11:39), Get-StaleBinaryProblem vide après le build.
- Sons : les 10 `SW_s_horse5_*.uasset` sont présents dans `Content\Audio\Candidates\soft\` d'Arwen (hors git).
- `preflight.ps1 audio_compare_buildings_stable -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` : PASS (check PASS 31 s, le scénario atteint « ready » donc chaque `waitlog candidate played Building.Stable A..J` est passé ; aucun asset manquant, aucune étape expirée, aucune erreur ; 1 WARN : pas de ligne HUD overlap).
- `session_gate.ps1 -Phase Essay -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen -Essay audio_compare_buildings_stable` : aucun FAIL (aucun écart de code avec main 3c3150d, binaire 11:39, tip de d751-root contenu).
- PNG OUVERT et vu (11:41) : audio_compare_buildings_stable_start : panneau « tour 5 » de l'ÉCURIE lisible ; le bâtiment au centre a l'aspect du Forum, et c'EST le Forum de départ : le scénario ne pose aucune Écurie (il attend les héros de départ puis `hol.Camera.FocusNear -900 300` sur le point de départ ; le son est joué par `hol.Audio.Candidate`, sans sélection de bâtiment). Le texte « au-dessus de l'Écurie » du pré-vol du matin était donc faux. Même image pour la Caserne et le Champ de tir. Sans effet sur l'écoute.
- CORRIGÉ 2026-11-15 11:46-11:52 (agent ARWEN-1115f, sans C++, d751-root 4688f7b puis main 13606d6 fusionné = 5aa2c16) : le scénario pose maintenant une Écurie finie à toi (`hol.Cheat.SpawnBuilding Stable 0 -3800 4600`, la place du Champ de tir d'archers.txt, derrière le centre ; `waitlog Cheat: spawned Stable of slot 0`) et centre la caméra dessus (`hol.Camera.FocusNear -3800 4600`). Test rouge d'abord (0c0564c : le .look exige « le bâtiment Ecurie est visible au centre de l'écran, pas le Forum » ; le PNG de 11:41 montrait le Forum = ROUGE). Puis `preflight.ps1 audio_compare_buildings_stable -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (34 s, 1 WARN : pas de HUD overlap) et `session_gate.ps1 -Phase Essay -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen -Essay audio_compare_buildings_stable` aucun FAIL.
- PNG OUVERT et vu (11:50) : audio_compare_buildings_stable_start : l'Écurie provisoire (bloc bleu, couleur du joueur, toit bleu clair) au centre de l'écran, sous le panneau ; le Forum n'est plus qu'un coin en haut à gauche. Attention : l'Écurie provisoire a exactement la même forme que la Caserne provisoire (BlockAndTower, BuildingTypes.csv) : à l'image, rien ne la distingue de la Caserne ; c'est bien une Écurie (le scénario attend « spawned Stable » au journal), mais seule la forme de remplacement à venir la rendra reconnaissable.
- Commande complète (règle 13), copiable telle quelle :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_stable -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
- Inchangé : GESTE interne, SON non entendu (`-nosound`, aucun son entendu par l'agent) : reste LOGIQUE SEULEMENT tant que tu n'as pas écouté.
