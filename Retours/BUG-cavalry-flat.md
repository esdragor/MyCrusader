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

## Diagnostic (agent Elrond sans C++, 2026-11-06 11:1x, tiers = main 0f6ecee + d611d94) : CAUSE DE SCENARIO (cadrage), pas un defaut d'orientation du jeu
- Lu dans le code : aucun pitch/roll nulle part pour un cavalier. UnitTypes.csv donne a LightCavalry /Game/Units/Mannequin/SM_Mannequin (MAILLAGE STATIQUE, copie de SK_Mannequin en pose de reference, echelle 2,25, MeshYawOffset -90) comme le Belier et le Mangonneau ; HOLInterpolatedInstancesProcessor.cpp n'applique qu'un lacet (FRotator(0, yaw, 0)) et HOLUnitSubsystem.cpp l. 488 un decalage de lacet seul. PV 120 = la table (CSV, UnitStats.md) : la note 4 (« 80 PV ») etait perimee, corrigee.
- Mesure A/B sur la vraie chaine (meme hol.Units.SpawnHere 1 LightCavalry 0 100, meme build, copie jetable zz_cav) : AVANT, camera FocusNear 6400 -4800 : le cavalier, hors repos meme (capture au repos, apres 3 images et apres 63 images de marche), est au coin bas-droit de l'ecran, figure AU SOL horizontale (zzcav_idle / _o3 / _o23 / _o63, tous pareils) ; APRES, camera sur l'axe du couloir (FocusNear 5600 -3000, 5600 0, 5600 3000) : cavalry_vs_archers_start et cavalry_vs_spearmen_start montrent un cavalier DEBOUT (pose en T) au centre de l'ecran, PV 120/120.
- Conclusion : le cavalier du « start » etait vu au bord de l'ecran, en perspective, sous l'angle de la camera du jeu ; le scenario le cadrait mal. Corrige dans Tools/Scenarios/cavalry.txt (commit d611d94, tiers). Preflight PASS (106 s, sanity OK, HUD OK), les 8 PNG ouverts et vus.
- NON VERIFIE : cavalry_vs_mangonel_start montre encore un cavalier un peu penche au centre (pose en T vue d'en haut, lacet different) : je n'ai pas pu separer « lacet de la pose en T » de « inclinaison » sans la valeur de rotation ; aucune ligne C++ a ouvrir tant que la camera au bord reste la seule cause : si l'utilisateur voit encore un cavalier couche au CENTRE de l'ecran, c'est alors un point [ ] C++ (lacet vs pose de SM_Mannequin) ; il rejoue : play.ps1 cavalry. Gris (non bleu) et sans animation : placeholder assume (CavalryInProgress.md), non change.
- Statut : rejeu par un AUTRE agent sur le build de main fusionne avant toute commande a l'utilisateur (regle 19) : pas encore fait.

## Recette indépendante 2026-11-06 12:12 (intégrateur, main 2bb0d6b) : RÉSOLU côté scénario
Rejoué sur le build intégré : le cavalier est debout au centre (pose en T, anneau aux pieds) dans les trois « start » ; cavalry_vs_mangonel_start n'est plus incliné (recadrage x4 vu). PNG vus : les 8 de cavalry. Voir E-cavalry.md.
