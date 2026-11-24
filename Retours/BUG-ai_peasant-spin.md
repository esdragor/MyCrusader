# BUG-ai_peasant-spin : l'audit « Sanity » signale un Paysan de l'IA qui tourne sans but (pas de mêlée, aucune cible)

Découvert par : FARAMIR-1124sk (Paysan IA #7 cité dans AIInProgress.md), reconfirmé par FARAMIR-1124sl 2026-11-24 21:25 en remettant `hol.Sanity.Watch 1` / `hol.Test.Sanity` dans `ai_tower` (branche faramir-1124sg, commit 90c4593, Plain, toi slot 0 contre IA Normale slot 1).

Ligne du log (`Saved_Tests\Logs\scenario_ai_tower.log`) : `Sanity: FAIL unit #7 (slot 1) at (16702, -1989) spins over 8 s: path 1520 cm, net 0 cm, turn 13.3 rad (net -6.0, to and fro 0.0), 2 reversals, ordered 25%`.

Relevé de l'unité : #7 = Paysan IA de départ (type d'après FARAMIR-1124sk), position (16702, -1989) (~2,7 km de la tour posée en (14800, 0), près des arbres de l'IA), ordre : `ordered 25%` (la moitié de la fenêtre sans ordre). SanityState #7 : 20 pas immobile (`ap0 at0 t-1 ph0 v0,0`), puis marche `ph2` en changeant de vitesse (v 637,484 -> 243,762 -> -72,797 -> -467,650) : aucune cible (`t-1`), aucune attaque (`at0`).

Comparaison avec BUG-melee-spin-sanity.md : DIFFÉRENT. Celui-là est une mêlée (`at1`, cible `t12`, soldats d'armes en seconde ligne, D641). Ici : un récolteur sans ennemi, sans combat, immobile puis qui vire sur place avec un ordre de déplacement qui change ; piste : récolte / évitement (le Paysan reçoit un nouveau chemin à chaque pas ou son point de récolte est occupé). Non rattaché à D641 ; fiche séparée, pas de preuve ajoutée à BUG-melee-spin-sanity.md.

Conséquence : le scénario `ai_tower` reste LOGIQUE SEULEMENT (preflight FAIL) tant que ce spin n'est pas corrigé ; l'audit n'est pas retiré.
