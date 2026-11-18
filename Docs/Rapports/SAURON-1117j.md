# SAURON-1117j : D844, la foret ne ralentit plus personne (branche visual, NON COMPILE)

- Decision D844 (2026-11-17) : F4 foret = 100 % pour toutes les categories ; gue F5 (120 %) et marais (60 %) inchanges.
- 7f789e0 (tests) : HOLTerrainTests.cpp (Terrain.Rules : 100/100, marais seul sous foret, 500 reste 500 ; Terrain.ForestSpeed : table pour pied, cavalerie, siege) ; HOLTerrainRules_Test.as (Test_TerrainForestSlowsNobody, ReadsTheCppRule, WalkerSpeedAtPoint).
- 9187d15 (code) : HOLTerrainRules.h DefaultForestCavalrySpeedPercent 100, DefaultForestSiegeSpeedPercent 100 ; commentaires (.h, ReadLibrary.h, HOLTerrainRules.as). Type Forest = 3, peinture, GetForestSpeedPercent, reglages gardes.
- Rouge : visible au build (les tests du commit 7f789e0 echouent sur l'ancien code, y compris les tests script qui lisent les defauts du C++).
- Non touche : blocs de formation (marais seul), Forest_Medium.json (peinture), Placeholders.txt (rien de provisoire ajoute).
