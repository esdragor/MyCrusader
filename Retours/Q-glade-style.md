# [À RÉPONDRE] Q-glade-style : quel style de clairière pour chaque carte ?

Contexte : tu as répondu OUI à une clairière différente selon la carte (E-glade-sweep, 2026-11-09). Huit styles existent (A à H, scénario `glade_sweep` : Entrée = style suivant, le nom du style ET de la carte recommandée sont écrits à l'écran). Le choix par carte est maintenant une ligne par carte dans `Tools\Editor\maps\common.py` (`GLADE_STYLE_BY_MAP`) ; seules les forêts tirées par le générateur sont touchées, jamais les forêts peintes à la main. Rien n'est régénéré tant que la tâche de régénération (à part) n'a pas lieu.

Voir le choix : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 glade_sweep -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` (la carte d'essai `Crown.umap` est déjà faite).

| Carte | Style recommandé (défaut codé) | Pourquoi |
|---|---|---|
| Forêt (Forest) | C : 1 grande clairière x2,5 au centre | forêt profonde, anneau d'arbres ; garde la clairière à l'intérieur (la Forêt est déjà à la limite du budget d'arbres) |
| Plaine (Plain) | D : 2 clairières x1,5 | forêts de plaine ouvertes |
| Steppe | G : 5 petites clairières x1 | ton exemple ; bosquets de 30 à 40 arbres percés de petits trous |
| Cols | F : 3 clairières x1,5 gardées à l'intérieur | carte déjà à la limite des arbres : contour intact |
| Gués (Fords) | B : 1 clairière x1,5 | une seule, discrète, près d'un bord |
| El Dorado | E : 3 clairières x1,5 | forêts claires |
| Couronne (Crown) | H : 2 grandes clairières x2,5 gardées à l'intérieur | grands vides sans ouvrir le contour |
| Arrière-pays (Hinterland) | A : inchangé (aujourd'hui) | poches de 210 arbres déjà très peintes ; le moins de risque |

Options : A) le tableau ci-dessus ; B) tu donnes une lettre par carte (ex. « Forêt B, Plaine E, ... ») ; C) un seul style pour toutes les cartes ; Autre) ta proposition (plus / moins de clairières, plus grandes / plus petites : nouveau balayage).

Conséquences visibles : seules les cartes régénérées changent, dans la tâche à part (UNE régénération des 7 cartes) ; chaque lettre = un look de forêt différent d'une carte à l'autre. Le nombre d'arbres par carte ne change pas, mais une clairière agrandit le bosquet : les cartes à la limite (Forêt, Cols, Arrière-pays) peuvent refuser (RoomError) ; c'est vérifié à la régénération.
Non vérifié : le style G (5 clairières) n'a été VU que sur une forêt de 300 arbres ; sur les bosquets de 35 arbres de la Steppe il n'est pas vu (en Python, l'étendue d'un bosquet de 14 ou 35 arbres ne change pas d'un style à l'autre, mais le rendu n'est pas regardé).

Recommandation : A (le tableau). Si tu hésites sur la Steppe, garde A pour elle (aucun risque).
Défaut codé : le tableau (une ligne par carte à changer : `GLADE_STYLE_BY_MAP` dans `Tools\Editor\maps\common.py`, test `Tools\Tests\glade_style_test.py`).

Réponse (utilisateur) : « vas y » (2026-11-09 16:36) = le tableau recommandé : Forêt C, Plaine D, Steppe G, Cols F, Gués B, El Dorado E, Couronne H, Arrière-pays A. Déjà codé par défaut dans GLADE_STYLE_BY_MAP (Haldir, a1e735b). Reste : la régénération des 7 cartes (tâche à part) + `bSitesLaidByMaps = true`.
