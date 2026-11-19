# [À RÉPONDRE] Marche en bloc : les files s'écartent jusqu'à 1,7 m avec le réglage choisi le 2026-11-15 (D815) ; quelle valeur garder ?

Contexte : le 2026-11-15 tu as choisi de garder 8 pas de « mémoire du détour » autour d'un voisin de bloc (`hol.Units.MarchMemberDetourSteps 8`, D815), pour supprimer un tremblement de deux soldats. L'estimation donnée ce jour-là (« écart de file d'environ 25 cm ») était fausse : mesuré le 2026-11-19, des files d'un bloc en marche s'écartent jusqu'à 174 cm, et le contrôle de formation de `main` est rouge depuis (seuil 60 cm). Ce rouge ne vient pas de la migration.

Mesures (contrôle `quick_check -Parts formation`, 12 marches ; tremblement = scénario `neutrals_trace`, une passe = un lancement du jeu) :

| Valeur | Plus grand écart de file | Contrôle des files | Tremblement de deux soldats |
|---|---|---|---|
| 8 (aujourd'hui) | 174 cm | ROUGE | 0 passe sur 24 |
| 6 | 136 cm | ROUGE | non mesuré |
| 4 | 97 cm | ROUGE | non mesuré |
| 3 | 78 cm | ROUGE | non mesuré |
| 2 | 58 cm | vert (2 cm de marge) | **0 passe sur 24** (mesuré le 2026-11-19 11:09-11:40, Celeborn, `neutrals_trace` avec la valeur 2) |
| 1 | 58 cm | vert (mêmes chiffres que 2) | non mesuré |
| 0 (avant D815) | 44 cm | vert | 3 tremblements sur 24 passes (environ 1 passe sur 10) |

- A : passer à 2. En jeu : les blocs gardent leurs files (58 cm au pire, limite du contrôle) et le tremblement ne sort pas : 0 passe sur 24, comme avec 8.
- B : revenir à 0. En jeu : files les plus serrées (44 cm), mais deux soldats d'un bloc tremblent parfois quelques secondes (environ une marche sur dix dans le scénario de mesure).
- C : garder 8 et relever le seuil du contrôle à 180 cm. En jeu : rien ne change, les blocs en marche restent relâchés (une file peut se trouver à 1,7 m de sa place), aucun tremblement ; le contrôle ne surveille plus la tenue des files.
- D : garder 8 comme valeur, mais corriger la cause (le détour ne devrait pas faire dériver la file) : chantier C++ dans le processeur de marche, à part, durée inconnue ; le contrôle reste rouge connu en attendant.
- Autre.

Recommandation : **A** (2) : c'est la seule valeur mesurée qui tient les deux à la fois (files à 58 cm, 0 tremblement sur 24 passes). Réserve : la marge du contrôle des files n'est que de 2 cm. Les 16 passes rouges sur 24 de la mesure sont toutes des « spins » de seconde ligne en mêlée (famille connue D641, présente à l'identique avec 0 et avec 8), pas la marche en bloc. Valeur codée aujourd'hui : 8 (rien n'est changé, `known_reds.txt` intact) ; le changement est une ligne (`MarchMemberDetourSteps` dans HOLSimulationProcessors.cpp).

Détail : `Docs\Rapports\DIAG-1119.md`, `Docs\Rapports\DIAG-1119b.md` (cause prouvée sans build), `Docs\Rapports\SWEEP-1119.md` (balayage) ; question d'origine : `Docs\Archive\Retours_2026-11-15\Q-march-member-detour.md`.

Reponse:
