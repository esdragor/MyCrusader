# GLORFINDEL-1123pd (2026-11-23, 15:53 -> 16:03), ability-columns, worktree HeroesOfLiberty-Glorfindel

Joueur : le premier pouvoir (Lumiere sacree) ne sera plus refuse en debut de partie (heros + paysans seuls) et l'adversaire de l'Aube pourra planter sa banniere. NON COMPILE : rien n'est visible tant que la session principale n'a pas construit.

## Cause commune
Le script n'avait pas la position du heros d'un emplacement (le heros n'est pas dans `Summary.Army`, HOLAISummary.cpp:105). Aucun accesseur appelable du script n'existait (Grep `GetHeroLocation` : seulement `FHOLStartRules::GetHeroLocation` C++, `HeroLocation()` interne de HOLDuelSubsystem.cpp:48). Liaison NEUVE.

## Commits (worktree Glorfindel, branche ability-columns, tip b9c6d18 avant moi ; PAS de merge main)
- c224ca0 (RED, NOT COMPILED) : bouchon `UHOLHeroLibrary::GetHeroLocation` (rend faux), bouchons script `CastOriginOf(banniere, armee, bHeroKnown, HeroPlace)` (ignore le heros) et `HOLAIPowerCall::PlanBannerOf` (passe (0,0,0)), `HOLFactionPowerServer::HeroPlaceOf(Slot, OutPlace)`, 3 tests.
- 3974596 (corps, NOT COMPILED) : C++ `GetHeroLocation` = `UHOLUnitSubsystem::GetUnitView(FindHeroUnit(Slot))`, faux si pas de vue ou PV <= 0, Location copiee (+ include HOLUnitView.h) ; `CastOriginOf` a 2 arguments delegue au 4 arguments (une source) : banniere, sinon centre de l'armee, sinon le heros, sinon la banniere telle quelle ; `TryCastPower` lit `HeroPlaceOf` ; `ThinkPowers` : `HeroPlace` = `HeroPlaceOf`, `PlanBannerOf` (heros inconnu ou capacite non prete = rien), origine du lancer a 4 arguments ; `BannerCooldownMs` calcule en double (avertissement de division entiere supprime).

## Tests rouges / a lancer par la session principale
Noms exacts (script) : `Test_CastOriginIsTheHeroWhenNoBannerNorArmy` (HOLFactionPowerOrigin_Test.as : ROUGE sur c224ca0, origine (0,0)), `Test_AIPlantsBannerFromTheHeroRealPlace` (HOLAIPowerCall_Test.as : ROUGE sur c224ca0, point a ~(1599,..) hors de portee du vrai heros), `Test_CastOriginOrderIsBannerThenArmyThenHero` (caracterisation : verte aussi sur le bouchon, verrouille l'ordre banniere > armee > heros).
Les rouges n'ont PAS ete vus a l'execution (C++ ajoute : pas de build dans l'agent, plafond de 3 agents C++). Tests PURES (origine choisie a partir de {banniere, armee, position du heros ou aucun} ; HOLTestWorld::SpawnTestHero absent) : le lecteur C++ n'est couvert que par le scenario.
Commandes : `Tools\build.ps1 -LiveCoding=0 -Wait` puis `quick_check.ps1 -Parts tests` (attendu 2147 + 3 = 2150), puis `preflight.ps1 ai_sacred_light` : lire `banner at (...) planted` et `Hero sheet slot 1: ... Alive`; et rejouer faction_click_cast avec heros + 5 paysans SANS les 4 Hommes d'armes (le scenario retouche par RECETTE-1123pa les spawne pour contourner ce defaut : on peut les retirer).

## RELECTURE_CPP (L20-2)
GetHeroLocation : WorldContext via GetWorldOf (comme les voisines) ; `FHOLUnitView` a Location (FVector) et Health (usage HOLHeroSubsystem.cpp:710 / HOLHeroReturnTests) ; garrison = pas de vue = faux (heros en garnison : retombe sur l'armee). UFUNCTION(BlueprintPure) avec `FVector&` sortie : le script la voit `FVector&out`. Pas de nouvel en-tete central.

## Champs
TEST_ROUGE: non vu (NOT COMPILED) ; TEST_ROUGE_SEPARE: oui (c224ca0 puis 3974596) ; TESTS_TAUTOLOGIQUES: 0 ; BUILDS_DEMANDES: 1 ; TESTS_SCRIPT_AJOUTES: 3 (noms ci-dessus) ; TYPES_A_EXPOSER: aucun de plus (HOLHero::GetHeroLocation est fait) ; DIAGNOSTICS_AS: non consulte ; Placeholders.txt : rien d'ajoute ni retire.
## NON VERIFIE
Compilation C++ et script (HeroPlaceOf utilise `FVector&out` et la surcharge de CastOriginOf : a verifier au premier build) ; que le heros de l'IA soit bien un `FindHeroUnit` valide a ce moment ; la partie (b) du defaut de recette (journal des PV de la zone de soin) reste ouverte (nouvelle ligne de la file).
## Anomalies
1er essai de commit-2 abandonne par une erreur de mon script (variable `$TOutLocation`), sans effet sur les fichiers (le script s'arrete avant ecriture d'autre chose que l'include, relance sans doublon). Appels : ~13.