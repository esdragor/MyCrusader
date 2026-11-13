# [À RÉPONDRE] Un engin de siège MONTÉ et au repos tire-t-il seul sur un bâtiment ennemi à portée ?

**Contexte.** Palier 3, G6 (engin déployable, branche `tiers`, Elrond, 2026-11-13). En partie réelle, la Mangonnelle montée (substitut du Trébuchet) reste muette à 22 m d'une Maison ennemie : sa colonne `AggressionRadius` vaut 0 dans `UnitTypes.csv` (« seulement sur ordre »), donc au repos elle ne cherche aucune cible, ni unité ni bâtiment. Il faut un clic droit sur la Maison pour qu'elle tire.

**Options.**
- **A. Seulement sur ordre (actuel).** Le joueur monte l'engin puis fait un clic droit sur le bâtiment. Un engin oublié monté près d'une base ennemie ne fait rien ; aucune surprise, mais micro obligatoire à chaque cible.
- **B. Monté, il prend seul un bâtiment ennemi à sa portée de tir sur bâtiment (3000 cm).** Comme le Trébuchet d'AoE4 une fois installé : il enchaîne les bâtiments à portée sans ordre, le plus proche d'abord ; démonté, il ne cherche rien (pas de montage automatique). Il ne prend toujours pas seul une unité.
- **C. B + les unités ennemies à portée** (au-delà de la portée minimale), unité d'abord comme les autres unités. Plus autonome, mais l'engin gaspille ses coups lents sur des unités mobiles.
- **Autre.**

**Recommandation : B** (le geste AoE4 « installer le trébuchet puis le laisser raser ») ; C seulement si les essais montrent des engins inutiles face aux sorties ennemies.

**Codé.** Réglage `hol.Siege.DeployedAutoFire` : `0` = A (**défaut codé**, comportement inchangé), `1` = B (`Source\HeroesOfLiberty\Simulation\HOLDeployProcessor.cpp`, une ligne à changer pour le défaut). C n'est pas codé. Test : `HeroesOfLiberty.Siege.Deploy.DeployedAutoFire` (0 : la Maison intacte ; 1 : touchée). Le scénario `deploy` donne un vrai clic droit sur la Maison (`hol.Select.OrderNear 5300 0`) en attendant.

Reponse:
