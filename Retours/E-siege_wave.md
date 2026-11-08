# [À ESSAYER] E-siege_wave : la vague d'une IA Difficile emmène ses Béliers, escortés par ses Lanciers, jusqu'à ton Centre

STATUT : VU À L'ÉCRAN pour la vague et l'arrivée des Béliers ; LOGIQUE SEULEMENT pour « partie naturelle complète » (voir NON VÉRIFIÉ : l'armée et les engins sont DONNÉS à l'IA par le scénario, seule la décision de partir en vague et l'emploi des engins sont ceux de l'IA)
GESTE: interne   (les engins et l'armée de l'IA sont posés par hol.Units.SpawnHere ; l'Entrée du scénario envoie les 8 Hommes d'armes par commande ; aucun clic de joueur)
SON: non entendu
branches: fog

Pré-vol (2026-11-08 11:10, agent Faramir-14, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, main c158640) : `preflight.ps1 siege_wave -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` PASS (84 s, 4 PNG, aucun recouvrement, audit Sanity passé).
PNG OUVERTS et vus :
- siege_wave_base : forum de l'IA, Lanciers rouges et deux grands mannequins gris (les Béliers) devant lui.
- siege_wave_march : Lanciers et Béliers en marche hors du forum, côté ton camp.
- siege_wave_arrive : devant ton Centre, un Bélier gris et un Lancier rouge contre la tour, ton mini-plan montre la vague.
- siege_wave_hits : deux Béliers et un Lancier rouge contre ton Centre ; les journaux disent « Attacking, target #0 » pour toute l'escorte.
Observé au journal (Units.Where) : l'escorte arrive la première (Lanciers à -13 960 cm de X, devant le Centre), les Béliers 3 000 cm derrière à la même seconde (escorte étirée, comme le mesure déjà `quick_check -Parts aiwave`). Le Mangonneau de l'IA n'a PAS suivi : tiré par les tirs de tes 8 Hommes d'armes derrière son forum, il est resté à son camp (hp 92/200) et est mort (NON tranché : c'est la conduite du scénario, pas un défaut établi).
NON VÉRIFIÉ : une vraie partie complète où l'IA bâtit l'Atelier et produit seule les engins puis part (seule la partie de 840 s de `-Parts aisiege` le prouve, sans écran) ; Facile et Normal : par défaut ils ne bâtissent PAS d'Atelier (réglage `FHOLAISiegeSettings.MinDifficulty = 2`, Hard seulement), donc aucune vague de siège Facile/Normal à observer sans changer ce réglage (décision de design, non prise) ; le Bélier qui frappe un bâtiment dans cette vague (les journaux disent « Attacking » ; le coup est prouvé par `siege_raze`) ; le son.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 siege_wave -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
Durée : environ 2 min (Entrée, puis ~50 s de marche à vitesse x5). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.


integre sur main fc601b7 (fusion groupee n1, 2026-11-08 11:38) : session_gate -Phase Essay PASS ; preflight siege_wave PASS sur ce build (Sauron), PNG siege_wave_hits ouvert : Centre en ruine, 2 engins gris et Lanciers rouges contre Hommes d'armes bleus, HUD 'Votre forum est tombe - 1 vie' ; statut VU A L'ECRAN (geste interne tel que declare dans la fiche).
Reponse: OK (works) + BUG: les unites ont 'attaque' dans leurs stats mais la valeur manque (utilisateur, 2026-11-08 ~11:55). Agent Galadriel-4 lance.
