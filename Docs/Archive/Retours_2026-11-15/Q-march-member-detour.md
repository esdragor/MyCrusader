# [À RÉPONDRE] Tremblement de la marche en bloc : garder la mémoire du détour autour d'un voisin de bloc ? (CELEBORN-1115b, hero-j8-close)

Contexte : deux soldats du même bloc qui convergent vers leur place tremblaient (cap qui change à chaque pas, neutrals_trace run3) : l'évitement les faisait tourner un pas et repartir droit le suivant, en phase. Correctif codé (5e01993) : le détour autour d'un membre du même bloc est gardé au plus 8 pas (`hol.Units.MarchMemberDetourSteps`, défaut 8 ; 0 = avant).

- A : garder 8 (défaut codé). Le test MarchPairNoTremble passe (le tremblement disparaît dans la simulation des deux soldats) ; en jeu neutrals_trace 3/8 avec, 3/8 sans, et le tremblement n'est sorti dans aucune des 16 passes (on ne l'a pas revu, ni avant ni après).
- B : revenir à 0 tant qu'une mesure en jeu n'a pas montré le tremblement disparaître (x16).
- C : une autre valeur (4 = plus serré sur la file, 16 = plus lisse).
- Autre.

Conséquence visible : avec A, les soldats d'un bloc qui se croisent font un petit écart régulier au lieu d'un tremblement ; risque (MV.2) d'écart de la file, borné à 8 pas (~25 cm).
Recommandation : A (valeur à changer : une ligne, `MarchMemberDetourSteps` dans HOLSimulationProcessors.cpp, ou la commande console).

## Mesure x16 2026-11-15 (CELEBORN-1115c, binaire 10:06, machine calme, sans build ; la question reste ouverte, défaut A codé)

Même binaire, une copie, passes une par une. Logs gardés dans `HeroesOfLiberty-Celeborn\Saved\c1115c\`.

| Réglage | neutrals_trace x16 | rejeu de nt_run3 x8 (écart d'ordres fixe 0,6 s) | « shakes » (tremblement de marche) | « spins » |
|---|---|---|---|---|
| 8 (défaut, A) | PASS 6/16 | PASS 4/8 | **0** (0 passe sur 24) | 18 |
| 0 (avant, B) | PASS 4/16 | PASS 3/8 | **3** (2 passes sur 24) | 22 |

- Le tremblement de la marche en bloc est **revu avec 0** : neutrals_trace_mm0 passe 6 (#12, cap qui alterne à chaque pas avec son voisin #10, même forme que nt_run3) et rejeu passe 1 (#9 et #13). **Avec 8, il n'est sorti dans aucune des 24 passes** (ni dans les 8 de CELEBORN-1115b ce matin ; 0 l'avait donné 0 fois sur 8 ce matin : il est rare, ~1 passe sur 10 avec 0).
- Les « spins » restants sont des boucles de seconde ligne pendant la mêlée du camp (connues, D641), pas la marche en bloc ; il y en a autant avec 8 qu'avec 0. Le spin de #9 à (-873, -346) sort seulement avec 8 (2 fois sur 16), mais au même pas d'ordre, 0 fait tourner une autre unité : le réglage change QUI tourne, pas combien.
- Le taux de PASS dépend surtout du pas de simulation où arrive l'ordre (même pas = même échec, au cm près) : 6/8 (CELEBORN-1115a) puis 3/8 (1115b) ce matin, c'est la variance, pas le correctif.

Recommandation (inchangée) : **A, garder 8** ; le tremblement disparaît en jeu, les autres échecs restent les mêmes.

Reponse (2026-11-15) : A : garder 8 pas (MarchMemberDetourSteps 8, défaut codé) ; mesure x16 : 0 tremblement sur 24 passes avec 8. (réponse donnée dans la conversation, 2026-11-15) Suite : reportée dans le GDD (D815) et le journal du chantier.
