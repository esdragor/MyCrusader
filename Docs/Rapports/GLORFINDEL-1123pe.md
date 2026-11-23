# GLORFINDEL-1123pe (2026-11-23 16:10 -> 16:2x), ability-columns, worktree HeroesOfLiberty-Glorfindel

Joueur : l'adversaire de l'Aube plante sa banniere au vrai lieu de son heros puis lance Lumiere sacree dessus ; preuve en partie faite.

## 1. Echec du test
Test_AIPlantsBannerFromTheHeroRealPlace (quick_tests.log:14277+) : `Angelscript: Error: Assertion failed (Expected true, got false) / a far group: the banner is pushed from the hero's real place toward it, at the edge of his reach` = la 4e assertion (groupe lointain). Les assertions 1-3 (plantage, portee reelle, centre des blesses) passaient.
CAUSE : ASSERTION fausse, pas la regle. La portee de plantage est 1600 cm (HOLHeroAbility_Test.as:44-45 : 1600 vrai, 1601 faux). Heros (14900, 0) et groupe a 17900 : la regle (PlanBanner, dichotomie sur CanPlantBanner) pose le point a ~16499.99 ; le test exigeait X < 16000 (borne mal calculee, 1100 au lieu de 1600). Correction : `> Hero.X + 1500 && <= Hero.X + 1600`. Une seule ligne de test, Script/Tests/HOLAIPowerCall_Test.as:190. Regle (HOLAIPowerCall.as PlanBanner / PlanBannerOf) inchangee.

## 2. Tests
Commit 2487484 (message termine par `Agent: GLORFINDEL-1123pe`). quick_check -Parts tests : 2150/2150 PASS (218 s), 0 erreur Angelscript.

## 3. Preuve en partie (preflight ai_sacred_light : PASS 43 s)
Journal du jeu : Saved_Tests\Logs\scenario_ai_sacred_light.log (-abslog de preflight ; PAS Saved\Logs, d'ou les « banner at » introuvables par la session principale).
- step 14 : heros IA vivant, repli de l'origine = lieu du heros `banner (14940, -46)` (avant tout plantage), wounded 0 of 7.
- Hero sheet slot 1 : state Alive level 5 hp 594.
- step 99 : `banner at (14801, 53) planted` (distance au heros (14940,-46) ~170 cm, a portee 1600) ; meme ligne : `wounded 2 of 7, decision cast=true row 0, banner (14802, 54), nearest wounded 175.6 cm` ; `Sacred Light cast on 2 wounded unit(s)` (origine = la banniere).
- Hero sheet slot 1 apres : `banner 14802 54 until 999`.
- Honneur : 100 avant (step 94), 100 apres (step 104 et suivants), cooldown 19723 ms apres le lancer. L'Honneur ne baisse PAS dans le journal (le cout du pouvoir pour l'IA n'est pas visible : ANOMALIE a verifier ; l'IA regenere peut-etre, ou le cout n'est pas debite pour l'IA).
- Erreurs Angelscript : 0 (aucun « Angelscript: Error », « requires WorldContext », « Null pointer », « Script exception » ; seules les 20 « Condition failed » de LogAutomationTest au demarrage, tests moteur sous -culture=fr, hors scenario).

## PNG after
Ouvert : ai_sacred_light_after.png. On voit, au centre, la melee de petits personnages rouges et bleus et DEUX anneaux dores concentriques (un petit, un grand) centres sur cette melee. La banniere elle-meme n'est PAS distinguable (trop petite, noyee dans le groupe). Le plateau « en preparation » masque une partie du haut. Je ne peux pas affirmer que les anneaux sont centres au pixel pres sur la banniere ; ils sont centres sur le groupe, et le journal place la banniere a (14802, 54), a 175 cm des blesses.

## Verdict
SURE a integrer a 2487484 (tests 2150/2150, banniere plantee vue au journal, cast=true depuis la banniere, 0 erreur AS), sous reserve de l'anomalie Honneur (100 -> 100) qui n'a pas ete creusee.

## Champs
TEST_ROUGE: sans objet (correction d'assertion) ; TESTS_TAUTOLOGIQUES: 0 ; BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; TYPES_A_EXPOSER: aucun ; DIAGNOSTICS_AS: non consulte (quick_check 0 erreur) ; Placeholders.txt : rien. Merge main non fait (consigne).
## NON VERIFIE
Honneur depense par l'IA (100 -> 100) ; centrage de l'anneau sur la banniere au pixel ; AubeInProgress.md A12 non edite faute d'appels (ligne a mettre a jour par la session principale). Le D897 n'est pas commence.