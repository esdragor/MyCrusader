# E-playtest_hero_army : combat A (avec héros) vs B (sans héros), 6 Hommes d'armes contre 10 Lanciers — PREFLIGHT FAIL (audit Sanity), LOGIQUE SEULEMENT

STATUT : LOGIQUE SEULEMENT (preflight FAIL au pas 5 « sanity audit », pas listé [À ESSAYER]). Les 4 PNG ont été ouverts et vus.

Pré-vol (2026-11-05 21:19, agent d'intégration n°2, worktree HeroesOfLiberty-Elrond = main eb7b5fe, SANS build) : `preflight.ps1 playtest_hero_army -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` : check PASS (63 s), 1 waitkey (Entrée = combat B, voulu), 40 s d'attente explicite, journal sans warning/erreur. FAIL : `Sanity: FAIL unit #18 (slot 0) at (-15562, 665) spins over 8 s: path 312 cm, net 144 cm, turn 20.2 rad (net 10.6, to and fro 1.3), 3 reversals, ordered 100%` : un Homme d'armes tourne sur place en mêlée (même famille que BUG-melee-spin-sanity.md, D641). REJOUÉ une 2e fois (21:23) : FAIL de nouveau, autre unité : `Sanity: FAIL unit #16 (slot 0) at (-15541, 709) spins over 8 s: path 126 cm, net 95 cm, turn 16.2 rad (net 15.5, to and fro 0.4), 5 reversals, ordered 100%` : 2 passages sur 2, donc reproductible ici (voir BUG-playtest_hero_army-spin.md).

PNG OUVERTS et vus :
- playtest_hero_army_a_start : forum bleu, héros (anneau doré, niveau 3, PV 522/522) et 6 Hommes d'armes bleus devant ; les 10 Lanciers rouges sont une petite grappe au bord droit (~7 m) ; rien ne bouge.
- playtest_hero_army_a_end : mêlée en cours à droite du forum, corps rouges au sol (4-5 visibles), survivants bleus ; héros PV 506/522 (a perdu 16 PV). Les Lanciers rouges NE sont PAS tous morts à la capture (reste d'une grappe rouge vivante) : « fin du combat A » = temps fixe, pas fin réelle.
- playtest_hero_army_b_start : le héros est tombé (PV 0/522, « Prêt dans 0:54 »). Le combat B n'est PAS « plus bas sur la carte » comme dit le .look : il se déroule dans la même zone visible que A (corps rouges de A au sol à gauche, survivants de A en rang, un reste de mêlée A encore actif près du forum) ; B (6 bleus / grappe rouge) est juste à droite.
- playtest_hero_army_b_end : mêlée de B en cours (bleus et rouges mêlés, ~10 rouges encore debout), pas de « fin » nette ; héros toujours mort (« Prêt dans 0:34 »).

Défauts du scénario (à décider/corriger par l'auteur, rien tranché ici) : (1) les deux combats sont proches et A n'est pas fini quand B démarre, ce qui pollue la comparaison A/B annoncée par la note 4 ; (2) les captures « end » tombent en pleine mêlée, donc rien à comparer à l'écran (le résultat est dans le journal : lignes `Combat: ... killed`, non dépouillées) ; (3) le .look contient des accents mal encodés à la lecture par preflight (« alliÃ©s ») : fichier non UTF-8 ou lu en CP1252 (les notes du panneau, elles, s'affichent bien).

NON VÉRIFIÉ : l'issue réelle de A et de B (nombre de survivants : journal non dépouillé) ; si l'écart A/B est significatif ; la même Sanity sur un second passage.

Lancer UNE commande (pour l'utilisateur, à ses risques, non prêt) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 playtest_hero_army -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min 15 + attente de la touche Entrée. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
