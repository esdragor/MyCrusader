# Priorité : le socle commun des factions (relevé du 2026-11-16)

**Pour qui** : la prochaine séance Auto Journey (orchestrateur et agents qu'il lance).
**Consigne de l'utilisateur** : concentrer la séance sur ce qui manque pour qu'**une faction quelconque** (Aube, Légions, puis les autres) puisse avoir son identité jouable. Ce document ne remplace pas les dossiers `*InProgress.md` : il dit **quoi faire en premier** et renvoie à l'étape du dossier qui porte le détail (test rouge, valeurs, fin).

**État relevé le 2026-11-16** sur la copie principale (`main` 2b89ee3), alors que des agents tournaient encore (Elrond sur la case de spécialisation). **Avant de lancer une ligne, revérifier dans `main`** : elle a pu avancer depuis.

Rappel de contexte : le chantier de l'Aube ([AubeInProgress.md](../AubeInProgress.md), A1 à A14) n'est **pas commencé** (aucune ligne `Halberdier` / `VirtuousKnight` / `LuminousMonk`, aucun code d'Honneur). Le socle L0 dont il dépendait est fait : **A1 (Hallebardier) peut démarrer**, mais l'essentiel de l'identité de l'Aube (Honneur, pouvoirs, *Lumière sacrée*, Moine Lumineux, *Rempart béni*) bute sur les manques ci-dessous.

## Ordre de priorité

### 1. HUD de faction et livre de pouvoirs visés au sol (manquant, priorité 1)

- **Pourquoi en premier** : c'est ce qui débloque le plus de chantiers. Toute faction à ressource propre ou à pouvoirs en a besoin (Aube A4 et A6 ; Légions : affichage des cadavres ; Templiers, etc.).
- **État** : aucun code trouvé (`FactionSlot`, `FactionBlock`, « emplacement de faction » : rien dans `Source/` ni `Script/`). GDD § 15 : « un emplacement de faction au-dessus du bloc du héros » (D201).
- **À faire** :
  1. **L'emplacement** au-dessus du bloc du héros, par `UHOLScriptScreen` (règle d'affichage en script, D483), vide pour une faction sans ressource.
  2. **Une jauge générique de ressource de faction** (entier, plancher, plafond), état de simulation **répliqué** et dans l'**empreinte**, sur le même schéma que les cadavres des Légions (`HOLCorpse`). L'Honneur de l'Aube (A4) en est le premier client.
  3. **Le livre de pouvoirs** : un bouton par pouvoir (coût, recharge, grisé si ressource ou palier insuffisant, « Bientôt » pour un pouvoir sans effet, D601), et une **commande à ciblage au sol** passant par le serveur (`-Parts net`).
- **Coordination** : la ligne de `Auto_Journey_File.md` « SP3 : bouton permanent à l'emplacement de faction (D201, D823) » pose aussi un bouton à cet emplacement. **Un seul emplacement pour les deux** : la première session qui le crée le fait générique, l'autre s'y branche.
- **Détail** : [AubeInProgress.md](../AubeInProgress.md) A4 (jauge, triche `hol.Cheat.Honor`) et A6a (livre, ciblage au sol).

### 2. Capacités d'unités ordinaires, ciblage au sol (L6a, avancé)

- **État** : la règle en script (`Script/Units/HOLUnitAbilityRules.as`, `Script/Units/HOLUnitAbilityTable.as`) et la table `Tools/Data/UnitAbilities.csv` existent, mais **une seule capacité** les utilise (Rage nécrotique du Guerrier damné, auto-ciblée). **Aucune capacité visée au sol.**
- **À faire** : étendre le cadre à une capacité **visée au sol** (zone, durée, recharge) et à un **effet temporaire de zone** côté simulation. Premiers clients : *Sceau de lumière* (Aube A3), *Défi du Chevalier* actif (A7), les pouvoirs du point 1.
- **Détail** : [LegionsInProgress.md](../LegionsInProgress.md) L6a ; règle D690 (la première session qui en a besoin le construit, l'autre le reprend).

### 3. Moteur de moral côté simulation (M3 à M5, manquant)

- **État** : la règle (M1), la table `Effects.csv` (M2) et la règle d'affichage (M6, script) existent ; **aucun code de moral en C++**, donc les effets ne s'appliquent pas aux unités.
- **Débloque** : *Lumière sacrée* (retrait et immunité, D404), l'atténuation du Moine Lumineux (D125), l'*Hésitation* du duel, l'*Aura de terreur* des Légions, *Discipline de l'Aube*, *Monastère*.
- **À faire** : M3 (copie C++ de la règle, fragment d'effets), M4 (moteur, migration de l'aura, du cri et de la bannière de l'Aube : comportement identique sauf les Paysans), M5 (masque d'effets réseau).
- **Détail** : [MoraleInProgress.md](../MoraleInProgress.md) M3 à M5 (puis M7 et M8 pour le duel et les points d'entrée de l'Aube).

### 4. Choix de spécialisation de palier (avancé, à finir)

- **État** : serveur, règle et cases D / F du Forum faits ; essai `specialization_button` jugé OK par l'utilisateur le 2026-11-16. Elrond y travaillait encore le 2026-11-16 (case sur 2 lignes + `hol.HUD.Report`).
- **Reste** : `hol.Cheat.Specialization` ; test `net_match` à deux clients ; Templiers à 3 options ; branchement du Forum Nomade (N2) ; bouton permanent à l'emplacement de faction (voir point 1).
- **Détail** : [SpecializationInProgress.md](../SpecializationInProgress.md).

### 5. Récompenses du duel branchées (D7 du duel, manquant)

- **État** : `bRefusalCost` et `bFactionEffect` sont posés dans `Script/Duel/HOLDuelRules.as` mais **aucun code ne les lit**.
- **À faire** : D7 de [DuelInProgress.md](../DuelInProgress.md) (XP, Triomphe / Démoralisé, *Hésitation* : dépend du point 3). La part propre à l'Aube (Honneur, recharge de la Bannière) reste à A5c d'[AubeInProgress.md](../AubeInProgress.md).

### 6. Fortifications : murs, portes, tours (F4 à F12, manquant, gros chantier)

- **État** : seule la Forge existe dans `Tools/Data/BuildingTypes.csv` ; la garnison (F7) a son code mais attend l'appelant C++ de l'IA et l'interface (D811, D813). Ni palissade, ni mur, ni porte, ni tour.
- **Débloque** : *Rempart béni* et *Bastion de l'Aube* (D686), la garnison du Roi.
- **Détail** : [FortificationsInProgress.md](../FortificationsInProgress.md), dans l'ordre F4 (pose en ligne), F5 (palissade), F8 (tour qui tire), F9 (portes), F10 (murs et tour de pierre). Long : à faire avancer en parallèle, pas à finir avant le reste.

### 7. IA capable de jouer une autre faction (L12, manquant)

- **État** : l'IA connaît la faction de son emplacement (G4 de [Palier3InProgress.md](../Palier3InProgress.md)), mais l'IA des Légions ne sait que lancer la rage : ni ressource de faction, ni pouvoirs, ni ouverture propre.
- **À faire après les points 1 et 2** : une règle simple de dépense par pouvoir (D688), réglée par niveau, sans triche.
- **Détail** : [LegionsInProgress.md](../LegionsInProgress.md) L12, [AubeInProgress.md](../AubeInProgress.md) A12.

## Déjà fait (ne pas refaire)

- Socle multi-faction L0 et L2 : rôles (`Factions.csv`), surcharges (`FactionOverrides.csv`) appliquées à la naissance, colonne `Faction`, grilles de production et de construction filtrées, refus serveur.
- Technologies filtrées par faction : `GetBuildingTechnologies` (`Source/HeroesOfLiberty/Technologies/HOLTechnologyTypes.cpp:35`).
- Catégorie « Profane » (L3).
- Ressource propre de faction côté simulation : modèle des cadavres des Légions (`HOLCorpse`).
- Faction de l'emplacement lue par l'IA (G4).

## En parallèle, sans attendre le socle

- **Aube A1 (Hallebardier)** : données + script, ne dépend que de L0 (fait). Bon candidat pour un worktree sans C++ lourd.
- **Aube A2 (Chevalier Vertueux, *Défi* passif)** : C++ de simulation, ne dépend d'aucun manque ci-dessus.

## Règles de séance à garder

Celles de `C:\Users\baudo\Documents\MyCrusader\Auto_Journey.md` (test rouge d'abord sur la vraie chaîne, `Tools\build.ps1 -LiveCoding=0`, `quick_check.ps1 -Parts`, scénario pour tout essai demandé, valeurs proposées puis choisies par l'utilisateur (D345), commits en anglais, rien poussé sans accord). Mettre à jour le dossier `*InProgress.md` concerné et `EtapeDefini.md` à chaque avancée, comme d'habitude.
