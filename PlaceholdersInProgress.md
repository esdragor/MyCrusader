# Remplacement des placeholders — avancement

Chantier ouvert le 2026-11-08 (copie principale `main`, journaux seuls, aucun code, rien commité). Demande de l'utilisateur : « un chantier pour remplacer les placeholders (logos, arbres, etc.) », avec **deux fichiers** : ce journal (le chantier de remplacement) et **`Placeholders.txt`** (la liste vivante de tout ce qui est provisoire dans le jeu). Chaque étape est notée ici dès qu'elle avance, et `EtapeDefini.md` est mis à jour en même temps.

Distinct de `VisualInProgress.md` (le visuel des bâtiments : méthode, kit Kenney, Forum fait), de `AnimationsInProgress.md` (animations), de `AudioInProgress.md` / `AudioBuildingsInProgress.md` (sons) et de `MapsInProgress.md` / `BiomeInProgress.md` (cartes, sol, biomes). **Ce chantier ne remplace pas leur travail : il tient la liste commune et décide de l'ordre.** Quand un asset est remplacé, le travail se fait dans le chantier qui le possède (ou ici si aucun ne le possède, par exemple les arbres, les logos, les icônes).

## Le fichier `Placeholders.txt`

- Une ligne courte par chose provisoire (« Lancier Aube », « Arbre », « Arbre biome Automne »), rangée par section : unités, bâtiments, ressources, décor des cartes, matériel et effets, interface, menu et marque, sons et musique, animations.
- Étiquette en fin de ligne : `[forme]` (forme de base du moteur), `[mannequin]`, `[kit]` (kit CC0, un artiste repassera), `[rien]`, `[style]`, `[à venir]` (pas encore dans le jeu, provisoire dès sa création), `[?]` (à vérifier).
- **Règle permanente** : tout chantier qui ajoute un provisoire ajoute sa ligne ; tout chantier qui remplace ou supprime un provisoire retire sa ligne et le note ici (section « Remplacés »). Un chantier ne se clôt pas sans relire le fichier. La règle est aussi écrite dans `CLAUDE.md`.
- Relevé initial du 2026-11-08 fait **par lecture des tables (`UnitTypes`, `BuildingTypes`, `SourceTypes`, `Factions`, `FactionOverrides`), du code (`BuildProvisionalShape`, `MakeProvisionalPieces`, `DECOR_KINDS`, `HOLVolcanoLayout`, `HOLRubble`) et des journaux des chantiers** ; **jamais vu à l'écran**. Les lignes `[?]` sont des doutes, à lever avec l'utilisateur ou par un scénario qui montre la chose.

## Ce que le relevé a montré (2026-11-08)

- **Unités** : les 14 types de `UnitTypes.csv` portent le mannequin du moteur (`SM_Mannequin`, corps `SKM_*` à accessoires). Le Cavalier léger, le Bélier et le Mangonneau n'ont pas de corps animé (mannequin fixe). L'Aube et les Légions partagent les mêmes corps (reskin : seuls le nom et le coût changent, `FactionOverrides.csv`).
- **Bâtiments** : 13 types sur 14 sont des blocs du moteur (`BlockAndTower`, `Range`, D261). Seul le Forum a un modèle (kit Kenney Retro Fantasy, branche `visual`, **pas dans `main`**).
- **Ressources** : arbre, baies, 4 tailles de filon de pierre, 4 d'or, champ = formes de base (colonne `Mesh` de `SourceTypes.csv` vide, D297).
- **Décor des cartes** : 15 sortes (herbes, buissons, pierres, souches, rochers, fourrés, fleurs, roseau) en cônes, sphères et cylindres du moteur ; rochers de crête et murs de roche en sphères aplaties ; autel du volcan en cubes et cylindres ; lave et basalte pas dessinés.
- **Interface** : 6 icônes `T_Icon_*` seulement ; style écrit en C++ « jusqu'à ce qu'il y ait une direction artistique » (`HOLUIStyles.h`, D242).
- **Marque** : **aucun logo trouvé** dans `Content/` ni dans les tables ; menu principal = `MainMenu.umap` seul (8 Ko).
- **Sons** : pack Kenney (refaits en cours dans `AudioInProgress.md`), fanfares du héros en échantillons VSCO « provisoires », alerte du Forum en son du moteur ; pas de musique, pas de voix.

## Direction artistique et méthode (reprises de `VisualInProgress.md`)

- **Stylisé et franc** (formes lisibles, couleurs nettes), décidé le 2026-11-03 pour les bâtiments : à confirmer pour les arbres, rochers, ressources et unités (PQ-2). **À numéroter au GDD.**
- **Dix échantillons vus dans le jeu, l'utilisateur dit aimé / pas aimé, dix nouveaux** (mémoire `assets-ten-samples-method`).
- **Licences** : CC0 uniquement ou licence écrite ; chaque asset au registre (`Docs/assets_art.csv`, `Docs/assets_audio.csv`, `Docs/AssetsExternes.md`) avec la preuve de licence ; fichiers sources hors dépôt (`MyCrusader_Art\`). Génération 3D par IA (Tripo) abandonnée le 2026-11-03.
- **Un artiste repassera plus tard sur tout** : tout ce qui est fait ici reste remplaçable (colonne `Mesh` des tables = point d'entrée du modèle final). Les points d'entrée existent pour les bâtiments, les ressources (`SourceTypes.Mesh`) et les unités ; ceux du décor des cartes sont des scripts (`Tools/Editor/maps/common.py`, `DECOR_KINDS`).

## Plan

- **P0 (fait le 2026-11-08)** : demande, relevé, `Placeholders.txt`, ce journal, `EtapeDefini.md`, règle dans `CLAUDE.md`.
- **P1** : lever les `[?]` de `Placeholders.txt` (une session de relevé à l'écran, ou un scénario « tour du propriétaire » qui montre chaque famille) et corriger la liste.
- **P2 — contrôle mécanique de la liste** (leçon « les règles en prose ne tiennent pas », mémoire `lessons-must-be-mechanical`) : un script `Tools/check_placeholders.ps1` qui lit les tables (`UnitTypes`, `BuildingTypes`, `SourceTypes`) et échoue si un type à `Mesh` vide ou à corps de mannequin n'a pas sa ligne dans `Placeholders.txt` (et inversement, si une ligne cite un type disparu) ; appelé par `quick_check` au commit. Test rouge d'abord.
- **P3 — décider de l'ordre** (PQ-1) puis **un lot à la fois**, par la méthode des dix échantillons : probablement ressources et décor d'abord (visibles partout, peu nombreux : arbre, baies, filons), puis bâtiments, puis unités, puis interface et marque.
- **P4 — lots** (à numéroter quand l'ordre est tranché) : Arbres (+ un par biome), Rochers et décor, Filons et baies, Bâtiments (suite de `VisualInProgress.md`), Unités, Icônes et logos, Sons (suite des chantiers audio).

## Questions ouvertes (à trancher une à la fois, sans numéro D attribué ici)

- **PQ-1** — Par quoi commencer : A = arbres, baies et filons (les ressources ; recommandation, les plus visibles et les moins nombreux), B = les bâtiments (suite du Forum), C = les unités, D = interface et logo.
- **PQ-2** — Même direction « stylisé et franc » pour les arbres, rochers et ressources ? Et pour les unités, garder le mannequin avec des accessoires de couleur d'ici le prototype, ou viser des corps propres ?
- **PQ-3** — Le logo du jeu : y a-t-il un nom définitif et une idée de logo ? (aucun asset trouvé ; le projet s'appelle « HeroesOfLiberty » mais le titre de jeu n'est pas tranché dans mes lectures)
- **PQ-4** — Valider P2 (le script de contrôle) ou garder la règle en prose seule.
- **PQ-5** — `visual` (Sauron) n'est pas fusionnée dans `main` (96 commits de retard au dernier relevé) : la fusionner avant d'ouvrir d'autres lots de bâtiments ?

## Remplacés (à compléter au fil des chantiers)

- Rien encore. *(Le Forum a un modèle de kit sur la branche `visual`, mais reste `[kit]` : provisoire jusqu'à l'artiste.)*

## Journal

- 2026-11-09 (Sauron, agent sans C++, P1 faite par lecture, PAS vue à l'écran) : `[?]` levés dans `Placeholders.txt` par lecture du code et des scripts : chantier de construction = bloc qui monte (`[forme]`) ; cadavre = mannequin couché (`[mannequin]`) ; relais = anneau (`HOLSelectionMarkers`, `[style]`) ; flèche, carreau, bannière = meshes faits par script (`make_arrow_mesh.py`, `make_banner_mesh.py`, `[style]`) ; Mangonneau sans mesh de projectile (`[rien]`) ; soin = ligne de debug + « + » (`[style]`) ; aura et cri = anneaux ; icônes ressources, technologies, portraits, boutons de la grille = `[rien]` (seules les 6 `T_Icon_*` existent) ; curseur = système + croix ; rivière/gué = pièces plates (`[forme]`) ; campement des brigands = aucun décor (`[rien]`). **Restent `[?]`** (non tranchés par lecture) : arbre et rochers par biome (4 + 1 lignes), icônes des talents, trajets et marques d'ordres au sol. Questions posées : `Retours\Q-placeholders-ordre.md` (PQ-1, PQ-5), `Retours\Q-placeholders-direction.md` (PQ-2, PQ-3, PQ-4), valeurs par défaut dans les fichiers. D749 (Forum tour 2) est déjà dans `VisualInProgress.md` (dernière ligne) ; l'élection finale revient à l'utilisateur ; `biome_decor_cost` non lancé (machine non calme).
- 2026-11-08 : chantier ouvert à la demande de l'utilisateur (deux fichiers : ce journal et `Placeholders.txt`). Relevé fait par lecture, pas vu à l'écran. `EtapeDefini.md` (Fichiers de développement) et `CLAUDE.md` (règle de mise à jour de la liste) mis à jour. Aucun code, aucun test, rien commité.
