# REVUE-1123pm : relecture indépendante, lecture seule, du C++ de l'après-midi (2026-11-23)

Base fc41498, main 86a2e15. Aucun fichier de code touché, ni build ni test. Relu par diffs (14 appels, fin 16:49).
Bilan : **0 BLOQUANT, 5 GÊNANTS, 5 MINEURS**.

## BLOQUANTS
Aucun trouvé dans ce qui a été relu.

## GÊNANTS

### G1. Moral : état STATIQUE global jamais remis à zéro (table d'atténuation par camp)
- `Source/HeroesOfLiberty/Simulation/HOLMoraleProcessor.cpp:61` (branche socle-ai, a3de24e) : `int32 SlotAttenuationPercent[FHOLMatchConfig::MaxSlots] = {};`
- Défaut : la table est globale au processus, pas au monde. `hol.Morale.Attenuate 1 50` (ou un test qui plante avant sa remise à 0, `HOLMoraleRulesTests.cpp:1059-1061` remet à 0 seulement aux lignes 1078/1090/1156) reste actif pour la partie suivante et pour tous les autres mondes de test : les malus du camp 1 sont atténués de 50 % sans cause visible. La commande est marquée PROVISOIRE, mais rien ne la vide au `Deinitialize` / au démarrage d'une partie.
- Correctif : remettre la table à 0 au démarrage / à l'arrêt du processeur (ou la ranger dans un champ du processeur par monde), et enregistrer la commande en `ECVF_Cheat`.

### G2. Honneur : passage par réflexion sans contrôle de la disposition des paramètres
- `Source/HeroesOfLiberty/Simulation/HOLHealProcessor.cpp` (CreditHonorPoints, main) : `struct { int32 Slot; int32 ReturnValue; } Current{ Slot, 0 }; GameState->ProcessEvent(Getter, &Current);` (idem `Next` pour `SetFactionResource`).
- Défaut : le code suppose l'ordre, les types et la taille des paramètres de `GetFactionResource` / `SetFactionResource` (script). Si la signature change (int64, paramètre ajouté, ordre), `ProcessEvent` lit / écrit hors de la structure locale : corruption de pile silencieuse, pas d'erreur de build. Le coût par pas est faible (la réflexion ne passe que quand un point entier est gagné, pas à chaque pas : `CreditHonor` fait seulement un `GetGameState` et une boucle de MaxSlots sur le reste).
- Correctif : avant l'appel, `check(Getter->ParmsSize == sizeof(Current))` et `Setter->ParmsSize == sizeof(Next)` (ou `ensure` + sortie), ou rendre le compte du gauge accessible par une vraie fonction C++.

### G3. Honneur : valeurs statiques de la partie précédente, jamais coupées
- `HOLHealProcessor.cpp` (main) : `HonorHpPerPoint`, `HonorCap`, `HonorKillGain`, `HonorKillRadiusCm`, `HonorPendingHp` sont des `static` ; seul `ConfigureHonor` (appelé par `HOLZoneEffectLibrary.cpp:64`, donc par le script) les réécrit, et `Init(0., MaxSlots)` vide alors le reliquat.
- Défaut : rien ne les remet à 0 à la fin d'une partie ni à la destruction du monde. Après une partie de l'Aube, un monde de test suivant (ou une partie d'une autre faction dans le même processus) qui a un slot Aube crédite de l'Honneur avec les valeurs de l'ancienne partie, et le reliquat `HonorPendingHp` fuit d'une partie à l'autre tant que le script n'a pas rappelé `ConfigureHonor`. Non vérifié : à quel moment le script appelle `ConfigureHonor` (la recherche dans `*.as` n'a rien rendu ; seul l'appel `HOLZoneEffectLibrary.cpp:64` est vu).
- Correctif : `ConfigureHonor(0,0,0,0)` au `Deinitialize` du monde / à la fin de partie, ou vider les statiques dans le `Deinitialize` du sous-système d'unités.

### G4. Liaison `GetHeroLocation` : position d'une VUE interpolée, aucune garde d'autorité
- `Source/HeroesOfLiberty/Simulation/HOLHeroLibrary.cpp` (branche ability-columns, e809222) : `!Units->GetUnitView(Units->FindHeroUnit(Slot), View) || View.Health <= 0` puis `OutLocation = View.Location;`.
- Défaut 1 : `GetUnitView` appelle `MakeUnitView(..., Simulation->GetInterpolationAlpha(), ...)` (`HOLUnitSubsystem.cpp:1339`) : la position peut être la position interpolée de dessin (dépend de l'horloge murale), et c'est le point que l'IA prend pour planter sa bannière (`HOLAIBrain.as:173`). Si la vue interpole bien `Location`, deux exécutions identiques ne plantent pas au même point. Non vérifié : si `MakeUnitView` interpole `Location` (je n'ai pas lu ce corps).
- Défaut 2 : la doc dit « Server and solo », mais le code ne contrôle pas `NM_Client` (contrairement à `PlantHeroBanner` / `GetAuthorityHeroes`). Sur un client, la fonction rend ce que le sous-système d'unités du client contient ; non vérifié si le client y a le héros d'un camp caché par le brouillard. Le script qui l'appelle (`HOLFactionPowerServer.as:38`) est nommé « Server », donc le risque est latent, pas actif.
- Correctif : lire la position du pas (fragment de transformation, pas la vue) et ajouter `World->GetNetMode() != NM_Client` comme dans `GetAuthorityHeroes`.

### G5. Moral : règle en double, C++ et script, dans l'arbre
- `Source/HeroesOfLiberty/Simulation/HOLMoraleRules.cpp:4` (commentaire : « the script copy it replaces is Script/Morale/HOLMoraleRules.as ») ; `Script/Morale/HOLMoraleRules.as` existe toujours dans a3de24e, avec `HOLMoraleRules_Test.as`.
- Défaut : règle « jumeau C++ et script » interdite par CLAUDE.md. Le C++ est admis (processeur par entité), mais si le `.as` reste appelé par autre chose que ses tests, deux sources dérivent (par exemple `ClearMalusAndProtect` et le paramètre d'atténuation de `Apply` n'existent que côté C++). Non vérifié : qui appelle encore le `.as` hors tests.
- Correctif : supprimer ou rendre le `.as` simple appelant de `UHOLMoraleLibrary` (c'est le rôle des `Native*` déjà exposés) et retirer ses tests redondants.

## MINEURS

- M1. `HOLMoraleProcessor.cpp` : `PendingAround` / `PendingClear` sont statiques et ne se vident que pour un monde mort ou pour le monde qui exécute le processeur (`ExecutionFlags = Server | Standalone`). Si le script appelle `ApplyEffectAround` / `ClearMalusAround` sur un client (les fonctions de `UHOLMoraleLibrary` n'ont pas de garde d'autorité), les ordres s'y empilent à l'infini tant que le monde vit. Non vérifié : que le script les appelle côté client. Correctif : `if (World->GetNetMode() == NM_Client) return;` dans `QueueEffectAround` / `QueueClearMalusAround`.
- M2. `HOLCombatProcessor.cpp` : `TotalBuildingShotsLogged` (et `TotalHits`, etc.) est un `static int64` « depuis le lancement du programme » : jamais remis à 0 entre deux parties. Aucun défaut si les tests comparent un delta ; faux si un test lit la valeur absolue (`GetTotalBuildingShotsLogged`). Correctif : delta dans le test, ou remise à 0 au démarrage du monde.
- M3. `AwardKillHonor` (HOLCombatProcessor.cpp ~1536, main) : boucle sur tous les `StepBuildings` à CHAQUE mort dès que le script a installé rayon et gain > 0 (donc dans toute partie qui a une Aube, aussi pour les camps sans bâtiment de l'Aube). Coût O(bâtiments × morts par pas) borné par la mêlée ; à mesurer sur un grand combat. Aussi : `Near.Camp == Victim.Camp` ne teste pas les alliés de l'Aube (un mort d'un camp allié de l'Aube compte comme « ennemi » si les camps diffèrent). Non vérifié : si `Camp` regroupe les alliances.
- M4. `HOLTestWorldLibrary.h` (integ-25) : les UFUNCTION `SpawnTestHero` / `SetPlayerSpecialization` sont déclarées hors garde, mais leurs corps sont définis dans les deux configurations (`#if WITH_DEV_AUTOMATION_TESTS ... #else return 0/false`) : **pas d'erreur d'édition de liens en Shipping**, vérifié sur le diff. Reste un code de test (include de `HOLSpecializationView.h`, API de test publique) dans le module livré ; non vérifié : si le fichier entier est exclu du Shipping par le `.Build.cs`.
- M5. Journal des flèches de tour : `UE_LOG` avec arguments `Step.Id` (%u) et `Arrow.ArrivalStep - Arrow.ReleaseStep` (%d, caste en int32) : types de `Step.Id` non vérifiés contre `%u` (un `uint64` ou `int64` donnerait un journal faux, pas un plantage).

## VÉRIFIÉ SANS DÉFAUT

1. Liaisons du héros, autorité : `PlantHeroBanner` passe par `GetAuthorityHeroes(GetWorldOf(...))` (null sur un client) puis `UHOLHeroSubsystem::PlantBanner`, qui refuse déjà `NM_Client`, un slot hors tableau (`Sheets.IsValidIndex`), un héros mort / niveau / refroidissement / portée (`CanCast`, `CanPlantBanner`), et un héros en garnison (`GetUnitView` rend false si `Unit->Garrison.IsValid()`). Pointeurs nuls gardés (`Units`, `World`). Un client ne peut donc PAS planter une bannière.
2. Moral, combat : `HOLCombatProcessor.cpp` (2499-2501, 3289-3290) : `Fighter.Morale` testé nul ; le taux est borné à `Max(...,1)` avant la division et l'intervalle à `Max(1,...)` ; les plafonds (`BonusCap`/`MalusCap`, 30 à 50 %) gardent `Dealt` entre 60 et 140 et `Rate` entre 70 et 130 : pas de division par zéro, pas de dégâts négatifs.
3. Moral, ordre des processeurs : `UHOLMoraleProcessor` déclare `ExecuteBefore HOLCombatProcessor` (HOLMoraleProcessor.cpp:121) et vit dans le groupe Tasks : le combat lit le moral du même pas, pas un pas de retard. (L'ordre dans `DefaultGame.ini` met le moral APRÈS le combat, ligne 43 contre 41, mais c'est la contrainte `ExecuteBefore` qui prime, pas l'ordre du fichier.)
4. Moral, déterminisme : la file est drainée à l'ordre d'arrivée (parcours à l'envers + `Insert(...,0)`), verrou `FCriticalSection` correct, ordres hors monde éliminés par `TWeakObjectPtr`. Les ordres de nettoyage sont appliqués AVANT les sources (immunité vue par un malus posé au même pas). Aucun conteneur non trié n'est parcouru pour décider un résultat dans les lignes relues.
5. Flèche de tour : coût hors cvar nul. `UE_LOG` et tous ses arguments sont à l'intérieur de `if (LogBuildingShots != 0)` (HOLCombatProcessor.cpp ~971) ; un entier lu et comparé par flèche. `hol.Arrow.SlowMotion` ne touche que `Arrow.Duration` du `UHOLArrowSubsystem`, utilisé seulement pour l'alpha du dessin, le « gap » et le compte Landed (présentation) : la simulation (dégâts, pas d'arrivée `ArrivalStep`) est calculée côté combat avant et reste inchangée. `hol.Arrow.Scale` ne touche que l'échelle du dessin.
6. Honneur : plafond respecté : `FMath::Max(Courant, FMath::Min(Courant + Points, Cap))` ne baisse jamais une jauge au-dessus du plafond ; division par `HonorHpPerPoint` gardée par `> 0` ; `HonorPendingHp.IsValidIndex(Slot)` vérifié avant l'accès.
7. Contrôleur (8679ccd) : `hol.Mouse.LogDragEnd` : tout dans `if (CVar != 0)`, pointeurs `LocalPlayer` / `ViewportClient` / `Viewport` testés, aucun comportement changé ; mesure seule.
8. `.generated.h` reste le dernier include dans les en-têtes relus (HOLHeroLibrary.h, HOLSpecializationReadLibrary.h, HOLTestWorldLibrary.h). `HOLSpecializationReadLibrary.cpp` : monde nul et slot négatif rendent false ; `IsForumComplete` répète le même contrôle.
9. Aucun bouchon de test rouge vu dans les diffs de code livré relus.

## PAS RELU

- `HOLMoraleProcessor.cpp` : milieu du diff (lignes 279-300 : application des `Clears` par camp/rayon) lu seulement en fin de diff et par grep.
- `UHOLHeroSubsystem::PlantBanner` au-delà des 15 premières lignes ; `MakeUnitView` ; le corps de `HOLAIPowerCall::PlanBanner` et des scripts `.as` (AI, factions).
- Tous les fichiers de tests (HOLMoraleCombatTests, HOLZoneEffectTests, HOLBuildingShotTests, HOLBindingsLotTests, HOLGridCellStatusTests...), le C++ de `HeroesOfLibertyUI` (HOLGridCell, HOLGameScreen), `HOLZoneEffectLibrary.cpp`, le C++ des branches non listées.
- Réplication réseau réelle (aucun test multijoueur lancé) ; `HeroesOfLiberty.Build.cs` (Shipping) ; le moment d'appel de `ConfigureHonor` côté script.
