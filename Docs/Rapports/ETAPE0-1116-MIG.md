# ETAPE0-1116-MIG : plan de lancement de la séance de migration (2026-11-16 soir)

Étape 0, lecture seule du code (aucun build, aucun jeu lancé, aucun worktree modifié). Écrit à ~20:10.
Séance autonome, arrêt dur 23:00, fusion groupée sans push, Q2 = A (migration seule : aucune ligne de fonction lancée).
Sources : `Docs\Rapports\ANALYSE-1116a.md`, `Auto_Journey.md` (« Séance de migration »), `ChatInProgress.md`,
`AngelscriptInProgress.md`, `Auto_Journey_File.md`, `status.ps1` (19:56), `Docs\BuildQueue.csv`, Grep dans `Source\` et `Script\`.

## 1. Plan de lancement (une ligne par worktree)

| Worktree (branche) | Décision | C++ |
|---|---|---|
| Glorfindel (`net-playtest`) | **Lancer MIG-P1a** : pilote 1, tests du chat en script (garde, routage par alliances, observateur) | non pour P1a ; **oui pour P1b** (pont + suppression de `HOLChatRules.cpp/.h`, UN build) |
| Galadriel (`galadriel-tab`) | **Lancer MIG-P2a** : pilote 2, `HOLGameMenu` : relevé des liaisons, règles de l'écran et tests en script | non pour P2a et P2b ; **oui pour P2c** (ouverture par la classe du script + suppression de `HOLGameMenu.cpp/.h`, UN build) |
| Arwen (`d751-root`) | **Lancer MIG-J-Camp** : jumeau Camp, portage de `HOLCampRulesTests.cpp` (2 tests purs) en script | non |
| Celeborn (`hero-j8-close`) | **Lancer MIG-J-Relay** : jumeau Relay, portage de `HOLRelayRingTests.cpp` (39 lignes) et accord complété | non |
| Elrond (`tiers`) | **Lancer MIG-J-UnitTier** : jumeau UnitTier (le plus petit), accord étendu à toute la table | non ; proposé ensuite pour le lot C++ de fin de séance à 21:50 |
| Faramir (`fog`) | **Lancer MIG-J-Siege-a** sur `fog` tel quel, commits limités à `Script\` et à ses docs, fusion par cherry-pick (voir risque R2) | non |
| Haldir (`maps`) | **Lancer MIG-J-Terrain** : jumeau Terrain, accord étendu (forêt, gué, route) | non |
| Sauron (`visual`) | **Lancer MIG-J-ExplorationXP** : jumeau XP d'exploration, accord python vu rouge, relevé des appelants | non |

Lots suivants par worktree (ordre de la file) :

- Glorfindel : MIG-P1b (bascule, build) -> MIG-P1c (`quick_check -Parts tests,net`, mesure) -> MIG-P1d (RPC Server et Client en script, branche jetable `try-as-rpc`).
- Galadriel : MIG-P2b (écran en script) -> MIG-P2c (bascule, build) -> MIG-P2d (`HOLEndBanner`, même recette).
- Arwen : MIG-J-UnitAbility -> MIG-J-Diplomacy.
- Celeborn : MIG-J-Garrison -> (rien d'autre sans build : voir « Capacité »).
- Elrond : MIG-J-Technology -> MIG-CPP (lot C++ de fin de séance, si la session principale le lui confie).
- Faramir : MIG-J-Siege-b.
- Haldir : MIG-J-Site.
- Sauron : MIG-J-Deploy -> MIG-J-Market.

**Capacité, à dire franchement** : l'étape 2 offre ~15 lots sans build (13 jumeaux hors chat, deux coupés en deux). Pour 6 worktrees,
cela fait ~2,5 lots chacun, soit un travail épuisé vers 21:30. Au-delà, la valeur est dans les bascules C++ (limitées par les builds).
Je ne propose PAS de remplir le temps en créant de nouveaux jumeaux de l'étape 3 (règle pure réécrite en script pendant que le C++
appelle encore la sienne) : ce serait fabriquer la double source que le plan veut supprimer. Un worktree à sec reste libre.

## 2. Ordre de passage des builds

Au plus 2 agents C++ en même temps ce soir (plafond 3 respecté) ; les six lots de jumeaux portent `CPP: non`.

1. ~20:35 **Glorfindel, MIG-P1b** (chat) : `Tools\build.ps1 -LiveCoding=0 -Wait`, un seul build.
2. ~21:15 **Galadriel, MIG-P2c** (menu) : un seul build. Si P2b conclut que des liaisons manquent, pas de build : P2c devient une ligne du lot C++.
3. 21:45 **clôture des lots en script** (derniers commits, lignes de bascule complètes).
4. 21:50 **MIG-CPP + fusion groupée dans le même worktree, UN build `-Priority merge`** (proposition : Elrond) : fusion des branches, puis les bascules prêtes (un commit par bascule, pour pouvoir en retirer une), build, `quick_check -Parts tests,net`, avance rapide de `main`. Variante conforme à la lettre de la procédure : MIG-CPP sur sa branche (un build), puis la fusion (un build de plus).
5. ~22:20 `sync_worktrees.ps1` sans `-NoBuild` : **8 builds** (le chat et le menu changent le C++ de `main`, toutes les DLL deviennent périmées).

Mesure du jour à garder en tête (`Docs\BuildQueue.csv`, builds de 19 actions de la série 18:57-19:40) : 149, 189, 340, 516, 532 et 813 s ;
un build de 18 actions a duré 2 726 s à 18:11. Huit builds de synchronisation = 20 min au mieux, plus d'une heure au rythme lent.
Si le point 4 n'est pas fini à 22:20, la synchronisation dépasse 23:00.

## 3. Détail des lots

Règles communes (plan, parties 6 et 7) :
- Test en script d'abord. **Le rouge se voit par un bouchon qui compile** (fonction présente qui renvoie une valeur fausse), jamais par une
  fonction absente : un `.as` qui ne compile pas arrête l'éditeur, les tests et le serveur de tout le worktree.
- Test déjà vert à l'écriture (portage d'une règle existante) : fausser UNE valeur attendue, voir le rouge, la remettre (L12-5).
- Tant que deux copies existent : test d'accord par la bibliothèque existante, sinon **mêmes vecteurs que le test C++** (nom en regard dans le registre). Aucune bibliothèque neuve (ce serait un build).
- Aucun test C++ n'est supprimé par un agent de jumeau : la suppression fait partie de la bascule (lot C++).
- Registre des tests dans le rapport de l'agent (`Docs\Rapports\<AGENT>.md`, un fichier par agent, donc sans conflit) : tests C++ concernés, tests en script en regard, tests qui restent en C++ et pourquoi.
- **Le sens de chaque jumeau n'est pas tranché par l'agent** (voir R9) : il relève les appelants et écrit la ligne de bascule.
- Référence de départ : `quick_check -Parts tests` à l'entrée, rouges notés par NOM. `main` porte 2 rouges connus (`Match.StartingHero.NomadFirstForum`, `.NomadReplicated`, D831) et `quick_check` dit « REGRESSION » pour la partie `tests` à cause d'eux.

### Glorfindel — pilote 1, le chat

État vérifié : `Script\Chat\HOLChatRules.as` 118 lignes (le plan disait 77 : D806 et D807 y ont été ajoutés), `Script\Tests\HOLChatRules_Test.as`
114 lignes et 8 tests, `Source\HeroesOfLiberty\Match\HOLChatRules.cpp` 133 + `.h` 69 = 202, `Tests\HOLChatTransportTests.cpp` 129 lignes et
3 tests (`Chat.Route`, `Chat.Gate`, `Chat.ThreadLine`). Seul appelant C++ : `Player\HOLPlayerController.cpp` (13 emplois : commandes
`hol.Chat.Say` l. 1689-1714, cvars l. 5247-5253, `ServerPostChat_Implementation` l. 5261-5295). La partie `netsig3` de `quick_check.ps1` cite le chat 22 fois.

**Le test C++ `Chat.Observer` (rouge vu à 16:45)** :
- Il n'est pas dans les sources. Le commit 8610cee (dans `main`) ne contient que `Docs\Patches\D806_cpp_twin_observer.patch` et le journal ; `HOLChatTransportTests.cpp` porte 3 tests, sans `Observer`.
- La DLL de Glorfindel a été reconstruite à 19:23 (PASS, 149 s), après la remise des sources (16:45:47). La consigne « DLL à reconstruire avant tout quick_check » du journal est donc périmée. Non vérifié par un `quick_check` : c'est le premier geste de P1a (si `Chat.Observer` apparaît encore rouge, la DLL est périmée : build d'abord et le dire).
- **Ne pas appliquer le patch** : il corrige le jumeau C++ que P1b supprime. Ses 10 assertions deviennent le test en script `Test_ChatObserverBySeats` (P1a) ; son 3e morceau (`Viewer == this || Reaches(...)`, cas observateur / observateur retiré de `ServerPostChat`) est repris dans la bascule P1b ; le fichier du patch est retiré (`git rm`) dans le commit de P1b, et l'ancienne ligne « jumeau C++ HOLChatRules.cpp » de la file est close comme remplacée.

Trois écarts vus dans le code, à traiter dans le pilote :
- **Deux représentations du camp.** Le script prend un numéro d'équipe par siège (`TeamBySlot`) ; le serveur prend une matrice d'alliances mutuelles (`FSeats.Mates`, lue des relations répliquées par `HOLChat::ReadSeats`). En Diplomatie une alliance n'est pas transitive, un numéro d'équipe ne la décrit pas : le script doit recevoir la matrice.
- **La garde n'existe pas en script** : `CheckPost` (vide, longueur, cadence, canal, cible), ses réponses et ses défauts (200 caractères, 5 par 10 000 ms) ne sont qu'en C++.
- **`Chat.ThreadLine` ne teste pas `HOLChatRules`** : il teste `AHOLGameState::NoteChatReceived` et `HOLForumMessages`, par le délégué natif `OnSignalMessage` (invisible du script). Il reste en C++ ; « suppression des tests C++ du chat » = `Chat.Route` et `Chat.Gate`.

| Lot | Durée | Sous-étapes | Premier test rouge | Fichiers verrouillés |
|---|---|---|---|---|
| MIG-P1a (sans C++) | 25 min | (0) LaunchCheck, `quick_check -Parts tests` de référence. (1) Bouchon `HOLChat::CheckPost` (renvoie Ok) + constantes ; portage de `Chat.Gate` (5 refus + la cadence). (2) Bouchons `ReachesBySeats` / `RecipientsBySeats` (matrice `Mates` From * Slots + To, spectateurs, partie en équipes) ; portage des 9 assertions de `Chat.Route`. (3) Les 10 assertions du patch. (4) Registre. | `Test_ChatGateRefusals` contre le bouchon : « un caractère de trop » attend TooLong (2), reçoit Ok | `Script\Chat\HOLChatRules.as`, `Script\Tests\HOLChatRules_Test.as` |
| MIG-P1b (C++ : pont appelable du serveur + suppression du jumeau, UN build) | 30 min + build | (1) Pont abstrait neuf (modèle `HOLSignalSubsystem.h` l. 69-77 et `Script\Signals\HOLSignalBoardScript.as`) : événements `ChatReaches` et `ChatCheckPost`. (2) `ServerPostChat` appelle le pont ; `ReadSeats`, `ParseChannel`, cvars et commandes restent en C++ (déplacés hors de `HOLChatRules.cpp`). (3) Suppression de `HOLChatRules.cpp/.h`, de `Chat.Route` et `Chat.Gate` ; `Chat.ThreadLine` gardé (canaux en littéraux). (4) Build. (5) Sous-classe en script. (6) `git rm` du patch. | `Test_ChatBoardScriptAnswers` contre une sous-classe bouchon (répond faux), après le build | `Source\HeroesOfLiberty\Match\HOLChatSubsystem.h/.cpp` (neufs), `Match\HOLChatRules.cpp/.h`, `Player\HOLPlayerController.cpp/.h`, `Tests\HOLChatTransportTests.cpp`, `Script\Chat\HOLChatBoardScript.as` (neuf), `Docs\Patches\D806_cpp_twin_observer.patch` |
| MIG-P1c (sans C++) | 15 min | `quick_check -Parts tests,net` ; mesure : une valeur de la garde changée dans le `.as`, tests et `netsig3` relancés sans build ; lignes C++ supprimées comptées. | attentes de `netsig3` (un vaincu poste comme siège -1 : D806 lui retire le privé) | `Tools\Tests\quick_check.ps1` (partie `netsig3` seulement), `ChatInProgress.md` |
| MIG-P1d (sans C++, branche jetable `try-as-rpc`, jamais fusionnée) | 30 min | Un acteur ou composant en script, répliqué, possédé par le contrôleur ; une fonction `Server` et une `Client` ; commande console en script côté client ; 1 serveur + 1 client du harnais réseau. Réponse : oui ou non, lignes du journal du serveur et du client citées. | la ligne « reçu par le serveur » absente du journal avant l'écriture de la RPC | `Script\Dev\HOLScriptRpcProbe.as` (neuf, sur la branche jetable) |

### Galadriel — pilote 2, `HOLGameMenu`

État vérifié : `Source\HeroesOfLibertyUI\Screens\HOLGameMenu.cpp` 333 + `.h` 88 = 421 ; règles `Source\HeroesOfLiberty\Player\HOLGameMenuRules.cpp` 67
+ `.h` 88 (9 fonctions, 3 énumérations) ; `Tests\HOLGameMenuTests.cpp` 142 lignes, 7 tests purs (`Victory.GameMenu.Escape`, `.Surrender`,
`.QuitToMenu`, `.Input`, `.StatsButton`, `Victory.EndBanner.Which`, `.Closed`).

Deux points qui changent la recette :
- **`UHOLGameMenu` dérive de `UHOLScreen` (`UCommonActivatableWidget`), pas de `UHOLScriptScreen` (`UUserWidget`)** : activation, focus, mode de saisie. La recette J8a.5 (écran créé par `UHOLGameScreen` au-dessus de son cadre, `FindScriptClass`) ne s'applique pas telle quelle. P2a commence donc par le relevé des liaisons, et la liste des types à exposer est une sortie du pilote même si la bascule ne se fait pas ce soir.
- **Les règles du menu sont appelées par beaucoup de C++** : `HOLPlayerController.cpp` (2), `HOLGameHUD.cpp` (5), `HOLGameScreen.cpp` (5), `HOLEndBanner.cpp` (9), `HOLStatsScreen.cpp` (3), `HOLUIOpen.cpp` (3), `HOLManagerClientRules.cpp` (1). Elles ne peuvent pas disparaître du C++ ce soir. Le script ne voit pas le namespace C++ : l'écran en script a besoin d'une copie des seules fonctions qu'il emploie (5 sur 9 : `CanSurrender`, `CanQuitToMenu`, `OffersReturnToLobby`, `QuitsToList`, `ShowsStatsButton`). C'est un jumeau temporaire, créé par le pilote : à valider par la session principale (autre voie : une bibliothèque C++, donc le build d'abord). Les 7 tests C++ restent.

| Lot | Durée | Sous-étapes | Premier test rouge | Fichiers verrouillés |
|---|---|---|---|---|
| MIG-P2a (sans C++) | 25 min | (1) Lire `Screens\HOLScreen.h`, `HUD\HOLScriptScreen.h`, le début de `Script\UI\HOLGameScreenScript.as`, les appelants ; écrire la liste des liaisons. (2) Bouchons des 5 fonctions ; portage de `Surrender`, `QuitToMenu`, `StatsButton` sur les mêmes vecteurs. | `Test_GameMenuCanSurrender` contre le bouchon | `Script\UI\HOLGameMenuRules.as`, `Script\UI\HOLGameMenuRules_Test.as` (neufs) |
| MIG-P2b (sans C++) | 30 min | Écran en script (titre, Reprendre, Pause, Abandonner, Quitter, Retour au salon, Résultat), textes `NSLOCTEXT` ; test par `Describe` / `ClickWidget` (modèle `Script\UI\HOLSiteBanner_Test.as`). | `Test_GameMenuScriptButtons` (bouton Abandonner absent pour un vaincu) contre un écran vide | `Script\UI\HOLGameMenuScript.as`, `Script\UI\HOLGameMenuScript_Test.as` (neufs) |
| MIG-P2c (C++ : le HUD ouvre la classe du script + suppression de l'écran C++, UN build) | 30 min + build | Ouverture par la classe du script, suppression de `HOLGameMenu.cpp/.h`, `quick_check -Parts tests` puis un scénario qui ouvre le menu (Échap). Si P2b a trouvé des liaisons manquantes : pas de build, la ligne passe au lot C++. | le scénario d'ouverture du menu, rouge avant la bascule | `Source\HeroesOfLibertyUI\Screens\HOLGameMenu.cpp/.h`, `HUD\HOLGameHUD.cpp/.h`, `HUD\HOLGameScreen.cpp/.h`, `HOLUIOpen.cpp`, `Screens\HOLEndBanner.cpp`, `Screens\HOLStatsScreen.cpp` |
| MIG-P2d (sans C++ d'abord) | 30 min | `HOLEndBanner` (215 + 80 lignes, tests `Victory.EndBanner.Which` et `.Closed`), même recette. | `Test_EndBannerWhich` contre un bouchon | `Script\UI\HOLEndBannerScript.as` et son test (neufs) |

### Les six worktrees de jumeaux (aucun build)

Un jumeau = un seul worktree pour toute la séance (ses fichiers `.as`, son test en script). Chaque lot : (1) relevé des appelants C++ et du
sens proposé ; (2) tests portés ou accord étendu, rouge vu ; (3) registre ; (4) ligne de bascule dans « Lot C++ de fin de séance ».

| Lot | Worktree | État vérifié | Premier test rouge | Fichiers verrouillés |
|---|---|---|---|---|
| MIG-J-Camp | Arwen | `.as` 254 l. (8 tests), C++ 155 + 100, `HOLCampRulesTests.cpp` 135 l. pur, 2 tests (`Neutrals.CampRules`, `Neutrals.Owner`) ; aucun accord, aucune bibliothèque ; appelant `Simulation\HOLCampSubsystem.h` | `Test_CampOwner` (portage de `Neutrals.Owner`, sans équivalent parmi les 8 tests du script) | `Script\Neutrals\HOLCampRules.as`, `Script\Neutrals\HOLCampRules_Test.as` |
| MIG-J-UnitAbility | Arwen | `.as` 45 + table 54 (4 tests), C++ `HOLUnitAbilityRules.h` 45 l. seul ; appelants `HOLCombatProcessor.cpp` (4), `HOLUnitSubsystem.cpp` (4), `HOLGameScreen.cpp` (7) : appelé par Mass, le C++ reste ; `HOLDamnedWarriorTests.cpp` (Mass) reste | accord des constantes par un test python neuf (modèle `Tools\Tests\exploration_xp_accord_test.py`), une constante faussée une fois | `Script\Units\HOLUnitAbilityRules.as`, `Script\Tests\HOLUnitAbilityRules_Test.as`, `Tools\Tests\unit_ability_accord_test.py` (neuf) |
| MIG-J-Diplomacy | Arwen | `.as` 277 l. (14 tests), C++ 239 + 115, `HOLDiplomacyRulesTests.cpp` 162 l. pur, 1 test ; appelants nombreux dont `HOLHealProcessor.cpp` et `HOLVisionSubsystem.cpp` : le C++ reste | `Test_DiplomacyRulesVectors` (portage de `Diplomacy.Rules`) | `Script\Diplomacy\HOLDiplomacyRules.as`, `Script\Diplomacy\HOLDiplomacyRules_Test.as` |
| MIG-J-Relay | Celeborn | `.as` 114 l. (8 tests dont 2 d'accord), C++ 88 + 52, bibliothèque `HOLRelayLibrary` 81 + 71, `HOLRelayRingTests.cpp` 39 l. pur (`Relays.Ring`, emploie `HOLRelayView.h`) | `Test_RelayRing` si `FillPercent` / `MakeView` existent en script ; sinon un vecteur faussé dans `Test_RelayScriptAndSimulationRulesAgree` et `Relays.Ring` noté « reste en C++ » | `Script\Relays\HOLRelayRules.as`, `Script\Tests\HOLRelayRules_Test.as` |
| MIG-J-Garrison | Celeborn | `.as` 137 l., tests 4 + parité 2, C++ `HOLGarrisonRules.h` 80 l., bibliothèque `HOLGarrisonRuleLibrary` (2 fonctions : `AdmittedWith`, `AdmittedBySettings`) ; appelant `HOLGarrisonSubsystem.cpp` (3) | `Test_GarrisonScriptAndCppAdmitTheSame` étendu à chaque type d'unité, un vecteur faussé | `Script\Garrison\HOLGarrisonRules.as`, `Script\Tests\HOLGarrisonRules_Test.as`, `Script\Tests\HOLGarrisonParity_Test.as` |
| MIG-J-UnitTier | Elrond | `.as` 57 l. (6 tests dont 2 d'accord), C++ 35 + 30, bibliothèque 15 + 26 (`CheckUnit`, `GetUnitMinTier` : couvre les 2 fonctions C++) ; appelants `HOLProductionSubsystem.cpp` l. 446-448 et `HOLGameScreen.cpp` (4) ; `HOLUnitTierProductionTests.cpp` (monde) reste | `Test_UnitTierScriptAndSimulationRulesAgree` étendu à toute la table et aux paliers 0 à 3, une ligne de la table faussée | `Script\Units\HOLUnitTierRules.as`, `Script\Tests\HOLUnitTierRules_Test.as` |
| MIG-J-Technology | Elrond | `.as` 73 l. (8 tests dont 1 d'accord), C++ 185 + 131, bibliothèque `HOLTechnologyCheckLibrary` (`Check`), `HOLTechnologyTableTests.cpp` 444 l. pur, 3 tests ; appelé par `HOLCombatProcessor.cpp` (9) et `HOLGatherProcessor.cpp` (5) : le C++ reste | portage de `Technologies.Rules` par la bibliothèque (`Test_TechnologyRulesVectors`) | `Script\Technologies\HOLTechnologyRules.as`, `Script\Tests\HOLTechnologyRules_Test.as` |
| MIG-J-Siege-a | Faramir | `.as` 112 l. (7 tests), C++ 84 + 59, `HOLSiegeTests.cpp` 309 l. pur, 5 tests ; aucun accord script / C++ ; appelé par `HOLCombatProcessor.cpp` (3) : le C++ reste | `Test_SiegeTechnologyHealth` (portage de `Siege.TechnologyHealth`, sans équivalent en script) ; puis `Siege.Tiers`, `Siege.Technologies` | `Script\Siege\HOLSiegeRules.as`, `Script\Siege\HOLSiegeRules_Test.as` |
| MIG-J-Siege-b | Faramir | suite : `Siege.Combat`, `Siege.Table` ; `HOLTierCheatTests.cpp` (monde) reste | `Test_SiegeCombatVectors` | les mêmes |
| MIG-J-Terrain | Haldir | `.as` 151 l. (9 tests dont 1 d'accord), C++ 38 + 53, bibliothèque `HOLTerrainLibrary` 40 + 38 (5 fonctions) ; `HOLTerrainTests.cpp` (Mass) reste | `Test_TerrainServerAndScriptRulesAgree` étendu à la route, à la forêt et au gué. Si le rouge est RÉEL (le C++ n'a pas la forêt ni le gué, D810 non branché) : ne pas toucher au C++, noter l'écart, ne commiter que du vert | `Script\Terrain\HOLTerrainRules.as`, `Script\Terrain\HOLTerrainRules_Test.as` |
| MIG-J-Site | Haldir | `.as` 132 l. (14 tests dont 3 d'accord), C++ 185 + 86, bibliothèque `HOLSiteRulesLibrary` 86 + 54 (6 fonctions) ; appelant `HOLSiteSubsystem.h` | `Test_SiteScriptAndSimulationRulesAgree` étendu, un vecteur faussé | `Script\Sites\HOLSiteRules.as`, `Script\Tests\HOLSiteRules_Test.as` |
| MIG-J-ExplorationXP | Sauron | `.as` 126 l. (4 tests), C++ 87 + 52, accord python `Tools\Tests\exploration_xp_accord_test.py` (le plan ne l'avait pas vu) ; `HOLExplorationXPTests.cpp` (Mass) reste ; appelant `Map\HOLVisionSubsystem.h` (par case explorée : fréquence à relever) | l'accord python vu rouge avec un préréglage faussé dans le `.as`, puis remis ; vérifier qu'il est bien dans `quick_check` | `Script\Heroes\HOLExplorationXPRules.as`, `Script\Heroes\HOLExplorationXPRules_Test.as`, `Tools\Tests\exploration_xp_accord_test.py` |
| MIG-J-Deploy | Sauron | `.as` 252 l. (9 tests), C++ 157 + 116, `HOLDeployRulesTests.cpp` 295 l. pur, 9 tests ; aucun accord ; appelé par `HOLDeployProcessor`, `HOLCombatProcessor`, `HOLUnitOrderProcessor` : le C++ reste | registre nom à nom 9 contre 9 ; premier écart = `Test_DeployEveryCombination` sur les vecteurs de `Siege.Deploy.EveryCombination` | `Script\Siege\HOLDeployRules.as`, `Script\Tests\HOLDeployRules_Test.as` |
| MIG-J-Market | Sauron | `.as` 261 l. (16 tests), C++ 197 + 85 (namespaces `HOLMarket` et `HOLTribute`), `HOLMarketRulesTests.cpp` 472 l., 5 tests dont une part crée un monde ; aucun accord | portage de `Market.Rules` et `Market.Tribute` (parts pures) ; `Market.Apply`, `.BookState`, `.Chain` à classer | `Script\Economy\HOLMarketRules.as`, `Script\Economy\HOLMarketRules_Test.as` |

### Lot C++ de fin de séance (MIG-CPP)

Un agent, un worktree, un build (voir l'ordre des builds, point 4). Au plus 3 bascules : celles dont la ligne est complète à 21:45
(fichiers, appelants, accord vert, registre). Le reste devient le lot C++ d'ouverture de la prochaine séance (plan, partie 7, point 2).
Candidats déjà connus : la bascule du menu si P2c n'a pas eu son build ; les liaisons que les pilotes auront listées.
Hors migration, à décider par la session principale : les 2 rouges Nomad de D831 (`HOLStartingHeroTests.cpp`, ligne Celeborn de la file) tiendraient dans ce build.

## 4. Fichiers verrouillés hors des lots

- `Source\HeroesOfLiberty\Player\HOLPlayerController.cpp/.h` : Glorfindel seul.
- `Source\HeroesOfLibertyUI\HUD\HOLGameScreen.*`, `HOLGameHUD.*`, `Script\UI\HOLGameScreenScript.as` : Galadriel seule.
- `Tools\Tests\quick_check.ps1` : Glorfindel seul (attentes de `netsig3`). Un test d'accord python neuf se lance seul ce soir ; son branchement dans `quick_check` est une ligne pour l'agent de fusion.
- `Source\` en entier : interdit aux six agents de jumeaux.
- `Source\HeroesOfLiberty\Dev\HOLTestHooks.h`, `HOLTestSequence.cpp`, `Tests\HOLScenarioNoteTests.cpp` : WIP de `fog`, personne.
- `EtapeDefini.md`, `Auto_Journey_File.md` : chacun ses lignes. `Content\Maps\Crown.umap` (non suivi, Haldir) et les 16 fichiers `Tools\Scenarios\c28_*`, `c31_*` non suivis de Celeborn : ne pas ajouter.

## 5. Risques vus

- **R1, builds.** Voir les mesures du point 2. La charge de l'autre projet n'est pas mesurée ce soir. La fin de séance (fusion + 8 builds) est le chemin critique.
- **R2, WIP de `fog` (Faramir).** `fog` porte 2 commits hors `main` : de79413 (« WIP, not compiled » : `HOLTestHooks.h`, `HOLTestSequence.cpp`, `HOLScenarioNoteTests.cpp`) et 87999d5 (journal). Or le build de Faramir de 19:26-19:40 est PASS (813 s) sur ces sources : **le WIP compile**, contrairement à son étiquette. L'état de son test (`BackKeyReplaysOptionSetting`) n'est pas connu : il peut ajouter un rouge à la référence de Faramir. `fog` ne peut pas entrer dans la fusion groupée tant que ce WIP n'est pas vert : les commits de migration de Faramir (limités à `Script\` et aux docs) se fusionnent par cherry-pick. Ne pas changer de branche dans Faramir : les sources reviendraient à `main` sous une DLL construite avec le WIP.
- **R3, branches non fusionnées.** Les 7 autres branches sont à +0 de `main` : aucun conflit hérité. Les conflits de ce soir viendront des docs partagées (`EtapeDefini.md`, `Auto_Journey_File.md`).
- **R4, la file n'est pas dans les worktrees.** Les worktrees sont à ccbeff3, `main` à 20caec0 plus ce plan : les lignes MIG-* ne sont lisibles que dans la copie principale. L'énoncé cite l'identifiant du lot et `C:\Users\baudo\Documents\MyCrusader\Auto_Journey_File.md`.
- **R5, binaire.** Sept worktrees ont été reconstruits entre 18:57 et 19:40. Celeborn non : son dernier build est la fusion de 17:25-17:30. `Get-StaleBinaryProblem` n'a pas été lancé par l'étape 0 : `session_gate -Phase Launch -Root` le fait.
- **R6, un script cassé bloque son worktree** (éditeur, tests, serveur). D'où la règle du bouchon qui compile.
- **R7, le pilote 1 n'est pas une traduction** : la matrice d'alliances et la garde sont à écrire en script (voir plus haut). `Chat.ThreadLine` reste en C++. Tant que la sous-classe en script n'existe pas, le pont répond par son défaut : le serveur doit alors refuser, pas laisser passer.
- **R8, le pilote 2 sort de la recette** (`UHOLScreen` au lieu de `UHOLScriptScreen`) et crée un jumeau temporaire de 5 fonctions. Il est possible qu'il se termine sans bascule, avec la liste des liaisons pour seule sortie.
- **R9, le sens de chaque jumeau est une décision de conception.** Pour une règle appelée par un processeur Mass, le plan dit : le C++ reste et la copie en script disparaît (Deploy, Siege, Technology, UnitAbility, Diplomacy d'après les appelants relevés). Pour une petite règle appelée à basse fréquence par du C++ (UnitTier 65 lignes, Camp, Relay, Site, Garrison), faire appeler le script par un pont ajoute plus de C++ qu'il n'en retire tant que le socle de l'étape 1 n'existe pas. Les agents ne tranchent pas : ils relèvent et proposent. À décider avec l'utilisateur à la sortie de séance (réécriture de D483).
- **R10, lignes périmées de la file** : la ligne Glorfindel « jumeau C++ HOLChatRules.cpp » est remplacée par MIG-P1 et sa consigne de rebuild est périmée (DLL de 19:23) ; la ligne Galadriel D804 dit « a11ed2a ... pas dans main » alors que `galadriel-tab` est à +0 de `main`.

## 6. Chiffres du plan recoupés

- Jumeaux : les 14 existent des deux côtés. Tests d'accord trouvés pour Terrain, Garrison, Relay, Site, Technology, UnitTier (et Talent) : confirmé. ExplorationXP a un accord python que le plan n'avait pas vu. Rien pour Chat, Diplomacy, Market, Camp, UnitAbility, Deploy ; celui de Siege compare deux fonctions du script entre elles.
- Tests : 728 `Test_` dans 88 fichiers `*_Test.as` (plan : 725) ; 600 tests C++ (plan : 598).
- Chat, menu, bannière : lignes confirmées (seul écart : `HOLChatRules.as` 118 lignes, pas 77).
- Parmi les tests C++ des jumeaux, ceux qui créent un monde ou touchent Mass restent en C++ ce soir : `HOLExplorationXPTests`, `HOLTerrainTests`, `HOLUnitTierProductionTests`, `HOLDamnedWarriorTests`, `HOLTierCheatTests`, une part de `HOLMarketRulesTests`.

## Ce qui n'a pas été vérifié

- Aucun `quick_check` lancé : la référence de chaque worktree (dont l'absence de `Chat.Observer` dans la DLL de Glorfindel et l'état du test du WIP de `fog`) est à prendre par le premier agent.
- `Get-StaleBinaryProblem` et `session_gate` non lancés.
- Le contenu des tests C++ à porter n'a pas été lu (noms, lignes et présence d'un monde seulement), sauf `HOLChatTransportTests.cpp` et le patch. Un test « pur » peut employer un type que le script ne voit pas.
- La classification « appelé par Mass » vient des noms des fichiers appelants, pas de la lecture des appels.
- Que le script puisse dériver de `UHOLScreen`, et que les RPC en script passent avec Iris : ce sont les questions des pilotes.
- Les durées de lot (15-30 min) et les heures de build sont des estimations.
