# [À RÉPONDRE] Prise de la lance du Lancier (bras gauche dans le corps)

branches: hero-j8-close (aucun correctif de pose commité : rien à jouer)

**Contexte.** Le bras gauche du Lancier (main ARRIÈRE sur la hampe) traverse le torse pendant l'attaque (`A_Spearman_Attack`, tous les Lanciers, neutres et joueur) : la lance est tenue sur la hanche DROITE, main gauche derrière, main droite devant, donc le bras gauche doit croiser tout le ventre. Mesure (test rouge 966ae02, `Tools\Editor\arm_body_check.py`) : coude gauche 17,7 cm dans le tronc à l'image 29.

**Ce qui a été essayé hors jeu (2026-11-12, CELEBORN-1112a et b, ~35 variantes) :** directions du coude (17 variantes, 14-19 cm), lance avancée ou glissée (14-17 cm), épaule gauche avancée (clavicule ±20°) et torsion du tronc réduite jusqu'à zéro (13-15 cm), main gauche replacée devant la poitrine clé par clé (2-5 cm, mais la lance se retourne et devient verticale au recul : refusé à l'œil), lance entière déplacée (5-7 cm, mais la main droite lâche la hampe de 20 cm). **Aucune pose crédible avec la prise actuelle** : c'est la prise elle-même qui fait croiser le bras.

Images (hors jeu, vues de face / de côté / trois quarts) :
- actuelle : `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Saved\c1112\grip_0_0.png`
- main gauche forcée devant (refusée, lance qui se retourne) : `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Saved\c1112\solved_28_-6.png`

Scénario `spear_grip_compare` (balayage en jeu) : NON FAIT (il faut une animation cuite par prise et un binaire frais ; à faire après la réponse si B ou C).

**Options**
- **A. Garder la prise actuelle** (main gauche derrière, lance à droite) et accepter le défaut, ou le cacher (avant-bras plus court à l'écran, rien de propre). Conséquence : le bras reste dans le corps au coup.
- **B. Mains échangées** : main DROITE derrière, à la hanche droite ; main GAUCHE devant sur la hampe. C'est la prise habituelle d'une estocade à deux mains : chaque bras reste de son côté. Conséquence : toute l'attaque (et la première image de la chute, `spear_fall`) est à reposer ; l'essai sera revu à l'œil.
- **C. Lance au centre**, devant le ventre (pique), mains l'une derrière l'autre. Conséquence : silhouette différente des autres soldats, estocade plus « pique » que « lance ».
- **Autre** : à préciser.

**Recommandation : B** (la seule prise où aucun bras ne croise le corps ; c'est celle des lanciers à deux mains).

**Valeur par défaut codée : A** (rien n'a changé dans `Tools\Editor\soldier_poses.py`, `SPEAR_GUARD`..`SPEAR_SETTLE` ; la réponse B ou C rouvre le chantier dans `NeutralsInProgress.md`).
