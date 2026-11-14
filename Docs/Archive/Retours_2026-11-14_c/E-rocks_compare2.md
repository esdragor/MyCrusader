# [À ESSAYER] rocks_compare2 : décor des cartes, tour 2 (B, D et leur mélange sur un sol de biome)

STATUT: VU À L'ÉCRAN (8 PNG ouverts par l'agent SAURON-1114c, 2026-11-14 12:54)
GESTE: interne (commandes `hol.Rocks.Review2`, `hol.Biome.Index` ; Entrée = vue suivante)
SON: sans objet (aucun son dans cet essai)
branches: visual

**Commande** : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 rocks_compare2 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
(3 min au plus ; Entrée = vue suivante ; la lettre est écrite en jaune sur le sol.)

**Ce que tu juges** (D800 : tu as aimé B ET D au tour 1) : un coin de carte (8 rochers de 0,7 à 5 m, 2 groupes de petites pierres, 2 souches, 2 fourrés), couleurs plus foncées et souche deux fois plus grande qu'au tour 1.

| Lettre | Sol | Rochers | Souches / fourrés |
|---|---|---|---|
| A | Herbe fraîche | B seul : 8 à facettes gris-bleu foncé | coupe claire / cubes |
| B | Herbe fraîche | D seul : grappes rondes brun foncé | à racines / boules |
| C | Herbe fraîche | surtout B (6 sur 8) | une de chaque / boules |
| D | Herbe fraîche | moitié-moitié (4 / 4) | une de chaque / boules |
| E | Herbe fraîche | surtout D (2 sur 8 à facettes) | une de chaque / boules |
| F | Forêt sombre | moitié-moitié | une de chaque / boules |
| G | Forêt sombre | B seul | coupe claire / cubes |
| H | Forêt sombre | D seul | à racines / boules |

**Vu par l'agent** : sur l'Herbe fraîche les rochers se lisent maintenant gris anthracite (B) et brun très foncé (D), nets sur le vert ; les souches se voient à ce zoom (coupe claire beige, racines en étoile) ; les fourrés de boules vert moyen ressortent, ceux de cubes (A) aussi. Sur la Forêt sombre le contraste baisse : rochers sombres sur sol sombre encore lisibles, mais le fourré de cubes de G se fond presque dans le sol. Les mélanges C/D/E se lisent comme un mélange (pas deux tas séparés).

**Limites honnêtes** :
- Le sol est un carré de 120 m posé SOUS la scène par l'outil d'essai (matériau du sol de la Plaine, `MI_Ground_Plain`, avec les couleurs du biome) : la Plaine chargée dessine encore les dalles grises que `HOLBiomeApplier` ne reprend pas (cause du « damier gris » du tour 1, non corrigée ici). De fines lignes de grille restent visibles sur ce sol.
- Contexte 2026-11-14 13:13 (SAURON-1114d) : les dalles grises venaient de la carte de TEST Plain.umap (scenario sans taille), pas d une regression ; rocks_compare2 joue maintenant la Plaine generee Petite (Plain_Small, 3af0e15), sol de biome pose par la carte elle-meme ; preflight PASS, 8 PNG revus : meme scene, mais un autel de site sacre apparait au centre de la vue et un arbre en haut a gauche (les reponses aime / pas aime de la question restent valables pour les rochers).
- Formes du moteur (cube, sphère, cylindre), outil d'essai seulement : rien n'est changé dans les cartes.
- Préflight : la partie scénario est PASS (Sanity PASS, 8 captures) ; le contrôle « user's launch » dit binaire plus ancien que le C++ de main (fusion de main du 2026-11-14 12:44 sans build, agent sans C++) : relancer `Tools\build.ps1` dans Sauron avant de jouer.

Réponse : `Retours\Q-rocks_compare2.md`.
