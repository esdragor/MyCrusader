# Auto_Journey_File.md : lignes faites archivees telles quelles (nettoyage 0a du 2026-11-24 soir, CLEAN-B-1124s, main = ed77a6e)

Chaque ligne est copiee sans modification ; le numero est sa ligne dans Auto_Journey_File.md avant le nettoyage. Critere : ligne [x] dont les commits sont ancetres de main (git merge-base --is-ancestor) et dont le FAIT est deja verifie dans main (PREP-1124b) ou dont la ligne le dit.

L20 :
- [x] (sans C++, 2026-11-21) virtuous_knight : `hol.Select.Hold` employé pour la première fois dans un scénario (a97f228) : vérifier qu'il tient la garde à chaque rejeu ; `Retours\E-virtuous_knight.md` | FAIT 4387164 (PREP-1124b 2026-11-24, verifie dans main : SAURON-1121a rejoue 4 fois, garde tenue)

L44 :
- [x] (D899, 2026-11-23 17:00, essai ai_garrison, reponse « OK mais je crois que la tour ne tirait qu'une fleche et pas (1 + nombre d'occupants) * fleche ») D899 volée de la tour garnie = (1 + occupants) flèches : verdict VERDICT-1123pa, test rouge d'abord (tour avec 3 occupants : 4 flèches par volée) ; journal FortificationsInProgress.md ; fiche archivée Docs\Archive\Retours_2026-11-23\E-ai_garrison.md | FAIT 565b88a (test 27f7476) (PREP-1124b 2026-11-24, verifie dans main : HOLCombatProcessor.cpp:844 GetBuildingArrowCount, test HOLBuildingShotTests.cpp:512)

L46 :
- [x] (sans C++, reste du DEFAUT DE RECETTE RECETTE-1123pa (b), Glorfindel) FAIT ARWEN-1124d 604a7b2 (integ-37) : scenario sacred_light_heal, waitlog sur les lignes exactes Heal report healed total 0.00 -> 705.00 (hero PV 180 -> 330 / 450), preflight PASS x2 ; hol.Units.Report n imprime pas les PV, hol.Heal.Report (total soigne, zone incluse) suffit, aucune cvar ; fiche Retours/E-sacred_light_heal.md

L50 :
   - [x] (S1-b, sans C++ ; GLORFINDEL-1123c 09:24 : test rouge Test_CastPaysHonorAndHealsNotSeal commite b2ea3aa sur ability-columns, rouge vu 2099/2102 ; GLORFINDEL-1123d 09:30 : correctif ECRIT (Decide TrySpend row.Cost + recharge row.CooldownMs + RefusalGaugeShort, RPC StartSacredLight + SetFactionResource, bEffectCoded = true, 4 tests du Sceau ajustes) ; verdict quick_check tests : Docs/Rapports/GLORFINDEL-1123d.md ; reste -Parts net + commit vert) RPC : ServerCastFactionPower (HOLRpcPlayer.as:1064) lance la ligne du livre (SacredLight) et paie l'Honneur, plus le Sceau (D862) ; -Parts net | FAIT b2a698c (test b2ea3aa) (PREP-1124b 2026-11-24, verifie dans main : HOLDawnPowers.as:22 bEffectCoded, HOLFactionPowerServer.as PutDown)

L51 :
    - [x] (S1-c, sans C++ ; ARWEN-1123a + 1123b 2026-11-23 : script FAIT, rouge ebdd211, vert 53e0fec, scenario faction_click JOUE (preflight PASS, vrai clic hol.Mouse.Click FactionBox0, journal FactionAim: refuse soon, 2 PNG vus) 37ae304 ; LOGIQUE SEULEMENT, pas de fiche E tant que S1-b absent ; variante aim 0 / cast 0 en commentaire de faction_click.txt) panneau cliquable : reste apres S1-b : activer la variante, jouer le tir au sol, fiche E | FAIT 37ae304 + e809222 (PREP-1124b 2026-11-24, verifie dans main : lancer au point vise dans main, faction_click_cast joue)

L53 :
  - [x] (S1-e, FARAMIR-1123g, arret 11:09) Honneur a la MORT : GDD D859 = ennemi tue a moins de 12 m d un BATIMENT de l Aube ; CreditKillHonor = CreditHonorPoints(World, Slot, HonorKillGain), AwardKillHonor sur StepBuildings, test Zone.KillHonor avec un centre de l Aube ; build lance 10:56 ; resultat dans Docs\Rapports\FARAMIR-1123g.md | FAIT 9442b74 (PREP-1124b 2026-11-24, verifie dans main : HOLCombatProcessor.cpp:1566 CreditKillHonor, test Zone.KillHonor HOLZoneEffectTests.cpp:443)

L54 :
      -> FARAMIR-1123h 11:18 : tests dd7b3ed 2099/2100, Zone.KillHonor ROUGE (0 mort en 200 pas avec le centre de l Aube a 10 m), merge main fait (a302195, C++ : build en file), branche NON prete ; Docs\Rapports\FARAMIR-1123h.md

L55 :
-> FARAMIR-1123pa 15:2x : test Zone.KillHonor reecrit ec74528 (unite disparue par id, cas lointain), build lance 15:13, tests a rejouer ; Docs\Rapports\FARAMIR-1123pa.md

L56 :
      -> FARAMIR-1123pb 15:3x : Zone.KillHonor rouge LU (enemy hp 1, Dawn spearman hp 80, 2 unit(s) alive = aucun coup en 10 s) : cause (a) mise en scene, id non multiple de 4 donc aucune recherche de cible ; correctif test 0fcf23d, code de jeu INCHANGE, NON COMPILE : build + quick_check -Parts tests a lancer ; Docs\Rapports\FARAMIR-1123pb.md

L64 :
- [x] (script, REVUE-1123pm genant 4 ; reste de GALADRIEL-1123pf) Script/Morale/HOLMoraleRules.as n est PAS un jumeau de la regle : 119 lignes, chaque corps = un appel HOLMorale::Native* (UHOLMoraleLibrary), aucune regle ; mais il a des appelants reels (HOLMoraleTable.as:177 MakeEffect, UI/HOLMoraleBlock.as:40 et :61 IsMalus) : rediriger ces 3 appels vers HOLMorale::NativeMakeEffect / HOLMorale::NativeIsMalus, verifier par grep qu aucun .as hors tests n appelle plus un nom du relais ni ses constantes Lever*/Apply*, puis git rm Script/Morale/HOLMoraleRules.as + Script/Tests/HOLMoraleRules_Test.as (le test ne couvre que le renvoi, la regle est couverte par HOLMoraleRulesTests.cpp) ; TYPES_A_EXPOSER : aucun (les Native* sont deja exposes) FAIT FARAMIR-1124c ed8cbc9 (zone-effect, jeu charge hidden ai_economy_start : 0 erreur Angelscript) : relais + HOLMoraleRules_Test.as retires, 3 appelants + 6 fichiers de test passes a HOLMorale::Native* (Apply avec 0 explicite, Count = State.Effects.Num()), constantes Lever*/Apply* dans Script/Tests/HOLMoraleCodes.as (tests seuls)

L71 :
  - [x] (S4-b, C++ : appelant) brancher `HOLAIGarrison` (D811) : ELROND-1123c, integ-25 : `AI\HOLAIGarrisonCall.as` (Plan) + test `Test_AIGarrisonsAttackedTower` ecrits (rouge vu 09:27, 2099/2100) ; reste : donnees du resume (emplacement, places, occupants, instants des coups de la tour) + ordre de garnison de l IA (TYPES_A_EXPOSER) puis appel dans HOLAIBrain.as ; `Docs\Rapports\ELROND-1123c.md` | FAIT 1c9571f (PREP-1124b 2026-11-24, verifie dans main : HOLAIBrain.as:86-94 HOLAIGarrisonCall::Issue, ai_garrison OK vu)

L107 :
- [x] Test_CGTables rouge avec TowerWood (integ-40, 2026-11-21) : case de grille 7 en conflit ? corriger la ligne CSV ou le test, puis réintégrer integ-25 (0eba386) ; ensuite Haldir wall-line 4599ab8 (régénérer BuildingTypes.uasset depuis le CSV fusionné) | périmée (très probablement, ETAPE0-1122 §3) : 4599ab8 ancêtre de integ-50, tests 2083/2083 verts ; non revérifiée en jeu | FAIT 4599ab8 (PREP-1124b 2026-11-24, verifie dans main : ancetre de main, tests 2190/2190 a integ-62)

L110 :
- [x] ELROND-1123pf/pg (integ-25, 2026-11-23 16:10) : SpawnTestHero corrige b72562b NON COMPILE (feuille du heros Alive apres un pas) : build + tests a lancer ; voir Docs\Rapports\ELROND-1123pg.md | FAIT b72562b (PREP-1124b 2026-11-24, verifie dans main : ancetre de main, tests 2190/2190)

L156 :
  - [x] (sans C++, NETTOYAGE-1123f 2026-11-23, bilan du matin) rejouer faction_click_cast au geste REEL (hol.Mouse.Click FactionBox0 puis clic sol) apres le correctif d origine CastOriginOf (GLORFINDEL-1123n, cdb1b3d, ability-columns) : RECETTE-1123a le donnait FAIL (cast refuse 4 RefusalOutOfRange, PNG faction_click_cast_after jamais pris) ; Docs\Rapports\RECETTE-1123a.md ; puis fiche E | FAIT e809222 (PREP-1124b 2026-11-24, verifie dans main : D897 dans main, essai joue par l utilisateur)

L217 :
- [x] (sans C++, OUTIL, NETTOYAGE-1123f 2026-11-23, bilan du matin) brief_commun.md : ajouter la consigne "la DERNIERE ligne du message de commit = `Agent: <NOM>`" (trailer) : sans elle le gate Close L19-1 donne un faux FAIL sur les commits d agents (vu 2026-11-23) ; la ligne [x] gate Close L19-1 (GALADRIEL-1123d, rouge 151ff1e, vert b265309, branche socle-ai) est archivee et disait "consigne NON faite" ; gate_test.ps1 entier a relancer | FAIT 7ae89ba (PREP-1124b 2026-11-24, verifie dans main : brief_commun.md item 9)

L233 :
- [x] (sans C++, rapport, L19-8) champ `TEST_ROUGE_SEPARE: oui/non` au rapport (agent_rules.md) ; « non » = tâche renvoyée. | FAIT 5d40153 (PREP-1124b 2026-11-24, verifie dans main : agent_rules.md:55 + brief_commun.md item 4)
