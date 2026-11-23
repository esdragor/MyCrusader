# [RÉPONDU : REGLAGE, à rejouer après correction] E-faction_click_cast : cliquer la case Lumière sacrée du panneau de faction, puis le sol : l'anneau doré apparaît et l'Honneur passe de 100 à 75

STATUT : VU À L'ÉCRAN
GESTE: réel (clic de la case par hol.Mouse.Click FactionBox0 ; le clic sol est hol.Select.LeftClickGround, interne)
SON: non entendu
branches: main
Reponse: REGLAGE: actuellement, ca se met auto au milieu de l'armée, je devrai pouvoir choisir avec la position de ma souris (utilisateur, 2026-11-23 15:59, D897)

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`.

Pré-vol (2026-11-23 15:44, agent RECETTE-1123pa, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, main b5a285b + scénario 21e336f sur integ-58) : `preflight.ps1 faction_click_cast` rejoué après correction du scénario ; les points du .look sont OK (3 PNG), seule la ligne « 1 fichier non commité » a échoué avant le commit 21e336f (corrigé).
Premier jeu (15:41) : le scénario d'origine (héros + 5 paysans, aucun soldat) donnait `RpcPower: cast refused (4)` au pied du héros : origine = centre des soldats (HOLFactionPowerServer.as CastOriginOf + HOLAISummary.cpp:105 qui exclut héros et roi) = (0, 0) quand il n'y a aucun soldat. Le scénario fait donc apparaître 4 Hommes d'armes avant le lancer. Le cas « héros seul » reste refusé : voir le rapport RECETTE-1123pa.

À FAIRE et VOIR (gestes précis) :
1. Le jeu s'ouvre sur la plaine, ton château au centre bas, panneau de faction en bas à gauche : « 100 / 100 », « Lumière sacrée : coût 25, prêt ».
2. Clic sur la case « Lumière sacrée » : la case prend un contour (visée armée).
3. Clic sur le sol près de ton héros : un anneau doré (deux cercles, rayon 600 cm, 5 s) apparaît sous tes unités ; le panneau passe à « 75 / 100 » et « Lumière sacrée : coût 25, recharge 20 s ».

PNG OUVERTS et vus :
- faction_click_cast_before : panneau « 100 / 100 », « Lumière sacrée : coût 25, prêt » ; Population 9 ; seul le grand cercle pâle de portée est visible ; aucun anneau doré.
- faction_click_cast_aim : même scène, la case Lumière sacrée porte un contour (armée) ; pas d'anneau doré, 100 / 100.
- faction_click_cast_after : deux anneaux dorés épais autour du héros et des soldats devant le château ; panneau « 75 / 100 », « Lumière sacrée : coût 25, recharge 20 s ».
Journal : `Sacred Light put down by slot 0, Honor now 75` ; `HUD faction slot: slot gauge 75 / 100 ... cooling 20 s`.
NON VÉRIFIÉ : aucun blessé dans la zone, donc la guérison (points de vie qui remontent) n'est PAS vue ni lue ; son non entendu ; « héros seul sans soldat » = refus (4), défaut du jeu ; le curseur de visée n'est pas visible sur le PNG (seul le contour de la case l'est).

Commande complète (règle 13, 2026-11-14) :
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 faction_click_cast -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
