# Ordres Patrouille et Suivre (GDD § 7.3, D30, D340) : journal

Chantier B5 du relevé « Chapitres du GDD non commencés » (`Auto_Journey_Jour.md`). Agent GLORFINDEL-1115c, worktree
`C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel`, branche `net-playtest`, départ 17a9af8. SANS C++ (règles pures Angelscript).

## Existant (relevé 2026-11-15 10:19)
- Ordres d'unités en C++ : `EHOLOrderKind` (`Source\HeroesOfLiberty\Commands\HOLUnitCommand.h`) : Move, Attack, AttackMove, Stop, Hold.
- Grille (`HOLCommandGrid.h`) : Q = case 4 attaque-déplacement, S = 5 arrêt, V = 11 maintien, D = 6 Monter/Démonter (proposé).
  Cases libres du panneau des ordres : A Z E R (0-3), F (7), W (8), X (9), C (10).
- Aucun code `Patrol` / `Follow` ; l'IA donne ses ordres par `HOLAIOrder::IssueAIAttackMoveOrder`.

## Question
- `C:\Users\baudo\Documents\MyCrusader\Retours\Q-patrol-follow.md` [À RÉPONDRE] : case de chacun, Suivre (allié/ennemi), focus.
  Défaut codé : aucun (la règle pure ne choisit ni case ni touche ; le branchement C++ attend la réponse).

## Plan (étapes de 15 min)
1. [fait] Test rouge `Script\Orders\HOLPatrolRules_Test.as` + bouchon `HOLPatrolRules.as` ; VU ROUGE par `quick_check -Parts tests`.
   Rouge attendu : `Test_PatrolUnitGoesBackToAOnceAtB` (« l'unité repart vers A une fois en B ») et `Test_PatrolManualOrderStopsIt`.
2. [fait 3a636ce] Vert : `Advance` (demi-tour à `ArriveRadius` du bout de la jambe), `LegTarget`, `LegOrder` (AttackMove tant qu'active), `OnManualOrder`.
3. (après réponse, AGENT C++) brancher : nouvel `EHOLOrderKind::Patrol`, case de grille, relance de l'attaque-déplacement à chaque jambe
   dans `HOLUnitOrderProcessor.cpp`, fin sur tout ordre manuel ; scénario `patrol` (geste réel : touche puis clic).
4. (après réponse) règle pure de Suivre (allié : reste à distance ; ennemi : selon la réponse) et de focus.

## Décompte d'appels (un trait par appel vu)
||||||||||||||||

## Journal
- 10:19 ouvert ; test rouge + bouchon écrits.
- 10:23 VU ROUGE (`quick_check -Parts tests` : 1270/1272, KO = Test_PatrolUnitGoesBackToAOnceAtB et Test_PatrolManualOrderStopsIt) ; commit 85550d5.
- 10:27 VERT 3a636ce (tests 1272/1272). Le test 1 a été corrigé avec le code : il comptait des pas fixes (24 pas = déjà reparti
  vers B), il suit maintenant les arrivées (B atteint, puis A, puis repart > 400). Avec le bouchon il reste rouge (la jambe ne change
  jamais : A jamais rejoint) ; ce rouge-là est déduit, non relancé.
- Suivant pour un agent neuf : attendre `Q-patrol-follow.md`, puis étape 3 (agent C++). Aucun code Suivre/focus écrit.
