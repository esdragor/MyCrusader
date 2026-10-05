# GAME DESIGN DOCUMENT — RTS FANTASY

**Version 0.3** — Préproduction
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

La population maximale augmente grâce à certains bâtiments et technologies.

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

- **Poudre (D38) :** le Canon s'ajoute aux autres engins, il ne remplace rien. Variante des Héritiers du Feu : la *Bombarde*. Les Légions Noires n'ont pas de poudre (§ 13.3).

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
| Contre-mesure anti-héros | proposition : l'Arbalétrier (§ 7.2) | — |

Les détails des contres sont au § 7.2 (D21).

**Poudre *(décision D38)* : commune à toutes les factions, sauf contre-ordre, au palier 3.**

- La technologie commune **« Armes à poudre »** (palier 3) débloque deux nouvelles unités du socle : l'**Arquebusier** (tireur, catégorie d'armure distance) et le **Canon** (siège, § 6.1). Elles s'ajoutent à l'Arbalétrier et aux engins existants.
- **Exceptions de faction** (règle des variantes ci-dessous) :
  - **Légions Noires :** pas de poudre. ⚠️ Une réponse équivalente de fin de partie reste à définir pour ne pas les désavantager.
  - **Héritiers du Feu :** *Bombarde* à la place du Canon, et une variante de l'Arquebusier (nom et spécificité à définir). Ce sont leurs deux variantes du socle.
- ⚠️ **Points de vigilance :** l'Arquebusier chevauche l'Arbalétrier (tireur anti-armure) : place dans la matrice de contres (§ 7.2) à définir. Il ne doit pas devenir un anti-héros trop fort (D17).

**Amélioration et déblocage *(décision D38)* : une unité n'est jamais remplacée en cours de partie.**

- Une amélioration (technologie, palier) ne change une unité **qu'en statistiques et en visuel** : elle garde son nom, son rôle et ses capacités de base.
- La nouveauté passe par le **déblocage de nouvelles unités**, qui s'ajoutent aux anciennes. Exemple : l'Arquebusier s'ajoute à l'Arbalétrier, il ne le remplace pas.
- Les variantes rares ci-dessous ne sont pas concernées : ce sont des choix de composition de la faction, présents dès le début de la partie.

**Règle des variantes : l'exception, jamais la généralité.**

- Pour un rôle donné, **une seule faction (deux au maximum)** remplace l'unité commune par une variante.
- Chaque faction a **une ou deux variantes au maximum** dans tout son socle.
- Une variante garde le rôle de l'unité commune, mais ajoute une **spécificité nette**.

**Exemples :**

- **Légions Noires — Zombie** à la place du Paysan : coûte moins cher (spécificité exacte à définir, par exemple pas de nourriture mais plus lent à la collecte).
- **Une autre faction — Hallebardier** à la place du Lancier : anti-cavalerie avec, en plus, une efficacité contre l'infanterie lourde, mais plus cher.

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
| **Arbalétrier** | tireur anti-armure | **armures lourdes** (Homme d'armes, Cavalerie lourde) ; proposition : aussi **anti-héros** | Cavalerie légère, Cavalerie lourde |
| **Cavalier léger** | raid, harcèlement, éclairage | **tireurs** et **siège** | Lancier, Cavalier lourd |
| **Cavalier lourd** | choc, **charge** dévastatrice | infanterie légère et tireurs (bonus de charge) | Lancier, Arbalétrier |
| **Siège** | anti-bâtiments, anti-groupes | bâtiments, formations serrées | toute unité au contact, Cavalier léger |

**Lecture pour le joueur :** cavalerie → lanciers ; tireurs → cavalerie ; armure lourde → arbalétriers ; masse d'infanterie légère → archers ; lanciers et archers → hommes d'armes ou cavalerie lourde.

**Proposition anti-héros (D17) :** l'Arbalétrier ignore une partie de la résistance héroïque. Il est la contre-mesure anti-héros **du socle commun**. Une faction peut en avoir une variante propre (règle D20).

**Unités emblématiques et variantes :** chacune se rattache à une catégorie d'armure et à une ligne de la matrice, avec une spécificité. Par exemple : Chevalier Vertueux = infanterie lourde à redirection de dégâts ; Spectre Assassin = unité rapide anti-tireurs ; Hallebardier = anti-cavalerie avec un bonus contre l'infanterie lourde.

**Mise en œuvre :** types de dégâts et catégories d'armure dans les fiches `UnitData` (multiplicateurs de bonus).

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

Explorer donne : information, XP potentielle, opportunités, accès à des ressources, et (hors prototype) des reliques uniques de héros cachées dans les ruines (§ 9.7). Le scouting doit être une activité stratégique réelle.

### 8.4 Unités neutres et monstres

Les créatures neutres peuvent protéger des ressources, occuper des ruines, bloquer des passages, devenir des objectifs, ou être exploitées par certaines factions :

- Enfants du Dragon : interaction supérieure avec les créatures ;
- Cercle de l'Ombre : utilisation de créatures comme diversion ;
- Légions Noires : réanimation éventuelle de certains cadavres.

**Présence légère et ciblée *(décision D32)*, après le prototype :**

- **Quelques camps par carte**, placés symétriquement, qui **gardent quelque chose de précieux** : gisement riche, ruine avec relique (D09), passage stratégique.
- **Pas de réapparition :** un camp nettoyé l'est pour la partie.
- **XP modérée**, comptée dans la part « Exploration + Événements » (~15 %, D05).
- Point d'appui pour les mécaniques de faction : apprivoisement (Enfants du Dragon), cadavres (Légions Noires), diversion (Cercle de l'Ombre).
- Pas de chasse aux monstres façon Warcraft 3 : le combat contre les neutres reste une décision ponctuelle, pas une voie de progression.

---

## 9. Le héros

**Un héros unique par faction *(décision D10)*.** Le héros incarne sa faction. La variété entre parties vient des spécialisations de palier (D07), des talents (D08) et de l'équipement (D09). Les fiches `HeroData` permettront d'ajouter d'autres héros plus tard sans refonte.

**Héros du prototype :** Paladin-Commandant de l'Ordre de l'Aube (§ 13.2, D12) et Seigneur Damné des Légions Noires (§ 13.3, D13).

**Capacité ultime *(décision D36)* :** voir § 9.5. Ultimes du prototype décrits aux § 13.2 et § 13.3.

**Héros hors prototype :** Seigneur-Dragon des Enfants du Dragon (§ 13.4, D41).

⚠️ Héros du Cercle de l'Ombre et des Héritiers du Feu (et leurs ultimes) non décrits **[Q08]**.

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
| **Dégâts normaux reçus de** | autres héros, tours, siège, et **contre-mesures dédiées** : une unité ou une technologie « tueuse de héros » par faction (à définir). |
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
| Fin de partie | jusqu'à **+20 à 25 %** avec les talents et l'équipement |
| Rayon | un **groupe de bataille** (~20 à 30 unités), pas toute l'armée |

**Valeur totale du héros :** sur une armée d'environ 90 places de population, +15 % vaut environ 13 unités. Ajoutée à sa puissance personnelle (~5-6 unités, D17), elle donne un héros qui vaut **environ 20 unités**. Il est important sans être indispensable : sa mort se ressent dans une bataille sans la décider à elle seule.

### 9.3 Capacités RTS

Capacités possibles : aura, charge, cri de guerre, soin, renforcement, mobilité, reconnaissance, invocation, capacité de faction.

Toutes les capacités ont des temps de recharge. Le héros ne doit pas pouvoir enchaîner ses capacités jusqu'à résoudre tous les combats.

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
- **Le niveau 10** débloque la capacité ultime du héros.

**Forme de l'ultime *(décision D36)* : une capacité de bataille active du kit RTS.**

- Effet massif sur une zone, de courte durée, avec une longue recharge (indicatif : 3 à 4 min).
- **Annoncé et contrable :** un signal visuel clair au lancement, et une réponse possible pour l'adversaire (reculer, changer de terrain, etc.).
- Le héros doit être sur place : pas d'effet à l'échelle de la carte, pas de transformation (pour garder le repère de D17).
- **L'ultime de duel est distinct :** il reste dans le kit de duel (§ 10.3) et n'est pas lié au niveau 10, pour qu'un héros de niveau 10 ne gagne pas presque tous ses duels contre un héros de niveau inférieur.
- **Serviteurs temporaires** créés par un ultime : hors population, mais plafonnés (proposition, à confirmer en test).
- Hors prototype (niveaux 1 à 6) ; la capacité est prévue dans `HeroData` dès le départ.

| Niveau | Héros | Civilisation (noms temporaires) | Moment indicatif (1v1, joueur actif) |
|---|---|---|---|
| 1 | kit de départ | **Palier 0 — Fondation** : technologies fondamentales, bâtiments de départ, unités de base | début |
| 2 | talent | — | |
| 3 | talent | **Palier 1 — Essor** : premières infrastructures avancées, nouvelles améliorations, nouvelles unités | ~ 6-8 min |
| 4 | talent | — | |
| 5 | talent, nouvelle capacité | — | |
| 6 | talent | **Palier 2 — Puissance** : bâtiments avancés, technologies spécialisées, unités élites | ~ 13-15 min |
| 7 | talent | — | |
| 8 | talent | — | |
| 9 | talent | **Palier 3 — Légende** : technologies finales, bâtiments majeurs, options de fin de partie | ~ 20-23 min |
| 10 | **ultime** | — | ~ 25 min et plus |

Ce qui est gagné à chaque niveau de héros (talent, statistiques, capacité) n'est pas encore figé : le tableau est indicatif.

**Important :** un palier ne donne rien automatiquement. Il ouvre un niveau d'accès ; le joueur doit encore construire et rechercher.

**Ouverture d'un palier *(décision D07)* : un tronc commun et une spécialisation.**

Chaque palier (1, 2 et 3) ouvre :

1. **Un tronc commun**, garanti pour toute la faction : bâtiments et technologies essentiels, dont **les réponses de base aux contres**. Un joueur n'est jamais privé de réponse à cause d'un choix de spécialisation.
2. **Le choix d'une spécialisation parmi 2** : un bâtiment majeur, ou une branche de technologies et d'unités. Ce choix est définitif pour la partie.

Conséquences :

- Chaque partie produit un build différent : 2 × 2 × 2 = 8 combinaisons par faction.
- **L'éclairage des troupes adverses gagne de la valeur** : identifier la spécialisation adverse permet de s'adapter.
- Au prototype, chaque palier ouvre son tronc commun et 2 spécialisations.
- L'IA choisit ses spécialisations selon des profils de build.

⚠️ Courbe d'XP par niveau à définir en test **[Q04]**.

### 9.6 Arbre de talents

**Décision D08 : un arbre de talents entièrement propre à chaque faction.** Les familles, la structure et le contenu sont spécifiques. Exemple indicatif pour les Légions Noires : Nécromancie / Terreur / Sacrifice.

Le héros gagne environ **8 points de talent** par partie (niveaux 2 à 9, D04). Il ne peut pas prendre toutes les améliorations : le choix crée une spécialisation.

**Garde-fous proposés**, pour que 5 arbres différents restent lisibles et équilibrables :

- même nombre de points disponibles et même présentation à l'écran pour toutes les factions ;
- chaque arbre permet au moins un build orienté **commandement** (le héros comme multiplicateur, pilier 2) et un build orienté **duel / combat personnel** ;
- chaque arbre propose au moins une voie **stratégique** : vision, économie, mobilité ou autre, selon l'identité de la faction ;
- chaque arbre reflète la mécanique signature de sa faction (§ 13).

Ancienne proposition commune, à garder comme grille de référence pour vérifier les 5 arbres :

- **Commandant :** auras, efficacité des formations, vitesse de déplacement de l'armée, moral, résistance.
- **Guerrier :** survie, dégâts, duel, capacités personnelles.
- **Stratège :** vision, reconnaissance, capacités tactiques, économie, mobilité, soutien.

Au prototype, seuls les arbres des 2 factions retenues sont conçus : Ordre de l'Aube et Légions Noires (D11).

### 9.7 Équipement *(décision D09)*

**Équipement forgé par la civilisation.** Le héros est le reflet de sa civilisation : son équipement vient de l'économie, pas du hasard.

- **3 emplacements :** arme, armure, relique.
- Chaque objet se **recherche ou s'achète** dans un bâtiment (forge, sanctuaire, bâtiment de faction…) contre des ressources, avec une condition de palier.
- **Plusieurs objets possibles par emplacement, avec un choix exclusif.** Par exemple, une lame de duel contre un étendard de commandement. Changer d'objet est possible, mais l'objet remplacé est perdu.
- **Arbitrage économique :** l'or et la pierre investis dans le héros ne vont pas dans l'armée.
- L'équipement est **conservé à la mort** (§ 9.8).
- L'IA achète son équipement selon des règles simples, liées à son profil de build.

**Reliques uniques de carte** *(hors prototype)* : quelques reliques uniques peuvent se trouver dans des ruines ou être gardées par des monstres neutres (§ 8.3, § 8.4). Elles occupent l'emplacement relique. Elles sont liées au mode Reliques (§ 14.3), mais peuvent exister en mode standard en nombre très limité, pour récompenser l'exploration sans créer d'effet boule de neige.

### 9.8 Présence, mort et résurrection

**Héros actif :** bonus de commandement et capacités disponibles.
**Héros absent ou mort :** l'armée reste fonctionnelle, mais les bonus et capacités du héros sont indisponibles. Le joueur ne doit jamais être complètement paralysé par la mort du héros.

**À la mort, sont conservés :** niveau, XP, talents, déblocages, équipement, progression de la civilisation.

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

**L'adversaire peut :**

1. accepter ;
2. refuser ;
3. se retirer avant l'engagement définitif (si le design final le permet).

**Coût du refus *(décision D16)* : une petite pénalité de moral.**

- Refuser applique *Hésitation* aux unités de celui qui refuse, autour de son héros : un malus de moral **nettement plus faible que *Démoralisé*** (§ 10.5), pendant environ 20 à 30 s.
- La hiérarchie est : **duel perdu > refus > duel gagné**. On refuse quand on pense perdre le duel, on accepte quand on pense le gagner ou quand le malus tomberait au pire moment.
- **Anti-harcèlement :** chaque héros a un temps de recharge sur ses défis (indicatif : 2 à 3 min), pour qu'on ne puisse pas cumuler les refus imposés à l'adversaire.
- **Selon la faction :** l'Ordre de l'Aube perd en plus de l'Honneur s'il refuse. D'autres factions peuvent avoir un refus moins coûteux (le Cercle de l'Ombre, pour qui la fuite fait partie de l'identité).

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

**Contenu du kit** (catégories) : attaques, défenses, mobilité, contrôle, ultime de duel. Le kit de duel est séparé du kit RTS. L'ultime de duel n'est pas lié au niveau 10 du héros (D36).

**Le duel teste :** timing, lecture de l'adversaire, gestion des temps de recharge et de la posture, connaissance du héros.

**Exemple sur les héros du prototype :** le Paladin-Commandant (riposte) gagne en posture défensive en contrant les attaques annoncées. Le Seigneur Damné (agression, drain de vie) gagne en maintenant la pression sans s'exposer aux contres.

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
  - **Légions Noires :** le corps du héros vaincu relève un *Champion damné* temporaire (~45 s), et la *Moisson* est portée immédiatement à son maximum de cumuls.
  - **Enfants du Dragon, Cercle de l'Ombre, Héritiers du Feu :** à définir avec leurs héros (Q08).
- **Garde-fous :** effets temporaires (§ 10.6) et de valeur comparable d'une faction à l'autre ; le Champion damné a une puissance plafonnée et une durée courte, pour ne pas faire boule de neige. Valeurs à régler en test.
- **Technique :** l'effet de victoire est un `GameplayEffect` (ou une capacité) référencé dans le HeroData de chaque faction, appliqué par le serveur à la fin du duel.
- **Usage visé :** le duel est un outil tactique, idéalement lancé juste avant ou pendant une bataille.
- **Duel comme protection** *(acquis avec D18)* : pendant le duel, rien d'autre ne peut cibler les héros, ce qui permet à un joueur en infériorité militaire de régler le sort des héros sans exposer son armée.

**Pistes écartées pour le moment** (à reconsidérer plus tard ou pour un mode dédié) : enjeux déclarés par celui qui défie (lourd en interface et pour l'IA), trophée personnel sur le vainqueur, basculement d'un point stratégique, récupération allongée après une mort en duel.

### 10.6 Principes de balance

Le duel doit être : risqué, lisible, court, spectaculaire, optionnel, important.

Le joueur doit toujours peser : « Est-ce que je peux me permettre de risquer mon héros maintenant ? »

Le duel ne doit pas devenir une obligation de build. Le joueur doit pouvoir gagner sans duel.

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
| Héros | capacités, commandement, récupération, duel, équipement (§ 9.7) |
| Faction | mécanique signature, unités spécialisées, technologies propres |

**Mécanisme de déblocage :** un palier ouvre des **bâtiments** (tronc commun + spécialisation, D07). Chaque **technologie** demande un palier minimum, un bâtiment et des ressources.

**Répartition *(décision D27)* : arbre commun, technologies de faction ciblées.**

- **~70 à 80 % commun à toutes les factions :** forge (dégâts et armures par catégorie d'unités), économie (collecte, rendement), infrastructures (solidité des murs, population), siège.
- **~20 à 30 % propres à la faction :** mécanique signature (Honneur, cadavres…), unités emblématiques et variantes, **spécialisations de palier** (D07), technologies héros et équipement (D09).
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
| Ordre de l'Aube | discipline / honneur / défense | forte défense, milieu de partie |
| Légions Noires | mort / corruption / recyclage des pertes | attrition, combats prolongés |
| Enfants du Dragon | adaptation / éléments / créatures | adaptation, contrôle |
| Cercle de l'Ombre | information / subversion / pièges | information, harcèlement |
| Héritiers du Feu | élite / qualité / faible population | armée réduite, puissance individuelle |

**Question de validation pour chaque faction :** « Qu'est-ce que cette faction fait que les quatre autres ne font pas ? »

Une mécanique de faction doit modifier les décisions du joueur, pas seulement ajouter des statistiques ou une ressource.

L'équilibrage ne cherche pas à rendre les factions identiques : chacune est forte à un moment différent. Les timings diffèrent, mais les opportunités de victoire restent comparables.

**Structure d'identité *(décision D31)* : une mécanique signature forte, plus une particularité économique légère.**

- **Mécanique signature :** le cœur de la faction. Elle modifie les décisions militaires et stratégiques.
- **Particularité économique :** **une seule règle**, légère, souvent portée par une variante (D20) ou un bâtiment. Ce n'est ni un système, ni une ressource (§ 5.2).

| Faction | Mécanique signature | Particularité économique *(pistes)* |
|---|---|---|
| Ordre de l'Aube | Honneur | les fermes proches d'un sanctuaire produisent plus (économie compacte et défendable) |
| Légions Noires | Cadavres / Nécroflux | Zombie : travailleur bon marché, sans nourriture, plus lent |
| Enfants du Dragon | Adaptation élémentaire | collecte bonifiée selon le terrain ou le climat |
| Cercle de l'Ombre | Subversion | pillage : voler une partie des ressources en tuant des travailleurs ennemis |
| Héritiers du Feu | Élite globale *(D39)* : toute la faction fait la même chose, en mieux | peu de travailleurs, mais chacun collecte nettement plus |

**Prototype :** Honneur + règle économique de l'Aube ; cadavres + Zombie pour les Légions.

### 13.2 Ordre de l'Aube

**Thème :** chevaliers, lumière, honneur, discipline, défense.
**Style :** solide, méthodique, défensif, efficace en combat organisé, capable de tenir des positions.

| Unité | Rôle | Traits |
|---|---|---|
| **Chevalier Vertueux** | frontline / tank | armure lourde, épée et bouclier, aura défensive, *Défi du Chevalier* (voir ci-dessous) |
| **Moine Lumineux** | soutien | soins, purification, suppression de malédictions, sceaux lumineux ralentissant les ennemis |
| **Archer de l'Aube** | distance / contrôle | arc long sacré, tirs précis, flèches de lumière, zones réduisant l'efficacité offensive ennemie |

**Défi du Chevalier *(décision D19)* : le Chevalier prend les coups à la place des autres.**

- Dans une zone autour du Chevalier, **une partie des dégâts subis par les unités alliées est redirigée vers lui**. Une version active peut rediriger **la totalité** des dégâts pendant quelques secondes.
- Effet complémentaire : les unités ennemies proches sont **provoquées** et l'attaquent en priorité.
- **Les héros ne sont pas concernés**, ni comme protégés ni comme provoqués. Le héros est déjà résistant aux troupes (D17), et le protéger en plus le rendrait quasi intuable. Le duel reste aussi lisible.
- **Synergie :** les dégâts se concentrent sur l'unité la plus blindée, que le Moine Lumineux soigne.
- **Contre-jeu :** unités perforantes ou anti-lourdes, dégâts de zone (qui frappent aussi le Chevalier, plusieurs fois), et éliminer le Moine en priorité.
- À régler en test : le pourcentage redirigé (passif partiel contre actif total), le rayon de la zone et le temps de recharge.

**Mécanique à prototyper : HONNEUR.** Récompense les actions cohérentes avec l'identité de la faction : duels, défense, protection d'alliés, objectifs militaires. Ce n'est pas encore une ressource obligatoire.

**Héros *(décision D12)* : LE PALADIN-COMMANDANT** *(nom temporaire)*

| Combat personnel | Commandement | Valeur stratégique |
|---|---|---|
| ★★☆ | ★★★ | ★★☆ |

- **Rôle :** commandant défensif, multiplicateur d'une armée qui tient ses positions.
- **Kit RTS (pistes) :**
  - *Aura de l'Aube* : large aura défensive (armure, moral).
  - *Bannière de l'Aube* : plante un point de commandement fixe qui prolonge son aura dans une zone pendant qu'il se déplace ailleurs. Cela donne une réponse partielle au problème des fronts multiples.
  - *Serrez les rangs* : cri de guerre qui réduit les dégâts reçus et met les unités en formation défensive.
- **Kit de duel :** style **défensif à riposte**. Blocages, contres et punition des erreurs de l'adversaire. Un duelliste patient, fidèle à la discipline de la faction.
- **Lien avec l'Honneur :** gagne de l'Honneur en *acceptant* les duels et en tenant des positions sous pression.
- **Victoire en duel *(D35)* :** gros gain d'Honneur et recharge immédiate de la *Bannière de l'Aube*.
- **Capacité ultime (niveau 10) *(D36)* : *Dernier Rempart*.** Pendant ~10 s, les alliés dans une large zone autour du héros ne peuvent pas descendre sous 1 PV ; à la fin, ils récupèrent une partie des dégâts subis pendant l'effet. Contre : reculer et attendre la fin au lieu de frapper. Valeurs à régler en test.

### 13.3 Légions Noires

**Thème :** morts-vivants, nécromancie, sacrifice, terreur, corruption.
**Style :** attrition, affaiblissement, recyclage des pertes, pression persistante.

| Unité | Rôle | Traits |
|---|---|---|
| **Guerrier Damné** | frontline / berserker | massue ou hache, dégâts élevés, rage nécrotique, se consume au combat |
| **Nécromancien** | soutien / invocation | squelettes, goules, malédictions, drain de vie |
| **Spectre Assassin** | furtivité / DPS | dagues spectrales, dématérialisation, marquage de cibles, mobilité |

**Mécanique à prototyper : CADAVRES / NÉCROFLUX.** Les cadavres servent à créer des serviteurs, renforcer des unités, alimenter des capacités, corrompre une zone. Pas nécessairement une ressource économique traditionnelle.

**Héros *(décision D13)* : LE SEIGNEUR DAMNÉ** *(nom temporaire)*

| Combat personnel | Commandement | Valeur stratégique |
|---|---|---|
| ★★★ | ★★☆ | ★☆☆ |

- **Rôle :** combattant de première ligne qui se nourrit de l'attrition et pousse son armée à l'offensive.
- **Kit RTS (pistes) :**
  - *Aura de terreur* : réduit le moral et l'efficacité des ennemis proches.
  - *Moisson* : se renforce à chaque mort autour de lui, alliée ou ennemie (cumuls temporaires).
  - *Sacrifice* : consomme une unité alliée ou un cadavre pour se soigner, ou fait exploser un cadavre en dégâts de zone.
- **Kit de duel :** style **agressif à drain de vie**. Pression constante, soin en frappant, mais exposé aux ripostes.
- **Contraste avec le Paladin-Commandant :** l'agresseur contre le riposteur. Chaque duel entre les deux factions du prototype repose sur la lecture de l'adversaire : frapper ou laisser venir.
- **Lien avec les cadavres :** il est le premier consommateur de la mécanique de faction.
- **Pas de poudre *(D38)* :** les Légions n'ont accès ni à l'Arquebusier ni au Canon. ⚠️ Réponse équivalente de fin de partie à définir.
- **Victoire en duel *(D35)* :** le corps du héros vaincu relève un *Champion damné* temporaire (~45 s, puissance plafonnée) et la *Moisson* est portée à son maximum.
- **Capacité ultime (niveau 10) *(D36)* : *Marée des damnés*.** Les unités mortes récemment dans une large zone, alliées comme ennemies, se relèvent en serviteurs temporaires (~30 s, plafond ~15 à 20, hors population). Contre : éviter de se battre sur un charnier, s'éloigner le temps de l'effet. Valeurs à régler en test.

### 13.4 Enfants du Dragon

**Thème :** dragons, élémentalisme, créatures, adaptation.
**Style :** polyvalence, contrôle du terrain, choix d'élément, interactions avec les créatures.

| Unité | Rôle | Traits |
|---|---|---|
| **Champion Draconique** | frontline / dégâts | épée à deux mains, feu ou foudre, attaque en cône, forte présence au corps-à-corps |
| **Mage Élémentaire** | dégâts / contrôle à distance | choix feu, glace ou foudre, zones élémentaires, effets selon l'élément |
| **Dompteur de Bêtes** | hybride / soutien | arme courte, compagnon contrôlable (wyverne, dracogriffe), ordres attaquer / distraire / protéger |

**Mécanique à prototyper : ADAPTATION ÉLÉMENTAIRE.** La faction modifie son style selon l'élément choisi, le terrain, le climat et les bâtiments construits.

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
  - *Aura draconique* : aura de commandement ; l'armée proche prend l'élément du héros (lien avec l'Adaptation élémentaire).
  - *Lame draconique* : frappe de corps à corps de l'élément du héros.
- **Duel :** **toujours à pied.** Le dragon se pose ou s'éloigne. ⚠️ À préciser : défi lancé ou reçu en vol (accepter fait-il atterrir automatiquement ?), style du kit de duel.
- **Capacité ultime (niveau 10) *(D36)* : *Appel de la Couvée*.** 2 à 3 dragons adultes descendent sur une zone annoncée pendant ~20 s (plafonnés, hors population).
- ⚠️ **Points de vigilance :** le vol au-dessus des murs (proposition : le dragon ne franchit pas les murs, ou les tours lui infligent des dégâts normaux) ; une couche de déplacement aérien pour un seul acteur ; lisibilité (le dragon ne doit pas masquer le champ de bataille).
- **Victoire en duel (D35) :** à définir.

### 13.5 Cercle de l'Ombre

**Thème :** assassins, espionnage, manipulation, ruse.
**Style :** information, harcèlement, pièges, attaques opportunistes, faible efficacité en combat frontal prolongé.

| Unité | Rôle | Traits |
|---|---|---|
| **Maître des Ombres** | assassin | camouflage, invisibilité temporaire, attaques éclair, forte mobilité |
| **Piégeur** | contrôle / soutien | arbalète, mines, filets, poison, pièges tactiques |
| **Illusionniste** | contrôle / confusion | copies illusoires, perturbation des ordres, confusion, fuite ou retournement temporaire |

**Mécanique à prototyper : SUBVERSION.** Sabotage, fausses informations, perturbation des ordres, contrôle temporaire, vision avancée.

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

- Réservée au **héros du Cercle de l'Ombre**, sous forme de compétence ou de capacité ultime (à fixer avec ce héros, Q08).
- **Esquivable :** la conversion vise une zone annoncée et ne prend effet qu'après un délai de canalisation. Les unités qui sortent de la zone avant la fin y échappent ; interrompre le héros l'annule.
- Mêmes exclusions que le retournement *(proposition, à confirmer)* : jamais de héros, siège, bâtiment ni unité d'élite ; nombre d'unités et coût plafonnés ; longue recharge.

**Technique :** tags GAS de contrôle (`State.CC.Fear`, `State.CC.Confused`, `State.CC.Charmed`…) et effet d'immunité temporaire appliqué à la fin de chaque perte de contrôle ; la conversion définitive change le propriétaire de l'unité côté serveur. Valeurs à régler en test.

### 13.6 Héritiers du Feu

**Thème :** élite, puissance, qualité plutôt que quantité. Leur feu est celui de la **forge et de la flamme sacrée** (artisanat, qualité des armes, héritage), pas un élément magique : c'est ce qui les distingue des Enfants du Dragon *(D38)*.
**Style :** très peu d'unités, chaque unité est précieuse, recrutement lent, forte dépendance à la micro et au positionnement, économie exigeante.

**Règle de prototype** (direction, pas formule définitive) : coût ≈ ×2, population ≈ ×2, efficacité ≈ ×2, recrutement plus long.

**Principes de balance :** équilibrer autour de la population, de l'économie, du temps de production, des pertes, de la mobilité et de la qualité des unités. Une unité 2× plus efficace n'est pas 2× meilleure partout : elle peut être 2× meilleure en combat frontal, mais moins nombreuse, plus lente à produire, plus chère, vulnérable au contrôle, incapable de couvrir plusieurs fronts.

**Faiblesse principale :** « Je ne peux pas être partout. »

**Signature *(décision D39)* : l'élite globale, sans mécanique supplémentaire.** Les Héritiers font tout simplement la même chose que les autres, mais en mieux : ils collectent plus vite, tirent plus vite, frappent plus fort. Ce ne sont que des statistiques, mais **toute la faction** est construite ainsi (travailleurs, socle, emblématiques, siège).

- **Pas de Ferveur, pas de Prestige, pas de vétérance.** L'identité est la plus simple à lire des 5 factions, et la plus facile à prendre en main.
- **Les décisions viennent de la rareté :** peu d'unités, chaque perte coûte cher, impossible de couvrir plusieurs fronts. C'est une exception assumée à la règle de D31 (une signature qui modifie les décisions) : la règle d'élite les modifie indirectement.
- Le ratio ≈ ×2 est une direction ; il peut varier selon les statistiques (collecte, cadence, dégâts) et sera réglé en test.

**Unités emblématiques *(décision D38)*** (noms et traits provisoires) :

| Unité | Rôle | Traits |
|---|---|---|
| **Gardien de la Forge** | frontline lourde | armure et bouclier massifs, capable de tenir seul une ligne ; répond à « je ne peux pas être partout » |
| **Lame Ardente** | DPS mobile | charge, arme incandescente qui inflige de la brûlure, exigeante en micro |
| **Prêtre de la Flamme** | soutien | bénit les armes, confère de la résistance, protège des unités précieuses ; pas de magie élémentaire |

**Poudre (D38) :** commune à toutes les factions au palier 3 (§ 7.1). Les Héritiers en ont leurs propres versions, forgées : la *Bombarde* à la place du Canon et une variante de l'Arquebusier (nom et spécificité à définir). Ce sont leurs deux variantes du socle (D20).

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
- **Chaque emplacement peut être tenu par un humain ou une IA.** Une IA peut prendre le relais d'un joueur qui se déconnecte *(interprétation à confirmer)*.

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

- **Le serveur fait autorité.** Il peut être hébergé par un joueur, puis devenir un serveur dédié plus tard. L'IA tourne sur le serveur, ce qui facilite les emplacements IA et la reprise d'un joueur déconnecté.
- **Unités ordinaires = entités légères**, pas des `ACharacter` :
  - simulées sur le serveur (piste : **Mass Entity** d'Unreal, ou un gestionnaire maison) ;
  - répliquées sous forme compacte (positions et états compressés), interpolées côté client ;
  - affichées en **instances** avec des animations optimisées (animation par textures de sommets, ou équivalent).
- **Héros, bâtiments et engins de siège = acteurs classiques avec GAS.** Ils sont peu nombreux : GAS reste utilisé là où il compte (capacités, duels, auras, équipement).
- **Le brouillard de guerre est appliqué par le serveur**, qui n'envoie à chaque client que ce qu'il voit. Cela protège contre la triche « maphack » et réduit la bande passante.
- **Cible de performance :** ~1 400 unités (8 × 175, D03).

**Conséquence immédiate sur le code :** l'implémentation actuelle (`AUnitBase : ACharacter` avec un `AAIController` par unité) est à remplacer par le système d'unités légères. Les ordres de déplacement, la sélection et le combat de base sont à porter dessus.

L'IA joue avec les mêmes règles et les mêmes informations qu'un joueur (brouillard de guerre compris), sauf dans les niveaux de difficulté explicitement « tricheurs ».

### 16.5 Navigation sur plusieurs niveaux

Les remparts praticables (D24) imposent une navigation à deux niveaux : le sol et le chemin de ronde des murs de pierre.

- Les segments de mur portent une **surface de navigation** sur leur sommet, reliée au sol par des liens de navigation (escaliers dans les tours et les portes).
- Cette navigation est **mise à jour dynamiquement** à la construction et à la destruction de chaque segment.
- Elle doit fonctionner avec les **unités légères** (D29), pas seulement avec les `ACharacter` d'Unreal.
- C'est un chantier technique prioritaire du prototype, à valider tôt : performance avec ~1 400 unités et des murs étendus.

---

## 17. Prototype minimum viable

Avant de créer les cinq factions complètes :

| Élément | Quantité |
|---|---|
| Factions | 2 |
| Héros | 1 par faction |
| Unités | *proposition* : Paysan, Lancier, Homme d'armes, Archer, Arbalétrier, Cavalier léger + 1 unité emblématique par faction (Cavalier lourd et siège hors prototype, sauf besoin) |
| Ressources | 4 |
| Centre principal | 1 |
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
11. Cinq factions.
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
- ~~**Q07** — Paliers, talents, équipement~~ → **tranchée (D07, D08, D09)**.
- **Q08** — ~~Nombre de héros par faction~~ (D10 : un seul) ; ~~héros du prototype~~ (D12, D13) ; ~~capacités ultimes~~ (D36) ; ~~héros des Enfants du Dragon~~ (D41) ; reste : héros du Cercle de l'Ombre et des Héritiers du Feu, avec leurs ultimes, et les effets de victoire en duel des 3 factions hors prototype.

**Armée et base**
- ~~**Q09** — Socle et matrice de contres~~ → **tranchée (D20, D21)**. À confirmer : l'Arbalétrier comme contre-mesure anti-héros du socle.
- ~~**Q10** — Modèle de construction~~ → **tranchée (D22)**.
- ~~**Q11** — Place du siège~~ → **tranchée (D23, D24)**.

**Commandement et mort**
- ~~**Q12** — Force du commandement~~ → **tranchée (D26)**.
- ~~**Q13** — Récupération et résurrection~~ → **tranchée (D25)**. Valeurs à régler en test.

**Duel**
- ~~**Q14** — Contrôle du duel~~ → **tranchée (D14)**.
- ~~**Q15** — Coût d'un refus de duel~~ → **tranchée (D16)**.
- ~~**Q16** — Bénéfice propre au duel~~ → **tranchée (D15, D17, D35)**. Restent : effets de victoire des 3 autres factions (avec leurs héros, Q08).
- ~~**Q17** — Zone compatible et interventions~~ → **tranchée (D18)**.
- ~~**Q18** — Défi du Chevalier Vertueux~~ → **tranchée (D19)**.

**Technologies et événements**
- ~~**Q19** — Déblocage et répartition des technologies~~ → **tranchée (D07, D27)**.
- ~~**Q20** — Déclenchement des événements~~ → **tranchée (D28)**.

**Factions**
- ~~**Q21** — Identité mécanique et économique~~ → **tranchée (D31)**. Les règles économiques précises restent des pistes.
- ~~**Q22** — Limites des effets de perte de contrôle (Cercle de l'Ombre)~~ → **tranchée (D37)**. Reste : conversion définitive en compétence ou en ultime, à fixer avec le héros du Cercle (Q08).
- ~~**Q23** — Unités des Héritiers du Feu ; Ferveur nécessaire ?~~ → **tranchée (D38, D39)**.
- ~~**Q24** — Factions du prototype~~ → **tranchée (D11)**.

**Carte**
- ~~**Q25** — Terrain et combat~~ → **tranchée (D23, D33, D40)**.
- ~~**Q26** — Monstres neutres~~ → **tranchée (D32)**.

**Technique et contrôle**
- ~~**Q27** — Contrôles et raccourcis~~ → **tranchée (D30)**.
- ~~**Q28** — Niveau du héros dans GAS~~ → **tranchée (D34)**.
- ~~**Q29** — Modèle réseau~~ → **tranchée (D29)**.
- ~~**Q30** — Résistance du héros face aux unités~~ → **tranchée (D17)**. Reste à définir : les contre-mesures « tueuses de héros » de chaque faction.

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
| D09 | 2026-10-05 | Q07 (partie 3) — Équipement | Forgé par la civilisation : 3 emplacements (arme, armure, relique), objets achetés en bâtiment, choix exclusifs, conservé à la mort. Reliques uniques de carte hors prototype. | § 8.3, § 9.7, § 11 |
| D10 | 2026-10-05 | Q08 (partie 1) — Héros par faction | Un héros unique par faction ; extensible plus tard via HeroData. | § 9 |
| D11 | 2026-10-05 | Q24 — Factions du prototype | Ordre de l'Aube et Légions Noires. | § 9.6, § 17 |
| D12 | 2026-10-05 | Q08 (partie 2) — Héros de l'Ordre de l'Aube | Paladin-Commandant : commandement ★★★ ; Aura, Bannière, Serrez les rangs ; duel défensif à riposte ; Honneur gagné en acceptant les duels. | § 13.2 |
| D13 | 2026-10-05 | Q08 (partie 3) — Héros des Légions Noires | Seigneur Damné : combat ★★★ ; Aura de terreur, Moisson, Sacrifice ; duel agressif à drain de vie. | § 9, § 13.3 |
| D14 | 2026-10-05 | Q14 — Contrôle du duel | Semi-automatique tactique : attaque automatique, posture + 4 à 6 capacités, attaques fortes annoncées. Pendant un duel, capacités RTS remplacées par le kit de duel ; aura maintenue. | § 10.3 |
| D15 | 2026-10-05 | Q16 — Récompense du duel *(provisoire)* | Base : basculement de moral (Triomphe / Démoralisé, 30-45 s) + XP de duel. **À retravailler** : jugé insuffisant. | § 10.5 |
| D16 | 2026-10-05 | Q15 — Coût du refus | *Hésitation* (petit malus de moral, 20-30 s) ; temps de recharge des défis 2-3 min ; modulé par faction (Aube perd de l'Honneur). | § 10.1 |
| D17 | 2026-10-05 | Q30 — Résistance héroïque | Héros ≈ 5-6 unités standard en puissance offensive (valeur absolue, toutes factions) ; dégâts reçus des troupes fortement réduits (~×0,3) ; dégâts normaux des héros, tours, siège et contre-mesures dédiées ; hors duel, héros contre héros = usure, les compétences de duel font la différence. | § 9.1 bis, § 18 |
| D18 | 2026-10-05 | Q17 — Autour du duel | Duel protégé : seuls les deux duellistes peuvent se toucher ; cercle de duel ; un seul duel par héros, pas de 2 contre 1 ; zone : courte distance, visibles, hors rayon d'un centre principal ; temps écoulé = pas de vainqueur. | § 10.2, § 10.4, § 10.5, § 14.4 |
| D19 | 2026-10-05 | Q18 — Défi du Chevalier Vertueux | Redirection vers le Chevalier d'une partie des dégâts subis par les alliés dans une zone (totalité en version active, quelques secondes) + provocation des ennemis proches. Héros exclus. Pas de duel unité contre héros. | § 13.2 |
| D20 | 2026-10-05 | Q09 (partie 1) — Socle d'unités | Socle commun à toutes les factions (Paysan, Homme d'armes, Piquier, Archer, Cavalier, siège, anti-héros). Variantes rares : 1 faction (2 max) par rôle, 1-2 variantes max par faction (ex. Zombie des Légions, Hallebardier). Emblématiques en plus du socle. Héritiers : règle d'élite sur tout le socle. | § 7.1 |
| D21 | 2026-10-05 | Q09 (partie 2) — Matrice de contres | Modèle AoE4 : bonus par catégorie d'armure. Socle étendu : Lancier, Homme d'armes, Archer, Arbalétrier, Cavalier léger, Cavalier lourd. Arbalétrier proposé comme anti-héros. | § 7.1, § 7.2, § 17 |
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
| D36 | 2026-10-05 | Q08 (partie 4) — Capacités ultimes | Capacité de bataille active du kit RTS : zone, courte durée, recharge ~3-4 min, annoncée et contrable ; pas de transformation ni d'effet global. Paladin : *Dernier Rempart* (~10 s, alliés pas sous 1 PV, puis soin partiel). Seigneur : *Marée des damnés* (morts récents relevés, ~30 s, plafond ~15-20, hors population). Ultime de duel séparé, non lié au niveau 10. Hors prototype. | § 9, § 9.5, § 10.3, § 13.2, § 13.3 |
| D37 | 2026-10-05 | Q22 — Perte de contrôle (Cercle de l'Ombre) | Effets courts et encadrés : peur 2-4 s ; confusion 3-5 s (remplace la perturbation des ordres, jamais d'action sur les ordres ni l'interface adverses) ; retournement temporaire d'unités ordinaires, ~8-10 s, 1-3 unités, plafond de coût ; immunité ~10-15 s après chaque effet ; héros jamais pris ; fausses informations via des objets du monde uniquement. **Plus** une conversion définitive façon AoE, réservée au héros du Cercle (compétence ou ultime), esquivable en sortant de la zone avant la fin de la canalisation. | § 13.5 |
| D38 | 2026-10-05 | Q23 (partie 1) — Unités des Héritiers du Feu | Thème : feu de la forge et flamme sacrée (pas élémentaire). Emblématiques : Gardien de la Forge, Lame Ardente, Prêtre de la Flamme. **Poudre commune à toutes les factions, sauf contre-ordre** : la technologie commune « Armes à poudre » (palier 3) débloque l'Arquebusier et le Canon. Exceptions : Légions Noires sans poudre (réponse équivalente à définir) ; Héritiers avec leurs variantes (Bombarde à la place du Canon, variante de l'Arquebusier). **Règle générale :** une unité n'est jamais remplacée en cours de partie ; les améliorations ne touchent que les statistiques et le visuel ; la nouveauté passe par le déblocage de nouvelles unités. | § 6.1, § 7.1, § 11, § 13.3, § 13.6 |
| D39 | 2026-10-05 | Q23 (partie 2) — Ferveur / Prestige | Pas de mécanique supplémentaire : la signature des Héritiers est l'élite globale. Toute la faction fait la même chose en mieux (collecte, cadence, dégâts) ; ce ne sont que des statistiques, appliquées à tout. Les décisions viennent de la rareté des unités (exception assumée à D31). | § 13.1, § 13.6 |
| D40 | 2026-10-05 | Q25 (suite) — Zones alignées | Cartes compétitives neutres : aucune zone sacrée ou corrompue posée par la carte ; ces zones ne sont créées que par des capacités de faction (temporaires, visibles, contrables). Zones de carte possibles hors compétitif et en scénario. Pas d'affinité de terrain par faction. | § 8.1 |
| D41 | 2026-10-05 | Q08 (partie 5) — Héros des Enfants du Dragon | **Seigneur-Dragon**, façon Roi-Sorcier de BFME : bascule monté / à pied. Monté : vol, *Souffle*, *Cri du wyrm*, *Piqué* annoncé par l'ombre ; pas d'aura, dégâts normaux des tireurs et des tours. À pied : aura élémentaire, *Lame draconique*, duel (toujours à pied). Le dragon grandit avec les niveaux. Ultime : *Appel de la Couvée* (2-3 dragons, ~20 s). | § 9, § 13.4 |

---

## 22. Reprise de la review

*Section de travail : elle indique où en est la review question par question du GDD. À mettre à jour à chaque séance.*

**Méthode :** une question à la fois, 2 à 4 options (A/B/C) avec leurs conséquences et une recommandation. Chaque réponse est consignée dans le journal (§ 21, numéro D suivant : **D42**), reportée dans le corps du document, et le ⚠️ correspondant est retiré de la liste du § 20.

**Question en cours — Déblocage des capacités RTS des héros, façon BFME** (pas encore posée ; à poser en premier, car elle structure les kits des deux héros restants). Concerne tous les héros, y compris le Paladin et le Seigneur Damné :

- **A.** *(recommandée)* **Déblocage progressif, façon BFME**, aligné sur les paliers : aura (et une première capacité) au niveau 1, une capacité de plus au niveau 3, puis au niveau 6, ultime au niveau 10. Le prototype (niveaux 1 à 6) a donc tout le kit RTS sauf l'ultime. Progression ressentie forte ; début de partie plus simple à lire.
- **B.** **Kit RTS complet dès le niveau 1** ; la progression passe par les talents, les statistiques et l'ultime. Plus simple, mais moins de moments « nouvelle capacité ».
- **C.** **Déblocage par choix** : à chaque palier, le joueur choisit une capacité parmi deux (lien avec la spécialisation de palier, D07). Plus de variété, mais plus de contenu à produire et à équilibrer.
- Sous-question à traiter avec : le **kit de duel** est-il complet dès le niveau 1 (duels équitables quel que soit le niveau) ou suit-il le même déblocage ?

**Prochaines questions, dans l'ordre :**

1. Q08 (suite) : héros du Cercle de l'Ombre (dont la conversion définitive, D37), puis des Héritiers du Feu ; pour chacun, ultime et effet de victoire en duel (D35, D36). S'inspirer des kits de BFME.
2. D41 (suite) : effet de victoire en duel du Seigneur-Dragon ; défi en vol ; style de son kit de duel.
3. D38 (suite) : réponse de fin de partie des Légions Noires sans poudre ; place de l'Arquebusier dans la matrice de contres ; variante d'Arquebusier des Héritiers.

**À régler en test plutôt qu'en discussion :** courbe d'XP (Q04), valeurs de récupération et de prix de résurrection (D25), pourcentages d'aura (D26), valeurs de terrain (D33).

**Premier chantier de code identifié :** remplacer `AUnitBase : ACharacter` par le système d'unités légères (D29, § 16.4).
