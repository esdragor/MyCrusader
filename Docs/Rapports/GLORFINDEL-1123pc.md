# GLORFINDEL-1123pc (2026-11-23, 15:42 -> 15:51) : D891, la liaison PlantHeroBanner jouee en partie

Joueur : l'adversaire de l'Aube NE plante PAS encore sa banniere ; il lance toujours la Lumiere sacree depuis le centre de son armee.

Verdict : liaison SAFE a l'execution (0 erreur AS), mais PAS sure a integrer comme « D891 fait » : la banniere est refusee a chaque tentative. Sure a integrer seulement comme code inoffensif ; la fonctionnalite ne marche pas.

## Journal (worktree Glorfindel, ai_sacred_light, binaire 153bd68)
- Rejeu 1 (scenario inchange) : aucun heros pour l'IA (« Hero sheet slot 1: state None level 0 »), donc `bHeroCanPlant` faux, aucune ligne « banner at », lancer depuis le centre de l'armee (14939, -46), « Sacred Light cast on 1 wounded unit(s) » step 39.
- Mise en scene corrigee (commit b9c6d18) : `hol.Units.SpawnHere 1 Hero 1 300`, `waitunits 8`, `hol.Hero.SetXP 1 3000`, `hol.Hero.Report` avant et apres. Le heros de l'IA est vivant, niveau 5, 594 PV (« Hero sheet slot 1: state Alive level 5 hp 594 »).
- Rejeu 2 : `AI slot 1 step 149: banner at (1599, -2) refused`, puis `step 154: banner at (1599, -14) refused`. Lancer au meme step 149 : `decision cast=true row 0, banner (14941, -43)` = encore le centre de l'armee ; `Sacred Light cast on 5 wounded unit(s)`, `Honor now 75` (100 avant). Fiche finale : `banner 0 0 until -1` (aucune banniere).
- Erreurs Angelscript (« Angelscript » + « rror », « requires WorldContext », « Null pointer ») : 0 sur les deux rejeux. Seuls des avertissements de compilation (division entiere, dont HOLAIBrain.as:173 = ligne `BannerCooldownMs`, nouvelle).

## Cause (par le journal)
(1599, -2) est a 16 m de l'ORIGINE (0, 0). `HeroPlace` (HOLAIBrain.as:164-171) est cherche dans `Summary.Army` avec `Type == n"Hero"` : le heros n'y est pas (Army = 7 unites avec le heros vivant), donc `HeroPlace` reste (0,0,0). `PlanBanner` voit une distance de ~14900 cm, bisecte le point a 16 m de (0,0) vers les blesses, et `UHOLHeroSubsystem::PlantBanner` refuse (le vrai heros est a ~(14900, 0), a ~13 000 cm du point). Liaison C++ : appelee, rend faux a bon droit, aucun bug la ; le defaut est dans le script de decision (HOLAIBrain.as) : la position du heros est fausse.

## Correction a faire (non faite : demande du C++ ou un changement de resume)
- Exposer la position du heros d'un slot : p. ex. `UHOLHeroLibrary::GetHeroLocation(Slot)` (via `Units->FindHeroUnit(Slot)` + `FindEntity` + fragment de transform, comme `UpdateSheets`) ; ou inclure le heros dans `Summary.Army`. Puis HOLAIBrain.as:164-171 lit cette position. Test rouge script : scenario ou `HOLAIPowerCall_Test` avec un `HeroLocation` reel.
- Autre point : la banniere se plante a la position du heros (portee 16 m), pas pres des blesses : `PlanBanner` ramene deja le point dans la portee, correct une fois la position juste.

## PNG (ai_sacred_light_after.png, ouvert)
On voit une mêlee rouge / bleue (soldats de l'IA et Hommes d'armes du joueur) devant un batiment a toit de bois (forum), et un anneau dore (rayon ~600 cm) autour de la melee, centre sur elle. AUCUNE banniere visible (non plantee). L'ancrage de l'anneau est le centre de l'armee, pas une banniere. Le panneau de scenario (note 1) recouvre le haut. Le PNG « before » n'a pas ete rouvert (non modifie par le correctif, 2 rejeux sur 3 utilises).

## Champs
- Commit : b9c6d18 (scenario seul), `Agent: GLORFINDEL-1123pc`. Pas de C++, pas de build, pas de test rouge : TEST_ROUGE: aucun ; TESTS_SCRIPT_AJOUTES: 0 ; BUILDS_DEMANDES: 0 ; TYPES_A_EXPOSER: HOLHero::GetHeroLocation(Slot).
- preflight : « FAIL user's launch: 1 uncommitted file » (le .txt non commite pendant le rejeu, corrige par le commit) ; check PASS 25-28 s.
- NON VERIFIE : l'hypothese « le heros n'est pas dans Summary.Army » (deduite du point (1599,-2), pas d'un log de la liste) ; le .look n'a pas ete mis a jour pour le heros ; la capture « apres le plantage » n'existe pas faute de plantage ; DIAGNOSTICS_AS serveur de langage non consulte ; A12 d'AubeInProgress.md non edite.
- Anomalie : `hol.Hero.SetXP` journalise « xp 3000 » mais la fiche lue 10 images plus tard donne xp 0 puis niveau 5 : la fiche est mise a jour au pas suivant (sans gravite).
- Appels : ~19.
