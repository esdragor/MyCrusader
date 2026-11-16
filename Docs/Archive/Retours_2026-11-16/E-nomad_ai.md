# [À ESSAYER] E-nomad_ai : l'IA Nomade pose la fondation de son premier Forum et ses Paysans la BÂTISSENT (150 s pour 1 Paysan, D816), le forum n'est plus achevé d'un coup

STATUT : VU À L'ÉCRAN
GESTE: interne      (partie naturelle contre l'IA ; le scénario ne fait que placer la caméra par `hol.Camera.*` ; rien à cliquer : c'est un essai d'IA, pas de geste)
SON: sans objet
branches: hero-j8-close

Pré-vol (2026-11-16 15:19, agent CELEBORN-1116a, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, main 9c9d22b + 97f757c) : `preflight.ps1 nomad_ai -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn` PASS (47 s, 1 PNG).
Rouge d'avant le correctif (15:07, même scénario) : `timeout at step 13: waitlog Construction: slot 1 MainCentre at (` ; journal : `Construction: slot 1 finished MainCentre ... after 1 steps of work (by builders: 1 x1)`.
Vert (15:19) : `Construction: slot 1 first work on MainCentre at (-14000, -16800), 1 builders` puis `Construction: slot 1 MainCentre at (-14000, -16800), 2 builders at work`, aucun `finished MainCentre` avant la capture.
PNG OUVERTS et vus :
- nomad_ai_forum : au centre du cadre, un chantier de forum au contour rouge (couleur de l'IA) : dalle, quatre socles de tours ras, toit à plat au sol, pas le bâtiment dressé ; deux Paysans rouges au bord (en haut et à droite).
NON VÉRIFIÉ : la durée réelle jusqu'à l'achèvement (150 s / N^0,63 ; avec 5 Paysans ~54 s de travail) n'est pas mesurée (test C++ `MainCentre: 150 s for one builder` à faire, place C++ prise) ; le forum du joueur Nomade (nomad_start) suit la même donnée, non rejoué ; les centres secondaires (même type MainCentre, D783) prennent aussi 150 s, non rejoué.
Porte (15:21) : `session_gate.ps1 -Phase Essay -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn -Essay nomad_ai` : PASS tip hero-j8-close, PASS binaire ; FAIL attendu 'non integre' (BuildingTypes.uasset/.csv differents de main : branche hero-j8-close PRETE, pas fusionnee) ; la commande ci-dessous joue depuis Celeborn, qui contient le correctif.

À regarder : laisse tourner la partie : le forum rouge de l'IA monte peu à peu au lieu d'apparaître d'un coup ; ses Paysans arrivent un à un et travaillent dessus.

Commande complète (règle 13, 2026-11-14) :
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 nomad_ai -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
Reponse: OK (utilisateur, 2026-11-16 15:32, dans la conversation ; essai joué sur main 131b77d)
