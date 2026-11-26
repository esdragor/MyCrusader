# [À ESSAYER] Menus d'éditeur des artistes (AR2-a + AR2-b) — LOGIQUE SEULEMENT

STATUT: LOGIQUE SEULEMENT (les menus du Content Browser ne se voient qu'à l'écran de l'utilisateur ; règles pures vertes en tests script)
GESTE: réel (clic droit dans le Content Browser de l'éditeur ouvert par l'utilisateur ; l'agent ne l'a pas joué)
SON: non entendu (aucun son attendu)
branches: art-editor2-1125ar

Agent : SAURON-1125ar6 (2026-11-25). Code : `Script\Art\HOLArtChecks.as` (règles), `Script\Art\Editor\HOLArtAssetMenu.as` (menus).

## Avant

Ouvrir l'éditeur sur un build qui contient la branche `art-editor2-1125ar` (après fusion par la session principale) :

```
powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\build.ps1 -LiveCoding=0 -Wait
```

puis ouvrir `C:\Users\baudo\Documents\MyCrusader\HeroesOfLiberty.uproject`, fenêtre Output Log ouverte (Window > Output Log).

## Gestes

1. Content Browser, `/Game/Art/Units/Arquebusier/` : clic droit sur `DA_Art_Arquebusier` > section HOL Art > **Check for the game**.
   Attendu : Output Log `HOL Art check: DA_Art_Arquebusier`, une ligne `[OK]` / `[KO]` par règle, dont les NOUVELLES :
   `Projectile: ...` et `MuzzleSmoke: ...` (vide = OK pour une unité de mêlée ; rempli = charge + StaticMesh + pivot dans ses bornes),
   et le résumé en haut à gauche de l'écran (vert « ready for the game » ou rouge « N red line(s) »).
2. Clic droit sur un SkeletalMesh `SKM_*` sous `/Game/Art/Units/<Type>/` > **Check for the game**.
   Attendu : NOUVELLE ligne `triangles (LOD 0): N (budget 8000)` (budget provisoire des unités).
3. Clic droit sur une AnimSequence sous `/Game/Art/Units/<Type>/` > **Check for the game** (menu NOUVEAU).
   Attendu : `skeleton ...` (même squelette que le SkinnedMesh de `DA_Art_<Type>`), `length ... s (0 to 10)`, `frames: N (at least 2)`.
   Une AnimSequence hors de `/Game/Art/<Famille>/<Type>/` : ligne rouge « no type from the folder ».
4. Clic droit sur `DA_Art_Arquebusier` > **See in the Workshop** (menu NOUVEAU).
   Lancer une PIE depuis le script n'est pas lié : le menu écrit dans l'Output Log et à l'écran la commande à taper.
   Lancer la partie Plain (bouton Play de l'éditeur sur la carte de partie Plain), ouvrir la console (touche `²` / backquote) et taper :

```
hol.Art.Try Arquebusier 1
```

   Attendu : un Arquebusier du slot 0 près du Forum (ligne `hol.Art.Try: 1 Arquebusier of slot 0 ...`).

## Ce qui n'est pas vérifié

- Menus à l'écran, sonde Python (triangles du SkeletalMesh, squelette et nombre d'images d'une AnimSequence) : compilés, jamais exécutés par l'agent.
- Budgets et bornes provisoires (`Placeholders.txt`) : 8000 triangles par unité, 40000 par bâtiment, 10 s et 2 images par clip.
