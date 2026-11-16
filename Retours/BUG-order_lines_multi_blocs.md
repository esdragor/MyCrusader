# BUG-order_lines_multi_blocs : plusieurs unités sélectionnées hors d'un même bloc n'ont pas chacune leur ligne de mouvement

Signalé par l'utilisateur le 2026-11-16 (session principale, docs seulement, rien corrigé).
Constat de l'utilisateur : « quand je sélectionne plusieurs unités qui ne sont pas d'un même bloc, ils n'ont pas une ligne de mouvement chacun ».
Voulu (dit par l'utilisateur, précisé le 2026-11-16) : UNE ligne par bloc. Les unités d'un même bloc partagent la même ligne (pas une ligne par unité) ; des blocs différents ont chacun la leur.

Cause probable (lecture du code, non mesurée) : `AHOLSelectionMarkers::UpdateOrderLines` (Source\HeroesOfLiberty\Player\HOLSelectionMarkers.cpp, ~l. 1175-1225) trace UNE ligne par chaîne d'ordres (`FHOLNetOrderChain`), qui part du CENTRE de toutes les unités de la chaîne (`Start /= Count`). Un clic droit sur une sélection éparse crée une seule chaîne pour toutes les unités : la ligne part donc d'un point moyen (souvent au milieu de nulle part) au lieu de partir de chaque unité ou de chaque bloc.
Piste de correction : regrouper les unités de la chaîne par bloc et tracer une ligne depuis le centre de chaque bloc (une seule ligne quand toutes les unités forment un seul bloc).

Geste pour reproduire : sélectionner (cadre ou Maj+clic) des unités à deux endroits éloignés de la carte, clic droit sur un point du sol : une seule ligne s'affiche, partant du milieu des deux groupes.
Test à écrire d'abord (rouge) : deux groupes boîtés à 30 m l'un de l'autre, sélection des deux, ordre de déplacement ; contrôle = nombre de lignes affichées et leurs points de départ (près de chaque groupe).

Contrainte : le tracé est en C++ ; D830 (2026-11-16) arrête les nouvelles tâches C++ en attendant le plan de migration Angelscript (Docs\Rapports\ANALYSE-1116a.md). Correction à placer selon ce plan (le calcul des points de départ peut passer en Angelscript).
