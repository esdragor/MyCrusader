# [À RÉPONDRE] Q-spear-grip-b-look : la prise B du Lancier (D787) vue hors jeu, et le coup en travers de l'Homme d'armes

Agent CELEBORN-1113e, 2026-11-13 16:4x, worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`, branche `hero-j8-close`.

**PLANCHE À REGARDER** (une seule image, à ouvrir en grand) :
`C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Saved\c1113e\planche_pose_B.png`
(version réduite : `...\Saved\c1113e\planche_pose_B_half.png` ; script qui la refait : `...\Saved\c1113e\planche_pose_b.py`)

Ce qu'elle montre, de haut en bas : les 4 captures en jeu de `spear_arm` agrandies x2,2 ; puis 4 variantes de
l'attaque du Lancier (A ancienne, B cuite, B2, B3), chacune en 4 rangées (face, profil, vue RTS du rapport, et
**la vue de la caméra du jeu** : de derrière, plongée 55°) aux images 0, 22, 24, 27 (garde, chargée, passage, juste
avant la frappe), avec la mesure `arm_body_check.py` (bras dans le tronc, limite 1 cm).

## Question 1 : la prise B telle qu'elle est cuite vous convient-elle ?

Contexte : D787 (prise B, mains échangées) est cuite (commit e0cb438) et `arm_body_check` est vert (gauche -2,3 cm,
droite 0,0 cm). En jeu, la caméra ne descend pas sous 15 m : un Lancier fait ~70 px et le Lancier bleu semble avoir
« le bras gauche levé au-dessus de la tête ».

**Ce qui est prouvé (planche, 4e rangée de B, et mesures) :** la main gauche n'est PAS levée. Sa hauteur réelle est
121 à 132 cm (tête 154-158 cm, épaule 139-143 cm, coude gauche 115-130 cm). Elle tient la hampe ~45 cm DEVANT la
poitrine ; vue de derrière et d'en haut (55°), une main tenue devant le corps se projette à hauteur de la tête à
l'écran (+3, +1, -4, +13 cm par rapport à la tête aux images 0, 22, 24, 27 ; avec l'ancienne prise A, -17 à -35 cm).
La 4e rangée reproduit la silhouette des captures en jeu : c'est un effet de la caméra, pas une pose fausse.

- **A. Garder B telle quelle (défaut codé, déjà cuit).** En jeu : de dos, la main gauche apparaît au niveau de la tête
  et la lance en diagonale ; de face ou de côté, garde basse classique, lance à la taille. Mesure verte.
- **B. Baisser et rapprocher la main gauche** (lance plus basse devant le ventre). En jeu : silhouette plus « basse »
  vue de dos. Coût : les balayages de 1113b montrent que l'avant-bras gauche rentre alors dans le ventre (8 à 15 cm,
  rouge) ; il faudrait d'autres compromis (lance plus courte en main, tronc moins tourné), nouveau rebake.
- **C. Revenir à la prise A** (ancienne). En jeu : main gauche basse vue de dos, mais l'avant-bras gauche traverse la
  poitrine (17,7 cm, le bug d'origine). Rangée A de la planche.
- **Autre** : à préciser.

Variantes B2/B3 de la planche (coude droit ou gauche rabattu vers le bas) : elles ne changent PAS la place de la main
gauche et rendent la mesure rouge (B2 droite 5,3 cm, B3 gauche 1,3 cm) : montrées pour mémoire, non recommandées.

**Recommandation : A** (garder B cuite) : rien de faux mesuré, et ce qui gêne en jeu est l'angle de la caméra.
Défaut codé : A (aucun changement ; la pose est dans `Tools\Editor\soldier_poses.py`, SPEAR_GUARD..SPEAR_SETTLE).

## Question 2 : l'Homme d'armes, coup en travers (signalé par l'outil, pas par vous)

Contexte : `arm_body_check.py maa_attack` est ROUGE depuis le 2026-11-12 : à l'image 20 (0,67 s, fin du coup en
travers, épée qui passe devant le bouclier) les deux avant-bras entrent de 4,3 cm dans le tronc. Vu à l'œil hors jeu
(`...\Saved\c1113e\maa_attack_f20.png`, 3e colonne) : l'avant-bras droit balaie la poitrine en travers et le bras du
bouclier est replié devant elle ; cela dure une ou deux images d'un coup de 1,4 s et n'a jamais été vu en jeu.

- **A. Tolérer** : relever la limite de `arm_body_check` pour ce geste seul (5 cm) ; en jeu rien ne change. Le test redevient vert.
- **B. Corriger la pose** : écarter l'épée et le bouclier du torse à l'image 20 (rebake). En jeu : coup un peu plus
  « ouvert », invisible à 15 m.
- **C. Laisser rouge** et ne rien faire pour l'instant.
- **Autre** : à préciser.

**Recommandation : A** (une ou deux images d'un coup rapide, invisible à la distance du jeu ; aucun retour de votre part sur ce coup).
Défaut codé : C (rien changé : `arm_body_check.py` reste rouge sur `maa_attack` seulement ; `spear_attack` vert).

Reponse (17:40) : GIF du Lancier vu, l'utilisateur dit « très bien » : lecture probable = Q1 A (garder la prise B cuite), NON CONFIRMÉ explicitement, rien n'est noté comme décidé tant qu'il n'a pas dit A.
Reponse (17:10, plus ancienne) : Q1 : pas de choix encore, l'utilisateur demande UN GIF avec plusieurs angles de l'attaque du Lancier (prise B, et A pour comparer) pour juger ; la question Q1 reste ouverte jusqu'au GIF. Q2 : A (tolérer, limite de arm_body_check à 5 cm pour maa_attack seul). (réponse donnée dans la conversation, 2026-11-13 ~17:10)
GIF Q1 (CELEBORN-1113f, 2026-11-13 17:3x, hors jeu, vu par l'agent à l'image 24) : `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Saved\c1113e\lancier_attaque_A_B_angles.gif` — 57 images (0 à 56, frappe f28), 0,14 s chacune ; rangée du haut = A ancienne prise, rangée du bas = B cuite ; colonnes de gauche à droite : face, profil (de sa droite), dos, vue RTS (avant-droite, 55°), caméra du jeu (de derrière, plongée 55°, unité tournée de 40°). Q2 FAIT : arm_body_check maa_attack limite 5 cm (commit 132b234 sur hero-j8-close), spear_attack inchangé (1 cm, vert).
