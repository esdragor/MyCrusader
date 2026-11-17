# GLORFINDEL-1117j - MIG-B-SWEEP (net-playtest, sans C++, sans build, lecture des deux cotes)

Base : net-playtest = main 8d8411a (`git merge main` : deja a jour). Enonce : INVENTAIRE-1117b.md, taches 1 (classe B, ~15 tests retirables).

## Resultat : 0 test C++ retire (le total quick_check ne bouge pas)

Chaque candidat a ete lu cote C++ et cote script, assertion par assertion. Regle : un test C++ n'est retire que si CHAQUE assertion a un equivalent script nomme ; sinon il est garde. Aucun candidat ne passe : l'inventaire comptait des fichiers « portes en grande partie », mais les parties non portees (que les en-tetes des `*_Test.as` disent elles-memes) sont des assertions sans jumeau. Comme rien n'est retire, aucun faussement n'a ete lance (pas de lancement quick_check, aucun fichier modifie hors docs).

## Registre : nom C++ -> test script, lignes supprimees, raison

| Test C++ (fichier) | Equivalent script | Retire ? | Lignes supprimees | Raison (assertions sans equivalent) |
|---|---|---|---|---|
| Siege.Technologies (HOLSiegeTests.cpp) | Test_SiegeTechnologies, Test_SiegeTechnologyTiersNotBeforeTheWorkshop | non | 0 | Table importee par CreateTableFromCSVString (non expose) ; Stats des lignes (`ReinforcedRam->Stats.Contains(RamHealth)`) et Age / Produces du batiment (`UHOLBuildingSettings::FindType`) : l'en-tete de HOLSiege_Test.as dit « not readable from script, stay in C++ » |
| Siege.TechnologyHealth | Test_SiegeTechnologyHealth, Test_SiegeHealthRounding | non | 0 | Le C++ prouve `HOLCombat::GetMaxHealth` / `GetDamage` (regle C++, lignes reelles `FHOLUnitTypeRow::ToConstants`) ; le script teste sa propre `HOLSiege::HealthWithTechnology` (copie script) : ce n'est pas la meme regle, l'en-tete de HOLSiege_Test.as le dit (« no script library, stays tested in C++ ») |
| Siege.Combat | Test_SiegeCombatVectors (IsInArea, TakesUnitsOn) | non | 0 | `HOLCombat::ChooseTarget` (Seeker / Candidate / ESeekMode) n'est pas expose ; ARWEN-1117a : « ChooseTarget reste C++ » |
| Siege.Table | Test_SiegeTableVectors | non | 0 | Le script ne lit que cout, temps, PV, vitesse, population, armures ; sans jumeau : Category, Age, BuildGridSlot, Footprint (3 x 3 tuiles), Produces, Walkable, Accepts, SightRadius de l'atelier ; BuildingAttackType / BuildingAttack / BuildingAttackInterval / BuildingAttackRange, AreaRadius (Belier, Mangonel, « seul le Mangonel a une zone »), SightRadius des deux machines, `HasArmorCategory(Siege)` des produits |
| Match.Stats.Rules (HOLMatchStatsTests.cpp) | Test_MatchStats_Sampling | non | 0 | Moitie `RecordUnitDeath / RecordUnitTrained / RecordBuildingLost / AddSample / RecordEvent` (FHOLMatchStats) non exposee (en-tete de HOLMatchStats_Test.as) |
| Match.Stats.Send | Test_MatchStats_CanReceive | non | 0 | Moitie `MakeChunk / MergeChunk / GetNextFromSample` (4 chunks, copie du serveur) non exposee |
| Match.Stats.Curves | Test_MatchStats_SeriesValues, Test_MatchStats_Axes | non | 0 | `MakeCurve` (points FVector2D) non expose |
| Net.FactionChoice.Bodies (HOLFactionChoiceTests.cpp) | aucun (Test_FactionChoiceList/Names/TripleClick portent d'autres tests) | non | 0 | `FHOLManagerClientRules::MakeMeBody / MakeAISlotBody / ParseLobby` (JSON du gestionnaire) : bibliotheque ManagerClient non exposee (E3-02 : « non porte ») |
| Market.Table (HOLMarketTableTests.cpp) | Test_MarketTableRows | non | 0 | Le C++ prouve l'import REEL `UDataTable::CreateTableFromCSVString` + `FHOLMarketRow::StaticStruct()` (correspondance colonne -> propriete par reflexion) ; le script relit le CSV avec son propre analyseur (`MarketRowSetColumn`) : l'import de l'editeur n'est pas verifie (E3-14 : « CreateTableFromCSVString non expose », garde par E3-FIN) |
| Map.Rules (HOLMapRulesTests.cpp) | Test_MapRulesByKey, Test_MapRulesByAsset, Test_MapRulesDisplayName | non | 0 | Partie « recherche » portee ; restent : `FTextInspector` (cle Map_Plain / Map_Steppe, espace HOLMaps, chaine source), `FText::IdenticalTo`, `IsCultureInvariant`, forme `FSoftObjectPath` (en-tete de HOLMapRules_Test.as) |
| Neutrals.Owner (HOLCampRulesTests.cpp) | Test_CampOwner (partie slots 0..7 -> camp, NeutralOwnerSlot = 8) | non | 0 | `FHOLMatchConfig::NeutralOwner / IsNeutralOwner / Normalize`, `HOLVictoryRules::AreHostile` (equipes, allies), `HOLCamps::IsPassive / NoteHit` : C++ sans copie script (ARWEN-1116m : « C++ sans copie script ») |
| Technologies.Rules (HOLTechnologyTableTests.cpp) | Test_TechnologyRulesVectors (+ Test_Technology* de HOLTechnologyRules_Test.as) | non | 0 | Seul le Check (14 vecteurs) est porte ; sans jumeau : `MakeCost` en element de file (600 pas), `HOLProduction::TryEnqueue / Cancel / Advance` avec recherches, `GetBonusPercent` (total d'une ligne, Plough / MinerPicks / Wheelbarrow / Blades / Empennage), `ScaleByPercent`, `GetAttackStat`, `GetArmorStat` (masque de categories), `ArmorCoversAttack`, `FHOLTechnologyState`, mot « soon » du journal (en-tete du test script : « no script twin, stays in C++ ») |
| Technologies.Playable (meme fichier) | aucun test script (Table + Playable portes en python : technology_table_test.py, CELEBORN-1117a) | non | 0 | Hors du champ du sweep (pas de jumeau script) ; non touche |
| Market.Rules (HOLMarketRulesTests.cpp) | Test_MarketRulesVectors (tous les chiffres de Sell / BuyWithGold / bornes) | non | 0 | Le C++ part du CSV reel importe + `HOLMarket::SettingsFromRow` (reglages lus de la ligne `Standard`) : le test script bati ses reglages a la main (`MarketSettings()`), la chaine reelle CSV -> reglages n'est pas verifiee en script ; la regle C++ `HOLMarket::Sell/BuyWithGold` y est aussi appelee (et non la copie script) |
| Hero.ReturnQuote (HOLHeroReturnTests.cpp) | Test_HeroReturnQuoteAlive/Waiting/Ready/NotBuyable/Unknown | non | 0 | Le C++ est une preuve de bout en bout sur monde : `UHOLHeroReturnSubsystem::QuoteNow`, `GetSlotGold` (lecture du pont, -1 pour une IA), un ordre reel qui prend le prix cote (76), `HandleForumFall` (plus de place) ; le script ne teste que le calcul pur `Compute` avec des entrees donnees a la main |
| Hero.ReturnOrderPrice | `HOLHeroReturn::PriceAt` dans Test_HeroReturnQuoteReady (76 comme vecteur) | non | 0 | Test de monde (simulation Mass : `W.OrderReturn`, `RunStepsTo`, or retire = 76, PV 450) : classe A, l'effet de l'ordre n'a pas de jumeau script |

Total : 15 tests C++ examines (4 Siege, 3 MatchStats, 1 FactionChoice, 1 MarketTable, 1 MapRules, 1 CampRules/Owner, 1 Technologies.Rules, 1 Market.Rules, 2 HeroReturn), 0 retire, 0 ligne supprimee, aucun fichier vide, aucun `git rm`. `Technologies.Playable` regarde pour memoire, non compte.

## Rouge / vert

Aucun faussement lance : rien n'est retire, donc rien a prouver. Rouges deja documentes qui couvriraient ces jumeaux quand leur C++ sera retirable (pour la suite) : Test_MapRulesByKey/ByAsset/DisplayName (HALDIR-1116q, 1363/1365), Test_MarketTableRows (SAURON-1116q, 1346/1349), Test_SiegeEngineTiers (ARWEN-1116p, 1345/1348), Test_CampOwner (ARWEN-1116m, 1326/1330), Test_MatchStats_* (ELROND-1117c, 1387/1395). Aucun rouge documente pour Test_TechnologyRulesVectors, Test_HeroReturnQuote*, Test_MarketRulesVectors, Test_SiegeTechnologies / Test_SiegeTechnologyHealth / Test_SiegeTableVectors, Test_SiegeCombatVectors (a faire si l'un de ces C++ devient retirable).

## Ce qu'il faut pour retirer ces 15 (a faire par un lot C++ de bibliotheque, pas par ce sweep)

1. Siege : exposer `HOLCombat::ChooseTarget` / `GetMaxHealth` / `GetDamage` (+ lecture de `FHOLUnitTypeRow` / `FHOLBuildingTypeRow` : Category, Age, Produces, Footprint, building-attack, AreaRadius, SightRadius, Stats d'une technologie). Ce lot servirait aussi Siege.Table et Siege.Technologies.
2. MatchStats : exposer `FHOLMatchStats` (Record*, AddSample, MakeChunk, MergeChunk, GetNextFromSample) et `MakeCurve` (sans FVector2D : deux tableaux).
3. Import CSV : exposer `UDataTable::CreateTableFromCSVString` (ou une fonction `ImportMarketTable` qui rend les lignes) : leve Market.Table, Market.Rules, Siege.Technologies et la ligne Technologies.
4. Technologies.Rules : exposer MakeCost / GetBonusPercent / ScaleByPercent / GetAttackStat / GetArmorStat / FHOLTechnologyState.
5. ManagerClient (JSON) : MakeMeBody / MakeAISlotBody / ParseLobby pour FactionChoice.Bodies (deja au plan MIG-C-ManagerClient).
6. Map.Rules : seule la verification FTextInspector manque ; elle peut aussi etre decidee morte (verifier le texte par un test d'impression : localisation, groupe UI).
7. Neutrals.Owner : exposer `FHOLMatchConfig::NeutralOwner/IsNeutralOwner`, `AreHostile`, `HOLCamps::IsPassive/NoteHit` (3 petites fonctions).
8. Hero.ReturnQuote / ReturnOrderPrice : classe A (monde), restent en C++ (D838).

Correction de l'inventaire : la classe B (« retirable maintenant sans C++ ») est en fait vide ; chaque ligne marquee B a une moitie sans jumeau. Les 9 fichiers passent en classe C (petite bibliotheque), ~2 bibliotheques (CSV + Siege/Technologies) en levent 8 sur 15.

Commande de build / verification (session principale, rien a recompiler ici) : aucun C++ ni script modifie, donc `Tools\Tests\quick_check.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -Parts tests` donne le meme total qu'avant ; pas de build.
