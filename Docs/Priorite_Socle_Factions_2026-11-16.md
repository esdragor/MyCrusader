# Priorité : le socle commun des factions (relevé du 2026-11-16, remis à jour le 2026-11-22)

**Pour qui** : la prochaine séance Auto Journey (orchestrateur et agents qu'il lance).
**Consigne de l'utilisateur** : concentrer la séance sur ce qui manque pour qu'**une faction quelconque** (Aube, Légions, puis les autres) puisse avoir son identité jouable. Ce document ne remplace pas les dossiers `*InProgress.md` : il dit **quoi faire en premier** et renvoie à l'étape qui porte le détail. Les tâches sont les lignes S1 à S4 de la section « Socle commun des factions » d'`Auto_Journey_File.md`.

**État relevé le 2026-11-22** par lecture du code de `main` 357a1fc (code = integ-53, 2097/2097) ; rien n'a été rejoué. **Avant de lancer une ligne, revérifier dans `main`.** Ancienne version : `Docs/Archive/Priorite_Socle_Factions_2026-11-16_avant_maj_2026-11-22.md`.

**En une phrase** : l'Aube a ses unités, sa barre d'Honneur et ses auras, mais aucun pouvoir de faction ne se lance en jeu. Beaucoup de règles sont écrites et testées ; il manque le dernier branchement. « Fait » veut dire ici : appelé en jeu, pas seulement testé.

## Ordre de priorité

| Ordre | Lot | Lignes | Ce que le joueur y gagne |
|---|---|---|---|
| 1 | Premier pouvoir jouable | S1-a à S1-e | L'Aube gagne de l'Honneur et lance *Lumière sacrée* ; le Moine pose son Sceau |
| 2 | Moral, sous-étapes 5 et 6 (puis 7 à 10) | S2-a à S2-d | Triomphe, Démoralisé et Hésitation s'appliquent après un duel |
| 3 | Tir de tour, puis palissade | S3-a, S3-b | Une tour qui tire, puis un vrai mur |
| 4 | IA | S4-a, S4-b | L'adversaire utilise ses pouvoirs et ses garnisons |

S2 et S3 peuvent avancer en parallèle de S1. S4-a attend S1. S1-a, S2-a, S2-b et S3-a touchent `HOLCombatProcessor` ou la simulation : un seul agent à la fois sur ce fichier, un build groupé.

## État point par point

### 1. HUD de faction et livre de pouvoirs : À MOITIÉ (lot S1)

- **Fait** : jauge répliquée par emplacement (`AHOLGameState::FactionResource`), recharges répliquées (`PowerCooldownEndMs`), `hol.Cheat.Honor`, panneau affiché (essai `faction_slot` OK le 2026-11-20), Honneur gagné et perdu par les duels, visée C++ (`SetFactionAim`), règle de clic (`HOLFactionSlot::ClickPower`), RPC `ServerCastFactionPower`.
- **Manque** : ces morceaux ne sont pas reliés. `FactionPanel` est `HitTestInvisible` (`HOLGameScreenScript.as:2037`) ; `SetFactionAim`, `ClickPower` et le RPC n'ont aucun appelant en jeu ; *Lumière sacrée* n'a pas d'effet (`bEffectCoded` faux, donc « bientôt ») ; l'Honneur ne vient ni des combats ni des soins.
- **Piège** : la ligne 0 du livre est *Lumière sacrée*, mais le RPC pose le *Sceau de lumière*, que D862 donne au Moine. Relier tel quel lancerait le mauvais pouvoir.
- **Coordination** : le bouton permanent de spécialisation (SP3, D823) prend le même emplacement : un seul emplacement pour les deux.
- **Pas d'autre client** : `HOLFactionResource::HasResource` ne vaut vrai que pour l'Aube. Les cadavres des Légions (`HOLCorpse`) sont de l'affichage, pas une ressource de simulation : ne pas les prendre pour modèle (modèle = `TierOverrides`).
- **Détail** : [AubeInProgress.md](../AubeInProgress.md) A4 et A6a ; D689, D859, D862.

### 2. Capacités visées au sol : À MOITIÉ (S1-d et restes)

- **Fait** : colonnes `Target`, `RadiusCm`, `RangeCm` de `UnitAbilities.csv`, lignes `LightSeal` et `KnightChallenge`, zone qui ralentit lue par le combat (`HOLZoneEffect`).
- **Manque** : aucun bouton ne lance le Sceau ; `KnightChallenge` n'a pas d'effet en jeu (A7).
- **Détail** : [LegionsInProgress.md](../LegionsInProgress.md) L6a ; D690, D875.

### 3. Moteur de moral : BIEN AVANCÉ (lot S2)

- **Fait** : sous-étapes 1 à 4 du `PLAN-MORAL-1120` et 6a : règle C++ unique, pont, fragment, processeur inscrit ; aura de l'Aube (héros de l'Aube seulement, D880), bannière et cri posés ; le combat lit les dégâts reçus.
- **Manque** : dégâts infligés et cadence (sous-étape 5), pont du duel (6), atténuation, immunité et commandes (7, 8), masque réseau (9), affichage (10).
- **À vérifier** : le processeur de moral est inscrit après celui du combat dans `DefaultGame.ini`, le plan le veut avant ; l'ordre réel n'a pas été mesuré.
- **Détail** : [MoraleInProgress.md](../MoraleInProgress.md), `Docs/Rapports/PLAN-MORAL-1120.md` ; D866, D880.

### 4. Choix de spécialisation : CHOIX FINI

- **Fait** : serveur, cases du Forum, 3 voies des Templiers (D868), `hol.Cheat.Specialization` (essai OK), Forum Nomade branché (`HasCompleteForumNow`).
- **Manque** : aucun effet des voies trouvé en jeu ; bouton permanent (SP3, D823).
- **Détail** : [SpecializationInProgress.md](../SpecializationInProgress.md).

### 5. Récompenses du duel : À MOITIÉ (S2-b et restes)

- **Fait** : ordres d'Honneur appliqués à la jauge (accepter, gagner, refuser).
- **Manque** : Triomphe, Démoralisé et Hésitation s'empilent dans `PendingMorale`, que rien ne lit ; XP du duel 150 (D859 n. 9).
- **Détail** : [DuelInProgress.md](../DuelInProgress.md) D7.

### 6. Fortifications : DÉBUT (lot S3)

- **Fait** : Tour de guet en bois (données), pose en ligne (geste, RPC, validation, D867) ; règle de garnison.
- **Sur integ-41, hors main** : tir de bâtiment T1 à T6 (verts avant la fusion de `main`, à rejouer) ; restent T7 et T8.
- **Manque** : aucun mur, palissade ni porte (`WallTypes()` est vide).
- **Détail** : [FortificationsInProgress.md](../FortificationsInProgress.md) F5, F8, F9, F10.

### 7. IA d'une autre faction : RÈGLES SEULES (lot S4)

- **Fait** : `HOLAIPowerSpend` et `HOLAIGarrison` écrits et testés.
- **Manque** : aucun appelant en jeu.
- **Détail** : [LegionsInProgress.md](../LegionsInProgress.md) L12, [AubeInProgress.md](../AubeInProgress.md) A12.

## Déjà fait (ne pas refaire)

- Socle multi-faction L0 et L2 : rôles (`Factions.csv`), surcharges, colonne `Faction`, grilles filtrées, refus serveur ; technologies filtrées ; catégorie « Profane » (L3) ; faction lue par l'IA (G4).
- **Aube A1 (Hallebardier)** dans la Caserne, l'IA le connaît.
- **Aube A2 (Chevalier Vertueux, redirection passive 30 % / 8 m)** : combat, anneau et ligne de table ; essai `virtuous_knight` OK le 2026-11-21.
- D860 (marque de signal et Alt + clic sur la mini-carte), D861 (engin démonté qui attaque : tests présents ; le reste du chantier deploy est D877).

## Règles de séance à garder

Celles de `C:\Users\baudo\Documents\MyCrusader\Auto_Journey.md` (test rouge d'abord sur la vraie chaîne, `Tools\build.ps1 -LiveCoding=0`, `quick_check.ps1 -Parts`, scénario pour tout essai demandé, valeurs proposées puis choisies par l'utilisateur (D345), commits en anglais, fusion locale à chaque passe verte (D888), **jamais de push**). Code neuf en Angelscript, C++ par exception avec sa raison (D839). Mettre à jour le dossier `*InProgress.md` concerné et `EtapeDefini.md` à chaque avancée.
