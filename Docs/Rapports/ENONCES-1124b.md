# ENONCES-1124b : 10 enonces SANS C++ (PREP-1124b, 2026-11-24 11:2x, lecture seule du code)

Modele : `Auto_Journey.md` section « Modele d'enonce d'un agent » ; bloc commun : `Tools\AutoJourney\brief_commun.md`. Chaque enonce commence par `Tu es <AGENT> ... Lis C:\Users\baudo\Documents\MyCrusader\Tools\AutoJourney\brief_commun.md puis agent_rules.md et applique-les.` et donne l'arret dur `<HH:MM>` = Get-Date lu juste avant + 15 min. Tous : `session_gate.ps1 -Phase Launch -NoCpp`.

Ordre pris : `Docs\Priorite_Socle_Factions_2026-11-16.md` (lots 1 a 5), puis les `[ ]` d'`Auto_Journey_File.md` (Socle d'abord, puis Outils). Chaque ligne a ete confrontee a `main` (4547160) par `git merge-base --is-ancestor` et `git grep main`.

EXCLUS (pris aujourd'hui) : HOLWall* / grille fine (Celeborn), HOLHeroLibrary / GetHeroUnitId / D893 C++ (Glorfindel), HOLCombatProcessor / Rage / HOLMorale* C++ (Galadriel), HOLFactionAim* (Arwen), HOLZoneEffect (Arwen), D905 / HOLSpecializationSlot (Elrond).

Etat des worktrees (releve 11:2x, `git -C <wt> branch --show-current`, `rev-list`) : Faramir `zone-effect` (propre, 2 derriere main, libre depuis 11:1x) ; Arwen `integ-37` (13 fichiers modifies, libre) ; Elrond `integ-25` (10 modifies, libre) ; Haldir `integ-58` (10 modifies, 4 derriere main, libre) ; Sauron `integ-64` (38 modifies, 40 devant main, libre). Un worktree sale : l'agent ne commite QUE ses fichiers, par nom.

## Lignes de la file passees [x] (verifiees dans `main`)

| Ligne (avant) | Fait | Preuve |
|---|---|---|
| 14 virtuous_knight `hol.Select.Hold` | 4387164 | SAURON-1121a : rejoue 4 fois, garde tenue ; ancetre de main |
| 35 D899 volee de la tour = 1 + occupants | 565b88a (test 27f7476) | `HOLCombatProcessor.cpp:844` `GetBuildingArrowCount`, `:954`, test `HOLBuildingShotTests.cpp:512` |
| 41 S1-b RPC du pouvoir paie l'Honneur | b2a698c (test b2ea3aa) | `HOLDawnPowers.as:22` `bEffectCoded = true`, `HOLFactionPowerServer.as:115-133` `PutDown` |
| 42 S1-c panneau cliquable | 37ae304 | faction_click joue ; e809222 (lancer au point vise) dans main |
| 44 S1-e Honneur a la mort | 9442b74 | `HOLCombatProcessor.cpp:1566` `CreditKillHonor`, `HOLHealProcessor.cpp:123`, test `HOLZoneEffectTests.cpp:443` |
| 62 S4-b appel de garnison de l'IA | 1c9571f | `HOLAIBrain.as:86-94` `HOLAIGarrisonCall::Issue`, `HOLAIGarrisonCall.as`, `HOLAIGarrisonCall_Test.as`, `ai_garrison` OK vu par l'utilisateur |
| 98 Test_CGTables TowerWood | 4599ab8 | ancetre de main ; tests 2190/2190 a integ-62 |
| 101 ELROND-1123pf/pg SpawnTestHero | b72562b | ancetre de main ; tests 2190/2190 |
| 147 rejouer faction_click_cast au geste reel | e809222 | D897 dans main, essai joue par l'utilisateur (REGLAGE rendu, corrige) |
| 207 trailer `Agent:` dans brief_commun | 7ae89ba | `brief_commun.md` item 9 |
| 221 champ `TEST_ROUGE_SEPARE` | 5d40153 | `agent_rules.md:55`, `brief_commun.md` item 4 |

Lignes NON passees (doute) : voir le rapport final.

---

## F1 | Faramir (`zone-effect`) | verifier la banniere plantee par l'IA (lot 5, S4-a / D891)

Joueur : l'IA de l'Aube lance *Lumiere sacree* depuis SA banniere ; si la banniere est plantee au (0, 0) ou loin des blesses, le soin tombe a cote.

```
Tu es <AGENT> (verifier la position de la banniere plantee par l'IA de l'Aube, FARAMIR-1124d NON VERIFIE). Lis C:\Users\baudo\Documents\MyCrusader\Tools\AutoJourney\brief_commun.md puis agent_rules.md et applique-les.
ARRET DUR a <HH:MM> (Get-Date PowerShell lu juste avant ; 15 min ou 25 appels vus par toi, le premier atteint).
Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir (branche zone-effect, 8 commits d'avance, 2 derriere main : ne fusionne pas main) ; sans C++ (session_gate -Phase Launch -NoCpp). Lis la fin de ...\FortificationsInProgress.md et Docs\Rapports\FARAMIR-1124d.md (champ NON VERIFIE).
TACHE : le scenario Tools\Scenarios\ai_sacred_light.txt allume deja `hol.AI.LogPowers 1` (l.8) ; FARAMIR-1124d n'a PAS lu le journal. Lis en UN appel groupe : ai_sacred_light.txt (30 lignes), Script\AI\HOLAIBrain.as:161-209 (le plant : `HOLHero::PlantHeroBanner` l.173, ligne de journal « AI slot N step S: banner at (X, Y) planted|refused » l.181 ; la ligne « powers think ... banner (X, Y), nearest wounded D cm ... » l.204 ; origine du cast `CastOriginOf` l.184), Script\AI\HOLAIPowerCall.as:100-147 (PlanBanner : centre des blesses ramene a la portee du heros), Script\Factions\HOLFactionPowerServer.as:11-39 (CastOriginOf, HeroPlaceOf). PREMIER TEST ROUGE : dans ai_sacred_light.txt, avant `waitlog Sacred Light cast` (l.27 d'apres mon decompte, relis le fichier), ajoute `waitlog banner at (` puis `waitlog planted` ; lance `play.ps1 ai_sacred_light -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` (un appel) : ROUGE = le scenario n'atteint pas « planted » (« refused » = le heros n'a pas le niveau ou la banniere est en recharge, ou position du heros inconnue). Puis assertion sur les DEUX lignes de journal : la banniere n'est pas (0, 0) et `nearest wounded` <= 1200 cm (portee `HOLDawnPowers::SacredLightRangeCm`). Premier Edit (ai_sacred_light.txt) AVANT le 8e appel (L20-8).
Ne corrige pas HOLHero::GetHeroLocation (C++, Glorfindel, abe5629 : REVUE-1123pm genant 3) : si la position vient d'une vue interpolee, ecris-le au rapport (TYPES_A_EXPOSER / lot C++). Ouvre chaque PNG du .look et dis ce que tu vois (zone doree sur les blesses ? banniere visible ?).
Fichiers deja pris par un autre agent de la vague : HOLFactionAim*, HOLZoneEffect (Arwen), HOLHeroLibrary (Glorfindel), HOLCombatProcessor / HOLMorale* C++ (Galadriel).
```

## F2 | Faramir (`zone-effect`) | Lumiere sacree appelle ClearMalusAround (lot 3, S2-c)

Joueur : la Lumiere sacree retire le moral negatif des soldats dans la zone et les protege 5 s (D404) ; aujourd'hui elle ne fait que soigner.

```
Tu es <AGENT> (Lumiere sacree retire le moral negatif de la zone, D404). Lis ...\brief_commun.md puis agent_rules.md et applique-les.
ARRET DUR a <HH:MM> (...).
Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir (branche zone-effect ; FARAMIR-1124c vient d'y retirer le relais de HOLMoraleRules.as : relis-le AVANT) ; sans C++. A LANCER APRES F1 et AVANT F3 (meme fichier). Lis Docs\Rapports\PLAN-MORAL-1120.md etape 8 et MoraleInProgress.md.
TACHE : ligne S2-c « script, apres integration ». ClearMalusAround existe en C++ et est un UFUNCTION (Source\HeroesOfLiberty\Simulation\HOLMoraleLibrary.h:139 : `UHOLMoraleLibrary::ClearMalusAround(World, CenterXCm, CenterYCm, RadiusCm, OrderSlot)`, queue l'ordre, le processeur le pose au pas suivant ; tests C++ verts HOLMoraleRulesTests.cpp:1103-1230) ; AUCUN appel dans Script\ (git grep main). Point d'appel unique : Script\Factions\HOLFactionPowerServer.as `PutDown` l.115-133, juste apres `HOLZoneEffect::StartSacredLight(Slot, Pose)` l.123-127 (joueur ET IA passent par la, l.69 et l.111). Rayon = HOLZoneEffect::GetSacredLightRadiusCm() (l.77), centre = Pose, OrderSlot = Slot. Lis d'abord en UN appel : HOLFactionPowerServer.as, Script\Morale\HOLMoraleRules.as:1-40 (namespace HOLMorale, wrappers Native*), les tests de Script\Factions\ (git grep -l PutDown).
PREMIER TEST ROUGE : regle pure `HOLFactionPowerServer::SacredLightClearOrder(Slot, Pose)` -> struct {CenterXCm, CenterYCm, RadiusCm, OrderSlot} ; bouchon qui compile (renvoie un ordre vide) + test `Test_SacredLightClearsMalusAtThePose` (rayon == GetSacredLightRadiusCm, centre == Pose, slot) dans le MEME commit, rouge vu a l'execution ; DEUXIEME commit : le corps + l'appel dans PutDown apres StartSacredLight (un refus de StartSacredLight n'appelle rien). Premier Edit (le test) avant le 8e appel.
Ne tranche aucun design : si le Camp attendu n'est pas `Slot` (equipe ?), Retours\Q-<nom>.md [A REPONDRE] avec defaut Slot.
Fichiers deja pris : HOLFactionAim*, HOLZoneEffect (Arwen) ; HOLMorale* C++ (Galadriel) ; F3 touche le meme HOLFactionPowerServer.as : ne les lance PAS en meme temps.
```

## F3 | Faramir (`zone-effect`) | D900 : le heros s'approche hors de portee (lot 1, D900)

Joueur : viser Lumiere sacree trop loin ne donne plus un refus muet : le heros marche jusqu'a 12 m du point puis lance.

```
Tu es <AGENT> (D900 : Lumiere sacree hors de portee = le heros s'approche puis lance). Lis ...\brief_commun.md puis agent_rules.md et applique-les.
ARRET DUR a <HH:MM> (...).
Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir (zone-effect) ; sans C++ (si l'ordre de marche exige une liaison : TYPES_A_EXPOSER au rapport, pas de C++). A LANCER APRES F2. Lis AubeInProgress.md (fin) et Docs\Archive\Retours_2026-11-23\Q-lumiere-sacree-visee.md.
TACHE : ligne D900 (reponse C, PAS la recommandation A). Lis en UN appel : Script\Factions\HOLFactionPowerServer.as:42-70 (`TryCastPower` : DecideAimed l.62-63 ; l.69 pose la zone au point), Script\Factions\HOLFactionPowerCast.as:68-72, :110 et :146 (`DecideAimed`, refus `RefusalOutOfRange`), Script\Commands\HOLPlayerOrders.as:24 (`Move` -> `HOLCommandScript::IssueMovePlayer`), Script\Net\HOLRpcPlayer.as (`ServerCastFactionPower`, ~l.1064 : verifie la ligne), les tests de DecideAimed (Script\Tests\HOLFactionPowerAimed_Test.as). PREMIER TEST ROUGE : point vise a 2000 cm du heros -> `DecideAimed` rend un resultat « marcher d'abord » (nouveau champ `bApproach` + `ApproachPoint` a 1200 cm = HOLDawnPowers::SacredLightRangeCm du point, sur la droite heros -> point) et PAS un refus ; bouchon qui compile + test dans le meme commit, rouge vu ; 2e commit : le correctif ; 3e : le serveur emet l'ordre de marche (HOLPlayerOrders::Move) et memorise le lancer en attente (zone AU POINT CLIQUE, Honneur depense UNE seule fois a l'arrivee). Premier Edit avant le 8e appel.
Ne tranche pas : moment du paiement (defaut code : a l'arrivee, rien avant) et annulation (defaut : tout nouvel ordre du heros annule le lancer en attente) -> Retours\Q-lumiere-sacree-approche.md [A REPONDRE] (A/B/C, recommandation, defaut code), 1 question groupee.
Fichiers deja pris : HOLFactionAim.as (aperçu de visée, Arwen : NE LE TOUCHE PAS ; le client envoie deja le point) ; HOLZoneEffect (Arwen) ; F2 (HOLFactionPowerServer.as l.115-133).
```

## F4 | Elrond (`integ-25`) | D877 engin demonte + cible lointaine : scenario puis diagnostic (lot 5 / ligne BUG D877)

Joueur : un Mangonneau demonte envoye loin marche mais ne se monte pas et ne tire pas.

```
Tu es <AGENT> (BUG D877 moitie A : engin demonte + clic lointain, scenario deploy_far). Lis ...\brief_commun.md puis agent_rules.md et applique-les.
ARRET DUR a <HH:MM> (...).
Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond (branche integ-25 ; ELROND-1124b libre a 11:08) ; sans C++ (le correctif C++ eventuel va au lot C++ de la seance : tu ne le compiles pas). Lis la fin de Palier3InProgress.md et Docs\Rapports\RECETTE-1121se.md.
TACHE : ligne « BUG D877 moitie A TOUJOURS EN ECHEC » (RECETTE-1121se, main 1276d54 : cc7c56e / ef6cbf2 ne couvrent que la marche coupee). Lis en UN appel : Tools\Scenarios\deploy.txt (183 lignes ; le cas monte + clic droit l.69-100 avec `hol.Select.RightClickGround -13400 -1000` l.86 et `waitlog Scripted right click at` l.87 ; la fin du fichier), deploy.look, et `git grep -n "Deploy:" main -- Source Script` (la ligne de journal attendue). PREMIER TEST ROUGE : ajoute a la fin de deploy.txt (ou nouveau scenario deploy_far.txt, preflight d'abord) le cas DEMONTE : Mangonneau demonte (sans D), `hol.Select.RightClickGround` vers un point a plus de 2 x la portee de tir (valeur lue dans BuildingTypes / UnitTypes), `waitlog Deploy:` + `waitlog D=Pack` avec `hol.HUD.Report` ; capture deploy_far_arrived ; ROUGE attendu = timeout sur « Deploy: » (aucune ligne). Premier Edit avant le 8e appel. Puis diagnostic en lecture : ApplyCombatOrder (HOLCommandProcessor / HOLUnit... ~:3200 d'apres ELROND-1122e) : pourquoi l'arrivee ne declenche pas le montage ; rapporte la cause + le correctif propose (TYPES_A_EXPOSER) sans le coder si c'est du C++.
Fichiers deja pris : D905 / HOLSpecializationSlot (Elrond, fait 11:08) ; scenario deploy.txt : aucun autre agent.
```

## F5 | Arwen (`integ-37`) | journal et scenario du moral du duel : Triomphe / Demoralise (lot 3 et 5, S2-b)

Joueur : apres un duel gagne, ton camp doit sentir le Triomphe et l'autre le Demoralise ; rien de cela n'a jamais ete vu a l'ecran.

```
Tu es <AGENT> (rendre visible au journal le moral pose par un duel : Triomphe, Demoralise, Hesitation). Lis ...\brief_commun.md puis agent_rules.md et applique-les.
ARRET DUR a <HH:MM> (...).
Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen (integ-37, ARWEN-1124d libre) ; sans C++ (aucune commande console C++ neuve : hol.Morale.Report est du lot C++ de Galadriel, ne l'ecris pas). Lis DuelInProgress.md (D7) et MoraleInProgress.md.
TACHE : Script\Duel\HOLDuelLedger.as `ApplyMoraleOrders` l.113-120 appelle `HOLMorale::ApplyEffectAround` (l.118) sans AUCUNE trace au journal ; tests existants Script\Duel\HOLDuelLedger_Test.as:17 (`Test_LedgerDrainLaysTriumphAndDemoralisedAtTheCentre`) et :56 (`Test_LedgerPlacesHesitationAtTheRefusersPosition`). Lis les deux fichiers + Tools\Scenarios\duel_challenge_accept.txt (fin : `waitlog Duel rings: 1 |`, hol.Duel.ForceAnswer) en UN appel. PREMIER TEST ROUGE : fonction pure `HOLDuelRewards::DescribeMoraleOrder(const FHOLDuelMoraleOrder&)` -> `FString` « Duel morale: <effet> slot S at (X, Y) radius R ms D » ; bouchon qui compile (renvoie "") + test `Test_LedgerDescribesEachMoraleOrder` (Triumph gagnant, Demoralised perdant, Hesitation refus) dans le meme commit, rouge vu ; 2e commit : le corps + `Log(n"LogHOLMatch", ...)` dans ApplyMoraleOrders pour chaque ordre pose. Puis scenario : cherche (grep `hol.Duel.` dans Source\) la commande qui TERMINE un duel et ajoute a la fin de duel_challenge_accept.txt `waitlog Duel morale: Triumph` (preflight). Premier Edit avant le 8e appel.
Fichiers deja pris : HOLMorale* C++ et HOLCombatProcessor (Galadriel), HOLFactionAim* et HOLZoneEffect (Arwen, autre tache : ne touche pas Script\Factions\).
```

## F6 | Haldir (`integ-58`) | le gate Essay refuse une commande abregee dans une fiche E-* (outil, AX-9)

Joueur : l'utilisateur recoit toujours une commande `play.ps1` complete a copier.

```
Tu es <AGENT> (session_gate -Phase Essay refuse une commande abregee dans une fiche E-*). Lis ...\brief_commun.md puis agent_rules.md et applique-les.
ARRET DUR a <HH:MM> (...).
Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir (branche integ-58, 4 derriere main, 10 fichiers modifies : ne commite que les tiens) ; sans C++. Lis la ligne « (sans C++, enonce + outil, AX-9) » d'Auto_Journey_File.md et Tools\AutoJourney\E_template.md.
TACHE : Tools\AutoJourney\session_gate.ps1 phase Essay (l.268-340 ; resolution de la fiche par nom de scenario l.287-297, FAIL l.297) : ajoute un controle de fond : une fiche `Retours\E-*.md` dont la ligne de commande contient « meme commande avec », « même commande avec », « idem » ou « comme ci-dessus » SANS `play.ps1 <nom>` complet -> FAIL « commande abregee ». Tests : Tools\AutoJourney\gate_test.ps1:174-190 (modele : E-bad.md / E-good.md dans $Ret, `Check`). PREMIER TEST ROUGE : dans gate_test.ps1, apres la creation de E-good.md (l.184), ajoute E-abbrev.md avec « même commande avec -TimeoutSec 60 » et `Check ($ess -match '(?m)^FAIL .*abreg')` ; `powershell -NoProfile -ExecutionPolicy Bypass -File Tools\AutoJourney\gate_test.ps1` rouge VU (compte), puis commit separe du correctif. GARDE LE BOM de session_gate.ps1 / gate_test.ps1 (brief item 5), analyse avec Parser::ParseFile apres edition. Premier Edit avant le 8e appel.
Fichiers deja pris : aucun ; F9 (Sauron) touche la phase Launch du meme session_gate.ps1 (zone differente, fusion sans conflit).
```

## F7 | Haldir (`integ-58`) | outil `replace_text.ps1` (L15-3, outils)

Joueur : aucun direct ; moins d'appels perdus par les agents sur « Edit refuse, not read ».

```
Tu es <AGENT> (outil replace_text.ps1 : remplacement exact et unique, BOM garde). Lis ...\brief_commun.md puis agent_rules.md et applique-les.
ARRET DUR a <HH:MM> (...).
Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir (integ-58) ; sans C++. A lancer APRES F6 (meme worktree sale : commits par nom). Lis la ligne « replace_text.ps1 (+ replace_text_test.ps1, rouge d'abord), agents_table.ps1 -Add / -Update, build_wait.ps1 » de la file (L15-3 a L15-6) : agents_table.ps1 -Add / -Update EXISTE deja (en-tete l.4-5) ; replace_text.ps1 et build_wait.ps1 n'existent pas (ls Tools\AutoJourney).
TACHE : seulement replace_text.ps1 (build_wait.ps1 = tache suivante). Lis en UN appel : brief_commun.md item 5 (la commande PowerShell inline a remplacer : compte exact == 1, BOM, chemin ABSOLU, Parser::ParseFile pour un .ps1), Tools\AutoJourney\edit_line.ps1 (en-tete d'usage a copier : syntaxe du parametre), agents_table_test.ps1 (modele de test). PREMIER TEST ROUGE : Tools\AutoJourney\replace_text_test.ps1 seul (script absent = rouge) : 5 cas : remplacement unique OK ; 0 occurrence -> code 1 sans ecriture ; 2 occurrences -> code 1 sans ecriture ; fichier a BOM garde ses 3 octets EF BB BF ; chemin relatif -> code 2. Commit 1 = le test (rouge vu), commit 2 = replace_text.ps1 `-File <abs> -Old <txt> -New <txt>`. Mets aussi a jour brief_commun.md item 5 : « ou `replace_text.ps1` » (un seul Edit). Premier Edit avant le 8e appel.
Fichiers deja pris : F6 (session_gate.ps1, gate_test.ps1) ; brief_commun.md item 5 : aucun autre agent.
```

## F8 | Sauron (`integ-64`) | outil : identifiants perimes de quick_check.ps1 avant chaque passe (L22-3)

Joueur : aucun direct ; evite les integrations ratees (quick_check attendait encore un libelle F9 / F10 d'avant les verts).

```
Tu es <AGENT> (outil stale_quickcheck.ps1 : quick_check attend un identifiant ou un libelle modifie par les verts). Lis ...\brief_commun.md puis agent_rules.md et applique-les.
ARRET DUR a <HH:MM> (...).
Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron (branche integ-64, 40 commits d'avance, 38 fichiers modifies : AUCUN commit hors de tes deux fichiers neufs, par nom) ; sans C++. Lis la ligne « (sans C++, OUTIL, L22-3 / L19-4 recidive, BILAN-1123pm) » de la file et Docs\Rapports\INTEG-64.txt.
TACHE : Tools\AutoJourney\stale_quickcheck.ps1 `-Root <worktree> -Base main` : (1) lit `git diff <Base>...HEAD -- Script Tools\Data Source` et extrait les identifiants ajoutes / retires (noms de lignes CSV, cles NSLOCTEXT, noms de tests Test_*, libelles entre guillemets) ; (2) cherche chacun dans Tools\quick_check.ps1 (grep, numero de ligne) ; (3) imprime `STALE: <identifiant retire> : quick_check.ps1:<l>` pour tout identifiant RETIRE ou RENOMME encore cite. Premier cas reel : integ-58 / l.4273 (attente F9 / F10) : lis cette ligne. PREMIER TEST ROUGE : Tools\AutoJourney\stale_quickcheck_test.ps1 (script absent = rouge) sur un depot jetable (repertoire du scratchpad, `git init`) : un identifiant retire et cite -> 1 ligne STALE ; un identifiant ajoute seulement -> aucune ; commit 1 = le test, commit 2 = l'outil. Ne lance PAS quick_check. Premier Edit avant le 8e appel.
Fichiers deja pris : aucun.
```

## F9 | Sauron (`integ-64`) | gate Launch : DLL en retard de plus de 5 commits Source (AX-2)

Joueur : aucun direct ; un agent « sans C++ » ne tombe plus sur un binaire perime.

```
Tu es <AGENT> (session_gate -Phase Launch signale une DLL en retard de plus de 5 commits Source). Lis ...\brief_commun.md puis agent_rules.md et applique-les.
ARRET DUR a <HH:MM> (...).
Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron (integ-64, sale : commits par nom) ; sans C++. A lancer APRES F8. Lis la ligne « (sans C++, enonce + outil, AX-2) » de la file.
TACHE : Tools\AutoJourney\session_gate.ps1, phase Launch (cherche `Phase -eq 'Launch'` avec Grep ; il lit deja `Get-StaleBinaryProblem` de Tools\Scenarios\LaunchCheck.ps1) : ajoute un controle : nombre de commits qui touchent Source\ entre le commit de la DLL du worktree (mtime de Binaries\Win64\*.dll -> `git log --before`) et HEAD ; > 5 -> WARN « DLL en retard de N commits Source : build d'abord ou pas de git merge main » (FAIL si la tache n'est pas -NoCpp). Tests : Tools\AutoJourney\gate_test.ps1 (modele : l.159-175, depot jetable, DLL « plus recente que le fichier »). PREMIER TEST ROUGE : cas 6 commits Source apres la DLL -> `^WARN .*retard` ; rouge vu (compte), commit separe du correctif. GARDE LE BOM, Parser::ParseFile apres edition. Premier Edit avant le 8e appel.
Fichiers deja pris : F6 (Haldir) edite la phase Essay du MEME session_gate.ps1 et gate_test.ps1 (zones differentes ; integrateur : fusion sans conflit attendue).
```

## F10 | Haldir (`integ-58`) | le « grand cercle pale » de faction_aim_free : identifier son origine (lot 1)

Joueur : sur le sol de la Lumiere sacree, un grand cercle pale d'origine inconnue brouille le dessin dore (vu par RECETTE-1123pc).

```
Tu es <AGENT> (identifier le grand cercle pale vu a l'essai faction_aim_free et le declarer en provisoire). Lis ...\brief_commun.md puis agent_rules.md et applique-les.
ARRET DUR a <HH:MM> (...).
Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir (integ-58) ; sans C++. A lancer APRES F6 et F7. Lis Docs\Rapports\RECETTE-1123pc.md (defauts vus) et Retours\E-faction_aim_free.md (ou Docs\Archive\Retours_2026-11-23\).
TACHE : le dessin du pouvoir est Script\Factions\HOLFactionPowerServer.as:72-99 (`DrawSacredLightPlaceholder` l.75 : DrawDebugCircle au rayon de zone et a la moitie, l.97-98 ; `DrawLightSealPlaceholder` l.85). Hypotheses a trancher par LECTURE puis scenario : (a) le grand cercle = zone du Sceau / autre placeholder ; (b) = un effet de HOLZoneEffect (C++ visuel : ne le touche pas, Arwen) ; (c) = le cercle de portee du heros. Lis en UN appel : ce fichier l.72-99, le .look de Tools\Scenarios\faction_aim_free.look, les 2 PNG de la recette (ouvre-les), `git grep -n "DrawDebugCircle" main -- Script Source`. PREMIER TEST ROUGE : test script `Test_SacredLightPlaceholderDrawsOnlyTheDocumentedCircles` : fonction pure `HOLFactionPowerServer::PlaceholderCircleRadii(float ZoneRadius)` -> {R, R * 0.5} ; si le grand cercle vient d'ailleurs, le test reste vert et tu livres la CAUSE (fichier:ligne) au rapport + la ligne Placeholders.txt (C:\Users\baudo\Documents\MyCrusader\Placeholders.txt : relis avant d'editer, ne touche que ta ligne) + PlaceholdersInProgress.md. Premier Edit avant le 8e appel.
Fichiers deja pris : HOLFactionAim* (Arwen : l'aperçu de visée peut AJOUTER un cercle : demande-toi si c'est lui avant d'accuser le placeholder), HOLZoneEffect (Arwen) ; F2 / F3 (Faramir) editent HOLFactionPowerServer.as l.42-133 : ne touche que l.72-99 si tu dois editer, ou ne fais qu'un test neuf.
```
