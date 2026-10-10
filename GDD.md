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

Pire cas sur une carte à 8 joueurs : **environ 1 400 unités simulées** (population seule). **Plancher de sécurité *(D190)* : le jeu doit tenir au minimum 2 000 unités**, même si cette valeur ne sera jamais atteinte en partie (§ 16.4).

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

**Production militaire *(décision D128)* : un bâtiment par catégorie, façon AoE4.** ***Caserne*** (Lancier ou Hallebardier, Homme d'armes), ***Champ de tir*** (Archer, Arbalétrier), ***Écurie*** (cavaliers), ***Atelier de siège*** (Bélier, Mangonneau…). Les unités emblématiques sortent du bâtiment de leur catégorie ou d'un bâtiment propre à la faction. L'éclairage lit la composition (voir trois écuries, c'est voir venir la cavalerie, D07), la production se fait en parallèle, et raser un bâtiment militaire a un effet réel. *Prototype :* il peut commencer avec un bâtiment militaire unique (§ 17), à condition que les données (`BuildingData`) soient conçues pour ce découpage dès le départ.

La base doit être plus qu'un amas de bâtiments : défense, production, économie, projection. Certaines factions doivent être meilleures en défense que d'autres.

### 6.1 Siège *(décision D23)*

**Siège léger, façon AoE4 réduit.** Les héros ne rasent pas les bâtiments (D17), et la victoire standard passe par la destruction du centre principal : le siège est donc indispensable pour finir une partie.

| Engin | Palier | Rôle | Vulnérable à |
|---|---|---|---|
| **Bélier** | 1 | bâtiments et portes | corps à corps |
| **Mangonneau / Catapulte** | 2 | groupes et murs, longue portée | Cavalier léger, corps à corps |
| **Trébuchet** | 3 | fortifications de loin ; doit être monté et démonté | Cavalier léger, corps à corps |
| **Canon** *(D38)* | 3 (débloqué avec le palier, D180) | courte portée, gros dégâts, plus mobile que le Trébuchet | Cavalier léger, corps à corps |
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
| Soigneur *(D134)* | Moine | infanterie légère |

Les détails des contres sont au § 7.2 (D21).

**Moine *(décision D134)* : soigneur commun, façon AoE4.** Toutes les factions disposent d'un soigneur qui **soigne en combat** (**palier 1**, Moine Lumineux compris ; *D212 : aucun soin au palier 0, sauf *Lumière sacrée*, D213*). Il ne convertit pas : la conversion reste propre au Cercle de l'Ombre (D44, D63).
- **Exception à la règle des variantes :** pour ce rôle seulement, **plusieurs factions** peuvent remplacer le Moine par une unité différente (à la manière des éléphants guérisseurs des Tughlaq dans AoE4).
- **Ordre de l'Aube :** le **Moine Lumineux** (emblématique) tient lieu de variante (soin + atténuation, D125). Avec le Hallebardier, l'Aube atteint 2 variantes, le maximum.
- **Templiers :** Moine commun ; le *Frère infirmier* (D133, soin hors combat) le complète.
- **Autres factions *(D210)* :** **Légions Noires : aucun soigneur** ; elles ne soignent pas, elles recyclent (Squelettes, *Sacrifice*), et leurs unités sont moins chères (D66). **Enfants du Dragon et Cercle de l'Ombre :** Moine commun, apparence propre à la faction. **Héritiers du Feu :** Moine commun en **version élite** (règle ×2) ; le Prêtre de la Flamme reste un soutien qui bénit et protège, sans soin direct.
- **Bâtiment de production *(D211)* : une *Chapelle* commune** (nom provisoire, à distinguer de la *Chapelle de campagne* de l'Aube), comme le Monastère d'AoE4 : elle produit le soigneur et accueille une ou deux technologies de soin ; apparence propre à chaque faction ; **absente chez les Légions** (D210). **Palier 1** *(D212, après correction d'un malentendu sur le numérotage des paliers)* ; le Moine Lumineux de l'Aube sort aussi de la Chapelle de l'Aube. Vigilance : le soin généralisé allonge les combats.

**Poudre *(décision D38)* : commune à toutes les factions, sauf contre-ordre, au palier 3.**

- Le **palier 3** débloque automatiquement deux nouvelles unités du socle *(D180 : la technologie « Armes à poudre » est supprimée, aucune unité ne demande de recherche)* : l'**Arquebusier** (tireur, catégorie d'armure distance) et le **Canon** (siège, § 6.1). Elles s'ajoutent à l'Arbalétrier et aux engins existants.
- **Exceptions de faction** (règle des variantes ci-dessous) :
  - **Légions Noires :** pas de poudre *(D59)*. À la place du Canon : le ***Cracheur de bile***, engin de siège à acide (§ 6.1). Pas d'équivalent de l'Arquebusier ; en compensation, un bâtiment propre en plus au palier 3 : l'***Ossuaire*** (D60, § 13.3).
  - **Templiers *(D187)* :** poudre **maltaise** *(nuance de l'utilisateur ; justification : l'Ordre de Malte, ordre militaire voisin, a connu la poudre)* : *Arquebusier maltais* et *Canon maltais* (noms provisoires), **version plus primitive que le socle**, donc moins forte ; ils **s'intègrent aussi à la croisade**. **Statut *(D188)* : un écart de statistiques, pas une variante** (comme la règle d'élite des Héritiers, D39) : unités du socle aux statistiques plus basses, **moins chères**, avec nom et visuel propres, sans capacité en plus. La règle des variantes est respectée.
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
- « Moral » est une **catégorie** commune à ces effets : même affichage (icône sur l'unité), mêmes règles de cumul, même traitement par les capacités qui les atténuent.
- **Cumul (indicatif, à régler en test) :** un même effet ne se cumule pas avec lui-même (il est rafraîchi) ; des effets différents s'additionnent, dans un **plafond global** de bonus et de malus de moral.
- **Atténuation, pas de purification *(décision D125)* :** le Moine Lumineux **atténue** les effets de moral négatifs et les malédictions (intensité et/ou durée réduites) **sans les retirer**. Seule *Lumière sacrée* pourrait les retirer : ⚠️ à trancher en test. Le sol maudit (*Sol profané*, D115) ne se purifie pas.
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

- Enfants du Dragon : interaction supérieure avec les créatures ; ⚠️ à revoir depuis D149 (la faction n'a plus de bêtes) ;
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

**Héros hors prototype :** Seigneur-Dragon des Enfants du Dragon (§ 13.4, D41), la Voix du Cercle de l'Ombre (§ 13.5, D44), le Champion Héritier des Héritiers du Feu (§ 13.6, D46). le Grand Maître des Templiers (§ 13.7, D94).

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

**Palier 3 *(décision D179)* : « Finir ou renverser ».** Le tronc commun du palier 3 donne à toutes les factions les outils de fin de partie (siège lourd, poudre, technologies finales). Les deux spécialisations de chaque faction sont **opposées** : l'une sert à **conclure** (percée, siège, pression), l'autre à **renverser** (tenir, contre-attaquer, reprendre l'avantage). Le joueur choisit en lisant l'état de la partie ; l'éclairage de l'adversaire gagne encore en valeur. C'est l'application directe des leviers de retour du § 14.2.

- **Exception *(D193)* :** les Héritiers du Feu n'ont pas de spécialisation au palier 3 (tronc commun seul).
- ⚠️ Vigilance : la voie « renverser » n'est jamais un *rubber band* frustrant pour celui qui mène. Elle aide à tenir et à contre-attaquer, sans punir l'avance adverse (pas de bonus indexé sur le retard).

⚠️ Courbe d'XP par niveau à définir en test **[Q04]**.

### 9.6 Arbre de talents

**Décision D08 : un arbre de talents entièrement propre à chaque faction.** Les familles, la structure et le contenu sont spécifiques. Ordre de l'Aube *(D106)* : les trois Serments, Gardien / Capitaine / Champion (§ 13.2). Légions Noires *(D112)* : les trois Rites, Moisson / Charnier / Effroi (§ 13.3). Templiers *(D101)* : Croisade / Chevalerie / Trésor / Les Citadelles. Enfants du Dragon *(D153)* : Le Ciel / La Lignée / La Mue (§ 13.4). Cercle de l'Ombre *(D163)* : La Lame / Le Miroir / La Toile (§ 13.5). Héritiers du Feu *(D173)* : Le Marteau / L'Enclume / La Flamme (§ 13.6).

Le héros gagne environ **8 points de talent** par partie (niveaux 2 à 9, D04).

**Rangées 7 à 9 *(décision D194)* : continuité des rangées 4 à 6.** Elles modifient des capacités, avec une valeur comparable dans chaque rangée. Cibles : la **capacité 3** (niveau 7), les capacités plus anciennes et le **contenu fixe** de la faction (jamais une spécialisation). **Aucun talent sur l'ultime** (inactif avant le niveau 10). Le contenu du palier 3 ne peut être visé qu'à partir de la rangée 9 (niveau 9). Pas de talent mort. Il ne peut pas prendre toutes les améliorations : le choix crée une spécialisation.

**Forme de l'arbre *(décision D103)* : des rangées avec seuils de famille** (hybride entre *Heroes of the Storm* et un arbre à profondeur).

- **Une rangée par niveau** (niveaux 2 à 9) : le joueur y choisit **1 talent parmi 3**, chacun rattaché à une famille (code couleur). Le choix est rapide et lisible, même en plein combat.
- **Les familles récompensent la constance :**
  - **3 talents** pris dans une même famille donnent son **sceau**, un bonus passif marquant ;
  - **5 talents** donnent son **talent clé**, qui transforme une capacité ou l'aura du héros (exemple indicatif : *Relève impie* lève aussi des Squelettes).
- **Le dilemme de chaque niveau :** prendre le meilleur talent de la rangée, ou rester fidèle à sa famille pour atteindre le sceau puis le talent clé.
- Avec 8 points, le maximum est **un talent clé et un sceau** (5 + 3), ou deux sceaux et deux talents isolés.
- **Prototype (niveaux 1 à 6, 5 points) :** le talent clé n'est atteignable qu'en restant dans une seule famille du niveau 2 au niveau 6.
- **Lisibilité pour l'adversaire :** les sceaux et talents clés obtenus sont signalés (à l'écran, sur le héros), ce qui donne de la valeur à l'éclairage, comme les spécialisations de palier (D07).
- **IA :** un profil de build désigne une famille principale et une famille secondaire.
- Contenu à écrire par faction : environ 24 talents (8 rangées × 3), 3 sceaux et 3 talents clés.

**Nombre de familles *(décision D104)* : 3, sauf exception templière.** Les Templiers gardent leurs 4 familles (D101) : leurs rangées proposent **4 talents**, un par famille, avec les mêmes seuils (3 et 5). Contenu templier : 32 talents, 4 sceaux, 4 talents clés. Vigilance : plus d'options par rangée, plus de risque d'une option dominante, à surveiller en test.

**Moment du choix *(décision D105)* : réserve libre.**

- Le point de talent reste **en réserve sans limite de temps** et se dépense quand le joueur le veut, y compris pendant la récupération du héros mort.
- Les rangées se remplissent **dans l'ordre** (celle du niveau 2 avant celle du niveau 3). L'effet est immédiat.
- Un **rappel** reste affiché tant qu'un point n'est pas dépensé, avec un bouton **« choix conseillé »** qui prend en un clic le talent de la famille la plus remplie.
- Garder un point permet de **s'adapter** après avoir éclairé l'adversaire (spécialisation, sceau, talent clé), ce qui renforce la valeur de l'éclairage voulue par D103.

**Garde-fous proposés**, pour que 6 arbres différents restent lisibles et équilibrables :

- même nombre de points disponibles et **même structure** pour toutes les factions : rangées par niveau, seuils de sceau et de talent clé (D103). Seul le nombre de talents par rangée varie : 3, ou 4 chez les Templiers (D104) ;
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
- **Fenêtre de défi à l'écran *(décision D117, demande de l'utilisateur)* :** un défi reçu ouvre une **fenêtre dans l'interface** (popup), impossible à manquer.
  - Contenu : portrait et niveau du héros qui défie, ses sceaux et talents clés visibles (D103), barre du temps de réponse restant ; boutons **Accepter** et **Refuser**, chacun avec un raccourci clavier.
  - La fenêtre **ne met pas le jeu en pause** et ne bloque pas le reste de l'écran : le joueur peut continuer à donner des ordres pendant qu'il décide. Un bouton ou un raccourci centre la caméra sur le duel proposé.
  - Le défi est aussi signalé sur la minimap. Le joueur qui défie voit de son côté un indicateur « défi envoyé » avec le même compte à rebours.
  - Détails de présentation indicatifs, à valider en test.
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
  - **4 capacités de duel** avec temps de recharge : la parade commune et 3 capacités propres, plus l'ultime de duel au niveau 10 (D119).
- **Les attaques fortes sont annoncées** par une animation de préparation (~0,5 à 1 s). Cela permet de lire et de contrer malgré la latence du réseau (modèle client-serveur, D29).
- **Arbitrage d'attention :** le joueur peut détourner les yeux pour gérer sa base pendant le duel, mais un duel suivi attentivement se gagne plus souvent.
- **L'IA** joue les duels avec les mêmes temps de réaction qu'un humain, sans réflexes surhumains.

**Pendant un duel :**

- les **capacités RTS actives** du héros sont remplacées par son **kit de duel** ;
- son **aura de commandement reste active**, puisqu'il est physiquement sur la carte.

**Gabarit commun des kits *(décision D119)* : une parade commune, 3 capacités propres, l'ultime.**

- **Parade (commune aux six héros) :** touche fixe, courte fenêtre ; elle **réduit fortement** une attaque forte annoncée, **sans l'annuler**, et a une courte recharge. Sa fenêtre dépend de la posture (D118). Toute attaque annoncée peut donc être lue et contrée, quel que soit le héros, IA comprise.
- **3 capacités propres :** elles portent le style du héros (attaques, défenses, mobilité, contrôle, au choix de chaque héros). Une capacité propre peut **améliorer la parade** (riposte du Paladin) ou **la tromper** (feintes de la Voix).
- **Ultime de duel :** au niveau 10 (D43).
- **Le triangle repose sur la parade :** la feinte trompe la parade, la riposte l'améliore, l'agression la sature.
- ⚠️ Vigilance : la parade commune ne doit pas éclipser les défenses propres ; elle reste imparfaite (réduction partielle, recharge).

**Déplacement *(décision D120)* : contact automatique.** Les deux héros restent au corps à corps sans action du joueur ; on ne se déplace dans le cercle qu'avec une **capacité propre** (bond, recul, charge). Regarder sa base ne coûte pas de terrain ; la mobilité devient une signature de certains héros (la Voix, le Dragon), pas un acquis de tous.

Le kit de duel est séparé du kit RTS.

**Déblocage *(décision D43)* : kit de duel complet dès le niveau 1, sauf l'ultime de duel, débloqué au niveau 10.**

- Du niveau 1 au niveau 9, le niveau donne un avantage de **statistiques** en duel, pas d'**outils** : un héros en retard de quelques niveaux garde une vraie chance s'il lit mieux son adversaire.
- Au niveau 10, l'ultime de duel s'ajoute : c'est la récompense de fin de progression, en miroir de l'ultime RTS.
- Le prototype (niveaux 1 à 6) teste donc le kit de duel complet, sans ultime de duel.
- ⚠️ **Point de vigilance :** un héros de niveau 10 a un outil de plus que son adversaire. L'ultime de duel doit rester puissant mais lisible et contrable (annoncé, comme les attaques fortes), pour ne pas décider seul l'issue d'un duel. À régler en test.

**Postures *(décision D118)* :** on change de posture en un clic, à tout moment ; c'est le geste de base du duel, même quand le joueur regarde ailleurs (« je gère ma base, je passe en défensive »).

| Posture | Effet |
|---|---|
| **Offensive** | plus de dégâts, fenêtres de défense (parade, riposte) plus courtes |
| **Équilibrée** | valeurs de référence |
| **Défensive** | moins de dégâts, fenêtres de défense plus larges |

Valeurs à régler en test. Exception : le Seigneur-Dragon remplace les trois postures par trois éléments (D49).

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
| Militaire | dégâts, armure, portée, vitesse, capacités (statistiques et visuel uniquement, jamais de remplacement d'unité, D38). Les unités ne se débloquent **jamais par une recherche** mais avec le palier (D180, sauf contre-ordre) |
| Infrastructure | bâtiments, défenses, production, population |
| Héros | capacités, commandement, récupération, duel |
| Faction | mécanique signature, unités spécialisées, technologies propres |

**Mécanisme de déblocage :** un palier ouvre des **bâtiments** (tronc commun + spécialisation, D07) et **débloque automatiquement ses unités**, sans recherche (D180, sauf contre-ordre). Chaque **technologie** demande un palier minimum, un bâtiment et des ressources.

**Répartition *(décision D27)* : arbre commun, technologies de faction ciblées.**

- **~70 à 80 % commun à toutes les factions :** forge (dégâts et armures par catégorie d'unités), économie (collecte, rendement), infrastructures (solidité des murs, population), siège.
- **~20 à 30 % propres à la faction :** mécanique signature (Honneur, cadavres…), unités emblématiques et variantes, **spécialisations de palier** (D07), technologies héros.
- L'identité se concentre là où elle se voit. Un joueur qui connaît une faction se repère dans les autres (pilier 1). C'est le modèle d'AoE4 : forge commune, monuments et technologies uniques.

### 11.1 Tronc commun des paliers 0 à 2 (prototype)

**Grille validée *(décision D129)*** (noms, coûts et valeurs indicatifs, à régler en test). Méthode : grille complète amendée (comme D110).

**Bâtiments et unités par palier**

| Palier | Bâtiments débloqués | Unités du socle débloquées | Emblématiques (prototype) |
|---|---|---|---|
| **0 — Fondation** | Centre principal, Maison, Camps de collecte (bois, pierre, or), Ferme, **Caserne**, **Champ de tir**, Palissade, Tour de guet en bois ; *Sanctuaire* (Aube) | Paysan / Zombie, Lancier / Hallebardier, Archer | — |
| **1 — Essor** | **Écurie**, **Forge**, Marché, **Atelier de siège**, Centre secondaire, Murs de pierre, Porte, Tour de pierre, ***Chapelle*** *(D211, D212 ; sauf Légions)* ; spécialisation (D123, D126) | Homme d'armes, Arbalétrier, Cavalier léger, Bélier, **Moine** *(D134, D212 ; Chapelle)* | Chevalier Vertueux, Moine Lumineux ; Guerrier Damné, Nécromancien |
| **2 — Puissance** | **Académie** (technologies avancées), Tour renforcée ; spécialisation (D124, D127) | Cavalier lourd (hors prototype), Mangonneau | — |

Réponses aux contres garanties (D07) : le Lancier (anti-cavalerie) dès le palier 0, l'Arbalétrier (anti-armure) et l'Homme d'armes au palier 1.

**Technologies communes**

| Bâtiment | Palier 1 | Palier 2 |
|---|---|---|
| **Forge** | Lames affûtées (attaque mêlée I), Empennage (attaque à distance I), Maille (armure infanterie I), Barde (armure cavalerie I) | les mêmes en II |
| **Camps et ferme** | Haches doubles (bois), Pics de mineur (pierre et or), Charrue (nourriture), Brouette (vitesse et capacité des travailleurs) | les mêmes en II |
| **Centre principal** | Milice de défense (les travailleurs se défendent mieux près des centres) | Cloche d'alarme améliorée (garnison plus rapide) |
| **Académie** | — | Maçonnerie (murs et tours +PV), Vétérans (unités du socle +PV), Logistique (vitesse hors combat de l'armée) |
| **Atelier de siège** | Bélier renforcé | Munitions lourdes (dégâts du Mangonneau) |

**Technologies propres (D27 : ~20-30 %)**, hors spécialisations :

| Faction | Palier 1 | Palier 2 |
|---|---|---|
| **Aube** | *Liturgie* (gains d'Honneur +10 %) ; *Hallebardes trempées* (Hallebardier : bonus contre l'infanterie lourde accru) | *Bénédiction des armes* (Chevaliers et Hallebardiers +attaque contre les morts-vivants et les créatures) ; *Vœu de pauvreté* (Moines moins chers) |
| **Légions** | *Os taillés* (Squelettes +armure) ; *Chair putride* (Zombies +PV) | *Rites funèbres* (cadavres au sol +20 s) ; *Lames empoisonnées* (Guerriers Damnés : dégâts sur la durée) |

⚠️ Vigilance : *Bénédiction des armes* est un bonus contre une catégorie de factions ; *Os taillés*, *Rites funèbres* et *Liturgie* recoupent des talents (*Os durcis*, *Charnier fertile*, Serments). À arbitrer à la validation.

### 11.2 Tronc commun du palier 3 *(décision D180)*

**Grille validée avec amendements** (noms, coûts et valeurs indicatifs, à régler en test). Principe du palier : « Finir ou renverser » (D179). Le tronc commun penche volontairement vers la défense (*Bastion*, *Fortifications*, *Levée de la garnison*) : c'est le socle de « renverser », garanti à tous ; le siège lourd et la poudre forment le socle de « finir ».

**Règle *(D180)* : aucune unité ne demande de recherche pour être débloquée** (sauf contre-ordre). Les unités du palier sont disponibles dès qu'il est atteint, dans leur bâtiment de production. La technologie « Armes à poudre » de D38 est supprimée, et avec elle la *Fonderie* (D181).

| Palier | Bâtiments débloqués | Unités du socle débloquées (automatiquement) |
|---|---|---|
| **3 — Légende** | ***Bastion*** (tour lourde, défense de fin de partie) ; spécialisation (D179) | **Trébuchet** et **Canon** (Atelier de siège) ; **Arquebusier** (Champ de tir) |

- **Légions :** pas de poudre (D59) ; le *Cracheur de bile* remplace le Canon à l'Atelier de siège, débloqué avec le palier ; l'*Ossuaire* (D60) est leur bâtiment en plus.
- **Héritiers :** *Bombarde* et *Arquebusier de la Forge* (D61) à la place du Canon et de l'Arquebusier, débloqués avec le palier.
- **Templiers *(D187)* :** poudre **maltaise** (*Arquebusier maltais*, *Canon maltais*, noms provisoires) : plus primitive que le socle, donc moins forte ; intégrée à la croisade. Justification : l'Ordre de Malte (les Hospitaliers, ordre militaire voisin) a connu la poudre à canon (sièges de Rhodes en 1480 et 1522, de Malte en 1565). Voir § 7.1.

**Technologies communes du palier 3**

| Bâtiment | Technologies |
|---|---|
| **Forge** | les quatre technologies de Forge en **III** (attaque mêlée, attaque à distance, armure infanterie, armure cavalerie) ; *Poudre raffinée* (armes à poudre +dégâts ; sans objet pour les Légions) *(D181)* |
| **Camps et ferme** | collecte et Brouette en **III** |
| **Académie** | *Fortifications* (murs, portes et Bastions +PV) ; *Balistique* (tireurs et tours touchent mieux les cibles en mouvement) |
| **Atelier de siège** | *Contrepoids* (Trébuchet monté et démonté plus vite) |
| **Centre principal** | *Levée de la garnison* (centres et Bastions garnis tirent plus de projectiles) |

Technologies propres du palier 3 : vues avec la spécialisation de chaque faction.

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

**Calendrier : ~~les événements mondiaux arrivent après le prototype~~** → *révisé par D205* : l'option **Catastrophe** entre dans le prototype, donc le **système d'événements mondiaux et le volcan** (§ 12.1, seul événement défini) aussi. Les autres événements (§ 12.3) restent pour après. **Option Catastrophe au prototype *(D208)* : le volcan seul, sur plus de sites** : 3 à 4 éruptions par partie, sur plusieurs sites volcaniques, fenêtres de déclenchement plus précoces ; **préavis inchangé** (60 à 90 s) : plus fréquent ne veut jamais dire moins prévisible (§ 12.2). Le prototype se concentre sur le socle RTS, le héros, le duel et les fortifications. L'architecture (World State Manager, § 16.2) est tout de même prévue dès le départ, pour accueillir les événements sans refonte.

### 12.3 Événements futurs

~~Volcan, tempête, séisme, inondation, incendie, invasion de monstres, ouverture d'une faille, apparition d'une créature, corruption magique, changement temporaire du climat.~~

**Choix et ordre après le prototype *(décision D209)* : quatre événements contrastés.** Le volcan est au prototype (D205, D208).

1. **Inondation** : lente ; coupe les routes basses et noie les champs, sans rien détruire. L'opposé du volcan.
2. **Invasion de monstres** : vagues neutres qui attaquent tout le monde près d'un site ; s'appuie sur les camps neutres (D32) ; la défense devient un enjeu commun.
3. **Ouverture d'une faille** : une **opportunité** (gisement rare ou passage nouveau à disputer) : on se bat pour quelque chose, pas contre la carte.
4. **Tempête** : sur une zone, portée des tireurs et vitesse réduites ; **aucune vision retirée**.

- **Reportés :** séisme, incendie, apparition d'une créature, changement temporaire du climat.
- **Retirée :** corruption magique (contraire à D40 : pas de zone corrompue posée par la carte en compétitif).
- Tous suivent le § 12.2 et D28 (sites neutres et symétriques, préavis de 60 à 90 s).

---

## 13. Factions

### 13.1 Principes

Chaque faction possède une **identité mécanique principale** :

| Faction | Identité | Point fort (timing) |
|---|---|---|
| Ordre de l'Aube | discipline / honneur / défense : **tenir et protéger** (D87) | forte défense, milieu de partie |
| Légions Noires | mort / corruption / recyclage des pertes | attrition, combats prolongés |
| Enfants du Dragon | adaptation / éléments ; **sans bêtes** *(D149)* | adaptation, contrôle |
| Cercle de l'Ombre | information / subversion / pièges | information, harcèlement |
| Héritiers du Feu | élite / qualité / faible population | armée réduite, puissance individuelle |
| Templiers *(D87)* | croisade / guerre sainte : **partir en croisade** | offensive *(D130)* : solides en continu, **sommet pendant une croisade** |

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
| **Moine Lumineux** | soutien | soins, **atténuation** des malus de moral et des malédictions (sans les retirer, D125), sceaux lumineux ralentissant les ennemis |
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

**Spécialisations de palier *(D07)* :**

| Palier | Choix (définitif) | Rôle |
|---|---|---|
| **1 — Essor** *(D123)* | ***Bastion de l'Aube*** : murs et tours plus solides ; les **tours bénies** soignent lentement les alliés proches | **repli** : une base imprenable |
| | ***Chapelle de campagne*** : petit bâtiment constructible loin de la base ; étend l'effet du *Sanctuaire* aux fermes proches, sert de point de ralliement et forme des Moines | **projection** : l'Aube s'étend (répond à la vigilance de D67) |
| **2 — Puissance** *(D124)* | ***Lices de l'Aube*** : Chevaliers Vertueux formés plus vite, *Défi du Chevalier* renforcé | **l'Acier** |
| | ***Monastère*** : Moines Lumineux qui soignent mieux, atténuent plus fortement les malus de moral et les malédictions (D125), et dont les soins rapportent plus d'Honneur | **la Foi** |
| **3 — Légende** *(D182)* | ***Autel du Jugement*** : le pouvoir *Jugement* (D85) coûte moins d'Honneur, frappe une zone plus large et fait de gros dégâts aux bâtiments | **finir** (D179) : la lumière qui conclut |
| | ***Crypte des saints*** : un second pouvoir d'Honneur au palier 3, ***Veille des saints*** *(D183)* : pendant ~10 s, aucun bâtiment de la zone (centre principal compris) ne descend sous 1 PV, et les paysans y réparent ~50 % plus vite. Zone annoncée ; ne touche jamais les unités (distinct de *Dernier Rempart*) | **renverser** (D179) : tenir le temps que l'armée revienne |

Le palier 1 décide de la forme de la partie (repli ou expansion), le palier 2 de la composition de l'armée. Les deux se lisent à l'éclairage. Valeurs à régler en test.

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
  | 0 | *Lumière sacrée* : soin sur une zone, **dès le palier 0** *(D213 : exception assumée à « aucun soin au palier 0 », D212 ; pouvoir payé en Honneur, donc rare en ouverture)* ; retrait possible des effets de moral négatifs (D74), ⚠️ à trancher en test (D125) | soutien : sauver une ligne qui tient |
  | 1 | *Rempart béni* : un segment de mur devient invulnérable quelques secondes | défense : tenir une brèche face au siège |
  | 2 | *Renforts de l'Aube* : une escouade de Chevaliers Vertueux arrive au centre principal (comptée dans la population) | renfort |
  | 3 | *Jugement* : frappe de lumière annoncée sur une zone, contrable | offensive |
  | 3 *(Crypte des saints, D183)* | *Veille des saints* : ~10 s, les bâtiments d'une zone ne descendent pas sous 1 PV ; réparation ~50 % plus rapide | défense : sauver la base d'une percée |

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
  - **Kit *(décision D121)* : « Le Juge », chaque faute se paie.** Parade commune (D119) +
    - ***Riposte*** : parade améliorée ; réussie, elle contre-attaque et inflige une **Faute** à l'adversaire (cumuls visibles au-dessus de lui). **Ratée** (par exemple sur une feinte), elle laisse le Paladin brièvement exposé : c'est ce qui permet à la feinte de battre la riposte.
    - ***Coup de bouclier*** : coup rapide qui **interrompt** une attaque forte en préparation. Seconde réponse, plus risquée que la parade (il faut viser le bon moment). Sans effet sur le Grand Maître, qu'on ne peut interrompre (D99) : contre voulu.
    - ***Sentence*** : consomme toutes les Fautes en dégâts, croissants avec le nombre de cumuls.
    - Ultime de duel (niveau 10) ***Verdict*** : attaque forte annoncée **impossible à parer**, dont les dégâts dépendent des Fautes. Contre-jeu : jouer proprement pour ne pas offrir de Fautes, ou s'éloigner par une capacité de mobilité.
  - Arc du duel : encaisser, accumuler les Fautes, rendre la sentence. L'adversaire voit les cumuls monter et doit choisir : continuer d'attaquer ou ralentir. Synergies : *Ordalie* (chaque riposte donne de la *Ferveur*), *Contre parfait* (la riposte étourdit). Valeurs à régler en test.
- **Lien avec l'Honneur *(D76)* :** gagne de l'Honneur en *acceptant* les duels (quelle que soit l'issue), en les gagnant et en tenant des positions sous pression ; en perd en refusant.
- **Victoire en duel *(D35)* :** gros gain d'Honneur et recharge immédiate de la *Bannière de l'Aube*.
- **Capacité ultime (niveau 10) *(D36)* : *Dernier Rempart*.** Pendant ~10 s, les alliés dans une large zone autour du héros ne peuvent pas descendre sous 1 PV ; à la fin, ils récupèrent une partie des dégâts subis pendant l'effet. Contre : reculer et attendre la fin au lieu de frapper. Valeurs à régler en test.
- **Arbre de talents *(D106)* : les trois Serments.** Chaque famille correspond à une grande source d'Honneur (D76) : choisir un Serment, c'est choisir **comment son Aube gagne l'Honneur**.

  | Serment | Rôle (garde-fous D08) | Contenu | Talent clé (piste, à valider) |
  |---|---|---|---|
  | **Serment du Gardien** | stratégique : défendre et tenir | *Bannière de l'Aube*, murs et tours, *Sanctuaire* ; plus d'Honneur en tenant un point et en défendant | ***Bannière inébranlable*** (D107) |
  | **Serment du Capitaine** | commandement : protéger | *Aura de l'Aube*, *Serrez les rangs*, Chevaliers Vertueux, Moines ; plus d'Honneur par les dégâts absorbés et les soins | ***Mur de boucliers*** (D108) |
  | **Serment du Champion** | duel : combattre avec honneur | riposte, survie ; plus d'Honneur en duel | ***Ordalie*** (D109) |

  - **Serment du Gardien *(D107)* : « On ne passe pas ».**
    - Sceau ***Pierres de l'Aube*** : murs et tours gagnent des PV, les paysans les réparent plus vite ; tenir un point rapporte plus d'Honneur.
    - Talent clé ***Bannière inébranlable*** : la Bannière ne disparaît plus tant que des alliés se tiennent dans sa zone ; plus ils y restent, plus ils gagnent d'armure (cumuls plafonnés) ; la zone produit de l'Honneur en continu. Crée un objectif visible à briser. Contre-jeu : déloger les troupes, siège ou dégâts de zone, contourner.
    - ⚠️ Vigilance : Aube « tortue » trop solide ; le plafond des cumuls et le contournement doivent l'empêcher. Valeurs à régler en test.
  - **Serment du Capitaine *(D108)* : « Un pour tous ».**
    - Sceau ***Discipline de l'Aube*** : les unités dans l'aura résistent aux effets de moral négatifs (durée réduite d'environ moitié) ; les dégâts absorbés rapportent plus d'Honneur. Répond à l'*Aura de terreur* des Légions.
    - Talent clé ***Mur de boucliers*** : pendant *Serrez les rangs*, les dégâts subis par une unité de la formation sont **répartis entre toutes les unités de la formation** ; aucune ne tombe seule. Contre la concentration de tirs et les assassins. Contre-jeu : les dégâts de zone, répartis à chaque coup, deviennent redoutables (*Sacrifice*, *Cracheur de bile*). Distinct du *Défi du Chevalier* (une unité encaisse) et de *Dernier Rempart* (plancher à 1 PV).
  - **Serment du Champion *(D109)* : « Jugement des armes ».**
    - Sceau ***Vœu du Champion*** : le temps de recharge du défi est réduit d'environ un tiers ; accepter et gagner un duel rapportent plus d'Honneur.
    - Talent clé ***Ordalie*** (le jugement par les armes) : pendant un duel du Paladin, chaque riposte réussie donne un cumul de *Ferveur* (attaque et moral, cumuls plafonnés) aux alliés autour du cercle de duel ; en cas de victoire, le *Triomphe* (§ 10.5) dure plus longtemps et touche une zone plus large. Le duel devient un événement de bataille. Contre-jeu : refuser le défi (avec *Hésitation*), attaquer sans coups annoncés pour ne rien offrir à riposter, frapper les troupes spectatrices. La Voix (feintes) le contre naturellement (triangle du § 10.3).
    - ⚠️ Vigilance : le défi plus fréquent multiplie les refus imposés à l'adversaire ; l'anti-harcèlement de D16 doit tenir. Valeurs à régler en test.
    - ⚠️ Vigilance : le build est faible entre deux duels (recharge du défi, refus toujours possible). Les talents des rangées de la famille devront lui donner une utilité hors duel (piste : riposte contre les troupes).
  - Un Serment **ajoute** un bonus à une source d'Honneur, sans jamais retirer les autres.
  - Le sceau adverse visible (D103) annonce quel comportement l'Aube va chercher.
  - **Talents des rangées 2 à 6 (prototype) *(D110)* :** grille complète proposée d'un bloc, puis amendée case par case. Règles : les rangées 2 et 3 donnent des bonus simples et lisibles ; les rangées 4 à 6 modifient des capacités (la *Bannière* arrive au niveau 4, le talent clé au niveau 6) ; les trois talents d'une rangée ont une valeur comparable.

    **Grille validée *(D111)*** (valeurs indicatives, à régler en test) :

    | Rangée | Gardien | Capitaine | Champion |
    |---|---|---|---|
    | 2 | ***Moisson bénie*** : rayon du *Sanctuaire* +25 % | ***Boucliers levés*** : +1 armure dans l'*Aura de l'Aube* | ***Lame bénie*** : hors duel, ~1 attaque de mêlée sur 4 contre le Paladin est contrée automatiquement |
    | 3 | ***Vigie de l'Aube*** : tours +15 % vision, +10 % portée | ***Mains du guérisseur*** : Moines dans l'aura +20 % de soins | ***Endurance du croisé*** : +10 % PV, régénération hors combat doublée |
    | 4 | ***Étendard du rempart*** : murs et tours dans la zone de la *Bannière* gagnent de l'armure, tours plus rapides | ***Pas de l'Aube*** : la formation de *Serrez les rangs* peut avancer à vitesse réduite sans perdre sa réduction de dégâts | ***Cri du défi*** : lancer un défi donne 1 cumul de *Ferveur* (~15 s) aux alliés proches, quelle que soit la réponse (remplace *Marque du parjure*, jugée trop forte : elle rendait le refus presque impossible, § 10.6) |
    | 5 | ***Remparts de lumière*** : *Rempart béni* protège aussi les segments et tours adjacents | ***Grâce abondante*** : *Lumière sacrée* a une seconde charge | ***Contre parfait*** : en duel, une riposte sur une attaque forte annoncée étourdit ~0,5 s et compte double pour *Ordalie* |
    | 6 | ***Double étendard*** : deux *Bannières* en même temps | ***Frères d'armes*** : dans l'aura, le passif du *Défi du Chevalier* redirige deux fois plus de dégâts | ***Escorte du champion*** : les Chevaliers de *Renforts de l'Aube* arrivent auprès du Paladin |

    ⚠️ Vigilances : *Double étendard* + *Bannière inébranlable* (Aube « tortue », D107) ; *Escorte du champion* faible tant que *Renforts de l'Aube* (palier 2) n'est pas disponible.

    **Rangées 7 à 9 *(D195)*** (règles D194 ; valeurs indicatives, à régler en test) :

    | Rangée | Gardien | Capitaine | Champion |
    |---|---|---|---|
    | 7 | ***Bannière bénie*** (*Bannière*) : hors combat, les alliés dans sa zone régénèrent lentement leurs PV | ***Charge disciplinée*** (*Charge de l'Aube*) : les unités qui ont suivi la charge subissent ~−15 % de dégâts ~8 s après l'impact | ***Fer de lance*** (*Charge de l'Aube*) : impact du Paladin +30 % de dégâts |
    | 8 | ***Rempart éternel*** (*Rempart béni*) : durée +30 % | ***Ordre serré*** (*Serrez les rangs*) : recharge −20 % | ***Sentence publique*** (duel, *Sentence*) : chaque Faute consommée rapporte un peu d'Honneur |
    | 9 | ***Jugement des assiégeants*** (*Jugement*) : +50 % de dégâts aux engins de siège | ***Renforts aguerris*** (*Renforts de l'Aube*) : +1 Chevalier Vertueux dans l'escouade | ***Verdict des armes*** (*Jugement*) : lancé dans les ~30 s après un duel gagné, coûte ~50 % d'Honneur en moins |

    ⚠️ Vigilance : *Jugement des assiégeants* renforce l'Aube « tortue » (D107) ; contre-jeu par le contournement et les unités. *Fer de lance* et *Verdict des armes* donnent au Champion une utilité hors duel (vigilance D109).

### 13.3 Légions Noires

**Thème :** morts-vivants, nécromancie, sacrifice, terreur, corruption.
**Style :** attrition, affaiblissement, recyclage des pertes, pression persistante.

| Unité | Rôle | Traits |
|---|---|---|
| **Guerrier Damné** | frontline / berserker | massue ou hache, dégâts élevés, rage nécrotique, se consume au combat |
| **Nécromancien** | soutien / invocation | lève des Squelettes à partir des cadavres (D86), malédictions, drain de vie |
| **Spectre Assassin** | furtivité / DPS | dagues spectrales, dématérialisation, marquage de cibles, mobilité |

**Économie *(D66)* :** les unités des Légions **coûtent moins cher** que celles des autres factions (valeur à régler en test, en tenant compte de la réduction de l'*Ossuaire*). Leurs travailleurs, les Zombies, collectent plus lentement : la faction vit avec moins de revenu.

**Spécialisations de palier *(D07)* :**

| Palier | Choix (définitif) | Rôle |
|---|---|---|
| **1 — Essor** *(D126)* | ***Fosses de labeur*** : les Zombies collectent plus vite et coûtent moins cher | **le Labeur** : une Légion qui gonfle lentement |
| | ***Autel de sang*** : bâtiment **avancé**, constructible loin de la base (hors du rayon anti-rush de D22) ; forme des Guerriers Damnés et l'infanterie du socle près du front ; les cadavres durent plus longtemps dans son rayon | **l'Assaut** : une Légion qui frappe tôt |
| **2 — Puissance** *(D127)* | ***Fosse des damnés*** : Guerriers Damnés moins chers, *rage nécrotique* plus longue | **la chair** |
| | ***Tour des nécromanciens*** : Nécromanciens qui lèvent plus vite, malédictions plus fortes (distinct du Rite du Charnier, qui renforce les Squelettes) | **la nécromancie** |
| **3 — Légende** *(D184)* | ***Fosse de suture*** : débloque l'***Abomination***, colosse de chair cousue, lent et cher, qui frappe fort les bâtiments et les murs (siège vivant) ; **à sa mort, il éclate en plusieurs cadavres** (Nécromanciens, *Sacrifice*). Contre : Arbalétriers, tir concentré, le tuer loin des Nécromanciens | **finir** (D179) : l'assaut qui se nourrit de ses propres pertes |
| | ***Cimetière maudit*** : zone fixe autour de la base ; **chaque unité ennemie qui y meurt se relève en Squelette temporaire** (~30 s, plafonné) du côté des Légions. Contre : siège à distance, repli. Distinct de *Relève impie* (rayon mobile du héros, chance, alliés et ennemis) | **renverser** (D179) : attaquer les Légions chez elles leur offre une armée |

Pendant inversé de l'Aube : l'Aube choisit entre se replier et s'étendre, les Légions entre bâtir et frapper. ⚠️ Vigilance : pas de rush depuis un bâtiment avancé (règle anti-rush de D22, *Autel de sang* fragile). Valeurs à régler en test.

**Mécanique signature : CADAVRES *(décision D86)*.** Pas une ressource économique : une ressource de champ de bataille.

- **Cadavres au sol :** chaque unité morte (alliée ou ennemie) laisse un **cadavre** pendant un temps limité (~60 à 90 s, indicatif), avec un **plafond** de cadavres sur la carte pour la performance. Une unité relevée par *Relève impie* ne laisse pas de cadavre.
- **Le Nécromancien consomme un cadavre pour lever un Squelette :** unité **permanente**, faible, gratuite, qui **compte dans la population**. Les Légions bâtissent une armée durable à partir de l'attrition ; la population limite l'effet boule de neige.
- **Autres consommateurs :** *Sacrifice* du Seigneur Damné. L'Ossuaire compte les morts sans consommer les cadavres (D65).
- **Pas de déni :** aucune unité adverse ne peut détruire ou purifier les cadavres (le Moine Lumineux atténue les effets de moral, D74, D125 ; il n'agit pas sur les cadavres).
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
  - **Kit *(décision D122)* : « Le Boucher », saturer, drainer, recommencer.** Parade commune (D119) +
    - Passif : chaque coup **draine** un peu de PV.
    - ***Enchaînement*** : deux coups rapides, puis un coup final **annoncé**. Lancé pendant la recharge de la parade adverse, il sature la défense ; le coup final reste ripostable.
    - ***Frappe dévorante*** : attaque forte annoncée qui draine ~50 % des dégâts infligés.
    - ***Rage nécrotique*** : ~6 s de cadence et de drain accrus, mais le Seigneur **subit plus de dégâts** (exposition voulue, fenêtre pour une *Sentence* adverse).
    - Ultime de duel (niveau 10) ***Faim des damnés*** : ~8 s pendant lesquelles tous ses coups drainent fortement et ses attaques fortes ne peuvent plus être interrompues (elles restent annoncées).
  - Face au Juge (D121) : quand le Seigneur entre en rage, le Paladin tient, accumule ses Fautes et le punit. Synergies : *Festin* (chaque coup ajoute un cumul de *Moisson*), *Curée* (drain doublé sous 50 % de PV adverse). Valeurs à régler en test.
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
- **Arbre de talents *(D112)* : les trois Rites.** Pendant des Serments de l'Aube : chaque famille décide **à quoi servent les morts** du champ de bataille (cadavres, D86). Choisir un Rite, c'est choisir ce que rapporte chaque mort.

  | Rite | Rôle (garde-fous D08) | Contenu | Principe |
  |---|---|---|---|
  | **Rite de la Moisson** | duel / combat personnel | *Moisson*, drain de vie, *Sacrifice* sur soi | la mort nourrit le Seigneur |
  | **Rite du Charnier** | commandement | cadavres plus durables, Nécromanciens, Squelettes, *Sacrifice* explosif ; *Relève impie* hors prototype | la mort grossit l'armée |
  | **Rite de l'Effroi** | stratégique | *Aura de terreur*, Spectres (vision, marquage, harcèlement des travailleurs) | la mort répand la peur, qui précède l'armée |

  - **Rite de la Moisson *(D113)* : « Le Festin du duel ».**
    - Sceau ***Faim insatiable*** : les cumuls de *Moisson* durent plus longtemps et leur plafond monte (~+2). Garde la famille utile hors duel.
    - Talent clé ***Festin*** : en entrant en duel, le Seigneur **conserve ses cumuls de *Moisson***, convertis en puissance de duel (dégâts et drain) ; chaque coup porté en duel en ajoute un. La bataille se paie en duel : miroir d'*Ordalie* (Aube), qui porte le duel dans la bataille.
    - Lecture pour l'adversaire : un Seigneur chargé de cumuls se voit. Refuser (*Hésitation*) ou affronter un monstre ; le défier tôt, avant le carnage, devient une stratégie.
    - ⚠️ Vigilance : face à un Seigneur chargé, le refus devient la réponse évidente (sain au regard du § 10.6) ; le talent ne doit pas pour autant devenir faible. Plafond des cumuls et conversion à régler en test.
  - **Rite du Charnier *(D114)* : « La Légion d'os ».**
    - Sceau ***Os durcis*** : les Squelettes ont ~+20 % de PV ; les Nécromanciens lèvent plus vite.
    - Talent clé ***Légion d'os*** : dans l'aura du Seigneur, chaque Squelette gagne de l'attaque et de l'armure **selon le nombre de Squelettes proches** (cumuls plafonnés). Une horde faible devient un mur d'os tant qu'elle reste groupée autour de son maître : multiplicateur de commandement. Respecte les rôles de D86 (le Seigneur renforce, le Nécromancien lève).
    - Contre-jeu : dégâts de zone (comme pour *Mur de boucliers*), disperser la horde, l'attirer hors de l'aura. Valeurs à régler en test.
  - Le sceau adverse visible (D103) annonce ce que les Légions vont faire des morts.
  - **Rite de l'Effroi *(D115)* : « Terre maudite ».** Les batailles marquent la carte.
    - Sceau ***Sol profané*** : là où beaucoup sont morts (seuil de morts dans une zone), le sol devient **maudit** ~90 s ; les **unités militaires** ennemies y subissent un léger malus de moral (catégorie D74). Les travailleurs ne sont jamais touchés.
    - Talent clé ***Champ des lamentations*** : sur une terre maudite, l'*Aura de terreur* double de rayon et les morts-vivants des Légions régénèrent leurs PV. Les Légions se battent « chez elles » sur les ruines des batailles passées.
    - Effet stratégique : la carte garde la mémoire des combats ; l'adversaire évite ces zones ou doit les reprendre.
    - Contre-jeu *(révisé par D125)* : le sol maudit **ne se purifie pas** ; le Moine Lumineux atténue le malus sur les unités, *Lumière sacrée* pourrait le retirer (à trancher en test) ; sinon, combattre ailleurs ou attendre la fin (~90 s).
    - ⚠️ Vigilance : nombre de zones maudites plafonné (performance, lisibilité). Une bataille au pied des murs de l'Aube maudit son propre terrain : tension voulue, à surveiller. Valeurs à régler en test.
  - **Rite de l'Effroi — pistes écartées** (choix de l'utilisateur, 2026-10-08) : vision forte (cadavres qui voient), panique des travailleurs ou des villages (jugée ultra frustrante), *Aura de terreur* portée par un Spectre, saut du Seigneur vers un cadavre.
  - ⚠️ Vigilance : au prototype, l'Effroi n'a ni l'*Ossuaire* (palier 3) ni la corruption de zone ; son contenu repose sur l'*Aura de terreur* et les Spectres.
  - **Talents des rangées 2 à 6 (prototype)** : méthode D110. **Grille validée *(D116)*** (valeurs indicatives, à régler en test) :

    | Rangée | Moisson | Charnier | Effroi |
    |---|---|---|---|
    | 2 | ***Lame avide*** : hors duel, le Seigneur se soigne de ~10 % des dégâts qu'il inflige | ***Charnier fertile*** : les cadavres restent ~30 % plus longtemps au sol | ***Voix sépulcrale*** : rayon de l'*Aura de terreur* +20 % |
    | 3 | ***Carapace de chair*** : chaque cumul de *Moisson* donne aussi un peu d'armure | ***Maîtres des tombes*** : Nécromanciens +20 % PV, levée à plus grande distance | ***Lames spectrales*** : les Spectres Assassins infligent plus de dégâts aux ennemis sous un effet de moral négatif |
    | 4 | ***Sacrifice vorace*** : *Sacrifice* sur une unité alliée donne en plus 3 cumuls de *Moisson* | ***Éclats d'os*** : l'explosion de *Sacrifice* fait surgir 2 Squelettes **temporaires** (~20 s, serviteurs du Seigneur, D86) | ***Sacrifice maudit*** : l'explosion de *Sacrifice* maudit le sol dans un petit rayon (terre maudite sans seuil de morts) |
    | 5 | ***Exécuteur*** : plus de dégâts contre les ennemis sous 30 % de PV ; chaque achèvement donne 2 cumuls | ***Levée prompte*** : dans l'aura, les Nécromanciens lèvent sans temps de canalisation | ***Effroi tenace*** : les malus de l'*Aura de terreur* persistent ~5 s après la sortie de l'aura |
    | 6 | ***Curée*** : en duel, sous 50 % de PV adverse, le drain de vie du Seigneur double | ***Commandant des morts*** : dans l'aura, l'explosion de *Sacrifice* ne consomme plus le cadavre | ***Hurlement funèbre*** : quand le Seigneur tue une unité, les ennemis militaires proches subissent un bref malus d'attaque (~5 s, cumul plafonné D74) |

    ⚠️ Vigilances : *Éclats d'os* puis *Relève impie* (hors prototype) multiplient les serviteurs temporaires (plafond commun) ; *Sacrifice maudit* + *Champ des lamentations* forment une synergie forte ; *Commandant des morts* fait de chaque cadavre une bombe puis un Squelette.

    **Rangées 7 à 9 *(D196)*** (règles D194 ; valeurs indicatives, à régler en test) :

    | Rangée | Moisson | Charnier | Effroi |
    |---|---|---|---|
    | 7 | ***Moisson des relevés*** (*Relève impie*) : chaque unité relevée donne 1 cumul de *Moisson* | ***Relève nombreuse*** (*Relève impie*) : plafond de serviteurs relevés +2 | ***Relevés effrayants*** (*Relève impie*) : les unités relevées portent une petite aura de malus de moral (rayon court, sans cumul avec l'*Aura de terreur*) |
    | 8 | ***Frappe insatiable*** (duel, *Frappe dévorante*) : drain ~50 % → ~65 % | ***Linceul*** (*Sacrifice*) : une unité alliée sacrifiée laisse un cadavre | ***Marque funeste*** (marquage des Spectres Assassins) : une cible marquée qui meurt maudit le sol autour d'elle |
    | 9 | ***Grand festin*** (*Sacrifice*) : soin du Seigneur +50 % | ***Ossuaire profond*** (*Ossuaire*) : capacité de la jauge +30 % | ***Bile maudite*** (*Cracheur de bile*) : la flaque d'acide maudit le sol pendant sa durée |

    ⚠️ Vigilances : *Relève nombreuse* et le plafond commun des serviteurs temporaires (performance, D190) ; *Linceul* + *Commandant des morts* (recyclage très fort) ; *Marque funeste* et *Bile maudite* dans le plafond des terres maudites (D115), travailleurs jamais touchés.

### 13.4 Enfants du Dragon

**Thème :** dragons, élémentalisme, adaptation. **Aucune bête dans la faction *(décision D149)*** : ni Dompteur ni compagnons ; les seuls dragons sont celui du héros et ceux de l'*Appel de la Couvée*.
**Style :** polyvalence, contrôle du terrain, choix d'élément.

| Unité | Rôle | Traits |
|---|---|---|
| **Champion Draconique** | frontline / dégâts | épée à deux mains, feu ou foudre (choisi par unité, D78), attaque en cône, forte présence au corps-à-corps |
| **Mage Élémentaire** | dégâts / contrôle à distance | feu, glace ou foudre (choisi par unité, D78), zones élémentaires, effets selon l'élément |
| **Garde d'écailles** *(D150)* | tank / infanterie lourde | armure d'écailles de dragon ; ***Écailles adaptatives*** : s'adapte au type de dégâts qu'elle reçoit le plus (mêlée ou distance), gagne progressivement de l'armure contre lui et bascule avec un délai si l'adversaire change d'approche (règle de délai de la faction, D72, D78). Sans micro. Contre-jeu : la frapper des deux types à la fois, ou changer d'attaque pour la prendre à contre-pied. Remplace le Dompteur de Bêtes (D149) |

**Économie *(D68, D72, D73)* : économie élémentaire.** Pas d'affinité de terrain (D40).

- **Le *Nid élémentaire* *(D72, nom temporaire)*** : bâtiment propre à la faction, réglé sur **un élément** (feu, glace ou foudre). Chaque Nid **bonifie la collecte de la ressource liée à son élément** (~ +10 % par Nid, valeur indicative).
- **Plusieurs Nids, effets cumulés :** le bonus s'additionne d'un Nid à l'autre (3 Nids en feu ≈ +30 % sur la ressource du feu). Le joueur peut **répartir** ses Nids entre plusieurs éléments ou tout miser sur un seul.
- **Changer l'élément d'un Nid est gratuit, mais pas immédiat** (façon citernes byzantines d'AoE4) : le nouveau réglage ne devient actif qu'après **X s** (à régler en test) ; **pendant ce délai, l'ancien réglage continue de produire**. La faction s'adapte sans trou de production, mais jamais instantanément.
- Les Nids sont des **cibles** : en raser un retire son bonus.
- L'élément des Nids ne touche que l'économie : l'élément du héros (*Souffle*, *Aura draconique*) et celui de chaque Mage Élémentaire restent indépendants.
- **Correspondance *(D73)* :** **feu → or**, **glace → pierre**, **foudre → bois** ; la **nourriture n'est jamais bonifiée** (pas de spam d'unités de base nourri par les Nids). Elle reprend les postures du duel (D49) : la glace défensive bâtit les fortifications, le feu offensif paie les unités avancées, la foudre rapide alimente production et expansion. La pierre reste à conquérir : le bonus multiplie une collecte existante, il faut toujours tenir les gisements.
- ⚠️ Vigilance : le cumul est sans plafond ; le coût d'un Nid (ou un plafond) devra empêcher qu'une ressource soit démultipliée. À régler en test.

**Spécialisations de palier *(D07)* :**

| Palier | Choix (définitif) | Rôle |
|---|---|---|
| **1 — Essor** *(D151)* | ***Foyer élémentaire*** : délais de changement d'élément (héros, emblématiques, Nids) ~−40 % ; Mages Élémentaires ~+10 % de portée | **les arcanes** : une armée qui change de réponse très vite |
| | ***Forge d'écailles*** : Champions Draconiques et Gardes d'écailles formés ~20 % plus vite, +1 armure ; *Écailles adaptatives* ~40 % plus rapides | **les écailles** : des guerriers qui encaissent en s'adaptant |
| **2 — Puissance** *(D152)* | ***Perchoir du dragon*** : le dragon a les statistiques de 2 niveaux de plus ; bascule plus rapide ; recharge du *Souffle* ~−20 % | **le ciel** : le héros monté |
| | ***Pierre de résonance*** : *Aura draconique* ~+30 % de rayon, effet élémentaire ~+30 % | **le sol** : le héros à pied qui commande |
| **3 — Légende** *(D191)* | ***Cercle des invocateurs*** : les Mages Élémentaires gagnent le rituel ***Météore*** : trois Mages proches canalisent ~5 s, puis un bolide frappe une **zone annoncée** (gros dégâts aux bâtiments et aux murs, plus l'effet de l'élément des Mages). Contre : tuer ou interrompre les Mages, sortir de la zone | **finir** (D179) : le siège du Dragon, par ses arcanes |
| | ***Nids gardiens*** : les *Nids élémentaires* projettent leur élément sur les ennemis proches (feu : brûlure ; glace : ralentissement, jamais d'immobilisation ; foudre : chaîne). Changer l'élément d'un Nid change sa défense (délai D72). Contre : siège à distance ; raser un Nid retire aussi son bonus économique | **renverser** (D179) : l'économie devient défense adaptable |

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
  - **Kit *(décision D148)* : « Le wyrm veille », la Bête reste présente.** Parade commune (D119) + éléments à la place des postures (D49) +
    - ***Mue élémentaire*** : change d'élément **instantanément** (sans délai) et libère un éclat de l'élément choisi : brûlure (feu), bouclier (glace) ou interruption (foudre). Recharge moyenne. Le cœur du kit : changer au bon moment.
    - ***Bond draconique*** (mobilité, D120) : bond en arrière ou vers l'avant ; l'atterrissage applique l'élément actif (par exemple, ralentissement en glace).
    - ***Ombre du wyrm*** : le dragon passe au-dessus du cercle ; son ombre annonce le passage (~1 s), puis il souffle l'élément actif sur l'adversaire. Attaque forte annoncée, **parable**, longue recharge.
    - Ultime de duel (niveau 10) ***Furie du ciel*** : **trois passages** du dragon, chacun annoncé par son ombre ; le héros peut **changer d'élément entre deux passages** pour en changer l'effet. L'adversaire doit parer chaque passage.
  - Le dragon est la monture du héros, pas une intervention extérieure : la protection du duel (D18) est respectée. ⚠️ Vigilance : performance et lisibilité (le dragon ne doit pas masquer le duel).
- **Capacité ultime (niveau 10) *(D36)* : *Appel de la Couvée*.** 2 à 3 dragons adultes descendent sur une zone annoncée pendant ~20 s (plafonnés, hors population).
- **Murs *(D64)* : survol libre.** Le dragon survole murs et remparts comme le reste du relief. Les bases se défendent par les tours et les tireurs postés sur les remparts (bonus de hauteur, D23). **En vol, il prend plus de dégâts à distance que le héros à pied** : dégâts normaux des tireurs et des tours, contre ×0,3 à pied (D17, D41). Le héros ne rasant pas une base (D17), le survol sert surtout à l'éclairage et au harcèlement, et coûte cher face à une base défendue.
- ⚠️ **Points de vigilance :** une couche de déplacement aérien pour un seul acteur ; lisibilité (le dragon ne doit pas masquer le champ de bataille).
- **Victoire en duel *(D35, D49)* : *Furie du wyrm*.** Le dragon fond sur l'armée adverse proche et **combat seul ~30 s** (puissance plafonnée, dégâts normaux des tireurs comme en forme montée). Ensuite, le héros peut **remonter sans délai de bascule**. Valeurs à régler en test.

**Arbre de talents *(D153)* : trois familles, les trois voies du héros.** Même forme que les autres factions (D103 : rangées de 3, sceau à 3, talent clé à 5). Aucune famille n'enferme dans un élément : les trois éléments restent présents partout.

| Famille | Thème | Spécialisation liée | Sceau (3 talents) | Talent clé (5 talents) |
|---|---|---|---|---|
| ***Le Ciel*** *(D154)* : « La Bête ailée » | forme montée : *Souffle*, *Piqué*, *Cri du wyrm*, vitesse et résistance du dragon | *Perchoir du dragon* | ***Vents porteurs*** : vol ~+15 % de vitesse, bascule ~−25 % | ***Piqué dévastateur*** (transforme *Piqué*) : l'impact **renverse** les unités touchées et le héros peut **sauter à terre directement au point d'impact**, sans bascule (le Roi-Sorcier descend de sa Bête). Zone toujours annoncée par l'ombre (D41) ; contre aux tireurs intact |
| ***La Lignée*** | commandement à pied et armée : *Aura draconique*, *Lame draconique*, Champions, Gardes d'écailles | *Pierre de résonance*, *Forge d'écailles* | ***Sang draconique*** *(D155)* : Champions Draconiques et Gardes d'écailles ~+10 % PV | ***Écho élémentaire*** *(D155)* (transforme *Aura draconique*) : quand le héros change d'élément, les unités dans l'aura **gardent l'effet de l'ancien élément ~8 s** en plus du nouveau. Chaque changement devient un pic de puissance. Contre-jeu : changement visible, reculer pendant la fenêtre |
| ***La Mue*** *(D156)* : « L'instinct du wyrm » | adaptation : changements d'élément, Nids, Mages, duel élémentaire | *Foyer élémentaire* | ***Esprit vif*** : délai de changement d'élément des emblématiques et des Nids ~−25 % ; Mages ~+5 % de portée (cumulable avec *Foyer élémentaire*) | ***Mue instinctive*** (transforme le changement d'élément du héros) : **en RTS aussi**, changement **instantané** qui libère autour du héros l'éclat de la *Mue élémentaire* (brûlure ; bouclier allié ; interruption). Recharge entre deux changements. ⚠️ Vigilance : pas d'éclats en boucle |

- **Rangées 2 à 6 *(décision D157)*** : grille validée d'un bloc (méthode D110). Valeurs indicatives, à régler en test. Rangées 7 à 9 : plus tard (bloc 4).

  | Rangée | **Le Ciel** | **La Lignée** | **La Mue** |
  |---|---|---|---|
  | **2** | ***Écailles du dragonnet*** : dragon +8 % PV | ***Lignée ancienne*** : *Aura draconique* +10 % de rayon | ***Nids de pierre*** : Nids +15 % PV, construits 15 % plus vite |
  | **3** | ***Ailes larges*** : cône du *Souffle* +10 % de portée | ***Écailles trempées*** : Gardes d'écailles +1 armure de base | ***Apprentis des arcanes*** : Mages Élémentaires formés 15 % plus vite |
  | **4** | ***Souffle ardent*** (*Souffle*) : recharge −15 % | ***Lame tranchante*** (*Lame draconique*) : +15 % de dégâts, effet de l'élément prolongé | ***Transition fluide*** (changement d'élément du héros) : délai −20 % |
  | **5** | ***Ombre immense*** (*Piqué*) : rayon d'impact +20 %, toujours annoncé | ***Cri de la lignée*** (*Aura draconique*) : Champions dans l'aura +10 % de vitesse d'attaque | ***Mue de combat*** (duel, *Mue élémentaire*) : recharge −20 % |
  | **6** | ***Atterrissage brutal*** (bascule) : en descendant à pied, choc de l'élément actif autour du héros | ***Garde du sang*** (*Aura draconique*) : Gardes d'écailles dans l'aura, adaptation conservée 50 % plus longtemps | ***Éclat amplifié*** (*Mue élémentaire*, et *Mue instinctive* si prise) : éclat +25 % |

  - Garde-fous : rien ne réduit les dégâts des tireurs contre le dragon en vol ; ni vision ni harcèlement ; la règle du délai d'élément est réduite, jamais contournée (sauf le talent clé *Mue instinctive*, pour le héros).

- **Rangées 7 à 9 *(décision D198)*** (règles D194 ; valeurs indicatives, à régler en test) :

  | Rangée | **Le Ciel** | **La Lignée** | **La Mue** |
  |---|---|---|---|
  | **7** | ***Cri perçant*** (*Cri du wyrm*) : rayon +20 % | ***Lame du wyrm*** (*Lame draconique*) : la frappe touche en arc 2 cibles de plus | ***Cri élémentaire*** (*Cri du wyrm*) : le Cri applique aussi l'effet de l'élément actif |
  | **8** | ***Piqué foudroyant*** (*Piqué*) : recharge −20 % | ***Garde d'honneur*** (*Aura draconique*) : dans l'aura, les *Écailles adaptatives* des Gardes montent d'un cran de plus | ***Mages harmoniques*** (Mages Élémentaires) : les Mages réglés sur l'élément actif du héros +10 % de dégâts |
  | **9** | ***Souffle adulte*** (*Souffle*) : dégâts +15 % | ***Champions de la couvée*** (*Aura draconique*) : dans l'aura, attaque en cône des Champions Draconiques +15 % de dégâts | ***Bond élémentaire*** (duel, *Bond draconique*) : recharge −20 % |

  - Aucun nouveau talent de délai de changement d'élément (cumul avec *Esprit vif* et *Foyer élémentaire*, D72). ⚠️ Vigilance : *Souffle adulte* renforce le harcèlement en vol (D41), contre des tireurs intact.

### 13.5 Cercle de l'Ombre

**Thème :** assassins, espionnage, manipulation, ruse.
**Style :** information, harcèlement, pièges, attaques opportunistes, faible efficacité en combat frontal prolongé.

| Unité | Rôle | Traits |
|---|---|---|
| **Maître des Ombres** | assassin | camouflage, invisibilité temporaire, attaques éclair, forte mobilité |
| **Piégeur** | contrôle / soutien | arbalète, pièges cachés à déclenchement annoncé (D159) : ralentissement, révélation, poison |
| **Illusionniste** | contrôle / confusion | copies illusoires, confusion (D37), peur ou retournement temporaire |

**Piégeur *(décision D159)* : « L'embuscade annoncée ».** Du contrôle qui ne fait pas râler : le piège est caché, mais son déclenchement laisse toujours une réaction possible. C'est la règle de la Voix (puissant, mais annoncé et esquivable) appliquée au terrain.

- **Pièges cachés :** le Piégeur pose ses pièges à l'avance. Le joueur adverse ne les voit pas, sauf avec des éclaireurs à proximité.
- **Déclenchement annoncé :** quand une unité ennemie marche sur un piège, un **signal visible et sonore (~1 s)** part avant qu'il se referme (déclic, corde qui se tend). Les unités qui sortent de la zone à temps y échappent.
- **Effet :** les unités restées dans la zone sont **ralenties fortement**, **révélées** et **empoisonnées**. Peu de dégâts : **le piège prépare, il ne tue pas**. C'est l'armée de l'Ombre qui punit.
- **Garde-fous :**
  - jamais d'immobilisation totale : les filets ralentissent, ils ne figent pas ;
  - **les travailleurs ne déclenchent pas les pièges** (pas de harcèlement de l'économie) ;
  - nombre de pièges actifs plafonné par Piégeur (pas de champ de mines) ;
  - immunité après effet (D37).
- **Arbalète :** attaque à distance modeste, pour que le Piégeur ne soit pas inutile hors de ses pièges.
- ⚠️ Vigilance : la fenêtre d'alerte (~1 s) et la portée de détection des éclaireurs sont à régler en test ; trop courte, le piège redevient injuste ; trop longue, il ne prend plus personne.
- Écartés : pièges visibles par tous (« Le terrain », perte de la surprise) ; filet ciblé sur une seule unité (« La proie », plus de préparation du terrain).

**Économie *(D69)* : le marché noir.** Le Cercle échange ses ressources au marché à un **meilleur taux** que les autres factions : une économie souple, qui s'adapte aux besoins du moment. Le marché est commun à toutes les factions (D70, § 5.5).

**Spécialisations de palier *(D07)* :**

| Palier | Choix (définitif) | Rôle |
|---|---|---|
| **1 — Essor** *(D160)* | ***Comptoir du marché noir*** *(D161)* : **ordres à seuil** au marché, exécutés automatiquement (voir ci-dessous) | **la bourse** : l'Ombre qui spécule, économie souple et rapide |
| | ***Atelier du piégeur*** : Piégeurs formés plus vite ; +1 piège actif par Piégeur ; pièges posés plus vite ; poison plus long | **le terrain** : l'Ombre qui tient la carte (passages piégés, embuscades préparées, expansion protégée) |
| **2 — Puissance** *(D160, D162)* | ***Repaire des assassins*** : Maîtres des Ombres formés plus vite ; ***Coup de grâce*** : gros dégâts contre les unités sous contrôle (ralenties par un piège, confuses, effrayées). Camouflage **non** allongé | **la lame** : l'assassin achève ce que le contrôle a préparé |
| | ***Salle des miroirs*** : Illusionnistes formés plus vite ; ***Reflets multipliés*** : +1 copie illusoire par Illusionniste, copies plus durables | **l'esprit** : plus d'illusions, plus longtemps |
| **3 — Légende** *(D192)* | ***Chambre des traîtres*** : les Maîtres des Ombres gagnent ***Tour livrée*** (d'après Antioche, 1098) : canalisation ~8 s au contact d'une **porte, tour ou mur** ennemi (Maître révélé), signal annoncé ~5 s, puis la porte **s'ouvre** ou la tour **se tait** ~20 s. Jamais les bâtiments économiques ni les travailleurs. Contre : tuer le Maître pendant la canalisation, garder ses portes | **finir** (D179) : la brèche par la ruse |
| | ***Passages secrets*** : réseau de souterrains entre les bâtiments du Cercle ; les unités entrent dans l'un et ressortent d'un autre après un court délai. Sorties visibles par tous, aucune vision. Contre : raser les entrées, attendre à la sortie | **renverser** (D179) : défendre à temps, ressortir dans le dos de l'assaillant |

Même logique que l'Aube : le palier 1 décide de la forme de la partie, le palier 2 de la composition de l'armée. Le Maître des Ombres n'est renforcé qu'au palier 2 : pas de harcèlement précoce. Valeurs à régler en test.

- **Principe du palier 2 *(D162)* : « Finir le travail ».** La règle de D159 s'étend à toute la faction : **l'Ombre ne frappe fort qu'après avoir préparé son coup**, et cette préparation (piège, confusion, peur) est toujours visible et esquivable. Le contre-jeu de l'adversaire est d'éviter le premier contrôle.
- ⚠️ Vigilance : *Coup de grâce* recoupe en partie l'aura *Murmures* de la Voix (alliés plus forts contre les unités sous contrôle). *Coup de grâce* agit sans le héros, partout ; le cumul des deux est à régler en test.
- Écartés : bonus purs (camouflage plus long, plus de dégâts : amplifie l'assassin invisible) ; *Retraite dans l'ombre* (recamouflage après une élimination) et *Éclat de miroir* (une copie détruite confond son agresseur), jugés frustrants.

- ***Comptoir du marché noir* *(décision D161)* : « Les ordres », le spéculateur.** Le Cercle peut passer des **ordres à seuil** au marché : « acheter 200 bois si le cours descend sous X », « vendre ma pierre s'il monte au-dessus de Y ». Ils s'exécutent **automatiquement**, même quand le joueur est occupé ailleurs. Le cours étant commun (D84), le Cercle **profite des mouvements causés par les autres joueurs**.
  - Pas de cumul de taux : le meilleur taux de D69 reste le seul bonus de prix.
  - Rien ne change pour l'adversaire : il ne voit que le cours, déjà public. Ni vision ni action sur son économie.
  - Nombre d'ordres actifs plafonné ; interface courte (ressource, seuil, quantité). L'IA doit savoir s'en servir.
  - Pendant de l'*Atelier du piégeur* : dans les deux cas, on prépare un coup à l'avance et le monde le déclenche (le piège sur le terrain, l'ordre sur le marché).
  - Écartés : taux encore meilleur (« Le taux », simple chiffre, cumul avec D69) ; cours deux fois plus sensible aux échanges du Cercle (« La manipulation », action sur l'économie adverse).
  - Valeurs (nombre d'ordres, quantités) à régler en test.

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
  - **Kit *(décision D158)* : « Le Menteur ».** Parade commune (D119) +
    - ***Feinte*** : fausse attaque forte annoncée, avec son indice subtil. Si l'adversaire pare ou riposte dessus, il est **exposé** ~1 s (dégâts subis accrus) et sa parade part en recharge.
    - ***Vraie menace*** : la prochaine attaque forte **porte l'indice d'une feinte** mais frappe vraiment, et fort. Le mensonge dans le mensonge : punit l'adversaire qui a appris à ignorer les feintes.
    - ***Pas de côté*** (mobilité, D120) : esquive en recul ; déclenchée pendant la préparation d'une attaque forte adverse, elle l'évite entièrement.
    - Ultime de duel (niveau 10) ***Mot de silence*** : ~4 s pendant lesquelles l'adversaire ne peut utiliser que la parade commune (contrôle bref autorisé sur les héros, D37). Contre-jeu : la parade fonctionne toujours, durée courte.
  - Lisibilité : chaque attaque annoncée porte un indice, *Vraie menace* comprise ; un joueur très attentif peut tout lire. Valeurs à régler en test.
- **Triangle de duel avec le prototype :** la feinte bat la riposte du Paladin (qui se met en garde pour rien puis s'expose) ; l'agression du Seigneur Damné bat la feinte (pas le temps de l'installer) ; la riposte bat l'agression.
- **Victoire en duel *(D35, D45)* : *Voix usurpée*.** Pendant ~45 s, la Voix prend l'**aura du héros vaincu** et l'applique à sa propre armée. Technique : application du `GameplayEffect` d'aura tiré du `HeroData` du vaincu. Valeurs à régler en test.

**Arbre de talents *(D163)* : La Lame, le Miroir, la Toile.** Même forme que les autres factions (D103 : rangées de 3, sceau à 3, talent clé à 5). Chaque famille prolonge une ou deux spécialisations de palier, comme au Dragon (D153).

| Famille | Build | Thème | Spécialisation liée | Sceau (3 talents) | Talent clé (5 talents) |
|---|---|---|---|---|---|
| ***La Lame*** *(D164)* : « Le signal » | duel / combat, commandement des assassins | kit du Menteur (*Feinte*, *Vraie menace*, *Pas de côté*), *Mot d'arrêt* (D164), Maîtres des Ombres, *Coup de grâce* | *Repaire des assassins* | ***Confrérie*** : Maîtres des Ombres ~15 % plus vite formés, ~+10 % de dégâts contre les unités sous contrôle (cumulable avec *Coup de grâce*) ; en duel, recharge de *Feinte* ~−20 % | ***Signal de la Voix*** (transforme *Mot d'arrêt*) : quand le *Mot d'arrêt* effraie des ennemis, les **Maîtres des Ombres alliés proches bondissent sur eux**, révélés pendant le bond, et frappent aussitôt. La peur reste annoncée, esquivable et encadrée par D37 ; contre-jeu : sortir de la zone, punir les assassins révélés |
| ***Le Miroir*** *(D165)* : « La rumeur » | commandement (par la manipulation) | *Murmures*, *Mensonge*, Illusionnistes, confusion, conversion (*Serment de l'Ombre*, *Discours du Maître*) | *Salle des miroirs* | ***Voix porteuse*** : *Murmures* ~+15 % de rayon ; Illusionnistes ~15 % plus vite formés | ***Murmures contagieux*** (transforme *Murmures*) : quand une unité ennemie dans l'aura subit un contrôle (confusion, peur, piège), **ses voisins immédiats reçoivent le malus de moral de *Murmures*** (D74). La panique se propage, sans contrôle supplémentaire. Contre-jeu : se disperser, purification (D125) |
| ***La Toile*** *(D166)* : « L'appât » | stratégique | Piégeurs et pièges, *Comptoir* et ordres du marché noir, *Mensonge* (partagé avec le Miroir) : préparer un coup que le monde déclenche (D159, D161) | *Atelier du piégeur*, *Comptoir du marché noir* | ***Fils tendus*** : Piégeurs ~15 % plus vite formés, +1 piège actif par Piégeur ; avec le *Comptoir*, +1 ordre actif | ***L'Appât*** (transforme *Mensonge*) : la fausse armée peut être posée **sur des pièges alliés** ; attaquée, elle se dissipe et déclenche les pièges dessous, **avec leur signal habituel (~1 s, D159)**. Contre-jeu : indice de la fausse armée (D37), éclairage, recul pendant le signal |

- **Rangées 2 à 6 *(décision D167)*** : grille validée d'un bloc (méthode D110). Valeurs indicatives, à régler en test. Rangées 7 à 9 : plus tard (bloc 4).

  | Rangée | **La Lame** | **Le Miroir** | **La Toile** |
  |---|---|---|---|
  | **2** | ***Sang-froid*** : la Voix +8 % PV | ***Chuchoteurs*** : malus de moral de *Murmures* +10 % | ***Réseau de passeurs*** : Marché −25 % de coût, construit 25 % plus vite |
  | **3** | ***Cuir noirci*** : Maîtres des Ombres +10 % PV | ***Voile tenace*** : Illusionnistes +10 % PV | ***Mains habiles*** : pièges posés 20 % plus vite |
  | **4** | ***Mot tranchant*** (*Mot d'arrêt*) : recharge −15 % | ***Mensonge tenace*** (*Mensonge*) : fausse armée +20 % de durée | ***Poison virulent*** (pièges) : le poison réduit les soins reçus de 25 % |
  | **5** | ***Pas de l'ombre*** (duel, *Pas de côté*) : recharge −20 % | ***Esprit troublé*** (confusion des Illusionnistes) : zone +15 %, durée inchangée (D37) | ***Carreaux empoisonnés*** (arbalète du Piégeur) : poison léger, sans ralentissement |
  | **6** | ***Écho du Mot*** (*Mot d'arrêt*) : rayon +15 %, toujours annoncé | ***Mots perfides*** (*Murmures*) : les ennemis qui quittent l'aura gardent le malus de moral ~3 s | ***Double détente*** (pièges) : un piège déclenché se réarme une fois après ~20 s, signal habituel (D159) |

  - Garde-fous : ni camouflage prolongé, ni vision, ni durée de contrôle allongée (D37), ni nouveau bonus contre les unités sous contrôle (vigilance D165) ; aucun gain de dégâts précoce pour le Maître des Ombres (D162) ; travailleurs jamais touchés. Pas de cumul de taux au marché (D161) : le *Comptoir* n'est servi que par *Fils tendus*.

- **Rangées 7 à 9 *(décision D199)*** (règles D194 ; valeurs indicatives, à régler en test) :

  | Rangée | **La Lame** | **Le Miroir** | **La Toile** |
  |---|---|---|---|
  | **7** | ***Sang-froid du Menteur*** (*Serment de l'Ombre*) : pendant la canalisation, la Voix subit ~−20 % de dégâts (toujours interruptible) | ***Serment élargi*** (*Serment de l'Ombre*) : +1 unité convertible, plafond de coût total inchangé | ***Pose à distance*** (pièges) : le Piégeur peut lancer un piège à courte distance (~6 m) |
  | **8** | ***Lames promptes*** (attaques éclair des Maîtres des Ombres) : recharge −15 % | ***Reflets tenaces*** (copies des Illusionnistes) : +15 % PV | ***Poison lent*** (pièges) : durée du poison +30 % (dégâts, pas contrôle) |
  | **9** | ***Vraie menace aiguisée*** (duel, *Vraie menace*) : +15 % de dégâts | ***Mensonge mouvant*** (*Mensonge*) : un ordre de déplacement possible pour la fausse armée | ***Pièges de siège*** (pièges) : +50 % de dégâts aux engins de siège |

  - Garde-fous de D167 respectés ; *Serment élargi* garde le plafond de coût (D63) ; *Mensonge mouvant* reste un objet du monde démasqué par l'éclairage (D37) ; *Pièges de siège* prépare la conversion du siège sans le détruire.

- ⚠️ Vigilance *(D166)* : l'enchaînement complet (*L'Appât*, pièges, *Signal de la Voix*, *Coup de grâce*) peut devenir mortel. Chaque étape reste annoncée et esquivable ; vérifier en test qu'il ne paraît pas injuste.
- ⚠️ Vigilance *(D165)* : les bonus contre les unités sous contrôle s'additionnent (*Murmures*, *Coup de grâce*, *Confrérie*) ; plafond à régler en test.
- ⚠️ Vigilance : le Miroir est la famille la plus riche (aura, discours, illusions, conversion) ; son talent clé devra rester mesuré. La Toile ne doit pas devenir une voie de vision : ses talents portent sur les pièges et l'économie, pas sur l'éclairage.

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
| **Lame Ardente** *(D171)* | DPS mobile | **homme d'armes** (infanterie lourde à pied, même catégorie d'armure) **plus rapide que tous les autres hommes d'armes du jeu**, avec une **charge** ; arme incandescente qui inflige de la brûlure, exigeante en micro |
| **Prêtre de la Flamme** | soutien | bénit les armes, confère de la résistance, protège des unités précieuses ; pas de magie élémentaire. **Bénédictions *(D177)* : un nombre de cibles à la fois, à sa portée** (pas une zone) |

**Poudre (D38) :** commune à toutes les factions au palier 3 (§ 7.1). Les Héritiers en ont leurs propres versions, forgées : la *Bombarde* à la place du Canon et l'*Arquebusier de la Forge* (D61), qui tire plus vite et plus loin. Ce sont leurs deux variantes du socle (D20).

**Spécialisations de palier *(D07)* :**

| Palier | Choix (définitif) | Rôle |
|---|---|---|
| **1 — Essor** *(D169)* | ***Grande Forge*** : toute la faction ~+20 % de dégâts (pas d'ignorance d'armure) | **le marteau** : on tue avant d'être touché |
| | ***Halle des serments*** : toute la faction ~+20 % de PV ; ***Résistance légendaire*** *(D170)* pour les **Gardiens de la Forge** : sous ~20 % de PV, ils subissent moins de dégâts | **l'enclume** : on encaisse tout, on ne perd personne |
| **2 — Puissance** *(D172)* | ***Salle des Lames*** : Lames Ardentes formées ~20 % plus vite ; charge ~+25 % de dégâts ; brûlure cumulable jusqu'à 3 fois | **la lame** : une charge de Lames efface une unité |
| | ***Temple de la Flamme*** : bénédictions des Prêtres de la Flamme ~+30 % (résistance, armes bénies) ; ***Garde sacrée*** : le Prêtre protège une unité précieuse, qui subit ~−30 % de dégâts pendant quelques secondes (annoncé, recharge) | **la flamme** : on ne perd personne parce qu'on est protégé |
| **3 — Légende** *(D193)* | **Aucune spécialisation** : tronc commun seul (§ 11.2) | **les statistiques font la différence** |

- **Principe du palier 1 *(D169)* : amplifier l'élite, jamais adoucir les pertes.** La faction est chère et peu nombreuse, chaque perte doit coûter ; en contrepartie, chaque unité est très forte. Les deux voies renforcent une moitié de ce principe : frapper plus fort ou ne pas tomber.
- **Palier 3 *(D193)* : aucune spécialisation** *(choix de l'utilisateur)*. Les Héritiers n'ont « rien de plus » : ils reçoivent le tronc commun du palier 3 (dont *Bombarde* et *Arquebusier de la Forge*), et leurs statistiques d'élite font la différence. Exception assumée à D07 et D179 (4 combinaisons de spécialisations au lieu de 8), cohérente avec D39. ⚠️ Vigilance : les autres factions gagnent un outil de fin de partie au palier 3 ; l'équilibrage de fin de partie des Héritiers repose sur leurs statistiques, à vérifier en test.
- **Principe du palier 2 *(D172)* : « La lame ou la flamme ».** Choix de composition, comme pour l'Aube et les Légions : l'élite qui frappe ou l'élite protégée. ⚠️ Vigilance : cumul *Halle des serments* + *Temple de la Flamme* sur un Gardien de la Forge (PV, *Résistance légendaire*, *Garde sacrée*) ; valeurs à régler en test.
- *Résistance légendaire* : réservée au Gardien de la Forge (D170), la frontline qui tient seule une ligne ; c'est une statistique, sans mécanique supplémentaire (D39). La réduction de dégâts sous le seuil est visible (l'armure rougeoie), pour que l'adversaire sache pourquoi l'unité tient. Seuil et réduction à régler en test.

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
  - **Kit *(décision D168)* : « Le Sans-pair », la supériorité qui s'installe.** Plus le duel dure, plus il écrase. Parade commune (D119) +
    - ***Ascendant*** (passif) : plus la Chaleur monte, plus ses coups **traversent la parade adverse** (elle réduit de moins en moins). L'arme rougit à vue d'œil : l'écart se lit. Effet plafonné : la parade garde toujours une utilité.
    - ***Frappe de forge*** : frappe chargée, la plus forte du jeu, longue annonce. Consomme la Chaleur, donc l'*Ascendant* retombe.
    - ***Mépris*** : il baisse sa garde ~3 s, sans parade ; chaque coup normal reçu lui donne de la Chaleur. Une attaque forte bien placée le punit durement : l'arrogance de l'élite force l'adversaire à prendre un risque.
    - **Pas de capacité de mobilité** (D120) : il ne recule pas.
    - Ultime de duel (niveau 10) ***Jugement de l'acier*** : très longue annonce, puis un coup que la parade ne réduit que de moitié ; il achève l'adversaire sous ~30 % de PV.
  - Contre-jeu : le battre tôt, avant que l'*Ascendant* s'installe, et punir *Mépris*. Valeurs (plafond de l'*Ascendant*, seuil du *Jugement*) à régler en test.
- **Victoire en duel *(D35, D47)* : *Armes chauffées à blanc*.** Pendant ~45 s, les alliés proches ont des armes incandescentes (bonus de dégâts et brûlure). L'effet profite à l'armée et compense son aura faible. Valeurs à régler en test.

**Arbre de talents *(D173)* : le Marteau, l'Enclume, la Flamme.** Même forme que les autres factions (D103 : rangées de 3, sceau à 3, talent clé à 5). Les familles reprennent les mots des spécialisations (D169, D172) : le lien entre spécialisation et arbre se lit tout de suite.

| Famille | Thème | Spécialisation liée | Sceau (3 talents) | Talent clé (5 talents) |
|---|---|---|---|---|
| ***Le Marteau*** *(D174)* : « Le coup qui décide » | duel et combat : Chaleur, *Ascendant*, *Frappe de forge*, *Jugement*, Lames Ardentes | *Grande Forge*, *Salle des Lames* | ***Acier de lignée*** : Chaleur ~15 % plus rapide (RTS et duel) ; charge des Lames Ardentes ~+10 % de dégâts | ***Frappe fendante*** (transforme *Frappe de forge*) : lancée à pleine Chaleur, la frappe devient une **onde en cône** qui touche toutes les unités devant le Champion. Cône annoncé et visible, esquivable ; n'ignore pas l'armure (D169) |
| ***L'Enclume*** *(D175)* : « La lignée tient » | tenir, ne perdre personne : survie du Champion, Gardiens de la Forge, *Résistance légendaire*, *Cor de l'Héritage* | *Halle des serments* | ***Trempe des serments*** : Gardiens de la Forge +1 armure ; Champion ~+10 % PV | ***Exemple inébranlable*** (transforme *Exemple*) : les alliés proches du Champion gagnent une *Résistance légendaire* atténuée (sous ~20 % PV, dégâts subis réduits, moins que chez les Gardiens) ; pas de cumul avec celle des Gardiens (la plus forte s'applique). Visible (l'armure rougeoie) ; contre-jeu : burst, contrôle, attirer le Champion ailleurs |
| ***La Flamme*** *(D176)* : « La Chaleur qui circule » | commandement : *Transmission*, Prêtres de la Flamme, *Armes chauffées à blanc* | *Temple de la Flamme* | ***Flamme des pères*** : Prêtres de la Flamme formés ~15 % plus vite ; recharge de *Transmission* ~−15 % | ***Héritage vivant*** (transforme *Transmission*) : les armes embrasées **gardent la flamme tant que l'unité combat** (chaque coup la prolonge, plafond ~15 s) et chaque coup d'une unité embrasée **rend un peu de Chaleur au Champion**. Contre-jeu : se désengager, la flamme s'éteint dès que l'unité ne frappe plus |

- ⚠️ **Vigilance (D173) :** l'Enclume aide à survivre, jamais à être remboursé ni à couvrir plus de terrain ; elle n'adoucit ni le coût des pertes ni « je ne peux pas être partout » (D169).
- **Rangées 2 à 6 *(décision D177)*** : grille validée avec amendements (méthode D110). Valeurs indicatives, à régler en test. Rangées 7 à 9 : plus tard (bloc 4).

  | Rangée | **Le Marteau** | **L'Enclume** | **La Flamme** |
  |---|---|---|---|
  | **2** | ***Poigne héritée*** : Champion +8 % de dégâts | ***Cuir de forge*** : Gardiens de la Forge +8 % PV | ***Encens de la forge*** : chaque Prêtre de la Flamme bénit **+1 cible à la fois** à sa portée |
  | **3** | ***Élan des Lames*** : dégâts de charge des Lames Ardentes +10 % | ***Plates de lignée*** : Champion +1 armure | ***Prières ferventes*** : bénédictions des Prêtres +10 % de durée |
  | **4** | ***Trempe rapide*** (*Frappe de forge*) : recharge −15 % | ***Exemple tenace*** (*Exemple*) : rayon de l'aura +10 % | ***Flamme large*** (*Transmission*) : rayon d'effet +20 % |
  | **5** | ***Ascendant précoce*** (duel, *Ascendant*) : agit dès un niveau de Chaleur plus bas, plafond inchangé | ***Mépris d'acier*** (duel, *Mépris*) : pendant *Mépris*, coups normaux reçus −15 % de dégâts | ***Gloire de la lignée*** (*Armes chauffées à blanc*) : durée +20 % |
  | **6** | ***Braises*** (*Frappe de forge*) : après la frappe, ~25 % de la Chaleur reste | ***Mur de la lignée*** (*Exemple*) : Gardiens de la Forge dans l'aura +2 armure | ***Feu des Prêtres*** (*Transmission*) : près d'un Prêtre de la Flamme, l'embrasement dure +30 % |

  - ⚠️ Vigilances : *Braises* rend la *Frappe fendante* plus vite disponible (voulu, à surveiller) ; *Mur de la lignée* s'ajoute à la *Halle des serments* et au *Temple de la Flamme* sur le Gardien (D172).

- **Rangées 7 à 9 *(décision D200)*** (règles D194 ; valeurs indicatives, à régler en test) :

  | Rangée | **Le Marteau** | **L'Enclume** | **La Flamme** |
  |---|---|---|---|
  | **7** | ***Cor de guerre*** (*Cor de l'Héritage*) : le Cor remplit ~30 % de la Chaleur | ***Cor du rempart*** (*Cor de l'Héritage*) : les alliés touchés par le regain de moral subissent ~−10 % de dégâts ~8 s | ***Cor de la flamme*** (*Cor de l'Héritage*) : le Cor embrase ~5 s les armes des alliés touchés, sans dépenser de Chaleur |
  | **8** | ***Frappe chauffée à blanc*** (*Frappe de forge*) : à pleine Chaleur, la frappe applique une brûlure | ***Serment tenu*** (Gardiens de la Forge) : hors combat, régénération lente des PV | ***Bénédiction de la forge*** (bénédictions des Prêtres) : unités bénies ~+10 % de cadence |
  | **9** | ***Bombarde de lignée*** (*Bombarde*) : +15 % de dégâts aux bâtiments | ***Arquebusiers gardés*** (*Arquebusier de la Forge*) : près d'un Gardien de la Forge, ~−15 % des dégâts des tirs | ***Salve embrasée*** (*Transmission*) : sur des Arquebusiers de la Forge, leurs tirs brûlent |

  - Fidèle à D169 : protéger avant la chute, jamais rembourser une perte ; ni mobilité ni couverture de plusieurs fronts. ⚠️ *Cor de la flamme* recoupe un peu *Transmission* (plus court, lié au Cor).

---

### 13.7 Templiers *(décision D87)*

**Sixième faction, à part entière.** Ordre militaire et religieux, fondé entre autres sur l'**appel à la croisade**.

**Principe *(décision D93)* : une faction « lore accurate », quasiment sans fantasy.** Les Templiers sont l'ordre historique : pas de magie, pas de surnaturel, pas de créatures. Leur foi s'exprime par le **moral** (D74), la **discipline**, l'**organisation** (commanderies, or, frères convers) et des faits d'armes historiques. Dans un monde de dragons, de morts-vivants et de magie, c'est ce réalisme qui les rend reconnaissables (pilier 5). Tout élément de conception templier doit passer ce filtre.

**Distinction avec l'Ordre de l'Aube :** deux factions de chevaliers saints, deux doctrines. L'Aube **tient et protège** (défense, Honneur, remparts) ; les Templiers **partent en croisade** (offensive, guerre sainte). La différence doit se lire au rythme de jeu **et à l'œil** : silhouettes, couleurs et architecture nettement distinctes de l'Aube (⚠️ point de vigilance, pilier 5).

**Point fort (timing) *(décision D130)* : solides en continu, sommet pendant une croisade.** Les Templiers sont une faction offensive **compétitive à tout moment de la partie** (noyau fixe, frères convers, citadelles, commanderies) ; ils n'ont **pas de creux marqué** pendant la recharge de l'appel. Leur **pic de puissance**, c'est la croisade : armée réelle + troupes de croisade + bonus de moral, des croisades de plus en plus fortes au fil des paliers (commanderies, gloire, citadelles).
- Conséquence : le contre-jeu principal porte sur **la croisade elle-même** (défendre la cible, intercepter la colonne, tenir jusqu'à la fin du délai), pas sur une fenêtre de faiblesse après elle.
- ⚠️ Vigilance d'équilibrage : la force « hors croisade » doit rester dans la moyenne des factions, sinon le pic s'ajoute à une base trop haute. La recharge et le contrecoup d'un échec (*Désillusion*) restent les leviers de réglage.

**Mécanique signature : L'APPEL À LA CROISADE *(décision D88)* — une cible, un enjeu.**

- Le joueur **désigne une cible** : bâtiment ou centre ennemi, ou point stratégique. La croisade est **annoncée à l'adversaire**, qui voit la cible.
- L'appel **réunit une armée de croisade** au centre principal ou à la citadelle la plus proche de la cible (D96), qui marche vers la cible. L'armée qui avance vers la cible reçoit un effet de moral (D74).
- **Succès** (cible détruite ou prise) : un **rang de gloire** (D90), beaucoup d'**or** (D91) et *(décision D138)* une **XP modérée pour le héros**, comptée comme un fait d'armes (D05), même s'il n'a pas suivi la croisade ; **recharge normale**. **Échec** (délai écoulé) : pas d'XP, contrecoup (*Désillusion*, malus de moral) et **longue recharge**. Un succès ne raccourcit pas la recharge (pas d'accélération de l'effet boule de neige).
- **Les commanderies définissent la croisade *(précision de l'utilisateur)* :** au fil de la partie, le Templier **débloque des commanderies** ; ce sont elles qui déterminent **la composition** de l'armée de croisade. Deux Templiers n'appellent pas la même croisade.
- Contre-jeu : défendre la cible, tenir jusqu'à la fin du délai, intercepter l'armée en marche.
- **Troupes de croisade *(D89)* : autonomes, hors population, éphémères.**
  - Elles **marchent au plus court vers la cible**, sans ordres possibles, et **disparaissent** à la victoire ou à l'échec de la croisade.
  - Le joueur accompagne la croisade avec sa vraie armée : c'est la combinaison des deux qui fait la force.
  - Contre-jeu lisible : colonne et chemin visibles ; embuscade, ralentissement par les murs, ou tenir jusqu'à la fin du délai.
  - Distinction avec l'Aube : l'Aube appelle des renforts qu'elle dirige (*Renforts de l'Aube*, D85) ; les Templiers déclenchent une croisade qu'ils ne retiennent plus.
  - **Contingent maltais *(D189)* :** au palier 3, +3 Canons maltais et +5 Arquebusiers maltais par croisade, **hors enveloppe** (maximum ~48).
  - **Taille de la croisade *(décision D139)* :** une **population propre**, totalement indépendante de celle du joueur, fondée sur le **nombre de commanderies** : **~10 croisés par commanderie** *(précision de l'utilisateur ; indicatif, à régler en test)*. Hypothèse de travail : le socle de *Pèlerins armés* (palier 0) compte comme une commanderie, soit ~40 croisés au maximum en fin de partie (socle + 3) ; le Sénéchal, sans contingent (D135), n'ajoute rien. Ce plafond borne aussi la cible de performance (D03).
  - **Les bonus remplissent l'enveloppe *(décision D140)* :** les ~10 par commanderie sont un **maximum**. Un contingent de base arrive **en dessous** (par exemple 6 sur 10, indicatif) ; les **citadelles** (D96), le **Prêcheur** (D135) et la **gloire** (D90) le complètent jusqu'au maximum ; **au-delà, le surplus devient de la vétérance** (PV et attaque accrus, visuel de vétéran). Chaque bonus garde ainsi son intérêt jusqu'en fin de partie, et raser une citadelle fait vraiment maigrir les croisades.
  - **Face aux murs *(décision D141)* :** les croisés prennent le **chemin existant le plus court** (brèche, porte ouverte). À défaut, ils attaquent **la porte ou le pan de mur le plus proche de la cible** ; les engins et les contingents de la voie Pierre s'en chargent en priorité (le *Beffroi* se colle au mur, le *Frère maçon* sape). Les murs font perdre du temps sur le délai (contre-jeu), sans rendre la croisade inutile ; la voie Pierre est précieuse contre un adversaire fortifié, pas obligatoire.
  - La perte de contrôle est assumée, comme pour une invocation temporaire.
  - Piste : un talent ou une capacité du héros templier pour que les croisés le suivent.
- **Commanderies *(D90)* : les paliers donnent l'accès, les victoires donnent la gloire.**
  - **Palier 0 :** la croisade n'a qu'une base, par exemple des *Pèlerins armés* (masse légère).
  - **Paliers 1, 2 et 3 :** le Templier choisit **une commanderie parmi 3**, définitivement. *(Décision D131)* : les trois commanderies proposées sont **fixes pour chaque palier** (9 commanderies au total, 27 croisades possibles) ; chaque commanderie n'est équilibrée que face aux deux autres de son palier. Chacune **ajoute un contingent** à la croisade. Pour les Templiers, **ce choix tient lieu de spécialisation de palier** (D07) : pas de double choix.
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
- **Trois voies *(décision D132)* :** à chaque palier, une commanderie de chaque voie : **Fer** (mêlée, choc), **Foi** (soutien, moral), **Pierre** (tir, siège). On peut suivre une voie ou les mélanger. **Chaque commanderie débloque une unité propre aux Templiers**, jamais une unité du socle commun ni un simple renfort de celui-ci *(nuance de l'utilisateur)*.
- **Grille des unités de commanderie *(décision D133)*** (9 au total, 3 fixes par palier, D131 ; noms provisoires, valeurs à régler en test). Chaque unité ajoute aussi son contingent à la croisade. Chaque partie montre une armée templière différente.

  | Palier | **Fer** (mêlée, choc) | **Foi** (soutien, moral) | **Pierre** (tir, siège) |
  |---|---|---|---|
  | 1 | ***Sergent du Temple*** | ***Frère infirmier*** | ***Arbalétrier à pavois*** |
  | 2 | ***Sergent à cheval*** | ***Prêcheur de croisade*** *(D135)* | ***Frère maçon*** |
  | 3 | ***Maréchal du Temple*** (unique) | ***Sénéchal*** (unique, hors croisade) *(D135)* | ***Beffroi*** |

  - ***Sergent du Temple*** (infanterie à lance) : *Ouvrir les rangs* : la cavalerie alliée traverse leur ligne sans être bloquée et gagne un bonus de charge en en sortant. Léger anti-cavalerie, moindre que le Lancier.
  - ***Sergent à cheval*** (cavalerie moyenne) : *Seconde vague* : plus de dégâts contre les ennemis engagés ou renversés par un Chevalier du Temple. Exploite la charge, ne la mène pas.
  - ***Maréchal du Temple*** (officier unique, cavalerie lourde) *(D133, D137)* :
    - **Recruté :** *Escadron* : les Chevaliers du Temple autour de lui chargent ensemble, avec un bonus. S'il accompagne une croisade, **les croisés le suivent** au lieu de marcher seuls vers la cible (répond à la piste de D89) ; s'il meurt, la croisade reprend sa marche automatique.
    - **Contingent *(D137)* :** un **escadron de Chevaliers du Temple vétérans** qui chargent ensemble (*Escadron*) dans chaque croisade.
    - Au palier 3, trois choix nettement opposés : Maréchal (croisade plus forte et dirigeable), Sénéchal (second front), Beffroi (prise des murs). *(D185, « Finir ou renverser », D179)* : **Maréchal et Beffroi pour finir, Sénéchal pour renverser** (*Contre-croisade*).
  - ***Frère infirmier*** (soin) : ne soigne que les unités **hors combat depuis quelques secondes**, mais vite, y compris la colonne de croisade en marche. Pas de magie.
  - ***Prêcheur de croisade*** *(D135)* (soutien) : **contingent** : toutes les croisades **grossissent en marchant** (des pèlerins rejoignent la colonne) ; c'est la valeur de cette commanderie. Favorise les cibles lointaines ; contre-jeu : intercepter tôt, tuer le Prêcheur de la colonne. **Recruté** : *Sermon*, effet de moral bref sur un groupe. ⚠️ Plafond commun avec le bonus des citadelles (D96) dans le plafond de croisade (D89).
  - ***Sénéchal*** *(D135)* (officier unique, second du Grand Maître) : **exception à la règle du contingent : il n'entre pas dans la croisade.** Son pouvoir : ***Mini-croisade*** *(proposition de l'utilisateur)* : il prend avec lui **une partie de l'armée** et lance sa propre croisade ; les troupes engagées deviennent autonomes ; **en cas de victoire, elles redeviennent contrôlables**. Distinction avec *Prendre la croix* (héros, D97) : celle-ci rejoint la croisade en cours, le Sénéchal en crée une seconde, plus petite.
    - **Règles *(décision D136)* : une croisade en miniature.** **Taille :** au plus **~30 % du maximum d'une croisade** *(précision de l'utilisateur ; indicatif, à régler en test)*. Les troupes de la mini-croisade **restent dans la population du joueur** (D139) : ce sont ses propres unités. **Annonce :** cible annoncée à l'adversaire, comme l'appel (D88). **Succès :** or réduit, **sans rang de gloire** (réservé à l'appel principal, D90) ; troupes de nouveau contrôlables. **Échec** (délai écoulé ou Sénéchal tué) : troupes **rendues au joueur** avec *Désillusion*, longue recharge. **Recharge propre**, indépendante de l'appel : deux fronts possibles.
    - Conséquence : choisir le Sénéchal au palier 3, c'est échanger un contingent de plus dans la grande croisade contre un second front.
  - **Poudre maltaise *(décision D187)* :** au palier 3, les Templiers ont l'*Arquebusier maltais* et le *Canon maltais* (noms provisoires), plus primitifs que la poudre du socle, donc moins forts. Ils **s'intègrent à la croisade** *(D189)* : à partir du palier 3, chaque croisade reçoit un **contingent maltais hors enveloppe** de **3 Canons maltais et 5 Arquebusiers maltais** *(précision de l'utilisateur)*, soit ~48 croisés au maximum. ⚠️ Vigilance : la croisade grossit à son sommet (D130) et le plafond de performance (D139) recule ; valeurs à régler en test. **Statut *(D188)* :** écart de statistiques (moins forts **et moins chers**), pas une variante (§ 7.1).
  - ***Reliquaire*** *(décision D186)* : **unité fixe du palier 3, hors commanderies**, débloquée automatiquement (D180) pour tous les Templiers. Char processionnel **unique, lent et cher**, portant une relique (une relique de l'Ordre, **pas la Vraie Croix**, *précision de l'utilisateur*).
    - **Effet :** les alliés proches gagnent **de l'armure** et **~+25 % de dégâts aux bâtiments et aux murs**. Pas de moral : il reste au Porte-gonfanon (distinction).
    - **Procession :** s'il accompagne une croisade, celle-ci marche un peu plus lentement mais son **délai est allongé** (~+20 %) : cibles lointaines ou fortifiées.
    - **Non réparable** *(nuance de l'utilisateur)* : ses PV perdus ne reviennent pas (ni frères convers, ni Frère maçon, ni soins). Détruit, il peut être reconstruit après un long délai ; **sa perte n'applique pas *Désillusion*** *(nuance de l'utilisateur)*.
    - Contre-jeu : l'abattre (lent, visible, chaque dégât compte). Rôle : surtout **finir**, chaque commanderie l'utilise à sa manière.
    - ***Contre-croisade*** *(décision D185, « les cloches »)* : quand les Templiers **repoussent un assaut** contre une citadelle ou leur base, la mini-croisade du Sénéchal est **aussitôt disponible** (recharge remise à zéro), avec pour cible le camp ou la base de l'attaquant. Déclencheur : un succès défensif, jamais un retard (pas de *rubber band*, D179).
  - ***Arbalétrier à pavois*** (tir) : *Planter le pavois* : immobile et très résistant aux tirs. Fort en siège et pour tenir une ligne ; plus lent que l'Arbalétrier du socle.
  - ***Frère maçon*** (ingénieur) : *Sape* : creuse sous un mur ou une tour, qui s'effondre après un délai visible ; l'adversaire l'interrompt en tuant les sapeurs. Répare aussi.
  - ***Beffroi*** (tour de siège mobile, absente du socle) : collé à un mur, il laisse les troupes **passer par-dessus**. Avec le Frère maçon, piste de réponse au comportement de la croisade face aux murs (D89).
  - **Règle de conception *(rappel de l'utilisateur, D90, D92)* :** la croisade se compose **automatiquement** à partir des commanderies choisies. Chaque unité de commanderie se conçoit donc **sous deux formes** : **recrutée** (contrôlée, dans la population) et **contingent** (autonome, présent dans **chaque** croisade). Un effet porté par le contingent devient un effet permanent de toutes les croisades.
  - Écartés *(D133, D135)* : *Frère chapelain* (*Confession*), *Frère pénitent* (*Rachat*), *Vraie Croix* (trop proche du Porte-gonfanon ; relique non recrutable ; *revue par D185 et D186 : un **Reliquaire** (une relique, pas la Vraie Croix) est retenu comme unité fixe du palier 3, sans moral*), *Frère drapier*. Anciens noms de travail abandonnés : commanderies de l'Hôpital (autre ordre), des Zélotes (non historique), des Arbalétriers et des Bâtisseurs (unités du socle).
- **Exception assumée** à « 3 emblématiques par faction » : 2 fixes + 3 débloquées par les commanderies (une par palier).
- **Écarté :** archer monté (*Turcopole*), à la demande de l'utilisateur.
- ⚠️ Vigilance : beaucoup d'unités à concevoir (9 + 2), et à distinguer du socle commun et de l'Aube (Sergent du Temple ≠ Homme d'armes, Frère infirmier ≠ Moine Lumineux).

**Citadelles templières *(D95)* :** bâtiment propre aux Templiers, **construit librement** (règles de construction de D22). C'est une **place forte** (solide, garnison) **et** elle a un **effet sur la civilisation** :
  - **Les citadelles nourrissent la croisade *(D96)* :** chaque citadelle **grossit chaque appel** (+X croisés par citadelle, plafonné ; valeurs à régler en test).
  - **Les croisades se rassemblent à la citadelle la plus proche de la cible** (à défaut, au centre principal, D88).
  - Raser une citadelle affaiblit toutes les croisades suivantes : c'est une cible stratégique claire pour l'adversaire.
  - Plafond : le bonus des citadelles remplit l'enveloppe des contingents, le surplus devient de la vétérance (D139, D140). Pistes écartées : citadelle fondée par une croisade réussie ; citadelle-trésor.

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
  - **Kit *(décision D142)* : « Le Martyr », la puissance par la douleur.** Parade commune (D119) + passif *La Règle* (D99 : ni repoussé ni interrompu ; dégâts croissants avec les PV manquants) +
    - ***Pas un pas en arrière*** : ~5 s, ~40 % de dégâts reçus en moins, mais **pas de parade** possible. Il encaisse sans esquiver et descend vers son seuil sans mourir trop vite.
    - ***Serrer les dents*** : **sacrifie ~10 % de ses PV** pour gagner aussitôt le bonus de dégâts correspondant. L'ascèse de la Règle devient un pari contrôlé par le joueur.
    - ***Frappe de la Règle*** : attaque forte annoncée, dégâts selon les **PV manquants**. Le coup de grâce.
    - Ultime de duel (niveau 10) ***Non nobis*** (devise de l'Ordre) : ~6 s pendant lesquelles **les dégâts reçus sont différés** (appliqués à la fin) et il frappe comme à son seuil maximal. Contre-jeu : parer et survivre ; si le duel n'est pas gagné à la fin, il encaisse tout d'un coup.
  - Arc du duel : descendre vers le seuil (encaisser ou se blesser), puis frapper. L'adversaire doit l'achever avant le seuil ou se défendre une fois qu'il l'a atteint. Face au Juge (D121) : *Coup de bouclier* sans effet, mais la parade et les Fautes le punissent toujours. ⚠️ Vigilance : pic de dégâts de *Serrer les dents* + *Frappe de la Règle* ; valeurs à régler en test.
- **Victoire en duel *(D100)* : *La gloire du Temple*.** Le duel gagné donne **un rang de gloire** à un contingent de la croisade, comme une croisade réussie (D90), **sans l'or** (D91). Effet permanent, mais borné par le plafond de gloire : exception mesurée au § 10.6 (gain temporaire).

**Arbre de talents *(D101)* : quatre familles**, **Croisade** (taille, vitesse et gloire des croisades, *Prendre la croix*), **Chevalerie** (combat, duel *La Règle*, Chevaliers du Temple, *Deus lo vult !*), **Trésor** (or des croisades, *Compagnie franche*, frères convers) et **Les Citadelles** (solidité, *Sortie*, *Camp retranché*). Contenu des talents à concevoir plus tard (D08).

- **Forme *(D103, D104)* :** rangées de **4 talents** (une par famille), contre 3 dans les autres factions ; mêmes seuils (sceau à 3, talent clé à 5). Exception assumée au garde-fou « même structure » (§ 9.6). Vigilance : risque d'une option dominante, à surveiller en test.
- **Sceaux et talents clés :**

  | Famille | Thème | Sceau (3 talents) | Talent clé (5 talents) |
  |---|---|---|---|
  | **Croisade** *(D143)* : « Dieu le veut » | taille, vitesse, gloire, *Prendre la croix* | ***Appel fervent*** : croisades rassemblées plus vite, marche ~+10 %, délai ~+15 % | ***Pèlerinage armé*** (transforme *Prendre la croix*) : les unités qui ont pris la croix **reviennent vétéranes** après une croisade réussie (rang de vétérance permanent, plafonné) |
  | **Chevalerie** *(D144)* : « Frères du Temple » | combat, duel, Chevaliers du Temple, *Deus lo vult !* | ***Vœux de chevalerie*** : Chevaliers du Temple formés ~15 % plus vite, ~+10 % PV | ***Beauséant*** (transforme *La Règle du Temple*) : toutes les unités templières dans l'aura gagnent **« on ne recule pas »** (plus de dégâts en infériorité numérique). Contre-jeu : sortir l'armée de l'aura, combattre à effectifs égaux |
  | **Trésor** *(D145)* : « Les banquiers de la chrétienté » | or des croisades, *Compagnie franche*, frères convers | ***Commanderies rurales*** : frères convers ~+10 % de collecte **en permanence** (en plus du bonus de croisade, D91) | ***Solde du Temple*** (transforme *Compagnie franche*) : la compagnie arrive **vétérane** et **hors population**, pour ~+30 % d'or ; une seule à la fois. Convertit l'or de fin de partie en troupes quand la population est pleine |
  | **Les Citadelles** *(D146)* : « Le réseau du Temple » | solidité, *Sortie*, *Camp retranché* | ***Routes du Temple*** : hors combat, unités templières ~+10 % de vitesse entre deux citadelles alliées | ***Camp retranché*** (transforme *Sortie*) : *Sortie* peut aussi se lancer **n'importe où** près du héros ; elle dresse un camp retranché (~60 s, palissade et pieux) dont la garnison sort combattre. **Le camp ne compte pas comme citadelle pour les croisades** *(nuance de l'utilisateur)*. Actif à partir du niveau 7 (déblocage de *Sortie*) |

- **Rangées 2 à 6 *(décision D147)*** : grille validée d'un bloc (méthode D110 : rangées 2-3 en bonus simples, 4-6 en modifications ; valeur comparable dans une rangée). Valeurs indicatives, à régler en test. Rangées 7 à 9 : plus tard (bloc 4).

  | Rangée | **Croisade** | **Chevalerie** | **Trésor** | **Citadelles** |
  |---|---|---|---|---|
  | **2** | ***Marche forcée*** : croisés +8 % de vitesse de marche | ***Lance couchée*** : Chevaliers du Temple +10 % de dégâts de charge | ***Dîme*** : +5 % d'or collecté | ***Maçons du Temple*** : citadelles construites 15 % plus vite |
  | **3** | ***Foule des pèlerins*** : contingent de base +2 croisés (dans l'enveloppe, D140) | ***Haubert renforcé*** : Chevaliers du Temple et Sergents +1 armure | ***Frères armés*** : frères convers +15 % PV et attaque | ***Garnison renforcée*** : citadelles +2 places de garnison, tirs +10 % |
  | **4** | ***Croix cousue*** (*Prendre la croix*) : rayon +30 %, unités qui la prennent soignées de 20 % | ***Discipline de fer*** (*La Règle du Temple*) : l'aura protège aussi des ralentissements | ***Recrutement rapide*** (*Compagnie franche*) : recharge −25 % | ***Appel aux armes*** : les frères convers peuvent entrer dans une citadelle et renforcer ses tirs |
  | **5** | ***Foi éprouvée*** : après un échec, *Désillusion* −50 % de durée et recharge −20 % | ***Mépris de la mort*** (duel) : bonus de PV manquants ~10 % plus tôt | ***Solde à crédit*** (*Compagnie franche*) : achat possible sans assez d'or ; dette remboursée sur les revenus suivants, collecte réduite jusqu'au remboursement | ***Commanderie fortifiée*** : bâtiments templiers dans le rayon d'une citadelle +5 % d'armure |
  | **6** | ***Martyrs*** : chaque croisé tombé donne à sa croisade un petit bonus de moral cumulable (plafonné) | ***Charge du Temple*** : Chevaliers du Temple, charge récupérée 25 % plus vite | ***Frères de métier*** : frères convers, réparation des bâtiments et engins +30 % | ***Ralliement*** : croisade rassemblée à une citadelle, bouclier de ~5 % de ses PV au départ |

  - Garde-fous : *Foi éprouvée* adoucit l'échec sans accélérer les succès (D138) ; ni vision ni harcèlement des travailleurs ; pas de citadelle-trésor (écartée avec D96).

- **Rangées 7 à 9 *(décision D197)*** (règles D194 ; valeurs indicatives, à régler en test) :

  | Rangée | **Croisade** | **Chevalerie** | **Trésor** | **Citadelles** |
  |---|---|---|---|---|
  | **7** | ***Croix de rappel*** (*Prendre la croix*) : après une croisade **réussie**, les unités qui ont pris la croix reviennent avec tous leurs PV | ***Sortie des chevaliers*** (*Sortie*) : la garnison sort avec 2 Chevaliers du Temple en plus | ***Compagnie aguerrie*** (*Compagnie franche*) : +2 hommes dans la compagnie | ***Sortie prolongée*** (*Sortie*) : durée ~30 s → ~40 s |
  | **8** | ***Haltes de pèlerins*** (croisade) : les croisés régénèrent lentement leurs PV pendant la marche, hors combat | ***Gonfanon haut*** (Porte-gonfanon) : rayon du *Beauséant* +20 % | ***Frères de la commanderie*** (frères convers) : pendant une croisade, bonus de collecte ~+15-20 % → ~+25 % | ***Mâchicoulis*** (citadelles) : +30 % de dégâts aux unités au pied de leurs murs |
  | **9** | ***Grande procession*** (*Reliquaire*) : avec une croisade, délai gagné ~+20 % → ~+30 % | ***Règle de fer*** (duel, *Frappe de la Règle*) : +15 % de dégâts | ***Poudre payée comptant*** (poudre maltaise) : Arquebusiers et Canons maltais ~−15 % d'or | ***Arsenal de la citadelle*** (poudre maltaise) : les citadelles peuvent former les unités de poudre maltaises |

  - **La *Sortie* reste défensive** *(nuance de l'utilisateur)* : aucun talent ne la fait rejoindre la croisade (*Sortie croisée* écartée).
  - ⚠️ Vigilances : *Frères de la commanderie* + or des croisades (boule de neige, D91) ; *Poudre payée comptant* sur une poudre déjà moins chère (D188) ; *Croix de rappel* recoupe en partie *Croix cousue* et *Pèlerinage armé*.

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

**Modes et options *(décision D204)* :** le mode **Standard** reçoit des **options de partie** *(choix de l'utilisateur)* :

- **Nomade *(D207)*** (façon AoE2) : départ **sans centre principal** ; les travailleurs sont **dispersés en petits groupes à des endroits aléatoires de la carte**, et chaque joueur a de quoi construire un centre. **Le héros apparaît avec l'un des groupes.** Avec Régicide, **le Roi apparaît avec un travailleur** *(nuance de l'utilisateur)*, pas forcément avec le héros. ⚠️ Vigilance : distance minimale entre les groupes de joueurs différents (surtout à 8 joueurs).
- **Régicide *(D206)*** (façon AoE2) : au début de la partie, un **Roi** apparaît près du centre principal. C'est une **unité, pas le héros** : elle **ne coûte pas de population** et n'a qu'**une attaque de 1** (elle peut attaquer, mais ne fait presque rien). **Si le Roi meurt, le joueur a perdu.** Le héros, le duel et la résurrection suivent les règles normales.
  - Par défaut *(à confirmer)* : le Roi n'est **jamais converti, retourné ni relevé** (comme les héros, D37, D57, D63). PV et vitesse à régler en test.
- **Catastrophe** : événements mondiaux plus fréquents (dépend du système d'événements, D28).

**Domination, Reliques et Scénario** sont repoussés **après la création du prototype**. **Les trois options sont dans le prototype *(D205)*.**

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

**Structure du HUD *(décision D201)* : base AoE4, avec un emplacement de faction fixe.**

- **En bas :** mini-carte à gauche, sélection au centre, commandes et production à droite (grille de raccourcis de D30).
- **En haut :** ressources, population, palier.
- **Bloc du héros, permanent**, près de la mini-carte : portrait, niveau et XP, barre de vie, capacités, indicateur de récupération ou de duel. Clic sur le portrait : sélection du héros ; double-clic : caméra centrée sur lui (comme `F4`, D30).
- **Emplacement de faction : le même pour les 6 factions**, juste au-dessus du bloc du héros. Il accueille la mécanique de chaque faction : barre d'Honneur et pouvoirs (Aube), panneau de croisade (Templiers : cible, délai, contingents), jauge de l'*Ossuaire* (Légions), élément actif (Dragon), ordres du marché noir (Ombre), Chaleur (Héritiers). Un joueur qui connaît une faction sait où regarder dans les autres (pilier 1).

**Alertes *(décision D202)* : deux niveaux, doublés de signaux dans le monde** *(choix de l'utilisateur : options A et C)*.

- **Majeures** (ce qui vous vise directement : croisade ou mini-croisade contre vous, événement mondial, *Tour livrée* sur vos murs, attaque de votre centre) : **bandeau bref en haut de l'écran**, **voix d'un héraut propre à la faction**, **ping sur la mini-carte** ; clic ou raccourci pour centrer la caméra.
- **Mineures** (unité ou bâtiment attaqué ailleurs, recherche terminée, population pleine…) : **fil discret** sur le côté, ping et **son court** propre à chaque type.
- **Signaux dans le monde et sur la mini-carte**, en plus des deux niveaux : icônes animées sur la mini-carte (cible de croisade, zone d'événement), et signaux visibles sur le terrain (colonne et bannières de croisade, fumée ou grondement d'un événement, signal de la *Tour livrée* sur la porte visée). Le joueur qui regarde la bataille perçoit l'annonce sans quitter le terrain des yeux.
- Le défi de duel garde sa fenêtre propre (D117).
- But : les mécaniques « annoncées, donc contrables » n'ont de sens que si l'annonce est perçue.

**Lecture de l'adversaire *(décision D203)* : l'inspection seulement.** Un clic sur un héros, une unité ou un bâtiment adverse **visible** affiche sa fiche (héros : niveau, sceaux et talents clés, D103, état). La spécialisation adverse se reconnaît à son bâtiment ou à son unité propre. Sans vision, aucune information : pas de carnet de renseignements, pas d'information publique sur le palier ou les spécialisations. L'éclairage garde toute sa valeur (D07).

**Duel :** fenêtre de défi reçu (Accepter / Refuser, compte à rebours, sans pause, D117), indicateur de défi envoyé, recharge du défi, posture active et kit de duel pendant l'affrontement.

Le joueur doit comprendre immédiatement :

1. ce qu'il peut construire ;
2. ce qu'il peut produire ;
3. ce qu'il peut rechercher ;
4. ce que fait son héros ;
5. ce que fait l'ennemi.

---

## 16. Orientation technique

**Version du moteur *(décision D214)* : Unreal Engine 5.8** (dernière version d'Unreal 5). Le projet, créé en 5.4, est migré avant le premier chantier de code.

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
  - simulées sur le serveur, avec une **architecture hybride *(D215)*** : **Mass** pour le cœur (stockage des entités, traitements multicœurs, suivi de chemin sur le navmesh, évitement, représentation en instances avec niveaux de détail, StateTree pour les comportements) et une **couche RTS maison** (ordres de groupe et formations, combat, réplication compacte via Iris, brouillard de guerre). Garde-fou : un essai d'environ deux jours ouvre le chantier (2 000 entités en mouvement sur le navmesh, affichées en instances) ; si Mass résiste trop, repli sur un gestionnaire maison orienté données ;
  - répliquées sous forme compacte (positions et états compressés), interpolées côté client ;
  - affichées en **instances** avec des animations optimisées (animation par textures de sommets, ou équivalent).
- **Héros, bâtiments et engins de siège = acteurs classiques avec GAS.** Ils sont peu nombreux : GAS reste utilisé là où il compte (capacités, duels, auras).
- **Le brouillard de guerre est appliqué par le serveur**, qui n'envoie à chaque client que ce qu'il voit. Cela protège contre la triche « maphack » et réduit la bande passante.
- **Cible de performance *(D190)* : au minimum 2 000 unités simultanées**, comme marge de sécurité *(exigence de l'utilisateur)*. Le pire cas de population est de ~1 400 (8 × 175, D03), auquel s'ajoutent les unités hors population (croisades jusqu'à ~48, contingent maltais compris, D189 ; serviteurs, Squelettes temporaires, invocations, troupes de *Sortie*). Cette valeur ne sera jamais atteinte en partie : c'est une sécurité.
- **Stress tests *(D190)* :** une **batterie de tests de charge** accompagnera tout le développement pour garantir ce plancher (à concevoir le moment venu).

**Conséquence sur le code :** l'ancien code hérité d'un autre projet (`AUnitBase : ACharacter` avec un `AAIController` par unité, et ses Blueprints) a été **supprimé le 2026-10-07**. Le système d'unités légères part de zéro ; les ordres de déplacement, la sélection et le combat de base seront construits dessus.

L'IA joue avec les mêmes règles et les mêmes informations qu'un joueur (brouillard de guerre compris), sauf dans les niveaux de difficulté explicitement « tricheurs ».

### 16.5 Navigation sur plusieurs niveaux

Les remparts praticables (D24) imposent une navigation à deux niveaux : le sol et le chemin de ronde des murs de pierre.

- Les segments de mur portent une **surface de navigation** sur leur sommet, reliée au sol par des liens de navigation (escaliers dans les tours et les portes).
- Cette navigation est **mise à jour dynamiquement** à la construction et à la destruction de chaque segment.
- Elle doit fonctionner avec les **unités légères** (D29), pas seulement avec les `ACharacter` d'Unreal.
- C'est un chantier technique prioritaire du prototype, à valider tôt : performance avec ~1 400 unités et des murs étendus.

### 16.6 Premier chantier : unités légères *(D214 à D218, plan validé)*

Principe : la **simulation** fait autorité (serveur) et reste séparée de la **présentation** (ce que voit le client, interpolé). Les ordres passent toujours par une file de commandes, même en solo.

**Fréquence *(D217)* :** la simulation (Mass et couche RTS) tourne à **pas fixe de 20 Hz**, indépendamment des images par seconde de l'hôte ; le client interpole à la fréquence de l'écran. Les états sont envoyés à 10 ou 20 Hz selon le budget mesuré à M2.5. La fréquence est un réglage de configuration.

| Étape | Contenu | Stress test |
|---|---|---|
| **M0** | Migration en 5.8 (D214), modules et plugins, carte de test, nettoyage de `BattleMap` | |
| **M1** | Essai Mass (~2 jours, D215), puis cœur de la simulation : entités, identifiants stables, apparition et disparition, `UnitData` | S1 : 2 000 unités immobiles |
| **M2** | Rendu : instances par type et par joueur, animations cuites dans des textures, interpolation | S2 : 2 000 unités en marche |
| **M2.5** | Tranche réseau (D216) : réplication des positions et états via Iris, serveur hébergé + 2 clients | S5 : bande passante à 2 000 unités en mouvement |
| **M3** | Déplacement : chemins sur le navmesh, ordres de groupe et formations, évitement | S2 |
| **M4** | Sélection, ordres de déplacement et d'attaque, groupes de contrôle | |
| **M5** | Combat de base : cible, mêlée, tir, dégâts et armure, mort, cadavres (D86) | S3 : 1 000 contre 1 000 en mêlée ; S4 : armées mixtes |
| **M6** | Réseau complet : ordres, interpolation, 8 clients | S5 à 8 clients |
| **M7** | Brouillard de guerre appliqué par le serveur | |
| **M8** | Remparts : navigation sur deux niveaux (§ 16.5) | S6 : siège avec murs étendus |

**Seuils des stress tests *(D218, niveau exigeant)*** — machine de référence : le poste de développement (Ryzen 7 5700X, RTX 5060 Ti, 64 Go) ; scénarios S1 à S6 à 2 000 unités :

| Mesure | Seuil |
|---|---|
| Simulation serveur (tick de 50 ms) | ≤ 3 ms en moyenne |
| Rendu client (1080p, qualité haute, 2 000 unités à l'écran) | ≥ 144 i/s |
| Bande passante par client | ≤ 32 Ko/s |
| Envoi de l'hôte (7 clients) | ≤ ~225 Ko/s |
| Mémoire | mesurée et suivie, sans seuil |

Un seuil dépassé bloque l'étape. Un seuil ne se révise que par une décision consignée au journal, jamais en silence. Valeurs indicatives, à régler en test.

À partir de M2.5, une vérification à 2 clients doit passer à la fin de chaque étape. Les stress tests se lancent par une commande console sur une carte dédiée ; ils mesurent le temps de simulation par tick, les images par seconde, la bande passante par client et la mémoire. Héros, bâtiments et passerelle avec GAS viennent après M8.

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
| Bâtiment militaire | 1 au départ, puis le découpage par catégorie (D128) une fois les combats validés |
| Bâtiment technologique | 1 |
| Fortifications | murs de pierre praticables, porte, tour (D24) |
| Niveaux de héros | 1 à 6 (paliers 0, 1 et 2) |
| Duel | 1 |
| Système de mort / résurrection | 1 |
| Événement volcanique | **1** *(D205, revient au prototype pour l'option Catastrophe ; révise D28)* |
| Mode et options *(D205)* | mode **Standard** (destruction du centre principal) + options **Nomade**, **Régicide** et **Catastrophe** (D204) |

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
8. **Volcan** *(au prototype, D205)* **:** l'événement crée-t-il une opportunité plutôt qu'une frustration ?
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
| D103 | 2026-10-07 | Arbres de talents — Forme | **Rangées avec seuils de famille** : à chaque niveau (2 à 9), 1 talent parmi 3, chacun rattaché à une famille ; 3 talents d'une famille donnent son **sceau** (bonus passif), 5 son **talent clé** (transforme une capacité ou l'aura). Maximum : un talent clé et un sceau. Sceaux et talents clés visibles par l'adversaire. Précise D08. | § 9.6 |
| D104 | 2026-10-07 | Arbres de talents — Nombre de familles | **3 familles, exception templière** : les Templiers gardent leurs 4 familles (D101), avec des rangées de 4 talents et les mêmes seuils. Le garde-fou « même présentation » de D08 devient « même structure ». Clôt la vigilance de D101. | § 9.6, § 13.7 |
| D105 | 2026-10-07 | Arbres de talents — Moment du choix | **Réserve libre** : point gardé sans limite de temps, dépensé à tout moment (même héros mort), rangées remplies dans l'ordre, effet immédiat. Rappel affiché et bouton « choix conseillé » (famille la plus remplie). | § 9.6 |
| D106 | 2026-10-07 | Arbre de l'Aube — Familles | **Les trois Serments**, chacun lié à une source d'Honneur (D76) : **Gardien** (stratégique : défendre, tenir, *Bannière*, murs, *Sanctuaire*), **Capitaine** (commandement : protéger, aura, *Serrez les rangs*, Chevaliers, Moines), **Champion** (duel : riposte, survie, Honneur en duel). Un Serment ajoute un bonus à une source sans retirer les autres. Talents clés proposés comme pistes. | § 9.6, § 13.2 |
| D107 | 2026-10-07 | Arbre de l'Aube — Serment du Gardien | **« On ne passe pas »** : sceau *Pierres de l'Aube* (murs et tours plus solides, réparation plus rapide, plus d'Honneur en tenant un point) ; talent clé *Bannière inébranlable* (Bannière maintenue tant que des alliés tiennent sa zone, cumuls d'armure plafonnés, Honneur en continu). Vigilance : Aube « tortue ». | § 13.2 |
| D108 | 2026-10-07 | Arbre de l'Aube — Serment du Capitaine | **« Un pour tous »** : sceau *Discipline de l'Aube* (résistance aux effets de moral négatifs, plus d'Honneur par les dégâts absorbés) ; talent clé *Mur de boucliers* (pendant *Serrez les rangs*, dégâts répartis entre toutes les unités de la formation ; contré par les dégâts de zone). | § 13.2 |
| D109 | 2026-10-08 | Arbre de l'Aube — Serment du Champion | **« Jugement des armes »** (option A, sans le sceau hybride recommandé) : sceau *Vœu du Champion* (défi rechargé ~1/3 plus vite, plus d'Honneur en acceptant et en gagnant) ; talent clé *Ordalie* (chaque riposte réussie en duel donne un cumul de *Ferveur* aux alliés autour du cercle, *Triomphe* prolongé et élargi en cas de victoire). Vigilances : anti-harcèlement (D16) ; build faible entre deux duels, à compenser par les talents des rangées. | § 13.2 |
| D110 | 2026-10-08 | Arbre de l'Aube — Méthode des rangées | **Grille complète des 15 talents (rangées 2 à 6) proposée d'un bloc puis amendée**. Règles : rangées 2-3 en bonus simples, rangées 4-6 en modifications de capacités, valeur comparable au sein d'une rangée. | § 13.2 |
| D111 | 2026-10-08 | Arbre de l'Aube — Rangées 2 à 6 | **Grille validée** (15 talents, § 13.2). Gardien : *Moisson bénie*, *Vigie de l'Aube*, *Étendard du rempart*, *Remparts de lumière*, *Double étendard*. Capitaine : *Boucliers levés*, *Mains du guérisseur*, *Pas de l'Aube*, *Grâce abondante*, *Frères d'armes*. Champion : *Lame bénie*, *Endurance du croisé*, *Cri du défi* (remplace *Marque du parjure*, jugée trop forte), *Contre parfait*, *Escorte du champion*. Vigilances : Aube « tortue », *Escorte* faible avant le palier 2. | § 13.2 |
| D112 | 2026-10-08 | Arbre des Légions — Familles | **Les trois Rites**, chacun décidant à quoi servent les morts (D86) : **Moisson** (duel / combat : la mort nourrit le Seigneur), **Charnier** (commandement : la mort grossit l'armée), **Effroi** (stratégique : la mort répand la peur ; Spectres, *Aura de terreur*). Remplace l'exemple indicatif Nécromancie / Terreur / Sacrifice. Vigilance : contenu de l'Effroi au prototype. | § 9.6, § 13.3 |
| D113 | 2026-10-08 | Arbre des Légions — Rite de la Moisson | **« Le Festin du duel »** : sceau *Faim insatiable* (cumuls de *Moisson* plus durables, plafond ~+2) ; talent clé *Festin* (cumuls conservés en entrant en duel et convertis en dégâts et drain, chaque coup de duel en ajoute un). Miroir d'*Ordalie*. Vigilance : le refus devient évident face à un Seigneur chargé. | § 13.3 |
| D114 | 2026-10-08 | Arbre des Légions — Rite du Charnier | **« La Légion d'os »** : sceau *Os durcis* (Squelettes ~+20 % PV, levée plus rapide) ; talent clé *Légion d'os* (dans l'aura, chaque Squelette gagne attaque et armure selon le nombre de Squelettes proches, plafonné). Contre-jeu : dégâts de zone, dispersion. | § 13.3 |
| D115 | 2026-10-08 | Arbre des Légions — Rite de l'Effroi | **« Terre maudite »** (2e série d'options ; la 1re, vision forte, panique des travailleurs, aura portée par un Spectre, saut vers un cadavre, a été rejetée) : sceau *Sol profané* (là où beaucoup sont morts, sol maudit ~90 s, malus de moral léger aux unités militaires ennemies, jamais aux travailleurs) ; talent clé *Champ des lamentations* (sur terre maudite, *Aura de terreur* doublée et régénération des morts-vivants). Contre-jeu : purification (Moine, *Lumière sacrée*). | § 13.3 |
| D116 | 2026-10-08 | Arbre des Légions — Rangées 2 à 6 | **Grille validée** (15 talents, § 13.3). Moisson : *Lame avide*, *Carapace de chair*, *Sacrifice vorace*, *Exécuteur*, *Curée*. Charnier : *Charnier fertile*, *Maîtres des tombes*, *Éclats d'os*, *Levée prompte*, *Commandant des morts*. Effroi : *Voix sépulcrale*, *Lames spectrales*, *Sacrifice maudit*, *Effroi tenace*, *Hurlement funèbre*. Rangée 4 entièrement consacrée à *Sacrifice*. Vigilances : plafond commun des serviteurs temporaires, synergies *Sacrifice maudit* + *Champ des lamentations* et *Commandant des morts*. | § 13.3 |
| D117 | 2026-10-08 | Duel — Fenêtre de défi | **Popup dans l'interface** à la réception d'un défi (demande de l'utilisateur) : portrait, niveau, sceaux et talents clés du héros qui défie, compte à rebours, boutons Accepter / Refuser avec raccourcis ; sans pause ni blocage de l'écran ; centrage caméra et signal minimap. Indicateur « défi envoyé » chez celui qui défie. | § 10.1, § 15 |
| D118 | 2026-10-08 | Duel — Postures | Description du duel validée (« c'est parfait »). **Offensive** : plus de dégâts, fenêtres de défense plus courtes ; **équilibrée** : référence ; **défensive** : moins de dégâts, fenêtres de défense plus larges. Changement en un clic, à tout moment. | § 10.3 |
| D119 | 2026-10-08 | Duel — Gabarit des kits | **Parade commune + 3 capacités propres + ultime (niveau 10)**. La parade, identique pour les six héros, réduit fortement une attaque forte annoncée sans l'annuler (courte recharge, fenêtre selon la posture). Les capacités propres portent le style (la riposte améliore la parade, la feinte la trompe). Remplace « 4 à 6 capacités » (D14). | § 10.3 |
| D120 | 2026-10-08 | Duel — Déplacement | **Contact automatique** : pas de déplacement libre dans le cercle ; la mobilité passe uniquement par des capacités propres (bond, recul, charge), signature de certains héros. | § 10.3 |
| D121 | 2026-10-08 | Kit de duel du Paladin | **« Le Juge »** : *Riposte* (parade améliorée, inflige une Faute ; ratée, expose le Paladin), *Coup de bouclier* (interrompt une attaque forte en préparation ; sans effet sur le Grand Maître), *Sentence* (consomme les Fautes en dégâts) ; ultime de duel *Verdict* (attaque forte annoncée imparable, dégâts selon les Fautes). | § 13.2 |
| D122 | 2026-10-08 | Kit de duel du Seigneur Damné | **« Le Boucher »** : passif de drain ; *Enchaînement* (2 coups rapides + coup final annoncé, sature la parade), *Frappe dévorante* (attaque forte annoncée, draine ~50 %), *Rage nécrotique* (cadence et drain accrus, dégâts subis accrus) ; ultime de duel *Faim des damnés* (drain fort, attaques fortes ininterruptibles mais annoncées). | § 13.3 |
| D123 | 2026-10-08 | Aube — Spécialisation palier 1 | **Repli ou projection** : *Bastion de l'Aube* (murs et tours plus solides, tours bénies qui soignent) ou *Chapelle de campagne* (bâtiment lointain qui étend l'effet du *Sanctuaire*, ralliement, forme des Moines). | § 13.2 |
| D124 | 2026-10-08 | Aube — Spécialisation palier 2 | **L'Acier ou la Foi** : *Lices de l'Aube* (Chevaliers plus rapides à former, *Défi* renforcé) ou *Monastère* (Moines : meilleurs soins, purification plus large, plus d'Honneur par les soins). | § 13.2 |
| D125 | 2026-10-08 | Moral — Purification supprimée | **Le Moine atténue, il ne retire pas** : les effets de moral négatifs et les malédictions sont atténués (intensité ou durée), jamais retirés. *Lumière sacrée* pourrait les retirer : à trancher en test. Le sol maudit ne se purifie pas. Révise D74 (purification), D85 (*Lumière sacrée*) et le contre-jeu de D115. | § 7.2, § 13.2, § 13.3 |
| D126 | 2026-10-08 | Légions — Spécialisation palier 1 | **Le Labeur ou l'Assaut** : *Fosses de labeur* (Zombies plus rapides et moins chers) ou *Autel de sang* (bâtiment avancé qui forme Guerriers Damnés et infanterie près du front, cadavres plus durables dans son rayon). | § 13.3 |
| D127 | 2026-10-08 | Légions — Spécialisation palier 2 | **La chair ou la nécromancie** : *Fosse des damnés* (Guerriers moins chers, rage plus longue) ou *Tour des nécromanciens* (levée plus rapide, malédictions plus fortes). | § 13.3 |
| D128 | 2026-10-08 | Bâtiments — Production militaire | **Un bâtiment par catégorie** (façon AoE4) : Caserne, Champ de tir, Écurie, Atelier de siège ; emblématiques depuis leur catégorie ou un bâtiment de faction. Prototype : bâtiment unique possible au départ, données conçues pour le découpage. | § 6, § 17 |
| D129 | 2026-10-08 | Tronc commun des paliers 0 à 2 | **Grille validée** (§ 11.1) : bâtiments et unités par palier (Caserne et Champ de tir au palier 0 ; Écurie, Forge, Marché, Atelier, Centre secondaire, murs de pierre au palier 1 ; Académie au palier 2), technologies communes (Forge I/II, collecte, Centre, Académie, Atelier) et technologies propres de l'Aube et des Légions. Vigilances : *Bénédiction des armes* (bonus contre une catégorie de factions), recoupements technologies / talents. | § 11.1 |
| D130 | 2026-10-09 | Templiers — Point fort (timing) | **Solides en continu, sommet pendant une croisade** (nuance de l'utilisateur sur l'option « par vagues ») : compétitifs à tout moment, sans creux marqué pendant la recharge ; pic de puissance pendant la croisade, croissant au fil des paliers. Contre-jeu centré sur la croisade elle-même. Vigilance : garder la force hors croisade dans la moyenne. | § 13.1, § 13.7 |
| D131 | 2026-10-09 | Templiers — Commanderies par palier | **Trois commanderies fixes par palier** (9 au total, 27 croisades possibles) : chaque commanderie n'est équilibrée que face aux deux autres de son palier ; cohérent avec les spécialisations des autres factions (D123 à D127). Écartés : tirage aléatoire, choix libre dans toute la réserve. | § 13.7 |
| D132 | 2026-10-09 | Templiers — Structure des commanderies | **Trois voies** : Fer (mêlée, choc), Foi (soutien, moral), Pierre (tir, siège), une commanderie de chaque voie par palier ; voies cumulables. Nuance de l'utilisateur : **chaque commanderie débloque une unité propre aux Templiers**, jamais une unité du socle commun (Sergent, Arbalétrier, Bélier, Trébuchet tels quels exclus). | § 13.7 |
| D133 | 2026-10-09 | Templiers — Grille des unités de commanderie | **7 unités validées** : Fer = *Sergent du Temple* (*Ouvrir les rangs*), *Sergent à cheval* (*Seconde vague*), *Maréchal du Temple* (unique, *Escadron*, la croisade le suit) ; Foi = *Frère infirmier* (soin hors combat, rapide) ; Pierre = *Arbalétrier à pavois*, *Frère maçon* (*Sape*), *Beffroi*. **Écartés** : *Frère chapelain*, *Frère pénitent* (« pas convaincu ») ; Foi paliers 2 et 3 à remplacer. Remarque de l'utilisateur : un moine soigneur en combat existe dans le socle commun, remplaçable par une unité différente (façon éléphants guérisseurs des Tughlaq, AoE4) : à préciser, le § 7.1 n'en contient pas. | § 13.7 |
| D134 | 2026-10-09 | Socle commun — Moine | **Moine ajouté au socle commun** (soigneur en combat, façon AoE4, palier 1, sans conversion). Exception à la règle des variantes : plusieurs factions peuvent le remplacer par une unité différente (façon éléphants guérisseurs des Tughlaq). Le Moine Lumineux devient la variante de l'Aube ; le *Frère infirmier* complète le Moine chez les Templiers. À préciser : Légions et autres factions, bâtiment de production. | § 7.1, § 11.1 |
| D135 | 2026-10-09 | Templiers — Voie de la Foi, paliers 2 et 3 | **Prêcheur de croisade** (palier 2) : contingent = toutes les croisades grossissent en marchant ; recruté = *Sermon*. **Sénéchal** (palier 3), nuance de l'utilisateur : **officier unique, hors croisade** (exception à la règle du contingent) ; pouvoir ***Mini-croisade*** : il emmène une partie de l'armée et lance sa propre croisade, troupes autonomes, **de nouveau contrôlables en cas de victoire**. Écartés : *Vraie Croix*, *Frère drapier*. Rappel de l'utilisateur : la croisade se compose automatiquement à partir des commanderies (chaque unité conçue sous deux formes). | § 13.7 |
| D136 | 2026-10-09 | Templiers — Mini-croisade du Sénéchal | **Croisade en miniature** : au plus **~30 % du maximum d'une croisade** (précision de l'utilisateur, indicatif, à régler en test) ; cible annoncée ; succès = or réduit, sans gloire, troupes de nouveau contrôlables ; échec (délai ou Sénéchal tué) = troupes rendues avec *Désillusion*, longue recharge ; recharge indépendante de l'appel principal. | § 13.7 |
| D137 | 2026-10-09 | Templiers — Maréchal et contingent | **Les deux formes** : contingent = escadron de Chevaliers du Temple vétérans chargeant ensemble (*Escadron*) dans chaque croisade ; recruté = officier unique, *Escadron* sur les Chevaliers contrôlés, la croisade qu'il accompagne le suit (marche automatique s'il meurt). | § 13.7 |
| D138 | 2026-10-09 | Templiers — Récompenses de croisade | **Succès** : gloire + or + **XP modérée au héros** (fait d'armes, D05, même s'il n'a pas suivi), **recharge normale**. **Échec** : pas d'XP, *Désillusion*, recharge longue. Un succès ne raccourcit pas la recharge. | § 13.7 |
| D139 | 2026-10-09 | Templiers — Plafond de la croisade | **Population de croisade propre**, indépendante de celle du joueur, fondée sur le **nombre de commanderies** : ~10 croisés par commanderie (précision de l'utilisateur, indicatif). Les troupes de la **mini-croisade restent dans la population du joueur**. Hypothèse : le socle de Pèlerins compte comme une commanderie (~40 au maximum). Reste à trancher : place des bonus (citadelles, Prêcheur, gloire). | § 13.7 |
| D140 | 2026-10-09 | Templiers — Bonus et plafond de croisade | **Enveloppe** : les ~10 par commanderie sont un maximum ; les contingents de base arrivent en dessous, citadelles, Prêcheur et gloire les complètent ; **le surplus devient de la vétérance**. | § 13.7 |
| D141 | 2026-10-09 | Templiers — Croisade face aux murs | **Chemin normal, sinon attaque du mur** : brèche ou porte ouverte par le plus court chemin ; à défaut, porte ou pan de mur le plus proche de la cible, attaqué en priorité par les engins et la voie Pierre (*Beffroi*, *Frère maçon*). Les murs coûtent du temps sur le délai. | § 13.7 |
| D142 | 2026-10-09 | Kit de duel du Grand Maître | **« Le Martyr »** : *Pas un pas en arrière* (~5 s, −40 % de dégâts reçus, pas de parade), *Serrer les dents* (sacrifie ~10 % de PV pour le bonus de dégâts), *Frappe de la Règle* (attaque forte annoncée selon les PV manquants) ; ultime de duel *Non nobis* (~6 s de dégâts reçus différés, frappe au seuil maximal). | § 13.7 |
| D143 | 2026-10-09 | Arbre des Templiers — Famille Croisade | **« Dieu le veut »** : sceau *Appel fervent* (rassemblement plus rapide, marche ~+10 %, délai ~+15 %) ; talent clé *Pèlerinage armé* (*Prendre la croix* : les unités reviennent vétéranes après une croisade réussie, rang permanent plafonné). Écarté : *Croisade sans fin* (enchaînement de croisades, boule de neige). | § 13.7 |
| D144 | 2026-10-09 | Arbre des Templiers — Famille Chevalerie | **« Frères du Temple »** : sceau *Vœux de chevalerie* (Chevaliers du Temple ~15 % plus vite formés, ~+10 % PV) ; talent clé *Beauséant* (l'aura *La Règle du Temple* donne « on ne recule pas » à toutes les unités templières). Écarté : build duelliste *Martyre* (trop proche d'*Ordalie*, faible entre deux duels). | § 13.7 |
| D145 | 2026-10-09 | Arbre des Templiers — Famille Trésor | **« Les banquiers de la chrétienté »** : sceau *Commanderies rurales* (frères convers ~+10 % de collecte en permanence) ; talent clé *Solde du Temple* (*Compagnie franche* vétérane et hors population, ~+30 % d'or, une à la fois). Écarté : *Dîme de croisade* / *Mercenaires de la croisade* (boule de neige, doublon de *Prendre la croix*). | § 13.7 |
| D146 | 2026-10-09 | Arbre des Templiers — Famille Les Citadelles | **« Le réseau du Temple »** : sceau *Routes du Temple* (~+10 % de vitesse hors combat entre citadelles alliées) ; talent clé *Camp retranché* (*Sortie* lançable n'importe où, camp ~60 s). Nuance de l'utilisateur : **le camp ne compte pas comme citadelle pour les croisades**. Écarté : « La forteresse » (trop défensif). | § 13.7 |
| D147 | 2026-10-09 | Arbre des Templiers — Rangées 2 à 6 | **Grille validée** (20 talents, § 13.7). Croisade : *Marche forcée*, *Foule des pèlerins*, *Croix cousue*, *Foi éprouvée*, *Martyrs*. Chevalerie : *Lance couchée*, *Haubert renforcé*, *Discipline de fer*, *Mépris de la mort*, *Charge du Temple*. Trésor : *Dîme*, *Frères armés*, *Recrutement rapide*, *Solde à crédit*, *Frères de métier*. Citadelles : *Maçons du Temple*, *Garnison renforcée*, *Appel aux armes*, *Commanderie fortifiée*, *Ralliement*. | § 13.7 |
| D148 | 2026-10-09 | Kit de duel du Seigneur-Dragon | **« Le wyrm veille »** : *Mue élémentaire* (changement instantané + éclat de l'élément), *Bond draconique* (mobilité, atterrissage élémentaire), *Ombre du wyrm* (passage annoncé du dragon, souffle parable) ; ultime de duel *Furie du ciel* (trois passages annoncés, élément changeable entre chacun). Écarté : « Maître des trois éléments ». | § 13.4 |
| D149 | 2026-10-09 | Enfants du Dragon — Pas de bêtes | **Aucune bête dans la faction** (« pas de bête dans cette faction ») : Dompteur de Bêtes et compagnons retirés, « créatures » retiré du thème ; seuls dragons : celui du héros et l'*Appel de la Couvée*. Troisième emblématique à concevoir. À revoir : interaction avec les créatures neutres (§ 8.4), cible « créatures » de *Bénédiction des armes* (§ 11.1). | § 8.4, § 13.1, § 13.4 |
| D150 | 2026-10-09 | Enfants du Dragon — Troisième emblématique | **Garde d'écailles** : tank d'infanterie lourde à ***Écailles adaptatives*** (armure croissante contre le type de dégâts le plus reçu, mêlée ou distance, bascule avec délai), sans micro. Trio : Champion Draconique (mêlée), Mage Élémentaire (distance), Garde d'écailles (tank). Écartés : *Gardien des Nids*, *Drakéide* (harcèlement). | § 13.4 |
| D151 | 2026-10-09 | Dragon — Spécialisation palier 1 | **Les arcanes ou les écailles** : *Foyer élémentaire* (délais de changement d'élément ~−40 %, Mages ~+10 % de portée) ou *Forge d'écailles* (Champions et Gardes plus vite formés, +1 armure, *Écailles adaptatives* plus rapides). Écarté au palier 1 : *Perchoir du dragon* (harcèlement aérien précoce, D51). | § 13.4 |
| D152 | 2026-10-09 | Dragon — Spécialisation palier 2 | **Le ciel ou le sol** : *Perchoir du dragon* (dragon à +2 niveaux de statistiques, bascule plus rapide, *Souffle* ~−20 % de recharge) ou *Pierre de résonance* (*Aura draconique* ~+30 % de rayon et d'effet). Écarté : *Nids ancestraux* (cumul des Nids sans plafond, D72). | § 13.4 |
| D153 | 2026-10-09 | Arbre du Dragon — Familles | **Les trois voies du héros** : *Le Ciel* (forme montée), *La Lignée* (commandement à pied, armée, Champions, Gardes), *La Mue* (adaptation, éléments, Nids, Mages, duel). Chaque famille répond à une spécialisation ; aucune n'enferme dans un élément. Écarté : familles Feu / Glace / Foudre (contraire à l'adaptation). | § 13.4 |
| D154 | 2026-10-09 | Arbre du Dragon — Le Ciel | **« La Bête ailée »** : sceau *Vents porteurs* (vol ~+15 %, bascule ~−25 %) ; talent clé *Piqué dévastateur* (renversement, saut à terre au point d'impact sans bascule). Écarté : *Écailles d'acier* / *Souffle persistant* (affaiblit le contre des tireurs). | § 13.4 |
| D155 | 2026-10-09 | Arbre du Dragon — La Lignée | **« Le sang du wyrm »** : sceau *Sang draconique* (Champions et Gardes ~+10 % PV) ; talent clé *Écho élémentaire* (après un changement d'élément, l'ancien effet reste ~8 s dans l'aura en plus du nouveau). Écarté : *Lame héritée* / *Garde rapprochée* (contourne la règle de délai). | § 13.4 |
| D156 | 2026-10-09 | Arbre du Dragon — La Mue | **« L'instinct du wyrm »** : sceau *Esprit vif* (délais de changement des emblématiques et des Nids ~−25 %, Mages ~+5 % de portée) ; talent clé *Mue instinctive* (changement d'élément du héros instantané en RTS, avec éclat de zone, sous recharge). Écarté : build duelliste *Ombre double*. | § 13.4 |
| D157 | 2026-10-09 | Arbre du Dragon — Rangées 2 à 6 | **Grille validée** (15 talents, § 13.4). Le Ciel : *Écailles du dragonnet*, *Ailes larges*, *Souffle ardent*, *Ombre immense*, *Atterrissage brutal*. La Lignée : *Lignée ancienne*, *Écailles trempées*, *Lame tranchante*, *Cri de la lignée*, *Garde du sang*. La Mue : *Nids de pierre*, *Apprentis des arcanes*, *Transition fluide*, *Mue de combat*, *Éclat amplifié*. | § 13.4 |
| D158 | 2026-10-09 | Kit de duel de la Voix | **« Le Menteur »** : *Feinte* (fausse attaque annoncée ; parée, elle expose l'adversaire ~1 s), *Vraie menace* (vraie attaque portant l'indice d'une feinte), *Pas de côté* (esquive qui évite une attaque forte en préparation) ; ultime de duel *Mot de silence* (~4 s, parade seule pour l'adversaire). Écarté : « L'Ombre » (invisibilité en duel, profil d'assassin). | § 13.5 |
| D159 | 2026-10-09 | Ombre — Le Piégeur | **« L'embuscade annoncée »** (l'utilisateur trouvait l'unité frustrante : du contrôle, mais qui ne fait pas râler) : pièges cachés (repérés par les éclaireurs proches) ; déclenchement annoncé (~1 s, signal visible et sonore), esquivable en sortant de la zone ; effet : fort ralentissement, révélation, poison, peu de dégâts. Garde-fous : jamais d'immobilisation totale, travailleurs non concernés, pièges actifs plafonnés, immunité après effet (D37). Écartés : pièges visibles (« Le terrain »), filet ciblé (« La proie »). | § 13.5 |
| D160 | 2026-10-09 | Ombre — Spécialisation palier 1 | **La bourse ou le terrain** : *Comptoir du marché noir* (effet à préciser, sans cumul excessif avec D69) ou *Atelier du piégeur* (Piégeurs formés plus vite, +1 piège actif, pose plus rapide, poison plus long). Palier 2 fixé dans son principe : *Repaire des assassins* (la lame) ou *Salle des miroirs* (l'esprit), contenu à préciser. Même logique que l'Aube (forme de la partie, puis composition). Écartés : « Les pièges ou les illusions », « Le poignard ou la bourse » (assassin renforcé tôt, harcèlement précoce). | § 13.5 |
| D161 | 2026-10-09 | Ombre — Effet du *Comptoir du marché noir* | **« Les ordres »** : ordres à seuil (achat sous un cours, vente au-dessus), exécutés automatiquement ; le Cercle profite des mouvements du cours commun (D84) causés par les autres. Aucun cumul de taux avec D69, aucun effet sur l'adversaire ; ordres actifs plafonnés. Écartés : « Le taux » (taux encore meilleur), « La manipulation » (cours deux fois plus sensible). | § 13.5 |
| D162 | 2026-10-09 | Ombre — Spécialisation palier 2 | **« Finir le travail »** : *Repaire des assassins* (Maîtres des Ombres formés plus vite ; *Coup de grâce*, gros dégâts contre les unités sous contrôle ; camouflage non allongé) ou *Salle des miroirs* (Illusionnistes formés plus vite ; *Reflets multipliés*, +1 copie, copies plus durables). L'Ombre ne frappe fort qu'après un contrôle annoncé et esquivable (extension de D159). Vigilance : cumul avec *Murmures*. Écartés : bonus purs ; « L'ombre et le reflet » (recamouflage, éclat de miroir). | § 13.5 |
| D163 | 2026-10-09 | Arbre de l'Ombre — Familles | **La Lame, le Miroir, la Toile** : *La Lame* (duel / combat : kit du Menteur, Maîtres des Ombres, *Coup de grâce* ; *Repaire*), *Le Miroir* (commandement par la manipulation : *Murmures*, *Mot d'arrêt*, *Mensonge*, illusions, conversion ; *Salle des miroirs*), *La Toile* (stratégique : pièges, *Comptoir*, ordres ; *Atelier*, *Comptoir*). Vigilance : Miroir riche, Toile sans vision. Écarté : « Les trois Paroles » (Menace / Mensonge / Serment : aucune voie stratégique). | § 9.6, § 13.5 |
| D164 | 2026-10-09 | Arbre de l'Ombre — La Lame | **« Le signal »** : sceau *Confrérie* (Maîtres des Ombres ~15 % plus vite formés, ~+10 % de dégâts contre les unités sous contrôle ; *Feinte* ~−20 % de recharge en duel) ; talent clé *Signal de la Voix* (le *Mot d'arrêt* fait bondir les Maîtres des Ombres proches sur les ennemis effrayés). **Révise D163 :** *Mot d'arrêt* passe du Miroir à la Lame (allège le Miroir). Écarté : « Le duelliste » (*Langue d'argent*, *Feinte parfaite* : creux en RTS). | § 13.5 |
| D165 | 2026-10-09 | Arbre de l'Ombre — Le Miroir | **« La rumeur »** : sceau *Voix porteuse* (*Murmures* ~+15 % de rayon, Illusionnistes ~15 % plus vite formés) ; talent clé *Murmures contagieux* (un contrôle subi dans l'aura propage le malus de moral de *Murmures* aux voisins immédiats, sans contrôle supplémentaire). Rien sur la conversion ni sur la durée des contrôles (D37). Vigilance : cumul des bonus contre les unités sous contrôle (*Murmures*, *Coup de grâce*, *Confrérie*). Écarté : « La fausse armée » (*Mensonge vivant*, empiète sur la Toile). | § 13.5 |
| D166 | 2026-10-09 | Arbre de l'Ombre — La Toile | **« L'appât »** : sceau *Fils tendus* (Piégeurs ~15 % plus vite formés, +1 piège actif ; +1 ordre actif avec le *Comptoir*) ; talent clé *L'Appât* (la fausse armée de *Mensonge* posée sur des pièges alliés les déclenche quand elle est attaquée, signal de ~1 s conservé). *Mensonge* partagé entre Miroir et Toile. Vigilance : enchaînement appât, piège, *Signal de la Voix*, *Coup de grâce*. Écarté : « Le réseau » (*Toile d'or*, or par piège déclenché : bonus sans transformation, risque de boule de neige). | § 13.5 |
| D167 | 2026-10-10 | Arbre de l'Ombre — Rangées 2 à 6 | **Grille validée** (15 talents, § 13.5). Lame : *Sang-froid*, *Cuir noirci*, *Mot tranchant*, *Pas de l'ombre*, *Écho du Mot*. Miroir : *Chuchoteurs*, *Voile tenace*, *Mensonge tenace*, *Esprit troublé*, *Mots perfides*. Toile : *Réseau de passeurs*, *Mains habiles*, *Poison virulent*, *Carreaux empoisonnés*, *Double détente*. Écarté : *Ordres liés* (talent mort sans *Comptoir*). | § 13.5 |
| D168 | 2026-10-10 | Kit de duel du Champion Héritier | **« Le Sans-pair »** (l'utilisateur voulait un kit qui montre la supériorité) : *Ascendant* (passif : plus de Chaleur, plus les coups traversent la parade, plafonné), *Frappe de forge* (frappe chargée, consomme la Chaleur), *Mépris* (~3 s sans parade, coups reçus → Chaleur) ; pas de mobilité ; ultime de duel *Jugement de l'acier* (parade réduite de moitié, achève sous ~30 % PV). Écartés : « L'Enclume », « La Surchauffe », « Le Rythme de la forge » (« bof »), « Le Maître d'armes » (trop proche du Paladin), « L'Implacable » (annule les kits adverses). | § 13.6 |
| D169 | 2026-10-10 | Héritiers — Spécialisation palier 1 | **Le marteau ou l'enclume** : *Grande Forge* (toute la faction ~+20 % de dégâts, **sans** ignorance d'armure) ou *Halle des serments* (toute la faction ~+20 % de PV ; *Résistance légendaire* pour une unité ou un type d'unité, à préciser : sous ~20 % de PV, dégâts subis réduits). Principe : amplifier l'élite, jamais adoucir les pertes (« on coûte cher, on ne perd personne, mais on est très forts »). Écartés : « L'héritage ou la route » (*Arsenal des lignées* rembourse les pertes, *Relais de la lignée* atténue la faiblesse), *Ne pas tomber* (1 PV invulnérable), « Préserver ou surclasser », « La trempe ou la cadence ». *Cercle des Pairs* (population réduite, statistiques accrues) gardé comme piste pour le palier 2 ou l'arbre. | § 13.6 |
| D170 | 2026-10-10 | Héritiers — Cible de la *Résistance légendaire* | **Le Gardien de la Forge** (type d'unité) : la frontline qui tient seule une ligne reçoit la réduction de dégâts sous ~20 % de PV (*Halle des serments*, D169). Lisible, reste une statistique (D39). Écartés : les trois emblématiques (moins lisible, équilibrage), une unité désignée par le joueur (mécanique en plus). | § 13.6 |
| D171 | 2026-10-10 | Héritiers — Lame Ardente | **Homme d'armes** (infanterie lourde à pied, même catégorie d'armure, donc contrée par l'Arbalétrier) **plus rapide que tous les autres hommes d'armes du jeu**, avec une **charge** ; arme incandescente, brûlure. Pas une unité montée. | § 13.6 |
| D172 | 2026-10-10 | Héritiers — Spécialisation palier 2 | **La lame ou la flamme** : *Salle des Lames* (Lames Ardentes formées ~20 % plus vite, charge ~+25 % de dégâts, brûlure cumulable ×3) ou *Temple de la Flamme* (bénédictions des Prêtres ~+30 %, *Garde sacrée* : ~−30 % de dégâts sur une unité protégée, annoncé, recharge). Vigilance : cumul avec la *Halle des serments* sur le Gardien. Écartés : « L'ost restreint » (*Cercle des Pairs*), « Le Champion ou ses pairs ». | § 13.6 |
| D173 | 2026-10-10 | Arbre des Héritiers — Familles | **Le Marteau, l'Enclume, la Flamme** : *Le Marteau* (duel et combat : Chaleur, *Ascendant*, *Frappe de forge*, Lames Ardentes ; *Grande Forge*, *Salle des Lames*), *L'Enclume* (tenir, ne perdre personne : survie du Champion, Gardiens, *Résistance légendaire*, *Cor de l'Héritage* ; *Halle des serments*), *La Flamme* (commandement : *Transmission*, *Exemple*, Prêtres, *Armes chauffées à blanc* ; *Temple de la Flamme*). Vigilance : l'Enclume n'adoucit jamais les pertes ni la faiblesse. Écartés : « Les trois Héritages » (Arme / Sang / Nom), « Garder, Libérer, Transmettre » (aucune voie stratégique). | § 9.6, § 13.6 |
| D174 | 2026-10-10 | Arbre des Héritiers — Le Marteau | **« Le coup qui décide »** : sceau *Acier de lignée* (Chaleur ~15 % plus rapide en RTS et en duel ; charge des Lames Ardentes ~+10 % de dégâts) ; talent clé *Frappe fendante* (*Frappe de forge* à pleine Chaleur → onde en cône sur toutes les unités devant le Champion ; annoncée, esquivable, sans ignorance d'armure). Écartés : « L'arme qui ne refroidit pas » (*Incandescence*, proche de *Transmission*), « Le meneur de charge » (doublon du *Signal de la Voix*, D164). | § 13.6 |
| D175 | 2026-10-10 | Arbre des Héritiers — L'Enclume | **« La lignée tient »** : sceau *Trempe des serments* (Gardiens +1 armure ; Champion ~+10 % PV) ; talent clé *Exemple inébranlable* (l'aura *Exemple* donne aux alliés proches une *Résistance légendaire* atténuée, sans cumul avec celle des Gardiens ; jouable au prototype). La Flamme garde *Transmission* pour son talent clé. Écartés : *Cor des serments* (inutile avant le niveau 7, hors prototype), *Dernier debout* (héros seul, comeback frustrant). | § 13.6 |
| D176 | 2026-10-10 | Arbre des Héritiers — La Flamme | **« La Chaleur qui circule »** : sceau *Flamme des pères* (Prêtres ~15 % plus vite formés ; *Transmission* ~−15 % de recharge) ; talent clé *Héritage vivant* (armes embrasées tant que l'unité combat, plafond ~15 s ; chaque coup rend un peu de Chaleur au Champion). Écartés : *Feu de la victoire* (banalise la récompense du duel, D47), *Flamme sacrée* (dépend des Prêtres). | § 13.6 |
| D177 | 2026-10-10 | Arbre des Héritiers — Rangées 2 à 6 | **Grille validée avec amendements** (15 talents, § 13.6). Marteau : *Poigne héritée*, *Élan des Lames*, *Trempe rapide*, *Ascendant précoce*, *Braises*. Enclume : *Cuir de forge*, *Plates de lignée*, *Exemple tenace*, *Mépris d'acier*, *Mur de la lignée*. Flamme : *Encens de la forge*, *Prières ferventes*, *Flamme large*, *Gloire de la lignée*, *Feu des Prêtres*. Amendements de l'utilisateur : *Encens de la forge* donne +1 cible bénie à la fois (les bénédictions du Prêtre visent un nombre de cibles à sa portée, pas une zone) ; *Élan des Lames* passe de la distance de charge (« ne sert à rien ») aux dégâts de charge. | § 13.6 |
| D178 | 2026-10-10 | Bloc 4 — Ordre | **Palier 3, puis rangées 7 à 9, puis transversal** : rôle du palier 3, tronc commun, spécialisations des 6 factions ; ensuite rangées 7 à 9 faction par faction (elles s'appuient sur le *Cor*, les capacités 3 et l'ultime) ; enfin interface (§ 15), modes secondaires (§ 14.3), événements futurs (§ 12.3). Écartés : rangées 7 à 9 d'abord, transversal d'abord. | § 22 |
| D179 | 2026-10-10 | Palier 3 — Rôle | **« Finir ou renverser »** : tronc commun de fin de partie (siège lourd, poudre, technologies finales) ; pour chaque faction, deux spécialisations opposées, l'une pour conclure (percée, siège, pression), l'autre pour renverser (tenir, contre-attaquer). Choix fait en lisant l'état de la partie (§ 14.2). Vigilance : jamais de *rubber band* indexé sur le retard. Écartés : « Finir la partie » (boule de neige), « L'apogée de l'identité » (Héritiers sans signature, montée en puissance). | § 9.5, § 14.2 |
| D180 | 2026-10-10 | Palier 3 — Tronc commun | **Grille validée avec amendements** (§ 11.2) : Bastion, Trébuchet, Canon, Arquebusier ; Forge III, collecte III, *Fortifications*, *Balistique*, *Contrepoids*, *Levée de la garnison*, *Poudre raffinée*. **Règle (amendement de l'utilisateur) : aucune unité ne demande de recherche pour être débloquée, sauf contre-ordre** ; les unités arrivent avec le palier. **Révise D38** : la technologie « Armes à poudre » est supprimée. Rôle de la Fonderie à trancher. Poudre des Templiers : pas encore décidée. | § 6.1, § 7.1, § 11, § 11.2 |
| D181 | 2026-10-10 | Palier 3 — Fonderie | **Supprimée** : sans la recherche « Armes à poudre » (D180), elle n'avait plus de rôle. *Poudre raffinée* passe à la Forge. Le palier 3 ouvre le *Bastion* et la spécialisation. Écartés : Fonderie bâtiment de production de la poudre (exception à D128), Fonderie bâtiment de technologies de la poudre (peu de contenu). | § 11.2 |
| D182 | 2026-10-10 | Aube — Spécialisation palier 3 | **Le Jugement ou le Rempart** : *Autel du Jugement* (finir : *Jugement* moins cher, plus large, gros dégâts aux bâtiments) ou *Crypte des saints* (renverser : second pouvoir d'Honneur défensif au palier 3, contenu à préciser). Écartés pour l'Aube : « La procession ou les cloches » (*Reliquaire*, *Riposte de l'Aube*), **gardée comme piste pour les Templiers** ; « Le soleil ou le seuil » (choix de composition). | § 13.2 |
| D183 | 2026-10-10 | Aube — Pouvoir de la *Crypte des saints* | ***Veille des saints*** : pendant ~10 s, aucun bâtiment d'une zone (centre compris) ne descend sous 1 PV ; paysans ~50 % plus rapides à réparer. Zone annoncée ; jamais les unités (distinct de *Dernier Rempart*). Écartés : *Rappel des fidèles* (mobilité forte, tension avec la *Bannière*), *Lumière des reliques* (proche de l'Archer de l'Aube et de la *Charge*). | § 13.2 |
| D184 | 2026-10-10 | Légions — Spécialisation palier 3 | **L'Abomination ou le Cimetière** : *Fosse de suture* (finir : *Abomination*, siège vivant lent et cher, éclate en plusieurs cadavres à sa mort) ou *Cimetière maudit* (renverser : les ennemis morts dans une zone fixe autour de la base se relèvent en Squelettes temporaires, plafonnés). Écartés : « La marée ou la crypte » (recoupe la *Tour des nécromanciens*, héros seul), « La terre morte ou le cimetière » (corruption diffuse, proche de l'*Autel de sang*). | § 13.3 |
| D185 | 2026-10-10 | Templiers — Palier 3 face à « Finir ou renverser » | **Maréchal et Beffroi pour finir, Sénéchal pour renverser** : le Sénéchal gagne la ***Contre-croisade*** (« les cloches ») : après un assaut repoussé contre une citadelle ou la base, sa mini-croisade est aussitôt disponible, ciblant le camp ou la base de l'attaquant. **Nuance de l'utilisateur : le *Reliquaire* en plus** (revient sur le rejet de la *Vraie Croix*, D133) ; emplacement et distinction avec le Porte-gonfanon à préciser. Écartés : exception « trois façons de finir », Reliquaire à la place du Beffroi. | § 13.7 |
| D186 | 2026-10-10 | Templiers — Reliquaire | **Unité fixe du palier 3, hors commanderies** (débloquée avec le palier, D180) : char processionnel unique, lent, cher ; alliés proches +armure et ~+25 % de dégâts aux bâtiments et aux murs (pas de moral, distinct du Porte-gonfanon) ; avec une croisade, marche plus lente mais délai ~+20 %. **Nuances de l'utilisateur : non réparable ; sa perte n'applique pas *Désillusion*** (une relique, pas la Vraie Croix). Reconstructible après un long délai. Écartés : contingent automatique des croisades, capacité de citadelle. | § 13.7 |
| D187 | 2026-10-10 | Templiers — Poudre | **Oui, poudre « maltaise »** (nuances de l'utilisateur) : *Arquebusier maltais* et *Canon maltais* (noms provisoires), **plus primitifs que le socle, donc moins forts**, et **intégrés à la croisade**. Justification : l'Ordre de Malte a connu la poudre. À préciser : statut (variante ou écart de statistiques) et forme de l'intégration à la croisade. Écartés : canon seul, pas de poudre. | § 7.1, § 11.2, § 13.7 |
| D188 | 2026-10-10 | Templiers — Statut de la poudre maltaise | **Écart de statistiques, pas une variante** : unités du socle moins fortes et, *nuance de l'utilisateur*, **moins chères** ; nom et visuel propres, aucune capacité en plus (même procédé que la règle d'élite, D39). Règle des variantes respectée (Templiers : Frère convers seul ; rôle « Canon » : 2 variantes). Écartés : vraies variantes en exception, Arquebusier seul en variante. | § 7.1, § 13.7 |
| D189 | 2026-10-10 | Templiers — Poudre maltaise dans la croisade | **Contingent maltais hors enveloppe** : à partir du palier 3, chaque croisade reçoit **3 Canons maltais et 5 Arquebusiers maltais** (*précision de l'utilisateur*), en plus de l'enveloppe de D139 (maximum ~48). Vigilance : croisade plus grosse à son sommet (D130), plafond de performance. Écartés : poudre dans l'enveloppe des Pèlerins, seulement par *Prendre la croix*. | § 13.7 |
| D190 | 2026-10-10 | Performance — Plancher de sécurité | **Au minimum 2 000 unités simultanées** (exigence de l'utilisateur), même si cette valeur ne sera jamais atteinte : marge au-dessus du pire cas de population (~1 400, D03) et des unités hors population (croisades, serviteurs, invocations). **Batterie de stress tests** pendant tout le développement ; à détailler le moment venu. Complète D03 et D29. | § 5.3, § 16.4 |
| D191 | 2026-10-10 | Dragon — Spécialisation palier 3 | **La tempête ou le nid** : *Cercle des invocateurs* (finir : rituel ***Météore***, trois Mages canalisent ~5 s, zone annoncée, gros dégâts aux bâtiments et aux murs, effet de l'élément des Mages) ou *Nids gardiens* (renverser : les Nids élémentaires projettent leur élément sur les ennemis proches ; feu brûle, glace ralentit sans immobiliser, foudre en chaîne ; changer l'élément change la défense, délai D72). Écartés : *Aire du wyrm* (largage aérien, transport), *Rempart de givre* (proche du *Rempart béni*). | § 13.4 |
| D192 | 2026-10-10 | Ombre — Spécialisation palier 3 | **La tour livrée ou les passages** : *Chambre des traîtres* (finir : ***Tour livrée***, canalisation ~8 s d'un Maître des Ombres révélé contre une porte, tour ou mur, signal ~5 s, porte ouverte ou tour muette ~20 s ; jamais l'économie) ou *Passages secrets* (renverser : souterrains entre bâtiments du Cercle, sorties visibles, aucune vision). Écartés : *Base piégée* (recoupe l'*Atelier du piégeur*), « Le discours ou la retraite » (héros seul, camouflage de masse). | § 13.5 |
| D193 | 2026-10-10 | Héritiers — Palier 3 | **Aucune spécialisation** (choix de l'utilisateur : « ils n'ont rien de plus, leurs stats feront la différence ») : tronc commun seul. Exception assumée à D07 et D179, cohérente avec D39 (4 combinaisons au lieu de 8). Vigilance : équilibrage de fin de partie par les seules statistiques. Écartés : « Le Cercle des Pairs ou le Foyer », « La Grande Bombarde ou le Foyer ». | § 9.5, § 13.6 |
| D194 | 2026-10-10 | Arbres de talents — Règles des rangées 7 à 9 | **Continuité des rangées 4 à 6** : modifications de capacités, valeur comparable par rangée ; cibles : capacité 3 (niveau 7), capacités antérieures, contenu fixe de la faction ; jamais une spécialisation ; aucun talent sur l'ultime ; contenu du palier 3 seulement à la rangée 9. Écartés : « rangée de légende » sur l'ultime, transformation de la capacité 3 à la rangée 7. | § 9.6 |
| D195 | 2026-10-10 | Arbre de l'Aube — Rangées 7 à 9 | **Grille validée** (9 talents, § 13.2). Gardien : *Bannière bénie*, *Rempart éternel*, *Jugement des assiégeants*. Capitaine : *Charge disciplinée*, *Ordre serré*, *Renforts aguerris*. Champion : *Fer de lance*, *Sentence publique*, *Verdict des armes*. | § 13.2 |
| D196 | 2026-10-10 | Arbre des Légions — Rangées 7 à 9 | **Grille validée** (9 talents, § 13.3). Moisson : *Moisson des relevés*, *Frappe insatiable*, *Grand festin*. Charnier : *Relève nombreuse*, *Linceul*, *Ossuaire profond*. Effroi : *Relevés effrayants*, *Marque funeste*, *Bile maudite*. | § 13.3 |
| D197 | 2026-10-10 | Arbre des Templiers — Rangées 7 à 9 | **Grille validée avec amendement** (12 talents, § 13.7). Croisade : *Croix de rappel*, *Haltes de pèlerins*, *Grande procession*. Chevalerie : *Sortie des chevaliers*, *Gonfanon haut*, *Règle de fer*. Trésor : *Compagnie aguerrie*, *Frères de la commanderie*, *Poudre payée comptant*. Citadelles : *Sortie prolongée*, *Mâchicoulis*, *Arsenal de la citadelle*. Amendement de l'utilisateur : *Sortie croisée* écartée (« la Sortie sert de défense ») ; *Colonne serrée* écartée au profit de *Croix de rappel*. | § 13.7 |
| D198 | 2026-10-10 | Arbre du Dragon — Rangées 7 à 9 | **Grille validée** (9 talents, § 13.4). Le Ciel : *Cri perçant*, *Piqué foudroyant*, *Souffle adulte*. La Lignée : *Lame du wyrm*, *Garde d'honneur*, *Champions de la couvée*. La Mue : *Cri élémentaire*, *Mages harmoniques*, *Bond élémentaire*. | § 13.4 |
| D199 | 2026-10-10 | Arbre de l'Ombre — Rangées 7 à 9 | **Grille validée** (9 talents, § 13.5). La Lame : *Sang-froid du Menteur*, *Lames promptes*, *Vraie menace aiguisée*. Le Miroir : *Serment élargi*, *Reflets tenaces*, *Mensonge mouvant*. La Toile : *Pose à distance*, *Poison lent*, *Pièges de siège*. | § 13.5 |
| D200 | 2026-10-10 | Arbre des Héritiers — Rangées 7 à 9 | **Grille validée** (9 talents, § 13.6). Le Marteau : *Cor de guerre*, *Frappe chauffée à blanc*, *Bombarde de lignée*. L'Enclume : *Cor du rempart*, *Serment tenu*, *Arquebusiers gardés*. La Flamme : *Cor de la flamme*, *Bénédiction de la forge*, *Salve embrasée*. | § 13.6 |
| D201 | 2026-10-10 | Interface — Structure du HUD | **Base AoE4 avec un emplacement de faction fixe** : barre du bas (mini-carte, sélection, commandes), ressources en haut, **bloc du héros permanent** (portrait, niveau, XP, vie, capacités, récupération ou duel), **emplacement de faction au même endroit pour les 6 factions** (Honneur, croisade, Ossuaire, élément, ordres, Chaleur). Écartés : livre de pouvoirs façon BFME, HUD minimal contextuel. | § 15 |
| D202 | 2026-10-10 | Interface — Alertes | **Deux niveaux, plus signaux dans le monde** (options A et C retenues ensemble) : majeures (croisade contre vous, événement mondial, *Tour livrée*, attaque du centre) = bandeau en haut, voix d'un héraut de faction, ping, caméra au clic ; mineures = fil discret, ping, son par type. En plus : icônes animées sur la mini-carte et signaux sur le terrain (colonne de croisade, fumée d'événement, signal de *Tour livrée*). Le défi garde sa fenêtre (D117). Écarté : fil unique sans hiérarchie. | § 15 |
| D203 | 2026-10-10 | Interface — Lecture de l'adversaire | **Inspection seulement** : fiche d'un héros, d'une unité ou d'un bâtiment adverse visible (héros : niveau, sceaux, talents clés) ; spécialisation reconnue à son bâtiment ou à son unité. Sans vision, rien. Écartés : carnet de renseignements (mémoire des observations), informations publiques (palier, spécialisations). | § 15 |
| D204 | 2026-10-10 | Modes de jeu | **Standard avec options Nomade, Régicide et Catastrophe** (choix de l'utilisateur) ; Domination, Reliques et Scénario **après la création du prototype**. Règles de Nomade et de Régicide, calendrier des options : à préciser (Régicide : D206). | § 14.1, § 14.3 |
| D205 | 2026-10-10 | Options de partie — Calendrier | **Nomade, Régicide et Catastrophe dans le prototype** (choix de l'utilisateur). **Révise D28** : le système d'événements mondiaux et le volcan (seul événement défini) entrent dans le prototype ; les autres événements restent pour après. Écartés : options conçues maintenant et développées après le prototype ; Nomade et Régicide seuls dans le prototype. | § 12.2 bis, § 14.3, § 17 |
| D206 | 2026-10-10 | Option Régicide | **Un Roi façon AoE2** (définition de l'utilisateur) : unité qui apparaît près du centre principal au début de la partie, hors population, attaque de 1 ; **sa mort fait perdre le joueur**. Ce n'est pas le héros : héros, duel et résurrection inchangés (pas d'exception au § 14.1). Par défaut, à confirmer : jamais converti, retourné ni relevé. Écartés : « Trois couronnes », « Le roi tombe » (héros), « pas en duel ». | § 14.3 |
| D207 | 2026-10-10 | Option Nomade | **Façon AoE2** : sans centre principal, travailleurs dispersés en petits groupes à des endroits aléatoires, de quoi construire un centre ; le héros apparaît avec un groupe ; en Régicide, **le Roi apparaît avec un travailleur** (nuance de l'utilisateur). Vigilance : distance minimale entre joueurs. Écarté : Nomade d'AoE4 (départ à la position habituelle). | § 14.3 |
| D208 | 2026-10-10 | Option Catastrophe au prototype | **Le volcan seul, sur plus de sites** : 3 à 4 éruptions par partie, fenêtres plus précoces, **préavis inchangé** (60 à 90 s). Écartés : second événement au prototype (inondation), préavis raccourci (contraire au § 12.2). | § 12.2 bis, § 14.3 |
| D209 | 2026-10-10 | Événements futurs | **Quatre événements contrastés, dans l'ordre** : inondation, invasion de monstres, ouverture d'une faille, tempête (sans retrait de vision). Reportés : séisme, incendie, créature, climat. **Retirée : corruption magique** (contraire à D40). Écartés : liste entière sans ordre ; événements d'opportunité seulement. | § 12.3 |
| D210 | 2026-10-10 | Soigneur commun — Autres factions | **Légions sans soigneur** (elles recyclent : Squelettes, *Sacrifice* ; unités moins chères) ; **Dragon et Ombre : Moine commun** (apparence propre) ; **Héritiers : Moine commun en version élite** (×2), le Prêtre de la Flamme ne soigne pas directement. Complète D134. Écartés : *Embaumeur* (variante macabre des Légions), Moine commun pour tous. | § 7.1 |
| D211 | 2026-10-10 | Soigneur commun — Bâtiment de production | **Une *Chapelle* commune** (nom provisoire), façon Monastère d'AoE4 : produit le soigneur, une ou deux technologies de soin, apparence par faction, absente chez les Légions. Palier : voir D212 (palier 1). Écartés : Caserne, centre principal. | § 7.1, § 11.1 |
| D212 | 2026-10-10 | Soin — Palier | **Le soin commence au palier 1 (~6-8 min), jamais au palier 0** : la **Chapelle** est au **palier 1**, comme le Moine et le Moine Lumineux (D129, D134 inchangés sur ce point). Correction : l'utilisateur avait d'abord dit « palier 2 ou 3 », puis « pas de soin au palier 1, tout décalé au palier 2 », en comptant les paliers à partir de 1 ; après rappel du numérotage (palier 0 = début), il confirme le palier 1. Précise D211. | § 7.1, § 11.1 |
| D213 | 2026-10-10 | Aube — *Lumière sacrée* au palier 0 | **Exception assumée** (l'utilisateur a d'abord répondu A, puis B) : *Lumière sacrée* **soigne dès le palier 0**, seule exception à « aucun soin au palier 0 » (D212). Pouvoir payé en Honneur, qui se gagne lentement en ouverture : effet rare ; identité défensive de l'Aube dès le début. Écarté : soin repoussé au palier 1. | § 13.2 |
| D214 | 2026-10-10 | Technique — Version du moteur | **Migration vers Unreal Engine 5.8** (dernier correctif 5.8.x) avant le premier chantier de code, puisque le module de code est vide : Iris (réplication) prêt pour la production, Mass refondu avec suivi de chemin sur le navmesh, navmesh plus économe. 5.8 est la dernière version d'Unreal 5 (accès anticipé à Unreal 6 annoncé fin 2027) : c'est la version de travail du prototype. Assets réenregistrés au format 5.8 (irréversible, historique dans git). Rouvre la question Mass ou gestionnaire maison. | § 16 |
| D215 | 2026-10-10 | Technique — Moteur de simulation des unités | **Hybride** : Mass (cœur : entités, traitements, suivi de navmesh, évitement, représentation en instances, StateTree) + couche RTS maison (ordres de groupe et formations, combat, réplication via Iris, brouillard de guerre). Tout-Mass écarté (MassReplication sans filtrage par brouillard) ; tout-maison gardé en repli si l'essai de départ (~2 jours, 2 000 entités sur le navmesh, rendu en instances) échoue. Précise D29. | § 16.4 |
| D216 | 2026-10-10 | Technique — Calendrier du réseau | **Tranche réseau minimale tôt, puis gardée en vie** : architecture prête pour le réseau dès M1 (simulation séparée de la présentation, file de commandes) ; étape **M2.5** juste après le rendu : réplication des positions et états via Iris, serveur hébergé + 2 clients, mesure de bande passante à 2 000 unités en mouvement. Ensuite, développement en solo, mais une vérification à 2 clients doit passer à la fin de chaque étape. Le brouillard (M7) se branche sur ce socle. | § 16.6 |
| D217 | 2026-10-10 | Technique — Fréquence de simulation | **Pas fixe à 20 Hz** (50 ms) pour toute la simulation (Mass et couche RTS), piloté par un accumulateur et indépendant des images par seconde de l'hôte ; le client interpole à la fréquence de l'écran ; envoi des états à 10 ou 20 Hz selon le budget mesuré à M2.5. Fréquence réglable en configuration (essais à 15 ou 30 Hz possibles). Stress tests mesurés en millisecondes par tick. | § 16.6 |
| D218 | 2026-10-10 | Technique — Seuils des stress tests | **Niveau exigeant** (choix de l'utilisateur, contre la recommandation « avec marge ») ; machine de référence = poste de développement (Ryzen 7 5700X, RTX 5060 Ti, 64 Go), scénarios S1 à S6 à 2 000 unités : simulation serveur **≤ 3 ms** en moyenne par tick de 50 ms ; rendu client **≥ 144 i/s** (1080p, qualité haute, 2 000 unités à l'écran) ; bande passante **≤ 32 Ko/s** par client ; envoi de l'hôte **≤ ~225 Ko/s** (7 clients). Un seuil dépassé bloque l'étape ; un seuil ne se révise que par une décision consignée au journal. Mémoire mesurée et suivie, sans seuil. Valeurs indicatives, à régler en test. | § 16.6 |
| D74 | 2026-10-06 | Cohérence — Le moral | **Famille d'effets, sans jauge** (façon AoE4 / BFME) : effets nommés et temporaires (attaque, armure, cadence) regroupés dans une catégorie « moral » (affichage, cumul plafonné, purification par le Moine). Jamais de déroute. Moral de groupe à états : extension possible après le prototype. | § 7.2 |

---

## 22. Reprise de la review

*Section de travail : elle indique où en est la review question par question du GDD. À mettre à jour à chaque séance.*

**Méthode :** une question à la fois, 2 à 4 options (A/B/C) avec leurs conséquences et une recommandation. Chaque réponse est consignée dans le journal (§ 21, numéro D suivant : **D219**), reportée dans le corps du document, et le ⚠️ correspondant est retiré de la liste du § 20.

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

- **Décisions prises :** D01 à D213. Prochain numéro : **D214**.
- **Séance du 2026-10-07 (suite) : piste 3, arbres de talents (D08), choisie par l'utilisateur.** Questions dans l'ordre :
  1. ~~**Forme de l'arbre**~~ → **tranché (D103)** : rangées de 3 talents par niveau, sceau à 3 talents d'une famille, talent clé à 5.
  2. ~~**Nombre de familles**~~ → **tranché (D104)** : 3, sauf les Templiers (4, rangées de 4 talents).
  3. ~~**Moment du choix**~~ → **tranché (D105)** : réserve libre, rangées dans l'ordre, bouton « choix conseillé ».
  4. **Arbre de l'Ordre de l'Aube** : ~~familles~~ → **tranché (D106)** : les trois Serments (Gardien, Capitaine, Champion). Ensuite : sceau et talent clé de chaque Serment (~~Gardien~~ → D107 ; ~~Capitaine~~ → D108 ; ~~Champion~~ → D109), puis talents des rangées 2 à 6 (prototype) : méthode → D110 ; ~~grille des 15 talents~~ → **tranché (D111)**. Talents des rangées 7 à 9 (hors prototype) : plus tard.
  5. **Arbre des Légions Noires** : mêmes étapes (familles, sceaux et talents clés, rangées 2 à 6). ~~Familles~~ → **tranché (D112)** : les trois Rites (Moisson, Charnier, Effroi). Ensuite : sceau et talent clé de chaque Rite (~~Moisson~~ → D113 ; ~~Charnier~~ → D114 ; ~~Effroi~~ → D115), ~~grille des rangées 2 à 6~~ → **tranché (D116)**. **Les arbres des deux factions du prototype sont complets** (rangées 7 à 9 hors prototype : plus tard).
- **Séance du 2026-10-08 : finir entièrement le GDD avant le code** (demande de l'utilisateur). Ordre choisi : **le prototype d'abord** (option A).
  1. **Bloc 1 — Prototype (Aube, Légions) :** description du duel validée par l'utilisateur (D117 fenêtre de défi, D118 postures) ; ~~structure commune des kits de duel~~ → **tranché (D119)** ; ~~déplacement dans le cercle~~ → **tranché (D120)** ; ~~kit de duel du Paladin~~ → **tranché (D121)** ; ~~kit de duel du Seigneur~~ → **tranché (D122)** ; ~~spécialisations de l'Aube~~ → **tranché (D123, D124)** ; ~~purification~~ → **D125** (atténuation) ; ~~spécialisations des Légions~~ → **tranché (D126, D127)** ; ~~production militaire~~ → **tranché (D128)** ; ~~tronc commun des paliers 0 à 2~~ → **tranché (D129)**. **Bloc 1 terminé.** ; spécialisations des paliers 1 et 2 (D07) des deux factions ; bâtiments (tronc commun et propres) et arbre des technologies des paliers 0 à 2.
  2. **Bloc 2 — Templiers** (*en cours*) : ~~point fort (timing)~~ → **tranché (D130)** : solides en continu, sommet pendant une croisade. Commanderies : ~~mode de proposition~~ → **tranché (D131)** : 3 fixes par palier, 9 au total ; ~~structure~~ → **tranché (D132)** : trois voies Fer / Foi / Pierre, unités propres aux Templiers ; ~~grille des unités~~ → **D133** : 7 validées, Foi paliers 2 et 3 à remplacer ; ~~moine soigneur au socle~~ → **tranché (D134)** : oui, variantes multiples autorisées ; ~~voie de la Foi 2 et 3~~ → **tranché (D135)** : Prêcheur + Sénéchal (mini-croisade) ; ~~mini-croisade~~ → **tranché (D136)** : croisade en miniature, ~30 % max ; ~~cohérence du Maréchal~~ → **tranché (D137)** : contingent d'Escadron, recruté suivi par la croisade. **Grille des commanderies terminée.** ~~Récompenses de croisade~~ → **tranché (D138)** : XP modérée, recharge normale. ~~Plafond de croisade~~ → **tranché (D139)** : ~10 par commanderie, population propre ; mini-croisade dans la population du joueur. ~~Place des bonus~~ → **tranché (D140)** : enveloppe, surplus en vétérance. ~~Murs~~ → **tranché (D141)** : chemin normal, sinon attaque du mur. **Bloc 2 terminé (D130 à D141).** Valeurs (délai, or, tailles de contingent, % de mini-croisade) : fiche de valeurs, à régler en test.
  3. **Bloc 3 — Autres factions** (Dragon, Ombre, Feu, Templiers) : arbres de talents, kits de duel détaillés, spécialisations de palier. **Ordre choisi (option A) : faction par faction, Templiers d'abord**, puis Dragon, Ombre, Feu. Templiers : kit de duel détaillé (gabarit D119), puis talents des 4 familles (D101). ~~Kit de duel du Grand Maître~~ → **tranché (D142)** : « Le Martyr ». Ensuite, arbre des Templiers, mêmes étapes que l'Aube : sceau et talent clé de chaque famille (Croisade, Chevalerie, Trésor, Citadelles), puis grille des rangées 2 à 6 (20 talents). ~~Croisade~~ → **D143** (« Dieu le veut »). ~~Chevalerie~~ → **D144** (« Frères du Temple »). ~~Trésor~~ → **D145** (« Les banquiers de la chrétienté »). ~~Les Citadelles~~ → **D146** (« Le réseau du Temple », camp hors croisades). **Sceaux et talents clés terminés.** ~~Grille des rangées 2 à 6~~ → **tranché (D147)**. **Templiers terminés (D142 à D147)**, hors rangées 7 à 9. **Enfants du Dragon (en cours)** : kit de duel détaillé (gabarit D119, éléments à la place des postures), puis spécialisations des paliers 1 et 2, puis arbre de talents (familles, sceaux et talents clés, rangées 2 à 6). ~~Kit de duel du Seigneur-Dragon~~ → **tranché (D148)** : « Le wyrm veille ». ~~Bêtes~~ → **tranché (D149)** : aucune bête dans la faction (spécialisations A et B du palier 1 à reproposer sans *Enclos des drakes*). ~~Troisième emblématique~~ → **tranché (D150)** : Garde d'écailles. ~~Palier 1~~ → **tranché (D151)** : les arcanes ou les écailles. ~~Palier 2~~ → **tranché (D152)** : le ciel ou le sol. **Spécialisations du Dragon terminées.** Ensuite, arbre de talents. ~~Familles~~ → **tranché (D153)** : Le Ciel, La Lignée, La Mue. ~~Le Ciel~~ → **D154** (« La Bête ailée »). ~~La Lignée~~ → **D155** (« Le sang du wyrm »). ~~La Mue~~ → **D156** (« L'instinct du wyrm »). **Sceaux et talents clés du Dragon terminés.** ~~Grille des rangées 2 à 6~~ → **tranché (D157)**. **Enfants du Dragon terminés (D148 à D157)**, hors rangées 7 à 9. **Cercle de l'Ombre (en cours)** : kit de duel détaillé, spécialisations des paliers 1 et 2, arbre de talents. ~~Kit de duel de la Voix~~ → **tranché (D158)** : « Le Menteur ». ~~Piégeur (unité jugée frustrante par l'utilisateur)~~ → **tranché (D159)** : « L'embuscade annoncée ». ~~Palier 1~~ → **tranché (D160)** : la bourse ou le terrain (palier 2 : la lame ou l'esprit). ~~Effet du *Comptoir*~~ → **tranché (D161)** : « Les ordres ». ~~Palier 2~~ → **tranché (D162)** : « Finir le travail ». **Spécialisations de l'Ombre terminées.** Arbre de talents : ~~familles~~ → **tranché (D163)** : La Lame, le Miroir, la Toile. ~~La Lame~~ → **D164** (« Le signal » ; *Mot d'arrêt* passe à la Lame). ~~Le Miroir~~ → **D165** (« La rumeur »). ~~La Toile~~ → **D166** (« L'appât »). **Sceaux et talents clés de l'Ombre terminés.** ~~Grille des rangées 2 à 6~~ → **tranché (D167)**. **Cercle de l'Ombre terminé (D158 à D167)**, hors rangées 7 à 9. **Héritiers du Feu (en cours)** : kit de duel détaillé (gabarit D119), spécialisations des paliers 1 et 2, arbre de talents (familles, sceaux et talents clés, rangées 2 à 6). ~~Kit de duel du Champion Héritier~~ → **tranché (D168)** : « Le Sans-pair ». ~~Palier 1~~ → **tranché (D169)** : le marteau ou l'enclume. ~~Cible de la *Résistance légendaire*~~ → **tranché (D170)** : le Gardien de la Forge. ~~Lame Ardente~~ → **tranché (D171)** : homme d'armes rapide avec une charge. ~~Palier 2~~ → **tranché (D172)** : la lame ou la flamme. **Spécialisations des Héritiers terminées.** Ensuite, arbre de talents. ~~Familles~~ → **tranché (D173)** : le Marteau, l'Enclume, la Flamme. Ensuite : sceau et talent clé de chaque famille, puis grille des rangées 2 à 6. ~~Le Marteau~~ → **D174** (« Le coup qui décide »). ~~L'Enclume~~ → **D175** (« La lignée tient »). ~~La Flamme~~ → **D176** (« La Chaleur qui circule »). **Sceaux et talents clés des Héritiers terminés.** ~~Grille des rangées 2 à 6~~ → **tranché (D177)**. **Héritiers du Feu terminés (D168 à D177)**, hors rangées 7 à 9. **Bloc 3 terminé.**
  4. **Bloc 4 — Fin de partie et transversal** (*en cours*) : rangées 7 à 9, palier 3, interface (§ 15), modes secondaires (§ 14.3), événements futurs (§ 12.3). ~~Ordre~~ → **tranché (D178)** : palier 3 d'abord (rôle du palier, tronc commun, puis spécialisation des 6 factions), puis rangées 7 à 9 faction par faction, puis interface, modes et événements. ~~Rôle du palier 3~~ → **tranché (D179)** : « Finir ou renverser ». ~~Tronc commun du palier 3~~ → **tranché (D180)** : grille validée (§ 11.2) ; unités débloquées avec le palier, sans recherche. ~~Fonderie~~ → **tranché (D181)** : supprimée, *Poudre raffinée* à la Forge. Ensuite, spécialisations du palier 3, faction par faction : Aube, Légions, Templiers (avec leur poudre), Dragon, Ombre, Feu. ~~Aube, palier 3~~ → **tranché (D182)** : le Jugement ou le Rempart. ~~Pouvoir de la Crypte~~ → **tranché (D183)** : *Veille des saints*. ~~Légions, palier 3~~ → **tranché (D184)** : l'Abomination ou le Cimetière. ~~Templiers, palier 3~~ → **tranché (D185)** : Maréchal et Beffroi pour finir, Sénéchal pour renverser (*Contre-croisade*), avec le *Reliquaire* en plus. ~~Emplacement du Reliquaire~~ → **tranché (D186)** : unité fixe du palier 3, non réparable, sans *Désillusion*. ~~Poudre des Templiers~~ → **tranché (D187)** : poudre maltaise, plus primitive, intégrée à la croisade. ~~Statut de la poudre maltaise~~ → **tranché (D188)** : écart de statistiques, moins chère. ~~Poudre maltaise dans la croisade~~ → **tranché (D189)** : contingent hors enveloppe, 3 Canons et 5 Arquebusiers. **Templiers, palier 3 terminé.** ~~Dragon, palier 3~~ → **tranché (D191)** : la tempête ou le nid. ~~Ombre, palier 3~~ → **tranché (D192)** : la tour livrée ou les passages. ~~Héritiers, palier 3~~ → **tranché (D193)** : aucune spécialisation, leurs statistiques font la différence. **Spécialisations du palier 3 terminées (D179 à D193).** Ensuite, rangées 7 à 9, faction par faction (Aube, Légions, Templiers, Dragon, Ombre, Feu). ~~Règles des rangées 7 à 9~~ → **tranché (D194)** : continuité des rangées 4 à 6 (§ 9.6). ~~Aube, rangées 7 à 9~~ → **tranché (D195)**. ~~Légions, rangées 7 à 9~~ → **tranché (D196)**. ~~Templiers, rangées 7 à 9~~ → **tranché (D197)**. ~~Dragon, rangées 7 à 9~~ → **tranché (D198)**. ~~Ombre, rangées 7 à 9~~ → **tranché (D199)**. ~~Héritiers, rangées 7 à 9~~ → **tranché (D200)**. **Arbres de talents complets pour les 6 factions (rangées 2 à 9).** Ensuite, transversal : interface (§ 15), modes secondaires (§ 14.3), événements futurs (§ 12.3). Interface : ~~structure du HUD~~ → **tranché (D201)**. ~~Alertes~~ → **tranché (D202)** : deux niveaux, héraut de faction, plus signaux dans le monde et sur la mini-carte. ~~Lecture de l'adversaire~~ → **tranché (D203)** : inspection seulement. ~~Modes~~ → **tranché (D204)** : Standard avec options Nomade, Régicide, Catastrophe ; le reste après le prototype. ~~Calendrier des options~~ → **tranché (D205)** : les trois dans le prototype (révise D28 : système d'événements et volcan au prototype). ~~Régicide~~ → **tranché (D206)** : un Roi près du centre, hors population, attaque 1 ; sa mort fait perdre. ~~Nomade~~ → **tranché (D207)** : façon AoE2, le Roi apparaît avec un travailleur. **Options de partie terminées.** ~~Catastrophe au prototype~~ → **tranché (D208)** : volcan seul, 3 à 4 éruptions, préavis inchangé. ~~Événements futurs~~ → **tranché (D209)** : inondation, invasion de monstres, faille, tempête. **Bloc 4 terminé (D178 à D209). Les 4 blocs de conception sont terminés.** **Pistes notées pour les Templiers (palier 3)** : « La procession ou les cloches » (*Reliquaire*, *Cloches* et *Riposte*, proposée pour l'Aube, jugée « parfaite pour les Templiers » par l'utilisateur), à confronter aux commanderies du palier 3 déjà fixées (Maréchal, Sénéchal, Beffroi, D133, D135) et au principe D179 ; poudre : Ordre de Malte (voir § 11.2).
  - Les valeurs chiffrées restent pour la fiche de valeurs (à régler en test).
- **Séance du 2026-10-10 : blocs 3 et 4 terminés (D168 à D209).** Héritiers du Feu (D168 à D177) ; palier 3 « Finir ou renverser » (D179 à D193) ; rangées 7 à 9 des 6 factions (D194 à D200) ; interface (D201 à D203) ; modes et options (D204 à D208) ; événements futurs (D209) ; performance : plancher de 2 000 unités et stress tests (D190). **Toute la conception prévue est faite.** Restes ouverts :
  1. ~~Soigneur des autres factions~~ → **tranché (D210)** : Légions sans soigneur, Moine commun ailleurs (élite chez les Héritiers). ~~Bâtiment du Moine~~ → **tranché (D211)** : une *Chapelle* commune. ~~Palier de la Chapelle~~ → **tranché (D212)** : soin à partir du palier 1, Chapelle au palier 1. ~~Lumière sacrée~~ → **tranché (D213)** : exception assumée, elle soigne dès le palier 0. **Plus aucun point de conception ouvert** (hors valeurs à régler en test).
  2. Courbe d'XP (Q04) et toutes les valeurs « à régler en test » : fiche de valeurs.
  3. Batterie de stress tests (D190) : à concevoir le moment venu.
- **Code :** l'ancien code (classes C++ de gameplay et `Content/Blueprints`) a été supprimé et commité. Le module `MyCrusader` est vide et compile. À la première ouverture dans l'éditeur, `BattleMap` peut signaler des acteurs dont la classe n'existe plus : les supprimer et enregistrer la carte.
- **Pistes pour la suite, au choix de l'utilisateur :**
  1. **Premier chantier de code :** système d'unités légères (D29, § 16.4) : lire le projet, rédiger un plan (Mass Entity ou gestionnaire maison), le valider avant d'écrire du code.
  2. **Contenu détaillé des Templiers :** les ~8 commanderies et leurs unités (D90, D92), valeurs de la croisade (délai, plafond, or, gloire).
  3. ~~**Arbres de talents**~~ → fait (D103 à D116) pour le prototype. Restent : rangées 7 à 9, et les arbres des quatre autres factions (Cercle de l'Ombre, Héritiers du Feu, Enfants du Dragon, Templiers).
  4. **Fiche de valeurs de départ** pour tout ce qui est « à régler en test » (ci-dessous).

**À régler en test plutôt qu'en discussion :** courbe d'XP (Q04), valeurs de récupération et de prix de résurrection (D25), pourcentages d'aura (D26), valeurs de terrain (D33), bonus, coût et délai de changement des *Nids élémentaires* (D72), coût et nombre des pouvoirs d'Honneur (D75, D85), cadavres et Squelettes (D86), croisade : délai, plafond de troupes, or, gloire, bonus des citadelles (D88 à D96).

**Premier chantier de code identifié :** construire le système d'unités légères (D29, § 16.4). L'ancien code (`AUnitBase : ACharacter` et ses Blueprints) a été supprimé le 2026-10-07.

**Séance du 2026-10-10 (suite) : chantier de code ouvert.** L'utilisateur a choisi le premier chantier de code (unités légères, D29), avec la batterie de stress tests (D190) intégrée au plan, plutôt que la fiche de valeurs (gardée pour quand le prototype tournera, avec des valeurs provisoires dans les Data Tables). Plan technique en cours de validation, question par question : ~~version du moteur~~ → **tranché (D214)** : migration vers Unreal 5.8 ; ~~moteur de simulation~~ → **tranché (D215)** : hybride Mass + couche RTS maison, validé par un essai de départ ; ~~calendrier du réseau~~ → **tranché (D216)** : tranche réseau M2.5, plan au § 16.6 ; ~~fréquence de simulation~~ → **tranché (D217)** : pas fixe à 20 Hz ; ~~machine de référence et seuils des stress tests~~ → **tranché (D218)** : niveau exigeant. **Plan validé (D214 à D218, § 16.6).** Migration en 5.8.2 faite (moteur installé dans `C:/UE/UE_5.8_AS_1`, `Target.cs` passés en `BuildSettingsVersion.V7` et `Unreal5_8`), l'éditeur s'ouvre. Prochaine étape : fin de M0 (modules et plugins Mass, carte de test), puis l'essai Mass de M1.
