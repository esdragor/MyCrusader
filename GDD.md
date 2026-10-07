# GAME DESIGN DOCUMENT — RTS FANTASY

**Version 0.5** — Préproduction
Remplace `RTS_Fantasy_GDD_v0.1.txt`, `v0.2_CORRIGE.txt` et `v0.2_FINAL.txt`.

> **Convention :** les points marqués ⚠️ **[Qxx]** ne sont pas encore tranchés. Ils renvoient à la liste des questions ouvertes (§ 20). Chaque réponse validée est consignée dans le **journal des décisions** (§ 21), puis reportée dans le corps du document.

---

## 0. Résumé exécutif

**Genre :** RTS fantasy compétitif en temps réel.

**Références de sensation :** Age of Empires, Stronghold Crusader, Battle for Middle-earth.

**Phrase directrice :**

> « Un RTS fantasy classique dans lequel ton héros fait évoluer ta civilisation, commande ton armée, peut affronter directement les héros ennemis, et doit survivre à un champ de bataille qui peut lui-même changer. »

Le joueur dirige une civilisation fantasy complète : économie, infrastructure, technologies, armée et territoire. Au centre de cette civilisation se trouve un **héros-commandant** qui progresse pendant la partie.

- Le **niveau du héros** détermine le potentiel de développement de la civilisation, sans constituer un système d'âges classique. Les bâtiments donnent accès aux technologies concrètes, les ressources les financent.
- Les héros peuvent se **défier en duel**. Le héros vaincu meurt et devient indisponible pendant une durée limitée ; son niveau et ses acquis restent intacts.
- La **carte évolue** via des événements mondiaux (éruption, tempête, incendie, séisme…) qui créent des décisions stratégiques, pas seulement des effets visuels.

**Identité propre — trois éléments :**

1. Héros persistants et progression de civilisation.
2. Duels de héros à fort impact mais non obligatoires.
3. Champ de bataille dynamique et évolutif.

---

## 1. Piliers de design

**PILIER 1 — LE RTS AVANT TOUT**
Le joueur doit pouvoir jouer comme à un RTS traditionnel : récolter, construire, produire, rechercher, explorer, composer son armée, combattre, contrôler le terrain, détruire l'adversaire.
Le système de héros ne doit jamais remplacer la profondeur macro/micro du RTS.

**PILIER 2 — LE HÉROS COMME COMMANDANT**
Le héros est un multiplicateur de puissance et une représentation du développement de la civilisation. Il ne doit pas pouvoir gagner une bataille entière à lui seul dans la majorité des situations.

**PILIER 3 — LA PROGRESSION SANS SYSTÈME D'ÂGES CLASSIQUE**
Le joueur ne clique pas simplement sur « passer à l'âge suivant ». Le développement résulte de l'interaction :
`Niveau du héros + Infrastructure + Technologies + Ressources`

**PILIER 4 — LA MORT EST UNE PUNITION, PAS UN RESET**
Un héros mort crée une fenêtre de faiblesse. Il ne fait pas perdre plusieurs minutes de progression permanente.

**PILIER 5 — ASYMÉTRIE FORTE MAIS LISIBLE**
Les factions doivent être réellement différentes : économie, composition d'armée, technologie, rythme, rôle du héros, mécanique signature. Une faction ne doit pas être une autre faction avec des statistiques modifiées.

**PILIER 6 — LA CARTE EST UN SYSTÈME**
Le terrain n'est pas une décoration. Ressources, passages, points stratégiques et événements doivent modifier les décisions du joueur.

### Ce que le jeu ne doit pas devenir

- un MOBA avec construction de base ;
- un RPG avec une armée décorative ;
- un jeu de duel avec une économie ;
- un RTS où les événements aléatoires jouent à la place du joueur.

**Cœur :** `ÉCONOMIE + INFRASTRUCTURE + ARMÉE + POSITIONNEMENT + HÉROS + ÉVÉNEMENTS`
Le joueur doit toujours gagner parce qu'il a mieux commandé sa civilisation que son adversaire.

---

## 2. Fantasme du joueur

Le joueur doit se sentir souverain d'une civilisation en pleine montée en puissance. Il doit pouvoir raconter après une partie :

> « Mon héros a commencé comme un jeune commandant, puis a gagné de l'expérience, j'ai développé ma capitale, débloqué une nouvelle infrastructure, constitué une armée spécialisée, mon héros a défié celui de l'ennemi, je l'ai vaincu, puis un volcan s'est réveillé et a coupé la moitié de la carte. J'ai profité de cette fenêtre pour attaquer sa base. »

Le jeu doit produire ce type d'histoires émergentes.

---

## 3. Boucles de jeu

- **Macro :** Explorer → Récolter → Construire → Produire → Développer → Rechercher → Étendre → Combattre → Adapter → Répéter
- **Héros :** Agir → Gagner de l'XP → Monter de niveau → Débloquer du potentiel → Développer le commandement → Prendre davantage de risques → Duel / mort / victoire → Continuer sa progression
- **Stratégique :** Information → Décision → Investissement → Timing → Engagement → Conséquence → Nouvelle information

---

## 4. Structure d'une partie

**Durée cible *(décision D02)* :** **25 à 35 minutes en 1v1** (référence d'équilibrage). En équipes, les parties s'allongent naturellement (estimation : 35 à 50 minutes).

**Repères de rythme pour une partie 1v1 de 30 minutes** (calculés à partir de la durée cible, à valider en test) :

| Repère | Valeur indicative |
|---|---|
| Ouverture → Développement | ~ 5 min |
| Premier vrai conflit | ~ 10-12 min |
| Dernier palier de civilisation (héros niveau 9, joueur actif) | ~ 20-23 min, soit environ 70-80 % de la partie |
| Événements mondiaux majeurs | 1 à 2 par partie |
| Fenêtre de récupération du héros | ≈ 40 s (niveau 1) à ≈ 110 s (niveau 10), soit 2 à 6 % de la partie (D25) |

| Phase | Objectifs | Rôle du héros |
|---|---|---|
| **1. Ouverture** | lancer l'économie, explorer, identifier les ressources, sécuriser la base, choisir un premier plan de développement | relativement vulnérable ; l'économie est prioritaire |
| **2. Développement** | augmenter la production, construire les bâtiments militaires, premières technologies, contrôler les ressources contestées, première armée | commence à avoir un impact réel |
| **3. Conflit** | attaquer, défendre, harceler, contrôler les expansions, chercher les ouvertures, provoquer ou accepter un duel | premiers gros pics de puissance |
| **4. Domination / Crise** | sécuriser les ressources rares, technologies avancées, exploiter les faiblesses, gérer les événements de carte | commandant majeur |
| **5. Fin de partie** | supériorité militaire ou économique, détruire le centre principal adverse, accomplir la condition de victoire, empêcher le retour de l'adversaire | — |

---

## 5. Économie

### 5.1 Ressources

Quatre ressources de base. Valeurs, taux de collecte et coûts restent à équilibrer.

| Ressource | Utilisation | Sources | Note de design |
|---|---|---|---|
| **Nourriture** | unités, croissance économique, technologies agricoles, certaines unités spécialisées | animaux, fermes, chasse, pêche, ressources naturelles | — |
| **Bois** | bâtiments, fermes, archers, machines, infrastructures | forêts, améliorations économiques | — |
| **Pierre** | fortifications, bâtiments avancés, centres secondaires, murs, défenses | — | ressource **contestable** pour créer des points de tension |
| **Or** | unités avancées, technologies, héros / capacités selon faction, unités élites | — | devient progressivement la ressource stratégique majeure |

### 5.2 Ressources spéciales

Le jeu n'utilise pas, par défaut, de ressource spéciale propre à chaque faction.

Les factions peuvent disposer de mécaniques uniques, de règles particulières ou d'effets liés au territoire, aux cadavres, aux éléments, à l'honneur, à l'information, etc., sans transformer systématiquement ces mécaniques en ressources supplémentaires.

Une ressource spéciale n'est ajoutée que si elle crée une décision stratégique claire et apporte une identité impossible à obtenir autrement.

### 5.3 Population

La population représente la capacité économique et militaire maximale du joueur. Elle comprend les travailleurs, les unités militaires, les unités spéciales et éventuellement certaines entités propres à une faction.

Le joueur arbitre en permanence : **plus de travailleurs = meilleure économie** ; **plus d'armée = plus de puissance immédiate**.

**Augmentation de la population *(décision D83)* : des maisons, façon AoE.**

- Le centre principal et les centres secondaires donnent une **base** de population.
- La **maison**, petit bâtiment en bois, ajoute **~ +10** de population (valeurs indicatives, à régler en test), jusqu'au plafond de 175.
- Arbitrage bois / population ; les maisons sont des cibles de raid et peuvent servir d'obstacles autour de la base. Une faction pourra plus tard avoir sa propre version de la maison.

Règle spéciale des Héritiers du Feu : une unité d'élite coûte environ 2 places de population.

**Population maximale *(décision D03)* : 175 par joueur.**

Répartition indicative en milieu et fin de partie (à valider en test) :

| | Places de population |
|---|---|
| Travailleurs (économie standard) | ~ 70 à 90 |
| Armée | ~ 85 à 105 |
| Armée des Héritiers du Feu (unités à 2 places) | ~ 45 à 50 unités |

Pire cas sur une carte à 8 joueurs : **environ 1 400 unités simulées**. Ce chiffre sert de cible de performance (§ 16.4).

### 5.4 Expansion

Le joueur possède une base principale et peut développer des expansions pour :

- sécuriser de nouvelles ressources ;
- contrôler des positions stratégiques ;
- augmenter la production ;
- créer des points de projection militaire ;
- obtenir de nouvelles zones de construction.

Une expansion comporte un risque : plus elle est éloignée, plus elle est difficile à défendre.

Le joueur choisit entre investir dans sa base, prendre une expansion, investir dans l'armée ou accélérer sa technologie. Aucune option ne doit être systématiquement optimale.

### 5.5 Marché *(décision D70)*

**Un marché commun à toutes les factions**, façon AoE.

- Bâtiment économique standard : achat et vente de **toutes les ressources** (nourriture, bois, pierre) contre de l'or.
- **Taux évolutifs et communs *(D84)* :** un seul cours par ressource, **partagé par tous les joueurs, alliés comme ennemis**. Plus une ressource est vendue (par n'importe qui), moins elle rapporte ; plus elle est achetée, plus elle coûte. Les taux reviennent lentement à l'équilibre.
- **Rôle :** convertir un surplus en ce qui manque, quelle que soit la ressource. Le cours commun crée une interaction indirecte : vider le marché d'une ressource la rend plus chère pour l'adversaire.
- **Au prototype *(D84)* :** oui, pour régler les taux avant d'y ajouter le *marché noir* du Cercle.
- **Cercle de l'Ombre :** meilleur taux, grâce au *marché noir* (D69, § 13.5).
- C'est un bâtiment à protéger et à cibler. L'IA doit savoir s'en servir.

---

## 6. Construction et base

**Construction libre, façon AoE / Stronghold *(décision D22)*.**

- On construit partout sur le terrain constructible où l'on a la **vision**, murs et portes compris.
- **Règle anti-rush :** aucune construction dans un rayon autour d'un **centre principal ennemi** (rayon à régler en test).
- Les règles propres à certains bâtiments restent possibles (bâtiment de pêche près de l'eau, mine sur un gisement…).
- Le **territoire** (XP de territoire, D05) se mesure aux **points stratégiques et centres secondaires contrôlés**, pas à une zone de construction.

| Catégorie | Exemples |
|---|---|
| **Économiques** | structures de collecte, stockage, fermes, améliorations économiques |
| **Militaires** | casernes, stands de tir, écuries, ateliers de siège, bâtiments de production spécialisés |
| **Technologiques** | forge, académie, structures magiques, bâtiments propres aux factions |
| **Défensifs** | murs, portes, tours, bastions, garnisons, défenses spécialisées |
| **Centraux** | centre principal, centres secondaires, structures majeures |

Les bâtiments ont des rôles lisibles et participent directement à la progression technologique.

La base doit être plus qu'un amas de bâtiments : défense, production, économie, projection. Certaines factions doivent être meilleures en défense que d'autres.

### 6.1 Siège *(décision D23)*

**Siège léger, façon AoE4 réduit.** Les héros ne rasent pas les bâtiments (D17), et la victoire standard passe par la destruction du centre principal : le siège est donc indispensable pour finir une partie.

| Engin | Palier | Rôle | Vulnérable à |
|---|---|---|---|
| **Bélier** | 1 | bâtiments et portes | corps à corps |
| **Mangonneau / Catapulte** | 2 | groupes et murs, longue portée | Cavalier léger, corps à corps |
| **Trébuchet** | 3 | fortifications de loin ; doit être monté et démonté | Cavalier léger, corps à corps |
| **Canon** *(D38)* | 3, technologie « Armes à poudre » | courte portée, gros dégâts, plus mobile que le Trébuchet | Cavalier léger, corps à corps |
| *Cracheur de bile* *(D59, Légions Noires, à la place du Canon)* | 3 | projectile d'acide : dégâts à l'impact, puis **flaque d'acide** qui inflige des dégâts sur la durée pendant un court temps | Cavalier léger, corps à corps |

- **Poudre (D38) :** le Canon s'ajoute aux autres engins, il ne remplace rien. Variante des Héritiers du Feu : la *Bombarde*. Les Légions Noires n'ont pas de poudre : leur variante est le *Cracheur de bile* (D59, § 13.3).

- **Murs, portes et tours sont solides** : percer une base fortifiée demande du siège.
- **Arbitrage économique :** le bois et l'or du siège ne vont pas dans l'armée.
- Le Cavalier léger est le contre naturel du siège (§ 7.2).

### 6.2 Bonus de hauteur *(décision D23)*

**Les unités à distance qui tirent depuis une position plus haute que leur cible reçoivent un bonus** : colline, falaise, tour, rempart. Valeur indicative à régler en test : + dégâts et/ou + portée.

**Remparts praticables *(décision D24)*, dès le prototype :**

- Les unités montent sur les **murs de pierre** par les tours et les portes, s'y déplacent et tirent avec le bonus de hauteur. Les **murs de bois** ne sont pas praticables.
- Les unités sur un rempart sont la cible prioritaire du siège. La destruction d'un segment de mur fait tomber les unités qui s'y trouvent, avec des dégâts.
- L'IA doit savoir garnir ses remparts en défense et les cibler en attaque.
- Les échelles, les douves et l'huile bouillante, façon Stronghold, restent **hors périmètre** pour l'instant.

Conséquence technique : navigation sur plusieurs niveaux (§ 16.5).

---

## 7. Unités, combat et contrôle

### 7.1 Unités

Les unités sont individuelles et contrôlées directement, dans l'esprit des RTS classiques.

Chaque unité possède au minimum : rôle, coût, population, temps de production, points de vie, attaque, défense, vitesse, portée éventuelle, type de dégâts, type d'armure, capacités, contres, améliorations.

#### Socle commun et variantes rares *(décision D20)*

**Toutes les factions partagent un socle d'unités commun.** L'identité vient des unités emblématiques (§ 13) et de **quelques variantes rares**, pas d'une refonte générale du socle.

**Socle commun** (noms et rôles provisoires) :

| Rôle | Unité commune | Catégorie d'armure |
|---|---|---|
| Travailleur | Paysan | infanterie légère |
| Anti-cavalerie | Lancier | infanterie légère |
| Infanterie lourde | Homme d'armes | infanterie lourde |
| Tireur léger | Archer | distance |
| Tireur anti-armure | Arbalétrier | distance |
| Tireur à poudre *(palier 3, D38)* | Arquebusier | distance |
| Cavalerie légère | Cavalier léger | cavalerie légère |
| Cavalerie lourde | Cavalier lourd | cavalerie lourde |
| Siège | Bélier, Mangonneau, Trébuchet, Canon (§ 6.1) | siège |

Les détails des contres sont au § 7.2 (D21).

**Poudre *(décision D38)* : commune à toutes les factions, sauf contre-ordre, au palier 3.**

- La technologie commune **« Armes à poudre »** (palier 3) débloque deux nouvelles unités du socle : l'**Arquebusier** (tireur, catégorie d'armure distance) et le **Canon** (siège, § 6.1). Elles s'ajoutent à l'Arbalétrier et aux engins existants.
- **Exceptions de faction** (règle des variantes ci-dessous) :
  - **Légions Noires :** pas de poudre *(D59)*. À la place du Canon : le ***Cracheur de bile***, engin de siège à acide (§ 6.1). Pas d'équivalent de l'Arquebusier ; en compensation, un bâtiment propre en plus au palier 3 : l'***Ossuaire*** (D60, § 13.3).
  - **Héritiers du Feu :** *Bombarde* à la place du Canon, et l'***Arquebusier de la Forge*** (nom temporaire, D61) à la place de l'Arquebusier : il **tire plus vite et plus loin**. Ce sont leurs deux variantes du socle. ⚠️ Vigilance : l'Archer ne le dépasse plus en portée ; il reste vulnérable à la cavalerie, et la règle d'élite (coût ×2) limite leur nombre.
- **Place de l'Arquebusier *(D58)* :** tireur lourd de fin de partie, façon Handcannoneer d'AoE4 (§ 7.2). L'Arbalétrier reste le spécialiste rentable contre l'armure lourde ; l'Arquebusier est la puissance brute chère. Aucun des deux n'est une unité anti-héros (D62).

**Amélioration et déblocage *(décision D38)* : une unité n'est jamais remplacée en cours de partie.**

- Une amélioration (technologie, palier) ne change une unité **qu'en statistiques et en visuel** : elle garde son nom, son rôle et ses capacités de base.
- La nouveauté passe par le **déblocage de nouvelles unités**, qui s'ajoutent aux anciennes. Exemple : l'Arquebusier s'ajoute à l'Arbalétrier, il ne le remplace pas.
- Les variantes rares ci-dessous ne sont pas concernées : ce sont des choix de composition de la faction, présents dès le début de la partie.

**Règle des variantes : l'exception, jamais la généralité.**

- Pour un rôle donné, **une seule faction (deux au maximum)** remplace l'unité commune par une variante.
- Chaque faction a **une ou deux variantes au maximum** dans tout son socle.
- Une variante garde le rôle de l'unité commune, mais ajoute une **spécificité nette**.

**Exemples :**

- **Légions Noires — Zombie** à la place du Paysan *(D66)* : moins cher, **sans nourriture**, **collecte plus lente**, 1 place de population. Ce n'est pas un handicap de population : **les unités des Légions coûtent moins cher**, donc la faction a besoin de moins de revenu, et donc de moins de travailleurs.
- **Légions Noires — *Cracheur de bile*** à la place du Canon *(D59)* : leur seconde variante (2 au maximum respecté). Le rôle « Canon » a ainsi deux variantes : *Bombarde* (Héritiers) et *Cracheur de bile* (Légions).
- **Templiers — Frère convers** à la place du Paysan *(D91)* : se défend bien ; collecte plus vite pendant une croisade. Le rôle « Travailleur » a ainsi deux variantes (Zombie, Frère convers), le maximum de D20.
- **Ordre de l'Aube — Hallebardier** à la place du Lancier *(D79)* : anti-cavalerie avec, en plus, une efficacité contre l'infanterie lourde, mais plus cher. Un « mur de piques » qui tient la ligne, fidèle à l'identité défensive de l'Aube ; son coût ralentit l'armée de l'Aube en compensation.

**Unités emblématiques :** les 3 unités décrites pour chaque faction au § 13 **s'ajoutent** au socle commun. Exceptionnellement, l'une d'elles peut tenir lieu de variante d'un rôle (par exemple, l'Archer de l'Aube à la place de l'Archer).

**Héritiers du Feu :** leur règle d'élite (≈ ×2 en coût, population et efficacité) s'applique **à tout leur socle**. C'est une exception de faction assumée, pas une variante unité par unité.

### 7.2 Combat

Le combat repose sur : composition, positionnement, contre-unités, terrain, focus fire, capacités, commandement du héros.

**Principe :** aucune unité standard ne doit être universellement supérieure. Toutes les compositions doivent avoir des réponses.

#### Matrice de contres *(décision D21 — modèle AoE4)*

Les contres reposent sur des **bonus de dégâts par catégorie d'armure**, comme dans AoE4. Il n'y a pas de cycle strict : chaque unité a une cible privilégiée et des ennemis naturels.

| Unité | Identité | Bonus de dégâts contre | Vulnérable à |
|---|---|---|---|
| **Lancier** | anti-cavalerie bon marché | **toute la cavalerie** | Archer, Homme d'armes |
| **Homme d'armes** | infanterie lourde, **tank** : pas de bonus, une forte armure | — (gagne à l'usure contre Lanciers et Archers) | **Arbalétrier** |
| **Archer** | tireur léger de masse | **infanterie légère** (Lancier, Paysan…) | Cavalerie légère, Cavalerie lourde ; peu efficace contre les armures |
| **Arbalétrier** | tireur anti-armure | **armures lourdes** (Homme d'armes, Cavalerie lourde) | Cavalerie légère, Cavalerie lourde |
| **Arquebusier** *(D58, palier 3)* | tireur **lourd** de fin de partie : gros dégâts par tir, recharge lente, courte portée, cher en or | aucun bonus de catégorie, mais **ignore une partie de l'armure** de toutes les cibles  | Cavalerie légère, Cavalerie lourde, Archer (plus longue portée) |
| **Cavalier léger** | raid, harcèlement, éclairage | **tireurs** et **siège** | Lancier, Cavalier lourd |
| **Cavalier lourd** | choc, **charge** dévastatrice | infanterie légère et tireurs (bonus de charge) | Lancier, Arbalétrier |
| **Siège** | anti-bâtiments, anti-groupes | bâtiments, formations serrées | toute unité au contact, Cavalier léger |

**Lecture pour le joueur :** cavalerie → lanciers ; tireurs → cavalerie ; armure lourde → arbalétriers (ou arquebusiers, plus chers, en fin de partie) ; masse d'infanterie légère → archers ; lanciers et archers → hommes d'armes ou cavalerie lourde.

**Pas d'unité anti-héros *(D62)* :** aucune unité n'a de bonus contre les héros, et la résistance héroïque (×0,3, D17) s'applique à toutes les troupes. L'Arbalétrier et l'Arquebusier font simplement **beaucoup de dégâts de base** : un grand nombre d'entre eux finit par blesser sérieusement un héros, même réduit à ×0,3.

**Unités emblématiques et variantes :** chacune se rattache à une catégorie d'armure et à une ligne de la matrice, avec une spécificité. Par exemple : Chevalier Vertueux = infanterie lourde à redirection de dégâts ; Spectre Assassin = unité rapide anti-tireurs ; Hallebardier = anti-cavalerie avec un bonus contre l'infanterie lourde.

**Mise en œuvre :** types de dégâts et catégories d'armure dans les fiches `UnitData` (multiplicateurs de bonus).

#### Moral *(décision D74)*

**Le moral est une famille d'effets, pas une jauge** (façon AoE4 / BFME).

- Aucune statistique « moral » sur les unités. Les effets de moral sont des **effets nommés et temporaires** qui modifient attaque, armure et/ou cadence : *Triomphe*, *Démoralisé* (D15), *Hésitation* (D16), les auras de commandement (D26), *Aura de terreur*, *Murmures*, *Cor de l'Héritage*, etc.
- « Moral » est une **catégorie** commune à ces effets : même affichage (icône sur l'unité), mêmes règles de cumul, même traitement par les capacités qui purifient.
- **Cumul (indicatif, à régler en test) :** un même effet ne se cumule pas avec lui-même (il est rafraîchi) ; des effets différents s'additionnent, dans un **plafond global** de bonus et de malus de moral.
- **Purification :** le Moine Lumineux retire les effets de moral négatifs (lien avec « suppression de malédictions »).
- **Jamais de déroute :** le moral ne fait pas fuir les unités et ne retire pas le contrôle au joueur. La peur reste un contrôle bref, encadré par D37.
- **Technique :** catégorie d'effet (tag `Effect.Morale.*`) dans le système d'effets des unités légères (D29) ; mêmes tags pour les acteurs GAS.
- Un moral de groupe à états (Exalté / Stable / Ébranlé) reste une extension possible après le prototype, si le moral paraît trop abstrait en test.

### 7.3 Contrôle

Le contrôle doit être familier.

- **Sélection :** clic, sélection multiple, sélection par cadre, double-clic / filtres éventuels.
- **Raccourcis par défaut *(décision D30, standard AoE4)*** — tous reconfigurables par le joueur :

| Action | Raccourci |
|---|---|
| Assigner la sélection à un groupe | `CTRL + chiffre` |
| Sélectionner un groupe | `chiffre` |
| Centrer la caméra sur un groupe | `chiffre` deux fois |
| Ajouter la sélection au groupe | `MAJ + chiffre` |
| Retirer la sélection du groupe | `CTRL + MAJ + chiffre` |
| Bâtiments militaires | `F1` (appuis successifs : bâtiment suivant) |
| Bâtiments économiques | `F2` (appuis successifs : bâtiment suivant) |
| Bâtiments technologiques | `F3` (appuis successifs : bâtiment suivant) |
| Sélectionner le héros | `F4` (deux fois : centrer la caméra) |
| Sélectionner les paysans inactifs | `.` |
| Sélectionner toute l'armée | `CTRL + A` |
| Ordres de commande et de construction | grille `A Z E R / Q S D F / W X C V` (AZERTY ; une touche = une case du panneau de commandes) |
| Capacités du héros | ligne dédiée de la grille |
| Défier en duel | touche dédiée dans la grille du héros |
- **Ordres :** déplacement, attaque, patrouille, maintien de position, suivre, focus, capacités.

Le joueur doit pouvoir contrôler plusieurs fronts.

---

## 8. Carte

### 8.1 Terrain

Le terrain influence le déplacement, la visibilité, le positionnement, la défense, les lignes de tir et les événements.

Types possibles : forêt, plaine, montagne, marais, rivière, volcan, ruines, zones sacrées, zones corrompues.

Le terrain doit créer des décisions, pas seulement des bonus numériques.
**Acquis :** la hauteur modifie le combat. Les tireurs placés plus haut que leur cible reçoivent un bonus (§ 6.2, D23).

**Effets de terrain *(décision D33)* : peu de règles, mais fortes et lisibles.**

| Terrain | Effet |
|---|---|
| **Forêt** | bloque la vision (embuscades) ; ralentit la cavalerie et le siège ; **couvert** : moins de dégâts à distance reçus |
| **Marais** | ralentit tout le monde ; **empêche la charge** de la cavalerie lourde |
| **Gué / rivière** | passage étroit, ralentissement ; défense réduite pour les unités dans l'eau |
| **Route** | vitesse augmentée |
| **Hauteur** | bonus des unités à distance (D23) |
| **Zones sacrées / corrompues** | bonus pour la faction alignée, léger malus pour la faction opposée ; **sur les cartes compétitives, créées uniquement par des capacités de faction** (D40) |

**⚠️ Points de vigilance pour l'équilibrage :**

- **Symétrie des cartes :** forêts, marais, gués, routes et hauteurs (et, hors compétitif, zones sacrées ou corrompues) sont répartis de façon équitable entre toutes les positions de départ, de 2 à 8 joueurs.
- **Zones alignées *(décision D40)* : cartes compétitives neutres.**
  - Sur les cartes compétitives (classées), **la carte ne pose aucune zone sacrée ou corrompue**. Ces zones n'existent que si une capacité de faction les crée : elles sont **temporaires, visibles et contrables**.
  - Les zones alignées posées par la carte restent possibles sur les cartes non classées et dans les scénarios.
  - Pas d'affinité de terrain par faction : le terrain (forêt, marais, etc.) agit de la même façon pour tout le monde. Cela reste cohérent avec l'élite purement statistique des Héritiers (D39).
- **Couvert en forêt :** à surveiller face aux factions qui reposent sur les tireurs (Archer de l'Aube), pour qu'une forêt ne rende pas un camp intouchable.
- **Marais anti-charge :** il ne doit pas annuler entièrement une faction à cavalerie lourde sur une carte riche en marais.
- **Valeurs :** à régler en test, avec des modificateurs modérés. La règle doit se *sentir* sans décider seule d'une bataille.

### 8.2 Brouillard de guerre

Le joueur ne voit pas toute la carte. La vision dépend des unités, bâtiments, héros, tours, éclaireurs et capacités.

La reconnaissance est une ressource stratégique. Le Cercle de l'Ombre doit exceller dans cette dimension.

### 8.3 Exploration

La carte contient : ressources, passages, points élevés, objectifs, ruines, monstres, événements, ressources contestées.

Explorer donne : information, XP potentielle, opportunités, accès à des ressources. Le scouting doit être une activité stratégique réelle.

### 8.4 Unités neutres et monstres

Les créatures neutres peuvent protéger des ressources, occuper des ruines, bloquer des passages, devenir des objectifs, ou être exploitées par certaines factions :

- Enfants du Dragon : interaction supérieure avec les créatures ;
- Cercle de l'Ombre : utilisation de créatures comme diversion ;
- Légions Noires : réanimation éventuelle de certains cadavres.

**Présence légère et ciblée *(décision D32)*, après le prototype :**

- **Quelques camps par carte**, placés symétriquement, qui **gardent quelque chose de précieux** : gisement riche, passage stratégique.
- **Pas de réapparition :** un camp nettoyé l'est pour la partie.
- **XP modérée**, comptée dans la part « Exploration + Événements » (~15 %, D05).
- Point d'appui pour les mécaniques de faction : apprivoisement (Enfants du Dragon), cadavres (Légions Noires), diversion (Cercle de l'Ombre).
- Pas de chasse aux monstres façon Warcraft 3 : le combat contre les neutres reste une décision ponctuelle, pas une voie de progression.

---

## 9. Le héros

**Un héros unique par faction *(décision D10)*.** Le héros incarne sa faction. La variété entre parties vient des spécialisations de palier (D07) et des talents (D08). L'équipement a été supprimé (D102). Les fiches `HeroData` permettront d'ajouter d'autres héros plus tard sans refonte.

**Héros du prototype :** Paladin-Commandant de l'Ordre de l'Aube (§ 13.2, D12) et Seigneur Damné des Légions Noires (§ 13.3, D13).

**Capacité ultime *(décision D36)* :** voir § 9.5. Ultimes du prototype décrits aux § 13.2 et § 13.3.

**Héros hors prototype :** Seigneur-Dragon des Enfants du Dragon (§ 13.4, D41), la Voix du Cercle de l'Ombre (§ 13.5, D44), le Champion Héritier des Héritiers du Feu (§ 13.6, D46). Héros des Templiers : ⚠️ à concevoir (§ 13.7, D87).

### 9.1 Rôle

Le héros est un multiplicateur, pas une armée à lui seul. Il amplifie une armée plutôt que la remplacer.

**Trois axes de balance :**

- **Puissance personnelle :** capacité à combattre.
- **Puissance de commandement :** capacité à améliorer l'armée.
- **Valeur stratégique :** capacité à modifier les décisions du joueur.

Un héros peut être moyen en combat mais extrêmement puissant comme commandant. Cela permet d'avoir des héros différents sans hiérarchie basée uniquement sur les dégâts.

### 9.1 bis Résistance héroïque *(décision D17)*

**Le héros est un Sauron défensivement, un capitaine offensivement.**

| | Règle |
|---|---|
| **Puissance offensive** | Environ **5 à 6 unités standard**, mesurée en équivalent d'unités standard, identique pour toutes les factions (≈ 3 unités des Héritiers du Feu). Le héros **ne peut pas raser une armée** et fait peu de dégâts aux bâtiments : pas de raid solitaire sur une base. |
| **Défense face aux troupes** | **Dégâts fortement réduits** de la part des unités ordinaires (indicatif ×0,3). Une armée *peut* tuer un héros, mais il lui faut beaucoup d'unités concentrées sur lui pendant longtemps (~20-30 s), avec des pertes. |
| **Dégâts normaux reçus de** | autres héros, tours, siège. **Pas de contre-mesure dédiée** *(D62)* : aucune unité n'est « tueuse de héros » ; les tireurs à gros dégâts de base (Arbalétrier, Arquebusier) font mal en nombre, malgré la réduction. Exception : le Seigneur-Dragon monté reçoit les dégâts normaux des tireurs (D41). |
| **Héros contre héros hors duel** | Un héros frappe un autre héros plus fort qu'un homme d'armes, mais **ce sont les compétences de duel qui font la différence**. Hors duel, deux héros s'usent lentement ; en duel, ils peuvent se vaincre. |

**Conséquences :**

- **Le duel est la manière normale et la plus sûre d'abattre un héros.** C'est ce qui donne au duel sa valeur propre (Q16).
- **La logique de refus est intuitive :** le héros adverse est plus fort, je refuse et je le tiens à distance ; je suis plus fort, je défie.
- **Héritiers du Feu :** un héros de même valeur absolue pèse proportionnellement plus dans leur petite armée (~45-50 unités). C'est cohérent avec leur identité « qualité plutôt que quantité ».

### 9.2 Commandement

Le héros fournit un **rayon de commandement** autour de lui.

- **Dans la zone :** unités plus efficaces, moral amélioré, certaines formations disponibles, bonus propres à la faction.
- **Hors de la zone :** armée toujours contrôlable, mais perte d'une partie des bonus.

Le compromis : **héros près du front** = plus de puissance mais plus de risque ; **héros en retrait** = plus de sécurité mais moins d'impact.

**Force de l'aura *(décision D26)* : aura moyenne.**

| Élément | Valeur indicative |
|---|---|
| Bonus de base | **+10 à 15 %** sur 1 ou 2 statistiques, selon la faction (Aube : armure et moral ; Légions : dégâts et terreur) |
| Fin de partie | jusqu'à **+20 à 25 %** avec les talents |
| Rayon | un **groupe de bataille** (~20 à 30 unités), pas toute l'armée |

**Valeur totale du héros :** sur une armée d'environ 90 places de population, +15 % vaut environ 13 unités. Ajoutée à sa puissance personnelle (~5-6 unités, D17), elle donne un héros qui vaut **environ 20 unités**. Il est important sans être indispensable : sa mort se ressent dans une bataille sans la décider à elle seule.

### 9.3 Capacités RTS

Capacités possibles : aura, charge, cri de guerre, soin, renforcement, mobilité, reconnaissance, invocation, capacité de faction.

Toutes les capacités ont des temps de recharge. Le héros ne doit pas pouvoir enchaîner ses capacités jusqu'à résoudre tous les combats.

**Déblocage *(décision D42)* : progressif, façon BFME, en décalé des paliers.**

| Niveau | Kit RTS | Moment indicatif (1v1) |
|---|---|---|
| 1 | **aura + capacité 1** | début |
| 4 | **capacité 2** | ~ 9-11 min |
| 7 | **capacité 3** | ~ 16-18 min |
| 10 | **ultime** (D36) | ~ 25 min et plus |

- **Kit RTS cible :** une aura, 3 capacités actives et un ultime, pour tous les héros. Exception : le Seigneur-Dragon (aura + 4 capacités réparties sur deux formes exclusives, D51).
- **Rythme :** la civilisation progresse aux niveaux 3, 6 et 9 (paliers), le héros aux niveaux 1, 4, 7 et 10. Les deux types de moments forts ne tombent jamais au même niveau : le choix de spécialisation de palier (D07) n'est pas noyé sous une nouvelle capacité, et il se passe quelque chose de marquant entre deux paliers.
- **Prototype (niveaux 1 à 6) :** aura + capacités 1 et 2, soit les kits déjà décrits pour le Paladin-Commandant et le Seigneur Damné. La capacité 3 et l'ultime viennent après le prototype ; ils sont prévus dans `HeroData` dès le départ.
- Les déblocages sont conditionnés par l'attribut `Level` du héros (§ 16.1).

### 9.4 Sources d'XP

| Source | Exemples |
|---|---|
| **Combat** | dégâts infligés, ennemis vaincus, héros adverses, monstres puissants |
| **Commandement** | présence dans des combats, participation à une victoire, maintien d'une armée sous commandement, objectifs tactiques |
| **Exploration** | découverte de zones, de lieux importants, d'événements |
| **Développement** | construction de structures majeures, amélioration de la civilisation, recherche de technologies |
| **Territoire** | contrôle de points stratégiques, prise d'objectifs, expansion |
| **Événements** | participation à certains événements mondiaux, objectifs spécifiques |

Le but est d'éviter que « tuer le plus d'unités » soit la seule voie de progression.

**Répartition *(décision D05)* : un socle de développement, un bonus militaire.**

| Famille | Part indicative de l'XP d'une partie |
|---|---|
| Développement | ~ 35 % |
| Territoire | ~ 15 % |
| Combat + Commandement | ~ 35 % |
| Exploration + Événements | ~ 15 % |

- Un joueur qui se développe correctement atteint ses paliers **à peu près à l'heure** (§ 9.5). Celui qui domine militairement les atteint **plus tôt**, sans pouvoir s'envoler.
- **Commander une armée en combat rapporte de l'XP** : les unités qui combattent dans le rayon de commandement en font gagner au héros. Cela encourage le héros à se placer au front (§ 9.2).
- **Rattrapage :** tuer un héros de niveau supérieur rapporte beaucoup d'XP ; tuer un héros de niveau inférieur en rapporte peu.
- Les pourcentages sont des cibles d'équilibrage, à mesurer en test sur des parties de référence (humaines et IA).

**XP pendant la récupération *(décision D06)* :** les sources liées à la civilisation (développement, territoire, exploration par les unités, événements) continuent de rapporter de l'XP. Seuls le combat et le commandement s'arrêtent. Le héros peut monter de niveau et ouvrir un palier pendant sa mort.

### 9.5 Niveaux

Le niveau du héros définit le potentiel de développement de la civilisation. Il ne doit pas fonctionner comme un simple bouton d'âge : il ouvre un potentiel, mais les bâtiments, les ressources et les choix du joueur déterminent ce qui est réellement développé.

**Structure *(décision D04)* : 10 niveaux de héros, dont 3 paliers de civilisation.**

La progression du **héros** et celle de la **civilisation** sont distinctes :

- **Chaque niveau** fait grandir le héros : point de talent, statistiques, capacités. Sensation de progression fréquente, environ toutes les 2 à 3 minutes.
- **Les niveaux 3, 6 et 9** ouvrent en plus un **palier de civilisation** (accès à de nouveaux bâtiments et technologies).
- **Le niveau 10** débloque la capacité ultime du héros et son ultime de duel (D43).

**Forme de l'ultime *(décision D36)* : une capacité de bataille active du kit RTS.**

- Effet massif sur une zone, de courte durée, avec une longue recharge (indicatif : 3 à 4 min).
- **Annoncé et contrable :** un signal visuel clair au lancement, et une réponse possible pour l'adversaire (reculer, changer de terrain, etc.).
- Le héros doit être sur place : pas d'effet à l'échelle de la carte, pas de transformation (pour garder le repère de D17).
- **L'ultime de duel est distinct :** c'est une capacité du kit de duel (§ 10.3), séparée de l'ultime RTS. *Révision D43 :* il se débloque lui aussi au niveau 10 (D36 le prévoyait indépendant du niveau).
- **Serviteurs temporaires** créés par un ultime : hors population, mais plafonnés (proposition, à confirmer en test).
- Hors prototype (niveaux 1 à 6) ; la capacité est prévue dans `HeroData` dès le départ.

| Niveau | Héros | Civilisation (noms temporaires) | Moment indicatif (1v1, joueur actif) |
|---|---|---|---|
| 1 | aura + capacité 1 | **Palier 0 — Fondation** : technologies fondamentales, bâtiments de départ, unités de base | début |
| 2 | talent | — | |
| 3 | talent | **Palier 1 — Essor** : premières infrastructures avancées, nouvelles améliorations, nouvelles unités | ~ 6-8 min |
| 4 | talent, **capacité 2** (D42) | — | ~ 9-11 min |
| 5 | talent | — | |
| 6 | talent | **Palier 2 — Puissance** : bâtiments avancés, technologies spécialisées, unités élites | ~ 13-15 min |
| 7 | talent, **capacité 3** (D42) | — | ~ 16-18 min |
| 8 | talent | — | |
| 9 | talent | **Palier 3 — Légende** : technologies finales, bâtiments majeurs, options de fin de partie | ~ 20-23 min |
| 10 | **ultime** + ultime de duel (D43) | — | ~ 25 min et plus |

Le calendrier des capacités est fixé (D42, § 9.3) ; les gains de statistiques par niveau et les moments indicatifs restent à régler en test.

**Important :** un palier ne donne rien automatiquement. Il ouvre un niveau d'accès ; le joueur doit encore construire et rechercher.

**Ouverture d'un palier *(décision D07)* : un tronc commun et une spécialisation.**

Chaque palier (1, 2 et 3) ouvre :

1. **Un tronc commun**, garanti pour toute la faction : bâtiments et technologies essentiels, dont **les réponses de base aux contres**. Un joueur n'est jamais privé de réponse à cause d'un choix de spécialisation.
2. **Le choix d'une spécialisation parmi 2** : un bâtiment majeur, ou une branche de technologies et d'unités. Ce choix est définitif pour la partie. *Templiers :* la spécialisation est le choix d'une commanderie (D90, § 13.7).

Conséquences :

- Chaque partie produit un build différent : 2 × 2 × 2 = 8 combinaisons par faction.
- **L'éclairage des troupes adverses gagne de la valeur** : identifier la spécialisation adverse permet de s'adapter.
- Au prototype, chaque palier ouvre son tronc commun et 2 spécialisations.
- L'IA choisit ses spécialisations selon des profils de build.

⚠️ Courbe d'XP par niveau à définir en test **[Q04]**.

### 9.6 Arbre de talents

**Décision D08 : un arbre de talents entièrement propre à chaque faction.** Les familles, la structure et le contenu sont spécifiques. Exemple indicatif pour les Légions Noires : Nécromancie / Terreur / Sacrifice. Templiers *(D101)* : Croisade / Chevalerie / Trésor / Les Citadelles.

Le héros gagne environ **8 points de talent** par partie (niveaux 2 à 9, D04). Il ne peut pas prendre toutes les améliorations : le choix crée une spécialisation.

**Garde-fous proposés**, pour que 6 arbres différents restent lisibles et équilibrables :

- même nombre de points disponibles et même présentation à l'écran pour toutes les factions ;
- chaque arbre permet au moins un build orienté **commandement** (le héros comme multiplicateur, pilier 2) et un build orienté **duel / combat personnel** ;
- chaque arbre propose au moins une voie **stratégique** : vision, économie, mobilité ou autre, selon l'identité de la faction ;
- chaque arbre reflète la mécanique signature de sa faction (§ 13).

Ancienne proposition commune, à garder comme grille de référence pour vérifier les 6 arbres :

- **Commandant :** auras, efficacité des formations, vitesse de déplacement de l'armée, moral, résistance.
- **Guerrier :** survie, dégâts, duel, capacités personnelles.
- **Stratège :** vision, reconnaissance, capacités tactiques, économie, mobilité, soutien.

Au prototype, seuls les arbres des 2 factions retenues sont conçus : Ordre de l'Aube et Légions Noires (D11).

### 9.7 Équipement *(supprimé, décision D102)*

**Pas d'équipement de héros.** Le système d'équipement de D09 (3 emplacements, objets achetés en bâtiment, reliques uniques de carte) est supprimé. La progression du héros passe par ses niveaux, ses capacités (D42) et son arbre de talents (D08). Les reliques n'existent plus que comme objectifs du mode Reliques (§ 14.3).

### 9.8 Présence, mort et résurrection

**Héros actif :** bonus de commandement et capacités disponibles.
**Héros absent ou mort :** l'armée reste fonctionnelle, mais les bonus et capacités du héros sont indisponibles. Le joueur ne doit jamais être complètement paralysé par la mort du héros.

**À la mort, sont conservés :** niveau, XP, talents, déblocages, progression de la civilisation.

**Pendant la récupération :** pas de contrôle du héros, pas d'aura, pas de capacités de héros, certains bonus d'armée disparaissent. Puis le héros revient.

La civilisation continue de progresser : l'XP de développement, de territoire, d'exploration et d'événements reste acquise (D06). **La mort est une fenêtre de faiblesse militaire, pas un retard de civilisation.**

**Temps de retour *(décision D25)* :**

| Élément | Règle |
|---|---|
| **Durée** | **selon le niveau**. Indicatif : 30 s + 8 s × niveau, soit ≈ 40 s au niveau 1 et ≈ 110 s au niveau 10 (2 à 6 % d'une partie de 30 min). |
| **Rachat** | **aucun** : le temps de récupération ne peut pas être réduit en payant |
| **Prix** | **prix de résurrection obligatoire, payé à la fin du délai.** Base selon le niveau, modérée (un prix fixe reste l'alternative à comparer en test). |
| **Prix dégressif** | si le joueur ne paie pas à la fin du délai, **le prix baisse progressivement jusqu'à zéro** (indicatif : sur 60 s). Le joueur peut payer à tout moment pendant cette baisse. Un joueur ruiné récupère donc toujours son héros, simplement plus tard. |
| **Lieu de réapparition** | au choix : le centre principal ou un centre secondaire contrôlé |
| **Morts répétées** | aucune aggravation |
| **Bâtiment dédié** | non par défaut ; possible comme mécanique propre à une faction |
| **Mode de jeu** | les durées peuvent être modulées par mode (le mode Catastrophe, par exemple) |


**Principe :** plus le héros est important, plus sa mort doit créer une vraie fenêtre stratégique. La mort ne doit jamais supprimer plusieurs niveaux de progression.

**Momentum — exemple :** le joueur A tue le héros de B.

- A gagne : présence de héros, commandement, pression, possibilité d'attaque.
- B perd : aura, capacités, commandement.
- B conserve : niveau, XP, technologies, infrastructure, civilisation.

La fenêtre de vulnérabilité doit être assez longue pour être exploitable, mais assez courte pour permettre un retour.

La mort coûte du **temps** (délai selon le niveau) et des **ressources** (prix de résurrection), jamais de progression.

### 9.9 Progression hors partie

Mode compétitif : aucune progression persistante obligatoire entre les parties. Le héros recommence chaque partie au niveau initial. Cela évite que le jeu devienne un RPG avec un avantage permanent entre joueurs.

---

## 10. Duel de héros

Le duel est une interaction volontaire entre deux héros. Il ne constitue pas une condition obligatoire de victoire.

### 10.1 Déclenchement

Un héros peut cibler un héros adverse et lancer un défi. Le duel est une proposition de risque, pas une obligation permanente.

**Conditions de base proposées :**

- les deux héros sont vivants ;
- les deux héros sont dans une zone compatible avec le duel ;
- un héros en récupération ne peut pas défier ;
- un héros déjà en duel ne peut pas être défié ;
- certaines capacités ou certains événements peuvent empêcher temporairement un duel.

**L'adversaire peut accepter ou refuser *(décision D80)*.** La réponse est définitive : pas de retrait après coup.

- **Fenêtre de réponse :** ~5 à 8 s (indicatif), signalée par une alerte sonore et visuelle.
- **Accepter :** le duel commence immédiatement ; le cercle de duel (D18) se pose entre les deux héros.
- **Refuser, ou ne pas répondre à temps :** c'est un **refus**, avec ses coûts (*Hésitation*, Honneur pour l'Aube). Ignorer un défi n'est jamais gratuit.
- **Celui qui défie ne peut pas annuler** son défi : son temps de recharge de défi est consommé.

**Coût du refus *(décision D16)* : une petite pénalité de moral.**

- Refuser applique *Hésitation* aux unités de celui qui refuse, autour de son héros : un malus de moral **nettement plus faible que *Démoralisé*** (§ 10.5), pendant environ 20 à 30 s.
- La hiérarchie est : **duel perdu > refus > duel gagné**. On refuse quand on pense perdre le duel, on accepte quand on pense le gagner ou quand le malus tomberait au pire moment.
- **Anti-harcèlement :** chaque héros a un temps de recharge sur ses défis (indicatif : 2 à 3 min), pour qu'on ne puisse pas cumuler les refus imposés à l'adversaire.
- **Selon la faction *(D81)* :** une seule exception : l'Ordre de l'Aube perd en plus de l'Honneur s'il refuse (D76). Toutes les autres factions, Cercle de l'Ombre compris, paient la même *Hésitation*.

**Bénéfice propre au duel *(décision D35)* :** basculement de moral commun + effet de victoire propre à chaque faction (§ 10.5).

### 10.2 Arène

Le duel a lieu directement sur la carte, pour éviter d'en faire un mini-jeu séparé qui déconnecte le joueur du RTS. La caméra peut mettre l'affrontement en avant, mais la partie continue autour.

**Duel protégé *(décision D18)* :**

- **Pendant le duel, les deux héros ne peuvent être ciblés que l'un par l'autre.** Rien d'autre ne peut les toucher : unités, tours, autres héros, capacités, événements mondiaux.
- La bataille continue normalement autour ; les auras des deux héros restent actives (D14).
- Un **cercle de duel** visible, de rayon court, matérialise l'affrontement.
- **Parties à plusieurs :** un héros ne peut être que dans un seul duel à la fois, et aucun troisième héros ne peut intervenir. Pas de 2 contre 1.

**Zone compatible :**

- les deux héros sont à courte distance l'un de l'autre et visibles l'un pour l'autre ;
- **pas de duel dans le rayon de défense d'un centre principal**, pour qu'on ne puisse pas dueller sous ses propres tours.

### 10.3 Contrôle et kit de duel *(décision D14)*

**Duel semi-automatique tactique.**

- **Le héros attaque automatiquement.** Le joueur gère :
  - une **posture** : offensive / défensive / équilibrée ;
  - **4 à 6 capacités de duel** avec temps de recharge.
- **Les attaques fortes sont annoncées** par une animation de préparation (~0,5 à 1 s). Cela permet de lire et de contrer malgré la latence du réseau (modèle client-serveur, D29).
- **Arbitrage d'attention :** le joueur peut détourner les yeux pour gérer sa base pendant le duel, mais un duel suivi attentivement se gagne plus souvent.
- **L'IA** joue les duels avec les mêmes temps de réaction qu'un humain, sans réflexes surhumains.

**Pendant un duel :**

- les **capacités RTS actives** du héros sont remplacées par son **kit de duel** ;
- son **aura de commandement reste active**, puisqu'il est physiquement sur la carte.

**Contenu du kit** (catégories) : attaques, défenses, mobilité, contrôle, ultime de duel. Le kit de duel est séparé du kit RTS.

**Déblocage *(décision D43)* : kit de duel complet dès le niveau 1, sauf l'ultime de duel, débloqué au niveau 10.**

- Du niveau 1 au niveau 9, le niveau donne un avantage de **statistiques** en duel, pas d'**outils** : un héros en retard de quelques niveaux garde une vraie chance s'il lit mieux son adversaire.
- Au niveau 10, l'ultime de duel s'ajoute : c'est la récompense de fin de progression, en miroir de l'ultime RTS.
- Le prototype (niveaux 1 à 6) teste donc le kit de duel complet, sans ultime de duel.
- ⚠️ **Point de vigilance :** un héros de niveau 10 a un outil de plus que son adversaire. L'ultime de duel doit rester puissant mais lisible et contrable (annoncé, comme les attaques fortes), pour ne pas décider seul l'issue d'un duel. À régler en test.

**Le duel teste :** timing, lecture de l'adversaire, gestion des temps de recharge et de la posture, connaissance du héros.

**Exemple sur les héros du prototype :** le Paladin-Commandant (riposte) gagne en posture défensive en contrant les attaques annoncées. Le Seigneur Damné (agression, drain de vie) gagne en maintenant la pression sans s'exposer aux contres. Hors prototype, la Voix du Cercle de l'Ombre (feintes, D45) ferme un triangle : la feinte bat la riposte, l'agression bat la feinte, la riposte bat l'agression. Les autres styles : montée en Chaleur pour le Champion Héritier (D47), duel élémentaire pour le Seigneur-Dragon (D49), *La Règle* (l'inébranlable) pour le Grand Maître des Templiers (D99).

### 10.4 Durée

Objectif de prototype : **30 à 90 secondes**.

Assez long pour permettre la lecture de l'adversaire, l'usage de plusieurs capacités, des décisions de positionnement et un retournement potentiel. Pas assez long pour déconnecter les joueurs du RTS : la partie continue pendant le duel.

Un duel trop court devient une animation de burst ; trop long, un mini-jeu qui prend le dessus sur le RTS. La durée réelle sera validée par les tests.

**Temps écoulé (D18) :** si aucun héros n'est tombé à la durée maximale, le duel se termine **sans vainqueur**. Les deux héros survivent, sans récompense, et le temps de recharge des défis s'applique.

### 10.5 Issue

- **Victoire :** héros adverse mort, temps de récupération déclenché, le gagnant reste actif.
- **Défaite :** héros mort, récupération, progression conservée.

**Récompense propre au duel *(décisions D15, D17, D35)* :**

La valeur principale du duel vient de D17 : c'est la manière normale et la plus sûre d'abattre un héros. La récompense ci-dessous s'y ajoute.

- **Basculement de moral (commun à toutes les factions).** À la fin du duel, les unités alliées du vainqueur dans un rayon autour du duel reçoivent *Triomphe* (bonus de moral et d'attaque), et celles du perdant subissent *Démoralisé*. Durée indicative : 30 à 45 s.
- **Bonus d'XP de duel** modéré pour le vainqueur.
- **Effet de victoire propre à la faction** du vainqueur, un seul par faction :
  - **Ordre de l'Aube :** gros gain d'Honneur et recharge immédiate de la *Bannière de l'Aube*.
  - **Légions Noires :** un *Champion damné* temporaire (~45 s) surgit du corps du héros vaincu, et la *Moisson* est portée immédiatement à son maximum de cumuls. Le Champion damné est un serviteur générique, pas le héros vaincu : la règle « jamais les héros » (D37, D57) est respectée.
  - **Cercle de l'Ombre *(D45)* :** *Voix usurpée*, la Voix prend l'aura du héros vaincu pour sa propre armée (~45 s).
  - **Héritiers du Feu *(D47)* :** *Armes chauffées à blanc*, les alliés proches du Champion Héritier ont des armes incandescentes (~45 s).
  - **Templiers *(D100)* :** *La gloire du Temple*, un rang de gloire pour un contingent de la croisade (permanent, plafonné ; sans or).
  - **Enfants du Dragon *(D49)* :** *Furie du wyrm*, le dragon fond sur l'armée adverse proche et combat seul (~30 s, plafonné), puis le Seigneur-Dragon peut remonter sans délai de bascule.
- **Garde-fous :** effets temporaires (§ 10.6) et de valeur comparable d'une faction à l'autre ; le Champion damné a une puissance plafonnée et une durée courte, pour ne pas faire boule de neige. Valeurs à régler en test.
- **Technique :** l'effet de victoire est un `GameplayEffect` (ou une capacité) référencé dans le HeroData de chaque faction, appliqué par le serveur à la fin du duel.
- **Usage visé :** le duel est un outil tactique, idéalement lancé juste avant ou pendant une bataille.
- **Duel comme protection** *(acquis avec D18)* : pendant le duel, rien d'autre ne peut cibler les héros, ce qui permet à un joueur en infériorité militaire de régler le sort des héros sans exposer son armée.

**Pistes écartées pour le moment** (à reconsidérer plus tard ou pour un mode dédié) : enjeux déclarés par celui qui défie (lourd en interface et pour l'IA), trophée personnel sur le vainqueur, basculement d'un point stratégique, récupération allongée après une mort en duel.

### 10.6 Principes de balance

Le duel doit être : risqué, lisible, court, spectaculaire, optionnel, important.

Le joueur doit toujours peser : « Est-ce que je peux me permettre de risquer mon héros maintenant ? »

Le duel ne doit pas devenir une obligation de build. Le joueur doit pouvoir gagner sans duel. *Exception assumée et mesurée :* l'Aube perd de l'Honneur en refusant (D76) ; cette perte reste modérée.

Le gain principal doit être **temporaire** (« j'ai gagné un avantage maintenant ») et non permanent (« j'ai supprimé définitivement ton développement »).

---

## 11. Technologies

Chaque technologie peut avoir trois conditions :

1. niveau minimum du héros ;
2. bâtiment requis ;
3. coût en ressources.

> **Exemple :** Héros niveau 3 + Grande Forge construite + 300 bois / 200 or = technologie disponible.

Cela évite le schéma : « Je monte d'âge et tout est instantanément débloqué. »

**Catégories :**

| Catégorie | Contenu |
|---|---|
| Économie | collecte, stockage, agriculture, commerce, rendement |
| Militaire | dégâts, armure, portée, vitesse, capacités (statistiques et visuel uniquement, jamais de remplacement d'unité, D38) ; déblocage de nouvelles unités |
| Infrastructure | bâtiments, défenses, production, population |
| Héros | capacités, commandement, récupération, duel |
| Faction | mécanique signature, unités spécialisées, technologies propres |

**Mécanisme de déblocage :** un palier ouvre des **bâtiments** (tronc commun + spécialisation, D07). Chaque **technologie** demande un palier minimum, un bâtiment et des ressources.

**Répartition *(décision D27)* : arbre commun, technologies de faction ciblées.**

- **~70 à 80 % commun à toutes les factions :** forge (dégâts et armures par catégorie d'unités), économie (collecte, rendement), infrastructures (solidité des murs, population), siège.
- **~20 à 30 % propres à la faction :** mécanique signature (Honneur, cadavres…), unités emblématiques et variantes, **spécialisations de palier** (D07), technologies héros.
- L'identité se concentre là où elle se voit. Un joueur qui connaît une faction se repère dans les autres (pilier 1). C'est le modèle d'AoE4 : forge commune, monuments et technologies uniques.

---

## 12. Événements mondiaux

### 12.1 Exemple : le volcan

| Étape | Contenu |
|---|---|
| **Télégraphie** | grondements, fumée, fissures, température, alertes visuelles et sonores |
| **Préparation** | évacuer, déplacer les travailleurs, abandonner une position, renforcer une autre zone, attaquer pendant que l'adversaire évacue |
| **Éruption** | lave, feu, destruction, zone dangereuse, routes coupées |
| **Après** | terrain modifié, nouvelles ressources éventuelles, nouvelles routes, zone temporairement inaccessible |

### 12.2 Principes

Les événements doivent :

- être lisibles ;
- être prévisibles suffisamment tôt pour permettre de réagir (préavis de 60 à 90 s, D28) ;
- créer un contre-jeu ;
- être stratégiquement exploitables ;
- ne pas décider aléatoirement du vainqueur.

Le joueur doit pouvoir dire « J'ai perdu cette position parce que j'ai mal anticipé le volcan », et non « J'ai perdu parce que le jeu a choisi de détruire ma base ».

### 12.2 bis Déclenchement et placement *(décision D28)*

**Programmés par la carte, et déclenchables par les joueurs.**

- **Sites d'événement définis par la carte**, sur des **zones neutres et symétriques**, jamais sur une base de départ. L'équité est garantie sur les cartes de 2 à 8 joueurs.
- **Fenêtre de déclenchement** propre à chaque site (exemple : éruption entre la 14e et la 18e minute).
- **Préavis visible de tous** : 60 à 90 s de phase d'alerte (état `Warning` du World State Manager).
- **Fréquence :** 1 à 2 événements majeurs par partie (§ 4).
- **Déclenchement volontaire :** certains sites peuvent être réveillés plus tôt par un joueur qui remplit un **objectif** (par exemple, contrôler un autel ancien pendant 60 s). Le préavis reste obligatoire, même dans ce cas.
- Les Enfants du Dragon pourraient avoir un accès privilégié à ces déclenchements (climat, éléments).

**Calendrier : les événements mondiaux arrivent après le prototype.** Le prototype se concentre sur le socle RTS, le héros, le duel et les fortifications. L'architecture (World State Manager, § 16.2) est tout de même prévue dès le départ, pour accueillir les événements sans refonte.

### 12.3 Événements futurs

Volcan, tempête, séisme, inondation, incendie, invasion de monstres, ouverture d'une faille, apparition d'une créature, corruption magique, changement temporaire du climat.

---

## 13. Factions

### 13.1 Principes

Chaque faction possède une **identité mécanique principale** :

| Faction | Identité | Point fort (timing) |
|---|---|---|
| Ordre de l'Aube | discipline / honneur / défense : **tenir et protéger** (D87) | forte défense, milieu de partie |
| Légions Noires | mort / corruption / recyclage des pertes | attrition, combats prolongés |
| Enfants du Dragon | adaptation / éléments / créatures | adaptation, contrôle |
| Cercle de l'Ombre | information / subversion / pièges | information, harcèlement |
| Héritiers du Feu | élite / qualité / faible population | armée réduite, puissance individuelle |
| Templiers *(D87)* | croisade / guerre sainte : **partir en croisade** | offensive (⚠️ à préciser) |

**Question de validation pour chaque faction :** « Qu'est-ce que cette faction fait que les cinq autres ne font pas ? »

Une mécanique de faction doit modifier les décisions du joueur, pas seulement ajouter des statistiques ou une ressource.

L'équilibrage ne cherche pas à rendre les factions identiques : chacune est forte à un moment différent. Les timings diffèrent, mais les opportunités de victoire restent comparables.

**Structure d'identité *(décision D31)* : une mécanique signature forte, plus une particularité économique légère.**

- **Mécanique signature :** le cœur de la faction. Elle modifie les décisions militaires et stratégiques.
- **Particularité économique :** **une seule règle**, légère, souvent portée par une variante (D20) ou un bâtiment. Ce n'est ni un système, ni une ressource (§ 5.2).

| Faction | Mécanique signature | Particularité économique *(D39, D66 à D73)* |
|---|---|---|
| Ordre de l'Aube | Honneur | **Sanctuaire** *(D67)* : les fermes dans son rayon produisent plus (économie compacte et défendable) |
| Légions Noires | Cadavres / Nécroflux | **Zombie** *(D66)* : travailleur bon marché, sans nourriture, plus lent ; unités des Légions moins chères |
| Enfants du Dragon | Adaptation élémentaire | **Économie élémentaire** *(D68, D72)* : chaque *Nid élémentaire* bonifie la collecte de la ressource de son élément ; effets cumulés, élément changeable gratuitement avec délai |
| Cercle de l'Ombre | Subversion | **Marché noir** *(D69)* : échange de ressources au marché à un meilleur taux |
| Héritiers du Feu | Élite globale *(D39)* : toute la faction fait la même chose, en mieux | peu de travailleurs, mais chacun collecte nettement plus |
| Templiers *(D87)* | **Appel à la croisade** *(D88)* : une cible annoncée, une armée de croisade composée selon les commanderies | **Frère convers** *(D91)* : travailleur qui se défend bien et collecte plus vite pendant une croisade ; une croisade réussie rapporte énormément d'or |

**Prototype :** Honneur + règle économique de l'Aube ; cadavres + Zombie pour les Légions.

### 13.2 Ordre de l'Aube

**Thème :** chevaliers, lumière, honneur, discipline, défense.
**Style :** solide, méthodique, défensif, efficace en combat organisé, capable de tenir des positions.

| Unité | Rôle | Traits |
|---|---|---|
| **Chevalier Vertueux** | frontline / tank | armure lourde, épée et bouclier, aura défensive, *Défi du Chevalier* (voir ci-dessous) |
| **Moine Lumineux** | soutien | soins, purification, suppression de malédictions, sceaux lumineux ralentissant les ennemis |
| **Archer de l'Aube** | distance / contrôle | arc long sacré, tirs précis, flèches de lumière, zones réduisant l'efficacité offensive ennemie |

**Variante du socle *(D79)* :** le **Hallebardier** remplace le Lancier (anti-cavalerie, bonus contre l'infanterie lourde, plus cher ; § 7.1).

**Défi du Chevalier *(décision D19)* : le Chevalier prend les coups à la place des autres.**

- Dans une zone autour du Chevalier, **une partie des dégâts subis par les unités alliées est redirigée vers lui**. Une version active peut rediriger **la totalité** des dégâts pendant quelques secondes.
- Effet complémentaire : les unités ennemies proches sont **provoquées** et l'attaquent en priorité.
- **Les héros ne sont pas concernés**, ni comme protégés ni comme provoqués. Le héros est déjà résistant aux troupes (D17), et le protéger en plus le rendrait quasi intuable. Le duel reste aussi lisible.
- **Synergie :** les dégâts se concentrent sur l'unité la plus blindée, que le Moine Lumineux soigne.
- **Contre-jeu :** unités perforantes ou anti-lourdes, dégâts de zone (qui frappent aussi le Chevalier, plusieurs fois), et éliminer le Moine en priorité.
- À régler en test : le pourcentage redirigé (passif partiel contre actif total), le rayon de la zone et le temps de recharge.

**Économie *(D67)* : le Sanctuaire.** Les fermes situées dans le rayon d'un Sanctuaire produisent plus de nourriture. L'Aube a intérêt à bâtir une économie **compacte et défendable**, fidèle à son identité. ⚠️ Vigilance : une expansion lointaine lui rapporte moins qu'aux autres factions ; à surveiller sur les grandes cartes.

**Mécanique signature : HONNEUR *(décision D75)* — une monnaie de pouvoirs, façon livre de pouvoirs de BFME.** Propre à l'Aube : aucune autre faction n'a d'Honneur.

- **Sources *(D76)* : des actes honorables précis.**
  - **Défendre :** ennemis tués près de ses bâtiments, murs ou points stratégiques.
  - **Protéger :** dégâts absorbés par le *Défi du Chevalier*, soins du Moine Lumineux.
  - **Tenir** un point stratégique dans la durée.
  - **Accepter un duel :** petit gain, quelle que soit l'issue.
  - **Gagner un duel :** gros gain (D35).
- **Perte *(D76)* : refuser un duel coûte de l'Honneur**, en plus de l'*Hésitation* (D16). C'est le code d'honneur de la faction.
- ⚠️ **Vigilance (§ 10.6) :** l'Aube est la seule faction pour qui refuser coûte plus cher ; l'adversaire peut la défier au pire moment. La perte doit rester **modérée** (inférieure au gain d'une acceptation, par exemple), pour que refuser reste un choix viable. Exception assumée au principe « le duel ne doit pas devenir une obligation ». À régler en test.
- **Dépense :** l'Honneur se **dépense** dans des **pouvoirs de faction**. La décision porte sur le moment : économiser ou dépenser maintenant.
- **Pouvoirs *(D85)* : un par palier, coût en Honneur croissant** (valeurs à régler en test).

  | Palier | Pouvoir | Rôle |
  |---|---|---|
  | 0 | *Lumière sacrée* : soin et purification des effets de moral négatifs (D74) sur une zone | soutien : sauver une ligne qui tient |
  | 1 | *Rempart béni* : un segment de mur devient invulnérable quelques secondes | défense : tenir une brèche face au siège |
  | 2 | *Renforts de l'Aube* : une escouade de Chevaliers Vertueux arrive au centre principal (comptée dans la population) | renfort |
  | 3 | *Jugement* : frappe de lumière annoncée sur une zone, contrable | offensive |

  Le prototype (paliers 0 à 2) teste les trois premiers. Un livre à choix façon BFME (un pouvoir parmi deux par palier) reste une piste si l'Aube manque de variété.
- **Pas une ressource économique** (§ 5.2) : l'Honneur ne se récolte pas et ne paie ni unités, ni bâtiments, ni technologies.
- Interface : une barre d'Honneur et les pouvoirs dans le HUD de l'Aube. L'IA doit savoir quand dépenser.

**Héros *(décision D12)* : LE PALADIN-COMMANDANT** *(nom temporaire)*

| Combat personnel | Commandement | Valeur stratégique |
|---|---|---|
| ★★☆ | ★★★ | ★★☆ |

- **Rôle :** commandant défensif, multiplicateur d'une armée qui tient ses positions.
- **Kit RTS (pistes) :**
  - *Aura de l'Aube* : large aura défensive (armure, moral).
  - *Bannière de l'Aube* : plante un point de commandement fixe qui prolonge son aura dans une zone pendant qu'il se déplace ailleurs. Cela donne une réponse partielle au problème des fronts multiples.
  - *Serrez les rangs* : cri de guerre qui réduit les dégâts reçus et met les unités en formation défensive.
  - *Charge de l'Aube* *(D52)* : façon Gandalf et les Rohirrim à l'aube au Gouffre de Helm. Le Paladin mène une charge ; les cavaliers et fantassins proches le suivent avec un bonus d'impact, et les ennemis au point d'impact sont brièvement aveuglés. Tenir la ligne, puis contre-attaquer. Contre-jeu : charge visible au départ, Lanciers (D21), repli.
  - **Déblocage *(D42, D52)* :** niveau 1 : *Aura de l'Aube* + *Serrez les rangs* ; niveau 4 : *Bannière de l'Aube* (plus stratégique, utile quand l'armée se bat sur plusieurs fronts) ; niveau 7 (hors prototype) : *Charge de l'Aube* ; niveau 10 : *Dernier Rempart*.
- **Kit de duel :** style **défensif à riposte**. Blocages, contres et punition des erreurs de l'adversaire. Un duelliste patient, fidèle à la discipline de la faction.
- **Lien avec l'Honneur *(D76)* :** gagne de l'Honneur en *acceptant* les duels (quelle que soit l'issue), en les gagnant et en tenant des positions sous pression ; en perd en refusant.
- **Victoire en duel *(D35)* :** gros gain d'Honneur et recharge immédiate de la *Bannière de l'Aube*.
- **Capacité ultime (niveau 10) *(D36)* : *Dernier Rempart*.** Pendant ~10 s, les alliés dans une large zone autour du héros ne peuvent pas descendre sous 1 PV ; à la fin, ils récupèrent une partie des dégâts subis pendant l'effet. Contre : reculer et attendre la fin au lieu de frapper. Valeurs à régler en test.

### 13.3 Légions Noires

**Thème :** morts-vivants, nécromancie, sacrifice, terreur, corruption.
**Style :** attrition, affaiblissement, recyclage des pertes, pression persistante.

| Unité | Rôle | Traits |
|---|---|---|
| **Guerrier Damné** | frontline / berserker | massue ou hache, dégâts élevés, rage nécrotique, se consume au combat |
| **Nécromancien** | soutien / invocation | lève des Squelettes à partir des cadavres (D86), malédictions, drain de vie |
| **Spectre Assassin** | furtivité / DPS | dagues spectrales, dématérialisation, marquage de cibles, mobilité |

**Économie *(D66)* :** les unités des Légions **coûtent moins cher** que celles des autres factions (valeur à régler en test, en tenant compte de la réduction de l'*Ossuaire*). Leurs travailleurs, les Zombies, collectent plus lentement : la faction vit avec moins de revenu.

**Mécanique signature : CADAVRES *(décision D86)*.** Pas une ressource économique : une ressource de champ de bataille.

- **Cadavres au sol :** chaque unité morte (alliée ou ennemie) laisse un **cadavre** pendant un temps limité (~60 à 90 s, indicatif), avec un **plafond** de cadavres sur la carte pour la performance. Une unité relevée par *Relève impie* ne laisse pas de cadavre.
- **Le Nécromancien consomme un cadavre pour lever un Squelette :** unité **permanente**, faible, gratuite, qui **compte dans la population**. Les Légions bâtissent une armée durable à partir de l'attrition ; la population limite l'effet boule de neige.
- **Autres consommateurs :** *Sacrifice* du Seigneur Damné. L'Ossuaire compte les morts sans consommer les cadavres (D65).
- **Pas de déni :** aucune unité adverse ne peut détruire ou purifier les cadavres (le Moine Lumineux purifie les effets de moral, D74, pas les cadavres).
- **Rôles distincts :** le Seigneur crée des serviteurs **temporaires** sur le moment (*Relève impie*) ; le Nécromancien bâtit une armée **permanente**.
- Pistes pour plus tard : goules (amélioration du Squelette ou capacité), corruption de zone (D40).

**Héros *(décision D13)* : LE SEIGNEUR DAMNÉ** *(nom temporaire)*

| Combat personnel | Commandement | Valeur stratégique |
|---|---|---|
| ★★★ | ★★☆ | ★☆☆ |

- **Rôle :** combattant de première ligne qui se nourrit de l'attrition et pousse son armée à l'offensive.
- **Kit RTS (pistes) :**
  - *Aura de terreur* : réduit le moral et l'efficacité des ennemis proches.
  - *Moisson* : se renforce à chaque mort autour de lui, alliée ou ennemie (cumuls temporaires).
  - *Sacrifice* : consomme une unité alliée ou un cadavre pour se soigner, ou fait exploser un cadavre en dégâts de zone.
  - *Relève impie* *(D53, nom temporaire)* : **passif**. Chaque mort dans une zone autour du Seigneur a une **chance de se relever de son côté**. Seules les unités vivantes ou mortes-vivantes peuvent se relever : jamais le siège, ni les bâtiments. **Jamais les héros** *(D57, cohérent avec D37)*. **Unité relevée *(D54, D55)* :** l'unité se relève **sous sa forme de base** (un Arbalétrier mort se relève en Arbalétrier, sans les améliorations de son ancien propriétaire, avec un visuel mort-vivant), en **serviteur temporaire** (~30-45 s), hors population, nombre plafonné. Chance et plafond à régler en test.
  - **Unités emblématiques et d'élite *(D56)* :** elles se relèvent **telles quelles** (un Chevalier Vertueux en Chevalier Vertueux). **Pas de modèle mort-vivant par unité :** l'état « relevé » passe par des **FX et une teinte** communs (shader, particules), appliqués au modèle d'origine. ⚠️ Vigilance : face aux Héritiers du Feu, chaque serviteur relevé est une unité d'élite ; la chance et le plafond devront peut-être être pondérés par le coût de l'unité.
  - **Déblocage *(D42, D53)* :** niveau 1 : *Aura de terreur* + *Moisson* (son identité) ; niveau 4 : *Sacrifice* (plus de décisions, et plus de cadavres disponibles) ; niveau 7 (hors prototype) : *Relève impie* ; niveau 10 : *Grande Moisson* *(D57)*.
- **Kit de duel :** style **agressif à drain de vie**. Pression constante, soin en frappant, mais exposé aux ripostes.
- **Contraste avec le Paladin-Commandant :** l'agresseur contre le riposteur. Chaque duel entre les deux factions du prototype repose sur la lecture de l'adversaire : frapper ou laisser venir.
- **Lien avec les cadavres :** il est le premier consommateur de la mécanique de faction.
- **Pas de poudre *(D38, D59)* :** les Légions n'ont accès ni à l'Arquebusier ni au Canon. Réponse de fin de partie :
  - ***Cracheur de bile*** (palier 3, à la place du Canon) : engin de siège ; l'acide inflige des dégâts à l'impact, puis reste un court temps au sol et inflige des dégâts sur la durée.
  - ***Ossuaire*** *(D60, D65)* : bâtiment économique propre aux Légions, au palier 3. Il transforme les morts des batailles en **réduction du coût de production**. Plus les combats durent, plus les Légions produisent à bas prix (identité d'attrition).
    - **Collecte globale *(D65)* :** chaque mort sur la carte, alliée ou ennemie, remplit une **jauge plafonnée**. Le cadavre **reste sur le terrain** : aucune concurrence avec *Sacrifice*, le Nécromancien et *Relève impie*.
    - **Pas de rétroactivité :** une mort survenue sans Ossuaire construit, ou quand la jauge est pleine, n'est pas comptée. Chaque Ossuaire a sa propre capacité ; en construire plusieurs augmente le plafond total.
    - **Dépense automatique :** au clic de recrutement, la réduction s'applique d'elle-même, puisée dans la jauge. Aucune action du joueur.
    - ⚠️ Vigilance : l'Ossuaire ne répond pas directement aux armures lourdes ; en fin de partie, les Légions comptent sur l'Arbalétrier du socle et le *Cracheur de bile*.
- **Victoire en duel *(D35)* :** le corps du héros vaincu relève un *Champion damné* temporaire (~45 s, puissance plafonnée) et la *Moisson* est portée à son maximum.
- **Capacité ultime (niveau 10) *(D36, D55)* : *Grande Moisson*.** Zone annoncée ; les unités ennemies ordinaires sous ~25 % de PV y sont **exécutées**, la *Moisson* passe au maximum et le Seigneur se soigne à chaque exécution. Ces morts déclenchent *Relève impie* normalement : l'ultime nourrit le passif sans le dupliquer. Elle achève ce que la bataille a commencé (identité « attrition »). Contre : retirer ses unités blessées de la zone avant l'impact. Valeurs à régler en test.
  - *Remplace* *Marée des damnés* (D36), qui faisait doublon avec *Relève impie* (D54).

### 13.4 Enfants du Dragon

**Thème :** dragons, élémentalisme, créatures, adaptation.
**Style :** polyvalence, contrôle du terrain, choix d'élément, interactions avec les créatures.

| Unité | Rôle | Traits |
|---|---|---|
| **Champion Draconique** | frontline / dégâts | épée à deux mains, feu ou foudre (choisi par unité, D78), attaque en cône, forte présence au corps-à-corps |
| **Mage Élémentaire** | dégâts / contrôle à distance | feu, glace ou foudre (choisi par unité, D78), zones élémentaires, effets selon l'élément |
| **Dompteur de Bêtes** | hybride / soutien | arme courte, compagnon contrôlable (wyverne, dracogriffe), ordres attaquer / distraire / protéger |

**Économie *(D68, D72, D73)* : économie élémentaire.** Pas d'affinité de terrain (D40).

- **Le *Nid élémentaire* *(D72, nom temporaire)*** : bâtiment propre à la faction, réglé sur **un élément** (feu, glace ou foudre). Chaque Nid **bonifie la collecte de la ressource liée à son élément** (~ +10 % par Nid, valeur indicative).
- **Plusieurs Nids, effets cumulés :** le bonus s'additionne d'un Nid à l'autre (3 Nids en feu ≈ +30 % sur la ressource du feu). Le joueur peut **répartir** ses Nids entre plusieurs éléments ou tout miser sur un seul.
- **Changer l'élément d'un Nid est gratuit, mais pas immédiat** (façon citernes byzantines d'AoE4) : le nouveau réglage ne devient actif qu'après **X s** (à régler en test) ; **pendant ce délai, l'ancien réglage continue de produire**. La faction s'adapte sans trou de production, mais jamais instantanément.
- Les Nids sont des **cibles** : en raser un retire son bonus.
- L'élément des Nids ne touche que l'économie : l'élément du héros (*Souffle*, *Aura draconique*) et celui de chaque Mage Élémentaire restent indépendants.
- **Correspondance *(D73)* :** **feu → or**, **glace → pierre**, **foudre → bois** ; la **nourriture n'est jamais bonifiée** (pas de spam d'unités de base nourri par les Nids). Elle reprend les postures du duel (D49) : la glace défensive bâtit les fortifications, le feu offensif paie les unités avancées, la foudre rapide alimente production et expansion. La pierre reste à conquérir : le bonus multiplie une collecte existante, il faut toujours tenir les gisements.
- ⚠️ Vigilance : le cumul est sans plafond ; le coût d'un Nid (ou un plafond) devra empêcher qu'une ressource soit démultipliée. À régler en test.

**Mécanique signature : ADAPTATION ÉLÉMENTAIRE *(décision D78)* — l'élément du héros commande l'armée.** C'est le duel élémentaire (D49) transposé au RTS.

- **Élément actif du héros :** le Seigneur-Dragon a **un élément actif** (feu, glace ou foudre). Il en change **gratuitement, avec un délai** (même règle que les Nids, D72) : l'ancien élément reste actif pendant la transition.
- **Effet sur l'armée :** l'*Aura draconique* donne aux unités proches l'effet de cet élément (valeurs à régler en test) :
  - **feu** : brûlure, plus de dégâts ;
  - **glace** : ralentit les ennemis touchés, plus d'armure ;
  - **foudre** : plus de cadence, coups en chaîne.
- Le *Souffle* et la *Lame draconique* utilisent aussi cet élément. Pendant un duel, le kit de duel élémentaire (D49) prend le relais (D14) ; l'élément actif reprend à la sortie.
- **Unités emblématiques :** le Mage Élémentaire (feu, glace, foudre) et le Champion Draconique (feu, foudre) choisissent leur élément **unité par unité** (bouton, même délai). Micro réservé aux emblématiques.
- **Une seule règle pour toute la faction :** changer d'élément, avec un délai, sert à l'économie (Nids), à la bataille (héros, emblématiques) et au duel.
- Pas d'affinité de terrain (D40). Le lien avec le climat et les événements mondiaux (§ 12.2 bis) reste une piste d'après prototype.
- **Piste écartée pour l'instant :** réactions entre éléments (feu + glace = vapeur, etc.), possible plus tard en technologie de faction.

**Héros *(décision D41)* : LE SEIGNEUR-DRAGON** *(nom temporaire ; inspiré du Roi-Sorcier sur sa Bête ailée dans BFME)*

| Forme | Combat personnel | Commandement | Valeur stratégique |
|---|---|---|---|
| Monté (sur son dragon) | ★★★ | ☆☆☆ | ★★★ |
| À pied | ★★☆ | ★★★ | ★☆☆ |

- **Rôle :** héros à deux formes. La décision constante : harceler et dominer le ciel, ou descendre pour commander et dueller.
- **Bascule monté / à pied :** quelques secondes de vulnérabilité pendant la transition. Le dragon **grandit avec les niveaux du héros** (dragonnet au début, grand wyrm au niveau 10 ; visuel et statistiques, D38).
- **Monté (kit RTS, pistes) :** vole au-dessus du relief, rapide, grande vision.
  - *Souffle* : dégâts en cône, de l'élément du héros.
  - *Cri du wyrm* : peur brève sur une zone (limites de D37).
  - *Piqué* : zone annoncée par l'ombre du dragon (~1-2 s), puis impact ; esquivable.
  - **Contreparties :** pas d'aura de commandement ; **dégâts normaux reçus des tireurs** (exception à la résistance héroïque de D17 : les tireurs sont la réponse à la monture volante, comme dans BFME) et des tours.
- **À pied (kit RTS, pistes) :**
  - *Aura draconique* : aura de commandement ; l'armée proche prend l'effet de l'élément actif du héros (Adaptation élémentaire, D78).
  - *Lame draconique* : frappe de corps à corps de l'élément du héros.
- **Déblocage *(D42, D51)* : les deux formes dès le niveau 1.**

  | Niveau | À pied | Monté |
  |---|---|---|
  | 1 | bascule, *Aura draconique*, *Lame draconique* | bascule, *Souffle* |
  | 4 | — | *Piqué* |
  | 7 | — | *Cri du wyrm* |
  | 10 | *Appel de la Couvée* | *Appel de la Couvée* |

  - **Exception assumée au kit cible (§ 9.3) :** aura + 4 capacités au lieu de 3, justifiée par l'exclusivité des formes (jamais plus de 2-3 boutons actifs à la fois).
  - La décision « ciel ou sol » existe dès la première minute. Le harcèlement aérien de début de partie reste limité : dragonnet faible, pas d'aura en vol, dégâts normaux des tireurs.
- **Duel :** **toujours à pied.** Le dragon se pose ou s'éloigne. **Défi en vol *(D50)* : atterrissage de défi.**
  - Lancer ou accepter un défi en vol déclenche une **descente annoncée** du dragon vers le lieu du duel ; le héros saute à terre et le duel commence.
  - La bascule est **absorbée par le début du duel**, protégé (D18) : pas de vulnérabilité supplémentaire.
  - **Refuser en vol coûte l'*Hésitation*** comme au sol (D16) : le vol ne permet jamais de fuir un duel gratuitement.
  - En vol, la distance de défi (D18) se mesure depuis le sol, à la verticale du dragon.
- **Kit de duel *(D49)* : duel élémentaire.** Les 3 postures de D14 deviennent 3 éléments : **feu** = offensive (dégâts sur la durée), **glace** = défensive (ralentit, protège), **foudre** = équilibrée et rapide (interruptions). Le duel teste la lecture de la posture adverse et le changement d'élément au bon moment (court délai à chaque changement). C'est l'Adaptation élémentaire de la faction, appliquée au duel.
- **Capacité ultime (niveau 10) *(D36)* : *Appel de la Couvée*.** 2 à 3 dragons adultes descendent sur une zone annoncée pendant ~20 s (plafonnés, hors population).
- **Murs *(D64)* : survol libre.** Le dragon survole murs et remparts comme le reste du relief. Les bases se défendent par les tours et les tireurs postés sur les remparts (bonus de hauteur, D23). **En vol, il prend plus de dégâts à distance que le héros à pied** : dégâts normaux des tireurs et des tours, contre ×0,3 à pied (D17, D41). Le héros ne rasant pas une base (D17), le survol sert surtout à l'éclairage et au harcèlement, et coûte cher face à une base défendue.
- ⚠️ **Points de vigilance :** une couche de déplacement aérien pour un seul acteur ; lisibilité (le dragon ne doit pas masquer le champ de bataille).
- **Victoire en duel *(D35, D49)* : *Furie du wyrm*.** Le dragon fond sur l'armée adverse proche et **combat seul ~30 s** (puissance plafonnée, dégâts normaux des tireurs comme en forme montée). Ensuite, le héros peut **remonter sans délai de bascule**. Valeurs à régler en test.

### 13.5 Cercle de l'Ombre

**Thème :** assassins, espionnage, manipulation, ruse.
**Style :** information, harcèlement, pièges, attaques opportunistes, faible efficacité en combat frontal prolongé.

| Unité | Rôle | Traits |
|---|---|---|
| **Maître des Ombres** | assassin | camouflage, invisibilité temporaire, attaques éclair, forte mobilité |
| **Piégeur** | contrôle / soutien | arbalète, mines, filets, poison, pièges tactiques |
| **Illusionniste** | contrôle / confusion | copies illusoires, confusion (D37), peur ou retournement temporaire |

**Économie *(D69)* : le marché noir.** Le Cercle échange ses ressources au marché à un **meilleur taux** que les autres factions : une économie souple, qui s'adapte aux besoins du moment. Le marché est commun à toutes les factions (D70, § 5.5).

**Mécanique à prototyper : SUBVERSION.** Sabotage, fausses informations (objets du monde uniquement), confusion, contrôle temporaire, vision avancée. Limites fixées par D37 ci-dessous.

**Limites de la perte de contrôle *(décision D37)* : effets courts, encadrés par des règles fixes, plus une conversion définitive réservée au héros.**

**Effets courts (unités du Cercle, dont l'Illusionniste) :**

- **Peur / fuite :** 2 à 4 s.
- **Confusion** (remplace la « perturbation des ordres ») : pendant 3 à 5 s, les unités touchées attaquent la cible la plus proche, quel que soit son camp. Les ordres en file et l'interface du joueur adverse ne sont jamais modifiés.
- **Retournement temporaire :** unités ordinaires uniquement (jamais héros, siège, bâtiments ni travailleurs), ~8 à 10 s, 1 à 3 unités par lanceur, avec un plafond de coût pour qu'on ne puisse pas retourner une unité d'élite des Héritiers du Feu.
- **Immunité après effet :** une unité qui vient de subir une perte de contrôle y est immunisée pendant ~10 à 15 s (pas d'enchaînement).
- **Héros :** jamais pris ni retournés ; ils ne subissent que les contrôles brefs, à durée réduite.
- **Lisibilité :** signal visuel et sonore clair sur chaque unité touchée.
- **Fausses informations :** autorisées si elles passent par des objets du monde (illusions, faux signaux sur la mini-carte) ; jamais en falsifiant l'interface adverse (ressources, population, etc.).

**Conversion définitive (façon conversion d'AoE) :**

- Réservée au **héros du Cercle de l'Ombre** : c'est sa capacité 3, *Serment de l'Ombre*, débloquée au niveau 7 (D44, voir ci-dessous).
- **Esquivable :** la conversion vise une zone annoncée et ne prend effet qu'après un délai de canalisation. Les unités qui sortent de la zone avant la fin y échappent ; interrompre le héros l'annule.
- **Cibles *(D63)* :** jamais les héros ni les bâtiments. **Le siège est convertible** (comme les moines d'AoE4) : c'est la réponse au siège d'une faction faible en combat frontal. **Pas d'exclusion d'élite**, mais un **plafond de coût total** : une unité des Héritiers, deux fois plus chère, compte double. Longue recharge.
- ⚠️ Vigilance : le siège, lent, sort difficilement de la zone de conversion. La canalisation et le plafond de coût devront être réglés en test pour que la conversion d'un Trébuchet, d'une Bombarde ou d'un *Cracheur de bile* reste une prise forte, pas une certitude.

**Technique :** tags GAS de contrôle (`State.CC.Fear`, `State.CC.Confused`, `State.CC.Charmed`…) et effet d'immunité temporaire appliqué à la fin de chaque perte de contrôle ; la conversion définitive change le propriétaire de l'unité côté serveur. Valeurs à régler en test.

**Héros *(décision D44)* : LA VOIX** *(nom temporaire ; inspirée de Saruman et de Langue de Serpent dans BFME)*

| Combat personnel | Commandement | Valeur stratégique |
|---|---|---|
| ★☆☆ | ★★☆ | ★★★ |

- **Rôle :** orateur corrupteur. Il ne gagne pas les batailles par la force, mais en retournant, figeant et trompant l'armée adverse.
- **Signature : les discours.** Ses capacités principales sont **canalisées en zone** : puissantes, mais annoncées et interruptibles. La Voix doit s'exposer pour parler, ce qui crée le risque et le contre-jeu.
- **Kit RTS (pistes, calendrier D42) :**
  - *Murmures* (aura, niveau 1) : réduit le moral des ennemis proches ; les alliés infligent plus de dégâts aux unités affaiblies ou sous contrôle.
  - *Mot d'arrêt* (niveau 1) : peur brève en zone (limites de D37).
  - *Mensonge* (niveau 4) : fait apparaître une fausse armée, visible sur la carte et la mini-carte (objet du monde, autorisé par D37).
  - *Serment de l'Ombre* (niveau 7, hors prototype) : la **conversion définitive** de D37. Zone annoncée, canalisation, 2 à 4 unités, plafond de coût, longue recharge ; esquivable en sortant de la zone, annulée si la Voix est interrompue.
- **Capacité ultime (niveau 10) *(D36)* : *Discours du Maître*.** Longue canalisation annoncée, puis confusion de masse sur une armée entière (durées de D37) et retournement temporaire des quelques unités les plus proches. Contre : l'interrompre pendant la canalisation, ou disperser l'armée. Valeurs à régler en test.
- **Contre-jeu général :** tireurs et charges pour interrompre les discours ; dispersion face aux zones ; éclairage pour démasquer les *Mensonges*.
- **Kit de duel *(D45)* :** style **à feintes**. Certaines attaques annoncées sont des feintes : l'annonce part, le coup ne vient pas. Chaque feinte porte un **indice subtil**, lisible par un joueur attentif (D14), et coûte un temps de recharge : pas de feinte en continu. Ses Combat ★☆☆ valent hors duel ; en duel, la lecture prime.
- **Triangle de duel avec le prototype :** la feinte bat la riposte du Paladin (qui se met en garde pour rien puis s'expose) ; l'agression du Seigneur Damné bat la feinte (pas le temps de l'installer) ; la riposte bat l'agression.
- **Victoire en duel *(D35, D45)* : *Voix usurpée*.** Pendant ~45 s, la Voix prend l'**aura du héros vaincu** et l'applique à sa propre armée. Technique : application du `GameplayEffect` d'aura tiré du `HeroData` du vaincu. Valeurs à régler en test.

### 13.6 Héritiers du Feu

**Thème :** élite, puissance, qualité plutôt que quantité. Leur feu est celui de la **forge et de la flamme sacrée** (artisanat, qualité des armes, héritage), pas un élément magique : c'est ce qui les distingue des Enfants du Dragon *(D38)*.
**Style :** très peu d'unités, chaque unité est précieuse, recrutement lent, forte dépendance à la micro et au positionnement, économie exigeante.

**Règle d'élite** (direction, pas formule définitive) : coût ≈ ×2, population ≈ ×2, efficacité ≈ ×2, recrutement plus long.

**Principes de balance :** équilibrer autour de la population, de l'économie, du temps de production, des pertes, de la mobilité et de la qualité des unités. Une unité 2× plus efficace n'est pas 2× meilleure partout : elle peut être 2× meilleure en combat frontal, mais moins nombreuse, plus lente à produire, plus chère, vulnérable au contrôle, incapable de couvrir plusieurs fronts.

**Faiblesse principale :** « Je ne peux pas être partout. »

**Signature *(décision D39)* : l'élite globale, sans mécanique supplémentaire.** Les Héritiers font tout simplement la même chose que les autres, mais en mieux : ils collectent plus vite, tirent plus vite, frappent plus fort. Ce ne sont que des statistiques, mais **toute la faction** est construite ainsi (travailleurs, socle, emblématiques, siège).

- **Pas de Ferveur, pas de Prestige, pas de vétérance.** L'identité est la plus simple à lire des 6 factions, et la plus facile à prendre en main.
- **Les décisions viennent de la rareté :** peu d'unités, chaque perte coûte cher, impossible de couvrir plusieurs fronts. C'est une exception assumée à la règle de D31 (une signature qui modifie les décisions) : la règle d'élite les modifie indirectement.
- Le ratio ≈ ×2 est une direction ; il peut varier selon les statistiques (collecte, cadence, dégâts) et sera réglé en test.

**Unités emblématiques *(décision D38)*** (noms et traits provisoires) :

| Unité | Rôle | Traits |
|---|---|---|
| **Gardien de la Forge** | frontline lourde | armure et bouclier massifs, capable de tenir seul une ligne ; répond à « je ne peux pas être partout » |
| **Lame Ardente** | DPS mobile | charge, arme incandescente qui inflige de la brûlure, exigeante en micro |
| **Prêtre de la Flamme** | soutien | bénit les armes, confère de la résistance, protège des unités précieuses ; pas de magie élémentaire |

**Poudre (D38) :** commune à toutes les factions au palier 3 (§ 7.1). Les Héritiers en ont leurs propres versions, forgées : la *Bombarde* à la place du Canon et l'*Arquebusier de la Forge* (D61), qui tire plus vite et plus loin. Ce sont leurs deux variantes du socle (D20).

**Héros *(décision D46)* : LE CHAMPION HÉRITIER** *(nom temporaire ; inspiré de Boromir et de Théoden en première ligne dans BFME)*

| Combat personnel | Commandement | Valeur stratégique |
|---|---|---|
| ★★★ | ★☆☆ | ★☆☆ |

- **Rôle :** duelliste de première ligne, porteur de l'arme forgée par sa lignée. Dans une armée de ~45-50 unités, ses ~5-6 unités de puissance offensive (D17) pèsent proportionnellement plus que chez les autres factions.
- **Signature : l'arme héritée et la *Chaleur*.** Chaque coup porté fait monter la *Chaleur* de l'arme ; le joueur la **libère** en frappes chargées. Rythme : accumuler, puis décharger au bon moment.
- **Kit RTS (pistes, calendrier D42) :**
  - Aura (niveau 1, piste) : *Exemple* — les alliés proches gagnent en moral et en cadence tant que le Champion combat.
  - Capacité 1 (niveau 1, piste) : *Frappe de forge* — libère la Chaleur en un coup puissant.
  - Capacité 2 (niveau 4) *(D48)* : *Transmission* — dépense la Chaleur pour embraser pendant quelques secondes les armes d'un groupe allié proche.
  - Capacité 3 (niveau 7, hors prototype) *(D48)* : *Cor de l'Héritage* (façon cor de Gondor de Boromir) — peur brève en zone (limites de D37) et regain de moral pour les alliés.
  - **La Chaleur devient une décision :** la dépenser pour soi (*Frappe de forge*) ou pour l'armée (*Transmission*). C'est ce qui distingue le Champion du Seigneur Damné, dont la Moisson ne fait que s'accumuler, et qui lui donne un peu de commandement. *Transmission* annonce aussi son effet de victoire (*Armes chauffées à blanc*).
- **Capacité ultime (niveau 10) *(D36)* : *Jugement de flamme*.** Bond sur une zone annoncée, puis impact de zone. Valeurs à régler en test.
- ⚠️ **Point de vigilance : proximité avec le Seigneur Damné** (même profil ★★★ en combat ; Chaleur proche de la Moisson). Distinction à tenir : la Moisson se nourrit des **morts autour** du Seigneur (attrition, soutien) ; la Chaleur ne vient que des **coups portés** par le Champion et se **dépense** (accumulation puis décharge). Thème forge et flamme sacrée, jamais nécromancie ni magie élémentaire (D38).
- **Kit de duel *(D47)* :** style **à montée en Chaleur**. La Chaleur monte à chaque coup porté ; le Champion la libère en **frappes chargées**, les attaques annoncées les plus puissantes du jeu, donc les plus lisibles et contrables. L'adversaire doit gagner tôt ou parer la décharge. Différence avec le Seigneur Damné : pression constante et soin en frappant pour l'un, accumulation puis décharge pour l'autre.
- **Victoire en duel *(D35, D47)* : *Armes chauffées à blanc*.** Pendant ~45 s, les alliés proches ont des armes incandescentes (bonus de dégâts et brûlure). L'effet profite à l'armée et compense son aura faible. Valeurs à régler en test.

---

### 13.7 Templiers *(décision D87)*

**Sixième faction, à part entière.** Ordre militaire et religieux, fondé entre autres sur l'**appel à la croisade**.

**Principe *(décision D93)* : une faction « lore accurate », quasiment sans fantasy.** Les Templiers sont l'ordre historique : pas de magie, pas de surnaturel, pas de créatures. Leur foi s'exprime par le **moral** (D74), la **discipline**, l'**organisation** (commanderies, or, frères convers) et des faits d'armes historiques. Dans un monde de dragons, de morts-vivants et de magie, c'est ce réalisme qui les rend reconnaissables (pilier 5). Tout élément de conception templier doit passer ce filtre.

**Distinction avec l'Ordre de l'Aube :** deux factions de chevaliers saints, deux doctrines. L'Aube **tient et protège** (défense, Honneur, remparts) ; les Templiers **partent en croisade** (offensive, guerre sainte). La différence doit se lire au rythme de jeu **et à l'œil** : silhouettes, couleurs et architecture nettement distinctes de l'Aube (⚠️ point de vigilance, pilier 5).

**Mécanique signature : L'APPEL À LA CROISADE *(décision D88)* — une cible, un enjeu.**

- Le joueur **désigne une cible** : bâtiment ou centre ennemi, ou point stratégique. La croisade est **annoncée à l'adversaire**, qui voit la cible.
- L'appel **réunit une armée de croisade** au centre principal ou à la citadelle la plus proche de la cible (D96), qui marche vers la cible. L'armée qui avance vers la cible reçoit un effet de moral (D74).
- **Succès** (cible détruite ou prise) : récompense, dont un **rang de gloire** (D90) ; autres récompenses (XP, recharge de l'appel) à préciser. **Échec** (délai écoulé) : contrecoup (par exemple *Désillusion*, malus de moral) et longue recharge.
- **Les commanderies définissent la croisade *(précision de l'utilisateur)* :** au fil de la partie, le Templier **débloque des commanderies** ; ce sont elles qui déterminent **la composition** de l'armée de croisade. Deux Templiers n'appellent pas la même croisade.
- Contre-jeu : défendre la cible, tenir jusqu'à la fin du délai, intercepter l'armée en marche.
- **Troupes de croisade *(D89)* : autonomes, hors population, éphémères.**
  - Elles **marchent au plus court vers la cible**, sans ordres possibles, et **disparaissent** à la victoire ou à l'échec de la croisade.
  - Le joueur accompagne la croisade avec sa vraie armée : c'est la combinaison des deux qui fait la force.
  - Contre-jeu lisible : colonne et chemin visibles ; embuscade, ralentissement par les murs, ou tenir jusqu'à la fin du délai.
  - Distinction avec l'Aube : l'Aube appelle des renforts qu'elle dirige (*Renforts de l'Aube*, D85) ; les Templiers déclenchent une croisade qu'ils ne retiennent plus.
  - ⚠️ À régler : **plafond** de troupes (elles dépassent la population de 175 et pèsent sur la cible de performance, D03) ; **comportement face aux murs** (passer par une porte ou attaquer le mur) ; la perte de contrôle est assumée, comme pour une invocation temporaire.
  - Piste : un talent ou une capacité du héros templier pour que les croisés le suivent.
- **Commanderies *(D90)* : les paliers donnent l'accès, les victoires donnent la gloire.**
  - **Palier 0 :** la croisade n'a qu'une base, par exemple des *Pèlerins armés* (masse légère).
  - **Paliers 1, 2 et 3 :** le Templier choisit **une commanderie parmi 2 ou 3**, définitivement. Chacune **ajoute un contingent** à la croisade. Pour les Templiers, **ce choix tient lieu de spécialisation de palier** (D07) : pas de double choix.
  - Exemples (noms et rôles provisoires) : commanderie **du Temple** (chevaliers lourds, choc) ; **de l'Hôpital** (frères hospitaliers, soin de la colonne) ; **des Arbalétriers** (tireurs anti-armure) ; **des Bâtisseurs** (engins de siège pour abattre la cible).
  - **Gloire :** chaque croisade **réussie** donne un **rang de gloire** à un contingent existant (plus de troupes, ou version vétérane). **Plafond** : ~2 rangs par contingent (indicatif). Un échec ne retire rien.
  - Une croisade de fin de partie compte jusqu'à 4 contingents (base + 3 commanderies) qui reflètent le build ; l'éclairage permet de la lire.

**Économie *(D91)* : les frères convers et l'or de la croisade.**

- **Frère convers** (variante du Paysan, D20) : il **se défend bien** (armure et attaque correctes), ce qui rend les raids coûteux pendant que l'armée part en croisade ; et **pendant une croisade, il collecte plus vite** (*Deus vult*, ~ +15 à 20 %, indicatif).
- **Une croisade réussie rapporte énormément d'or** *(précision de l'utilisateur)* : la victoire finance la suivante. Montant à régler en test (fixe ou selon la valeur de la cible).
- ⚠️ Vigilance : un succès rapporte à la fois un rang de gloire (D90) et beaucoup d'or ; à surveiller pour l'effet boule de neige (§ 14.2). Contre-jeu : défendre la cible ou tenir jusqu'à la fin du délai prive les Templiers des deux.

**Unités propres *(D92)* : un noyau fixe, plus des commanderies.**

| Unité | Disponibilité | Rôle | Traits (provisoires) |
|---|---|---|---|
| ***Chevalier du Temple*** | toujours | cavalerie lourde de choc | charge ; « on ne recule pas » : frappe plus fort quand il est en infériorité numérique autour de lui |
| ***Porte-gonfanon*** | toujours | soutien / étendard | porte le *Beauséant* : effet de moral (D74) pour les Templiers proches tant que l'étendard est levé ; s'il tombe, *Désarroi* (malus bref). Cible prioritaire pour l'adversaire |

- **Les autres unités templières viennent des commanderies (D90) :** chaque commanderie choisie **débloque une unité recrutable** et **ajoute le même type de troupe à la croisade**.
- **Réserve de commanderies** (~8, noms et unités provisoires ; 2 à 3 proposées à chaque palier) : du Temple (renforts lourds), de l'Hôpital (*Frère hospitalier*, soin), des Sergents (*Sergent du Temple*, infanterie robuste), de la Chapelle (*Frère chapelain*, ferveur au combat), des Zélotes (*Zélote*, infanterie fanatique, puissante mais fragile), des Arbalétriers (arbalétriers d'élite), des Bâtisseurs (engins de siège)… Chaque partie montre une armée templière différente.
- **Exception assumée** à « 3 emblématiques par faction » : 2 fixes + 3 débloquées par les commanderies (une par palier).
- **Écarté :** archer monté (*Turcopole*), à la demande de l'utilisateur.
- ⚠️ Vigilance : beaucoup d'unités à concevoir (~8 + 2), et à distinguer du socle commun et de l'Aube (Sergent ≠ Homme d'armes, Frère hospitalier ≠ Moine Lumineux).

**Citadelles templières *(D95)* :** bâtiment propre aux Templiers, **construit librement** (règles de construction de D22). C'est une **place forte** (solide, garnison) **et** elle a un **effet sur la civilisation** :
  - **Les citadelles nourrissent la croisade *(D96)* :** chaque citadelle **grossit chaque appel** (+X croisés par citadelle, plafonné ; valeurs à régler en test).
  - **Les croisades se rassemblent à la citadelle la plus proche de la cible** (à défaut, au centre principal, D88).
  - Raser une citadelle affaiblit toutes les croisades suivantes : c'est une cible stratégique claire pour l'adversaire.
  - ⚠️ Vigilance : le plafond de troupes de croisade (D89, performance) doit inclure le bonus des citadelles. Pistes écartées : citadelle fondée par une croisade réussie ; citadelle-trésor.

**Héros *(D94, D97, D98)* : LE GRAND MAÎTRE** *(nom temporaire)*

- ***La Règle du Temple*** (aura) : les Templiers proches gagnent de l'armure et sont immunisés à la peur (D37).
- ***Prendre la croix*** *(D97)* : les unités contrôlées dans une zone autour du héros **rejoignent la croisade en cours** ; elles deviennent autonomes, marchent sur la cible avec les bonus de croisade et partagent la gloire ; le joueur en reprend le contrôle à la fin de la croisade. Décision : renoncer au contrôle pour la puissance de la croisade.
- ***Compagnie franche*** : le héros **achète une compagnie de mercenaires** (sergents, arbalétriers…) qui arrive immédiatement près de lui ; dans la population, payée en or seulement.
- ***Sortie*** *(D97)* : la citadelle la plus proche du héros lance une sortie : sa garnison sort combattre ~30 s (troupes temporaires, hors population), puis rentre.
- **Capacité ultime (niveau 10) *(D98)* : *Deus lo vult !*** Une **grande zone** autour du héros (assez grande pour qu'aucun cavalier ne soit exclu pour quelques mètres) : pendant quelques secondes, **la cavalerie alliée gagne une charge non interruptible**, avec une **réduction des dégâts reçus pendant la charge**. Les Lanciers et Hallebardiers **n'annulent pas** cette charge ; les troupes traversées sont probablement **renversées** (proposition de l'utilisateur, à valider en test).
  - Annoncé (cri visible et sonore). Contre-jeu : se disperser, s'écarter de l'axe de charge ; durée courte et longue recharge (D36).
  - ⚠️ Vigilance : neutralise brièvement le contre anti-cavalerie (D21), à garder court. Marais (D33) : la charge y reste-t-elle impossible ? À trancher en test. Distinction avec la *Charge de l'Aube* (D52), capacité menée par le Paladin : *Deus lo vult !* rend toute la cavalerie d'une zone inarrêtable.

  | Niveau | Kit RTS |
  |---|---|
  | 1 | *La Règle du Temple* (aura) + *Prendre la croix* |
  | 4 | *Compagnie franche* |
  | 7 | *Sortie* |
  | 10 | *Deus lo vult !* |

- Écartés : *Solde*, *Mise à prix*, *Trésor du Temple*, *Rançon*, *La Brèche*. Piste pour l'arbre de talents : *Camp retranché* (palissade et pieux instantanés, ~60 s).
- **Kit de duel *(D99)* : style *La Règle*, l'inébranlable.** La Règle interdisait de fuir : le Grand Maître **ne peut être ni repoussé ni interrompu** par les attaques fortes adverses ; il encaisse et continue. **Plus ses PV baissent, plus il frappe fort** (comme le Chevalier du Temple). Le duel se joue sur l'usure : l'adversaire doit le finir vite ou ne pas le laisser au seuil dangereux. Distinction : le Paladin **pare** (riposte), le Grand Maître **encaisse** ; le Champion Héritier accumule par ses coups, le Grand Maître devient dangereux en en recevant.
- **Victoire en duel *(D100)* : *La gloire du Temple*.** Le duel gagné donne **un rang de gloire** à un contingent de la croisade, comme une croisade réussie (D90), **sans l'or** (D91). Effet permanent, mais borné par le plafond de gloire : exception mesurée au § 10.6 (gain temporaire).

**Arbre de talents *(D101)* : quatre familles**, **Croisade** (taille, vitesse et gloire des croisades, *Prendre la croix*), **Chevalerie** (combat, duel *La Règle*, Chevaliers du Temple, *Deus lo vult !*), **Trésor** (or des croisades, *Compagnie franche*, frères convers) et **Les Citadelles** (solidité, *Sortie*, *Camp retranché*). Contenu des talents à concevoir plus tard (D08).

- ⚠️ Vigilance (garde-fous de D08) : ~8 points répartis sur 4 familles, contre 3 dans l'exemple des Légions. « Même présentation pour toutes les factions » : soit tous les arbres passent à 4 familles, soit c'est une exception templière. À trancher à la conception des arbres.

---

## 14. Victoire, modes et momentum

### 14.1 Conditions de victoire

- **Mode standard :** destruction du centre principal adverse.
- **Autres conditions possibles :** élimination, reliques, objectifs stratégiques, domination territoriale, scénario.

Le duel de héros n'est jamais l'unique condition de victoire du mode standard. Un joueur doit pouvoir gagner grâce à son économie, son armée, son contrôle territorial, ses technologies, sa stratégie.

### 14.2 Snowball et comeback

Le jeu permet un avantage cumulatif sans transformer chaque avantage en victoire automatique.

Le but est **Avantage → pression → possibilité de victoire**, et non **Avantage → partie terminée**.

**Leviers de retour :** défense, fortifications, technologies spécialisées, contre-composition, raid économique, événement de carte, récupération du héros, contrôle d'une ressource stratégique, attaque d'une expansion ennemie.

### 14.3 Modes de jeu

| Mode | Description |
|---|---|
| **Standard** | 1v1, équipes ou chacun pour soi, jusqu'à 8 joueurs ; détruire le centre principal ou accomplir l'objectif de la carte |
| Domination | contrôle de points pendant une durée |
| Reliques | capturer et protéger des reliques |
| Catastrophe | événements mondiaux plus fréquents |
| Scénario | objectifs narratifs |

Les modes secondaires seront développés après validation du mode standard.

### 14.4 Cible joueurs et formats *(décision D01)*

**Conçu pour le multijoueur, prototypé en solo.**

- Les règles sont pensées dès le départ pour le **1v1 compétitif** : symétrie, lisibilité, pas d'aléatoire injuste. Le 1v1 sert de référence d'équilibrage.
- Le **prototype** se joue contre une IA simple ou en hotseat, pour tester le plaisir de jeu avant d'investir dans le réseau.
- **Cartes jusqu'à 8 joueurs**, dans n'importe quel mélange d'humains et d'IA : 1v1, équipes (2v2 à 4v4, équipes asymétriques), chacun pour soi.
- **Chaque emplacement peut être tenu par un humain ou une IA.**
- **Déconnexion *(décision D71)* : fenêtre de reconnexion.** Pendant ~2-3 min, une IA tient la position (défense, économie de base) et le joueur peut revenir et reprendre la main. Ensuite : en **1v1 classé**, défaite ; en **équipe**, l'IA continue la partie jusqu'au bout, pour ne pas laisser les alliés en infériorité.

**Conséquences de design :**

- **IA de premier ordre.** L'IA doit savoir jouer *tous* les systèmes : économie, armée, héros, accepter ou refuser un duel, réagir aux événements. Toute règle du jeu doit donc pouvoir être lue par une IA. Une mécanique que l'IA ne sait pas jouer est une mécanique à revoir.
- **Population et performance.** 8 joueurs × population maximale = nombre total d'unités simulées. Avec 175 par joueur (D03), cela fait environ 1 400 unités dans le pire cas.
- **Duels en partie à plusieurs.** Plusieurs duels peuvent avoir lieu en même temps. Aucune intervention extérieure n'est possible : le duel est protégé et il n'y a pas de 2 contre 1 (D18).
- **Événements mondiaux.** Ils restent équitables sur des cartes de 2 à 8 positions de départ grâce à des sites neutres et symétriques (D28).
- **Cartes.** Il faut des cartes symétriques pour chaque format (2, 4, 6, 8 joueurs), avec des ressources contestées entre chaque paire de voisins.

---

## 15. Interface

**HUD principal :** ressources, population, minimap, sélection, commandes, production, technologies, niveau du héros, XP du héros, état du héros, temps de récupération.

Le joueur doit comprendre immédiatement :

1. ce qu'il peut construire ;
2. ce qu'il peut produire ;
3. ce qu'il peut rechercher ;
4. ce que fait son héros ;
5. ce que fait l'ennemi.

---

## 16. Orientation technique

### 16.1 Unreal Engine + Gameplay Ability System

Architecture conceptuelle :
`HÉROS → Ability Sets RTS → Ability Sets Duel → Attributes → Gameplay Effects → Gameplay Tags`

**Exemples de Gameplay Tags :**

```
Hero.State.Alive        Hero.State.Dead        Hero.State.Respawning
Hero.Mode.RTS           Hero.Mode.Duel
Hero.Duel.InProgress    Hero.Duel.Protected
Civ.Tier.1              Civ.Tier.2             Civ.Tier.3
Army.Commanded
Faction.OrderOfDawn     Faction.BlackLegions
Event.Volcano.Active
```

**Niveau et paliers *(décision D34)* :**

- Le **niveau** et l'**XP** du héros sont des **attributs GAS** (`Level`, `XP`) dans l'`AttributeSet` du héros. Les capacités et effets qui dépendent du niveau les lisent, par exemple via une `CurveTable` indexée sur `Level`.
- Seuls les **3 paliers de civilisation** sont des tags (`Civ.Tier.1` à `Civ.Tier.3`), ajoutés une fois et jamais retirés.
- Les tags de palier sont portés par le **joueur / la civilisation**, pas par le héros. Ils restent valides pendant sa mort (D06).
- Les conditions de bâtiment et de technologie testent les tags de palier (par exemple : « requiert `Civ.Tier.2` »).

**Gameplay Effects :** bonus d'attaque, bonus d'armure, moral, ralentissement, malédiction, aura, récupération, dégâts de zone.

**Gameplay Cues :** duel, montée de niveau, mort, résurrection, éruption, zones élémentaires.

### 16.2 World State Manager

Système global qui gère les événements de carte : planifier, télégraphier, déclencher, appliquer les modifications, restaurer les zones temporaires, communiquer avec les systèmes de gameplay.

Exemple : `WorldEvent.Volcano` — états `Dormant → Warning → Eruption → Aftermath → Recovery`.

### 16.3 Données pilotées

Unités, technologies, héros et bâtiments sont pilotés par des données plutôt que codés individuellement.

Fiches : `HeroData`, `UnitData`, `BuildingData`, `TechnologyData`, `FactionData`, `WorldEventData`.

Avantages : équilibrage, tests, ajout de contenu, prototypage des factions.

### 16.4 Réseau, simulation et IA *(décision D29)*

Le multijoueur jusqu'à 8 joueurs, humains et IA mélangés, est une cible dès la conception (D01).

**Modèle retenu : client-serveur, avec des unités légères.**

- **Le serveur fait autorité.** Il peut être hébergé par un joueur, puis devenir un serveur dédié plus tard. L'IA tourne sur le serveur, ce qui facilite les emplacements IA et la reprise d'un joueur déconnecté (fenêtre de reconnexion, D71, § 14.4).
- **Unités ordinaires = entités légères**, pas des `ACharacter` :
  - simulées sur le serveur (piste : **Mass Entity** d'Unreal, ou un gestionnaire maison) ;
  - répliquées sous forme compacte (positions et états compressés), interpolées côté client ;
  - affichées en **instances** avec des animations optimisées (animation par textures de sommets, ou équivalent).
- **Héros, bâtiments et engins de siège = acteurs classiques avec GAS.** Ils sont peu nombreux : GAS reste utilisé là où il compte (capacités, duels, auras).
- **Le brouillard de guerre est appliqué par le serveur**, qui n'envoie à chaque client que ce qu'il voit. Cela protège contre la triche « maphack » et réduit la bande passante.
- **Cible de performance :** ~1 400 unités (8 × 175, D03).

**Conséquence sur le code :** l'ancien code hérité d'un autre projet (`AUnitBase : ACharacter` avec un `AAIController` par unité, et ses Blueprints) a été **supprimé le 2026-10-07**. Le système d'unités légères part de zéro ; les ordres de déplacement, la sélection et le combat de base seront construits dessus.

L'IA joue avec les mêmes règles et les mêmes informations qu'un joueur (brouillard de guerre compris), sauf dans les niveaux de difficulté explicitement « tricheurs ».

### 16.5 Navigation sur plusieurs niveaux

Les remparts praticables (D24) imposent une navigation à deux niveaux : le sol et le chemin de ronde des murs de pierre.

- Les segments de mur portent une **surface de navigation** sur leur sommet, reliée au sol par des liens de navigation (escaliers dans les tours et les portes).
- Cette navigation est **mise à jour dynamiquement** à la construction et à la destruction de chaque segment.
- Elle doit fonctionner avec les **unités légères** (D29), pas seulement avec les `ACharacter` d'Unreal.
- C'est un chantier technique prioritaire du prototype, à valider tôt : performance avec ~1 400 unités et des murs étendus.

---

## 17. Prototype minimum viable

Avant de créer les six factions complètes :

| Élément | Quantité |
|---|---|
| Factions | 2 |
| Héros | 1 par faction |
| Unités | *proposition* : Paysan, Lancier, Homme d'armes, Archer, Arbalétrier, Cavalier léger (Cavalier lourd hors prototype, sauf besoin). Variantes : Zombie (Légions, D66), Hallebardier (Aube, D79) |
| Unités emblématiques *(D82)* | **au moins 2 par faction** : Chevalier Vertueux + Moine Lumineux (Aube) ; Guerrier Damné + Nécromancien (Légions). Ce n'est pas un plafond : d'autres peuvent s'ajouter si de bonnes idées apparaissent. Archer de l'Aube et Spectre Assassin prévus après. |
| Siège *(D77)* | **Bélier** (palier 1) + **Mangonneau** (palier 2) : de quoi tester toute la boucle fortifications / siège de D23 et D24 (portes, segments abattus, cible prioritaire sur rempart, chute des occupants) |
| Ressources | 4 |
| Centre principal | 1 |
| Bâtiments économiques *(D83, D84)* | maison, camps de collecte (bois, pierre, or), ferme, marché (D70) ; *Sanctuaire* pour l'Aube (D67) |
| Bâtiment militaire | 1 |
| Bâtiment technologique | 1 |
| Fortifications | murs de pierre praticables, porte, tour (D24) |
| Niveaux de héros | 1 à 6 (paliers 0, 1 et 2) |
| Duel | 1 |
| Système de mort / résurrection | 1 |
| Événement volcanique | ~~1~~ → **après le prototype** (D28) |
| Condition de victoire | 1 |

**Objectif :** tester si le jeu est amusant AVANT d'investir dans le contenu.

**Factions du prototype *(décision D11)* : Ordre de l'Aube et Légions Noires.** Contraste maximal (défense, discipline et soin contre attrition, invocations et malédictions). L'Honneur de l'Ordre sert à tester les règles du duel ; les cadavres des Légions testent une mécanique de faction sans ressource spéciale.

### 17.1 Tests prioritaires

1. **Héros :** est-il important sans être obligatoire ?
2. **Mort :** est-elle suffisamment punitive ?
3. **Résurrection :** le délai crée-t-il une fenêtre intéressante ?
4. **Duel :** apporte-t-il une vraie décision ou devient-il une obligation ?
5. **Progression :** le niveau donne-t-il une sensation de progression sans ressembler à un système d'âges ?
6. **Économie :** les ressources créent-elles suffisamment de décisions ?
7. **Armée :** les combats restent-ils intéressants sans le héros ?
8. **Volcan** *(après le prototype)* **:** l'événement crée-t-il une opportunité plutôt qu'une frustration ?
9. **Snowball :** un avantage est-il puissant sans devenir irréversible ?
10. **Factions :** chaque faction offre-t-elle une façon réellement différente de jouer ?

### 17.2 Roadmap de conception

1. Prototype RTS minimal.
2. Économie + bâtiments + production.
3. Combat + contre-unités.
4. Héros + XP + niveaux.
5. Mort + résurrection.
6. Duel.
7. Technologies liées au niveau du héros.
8. Premier événement mondial (après validation du prototype).
9. Deuxième faction complète.
10. Tests de matchup.
11. Six factions.
12. Polish, UI, effets, audio, contenu.

---

## 18. Risques de design

| # | Risque | Solution |
|---|---|---|
| 1 | **Héros trop puissant** : s'il tue trop d'unités seul, le jeu devient un MOBA avec une base RTS. | Le héros amplifie une armée plutôt que la remplacer. Puissance offensive plafonnée à ~5-6 unités standard ; sa force est défensive (résistance héroïque, D17), pas destructrice. |
| 2 | **Duel obligatoire** : si perdre un duel signifie perdre la partie, tous les joueurs seront forcés d'y participer. | Le duel crée une fenêtre de puissance, pas une condition de victoire. |
| 3 | **Progression trop proche d'un système d'âges** : si chaque niveau débloque exactement une nouvelle génération d'unités. | Progression du héros distincte des paliers (D04) ; chaque palier = tronc commun + spécialisation choisie (D07) ; bâtiments, technologies et choix du joueur déterminent le développement. |
| 4 | **Héros trop facile à remplacer** : une résurrection trop rapide ôte toute valeur à la mort du héros. | Créer une vraie fenêtre de vulnérabilité. |
| 5 | **Mort trop punitive** : une récupération trop longue rend la partie frustrante. | Tester plusieurs durées et mesurer leur impact sur le taux de retour. |
| 6 | **Événements aléatoires frustrants** : une éruption qui détruit une base sans possibilité de réaction. | Télégraphie + préparation + contre-jeu. |
| 7 | **Factions trop complexes** : cinq ressources et quinze systèmes uniques par faction. | Une mécanique signature forte par faction. |

---

## 19. Critères de succès

Le jeu est réussi si :

1. un joueur comprend les bases en quelques minutes ;
2. la profondeur apparaît progressivement ;
3. le héros est important mais pas indispensable ;
4. la mort du héros est stressante sans être catastrophique ;
5. les duels créent des moments mémorables ;
6. les factions se jouent réellement différemment ;
7. l'économie reste aussi importante que l'armée ;
8. les événements de carte créent des décisions ;
9. une partie peut être retournée sans que le comeback soit automatique ;
10. le joueur termine une partie avec l'impression d'avoir vécu une histoire.

---

## 20. Questions ouvertes

Classées par ordre de résolution : les premières conditionnent les suivantes.

**Cadre**
- ~~**Q01** — Cible joueurs~~ → **tranchée (D01)**.
- ~~**Q02** — Durée cible d'une partie 1v1~~ → **tranchée (D02)**.
- ~~**Q03** — Population maximale~~ → **tranchée (D03)**.

**Héros et progression**
- **Q04** — ~~Nombre de niveaux~~ (tranché, D04) ; reste la courbe d'XP par niveau, à régler en test.
- ~~**Q05** — Répartition de l'XP~~ → **tranchée (D05)**.
- ~~**Q06** — XP pendant la récupération~~ → **tranchée (D06)**.
- ~~**Q07** — Paliers, talents, équipement~~ → **tranchée (D07, D08, D09)** ; équipement supprimé (D102).
- ~~**Q08** — Héros des factions~~ → **tranchée (D10 à D57)** : ~~Nombre de héros par faction~~ (D10 : un seul) ; ~~héros du prototype~~ (D12, D13) ; ~~capacités ultimes~~ (D36) ; ~~héros des Enfants du Dragon~~ (D41) ; ~~déblocage des capacités RTS~~ (D42) ; ~~déblocage du kit de duel~~ (D43) ; ~~héros du Cercle de l'Ombre~~ (D44, D45) ; ~~héros des Héritiers du Feu~~ (D46) ; ~~duel du Seigneur-Dragon~~ (D49, D50) ; ~~répartition des capacités du Seigneur-Dragon~~ (D51) ; ~~déblocage et 3ᵉ capacité du Paladin~~ (D52) ; ~~3ᵉ capacité et ultime du Seigneur Damné~~ (D53 à D55) ; ~~unités emblématiques relevées~~ (D56) ; ~~ordre *Moisson* / *Sacrifice*, héros jamais relevés~~ (D57).

**Armée et base**
- ~~**Q09** — Socle et matrice de contres~~ → **tranchée (D20, D21, D62)** : pas d'unité anti-héros.
- ~~**Q10** — Modèle de construction~~ → **tranchée (D22)**.
- ~~**Q11** — Place du siège~~ → **tranchée (D23, D24)**.

**Commandement et mort**
- ~~**Q12** — Force du commandement~~ → **tranchée (D26)**.
- ~~**Q13** — Récupération et résurrection~~ → **tranchée (D25)**. Valeurs à régler en test.

**Duel**
- ~~**Q14** — Contrôle du duel~~ → **tranchée (D14)**.
- ~~**Q15** — Coût d'un refus de duel~~ → **tranchée (D16)**.
- ~~**Q16** — Bénéfice propre au duel~~ → **tranchée (D15, D17, D35)**. Effets de victoire des 5 factions tranchés (D35, D45, D47, D49).
- ~~**Q17** — Zone compatible et interventions~~ → **tranchée (D18)**.
- ~~**Q18** — Défi du Chevalier Vertueux~~ → **tranchée (D19)**.

**Technologies et événements**
- ~~**Q19** — Déblocage et répartition des technologies~~ → **tranchée (D07, D27)**.
- ~~**Q20** — Déclenchement des événements~~ → **tranchée (D28)**.

**Factions**
- ~~**Q21** — Identité mécanique et économique~~ → **tranchée (D31)**. Les règles économiques précises restent des pistes.
- ~~**Q22** — Limites des effets de perte de contrôle (Cercle de l'Ombre)~~ → **tranchée (D37, D44)** : conversion définitive = capacité 3 de la Voix, *Serment de l'Ombre*.
- ~~**Q23** — Unités des Héritiers du Feu ; Ferveur nécessaire ?~~ → **tranchée (D38, D39)**.
- ~~**Q24** — Factions du prototype~~ → **tranchée (D11)**.

**Carte**
- ~~**Q25** — Terrain et combat~~ → **tranchée (D23, D33, D40)**.
- ~~**Q26** — Monstres neutres~~ → **tranchée (D32)**.

**Technique et contrôle**
- ~~**Q27** — Contrôles et raccourcis~~ → **tranchée (D30)**.
- ~~**Q28** — Niveau du héros dans GAS~~ → **tranchée (D34)**.
- ~~**Q29** — Modèle réseau~~ → **tranchée (D29)**.
- ~~**Q30** — Résistance du héros face aux unités~~ → **tranchée (D17, D62)** : pas de contre-mesure « tueuse de héros ».

---

## 21. Journal des décisions

| # | Date | Question | Décision | Sections mises à jour |
|---|---|---|---|---|
| D01 | 2026-10-04 | Q01 — Cible joueurs | Conçu pour le multijoueur (1v1 comme référence d'équilibrage), prototypé en solo. Cartes jusqu'à 8 joueurs, chaque emplacement humain ou IA. | § 14.3, § 14.4, § 16.4, ajout de Q29 |
| D02 | 2026-10-05 | Q02 — Durée de partie | 25 à 35 min en 1v1 (référence d'équilibrage) ; parties en équipes plus longues. | § 4 (repères de rythme) |
| D03 | 2026-10-05 | Q03 — Population | 175 par joueur ; ~70-90 travailleurs, ~85-105 armée ; pire cas ~1 400 unités à 8 joueurs. | § 5.3, § 14.4 |
| D04 | 2026-10-05 | Q04 — Niveaux du héros | 10 niveaux de héros ; paliers de civilisation aux niveaux 3, 6, 9 ; ultime au niveau 10. Prototype : niveaux 1 à 6. | § 4, § 9.5, § 16.1, § 17 |
| D05 | 2026-10-05 | Q05 — Sources d'XP | Développement ~35 %, Territoire ~15 %, Combat + Commandement ~35 %, Exploration + Événements ~15 %. Commander en combat rapporte de l'XP. Rattrapage selon l'écart de niveau entre héros. | § 9.4 |
| D06 | 2026-10-05 | Q06 — XP pendant la mort | Les sources liées à la civilisation continuent ; combat et commandement s'arrêtent. Montée de niveau possible pendant la mort. | § 9.4, § 9.8 |
| D07 | 2026-10-05 | Q07 (partie 1) — Ouverture d'un palier | Tronc commun garanti (dont réponses aux contres) + choix définitif d'une spécialisation parmi 2. | § 9.5, § 18 |
| D08 | 2026-10-05 | Q07 (partie 2) — Arbre de talents | Arbre entièrement propre à chaque faction. Garde-fous : mêmes points et même présentation ; au moins un build commandement, un build duel, une voie stratégique. | § 9.6 |
| D09 | 2026-10-05 | Q07 (partie 3) — Équipement *(supprimé, D102)* | Forgé par la civilisation : 3 emplacements (arme, armure, relique), objets achetés en bâtiment, choix exclusifs, conservé à la mort. Reliques uniques de carte hors prototype. | § 8.3, § 9.7, § 11 |
| D10 | 2026-10-05 | Q08 (partie 1) — Héros par faction | Un héros unique par faction ; extensible plus tard via HeroData. | § 9 |
| D11 | 2026-10-05 | Q24 — Factions du prototype | Ordre de l'Aube et Légions Noires. | § 9.6, § 17 |
| D12 | 2026-10-05 | Q08 (partie 2) — Héros de l'Ordre de l'Aube | Paladin-Commandant : commandement ★★★ ; Aura, Bannière, Serrez les rangs ; duel défensif à riposte ; Honneur gagné en acceptant les duels. | § 13.2 |
| D13 | 2026-10-05 | Q08 (partie 3) — Héros des Légions Noires | Seigneur Damné : combat ★★★ ; Aura de terreur, Moisson, Sacrifice ; duel agressif à drain de vie. | § 9, § 13.3 |
| D14 | 2026-10-05 | Q14 — Contrôle du duel | Semi-automatique tactique : attaque automatique, posture + 4 à 6 capacités, attaques fortes annoncées. Pendant un duel, capacités RTS remplacées par le kit de duel ; aura maintenue. | § 10.3 |
| D15 | 2026-10-05 | Q16 — Récompense du duel *(provisoire)* | Base : basculement de moral (Triomphe / Démoralisé, 30-45 s) + XP de duel. **À retravailler** : jugé insuffisant. | § 10.5 |
| D16 | 2026-10-05 | Q15 — Coût du refus | *Hésitation* (petit malus de moral, 20-30 s) ; temps de recharge des défis 2-3 min ; modulé par faction (Aube perd de l'Honneur). | § 10.1 |
| D17 | 2026-10-05 | Q30 — Résistance héroïque | Héros ≈ 5-6 unités standard en puissance offensive (valeur absolue, toutes factions) ; dégâts reçus des troupes fortement réduits (~×0,3) ; dégâts normaux des héros, tours, siège et contre-mesures dédiées *(contre-mesures abandonnées, D62)* ; hors duel, héros contre héros = usure, les compétences de duel font la différence. | § 9.1 bis, § 18 |
| D18 | 2026-10-05 | Q17 — Autour du duel | Duel protégé : seuls les deux duellistes peuvent se toucher ; cercle de duel ; un seul duel par héros, pas de 2 contre 1 ; zone : courte distance, visibles, hors rayon d'un centre principal ; temps écoulé = pas de vainqueur. | § 10.2, § 10.4, § 10.5, § 14.4 |
| D19 | 2026-10-05 | Q18 — Défi du Chevalier Vertueux | Redirection vers le Chevalier d'une partie des dégâts subis par les alliés dans une zone (totalité en version active, quelques secondes) + provocation des ennemis proches. Héros exclus. Pas de duel unité contre héros. | § 13.2 |
| D20 | 2026-10-05 | Q09 (partie 1) — Socle d'unités | Socle commun à toutes les factions (Paysan, Homme d'armes, Piquier, Archer, Cavalier, siège, anti-héros *(retiré, D62)*). Variantes rares : 1 faction (2 max) par rôle, 1-2 variantes max par faction (ex. Zombie des Légions, Hallebardier). Emblématiques en plus du socle. Héritiers : règle d'élite sur tout le socle. | § 7.1 |
| D21 | 2026-10-05 | Q09 (partie 2) — Matrice de contres | Modèle AoE4 : bonus par catégorie d'armure. Socle étendu : Lancier, Homme d'armes, Archer, Arbalétrier, Cavalier léger, Cavalier lourd. Arbalétrier proposé comme anti-héros *(écarté, D62)*. | § 7.1, § 7.2, § 17 |
| D22 | 2026-10-05 | Q10 — Modèle de construction | Libre façon AoE / Stronghold (vision requise, murs libres) ; pas de construction près d'un centre principal ennemi ; territoire = points stratégiques et centres secondaires. | § 6 |
| D23 | 2026-10-05 | Q11 — Siège | Siège léger : Bélier (palier 1), Mangonneau (palier 2), Trébuchet (palier 3) ; fortifications solides. Bonus de hauteur pour les tireurs plus hauts que leur cible (collines, tours, remparts). | § 6.1, § 6.2, § 7.1, § 8.1 |
| D24 | 2026-10-05 | Remparts | Murs de pierre praticables dès le prototype (accès par tours et portes ; murs de bois non praticables). Échelles, douves, huile hors périmètre. | § 6.2, § 16.5, § 17 |
| D25 | 2026-10-05 | Q13 — Récupération du héros | Durée selon le niveau (~40 s à ~110 s) ; **pas de rachat** (le temps ne se réduit pas en payant) ; prix de résurrection obligatoire en fin de délai (base selon le niveau), dégressif jusqu'à zéro si le joueur ne paie pas ; réapparition au centre principal ou secondaire ; pas d'aggravation. | § 4, § 9.8 |
| D26 | 2026-10-05 | Q12 — Force du commandement | Aura moyenne : +10-15 % sur 1-2 statistiques (selon la faction), jusqu'à +20-25 % en fin de partie ; rayon d'un groupe de bataille (~20-30 unités). Héros ≈ 20 unités au total. | § 9.2 |
| D27 | 2026-10-05 | Q19 — Technologies | Arbre ~70-80 % commun (forge, économie, infrastructures, siège) + ~20-30 % propre à la faction (signature, emblématiques, spécialisations, héros). | § 11 |
| D28 | 2026-10-05 | Q20 — Événements mondiaux | Sites définis par la carte (neutres, symétriques), fenêtre de déclenchement, préavis de 60-90 s, 1-2 par partie ; déclenchables plus tôt par objectif. **Tous les événements arrivent après le prototype** (architecture prévue dès le départ). | § 12.2, § 14.4, § 17 |
| D29 | 2026-10-05 | Q29 — Modèle réseau | Client-serveur avec serveur faisant autorité ; unités ordinaires = entités légères (Mass Entity ou équivalent), réplication compacte, rendu en instances ; héros, bâtiments et siège = acteurs GAS ; brouillard de guerre appliqué par le serveur. Remplacer `AUnitBase : ACharacter`. | § 16.4, § 16.5 |
| D30 | 2026-10-05 | Q27 — Contrôles | Standard AoE4 (CTRL+n assigner, n sélectionner, n×2 centrer, MAJ+n ajouter, CTRL+MAJ+n retirer), F1 bâtiments militaires, F2 économiques, F3 technologiques, F4 héros, grille AZERTY de raccourcis, touche de défi ; tout est reconfigurable. | § 7.3 |
| D31 | 2026-10-05 | Q21 — Identité des factions | Une mécanique signature forte + une particularité économique légère (une règle, pas de ressource) par faction. | § 13.1 |
| D32 | 2026-10-05 | Q26 — Monstres neutres | Présence légère (quelques camps symétriques gardant gisement, relique ou passage), sans réapparition, XP modérée ; après le prototype. | § 8.4 |
| D33 | 2026-10-05 | Q25 — Terrain | Forêt (vision, couvert), marais (anti-charge), gué, route, hauteur, zones sacrées et corrompues. Points de vigilance : symétrie des cartes, affinités pour toutes les factions, valeurs modérées. | § 8.1 |
| D34 | 2026-10-05 | Q28 — Niveau dans GAS | Attributs `Level` et `XP` sur le héros ; tags `Civ.Tier.1-3` portés par le joueur, jamais retirés ; conditions de déblocage sur les tags de palier. | § 16.1 |
| D35 | 2026-10-05 | Q16 — Récompense du duel | Basculement de moral commun (D15) + XP de duel + **un effet de victoire propre à chaque faction**. Aube : gros gain d'Honneur, recharge immédiate de la Bannière. Légions : Champion damné temporaire (~45 s, plafonné) relevé du corps du vaincu, Moisson au maximum. Effets temporaires, de valeur comparable. Enjeux déclarés écartés pour le moment. | § 10.1, § 10.5, § 13.2, § 13.3 |
| D36 | 2026-10-05 | Q08 (partie 4) — Capacités ultimes | Capacité de bataille active du kit RTS : zone, courte durée, recharge ~3-4 min, annoncée et contrable ; pas de transformation ni d'effet global. Paladin : *Dernier Rempart* (~10 s, alliés pas sous 1 PV, puis soin partiel). Seigneur : *Marée des damnés* (morts récents relevés, ~30 s, plafond ~15-20, hors population) *(remplacée par *Grande Moisson*, D55)*. Ultime de duel séparé, non lié au niveau 10 *(révisé par D43 : débloqué au niveau 10)*. Hors prototype. | § 9, § 9.5, § 10.3, § 13.2, § 13.3 |
| D37 | 2026-10-05 | Q22 — Perte de contrôle (Cercle de l'Ombre) | Effets courts et encadrés : peur 2-4 s ; confusion 3-5 s (remplace la perturbation des ordres, jamais d'action sur les ordres ni l'interface adverses) ; retournement temporaire d'unités ordinaires, ~8-10 s, 1-3 unités, plafond de coût ; immunité ~10-15 s après chaque effet ; héros jamais pris ; fausses informations via des objets du monde uniquement. **Plus** une conversion définitive façon AoE, réservée au héros du Cercle (compétence ou ultime), esquivable en sortant de la zone avant la fin de la canalisation. | § 13.5 |
| D38 | 2026-10-05 | Q23 (partie 1) — Unités des Héritiers du Feu | Thème : feu de la forge et flamme sacrée (pas élémentaire). Emblématiques : Gardien de la Forge, Lame Ardente, Prêtre de la Flamme. **Poudre commune à toutes les factions, sauf contre-ordre** : la technologie commune « Armes à poudre » (palier 3) débloque l'Arquebusier et le Canon. Exceptions : Légions Noires sans poudre (réponse équivalente à définir) ; Héritiers avec leurs variantes (Bombarde à la place du Canon, variante de l'Arquebusier). **Règle générale :** une unité n'est jamais remplacée en cours de partie ; les améliorations ne touchent que les statistiques et le visuel ; la nouveauté passe par le déblocage de nouvelles unités. | § 6.1, § 7.1, § 11, § 13.3, § 13.6 |
| D39 | 2026-10-05 | Q23 (partie 2) — Ferveur / Prestige | Pas de mécanique supplémentaire : la signature des Héritiers est l'élite globale. Toute la faction fait la même chose en mieux (collecte, cadence, dégâts) ; ce ne sont que des statistiques, appliquées à tout. Les décisions viennent de la rareté des unités (exception assumée à D31). | § 13.1, § 13.6 |
| D40 | 2026-10-05 | Q25 (suite) — Zones alignées | Cartes compétitives neutres : aucune zone sacrée ou corrompue posée par la carte ; ces zones ne sont créées que par des capacités de faction (temporaires, visibles, contrables). Zones de carte possibles hors compétitif et en scénario. Pas d'affinité de terrain par faction. | § 8.1 |
| D41 | 2026-10-05 | Q08 (partie 5) — Héros des Enfants du Dragon | **Seigneur-Dragon**, façon Roi-Sorcier de BFME : bascule monté / à pied. Monté : vol, *Souffle*, *Cri du wyrm*, *Piqué* annoncé par l'ombre ; pas d'aura, dégâts normaux des tireurs et des tours. À pied : aura élémentaire, *Lame draconique*, duel (toujours à pied). Le dragon grandit avec les niveaux. Ultime : *Appel de la Couvée* (2-3 dragons, ~20 s). | § 9, § 13.4 |
| D42 | 2026-10-06 | Q08 (partie 6) — Déblocage des capacités RTS | Progressif façon BFME, en décalé des paliers : aura + capacité 1 au niveau 1, capacité 2 au niveau 4, capacité 3 au niveau 7, ultime au niveau 10. Kit cible : aura + 3 capacités + ultime. Prototype (niveaux 1 à 6) : aura + 2 capacités. Bascule du Seigneur-Dragon dès le niveau 1. | § 9.3, § 9.5, § 13.2, § 13.3, § 13.4 |
| D43 | 2026-10-06 | Q08 (partie 7) — Déblocage du kit de duel | Kit de duel complet dès le niveau 1, **sauf l'ultime de duel, débloqué au niveau 10** (révise D36 sur ce point). Point de vigilance : l'ultime de duel doit rester lisible et contrable. | § 9.5, § 10.3, § 21 (D36) |
| D44 | 2026-10-06 | Q08 (partie 8) — Héros du Cercle de l'Ombre | **La Voix**, façon Saruman et Langue de Serpent : orateur corrupteur (★☆☆ / ★★☆ / ★★★), capacités canalisées en zone, annoncées et interruptibles. Aura *Murmures* ; *Mot d'arrêt* (niv. 1) ; *Mensonge* (fausse armée, niv. 4) ; *Serment de l'Ombre* = conversion définitive de D37 (niv. 7). Ultime : *Discours du Maître* (confusion de masse + retournement temporaire). | § 9, § 13.5, § 20 (Q22) |
| D45 | 2026-10-06 | Q08 (partie 9) — Duel de la Voix | Style **à feintes** : attaques annoncées parfois feintes, avec indice subtil et coût en recharge ; triangle feinte > riposte > agression > feinte. Victoire en duel : ***Voix usurpée***, la Voix prend l'aura du héros vaincu pour son armée (~45 s). | § 10.3, § 10.5, § 13.5 |
| D46 | 2026-10-06 | Q08 (partie 10) — Héros des Héritiers du Feu | **Champion Héritier**, façon Boromir / Théoden : duelliste de première ligne (★★★ / ★☆☆ / ★☆☆). Arme héritée qui accumule de la *Chaleur* à chaque coup, libérée en frappes chargées. Ultime : *Jugement de flamme* (bond, impact de zone). Vigilance : se distinguer du Seigneur Damné (Chaleur = coups portés et dépense, Moisson = morts autour). | § 9, § 13.6 |
| D47 | 2026-10-06 | Q08 (partie 11) — Duel du Champion Héritier | Style **à montée en Chaleur** : accumulation à chaque coup, décharge en frappes chargées annoncées (les plus fortes du jeu, donc contrables). Victoire en duel : ***Armes chauffées à blanc***, armes incandescentes pour les alliés proches (~45 s). | § 10.5, § 13.6 |
| D48 | 2026-10-06 | Q08 (partie 12) — Capacités 2 et 3 du Champion Héritier | **Chaleur partagée** : *Transmission* (niv. 4, dépense la Chaleur pour embraser les armes d'un groupe allié) ; *Cor de l'Héritage* (niv. 7, peur brève en zone et moral allié). La Chaleur se dépense pour soi ou pour l'armée. | § 13.6 |
| D49 | 2026-10-06 | Q08 (partie 13) — Duel du Seigneur-Dragon | **Duel élémentaire** : les 3 postures deviennent feu (offensive), glace (défensive), foudre (équilibrée) ; changement d'élément avec court délai. Victoire en duel : ***Furie du wyrm***, le dragon combat seul ~30 s (plafonné), puis remontée sans délai de bascule. | § 10.3, § 10.5, § 13.4, § 20 (Q16) |
| D50 | 2026-10-06 | Q08 (partie 14) — Défi en vol | **Atterrissage de défi** : lancer ou accepter un défi en vol fait descendre le dragon (annoncé) ; la bascule est absorbée par le début du duel protégé ; refuser en vol coûte l'*Hésitation*. Distance de défi mesurée depuis le sol, à la verticale du dragon. | § 13.4 |
| D51 | 2026-10-06 | Q08 (partie 15) — Déblocage du Seigneur-Dragon | Deux formes dès le niveau 1. Niv. 1 : bascule, *Aura draconique*, *Lame draconique* (à pied), *Souffle* (monté) ; niv. 4 : *Piqué* ; niv. 7 : *Cri du wyrm* ; niv. 10 : *Appel de la Couvée* (deux formes). Exception assumée au kit cible : aura + 4 capacités, formes exclusives. | § 9.3, § 13.4 |
| D52 | 2026-10-06 | Q08 (partie 16) — Déblocage du Paladin-Commandant | Niv. 1 : *Aura de l'Aube* + *Serrez les rangs* ; niv. 4 : *Bannière de l'Aube* ; niv. 7 : ***Charge de l'Aube*** (charge menée par le Paladin, bonus d'impact des alliés qui le suivent, aveuglement bref au point d'impact) ; niv. 10 : *Dernier Rempart*. | § 13.2 |
| D53 | 2026-10-06 | Q08 (partie 17) — 3ᵉ capacité du Seigneur Damné | Proposition de l'utilisateur : ***Relève impie*** (nom temporaire), passif au niveau 7. Chaque mort dans une zone autour du Seigneur a une chance de se relever de son côté ; unités vivantes ou mortes-vivantes uniquement (pas de siège ni de bâtiment). Détails (forme, durée, chance, plafond) à préciser *(précisés par D54 à D57)*. | § 13.3 |
| D54 | 2026-10-06 | *Relève impie* — unité relevée | **Serviteur temporaire** (~30-45 s), hors population, plafonné. Conséquence relevée par l'utilisateur : l'ultime *Marée des damnés* fait doublon et est à revoir (conversion permanente, éventuellement d'unités vivantes, ou autre ultime). | § 13.3 |
| D55 | 2026-10-06 | Ultime du Seigneur Damné ; forme de *Relève impie* | Ultime : ***Grande Moisson*** (exécution des ennemis ordinaires sous ~25 % de PV dans une zone annoncée, *Moisson* au maximum, soin par exécution) ; remplace *Marée des damnés*. *Relève impie* : l'unité se relève sous sa forme de base (un Arbalétrier en Arbalétrier), en serviteur temporaire. | § 13.3 |
| D56 | 2026-10-06 | *Relève impie* — unités emblématiques et d'élite | Elles se relèvent **telles quelles**. Pas de version mort-vivante de chaque unité : FX et teinte communs appliqués au modèle d'origine. Vigilance : serviteurs d'élite face aux Héritiers du Feu. | § 13.3 |
| D57 | 2026-10-06 | Seigneur Damné — derniers points | Déblocage : *Aura de terreur* + *Moisson* au niveau 1, *Sacrifice* au niveau 4. *Relève impie* ne relève jamais les héros. | § 13.3 |
| D58 | 2026-10-06 | D38 (suite) — Arquebusier dans la matrice | **Tireur lourd de fin de partie** (façon Handcannoneer d'AoE4) : gros dégâts, ignore une partie de l'armure sans bonus de catégorie, recharge lente, courte portée, cher en or ; vulnérable à la cavalerie et aux Archers. Résistance héroïque normale : l'Arbalétrier reste l'anti-héros du socle *(révisé, D62 : pas d'anti-héros)*. | § 7.1, § 7.2 |
| D59 | 2026-10-06 | D38 (suite) — Légions sans poudre | ***Cracheur de bile*** : **engin de siège** à la place du Canon (palier 3) ; acide = dégâts à l'impact + flaque au sol, dégâts sur la durée, courte durée. Seconde variante des Légions (D20 respecté). Pas d'équivalent de l'Arquebusier : **un bâtiment propre en plus** à la place (à définir) *(l'Ossuaire, D60)*. *Catapulte à cadavres* écartée. | § 6.1, § 7.1, § 13.3 |
| D60 | 2026-10-06 | D59 (suite) — Bâtiment des Légions | ***Ossuaire*** : bâtiment économique (palier 3) qui transforme les cadavres des batailles en réduction du coût ou du temps de production. Récupération des cadavres à définir *(collecte globale, D65)*. Vigilance : pas de réponse directe aux armures lourdes. | § 7.1, § 13.3 |
| D61 | 2026-10-06 | D38 (suite) — Arquebusier des Héritiers | Réponse de l'utilisateur : l'***Arquebusier de la Forge*** (nom temporaire) **tire plus vite et plus loin** que l'Arquebusier (cohérent avec D39 : même chose, en mieux). Vigilance : l'Archer ne le dépasse plus en portée. | § 7.1, § 13.6 |
| D62 | 2026-10-06 | Contre-mesures anti-héros (D17, Q09, Q30) | **Aucune unité anti-héros**, ni commune ni de faction. La résistance héroïque (×0,3) s'applique à toutes les troupes ; l'Arbalétrier et l'Arquebusier font beaucoup de dégâts de base, donc un grand nombre blesse un héros. Révise D17 (contre-mesures dédiées) et D21 (Arbalétrier anti-héros). | § 7.1, § 7.2, § 9.1 bis, § 20 |
| D63 | 2026-10-06 | Conversion de la Voix — cibles | Jamais les héros ni les bâtiments ; **siège convertible** (façon moines d'AoE4) ; pas d'exclusion d'élite, **plafond de coût total** (une unité des Héritiers compte double). Vigilance : conversion du siège à régler en test. | § 13.5 |
| D64 | 2026-10-06 | Seigneur-Dragon et murs | **Survol libre** des murs ; défense par tours et tireurs sur les remparts. En vol, le dragon prend plus de dégâts à distance que le héros à pied (dégâts normaux contre ×0,3, confirmé). | § 13.4 |
| D65 | 2026-10-06 | Ossuaire — collecte | **Collecte globale** : chaque mort sur la carte remplit une jauge plafonnée, le cadavre reste sur le terrain. **Pas de rétroactivité** (pas d'Ossuaire ou jauge pleine = mort non comptée) ; capacité par Ossuaire. **Dépense automatique** : réduction appliquée au clic de recrutement. | § 13.3 |
| D66 | 2026-10-06 | Zombie — particularité économique | **Piste actuelle confirmée** : moins cher, sans nourriture, collecte plus lente, 1 place de population. Pas de handicap de population, car **les unités des Légions coûtent moins cher** : moins de revenu nécessaire, donc moins de travailleurs. | § 7.1, § 13.1, § 13.3 |
| D67 | 2026-10-06 | Économie de l'Ordre de l'Aube | **Sanctuaire** (piste confirmée) : les fermes dans son rayon produisent plus. Économie compacte et défendable. Vigilance : expansions lointaines moins rentables. | § 13.1, § 13.2 |
| D68 | 2026-10-06 | Économie des Enfants du Dragon | **Économie élémentaire** : l'élément choisi bonifie la collecte d'une ressource ; changer d'élément rééquilibre l'économie. Remplace la piste « collecte selon le terrain » (contraire à D40). Correspondance élément → ressource à définir *(précisée par D72, D73)*. | § 13.1, § 13.4 |
| D69 | 2026-10-06 | Économie du Cercle de l'Ombre | **Marché noir** : le Cercle échange ses ressources au marché à un meilleur taux. Remplace la piste du pillage. Existence d'un marché à préciser *(marché commun, D70)*. | § 13.1, § 13.5 |
| D70 | 2026-10-06 | Marché | **Marché commun à toutes les factions**, façon AoE : achat et vente, taux évolutifs ; le Cercle y a un meilleur taux (D69). | § 5.5, § 13.5 |
| D71 | 2026-10-06 | Déconnexion d'un joueur | **Fenêtre de reconnexion** (~2-3 min) tenue par une IA, retour possible ; ensuite défaite en 1v1 classé, IA jusqu'au bout en équipe. | § 14.4, § 16.4 |
| D72 | 2026-10-06 | Élément de faction des Enfants du Dragon | **Option A, élargie** : l'élément est porté par un bâtiment, le *Nid élémentaire* (nom temporaire). **Plusieurs Nids possibles**, chacun réglé sur un élément ; bonus de collecte **cumulés** (~ +10 % par Nid sur la ressource de son élément). Changement d'élément **gratuit mais différé** (façon citernes byzantines d'AoE4) : actif après X s, l'ancien réglage tourne pendant le délai. L'élément du héros reste indépendant. Règle le « coût d'un changement d'élément » laissé ouvert par D68. | § 13.1, § 13.4 |
| D73 | 2026-10-06 | Correspondance élément → ressource (Enfants du Dragon) | **Feu → or, glace → pierre, foudre → bois ; nourriture jamais bonifiée.** Cohérent avec les postures du duel (D49) et lisible par l'adversaire. Clôt le reliquat de D68. | § 13.4 |
| D75 | 2026-10-06 | Cohérence — L'Honneur | **Monnaie de pouvoirs, façon livre de pouvoirs de BFME, propre à l'Aube.** L'Honneur se gagne par des actions honorables et se dépense dans 3 à 4 pouvoirs de faction débloqués avec les paliers (pistes : *Renforts de l'Aube*, *Lumière sacrée*, *Rempart béni*, *Jugement*). Pas une ressource économique. Sources de gain et de perte à préciser *(D76)*. | § 13.2 |
| D76 | 2026-10-06 | Cohérence — Sources d'Honneur | **Actes honorables précis** : défendre (ennemis tués près de ses bâtiments, murs, points), protéger (*Défi du Chevalier*, soins du Moine), tenir un point, accepter un duel (petit gain quelle que soit l'issue), gagner un duel (gros gain). **Refuser un duel coûte de l'Honneur** (code d'honneur, confirme D16). Vigilance § 10.6 : perte modérée, exception assumée. | § 10.6, § 13.2 |
| D77 | 2026-10-06 | Cohérence — Siège au prototype | **Bélier + Mangonneau** (paliers 1 et 2) dans le prototype : murs solides (D23), remparts praticables (D24), héros qui ne rasent pas (D17) et victoire par le centre principal rendent le siège indispensable ; le Mangonneau teste les interactions des remparts. Cavalier lourd toujours hors prototype. | § 17 |
| D78 | 2026-10-06 | Cohérence — Adaptation élémentaire | **L'élément du héros commande l'armée** : le Seigneur-Dragon a un élément actif, changé gratuitement avec délai (règle des Nids) ; l'*Aura draconique* donne aux unités proches l'effet de cet élément (feu : brûlure/dégâts ; glace : ralentissement/armure ; foudre : cadence/chaîne). Mage et Champion choisissent leur élément unité par unité. Plus de terrain (D40). Réactions élémentaires écartées pour l'instant. | § 13.4 |
| D79 | 2026-10-06 | Cohérence — Hallebardier | **Variante de l'Ordre de l'Aube** à la place du Lancier : « mur de piques », anti-cavalerie + bonus contre l'infanterie lourde, plus cher. Au prototype, chaque faction teste une variante (Zombie, Hallebardier). | § 7.1, § 13.2, § 17 |
| D80 | 2026-10-06 | Cohérence — Réponse à un défi | **Réponse définitive** : accepter ou refuser, pas de retrait. Fenêtre de réponse ~5-8 s ; **le silence vaut refus** (avec ses coûts). Acceptation = duel immédiat. Celui qui défie ne peut pas annuler (recharge consommée). | § 10.1 |
| D81 | 2026-10-07 | Cohérence — Refus du Cercle de l'Ombre | **Pas d'exception** : le Cercle paie la même *Hésitation* que les autres ; seule l'Aube a un coût de refus supplémentaire (Honneur). Depuis D45, la Voix est une vraie duelliste ; la piste du refus allégé est retirée. | § 10.1 |
| D82 | 2026-10-07 | Cohérence — Emblématiques du prototype | **Au moins 2 par faction** : Chevalier Vertueux + Moine Lumineux (Aube), Guerrier Damné + Nécromancien (Légions). Pas un plafond : d'autres possibles si de bonnes idées apparaissent. Permet de tester D19, D74, D76 et les cadavres. | § 17 |
| D83 | 2026-10-07 | Cohérence — Population | **Maisons façon AoE** (~ +10 chacune, en bois) ; base de population donnée par les centres ; plafond 175 (D03). Bâtiments économiques du prototype : maison, camps de collecte, ferme, *Sanctuaire* (Aube). | § 5.3, § 17 |
| D84 | 2026-10-07 | Cohérence — Marché | **Marché au prototype.** Précision de l'utilisateur : toutes les ressources s'achètent et se vendent contre de l'or, avec un **cours commun à tous les joueurs** (alliés comme ennemis), qui évolue avec les achats et les ventes de chacun. Le rôle « compenser la pierre » est retiré : le marché sert à tout surplus ou manque. | § 5.5, § 17 |
| D85 | 2026-10-07 | Pouvoirs d'Honneur | **Un pouvoir par palier, coût croissant** : *Lumière sacrée* (palier 0, soin et purification), *Rempart béni* (palier 1, segment de mur invulnérable), *Renforts de l'Aube* (palier 2, escouade de Chevaliers Vertueux, dans la population), *Jugement* (palier 3, frappe annoncée). Livre à choix façon BFME : piste pour plus tard. | § 13.2 |
| D86 | 2026-10-07 | Cadavres des Légions | **Cadavres au sol** (~60-90 s, plafonnés ; pas de cadavre après *Relève impie*). Le **Nécromancien consomme un cadavre pour lever un Squelette permanent**, faible, gratuit, dans la population. *Sacrifice* consomme aussi des cadavres. Nuance de l'utilisateur : **pas de déni**, le Moine ne purifie pas les cadavres. | § 13.3 |
| D87 | 2026-10-07 | Nouvelle faction — Templiers | **Sixième faction à part entière**, fondée entre autres sur l'appel à la croisade. L'Ordre de l'Aube est recentré sur « tenir et protéger » ; les Templiers « partent en croisade ». Vigilance : distinction visuelle forte entre les deux factions de chevaliers. | § 9, § 9.6, § 13.1, § 13.7, § 17 |
| D88 | 2026-10-07 | Templiers — Appel à la croisade | **Croisade déclarée** : une cible désignée et annoncée, une armée de croisade qui marche dessus, succès récompensé, échec sanctionné (contrecoup, longue recharge). Précision de l'utilisateur : les **commanderies** débloquées pendant la partie **définissent la composition** de la croisade. | § 13.1, § 13.7 |
| D89 | 2026-10-07 | Templiers — Troupes de croisade | **Autonomes, hors population, éphémères** : elles marchent au plus court vers la cible, sans ordres, et disparaissent à la victoire ou à l'échec. À régler : plafond (performance), comportement face aux murs. Piste : talent du héros pour être suivi. | § 13.7 |
| D90 | 2026-10-07 | Templiers — Commanderies | **Mélange accès par palier + gloire** : palier 0 = base (*Pèlerins armés*) ; paliers 1-3 = une commanderie choisie parmi 2-3, qui tient lieu de spécialisation (D07) et ajoute un contingent. Chaque croisade réussie donne un rang de gloire à un contingent (plus de troupes ou vétérans), plafonné (~2 par contingent) ; l'échec ne retire rien. | § 9.5, § 13.7 |
| D91 | 2026-10-07 | Templiers — Économie | **Frère convers** (variante du Paysan) : se défend bien, collecte plus vite pendant une croisade. Précision de l'utilisateur : **une croisade réussie rapporte énormément d'or**. Vigilance : gloire + or sur un même succès (effet boule de neige). Donations passives écartées. | § 7.1, § 13.1, § 13.7 |
| D92 | 2026-10-07 | Templiers — Unités propres | **Noyau fixe + commanderies** : *Chevalier du Temple* (cavalerie de choc, « on ne recule pas ») et *Porte-gonfanon* (*Beauséant*, effet de moral, *Désarroi* s'il tombe) toujours disponibles ; chaque commanderie débloque une unité recrutable et ajoute le même type à la croisade (réserve ~8). Exception à « 3 emblématiques ». Archer monté écarté. | § 13.7 |
| D93 | 2026-10-07 | Templiers — Principe | **Faction « lore accurate », quasiment sans fantasy** : ordre historique, sans magie ni surnaturel ; la foi passe par le moral, la discipline, l'organisation et des faits d'armes historiques. Filtre pour toute la conception templière. | § 13.7 |
| D94 | 2026-10-07 | Templiers — Héros (partiel) | **Grand Maître** (nom temporaire). Retenus : aura ***La Règle du Temple*** (armure, immunité à la peur) et ***Compagnie franche*** (achat de mercenaires en or, dans la population). Écartés : *Solde*, *Mise à prix*, *Trésor du Temple*, *Rançon*. Mots-clés de l'utilisateur : croisades, chevaliers, Jérusalem, ordre religieux et militaire, militaire, richesse, **citadelles templières**. | § 13.7 |
| D95 | 2026-10-07 | Templiers — Citadelles (principe) | Décision de l'utilisateur : **les citadelles se construisent librement**, sont des **places fortes** et ont **un effet sur la civilisation** (à définir). Écartés : citadelle fondée par la croisade (problème des cibles dans la base ennemie), citadelle-trésor. Production d'unités : non prévue. | § 13.7 |
| D96 | 2026-10-07 | Templiers — Effet des citadelles | **Les citadelles nourrissent la croisade** : chaque citadelle ajoute des croisés à chaque appel (plafonné) ; les croisades se rassemblent à la citadelle la plus proche de la cible. Raser une citadelle affaiblit les croisades suivantes. | § 13.7 |
| D97 | 2026-10-07 | Templiers — Capacités du Grand Maître | Niveau 1 : ***Prendre la croix*** (des unités contrôlées rejoignent la croisade en cours, contrôle rendu à la fin) ; niveau 4 : *Compagnie franche* ; niveau 7 : ***Sortie*** (la garnison de la citadelle la plus proche combat ~30 s). *Camp retranché* : piste de talent. | § 13.7 |
| D98 | 2026-10-07 | Templiers — Ultime du Grand Maître | ***Deus lo vult !*** (nom retenu par l'utilisateur) : grande zone autour du héros ; la cavalerie alliée gagne une **charge non interruptible** avec réduction des dégâts pendant la charge ; les Lanciers ne l'annulent pas ; les troupes traversées sont probablement renversées (à valider en test). Vigilance : contre anti-cavalerie neutralisé brièvement, marais. | § 13.7 |
| D99 | 2026-10-07 | Templiers — Duel du Grand Maître | Style ***La Règle***, l'inébranlable : ni repoussé ni interrompu par les attaques fortes ; frappe plus fort à mesure que ses PV baissent. Duel d'usure, distinct de la riposte (Paladin) et de la Chaleur (Héritier). | § 10.3, § 13.7 |
| D100 | 2026-10-07 | Templiers — Victoire en duel | ***La gloire du Temple*** : un rang de gloire pour un contingent (D90), comme une croisade réussie, **mais sans l'or** (nuance de l'utilisateur). Permanent mais plafonné : exception mesurée au § 10.6. | § 10.5, § 13.7 |
| D101 | 2026-10-07 | Templiers — Arbre de talents | **Quatre familles** (choix de l'utilisateur) : Croisade, Chevalerie, Trésor, Les Citadelles. Vigilance : 4 familles contre 3 ailleurs (garde-fou « même présentation », D08). | § 9.6, § 13.7 |
| D102 | 2026-10-07 | Équipement du héros | **Supprimé** (demande de l'utilisateur). Le héros progresse par ses niveaux, ses capacités et ses talents. Conséquences : plus de reliques uniques de carte en mode standard (camps neutres : gisement ou passage) ; reliques réservées au mode Reliques ; références retirées (§ 8.3, § 8.4, § 9, § 9.2, § 9.8, § 11, § 16.4). Révise D09. | § 8.3, § 8.4, § 9, § 9.2, § 9.7, § 9.8, § 11, § 16.4 |
| D74 | 2026-10-06 | Cohérence — Le moral | **Famille d'effets, sans jauge** (façon AoE4 / BFME) : effets nommés et temporaires (attaque, armure, cadence) regroupés dans une catégorie « moral » (affichage, cumul plafonné, purification par le Moine). Jamais de déroute. Moral de groupe à états : extension possible après le prototype. | § 7.2 |

---

## 22. Reprise de la review

*Section de travail : elle indique où en est la review question par question du GDD. À mettre à jour à chaque séance.*

**Méthode :** une question à la fois, 2 à 4 options (A/B/C) avec leurs conséquences et une recommandation. Chaque réponse est consignée dans le journal (§ 21, numéro D suivant : **D103**), reportée dans le corps du document, et le ⚠️ correspondant est retiré de la liste du § 20.

**Bilan au 2026-10-06 :** la liste prévue est terminée (D42 à D61). Points encore ouverts, à discuter dans cet ordre :

1. ~~**Contre-mesures anti-héros**~~ → **tranché (D62)** : aucune unité anti-héros.
2. ~~**Conversion de la Voix**~~ → **tranché (D63)** : siège convertible, plafond de coût total.
3. ~~**Seigneur-Dragon et murs**~~ → **tranché (D64)** : survol libre ; plus de dégâts à distance en vol.
4. ~~**Ossuaire**~~ → **tranché (D65)** : collecte globale, non rétroactive, dépense automatique.
5. ~~**Économie des factions**~~ → **tranché (D66 à D70)** : Zombie, Sanctuaire, économie élémentaire, marché noir ; marché commun (§ 5.5).
6. ~~**Déconnexion**~~ → **tranché (D71)** : fenêtre de reconnexion tenue par une IA.
7. ~~**Reliquat de D68 (Enfants du Dragon)**~~ → **tranché (D72, D73)** : *Nids élémentaires* cumulables, changement gratuit mais différé ; feu → or, glace → pierre, foudre → bois.

**Relecture de cohérence (2026-10-06).** Corrections rédactionnelles déjà appliquées (sans décision nouvelle) : « piquiers » → Lanciers (§ 13.2) ; « perturbation des ordres » → confusion (§ 13.5, D37) ; en-tête « pistes » retiré du tableau économique (§ 13.1) ; renvois ajoutés dans le journal (D53, D59, D60, D68, D69) ; « Règle de prototype » des Héritiers → « Règle d'élite ».

Questions de cohérence à trancher, dans cet ordre (numéros D à partir de **D74**) :

1. ~~**Le moral**~~ → **tranché (D74)** : famille d'effets nommés, sans jauge ni déroute (§ 7.2).
2. ~~**L'Honneur**~~ → **tranché (D75)** : monnaie de pouvoirs façon BFME, propre à l'Aube.
   - ~~Sources d'Honneur~~ → **tranché (D76)** : actes honorables ; refuser un duel coûte de l'Honneur (perte modérée).
3. ~~**Siège au prototype**~~ → **tranché (D77)** : Bélier + Mangonneau.
4. ~~**Adaptation élémentaire**~~ → **tranché (D78)** : l'élément actif du héros commande l'armée ; emblématiques réglées unité par unité.
5. ~~**Hallebardier**~~ → **tranché (D79)** : variante de l'Ordre de l'Aube.
6. ~~**Refus et retrait de duel**~~ → **tranché (D80, D81)** : réponse définitive, le silence vaut refus ; pas de refus allégé pour le Cercle.
7. ~~**Unités emblématiques du prototype**~~ → **tranché (D82)** : au moins 2 par faction (Chevalier + Moine ; Guerrier Damné + Nécromancien).
8. **Petits points :**
   - ~~*Champion damné* face à « jamais les héros »~~ → corrigé (rédactionnel) : serviteur générique, pas le héros vaincu (§ 10.5).
   - ~~« sanctuaire » comme lieu d'achat d'équipement~~ → corrigé (rédactionnel) : retiré du § 9.7, pour ne pas confondre avec le *Sanctuaire* de l'Aube.
   - ~~Population~~ → **tranché (D83)** : maisons façon AoE ; bâtiments économiques du prototype listés au § 17.
   - ~~Marché au prototype~~ → **tranché (D84)** : oui ; cours commun à tous les joueurs, toutes ressources.

**Relecture de cohérence terminée (D74 à D84).**

**Mécaniques du prototype encore au stade de pistes (2026-10-07) :**

1. ~~**Pouvoirs d'Honneur**~~ → **tranché (D85)** : un par palier, coût croissant.
2. ~~**Cadavres des Légions**~~ → **tranché (D86)** : cadavres au sol, Squelettes permanents du Nécromancien, pas de déni.

**Nouvelle faction demandée (2026-10-07) : les Templiers**, fondés entre autres sur l'**appel à la croisade**.

1. ~~**Place des Templiers**~~ → **tranché (D87)** : sixième faction à part entière ; l'Aube recentrée sur la défense.
2. ~~**L'appel à la croisade**~~ → **tranché (D88)** : croisade déclarée sur une cible annoncée ; composition définie par les commanderies.
   - ~~Troupes de croisade~~ → **tranché (D89)** : autonomes, hors population, disparaissent à la fin.
   - ~~Commanderies~~ → **tranché (D90)** : une par palier (= spécialisation), rangs de gloire plafonnés par croisade réussie.
3. ~~**Particularité économique**~~ → **tranché (D91)** : frère convers ; une croisade réussie rapporte énormément d'or.
4. ~~**Unités emblématiques**~~ → **tranché (D92)** : Chevalier du Temple + Porte-gonfanon fixes ; autres unités débloquées par les commanderies.
5. ~~**Principe de la faction**~~ → **tranché (D93)** : « lore accurate », quasiment sans fantasy.
6. **Héros des Templiers** (sous le filtre de D93) :
   - ~~Premières capacités~~ → **tranché (D94)** : Grand Maître ; *La Règle du Temple* (aura) et *Compagnie franche* (mercenaires).
   - ~~Citadelles templières (principe)~~ → **tranché (D95)** : construites librement, places fortes, avec un effet sur la civilisation.
   - ~~Effet des citadelles~~ → **tranché (D96)** : elles grossissent chaque croisade et en sont le point de rassemblement.
   - ~~Capacités restantes et ultime~~ → **tranché (D97, D98)** : *Prendre la croix*, *Sortie* ; ultime *Deus lo vult !*.
   - ~~Kit de duel~~ → **tranché (D99)** : *La Règle*, l'inébranlable.
   - ~~Effet de victoire en duel~~ → **tranché (D100)** : un rang de gloire, sans or.
7. ~~**Arbre de talents**~~ → **tranché (D101)** : Croisade, Chevalerie, Trésor, Les Citadelles.

**Équipement du héros :** supprimé (D102), à la demande de l'utilisateur.

**Conception des Templiers terminée** (D87 à D101), hors contenu détaillé (unités des commanderies, talents, valeurs).

---

**ÉTAT AU 2026-10-07 — point de reprise pour la prochaine séance**

- **Décisions prises :** D01 à D102. Prochain numéro : **D103**.
- **Aucune question de design en cours.**
- **Code :** l'ancien code (classes C++ de gameplay et `Content/Blueprints`) a été supprimé et commité. Le module `MyCrusader` est vide et compile. À la première ouverture dans l'éditeur, `BattleMap` peut signaler des acteurs dont la classe n'existe plus : les supprimer et enregistrer la carte.
- **Pistes pour la suite, au choix de l'utilisateur :**
  1. **Premier chantier de code :** système d'unités légères (D29, § 16.4) : lire le projet, rédiger un plan (Mass Entity ou gestionnaire maison), le valider avant d'écrire du code.
  2. **Contenu détaillé des Templiers :** les ~8 commanderies et leurs unités (D90, D92), valeurs de la croisade (délai, plafond, or, gloire).
  3. **Arbres de talents** (D08) : 3 ou 4 familles pour toutes les factions (vigilance de D101), puis contenu des arbres de l'Aube et des Légions (prototype).
  4. **Fiche de valeurs de départ** pour tout ce qui est « à régler en test » (ci-dessous).

**À régler en test plutôt qu'en discussion :** courbe d'XP (Q04), valeurs de récupération et de prix de résurrection (D25), pourcentages d'aura (D26), valeurs de terrain (D33), bonus, coût et délai de changement des *Nids élémentaires* (D72), coût et nombre des pouvoirs d'Honneur (D75, D85), cadavres et Squelettes (D86), croisade : délai, plafond de troupes, or, gloire, bonus des citadelles (D88 à D96).

**Premier chantier de code identifié :** construire le système d'unités légères (D29, § 16.4). L'ancien code (`AUnitBase : ACharacter` et ses Blueprints) a été supprimé le 2026-10-07.
