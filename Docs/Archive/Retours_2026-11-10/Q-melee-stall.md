# [À RÉPONDRE] Q-melee-stall : le contrôle Sanity.Stall (0,5 s) doit-il tolérer la fin d'une marche FinalLeg ?

Contexte : un soldat en marche FinalLeg (phase 3, place réservée) tenu par ses amis, hors de portée de sa cible, reste jusqu'à 2 s sans avancer
(FinalLegStuckSteps = 40 pas) avant de combattre ; le contrôle `hol.Test.Sanity` (stall > 0,5 s hors de portée, place libre) le signale
(melee_stuck_old, watch pas 372, etc.). Mesures de Celeborn-17 : voir NeutralsInProgress.md (tableau FAIL/8 ON contre ALLOFF).

Options :
- **A. Le contrôle tolère** (stall autorisé jusqu'à FinalLegStuckSteps + marge, ~2,2 s, tant que la marche FinalLeg est en cours) : plus de FAIL Sanity, le comportement du jeu ne change pas ; un soldat peut attendre ~2 s derrière ses amis avant de combattre.
- **B. Le jeu coupe la marche** (StopWalk dès `IsWalkStalled(StuckSteps, WalkRiposteSteps)`, ~10 pas) : le soldat arrête d'attendre et combat/cherche une autre cible plus tôt ; risque : reprend une marche au pas de recherche suivant (cycle) si hors de portée.
- **C. Raccourcir FinalLegStuckSteps** (40 -> ~20 pas) : l'attente tombe à ~1 s, un seul réglage, le contrôle reste à 0,5 s mais passe plus souvent.
- **Autre** : à préciser.

Conséquences visibles : A = rien à l'écran, seulement l'audit ; B = les soldats du second rang réagissent plus vite mais peuvent osciller ; C = marche d'approche abandonnée plus tôt dans les mêlées denses.

Recommandation : A (le défaut est un blocage de 2 s derrière des amis qui se battent, pas un soldat figé pour toujours ; ne change pas le comportement validé par les mesures).

Défaut codé : aucun changement (le contrôle reste à 0,5 s, cvar existante `hol.Combat.WalkRiposteSteps` / constante FinalLegStuckSteps inchangées) ; un seul réglage à changer le moment venu.

**Reponse (utilisateur, 2026-11-07 15:4x) : A** - le controle Sanity.Stall tolere la fin d'une marche FinalLeg (jusqu'a FinalLegStuckSteps + marge ~2,2 s). Rien ne change en jeu. A coder : tolerance dans l'audit hol.Test.Sanity (C++, test rouge d'abord). Piste mesuree (Celeborn-19) : SlotBlockedStopDistance 250 -> 120/150 a mesurer x8 sur les 3 scenarios.
