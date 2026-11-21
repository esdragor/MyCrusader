# [À RÉPONDRE] Engin démonté qui marche : un Stop le monte-t-il sur place ?

Impact joueur : quand tu arrêtes (Stop) un engin de siège (Mangonneau...) en pleine marche, il peut se retrouver monté au milieu de nulle part, ou rester démonté, selon ta réponse.

## Contexte
D877 : un engin démonté se monte seul en fin de déplacement (ou à portée de sa cible). Aujourd'hui, un Stop donné pendant sa marche le monte aussi sur place (`HOLUnitOrderProcessor.cpp:1542`, `MoveOrder->Phase = None` ; la note de marche `bMarching` reste à 1, le processeur de déploiement voit « fin de marche »).

## Options
- **A. Stop = fin de déplacement : l'engin se monte là où il s'arrête.**
  - En jeu : tu fais Stop, l'engin joue le montage (quelques secondes sans tirer), puis il est prêt à tirer sur ce qui entre à portée. Cohérent avec « un engin ne reste jamais démonté à l'arrêt ». Piège : un Stop de réflexe sous le feu ou en fuite le fige monté et vulnérable.
- **B. Stop = il reste démonté.**
  - En jeu : Stop annule la marche, l'engin reste en pose de marche, bouton « Monter » (D) disponible, il ne tire pas tant que tu ne le montes pas ou ne lui donnes pas un ordre. Garde l'engin mobile après un Stop (repli rapide), mais un engin laissé démonté sans ordre ne défend rien, et il faut une action de plus pour le monter. Le montage auto ne vaut plus que pour une fin de déplacement normale.
- **C. Stop = monte seulement si un ennemi est à portée de tir, sinon reste démonté.**
  - En jeu : le Stop pendant une approche d'un bâtiment ennemi monte l'engin (il tire), un Stop en pleine campagne le laisse démonté. Plus intelligent, mais deux comportements à expliquer au joueur et une règle de plus dans le contrôleur de déploiement.
- **Autre** : à préciser.

## Recommandation
**B** : Stop signifie « annule ce que je viens de demander » ; monter est une action coûteuse (délai sans tir) que le joueur doit choisir (bouton D) ou obtenir par un vrai déplacement / une attaque. Le Stop reste un moyen sûr de figer un engin sans le clouer au sol. Le montage auto de D877 reste garanti pour une fin de déplacement et pour l'attaque.

## Valeur actuelle du code
**A, de fait** (non voulue explicitement : effet de bord de `bMarching` non remis à 0 à l'arrêt). Pas encore de réglage. Si B ou C est choisi : une ligne dans `UHOLUnitOrderProcessor::ApplyQueuedOrders` (~1542) remet `FHOLDeployFragment::bMarching = 0` (même geste que le correctif ef6cbf2 d'`ApplyCombatOrder`) ; C ajoute un test de portée. Un cvar `hol.Deploy.StopMounts` (0/1) peut porter A/B sans recompiler les tests.

Chantier : Palier3InProgress.md (Elrond, integ-25), rapport `Docs\Rapports\ELROND-1121se.md`.
