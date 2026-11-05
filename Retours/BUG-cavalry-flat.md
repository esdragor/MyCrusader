# BUG-cavalry-flat : le Cavalier léger apparaît couché / incliné au repos (à plat sur les PNG)

Découvert par : agent d'intégration n°2, 2026-11-05 21:18, main eb7b5fe (binaire 21:07), scénario `cavalry`.

SCÉNARIO : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 cavalry -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
GESTE EXACT (dans cavalry.txt) : `hol.Units.SpawnHere 1 LightCavalry 0 100` (cavalier de ton camp, à 100 cm), puis capture `cavalry_vs_archers_start` / `cavalry_vs_spearmen_start` / `cavalry_vs_mangonel_start` avant `hol.Select.AttackMoveNear`.

VU (PNG dans C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- cavalry_vs_spearmen_start.png : cavalier presque HORIZONTAL (couché), PV 120/120, sélectionné, vivant (barre de vie verte).
- cavalry_vs_archers_start.png et cavalry_vs_mangonel_start.png : corps incliné à environ 45°.
- cavalry_produced.png et cavalry_vs_archers.png / cavalry_vs_mangonel.png (en action) : debout.
Hypothèse non vérifiée : l'orientation (pitch/roll) du Mannequin agrandi est mal initialisée à l'apparition par SpawnHere (ou le cavalier est penché par la pente/la vitesse). Pas de cause cherchée dans le code (classe sans C++).

ATTENDU : un cavalier vivant au repos est debout comme dans cavalry_produced.png.
AUTRE : la note 4 du scénario dit « 80 PV : 9 coups » alors que le panneau affiche PV 120/120.
NON VÉRIFIÉ : si le défaut apparaît pour un cavalier produit par la grille réelle après un déplacement ; s'il est l'effet d'un « Sanity » (l'audit est passé) ; la capture n'a qu'une image, pas de séquence.
