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

## Recette 2026-11-15 (build fusionné 753fd00)
RECETTE FAIL : les sons du tour 5 du Champ de tir sont absents du build fusionné de Faramir, le scénario s'arrête au premier son (RECETTE-1115a, intégrateur indépendant, 2026-11-15 11:25 ; worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, branche fog = main 753fd00, binaire de 11:08).
- `preflight.ps1 audio_compare_buildings_range -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` : FAIL (3) : check FAIL après 86 s (« a step timed out »), 1 ligne d'erreur au journal, 4 lignes d'asset manquant ; 2 WARN (pas de HUD overlap, pas de Sanity). Les contrôles statiques et « lancement joueur » sont OK (binaire à jour ; les candidats sont exclus de git). `session_gate.ps1 -Phase Essay -Essay audio_compare_buildings_range` : aucun FAIL (le worktree contient le tip de d751-root) : le code y est, pas les sons.
- Journal (Saved_Tests\Logs\scenario_audio_compare_buildings_range.log) : « LogUObjectGlobals: Warning: Objet 'Object /Game/Audio/Candidates/soft/SW_s_shot5_longbow_a.SW_s_shot5_longbow_a' introuvable », « candidate missing Building.Range A asset /Game/Audio/Candidates/soft/SW_s_shot5_longbow_a.SW_s_shot5_longbow_a », puis 60 s plus tard « Test sequence: timeout at step 18: waitlog candidate played Building.Range A ».
- Cause constatée (11:29) : `Content\Audio\Candidates` de Faramir = 140 fichiers (dossier soft, le plus récent du 2026-11-08), 0 du tour 5 ; celui d'Arwen = 373 fichiers, dont 30 du tour 5 (10 voice5, 10 shot5, 10 horse5, .uasset du 2026-11-15 09:34) ; aucun n'est suivi par git (exclusion `.git/info/exclude`).
- PNG OUVERT et vu : audio_compare_buildings_range_start (pris avant l'échec) : panneau du scénario lisible, « Ecoute le son de SELECTION du CHAMP DE TIR, tour 5 : arc long, arbalete, volee d'archers, ENREGISTRES », Entrée = variante suivante ; le panneau couvre le haut du bâtiment du centre, qui a l'aspect du Forum (même image que pour la Caserne et l'Écurie) : aucun Champ de tir distinct vu.
- Commande de la fiche (Arwen) : NON REJOUÉE et à ne pas donner telle quelle : le binaire d'Arwen est périmé (Get-StaleBinaryProblem à 11:18 et 11:29 : dll du 09:11 plus vieille que Source\HeroesOfLiberty\Tests\HOLStartingHeroTests.cpp et Source\HeroesOfLibertyUI\Tests\HOLNewGameSitesBoxTests.cpp de 11:01).
- Commande complète depuis Faramir, valable SEULEMENT après copie des 30 .uasset du tour 5 d'Arwen vers Faramir et un pré-vol PASS (aujourd'hui elle échoue) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 audio_compare_buildings_range -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
- Rien corrigé ni copié par la recette. Inchangé : GESTE interne, SON non entendu (`-nosound`, aucun son entendu par l'agent).

## Reprise 2026-11-15 (Arwen mis à jour sur main)
Agent ARWEN-1115e, 2026-11-15 11:32-11:45, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche d751-root avancée sur main 3c3150d (deux avances rapides : 5d83ef2 puis 3c3150d, main ayant bougé pendant la reprise), `Tools\build.ps1 -LiveCoding=0 -Wait` PASS (11:35 puis 11:39), Get-StaleBinaryProblem vide après le build.
- Sons : les 10 `SW_s_shot5_*.uasset` sont présents dans `Content\Audio\Candidates\soft\` d'Arwen (hors git).
- `preflight.ps1 audio_compare_buildings_range -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` : PASS (check PASS 31 s, le scénario atteint « ready » donc chaque `waitlog candidate played Building.Range A..J` est passé ; aucun asset manquant, aucune étape expirée, aucune erreur ; 1 WARN : pas de ligne HUD overlap).
- `session_gate.ps1 -Phase Essay -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen -Essay audio_compare_buildings_range` : aucun FAIL (aucun écart de code avec main 3c3150d, binaire 11:39, tip de d751-root contenu).
- PNG OUVERT et vu (11:40) : audio_compare_buildings_range_start : panneau « tour 5 » du CHAMP DE TIR lisible ; le bâtiment au centre a l'aspect du Forum, et c'EST le Forum de départ : le scénario ne pose aucun Champ de tir (il attend les héros de départ puis `hol.Camera.FocusNear -900 300` sur le point de départ ; le son est joué par `hol.Audio.Candidate`, sans sélection de bâtiment). Même image pour la Caserne et l'Écurie. Sans effet sur l'écoute.
- CORRIGÉ 2026-11-15 11:46-11:52 (agent ARWEN-1115f, sans C++, d751-root 4688f7b puis main 13606d6 fusionné = 5aa2c16) : le scénario pose maintenant un Champ de tir fini à toi (`hol.Cheat.SpawnBuilding ArcheryRange 0 -3800 4600`, la place du Champ de tir d'archers.txt, derrière le centre ; `waitlog Cheat: spawned ArcheryRange of slot 0`) et centre la caméra dessus (`hol.Camera.FocusNear -3800 4600`). Test rouge d'abord (0c0564c : le .look exige « le bâtiment Champ de tir est visible au centre de l'écran, pas le Forum » ; le PNG de 11:40 montrait le Forum = ROUGE). Puis `preflight.ps1 audio_compare_buildings_range -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (34 s, 1 WARN : pas de HUD overlap) et `session_gate.ps1 -Phase Essay -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen -Essay audio_compare_buildings_range` aucun FAIL.
- PNG OUVERT et vu (11:50) : audio_compare_buildings_range_start : le Champ de tir provisoire (bloc blanc à trois cibles, bande bleue du joueur) au centre de l'écran, sous le panneau ; le Forum n'est plus qu'un coin en haut à gauche.
- Commande complète (règle 13), copiable telle quelle :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings_range -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
- Inchangé : GESTE interne, SON non entendu (`-nosound`, aucun son entendu par l'agent) : reste LOGIQUE SEULEMENT tant que tu n'as pas écouté.
