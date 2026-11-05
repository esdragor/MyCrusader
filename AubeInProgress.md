# Ordre de l'Aube : la première faction du prototype

Ouvert le 2026-11-04 (Claude, session principale, à la demande de l'utilisateur : « un InProgress.md par feature, lié dans EtapeDefini, un seul dossier à la fois, fini avant d'ouvrir le suivant »). Deuxième dossier après `LegionsInProgress.md`. **Rien n'est codé, rien n'est commité, aucun worktree créé.** Ce journal est **autosuffisant** : une session qui reçoit « continue AubeInProgress » dans un worktree donné par l'utilisateur doit pouvoir démarrer à A1 sans rien relire d'autre que ce qui est cité ici.

**Pourquoi** : D11 fait de l'Aube la première faction du prototype ; le test prioritaire n° 10 (§ 17.1, « chaque faction offre-t-elle une façon réellement différente de jouer ? ») demande l'Honneur et le Chevalier Vertueux face aux Légions ; l'Honneur teste aussi les règles du duel (§ 17, D11). Aujourd'hui **le jeu n'a aucune mécanique propre à l'Aube** : l'Aube et les Légions jouent avec les mêmes unités et les mêmes bâtiments.

**Statut : PRÊT À LANCER, en attente de L0 (dossier des Légions) et d'un worktree** (l'utilisateur le prépare). **Toutes les questions de conception sont tranchées (AQ-1 à AQ-14, D685 à D690).** Sections : décisions prises, état des lieux, règles du projet, étapes A1 à A14 (test rouge, fin), liens avec les autres chantiers, après le prototype, comment lancer, journal.

## Décisions de l'utilisateur (2026-11-04), numérotées au GDD § 21 : D685 à D690

- **AQ-1 (D685)** : le **socle multi-faction reste L0 des Légions** (rôles, surcharges, colonne `Faction`, grille filtrée). L'Aube l'attend ; **ce dossier ne démarre qu'après L0 fusionné** dans `main`.
- **AQ-2 (D685)** : le **Hallebardier est un vrai type `Halberdier`** (ligne propre de `UnitTypes.csv`, faction Aube) qui tient le **rôle « Lancier »** de l'Aube dans `Factions.csv` ; la Caserne de l'Aube le produit à la place du Lancier. Ce dossier **ajoute à la table des rôles** les rôles « Lancier » et « Moine » (L0 n'a que Travailleur, Héros, Centre).
- **Ordre (D685)** : unités (A1 à A3), jalon « Aube contre Légions jouable », Honneur (A4 à A5), pouvoirs (A6), *Défi* actif (A7), spécialisations (A8 à A10), technologies (A11), IA (A12), visuel (A13), clôture (A14).
- **AQ-3 (D686)** : **aucun mur, porte ni tour n'existe** : ***Rempart béni*** (palier 1) et ***Bastion de l'Aube*** attendent le dossier des fortifications ; *Bastion* reste grisé « Bientôt » (D601) ; **le palier 1 n'a pas de pouvoir d'Honneur d'ici là** ; la spécialisation jouable du palier 1 est la *Chapelle de campagne*.
- **AQ-4 (D687)** : **Moine Lumineux = type propre `LuminousMonk`**, même soin que le Moine commun, produit de l'Honneur en soignant, plus la capacité active ***Sceau de lumière*** (ralentit les ennemis dans une zone) par le cadre de capacités (L6a des Légions).
- **AQ-5 (D687)** : **Chevalier Vertueux** : Caserne, palier 1, catégories **infanterie lourde + Lourd** (D428).
- **AQ-6 et AQ-12 (D687)** : ***Défi du Chevalier* en deux étapes** (passif de redirection, puis bouton actif avec provocation) ; **l'« aura défensive » du GDD est ce passif** (rayon visible au sol, pas de bonus d'armure en plus).
- **AQ-7 (D688)** : Honneur à **plafond fixe**, **plancher 0** (pas de dette).
- **AQ-8 et AQ-11 (D688)** : **les cinq sources de D76** (défendre, soins, duel, *Défi* absorbé, tenir un point), **une étape chacune** ; **point stratégique = relais neutres seulement**.
- **AQ-9 (D689)** : ***Lumière sacrée*** = soin continu ~5 s sur une zone visée ; le retrait de moral et l'immunité (D404) attendent le système de moral.
- **AQ-10 (D689)** : ***Renforts de l'Aube*** : arrivée annoncée ~5 s ; **refus total** (sans coût) si la population ne laisse pas toute l'escouade.
- **Recharge (D689)** : coût en Honneur **et** courte recharge propre à chaque pouvoir.
- **AQ-13 (D690)** : **le mécanisme du choix de palier n'existe pas** : la première session qui en a besoin (Aube A8 ou L9 des Légions) le construit, l'autre le reprend.
- **AQ-14 (D690)** : ***Chapelle de campagne*** : plusieurs exemplaires, hors rayon anti-rush, bonus de ferme de 30 m **sans cumul** avec le Sanctuaire.
- **IA (D688)** : règle simple de dépense par pouvoir, réglée par niveau.

## Références au GDD

- **§ 13.2** : unités, *Défi du Chevalier* (D19), économie du Sanctuaire (D67), spécialisations des paliers 1 à 3 (D123, D124, D182, D183), Honneur (D75, D76), pouvoirs (D85, D213, D404), héros Paladin-Commandant (D12, D35, D36), arbre des trois Serments (D106 à D111, D195), **§ 7.1** (socle, Hallebardier D79, Moine D134, Chapelle D211, D212), **§ 11.1** (grille des paliers, technologies propres de l'Aube, D583 où elles se cherchent, D316 et D393 pour le « Profane »), **§ 17** (prototype : Chevalier Vertueux + Moine Lumineux, D82 ; paliers 0 à 2).
- **D601** (une technologie sans effet est grisée « Bientôt »), **D602** (Sanctuaire : 250 bois, 3 × 3 cases, 30 s, 1 500 PV, rayon 30 m, +15 % de nourriture), **D603** (Chapelle avec le Moine), **D345** (stats choisies une par une), **D428** (catégorie Lourd), **D683 / D684** (technologie ou bâtiment ; choix de 30 / 45 / 60 s annulable), **D22** (pas de construction près d'un centre ennemi), **D190** (plafond de population).
- **Hors prototype** (palier 3, § 11.2) : *Autel du Jugement*, *Crypte des saints*, *Jugement*, *Veille des saints*, *Archer de l'Aube*, niveau 7 et au-delà du héros (*Charge de l'Aube*). **Ne pas les faire ici.**

## État des lieux (vérifié dans le dépôt le 2026-11-04, HEAD 755c6d4)

**Ce qui existe déjà**
- `EHOLFaction::DawnOrder` (`Match/HOLMatchConfig.h:23-31`) : choix du lobby, nom affiché, `Script/Talents/HOLAITalent.as:123-147` (`AnswerFamily`, famille de talent par faction pour l'IA). **Aucune règle de jeu ne lit la faction** : l'Aube et les Légions ont la même grille, les mêmes unités, le même Sanctuaire.
- **Sanctuaire** : ligne `Sanctuary` dans `BuildingTypes.csv` (bois 250, 30 s, 1 500 PV, 1 200 de côté, palier 0, `Produces` vide, file de 1 pour les recherches) ; effet des fermes dans `Simulation/HOLGatherRules.h:153-156` (`SanctuaryRadius = 3000` cm, `SanctuaryFarmPercent = 15`, sans cumul), crochet `HOLGatherProcessor.cpp:322-323` et `:602-617`, test `Tests/HOLGatherTests.cpp:1103` ; **non vu à l'écran** (aucun scénario) ; **pas limité à l'Aube**.
- **Chapelle** (page II, case S, palier 1, 200 bois, produit `Monk`) et **Moine** (80 PV, 100 nourriture + 100 or, soin 5 PV/s à 8 m, un allié à la fois, jamais lui-même : `Simulation/HOLHealRules.*`, `HOLHealProcessor.cpp`, ordre au clic droit `HOLPlayerController.cpp:2706`) : validés par l'utilisateur (E-monk, 2026-11-02).
- **Technologies de l'Aube** dans `Technologies.csv` (colonne `Faction` = `Dawn`) : *Liturgie* (Sanctuaire, palier 1, `HonourGain` +10, **`Implemented=False`**), *Hallebardes trempées* (Caserne, palier 1, **False**), *Bénédiction des armes* (Sanctuaire, palier 2, **False**), *Vœu de pauvreté* (Chapelle, palier 2, Moine −20 %, **True** et validé). La colonne `Faction` n'est lue que par les tests de table : **ces technologies sont aujourd'hui offertes à tous**.
- **Héros** : la ligne `Hero` (« Paladin-Commander », 450 PV, attaque 30, cadence 1,25 s, vitesse 697) ; capacités *Aura de l'Aube*, *Serrez les rangs*, *Bannière de l'Aube* codées (`Simulation/HOLHeroAbilityRules.*`, `HOLCombatProcessor.cpp:638-730`) ; deux talents à effet seulement (*Boucliers levés*, *Pas de l'Aube*) ; reste à `TalentsInProgress.md`.
- **Duel** : règles en script (`Script/Duel/HOLDuelRules.as`) ; `bRefusalCost` (ligne 116, posé à la ligne 284 sur Refusé / Ignoré, **pour toutes les factions**, **aucun lecteur**) et `bFactionEffect` (ligne 292, sur une victoire) ; seul consommateur : le journal `HOLDuelLedger.as:194-199`. L'étape D7 de `DuelInProgress.md` (récompenses branchées) **n'est pas faite**. `Script/AI/HOLAIDuel.as` a un booléen `bDawn` (pression d'acceptation +6) que **l'IA ne pose jamais** (`HOLAIBrain.as:441-460`).
- Textes d'interface : « Gain d'Honneur » (`HOLGameNames.cpp:374-376`), descriptions de talents (Rempart béni, Lumière sacrée, Renforts) dans `Talents.csv` et `HOLTalentNames.as`.
- Relais neutres (D647, D658, `RelaysInProgress.md`) : base de la source « tenir un point ».

**Ce qui n'existe pas**
- **L'Honneur** : aucune valeur par joueur, aucune source, aucune dépense, aucune barre ; aucun des pouvoirs (*Lumière sacrée*, *Rempart béni*, *Renforts de l'Aube*, *Jugement*, *Veille des saints*).
- **Chevalier Vertueux, Hallebardier, Moine Lumineux** : aucune ligne dans `UnitTypes.csv` (les 13 lignes : LightInfantry, LightInfantryNanite, Archer, Peasant, Spearman, ManAtArms, King, Hero, Crossbowman, Monk, Ram, Mangonel, LightCavalry). La Caserne produit `Spearman`, `ManAtArms`.
- ***Défi du Chevalier*** : ni redirection de dégâts, ni provocation (« Challenge » dans le code désigne le défi de duel de héros).
- **Murs, portes, tours, Forge, Académie** : aucune ligne dans `BuildingTypes.csv` (13 bâtiments : MainCentre, House, LumberCamp, MiningCamp, Mill, Barracks, Farm, Sanctuary, ArcheryRange, Market, SiegeWorkshop, Chapel, Stable).
- **Le mécanisme du choix de palier** (D07, D684), les quatre spécialisations de l'Aube (*Bastion*, *Chapelle de campagne*, *Lices*, *Monastère*), la catégorie « Profane » (L3 des Légions), le système général de moral, le cadre de capacités d'unités ordinaires (L6a des Légions).
- **HUD de faction** : aucun emplacement (GDD § 15 : « un emplacement de faction au-dessus du bloc du héros »).

## Règles du projet à suivre pendant ce chantier

- **Test rouge d'abord**, sur la vraie chaîne (pas de valeurs mises à la main) ; chaque cas de la demande testé et répété avant l'essai ; boucle jusqu'au vert.
- **EtapeDefini.md** (chemin absolu `C:\Users\baudo\Documents\MyCrusader\EtapeDefini.md`) mis à jour à chaque avancée, en même temps que ce journal ; relire juste avant d'éditer.
- Boucles de 15 min ; aucun sous-agent au-delà de 300 000 jetons ; compiler par `Tools\build.ps1` (file d'attente automatique), `-LiveCoding=0` ; moteur HeroesOfLiberty `C:\UE\UE_5.8_HOL` ; ne pas écraser AS_3 (ProjectShiva).
- Commits **en anglais**, sans ligne `Co-Authored-By` ; **rien committé tant que l'utilisateur n'a pas validé que tout marche** ; jamais de push sans accord.
- Tests : `Tools\Tests\quick_check.ps1` (< 1 min par défaut, `-Parts` pour les parties touchées, `-Full` seulement à une clôture) ; essais **rigoureux** (`preflight.ps1`, « VU À L'ÉCRAN » contre « LOGIQUE SEULEMENT », 3 min au plus, 12 règles de `Auto_Journey.md` « Qualité des essais ») ; chaque essai demandé vient avec son **scénario** `Tools\Scenarios\<nom>.txt` lancé par `play.ps1` ; deuxième écran (`-WinX=3940 -WinY=100 -saveddirsuffix=Tests`) ou sans fenêtre.
- **Angelscript** pour le neuf (D483) : règles pures de l'Honneur, des pouvoirs, de l'IA en script avec tests `*_Test.as` ; le C++ seulement pour la simulation, le réseau et le rendu.
- Un asset visuel : **une dizaine d'échantillons** (méthode `assets-ten-samples`), **placeholders d'abord**.
- Chemins absolus dans toute commande donnée à l'utilisateur et dans les sous-agents.
- Aucune statistique n'est inventée : chaque unité, bâtiment, pouvoir est **proposé** (valeurs du GDD d'abord) puis **choisi par l'utilisateur** (D345).
- Réseau : l'Honneur et les cadavres sont de l'état de simulation : empreintes identiques, `-Parts net`.

## Étapes

Chaque étape : **objectif**, **à toucher**, **test rouge d'abord**, **fin** et, si besoin, **valeurs à proposer**.

### A1 Hallebardier (données + script ; dépend de L0)
- **Objectif** : l'Aube produit le Hallebardier à la place du Lancier (D79) : anti-cavalerie, **bonus contre l'infanterie lourde**, **plus cher** que le Lancier.
- **À toucher** : ligne `Halberdier` dans `UnitTypes.csv` (`Faction` = `Dawn`, catégorie infanterie légère comme le Lancier, `Bonuses` contre `Cavalry` et `HeavyInfantry`) ; rôle « Lancier » dans `Factions.csv` (Aube → `Halberdier`, Légions → `Spearman`) ; `Produces` de la Caserne par faction ; `Script/AI/` (l'IA lit le rôle au lieu de `"Spearman"` pour l'Aube : `HOLAIOpening.as`, `HOLAINeeds.as`, table de contres) ; textes fr / en / ja ; maillage placeholder (Mannequin teinté, hallebarde).
- **Test rouge d'abord** : la grille de la Caserne de l'Aube propose `Halberdier` et jamais `Spearman` (celle des Légions l'inverse) ; un Hallebardier fait plus de dégâts à un Homme d'armes qu'un Lancier et autant à la cavalerie ; son coût est supérieur au Lancier ; l'IA de l'Aube produit des Hallebardiers contre la cavalerie ; Aube contre Aube sans plantage.
- **Valeurs à proposer** : PV, attaque, cadence, portée, bonus (cavalerie, infanterie lourde), coût, temps (plus cher que 60 nourriture + 20 bois, D79).
- **Fin** : scénario `dawn_halberd` (un Hallebardier et un Lancier face à Homme d'armes et Cavalier, coûts affichés) ; tests verts. *Hallebardes trempées* reste grisée jusqu'à A11.

### A2 Chevalier Vertueux et *Défi* passif (C++ de simulation + données)
- **Objectif** : le Chevalier Vertueux (Caserne, palier 1, D687) avec son passif : **une part des dégâts subis par les alliés proches est redirigée vers lui** (D19) ; **héros exclus** (ni protégés ni provoqués) ; rayon visible au sol à la sélection ; la part redirigée et le rayon sont des **valeurs de table**.
- **À toucher** : ligne `VirtuousKnight` dans `UnitTypes.csv` (catégories `HeavyInfantry` + `Heavy`, `Faction` = `Dawn`) ; `Produces` de la Caserne (palier 1) ; la redirection dans la chaîne de combat, **à côté** du modèle de `HOLCombatProcessor.cpp:638-730` (*Aura de l'Aube*, *Serrez les rangs*) ; nouveau champ de table (part redirigée, rayon) lu par la simulation ; **déterminisme et empreinte** (les dégâts redirigés sont de la simulation) ; rendu : anneau au sol à la sélection (`HOLSelectionMarkers`).
- **Test rouge d'abord** : un allié à portée du Chevalier reçoit moins de dégâts, le Chevalier en reçoit la part redirigée (somme conservée) ; hors rayon, aucun effet ; le héros n'est ni protégé ni redirigé ; deux Chevaliers ne redirigent pas deux fois la même part (règle à proposer : le plus proche, ou plafond de cumul, **à demander à l'utilisateur à l'étape**) ; deux simulations identiques donnent les mêmes PV ; la grille de la Caserne propose le Chevalier au palier 1 seulement.
- **Valeurs à proposer** : PV, attaque, cadence, armures, vitesse, coût, temps de production ; part redirigée (passif), rayon, cumul de plusieurs Chevaliers.
- **Fin** : scénario `dawn_knight` : un Chevalier au milieu d'Archers qui tirent sur des alliés (les PV du Chevalier baissent, ceux des alliés moins) ; essai de l'utilisateur ; `-Parts tests,net`.

### A3 Moine Lumineux (données + script ; dépend de L6a des Légions pour le *Sceau*)
- **Objectif** : l'Aube produit `LuminousMonk` à la Chapelle (rôle « Moine ») ; même soin que le Moine commun ; **produira de l'Honneur** (A5b) ; capacité active ***Sceau de lumière*** : un sceau au sol qui **ralentit les ennemis** dans une zone quelques secondes (recharge). Le Moine commun reste pour les autres factions (D134, D210).
- **À toucher** : ligne `LuminousMonk` (copie du Moine : 80 PV, soin 5 PV/s à 8 m ; `Faction` = `Dawn`) ; `Monk` passe à « toutes sauf Aube et Légions » ; rôle « Moine » dans `Factions.csv` ; `Produces` de la Chapelle par faction ; la capacité par le **cadre de capacités** (L6a, `HOLUnitAbility*`) ; ralentissement de zone : un effet de vitesse sur les ennemis dans le cercle (à poser comme effet temporaire de simulation) ; textes ; maillage placeholder ; *Vœu de pauvreté* (−20 %) vise aussi `LuminousMonk` ; l'atténuation de moral (D125) attend le système de moral.
- **Test rouge d'abord** : la Chapelle de l'Aube produit `LuminousMonk` et jamais `Monk` ; il soigne comme le Moine ; le *Sceau* ralentit les ennemis dans le cercle et eux seulement, pendant sa durée, puis la vitesse revient ; recharge respectée ; *Vœu de pauvreté* réduit son coût.
- **Valeurs à proposer** : coût, temps, durée et rayon du sceau, pourcentage de ralentissement, recharge.
- **Fin** : scénario `dawn_monk` ; tests verts. **Si le cadre L6a n'existe pas encore**, A3 livre le type et le soin ; le *Sceau* attend le cadre (règle D690 : le premier qui en a besoin le construit, voir A6a et A7).

### Jalon « Aube contre Légions jouable » (après L0 à L4 des Légions, A1, A2, A3)
- Partie Aube (Hallebardier, Chevalier, Moine Lumineux, héros avec capacités) contre Légions (Zombie, socle reskin, Guerrier Damné, héros sans kit), sans Honneur et sans IA propre (IA = A12, L12) ; scénario `dawn_vs_legions` (armées posées) ; **à montrer à l'utilisateur** avant de continuer ; partagé avec le jalon des Légions.

### A4 Honneur : jauge, plafond, interface (script + C++ léger)
- **Objectif** : chaque emplacement de l'Aube a un **Honneur** (entier, plancher 0, plafond fixe) ; **aucun autre joueur n'en a** ; état de simulation, **répliqué**, dans l'**empreinte** ; **barre d'Honneur dans le HUD de faction** (emplacement au-dessus du bloc du héros, GDD § 15).
- **À toucher** : un petit module de règles en script (`Script/Honor/HOLHonorRules.as` : gain plafonné, perte à 0, dépense) et un état par emplacement côté C++ (sous-système ou champ du `PlayerState`, au même niveau que les cadavres des Légions : même schéma de réplication) ; `Script/UI/` (barre et chiffre, par `UHOLScriptScreen`) ; commande de triche `hol.Cheat.Honor <n>` pour les essais ; textes ; `-Parts net`.
- **Test rouge d'abord** : un emplacement de l'Aube démarre avec l'Honneur de départ (**valeur à proposer**, probablement 0) ; un emplacement des Légions n'a ni état ni barre ; gagner au plafond perd le surplus ; dépenser plus que le stock est refusé ; le client voit la même valeur que le serveur (`-Parts net`) ; deux simulations identiques, mêmes empreintes.
- **Valeurs à proposer** : plafond, Honneur de départ.
- **Fin** : triche `hol.Cheat.Honor` visible à l'écran en solo et en réseau ; tests verts.

### A5 Sources d'Honneur (script, une sous-étape chacune)
- **A5a Défendre** : ennemis tués **près des bâtiments, murs ou points** de l'Aube (rayon à proposer) ; Honneur par kill (valeur à proposer ; selon le coût de l'unité ?).
- **A5b Protéger (soins)** : PV soignés par un Moine Lumineux (et les Moines de l'Aube) donnent de l'Honneur ; *Monastère* (A10) l'augmente.
- **A5c Duels** : **accepter** un duel (petit gain), **gagner** (gros gain, D35, avec la recharge immédiate de la *Bannière de l'Aube*), **refuser** (perte propre à l'Aube, **inférieure au gain d'une acceptation**, D76, § 10.6) ; ce sont les premiers lecteurs de `bRefusalCost` et `bFactionEffect` (`HOLDuelRules.as:284, 292`) : **à limiter à l'Aube** (les Légions ne perdent que l'*Hésitation*, D76) ; coordonner avec `DuelInProgress.md` D7 (récompenses branchées : XP, moral, Hésitation, ce dossier ne branche **que** l'Honneur et la recharge de la Bannière) ; poser `bDawn` à vrai dans la vue de duel de l'IA (`HOLAIBrain.as:441-460`).
- **A5d *Défi* absorbé** (après A2, avant ou avec A7) : les dégâts absorbés par un Chevalier donnent de l'Honneur.
- **A5e Tenir un point** : un **relais neutre** possédé et défendu pendant une durée sous pression ennemie (D688 : relais seuls) ; s'appuie sur `RelaysInProgress.md` (propriété d'un relais, présence ennemie).
- **Test rouge d'abord (chaque sous-étape, avec ses cas)** : le gain a lieu pour l'Aube et pas pour les Légions ; il respecte le plafond ; il suit la formule de table ; en duel, accepter donne moins que gagner, refuser retire moins que le gain d'une acceptation, le plancher 0 tient ; la Bannière se recharge sur une victoire ; deux simulations identiques, mêmes Honneurs.
- **Valeurs à proposer** : tous les gains et la perte du refus (D345).
- **Fin** : scénario `dawn_honor` (tuer près d'un bâtiment, soigner, duel accepté / gagné / refusé, relais tenu : la barre monte et descend) ; essai de l'utilisateur.

### A6 Pouvoirs d'Honneur (script + un peu de C++)
- **A6a Livre de pouvoirs et *Lumière sacrée* (palier 0)** : un **livre de pouvoirs** dans le HUD de faction (un bouton par pouvoir, coût, recharge, grisé si Honneur ou palier insuffisant), commande à ciblage au sol ; ***Lumière sacrée*** : le joueur vise un point, un cercle est annoncé au sol, les alliés du cercle sont **soignés en continu pendant ~5 s** (D689) ; le retrait de moral et l'immunité de D404 **attendent le système de moral** (notés « Bientôt »). **Courte recharge propre** à chaque pouvoir. Le **cadre de capacités** (L6a) y sert aussi : première des deux sessions (Aube ou Légions) qui en a besoin le construit.
- **A6b *Renforts de l'Aube* (palier 2)** : une escouade de Chevaliers Vertueux arrive au centre principal, **comptée dans la population** ; l'arrivée est **annoncée ~5 s** (colonne de lumière visible de l'ennemi) ; **refus total, sans coût**, si la population ne laisse pas toute l'escouade (D689).
- ***Rempart béni* (palier 1) : attend les fortifications** (D686) ; le livre le montre grisé « Bientôt » ; ***Jugement* et *Veille des saints* (palier 3) : hors prototype.**
- **Test rouge d'abord** : l'ordre est refusé sans Honneur suffisant ou avant le palier ; le coût est dépensé à la pose ; la recharge est respectée ; *Lumière sacrée* soigne les alliés du cercle et eux seulement, pas les ennemis ni le héros adverse ; *Renforts* ajoute N Chevaliers à la population au bout de ~5 s, est refusé si N places ne sont pas libres, et le coût n'est alors pas dépensé ; l'IA ne les utilise pas encore (A12) ; réseau : l'ordre passe par le serveur (`-Parts net`).
- **Valeurs à proposer** : coût en Honneur de chaque pouvoir (croissant par palier), recharge, rayon, soin total et durée de *Lumière sacrée*, taille de l'escouade des *Renforts*.
- **Fin** : scénario `dawn_powers` (barre d'Honneur pleine par triche, les deux pouvoirs lancés à la main) ; essai de l'utilisateur.

### A7 *Défi du Chevalier* actif (C++ + script)
- **Objectif** : bouton actif sur le Chevalier : pendant quelques secondes, **la totalité des dégâts** des alliés de la zone est redirigée vers lui, et les **ennemis proches sont provoqués** (ils l'attaquent en priorité, **héros exclus**) ; recharge ; source d'Honneur A5d.
- **À toucher** : le cadre de capacités ; la provocation dans la sélection de cible (`HOLCombatProcessor` / acquisition de cible d'unité : un état « provoqué par » avec durée) ; l'IA ennemie obéit à la provocation (l'acquisition de cible de l'IA et des unités neutres : à vérifier).
- **Test rouge d'abord** : pendant l'effet, 100 % des dégâts alliés de la zone vont au Chevalier ; les ennemis proches le ciblent et pas un allié plus proche ; après la durée, retour au passif ; recharge ; le héros allié et le héros ennemi ne sont ni protégés ni provoqués ; deux simulations identiques.
- **Valeurs à proposer** : durée, rayon, recharge, durée de la provocation.
- **Fin** : scénario `dawn_knight_active` ; essai de l'utilisateur.

### A8 Mécanisme du choix de palier (C++ + script ; **partagé avec L9 des Légions**)
- **Objectif** : le **choix définitif entre deux spécialisations** d'un palier (D07, D332 à D334, D683, D684) : un bouton dans le Forum (centre principal), un **compte à rebours en parallèle** (30 / 45 / 60 s selon le palier) dans le HUD de faction, **annulable sans coût avant la fin**, **définitif une fois appliqué** ; ne coûte rien (technologie) ou débloque un bâtiment payé ensuite (bâtiment) ; reste à régler : le compte à rebours quand le Forum est à terre (D684) et le signe visible (D333).
- **Règle D690** : **la première session qui a besoin de ce mécanisme le construit** (Aube ici, ou L9 des Légions) ; **vérifier d'abord dans `main` s'il existe déjà**.
- **Test rouge d'abord** : le choix lance le compte à rebours ; l'annuler rouvre le choix ; après l'application l'autre option est fermée pour toujours ; un deuxième joueur a son propre choix ; réseau (`-Parts net`).
- **Valeurs** : durées 30 / 45 / 60 s (D684), à régler en test.
- **Fin** : tests verts ; scénario `dawn_choice`.

### A9 Chapelle de campagne (données + script ; bâtiment)
- **Objectif** (D123, D690) : spécialisation du palier 1 (bâtiment) : **plusieurs exemplaires**, constructible **loin de la base mais pas près d'un centre ennemi** (D22), **fragile** ; **chaque exemplaire étend le bonus de ferme du Sanctuaire dans son rayon de 30 m, sans cumul** (une ferme couverte par deux sources reçoit +15 % une fois) ; point de ralliement ; forme des Moines Lumineux.
- **À toucher** : ligne `FieldChapel` dans `BuildingTypes.csv` (`Faction` = `Dawn`, débloquée par le choix de palier A8, `Produces` = `LuminousMonk`) ; règle de pose (réutiliser la règle anti-rush de l'**Autel de sang** des Légions, L9 : même étape, un seul code) ; `HOLGatherRules.*` : le rayon des Sanctuaires devient « rayon des Sanctuaires et Chapelles de campagne » (`GetSanctuaryFarmPercent`).
- **Test rouge d'abord** : après le choix, la Chapelle de campagne devient constructible ; la pose est refusée près d'un centre ennemi ; une ferme à 20 m d'une Chapelle de campagne seule reçoit +15 %, une ferme couverte par le Sanctuaire et la Chapelle +15 % **une fois** ; détruire la Chapelle retire le bonus ; elle forme des Moines Lumineux ; si le joueur choisit *Bastion de l'Aube* (grisé, A10), la Chapelle de campagne reste fermée.
- **Valeurs à proposer** : coût, taille, PV, temps de construction, rayon d'interdiction.
- **Fin** : scénario `dawn_fieldchapel` ; essai.

### A10 Spécialisations en technologies (données + script ; dépend de A8)
- **Palier 1** : ***Bastion de l'Aube*** : **grisé « Bientôt »** (D601, D686) jusqu'aux fortifications ; l'autre option, la *Chapelle de campagne* (A9), est jouable.
- **Palier 2** : ***Lices de l'Aube*** (technologie) : Chevaliers formés plus vite, *Défi du Chevalier* renforcé (passif et actif : part redirigée, durée) ; ***Monastère*** (technologie) : Moines Lumineux qui soignent mieux, **soins qui rapportent plus d'Honneur** ; l'atténuation de moral plus forte (D125) attend le système de moral.
- **Marques visibles** (D336) : bannières (Bastion), banderoles et barrière de joute dans la cour de la Caserne (Lices), cloche au fronton des Chapelles (Monastère) : placeholders (étape A13).
- **Test rouge d'abord** : *Lices* réduit le temps de production du Chevalier et augmente la part redirigée ; *Monastère* augmente le soin et l'Honneur par soin ; **une seule des deux options est active** ; le choix se fait après 45 s (D684).
- **Valeurs à proposer** : effet chiffré de chaque technologie.
- **Fin** : tests verts ; scénario `dawn_tier2`.

### A11 Technologies propres de l'Aube (données + script)
- ***Liturgie*** (Sanctuaire, palier 1) : gains d'Honneur +10 % (`Implemented` passe à vrai avec A4) ; ***Hallebardes trempées*** (Caserne, palier 1) : bonus du Hallebardier contre l'infanterie lourde accru (A1) ; ***Bénédiction des armes*** (Sanctuaire, palier 2) : Chevaliers et Hallebardiers +attaque contre « Profane » (dépend de **L3 des Légions**, catégorie Profane) ; ***Vœu de pauvreté*** déjà jouable (D603, adapter à `LuminousMonk`).
- **Filtrer par faction** : les quatre technologies ne sont proposées qu'à l'Aube (colonne `Faction` lue par la grille de recherche, aujourd'hui lue seulement par les tests).
- **Test rouge d'abord** : chaque technologie se recherche au bon bâtiment et au bon palier, change la statistique visée, n'est pas proposée aux Légions ; *Liturgie* augmente un gain d'Honneur de 10 % ; l'IA les recherche (`HOLAITechnologyLibrary.h`, table de retards, D657).
- **Fin** : tests verts ; la ligne T6 de `TechnologiesInProgress.md` mise à jour (Aube débloquée).

### A12 IA de l'Aube (script)
- **Objectif** : l'IA de l'Aube joue chaque système (§ 14.4) : production (Hallebardiers contre cavalerie, Chevaliers en ligne, Moines Lumineux derrière), **dépense de l'Honneur par une règle simple par pouvoir** (D688 : *Lumière sacrée* sous un seuil de PV d'un groupe en combat, *Renforts* quand le combat est perdu ou le centre attaqué, réserve gardée), *Défi* actif, *Sceau de lumière*, choix de spécialisation selon le profil, acceptation de duel (`bDawn` posé), niveaux Facile / Normal / Difficile et Évolutive comme les autres, **aucune triche**.
- **Test rouge d'abord** : l'IA de l'Aube lance *Lumière sacrée* quand son groupe est en difficulté et pas autrement ; elle ne dépense pas sous sa réserve ; elle produit selon les contres ; `aiduel` Aube IA contre Légions IA sans plantage ; les règles pures en `*_Test.as`.
- **Fin** : `aiduel` ; essai de l'utilisateur.

### A13 Visuel et son (art)
- Placeholders d'abord (Mannequin teinté, échelles), puis **dix échantillons à choisir** (`assets-ten-samples`, `VisualInProgress.md`) pour le Chevalier Vertueux, le Hallebardier, le Moine Lumineux, le Sanctuaire, la Chapelle de campagne, les sceaux, la colonne de lumière des Renforts ; marques visibles des spécialisations (D336) ; sons (`AudioInProgress.md`) ; bannières du joueur (VI-Q3 = B / D666).

### A14 Essais et clôture
- Scénarios `dawn_*` et leur pré-vol ; `aiduel` ; essai de l'utilisateur (Aube contre Légions en vrai, solo et réseau) ; **test prioritaire 10** (§ 17.1) noté pour l'Aube ; mesures S1 / S2 à la clôture seulement (`rest_measures.ps1`) ; `-Full` à la fusion ; décisions numérotées au GDD ; journal clos et archivé dans `Docs/Archive/`.

## Ce que ce chantier touche chez les autres

- **Légions** (`LegionsInProgress.md`) : L0 (socle) **prérequis** ; L3 (Profane) pour *Bénédiction des armes* ; L6a (cadre de capacités) partagé pour *Sceau de lumière*, *Défi* actif et les pouvoirs ; L9 (mécanisme du choix de palier, règle anti-rush de l'Autel de sang) partagé avec A8 et A9 : **la première session qui arrive le construit**, l'autre le reprend ; les Légions n'ont ni Honneur ni Chapelle ni Moine.
- **Duel** (`DuelInProgress.md`) : A5c lit `bRefusalCost` et `bFactionEffect` (Honneur et recharge de la Bannière) ; l'XP, le moral et l'*Hésitation* de l'étape D7 restent au chantier du duel.
- **Technologies** (`TechnologiesInProgress.md`) : les quatre lignes `Dawn`, le filtre par faction, la colonne `Implemented` (D601).
- **Talents** (`TalentsInProgress.md`) : les Serments ajoutent un bonus à une source d'Honneur (D106) ; hors de ce dossier (les gains de base viennent d'ici).
- **Héros** (`JourneyInProgress.md`, J8) : la *Bannière* reçoit sa recharge de duel ; rien d'autre.
- **Relais** (`RelaysInProgress.md`) : propriété d'un relais lue par A5e.
- **Réseau** : l'Honneur et les Renforts sont de l'état de simulation répliqué ; `-Parts net`.
- **Fortifications** (dossier à ouvrir) : *Rempart béni*, *Bastion de l'Aube* (D686).
- **Moral** (dossier à ouvrir) : retrait des malus et immunité de *Lumière sacrée*, atténuation du Moine Lumineux.
- **Visuel** (`VisualInProgress.md`), **son** (`AudioInProgress.md`).

## Après le prototype (ne pas faire ici)

Palier 3 : *Autel du Jugement* et *Jugement*, *Crypte des saints* et *Veille des saints* ; *Archer de l'Aube* (D82) ; *Rempart béni* et *Bastion de l'Aube* quand les fortifications existeront ; talents des rangées 7 à 9 (D195) ; *Charge de l'Aube* (héros niveau 7) ; livre à choix façon BFME (piste de D85).

## Comment lancer

1. **L0 des Légions doit être fusionné dans `main`** (sinon attendre ; A1 en dépend). Vérifier si `L6a` et le mécanisme du choix de palier existent déjà.
2. L'utilisateur donne un worktree (ou dit lequel prendre) ; réserver par `IN_USE.txt` sans écraser ; créer la branche `dawn` depuis `main` à jour.
3. Lire ce journal, `CLAUDE.md` du projet et `Auto_Journey.md` « Qualité des essais ».
4. Vérifier que le worktree compile (`Tools\build.ps1`), puis commencer par **A1**, test rouge d'abord ; mettre à jour ce journal et `EtapeDefini.md` à chaque étape.
5. Demander les valeurs à l'utilisateur étape par étape (une proposition du GDD, il tranche) ; numéroter les décisions au GDD § 21 (prochain numéro libre à relire à la fin du § 21).

## Journal

- 2026-11-04 : dossier ouvert à la demande de l'utilisateur (« Aube, un dossier à la fois »), état des lieux fait dans le code par un sous-agent de lecture (HEAD 755c6d4) : ni Honneur, ni pouvoir, ni Chevalier, ni Hallebardier, ni *Défi* dans le jeu ; Sanctuaire, Chapelle, Moine et *Vœu de pauvreté* existent.
- 2026-11-04 : AQ-1 à AQ-14 tranchées avec l'utilisateur (socle = L0 des Légions, Hallebardier vrai type, fortifications absentes donc *Rempart béni* et *Bastion* attendent, Moine Lumineux avec *Sceau de lumière*, Chevalier à la Caserne palier 1 + Lourd, *Défi* en deux temps avec aura = passif, Honneur plafonné, cinq sources, relais seuls, *Lumière sacrée* en soin continu ~5 s, *Renforts* annoncés et refusés si la population manque, coût + courte recharge, mécanisme du choix de palier construit par la première session qui en a besoin, Chapelle de campagne en plusieurs exemplaires sans cumul, IA à règle simple). Numérotées D685 à D690 au GDD. **Statut : prêt à lancer** après L0.
