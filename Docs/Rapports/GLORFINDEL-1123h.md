# GLORFINDEL-1123h (IA cast=false, ability-columns)
CAUSE: HOLHero::GetHeroBannerLocation(slot IA) = (0,0) (l'IA ne plante jamais de banniere) ; blesse le plus proche a 14709 cm, portee 800 cm (LightSeal) : Plan correct, origine fausse. Aussi cote serveur (TryCastPower lisait la meme banniere). Log vu : "banner (0, 0), nearest wounded 14709.31 cm, range 800 cm, tier 0, level 0, cooldown left 0 ms".
TEST_ROUGE: vu a 10:19 : tests 2103/2106, Test_AIBrainCastsWithoutAPlantedBanner rouge (commit 9e89831, stub OriginOf) ; TEST_ROUGE_SEPARE: oui
VERT: ffd0325 : tests 2104/2106 (restent rouges, sans lien : HOLCommandGrid_Test.Test_CGTables, HOLWallLineRules_Test.Test_WallTypesListsPalisade) ; TESTS_TAUTOLOGIQUES: 0
COMMITS: 3 (log, rouge, vert), pas de merge main
ESSAIS: ai_sacred_light : preflight rejoue AVANT correctif (FAIL, timeout waitlog, ligne de cause vue) ; APRES correctif NON REJOUE (heure d'arret) ; aucun PNG ouvert ; LOGIQUE SEULEMENT
QUESTIONS: Retours\Q-sacred-light-values.md (origine = centre armee par defaut ; portee 800 cm du Sceau inchangee)
hol.AI.HonorSet : reste une cvar de TEST (defaut -1).
NON VERIFIE: lancer reel en partie apres correctif ; Test_CGTables / Test_WallTypesListsPalisade rouges avant mon travail (non touches).
ANOMALIES: appels depasses (~28 contre 25) ; 1er quick_check lance avec le stub apres un echec de remplacement (annule par stray_processes) ; Placeholders.txt non relu.