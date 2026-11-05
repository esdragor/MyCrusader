# Assets externes

Registre de tout ce qui vient de l'extérieur du projet (textures, modèles, sons, polices). Une ligne par asset. Licence vérifiée sur la page officielle de l'asset.
Convention : les fichiers source sont dans `Tools/Data/<Famille>Sources/`, ce qu'Unreal importe est fabriqué par un script (jamais à la main).

## Textures de sol (Poly Haven, chantier Cartes, sol CC0 étape 1, 2026-11-03)

Source : Poly Haven (https://polyhaven.com), licence CC0 1.0 (domaine public, sans attribution) lue sur la page de chaque asset le 2026-11-03. Fichiers JPG 1k (1024 x 1024). Normales au format OpenGL (`nor_gl`) téléchargées mais pas utilisées (le sol n'a que l'albedo : D258 et le budget de 144 i/s).

| Nom | URL | Licence | Résolution | Fichiers | Où utilisé |
|---|---|---|---|---|---|
| Grass Ground (Charlotte Baglioni, 2,51 m) | https://polyhaven.com/a/grass_ground | CC0, vérifiée le 2026-11-03 | 1k | `Tools/Data/GroundSources/grass_ground_diff_1k.jpg`, `grass_ground_nor_gl_1k.jpg` (non utilisée), `grass_ground_detail.png` (fabriqué) ; Unreal : `/Game/Rendering/Textures/Ground/T_Ground_GrassGround_Detail` | `MI_Ground_Steppe` (la Steppe) |
| Sparse Grass (Amal Kumar, 2,0 m) | https://polyhaven.com/a/sparse_grass | CC0, vérifiée le 2026-11-03 | 1k | `Tools/Data/GroundSources/sparse_grass_diff_1k.jpg`, `sparse_grass_nor_gl_1k.jpg` (non utilisée), `sparse_grass_detail.png` ; Unreal : `T_Ground_SparseGrass_Detail` | alternative gardée pour la Steppe (herbe plus sombre, brins visibles), non branchée |
| Forest Ground 06 (Charlotte Baglioni, 2,11 m) | https://polyhaven.com/a/forest_ground_06 | CC0, vérifiée le 2026-11-03 | 1k | `Tools/Data/GroundSources/forest_ground_06_diff_1k.jpg`, `forest_ground_06_nor_gl_1k.jpg` (non utilisée), `forest_ground_06_detail.png` ; Unreal : `T_Ground_ForestGround06_Detail` | `MI_Ground_Hinterland` (l'Arrière-pays) |

Traitement (`Tools/prepare_ground_details.py`) : l'albedo est passé en lumière linéaire, divisé par sa moyenne (la couleur moyenne de la carte reste celle de `ColorA`), la variation de teinte est gardée pour moitié, le tout divisé par 2 pour tenir en 8 bits (le matériau remultiplie par 2). Importé en texture linéaire (sans sRGB), mipmaps, filtrage par défaut du projet. Branché par `Tools/Editor/make_ground_materials.py` et `Tools/Editor/maps/ground.py` (`DETAILS`).
