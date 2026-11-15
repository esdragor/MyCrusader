# [À RÉPONDRE] Tremblement de la marche en bloc : garder la mémoire du détour autour d'un voisin de bloc ? (CELEBORN-1115b, hero-j8-close)

Contexte : deux soldats du même bloc qui convergent vers leur place tremblaient (cap qui change à chaque pas, neutrals_trace run3) : l'évitement les faisait tourner un pas et repartir droit le suivant, en phase. Correctif codé (5e01993) : le détour autour d'un membre du même bloc est gardé au plus 8 pas (`hol.Units.MarchMemberDetourSteps`, défaut 8 ; 0 = avant).

- A : garder 8 (défaut codé). Le test MarchPairNoTremble passe (le tremblement disparaît dans la simulation des deux soldats) ; en jeu neutrals_trace 3/8 avec, 3/8 sans, et le tremblement n'est sorti dans aucune des 16 passes (on ne l'a pas revu, ni avant ni après).
- B : revenir à 0 tant qu'une mesure en jeu n'a pas montré le tremblement disparaître (x16).
- C : une autre valeur (4 = plus serré sur la file, 16 = plus lisse).
- Autre.

Conséquence visible : avec A, les soldats d'un bloc qui se croisent font un petit écart régulier au lieu d'un tremblement ; risque (MV.2) d'écart de la file, borné à 8 pas (~25 cm).
Recommandation : A (valeur à changer : une ligne, `MarchMemberDetourSteps` dans HOLSimulationProcessors.cpp, ou la commande console).