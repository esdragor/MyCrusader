# HALDIR-1120a : D859 sous-etapes 1a et 1b (Valeurs de depart de l'Honneur, Defi du Chevalier, Lumiere sacree)

Impact joueur : rien ne change a l'ecran (aucun appelant encore) ; les 11 valeurs d'Honneur / Defi du Chevalier et la Lumiere sacree (cout 25, recharge 20 s) existent maintenant dans UNE source script, lue par les regles de jauge et de pouvoir.

APPELS_DECLARES: 22 (arret a 25 vus par moi)
HEURE_DEBUT/HEURE_FIN: 09:03:58 / 09:06:52 (Get-Date PowerShell ; arret dur 09:27)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build demande)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 02d67b4 (1a : Test_HonorStartingValues, Test_KnightChallengeValues) et f9bd47e (1b : Test_SacredLightFromFullBar, Test_SacredLightCooldown) ; assertit l'effet : oui (jauge, surplus, kills / soins, zone, recharge, nombre de lancers : tout calcule par HOLFactionResource::Make/Gain/Lose/TrySpend, HOLPowerBook::StateOf, HOLGroundAbility::IsInZone/ZoneActive/CooldownEndMs). Rouge NON VU par moi (aucun quick_check) : a voir par la session principale.
COMMITS: 4, dernier a ~09:06 (branche wall-line), merge_main_fait: non (inutile : wall-line = main 901e391, sans C++)
  - 02d67b4 stub HOLDawnValues.as (11 fonctions rendant 0) + Script\Tests\HOLDawnValues_Test.as
  - f9bd47e stub HOLDawnPowers.as (Cost 0, CooldownMs 0) + Script\Tests\HOLDawnPowers_Test.as
  - 2e0212e HOLDawnPowers.as : SacredLight Cost 25, CooldownMs 20000
  - 78a28be HOLDawnValues.as : 100, 0, 10, 30, 5, 1, 1200, 50, 8000, 800, 45000 + 13 lignes Tools\Data\ValueSheet.csv
TESTS: aucun quick_check (consigne). Lance a la main : `python Tools\Data\value_sheet_test.py` 30/30 OK, `python Tools\Data\value_consumers_test.py` 16/16 PASS (sur l'arbre final). TESTS_SCRIPT_AJOUTES: 4 (Test_HonorStartingValues, Test_KnightChallengeValues, Test_SacredLightFromFullBar, Test_SacredLightCooldown) ; TESTS_TAUTOLOGIQUES: 0
BUILDS_DEMANDES: 0   TYPES_A_EXPOSER: aucun
ESSAIS: aucun (valeurs de regle, rien a l'ecran)
QUESTIONS: aucune (MinTier de la Lumiere sacree : laisse a 0, valeur par defaut de la structure)

COMMANDE A LANCER (session principale, un editeur a la fois) :
  1. Rouge : se placer sur 02d67b4 puis f9bd47e du worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir (le stub rend 0 : attendu 4 tests rouges ; Test_SacredLightFromFullBar a une boucle bornee a 10 pour ne pas boucler avec un cout nul), `Tools\quick_check.ps1 -Parts tests` ;
  2. Vert : tip 78a28be, meme commande : attendu +4 verts.

FAIT / RESTE:
- FAIT : 1a (n 1-4 et 10 : HonorCap 100, HonorStart 0, HonorDuelAccept 10, HonorDuelWin 30, HonorDuelRefuse 5, HonorKillGain 1, HonorKillRadiusCm 1200, HonorHealPerPoint 50, KnightChallengeDurationMs 8000, RadiusCm 800, CooldownMs 45000) ; 1b (n 5 : SacredLight, Cost 25, CooldownMs 20000).
- Lignes ValueSheet.csv ajoutees (13, Source `code`, Locator regex sur le `return` / l'affectation, ASCII sans accents pour ne pas corrompre l'encodage) : honor.cap, honor.start, honor.duel_accept, honor.duel_win, honor.duel_refuse, honor.kill_gain, honor.kill_radius_cm, honor.heal_per_point, knight.challenge_duration_ms, knight.challenge_radius_cm, knight.challenge_cooldown_ms, dawn.sacred_light_cost, dawn.sacred_light_cooldown_ms. Premiere version : 2 lignes avaient une virgule non quotee (value_sheet_test rouge 29/30), corrige avant le commit.
- RESTE : les appelants (boucle `Ended` de HOLDuelLedger.as, vue HOLFactionSlot) ; le kill / soin comptes par la simulation = C++ ; 1c (Glorfindel), 1d (Celeborn) ; fusion dans main (ordre conseille : apres Sauron et Glorfindel, conflit possible en fin de ValueSheet.csv).
- Choix a connaitre : HOLDawnPowers::Rows() laisse `bEffectCoded = false` (l'effet n'existe pas : la regle D601 l'affiche « Bientot ») ; les tests posent `bEffectCoded = true` sur une copie de la ligne pour tester le cout. Les fonctions `RadiusCm()` et `CooldownMs()` du namespace HOLDawnValues sont les noms demandes par l'enonce (ils designent le Defi du Chevalier : le commentaire le dit) ; le Locator de ces deux lignes est ancre sur `int RadiusCm\(\)` / `int CooldownMs\(\)` pour ne pas confondre avec HonorKillRadiusCm.
- Placeholders.txt : aucune ligne (valeurs de regle, aucun visuel provisoire) ; non touche.

NON VERIFIE: aucun test script lance (rouge ni vert) ; que `T.AssertEquals(FString, FString, FString)` et `HOLFactionResource::Describe` rendent exactement "Honor 0/100" (lu dans le source : f"{Name} {Value}/{Cap}") ; que `AssertEquals(int, int, ...)` compile avec un `int` issu d'une ternaire (`HealPoints`) ; la fusion de fins de ligne (git signale LF -> CRLF sur les 4 .as neufs, sans effet) ; le script du decouvreur de tests prend bien les fichiers neufs de Script\Tests.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
ANOMALIES_PROCESSUS: aucune. Edition `edit_line -Append` de EtapeDefini.md : ligne ajoutee en 6 (apres la ligne de seance) ; Auto_Journey_File.md : ligne `[x]` ajoutee en 32 (apres la ligne wall-line 30 de HALDIR-1119n2).
GIT_STATUS_PRINCIPALE: modifs hors de mes fichiers : DuelInProgress.md, Placeholders.txt, PlaceholdersInProgress.md, SignalsInProgress.md (autres sessions) ; les miens : AubeInProgress.md, Auto_Journey_File.md, EtapeDefini.md : oui (docs voulus seulement). Crown.umap non suivi dans le worktree Haldir : non touche.
