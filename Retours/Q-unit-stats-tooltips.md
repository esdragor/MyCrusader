# [À RÉPONDRE] Textes des infobulles par stat du bloc d'unité (2026-11-09, Galadriel-C++)

**Contexte.** Le bloc de stats sous les points de vie d'une unité sélectionnée est maintenant six petits textes (Attaque, Attaque bâtiments, Armure mêlée, Armure distance, Armure héros, Vitesse), chacun avec SA propre infobulle au survol. Les TEXTES ont été écrits par l'agent, pas par toi : à relire. Défaut codé = textes actuels (LOCTEXT dans `HOLGameScreen.cpp`, `RefreshUnitStats`, une ligne par texte ; fr seulement, en/ja pas encore traduits).

| Stat | Texte actuel de l'infobulle (exemple Homme d'armes) |
|---|---|
| Attaque | « Attaque de base 12 » + « Technologies +10 % » (seulement s'il y a un bonus) + « Portée mêlée » (ou « Portée 20 m » à distance) |
| Attaque bâtiments (Bélier : seule ligne d'attaque) | « Dégâts de chaque coup contre un bâtiment : 60 » |
| Armure mêlée | « Retire 4 à chaque coup de mêlée reçu (1 au minimum) » |
| Armure distance | « Retire 4 à chaque coup de tir reçu (1 au minimum) » |
| Armure héros | « Retire 2 à chaque coup reçu d'un héros (1 au minimum) » puis « Armure siège 0 : retire 0 à chaque coup de siège reçu (1 au minimum) » |
| Vitesse | « Vitesse de déplacement : 7 mètres par seconde » |

**Options.**
- **A. OK tel quel** (recommandé) : on garde ces textes.
- **B. Plus court** : seulement la valeur et l'effet en 3-5 mots (ex. « Armure mêlée : -4 par coup reçu »).
- **C. Armure siège à part** : une septième stat « Armure siège » avec sa propre infobulle, au lieu de la ligne ajoutée à l'armure héros.
- **Autre** : tes mots, stat par stat.

**Conséquence visible.** Seul le texte au survol change ; le bloc et ses valeurs restent.

**Recommandation : A.** Les textes disent l'effet réel en combat (retrait par coup, minimum 1), sans inventer de règle.

**Non vérifié par l'agent.** Le survol réel : l'infobulle Slate ne sort pas sur une capture hors écran ; la preuve est le texte lu sur chaque widget (`HUD stats tip` du journal, scénario `unit_stats_attack`). À essayer au survol, souris sur « Armure mêlée » d'un Homme d'armes.

Réponse : _(à remplir)_

Reponse (utilisateur, 2026-11-09 17:14) : B (plus court : la valeur et l'effet en 3 a 5 mots, ex. Armure melee : -4 par coup recu). Applique par Galadriel-3 (LOCTEXT de RefreshUnitStats dans HOLGameScreen.cpp + waitlog du scenario unit_stats_attack).
