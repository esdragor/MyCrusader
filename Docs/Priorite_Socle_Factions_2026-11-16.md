# Priorité : le socle commun des factions (relevé du 2026-11-16, remis à jour le 2026-11-23)

**Pour qui** : la prochaine séance Auto Journey (orchestrateur et agents qu'il lance).
**Consigne de l'utilisateur** : concentrer la séance sur ce qui manque pour qu'**une faction quelconque** (Aube, Légions, puis les autres) puisse avoir son identité jouable. Ce document ne remplace pas les dossiers `*InProgress.md` : il dit **quoi faire en premier** et renvoie à l'étape qui porte le détail. Les tâches sont les lignes S1 à S4 de la section « Socle commun des factions » d'`Auto_Journey_File.md`.

**État relevé le 2026-11-23 vers 10:12** d'après la section « Socle commun des factions » d'`Auto_Journey_File.md` et le tableau « Agents du 2026-11-23 » d'`Auto_Journey_Jour.md` (pas de relecture du code ; rien n'a été rejoué à l'écran). **Avant de lancer une ligne, revérifier dans `main`** (`Tools\AutoJourney\status.ps1 -InMain <sha>`). Anciennes versions : `Docs/Archive/Priorite_Socle_Factions_2026-11-16_avant_maj_2026-11-23.md` (celle du 2026-11-22) et `Docs/Archive/Priorite_Socle_Factions_2026-11-16_avant_maj_2026-11-22.md`.

**Trois états, à ne pas mélanger** :
- **DANS MAIN** (47f6fe4 = integ-54, build PASS, quick_check tests 2114/2114, cinq rouges vus) : S1-a soin de zone *Lumière sacrée*, T7 flèche de tour, règle S4-a (`HOLAIPowerCall`), S1-c panneau cliquable (script), XP du duel 150.
- **EN INTÉGRATION** (integ-55 lancée 10:11, aucun résultat) : S1-b (le RPC lance *Lumière sacrée*), Sceau du Moine en script, palissade, S4-a branché, T7b (flèche de tour répliquée), widget SP3, S2-d partie script, mineur 11.
- **SUR BRANCHE** (pas encore à l'intégration) : S1-e soin (Faramir, zone-effect, en cours), S4-b et lot de liaisons C++ (Elrond, integ-25), T8 (Celeborn, integ-41, commencé 10:10).

**En une phrase** : dans `main`, le soin de zone *Lumière sacrée* existe et le panneau de faction se clique, mais le pouvoir ne se lance toujours pas en jeu : le RPC qui le déclenche (S1-b) n'est pas encore dans `main`, il attend integ-55. Le panneau répond encore « bientôt » (`bEffectCoded` faux dans `main`). « Fait » veut dire ici : appelé en jeu ; ce matin, **aucun essai n'a montré à l'écran un pouvoir lancé** : tout ce qui suit est LOGIQUE SEULEMENT (tests verts, un seul scénario joué, `faction_click`, qui montre le refus « bientôt »).

## Ordre de priorité

| Ordre | Lot | Lignes | Ce que le joueur y gagne | État au 2026-11-23 10:12 |
|---|---|---|---|---|
| 1 | Premier pouvoir jouable | S1-a à S1-e | L'Aube gagne de l'Honneur et lance *Lumière sacrée* ; le Moine pose son Sceau | main : S1-a, S1-c ; integ-55 : S1-b, Sceau (script) ; branche : S1-e, bouton du Moine (C++) |
| 2 | Moral, sous-étapes 5 et 6 (puis 7 à 10) | S2-a à S2-d | Triomphe, Démoralisé et Hésitation s'appliquent après un duel | rien dans main ; integ-55 : S2-d partie script ; S2-a à S2-c non commencés |
| 3 | Tir de tour, puis palissade | S3-a, S3-b | Une tour qui tire, puis un vrai mur | main : T7 ; integ-55 : T7b, palissade ; branche : T8 |
| 4 | IA | S4-a, S4-b | L'adversaire utilise ses pouvoirs et ses garnisons | main : règle S4-a ; integ-55 : S4-a branché ; branche : S4-b |

S2 et S3 peuvent avancer en parallèle de S1. S4-a attend S1 (S1-b est son prérequis : sans lui le lancer de l'IA n'a rien à lancer en jeu). S1-a, S1-e, S2-a, S2-b et S3-a touchent `HOLCombatProcessor` ou la simulation : un seul agent à la fois sur ce fichier, un build groupé.

## État point par point

### 1. HUD de faction et livre de pouvoirs : À MOITIÉ (lot S1)

- **DANS MAIN** : jauge répliquée par emplacement, recharges répliquées, `hol.Cheat.Honor`, panneau affiché, Honneur gagné et perdu par les duels, visée (`SetFactionAim`), règle de clic (`HOLFactionSlot::ClickPower`), RPC `ServerCastFactionPower` ; **S1-a** : effet de *Lumière sacrée* (zone de soin ~5 s dans `HOLHealProcessor`, `StartSacredLight`, test `Zone.SacredLight` vert) ; **S1-c** : panneau cliquable (script, `HOLFactionAim.as` neuf, routage `FactionBox*`), scénario `faction_click` joué (vrai clic, 2 PNG vus) mais il n'affiche que le refus « bientôt ».
- **EN INTÉGRATION (integ-55)** : **S1-b** : `ServerCastFactionPower` lance *Lumière sacrée* (plus le Sceau), paie 25 Honneur, recharge 20 s, `bEffectCoded` vrai (rouge b2ea3aa vu 09:24, vert 2100/2102 avec 2 rouges uasset palissade).
- **SUR BRANCHE** : **S1-e** (Faramir, zone-effect) : Honneur du soin écrit (`HOLHealProcessor::CreditHonor`, installeur script), build en cours, ni vert ni commit ; reste l'Honneur à la mort (`HonorKillGain`, `HOLCombatProcessor.cpp`). **S1-d** : bouton du Moine, voir point 2.
- **Reste après S1-b** : activer la variante `faction_click` (tir au sol), fiche E ; elle reste LOGIQUE SEULEMENT tant que S1-b n'est pas joué.
- **Piège levé** : la ligne 0 du livre est *Lumière sacrée* et le RPC posait le *Sceau de lumière* (D862, Moine) ; S1-b corrige cela (en intégration).
- **Question du jour** : `Retours\Q-sacred-light-values` (rayon et soin de *Lumière sacrée* provisoires, défaut A codé).
- **Coordination** : le bouton permanent de spécialisation (SP3, D823) prend le même emplacement : un seul emplacement pour les deux.
- **Pas d'autre client** : `HOLFactionResource::HasResource` ne vaut vrai que pour l'Aube ; les cadavres des Légions (`HOLCorpse`) sont de l'affichage, pas une ressource de simulation (modèle = `TierOverrides`).
- **Détail** : [AubeInProgress.md](../AubeInProgress.md) A4 et A6a ; D689, D859, D862.

### 2. Capacités visées au sol : À MOITIÉ (S1-d et restes)

- **DANS MAIN** : colonnes `Target`, `RadiusCm`, `RangeCm` de `UnitAbilities.csv`, lignes `LightSeal` et `KnightChallenge`, zone qui ralentit lue par le combat.
- **EN INTÉGRATION (integ-55)** : Sceau du Moine en script (`HOLMonkSeal::Decide`, RPC `ServerCastUnitAbility`, texte `LightSeal` corrigé) ; tests ajoutés, rouges d'origine non vus à l'exécution pour la plupart.
- **SUR BRANCHE** (integ-25, lot de liaisons C++ d'Elrond, build PASS 705 s, tests non lancés) : `LightSeal` dans `EHOLGridCommand` et `EHOLGridCommandKind`, `bMonkUnit`, `GetUnitOwner` / `GetUnitType` ; reste la cellule `LightSeal` dans `HOLCommandGridRules.as` et le remplissage de `bMonkUnit` par le contrôleur : **aucun bouton du Moine n'existe donc encore**.
- **Manque** : `KnightChallenge` n'a pas d'effet en jeu (A7).
- **Détail** : [LegionsInProgress.md](../LegionsInProgress.md) L6a ; D690, D875.

### 3. Moteur de moral : BIEN AVANCÉ (lot S2)

- **DANS MAIN** : sous-étapes 1 à 4 du `PLAN-MORAL-1120` et 6a : règle C++ unique, pont, fragment, processeur inscrit ; aura de l'Aube, bannière et cri ; le combat lit les dégâts reçus.
- **EN INTÉGRATION** : S2-d partie script (`HOLMoraleView.as`, crochet HUD, 4 tests non joués) ; la vue reste vide tant que le masque d'effets répliqué n'existe pas.
- **Manque (rien commencé)** : dégâts infligés et cadence (5, S2-a), pont du duel (6, S2-b), atténuation, immunité et commandes (7 et 8, S2-c), masque réseau C++ (9) et `HOLMoraleView::Read` réel, scénario `morale_look` (10).
- **À vérifier** : le processeur de moral est inscrit après celui du combat (`DefaultGame.ini` l.43 contre l.41), le plan le veut avant ; ordre réel non mesuré.
- **Détail** : [MoraleInProgress.md](../MoraleInProgress.md), `Docs/Rapports/PLAN-MORAL-1120.md` ; D866, D880.

### 4. Choix de spécialisation : CHOIX FINI

- **DANS MAIN** : serveur, cases du Forum, 3 voies des Templiers (D868), `hol.Cheat.Specialization`, Forum Nomade branché.
- **EN INTÉGRATION** : widget SP3 (bouton permanent) en script, vue vide jusqu'à la liaison ; LOGIQUE SEULEMENT, pas de scénario.
- **SUR BRANCHE** : `GetSpecialization` exposé en UFUNCTION (integ-25) ; palier et Forum complet côté client NON FAIT.
- **Manque** : **aucun effet des voies en jeu** (`ChosenOption` n'est lu que par le tableau). Relevé : `Docs/Rapports/SPEC-VOIES-1123.md`. **Les 9 unités de commanderie n'existent pas** (`UnitTypes.csv` : 18 unités, aucune des Templiers) : à créer avant tout déblocage ; question `Retours\Q-spec-voies` (défaut A, non codé).
- **Détail** : [SpecializationInProgress.md](../SpecializationInProgress.md).

### 5. Récompenses du duel : À MOITIÉ (S2-b et restes)

- **DANS MAIN** : ordres d'Honneur appliqués à la jauge (accepter, gagner, refuser) ; **XP du duel 150** (D859 n. 9 : tests `HeroXP.Duel150` et `Test_DuelXPGoesToTheWinnerOnly` verts, essai en jeu à faire).
- **Manque** : Triomphe, Démoralisé et Hésitation s'empilent dans `PendingMorale`, que rien ne lit (S2-b).
- **Détail** : [DuelInProgress.md](../DuelInProgress.md) D7.

### 6. Fortifications : EN COURS (lot S3)

- **DANS MAIN** : Tour de guet en bois (données), pose en ligne (D867), règle de garnison ; **T7** (affichage du tir de tour, `BuildingShot*`).
- **EN INTÉGRATION (integ-55)** : **T7b** (flèche de tour répliquée, `Net.BuildingArrow` vert 2108/2108 sur integ-41) ; **palissade** (S3-b, script) : ligne `Palisade` dans `BuildingTypes.csv`, `WallTypes()` non vide, valeurs provisoires. Rouge du test de palissade non vu à l'exécution ; `regen_tables.ps1` (`BuildingTypes.uasset`) reste à faire, deux tests rouges en attendent.
- **SUR BRANCHE** : **T8** (coût par pas, banc 200 unités et 10 tours, scénario `tower_wood`) : commencé 10:10 par Celeborn sur integ-41.
- **Manque** : portes (F9), murs (F10) ; à l'essai, un glisser hors axe s'aligne sur 8 directions. Question du jour : `Retours\Q-palissade` (valeurs provisoires, défaut A codé).
- **Détail** : [FortificationsInProgress.md](../FortificationsInProgress.md) F5, F8, F9, F10.

### 7. IA d'une autre faction : RÈGLES + BRANCHEMENT PARTIEL (lot S4)

- **DANS MAIN** : `HOLAIPowerSpend` et `HOLAIGarrison` écrits et testés ; **règle S4-a** (`HOLAIPowerCall.as`, rouge 4c412cb vu).
- **EN INTÉGRATION (integ-55)** : **S4-a branché** : `ThinkPowers` dans `HOLAIBrain.as` et `HOLFactionPowerServer::TryCastPower` commun au RPC et à l'IA ; `aiserver` PASS (73 s) et `aiduel` PASS (146 s), mais **le lancer de *Lumière sacrée* n'a pas été vu en partie** (duel de 120 s, paysans seuls, aucun blessé) : LOGIQUE SEULEMENT ; reste un essai avec combat (scénario `ai_sacred_light` de Glorfindel en cours).
- **SUR BRANCHE** (integ-25) : **S4-b** : `HOLAIGarrisonCall.as` (`Plan`) et test `Test_AIGarrisonsAttackedTower` verts, **pas branché au cerveau** : liaisons C++ du lot d'Elrond (`FHOLAIBuilding` garnison, `IssueAIGarrisonOrder`) construites mais non testées ; tests du lot et branchement de `Plan` dans `HOLAIBrain.as` en cours (ELROND-1123f).
- **Détail** : [LegionsInProgress.md](../LegionsInProgress.md) L12, [AubeInProgress.md](../AubeInProgress.md) A12.

## Questions ouvertes du jour (`Retours\`, aucune tranchée par l'utilisateur)

- `Q-sacred-light-values` : rayon et soin de *Lumière sacrée* (défaut A codé, provisoire).
- `Q-palissade` : valeurs de la palissade (défaut A codé, provisoire).
- `Q-spec-voies` : les unités de commanderie n'existent pas ; faut-il créer celles du palier 1 pour que les voies débloquent quelque chose (défaut A : déblocage d'unité, non codé).
- `Q-empreinte-jauge` : la jauge de faction n'est pas dans l'empreinte `HashUnits` (reco B colonne séparée, défaut C rien ; aucune détection de désynchronisation réseau aujourd'hui).
- `Q-suivi-mutuel` : suivi mutuel de deux unités (mineur 12 de REVUE-1122 ; reco C : le dernier ordre gagne).

## Déjà fait (ne pas refaire)

- Socle multi-faction L0 et L2 : rôles (`Factions.csv`), surcharges, colonne `Faction`, grilles filtrées, refus serveur ; technologies filtrées ; catégorie « Profane » (L3) ; faction lue par l'IA (G4).
- **Aube A1 (Hallebardier)** dans la Caserne, l'IA le connaît.
- **Aube A2 (Chevalier Vertueux, redirection passive 30 % / 8 m)** : combat, anneau et ligne de table ; essai `virtuous_knight` OK le 2026-11-21.
- **Dans `main` depuis le 2026-11-23 (47f6fe4)** : S1-a soin de zone *Lumière sacrée* (logique, pas encore lancé en jeu), T7 flèche de tour, règle S4-a, S1-c panneau cliquable (script), XP du duel 150 (logique ; essai en jeu à faire).
- Texte périmé de la ligne `LightSeal` de `UnitAbilities.csv` corrigé (sur ability-columns, en intégration).
- Relevé des effets des voies de spécialisation fait (lecture seule) : aucun effet en jeu.
- D860 (marque de signal et Alt + clic sur la mini-carte), D861 (engin démonté qui attaque : tests présents ; le reste du chantier deploy est D877).

## Règles de séance à garder

Celles de `C:\Users\baudo\Documents\MyCrusader\Auto_Journey.md` (test rouge d'abord sur la vraie chaîne, `Tools\build.ps1 -LiveCoding=0`, `quick_check.ps1 -Parts`, scénario pour tout essai demandé, valeurs proposées puis choisies par l'utilisateur (D345), commits en anglais, fusion locale à chaque passe verte (D888), **jamais de push**). Code neuf en Angelscript, C++ par exception avec sa raison (D839). Mettre à jour le dossier `*InProgress.md` concerné et `EtapeDefini.md` à chaque avancée.
