APPELS_DECLARES: ~16 (arret 16:04)
HEURE_DEBUT/HEURE_FIN: 15:48 / 15:59
LAUNCHCHECK_DEBUT: binaire perime signale par build.ps1 (source 15:49 > dll 15:42), build lance
BUILD: PASS (HeroesOfLibertyEditor Development, 29 s, 15:51)
MESSAGE_ECHEC_LU (Saved_Tests\Logs\quick_tests.log:34515-34517, run 15:46) : "Expected 'the enemy killed near the centre is gone (enemy hp -7, Dawn spearman hp 80, 2 unit(s) alive)' to be true. [HOLZoneEffectTests.cpp(529)]" ; journal : "Combat: step 0 #8 Spearman of slot 1 killed by #4 Spearman of slot 0 (1 hits struck)" ; idem cas lointain l.540 "enemy hp -7". Les assertions de jauge (l.530, 531, 541) N'ONT PAS echoue.
CAUSE: aucune des 3 hypotheses du brief : le coup est porte, l'ennemi meurt (hp -7), l'Honneur est credite (jauge 3 pres, inchangee loin : les 3 TestEqual passent). Seul le predicat du TEST IsGone (GetUnitTypeName(Id).IsNone(), HOLZoneEffectTests.cpp ~l.483) etait trop strict : l'unite a hp <= 0 reste dans le sous-systeme a la fin du pas. 0fcf23d (id multiple de 4) etait donc le bon correctif de mise en scene ; le rouge restant etait le predicat.
CORRECTIF: 5294d6d (zone-effect, TEST seul, Agent: FARAMIR-1123pc) : IsGone = HealthOf(Id) <= 0 (comme IsDown de HeroXP.KillInMatch). CODE DE JEU INCHANGE (AwardKillHonor / CreditKillHonor intacts).
RELECTURE_CPP: lambda IsGone [&HealthOf] declaree apres HealthOf et avant RunUntilGone ; aucun include ajoute.
TESTS: quick_check -Parts tests : 2145/2145 PASS (321 s), Zone.KillHonor vert.
BUILDS_DEMANDES: 0 (build fait)   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun   TEST_ROUGE: vu 15:46 : 2144/2145   TEST_ROUGE_SEPARE: sans objet   TESTS_TAUTOLOGIQUES: 0
NON VERIFIE: exposition de l'Honneur a la mort en partie reelle (cas IsGone vs retrait differe de l'unite non teste hors de ce test).
EDITS_REFUSES: 0
ANOMALIES: une attente par boucle ping (consigne 3d) pendant le quick_check ; le diagnostic de 0fcf23d etait correct mais incomplet.
BRANCHE: zone-effect PRETE a 5294d6d.