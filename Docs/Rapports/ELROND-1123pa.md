# ELROND-1123pa : preuve en jeu de la garnison de l'IA (S4-b, ai_garrison), 2026-11-23 15:09 -> 15:17

Impact joueur : l'IA entre bien ses archers dans sa tour attaquée (ordre de garnison émis), mais on ne les voit pas encore entrer sur l'écran.

- Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, branche integ-25. Commit dba2ffe (Agent: ELROND-1123pa) : Tools/Scenarios/ai_garrison.txt et .look (le .json est inchangé). Pas de C++, pas de build, pas de merge de main (déjà fusionné).
- Cause du FAIL de ELROND-1123j (lue au journal Saved_Tests/Logs/scenario_ai_garrison.log du 1er rejeu) : « AI slot 1 step 129: defence begins (8 enemies) / defend with 7 soldiers » (HOLAIArmy.as:66 DefenceRadius 3000 cm autour des lieux gardés). Les 7 défenseurs posés d'avance partent au contact, l'attaque-déplacement les tue (pas 190 à 353) avant que la tour soit frappée. Aucun défaut du code de garnison : le cerveau n'avait ni tour frappée ni unités à appeler.
- Réglage de scénario (1 seul) : la tour est posée seule, « before » la montre seule ; 8 Hommes d'armes l'attaquent ; `wait 4` puis 4 Archers de l'IA apparaissent à ~3 m ; `waitlog Garrison order: slot 1`.
- Journal vu au rejeu : `Garrison order: slot 1, 3 unit(s) queued (AI)` puis `AI slot 1 step 149: 1 garrison order(s)` (texte exact de HOLAIGarrisonOrders.cpp:38 et HOLAIBrain.as:96).
- Preflight ai_garrison : 3 rejeux (FAIL timeout step 32 ; puis scénario prêt en 41 s, journal propre, audit sanity OK, 2 PNG ; dernier rejeu : seul FAIL = `.look` non commité, commité ensuite). Pas de 4e rejeu (limite 3).
- PNG ouverts : ai_garrison_before = tour de guet seule au centre, Forum de l'IA en haut à droite, objet visible ; ai_garrison_after = 8 Hommes d'armes bleus collés à la tour, un point rouge à son pied, archers entrés NON distinguables ; le panneau affiche encore la note 1.
- Fiche C:\Users\baudo\Documents\MyCrusader\Retours\E-ai_garrison.md : LOGIQUE SEULEMENT (GESTE interne, SON non entendu, branches integ-25), commande play.ps1 en chemin absolu.
- Suivis mis à jour : Auto_Journey_File.md l.50 (cochée), EtapeDefini.md l.48, FortificationsInProgress.md l.263. Placeholders.txt : aucun provisoire ajouté ni retiré (relecture non faite en entier : NON VÉRIFIÉ).

## Champs
- TEST_ROUGE: scénario (waitlog) rouge vu 15:11 (timeout step 32), vert vu 15:13 ; TEST_ROUGE_SEPARE: non (rouge = l'état avant, commit de 1123j)
- BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; TYPES_A_EXPOSER: aucun ; DIAGNOSTICS_AS: sans objet (aucun .as touché)
- RESTE : voir les archers entrer (capture plus tard que 3 s après l'ordre, ou compteur de garnison à l'écran) ; `NoHits` (HOLAIBrain.as:93) fonctionne grâce à Building.SecondsSinceHit, pas de correctif de file.
- NON VÉRIFIÉ : entrée visible des archers ; répétabilité (un seul passage vert) ; les ~25 appels et l'heure d'arrêt ont été tenus à vue (15:17 < 15:23).
- Anomalie : deux 'warning LF -> CRLF' au commit (fichiers scénario écrits en LF). IN_USE.txt non touché (agent sans C++, non relu).
- PROCESSUS_DETACHES: voir le dernier geste (stray_processes) ci-dessous si lancé.
