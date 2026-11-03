# Chantier biomes (ambiance réglable au salon)

Ouvert le 2026-11-03 (session principale), après la recherche `reports\Outils pour améliorer HeroesOfLiberty.md` et le travail
sol CC0 + ambiance lumineuse de Haldir (branche `maps`, non commité, voir `MapsInProgress.md`, entrées « Sol CC0 étape 1/2 » et
« Ambiance »).

## Demande de l'utilisateur (2026-11-03)

« Avoir un système de biome réglable dans le salon pour changer l'ambiance. »

## Décisions prises (inscrites au GDD le 2026-11-03 : D651 à D656, § 8 Carte « Biomes » et fin du GDD ; prochain numéro D657)

- **B-Q1 = l'hôte choisit, pour tous** *(proposé D652)* : le biome est un réglage du salon, comme la carte et la taille ; tous les
  joueurs voient le même. Pas d'option vidéo locale.
- **B-Q2 = le biome change aussi le décor posé (arbres, touffes)** *(proposé D653)* : en plus du sol (couleur et texture), du
  soleil, du ciel et de la brume. Choix le plus large des trois proposés.

Contraintes qui restent : D258 (aucune ombre), D265 (mat, aucun reflet, aucun post-traitement). Le décor posé reste **sans
collision** et **hors navmesh** (preuve déjà faite pour la Steppe, `hol.Nav.Path`). Les arbres de **ressource** (bois), l'or, la
pierre, les baies et les rochers de jeu (crêtes, murs, affleurements) ne changent PAS avec le biome : ce sont des choses de jeu.

## Où on en est (état du code, 2026-11-03)

- Le sol est `M_GroundVaried` + une instance par carte (`maps/ground.py`, `MI_Ground_Steppe`, `MI_Ground_Hinterland`), texture CC0
  Poly Haven (registre `Docs/AssetsExternes.md`). Cols, Couronne, El Dorado, Gués et Forêt réutilisent l'instance de la Steppe.
- L'ambiance (soleil, ciel, brume) est dans `Tools/Editor/maps/ambience.py` (table par carte, bornes, test `ambience_test.py`) et
  **cuite dans le fichier de carte** à la régénération. Vue sur Arrière-pays et Steppe : distinctes ; Forêt distincte ; El Dorado,
  Gués, Couronne et Cols trop proches (même sol).
- Le décor est posé **à l'édition** par la recette Python (`common.decor_transforms`, `spawn_decor` : un acteur « Decor », un
  composant d'instances par genre, graine fixe, coupé à 12 000 cm de la caméra), donc cuit dans chaque `.umap`.

## Piste d'architecture (à valider avant de coder)

Un biome est une **ligne de table** : paramètres du sol (couleurs, texture, échelle), ambiance (soleil, ciel, brume), genres et
densités de décor, teintes de décor. Avec le décor dans le biome, le cuire dans les fichiers donnerait cartes × biomes × tailles
fichiers : pas viable. Donc :

1. **Appliquer le biome au chargement de la carte**, chez le serveur et chez chaque client, d'après l'identifiant de biome du salon
   (sol : paramètres de l'instance de matériau ; lumières : propriétés des acteurs ; décor : généré au chargement).
2. **Porter `decor_transforms` du Python vers le runtime** (Angelscript d'abord, D483 : le neuf en AS, sauf Mass/simulation/réseau/
   rendu ; C++ si le rendu l'exige), **même algorithme, même graine** : deux clients tirent exactement le même décor. Test de
   déterminisme + comparaison avec le Python.
3. **Réglage du salon** : un champ « biome » ajouté aux règles du salon (`HOLMapRules`, `LobbyRules`, gestionnaire C# `Manager`,
   `Maps.csv` pour le biome par défaut de chaque carte), transmis au serveur et aux clients comme la taille de carte.
4. Les cartes déjà régénérées gardent leur ambiance cuite comme biome par défaut, en attendant le retrait de l'ambiance cuite.

## Étapes proposées (boucles de 15 min)

- **B0** : inscrire D652 et D653 au GDD (§ cartes / salon), lister les biomes (nombre, noms, un par carte au départ ?), question à
  l'utilisateur.
- **B1** : table de biomes + application visuelle chez le client **sans décor** (sol + lumières), un biome de test, vues avant/après.
- **B2** : réglage du salon et réseau (gestionnaire, règles, serveur, client), test `-Parts net`.
- **B3** : décor au chargement (port de `decor_transforms`), test de déterminisme, preuve navmesh inchangée, coût S1/S2 mesuré.
- **B4** : interface du salon (liste des biomes), scénario d'essai `biome_compare` (tous les biomes à l'écran, Entrée = suivant).
- **B5** : retrait de l'ambiance cuite, fusion avec `maps`, mesures de fin de chapitre.

## Réponses du 2026-11-03 (B0)

- **B-Q3 = cinq biomes pour commencer** : herbe fraîche, steppe sèche, automne, forêt sombre, neige.
- **B-Q4 = n'importe quel biome sur n'importe quelle carte** (la Forêt en neige, Les Cols en herbe fraîche) ; pas de liste permise par carte.
- **B-Q5 = la mini-carte ne reprend pas la couleur du sol du biome.**
- **B-Q6 = la carte garde ses formes, le biome les recolore** : le décor d'identité de la carte (D576 : rochers de Cols, roseaux
  d'El Dorado...) reste aux mêmes endroits avec les mêmes formes, le biome change sa couleur (Cols en neige : rochers blancs
  givrés) et **remplace le décor générique** (fleurs, touffes, buissons) par le sien (neige : touffes gelées, buissons nus).
  Conséquence : le décor d'identité d'une carte doit rester séparé du décor générique dans les recettes (deux listes).
- Purement visuel : aucun effet sur l'IA coéquipier, les neutres, la navigation ni les ressources.

## Questions ouvertes

1. Un biome par défaut pour chaque carte (celui qu'elle a aujourd'hui) : à confirmer ; l'hôte garde-t-il « défaut de la carte »
   comme premier choix de la liste du salon ?
2. Valeurs des cinq biomes (couleurs du sol, soleil, ciel, brume, liste du décor générique) : proposées en B1 sur captures, à juger
   par l'utilisateur ; la neige demandera sans doute une texture de sol propre (une texture CC0 de neige à choisir).
3. Décor générique : mêmes formes de base (cônes, sphères, cylindres du moteur) pour tous les biomes, ou formes propres (flocons,
   feuilles) ? Défaut proposé : mêmes formes, couleurs et densités propres au biome.

## NON VÉRIFIÉ

- Aucun code écrit pour les biomes. Coût de rendu d'un décor généré au chargement, déterminisme Python / runtime, et effet du
  biome sur le brouillard de guerre (`M_FogPost`) à mesurer.

## Journal

- 2026-11-03 : chantier ouvert, B-Q1 et B-Q2 répondues par l'utilisateur ; rien codé.
- 2026-11-03 : B0 en partie fait : B-Q3 à B-Q6 répondues (cinq biomes, n'importe quelle carte, mini-carte inchangée, formes de la
  carte gardées et recolorées).
- 2026-11-03 : **B0 faite** : B-Q1 à B-Q6 inscrites au GDD (D651 à D656, paragraphe « Biomes » du § 8 Carte). Le développement
  attend (demande de l'utilisateur : « on fera le dev plus tard »). Reprise = B1 (table de biomes + sol et lumière au chargement,
  un biome de test, vues avant/après), en repartant du travail non commité de Haldir (sol CC0, `ambience.py`).
