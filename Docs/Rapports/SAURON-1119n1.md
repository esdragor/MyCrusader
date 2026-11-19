# SAURON-1119n1 : plan pour le Hallebardier (A1), à dérouler après la réponse à `Retours\Q-hallebardier-valeurs.md`

Docs seulement : rien n'est codé. Tout numéro de ligne est relevé le 2026-11-19 ; relire avant d'éditer. Les chiffres viennent de `Tools\Data\UnitTypes.csv` (16 lignes de données + l'en-tête, `Spearman` en ligne 7 du fichier).

## Constats utiles lus dans le code

- Le bonus d'une attaque est une valeur ajoutée par coup (`Source\HeroesOfLiberty\Simulation\HOLUnitTypes.h:88`, `HOLCombatRules.h:31`) : Lancier = 8 + 17 contre `Cavalry`, armure de la cible retranchée, 1 au minimum.
- Les rôles de `Factions.csv` sont Worker, Hero, MainCentre, Holy seulement (`Source\HeroesOfLiberty\Match\HOLFactionRules.h:28-35`, message d'erreur `HOLFactionRules.cpp:289`) : le « rôle Lancier » écrit dans A1 n'existe pas, et l'ajouter est du C++ (lot C++ de la séance). L'autre voie, **sans C++** : la colonne `Faction` de l'unité, déjà utilisée par l'Arquebusier (`Dawn`) et le Guerrier damné (`Legions`). `HOLFactionRules::FilterProduces` (`HOLFactionRules.cpp:421`) retire de la grille de production tout type que la faction du joueur ne peut pas faire. Proposition : `Halberdier` avec `Faction` = `Dawn`.
- L'IA connaît les unités propres à une faction par `Data.CandidateFactions` (`Script\AI\HOLAIArmy.as:305`, saut du candidat aux lignes 40-42 et 615). `HOLAIOpening.as` et `HOLAINeeds.as` ne contiennent pas le mot `Spearman` (recherche faite) : A1 les cite à tort, seul `HOLAIArmy.as` doit changer.

## Étapes (15 min chacune)

### 1. Ligne de données (réponses 1 et 2 nécessaires)
- `Tools\Data\UnitTypes.csv` : ajouter la ligne `Halberdier` **à la fin** (ligne 18, après `Arquebusier`, pour ne pas décaler le test de l'étape 3). Copier la ligne `Spearman` (ligne 7) et changer : `Name`/`DisplayName` = `Halberdier` ; `MaxHealth`, `Speed`, `Cost`, `BuildTime` selon la réponse ; `Bonuses` = `"((Category=Cavalry,Damage=17),(Category=HeavyInfantry,Damage=<réponse>))"` ; `Faction` = `Dawn` ; `MinTier` = `0` (le GDD place Lancier / Hallebardier au palier 0, § 951). Maillage et animation : garder `SKM_Spearman` / `TP_Spearman` (placeholder).
- Si réponse 2 = A (remplace) : mettre la colonne `Faction` du `Spearman` (ligne 7) à `Legions` **seulement après** avoir vérifié (étape 2) que la faction `Common` n'est pas lue ailleurs pour le Lancier. Le coût Légions reste dans `Tools\Data\FactionOverrides.csv` lignes 2-3 (`Legions_Spearman_*`).
- `Tools\Data\BuildingTypes.csv` : la ligne `Barracks` (ligne 2), colonne `Produces`, devient `("Spearman","Halberdier","ManAtArms","DamnedWarrior")` ; aucune ligne par faction : le filtre `Faction` de l'étape ci-dessus fait la différence.

### 2. Vérifier le filtre et le Lancier des Légions (lecture, pas d'édition)
- Lire `GetTable().Allows(Row->Faction, key)` (`Source\HeroesOfLiberty\Match\HOLFactionRules.cpp`, autour des lignes 405-425) : confirmer ce que veut dire `Common` / `Legions` pour une unité (la valeur `Common` est lue par toutes les factions ; `Legions` seulement par les Légions).
- `Tools\Tests\legions_roster_test.py:21-24` fige `ROSTER` (dont `Spearman`) et `LEGIONS_HOUSES["Barracks"]` = `["Spearman", "ManAtArms", "DamnedWarrior"]` : le Lancier doit rester dans la liste des Légions ; la liste de l'Aube (s'il y en a une) doit gagner `Halberdier`. À lire pour décider quelles lignes changer.
- `Tools\Tests\camp_table_test.py:94` : le camp des Brigands garde `Spearman` ; la vérification ne porte que sur l'existence du type, sans filtre de faction (à confirmer).

### 3. Test qui fige le nombre de lignes
- `Script\Tests\HOLUnitTierRules_Test.as` : après la ligne 180 (`Expected.Add(3);` de l'Arquebusier), ajouter `Rows.Add("Halberdier"); Expected.Add(0);` ; ligne 181 : `16` -> `17` et le texte `"the 17 rows of the unit type table"` ; mettre aussi à jour les commentaires « 16 rows » des lignes 7 et 142.
- Test rouge d'abord, à écrire dans le même fichier ou le fichier voisin : le Hallebardier du palier 0 sort à son palier (déjà couvert par la boucle ligne 183-190), et un test de données : coût du Hallebardier > coût du Lancier ; dégâts contre un Homme d'armes (`HOLCombatRules::GetDamage`, `HOLCombatRules.h:44`) supérieurs à ceux du Lancier ; dégâts contre `LightCavalry` égaux à ceux du Lancier (test écrit en script, règle D839).
- `Tools\Data\unit_stats_test.py` et `Tools\Data\make_unit_stats.py` : relancer pour régénérer `UnitStats.md` (racine) ; vérifier que `unit_stats_test.py` ne fige pas un nombre de lignes (recherche faite sur « rows » : rien trouvé, à confirmer par lecture).

### 4. Régénérer l'asset `UnitTypes.uasset` (éditeur, par la session principale)
- Le script est `Tools\Editor\make_unit_types.py` (en-tête, lignes 1-7 : « Creates /Game/Units/UnitTypes ... from Tools/Data/UnitTypes.csv » ; il remplace toutes les lignes, la CSV reste la source ; l'éditeur se ferme à la fin). Commande type : `UnrealEditor.exe <projet> -ExecutePythonScript=<chemin absolu de make_unit_types.py>`. Aucun enveloppe `.ps1` dédié trouvé par recherche dans `Tools\` (à confirmer ; `Tools\Tests\quick_check.ps1:223` ne fait que lire la table).
- Regrouper avec les autres lots de la séance (une seule ouverture de l'éditeur, règle « grouper les builds »). Le fichier produit `Content\Units\UnitTypes.uasset` est binaire : un commit à part.
- Si une valeur de table `Value*` (feuille de valeurs `Tools\Data\ValueSheet.csv`, `make_value_sheet.py`, `value_consumers.py`) liste chaque unité : lancer `Tools\Data\value_sheet_test.py` et `value_consumers_test.py` ; ajouter la ligne si le test la réclame (`orphan_values_known.txt` ne doit pas grossir).

### 5. Grille de production (Caserne de l'Aube)
- Aucun code si l'étape 1 est faite (`FilterProduces`, `HOLFactionRules.cpp:421`). Test de grille : la grille de l'Aube propose `Halberdier` et jamais `Spearman`, celle des Légions l'inverse. Où écrire le test : `Script\Tests\` (script) ; calquer sur ce que fait `Tools\Tests\legions_roster_test.py:24` pour le Lancier des Légions.
- Textes fr / en / ja du nom « Hallebardier » : chercher le fichier de textes par `Grep` sur « Lancier » dans `Content\Localization` ou `Tools\` (non repéré lors de cette séance : NON VÉRIFIÉ).

### 6. IA (`Script\AI\HOLAIArmy.as`)
- Si réponse 2 = A : ligne 159 `Data.Types.Add(n"Spearman")` : ajouter `Data.Types.Add(n"Halberdier")` à la suite (ligne 160), pour que la règle des contres compte le Hallebardier dans le rapport de puissance ; `Spearman` reste (les Légions s'en servent). Ligne 166-168 : ajouter un candidat `Halberdier` à la Caserne avec la part 2 (comme le Lancier), puis dans la boucle ligne 304-306 : `Spearman` -> `Legions`, `Halberdier` -> `Dawn` (même mécanisme que l'Arquebusier et le Guerrier damné). Sans cela l'IA de l'Aube planifierait un Lancier que le serveur refuse en boucle (commentaire ligne 39-42).
- Si réponse 2 = B : un candidat de plus (`Halberdier`, `Barracks`, part à choisir, 1 sur 8), `Dawn`, et pas dans `Types` si on veut le laisser hors de la règle des contres, comme le Guerrier damné (commentaire ligne 297-298).
- Tests de l'IA à relire ou ajuster : `Script\AI\HOLAIArmyPlan_Test.as:255, 347, 412, 552` (« la Caserne produit un Lancier », fondés sur `Faction` = `Dawn` par défaut, `HOLAIArmy.as:42`), `HOLAIArmyRules_Test.as:159` (valeur du Lancier 60 + 20 = 80) ; ajouter : « l'IA de l'Aube produit des Hallebardiers contre la cavalerie » et « Aube contre Aube sans plantage » (A1, « Test rouge d'abord »).

### 7. Placeholder visuel et journaux
- `Placeholders.txt` ligne 20 (`- Lancier Aube [mannequin]`) : si l'Aube perd le Lancier (réponse 2 = A), remplacer cette ligne par `- Hallebardier Aube (corps du Lancier, mannequin teinté, hallebarde absente) [mannequin]` et le noter dans « Remplacés » de `PlaceholdersInProgress.md` ; si réponse 2 = B, ajouter la ligne sans retirer l'ancienne. Relire le fichier juste avant d'éditer.
- Mettre à jour `EtapeDefini.md` (titres seulement) et `AubeInProgress.md` (A1 : valeurs choisies, ligne de données faite) en même temps ; `Retours\Q-hallebardier-valeurs.md` : renommer le titre `[À RÉPONDRE]` en `[RÉPONDU]` et noter la décision (numéro D à demander au GDD à la clôture).
- Scénario `dawn_halberd` (fin de A1) : séance suivante, après ces étapes.

## NON VÉRIFIÉ
- Le comportement exact de `Allows(Faction, FactionKey)` pour une unité `Common` marquée `Legions` (étape 2).
- Si `legions_roster_test.py`, `HOLAIArmyPlan_Test.as` et les tests de grille cassent quand `Spearman` devient `Legions` seulement (lire avant de choisir A).
- Le fichier des textes fr / en / ja et l'absence d'un `.ps1` autour de `make_unit_types.py`.
- Les temps de combat du Q-file sont calculés à la main (une seule cible, sans approche, sans techniques) ; aucun test n'a été lancé.
