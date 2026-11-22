# CELEBORN-1122h : F8 T4, cadence du tir de batiment. NON COMPILE

Joueur : la Tour de guet en bois tire une fleche toutes les 1,625 s (au lieu d'une seule), la premiere tout de suite, sans rafale quand un ennemi arrive apres une longue pause ; plusieurs tours tirent chacune dans le meme pas.
Technique : UHOLCombatProcessor::ReleaseBuildingArrows : TMap<uint32, FBuildingCooldown> BuildingCooldownMs (compte a rebours en ms de SIMULATION = StepMs du pas) remplace le TSet BuildingsShot.

APPELS_DECLARES: 18 (arret a 25 vus ; le seuil se juge sur 	ool_uses de la session principale)
HEURE_DEBUT/HEURE_FIN: 17:59:42 / 6:04:23 PM (Get-Date)
PREMIER_BUILD: sans build (consigne : NE COMPILE PAS)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: e76adf9 (bouchon qui compile + test, meme commit), rouge NON VU a l'execution ; assertit l'effet : oui (fleches reellement lachees au pas fixe, PV perdus) ; TEST_ROUGE_SEPARE: oui (e76adf9 rouge, ba0fef9 vert)
TEST_ROUGE attendu sur e76adf9 : le bouchon ne cree aucune entree et garde le TSet (une fleche en tout) : HeroesOfLiberty.Combat.BuildingShotCadence echoue (1 fleche au lieu de 10 sur 16 s, pas de cadence, 0 entree au lieu de 1 / 2) ; Combat.BuildingShot echoue seulement sur Far.EntriesAlive == 1 (0 au bouchon).
COMMITS: 2 sur integ-41 (e76adf9, ba0fef9), dernier 18:03 ; merge_main_fait: non (DLL non compilee)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (le calendrier attendu est calcule dans le test depuis le pas et l'intervalle, les pas de lacher viennent du vrai processeur)
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ : Tools\build.ps1 -LiveCoding=0 dans C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, puis quick_check -Parts tests, filtre HeroesOfLiberty.Combat.BuildingShot (couvre BuildingShot, BuildingShotCadence, BuildingShotDamage))

## Fait
- HOLCombatProcessor.h : struct protegee FBuildingCooldown {RemainingMs, SeenStep}, TMap<uint32, FBuildingCooldown> BuildingCooldownMs, accesseur public GetBuildingCooldownCount() (tests) ; BuildingsShot supprime.
- HOLCombatProcessor.cpp ReleaseBuildingArrows : StepBuildings vide -> Reset() de la map et retour (aucun cout sans batiment debout) ; batiment non arme (BuildingAttack <= 0, pas fini, Gone) : saute, aucune entree ; entree absente -> creee a 0 (premiere fleche immediate, pas de retrait ce pas) ; entree presente -> RemainingMs -= StepMs ; > 0 -> pas de recherche de cible (aucun cout) ; sinon recherche de la cible (regle inchangee) ; pas de cible -> RemainingMs = 0 (rien ne s'accumule : pas de rafale) ; fleche lachee -> RemainingMs += max(1, ToMilliseconds(BuildingAttackInterval)) (le report negatif est conserve : intervalle exact, 1,625 s n'etant pas un nombre entier de pas) ; fin de boucle : toute entree dont SeenStep != StepIndex est retiree (batiment detruit, plus arme : pas de fuite). Un batiment lache au plus une fleche par pas, chaque tour la sienne dans le meme pas.
- HOLBuildingShotTests.cpp : RunScenario prend FShotOptions (victime solide, pause hors portee, 2 tours, duree, destruction finale) et enregistre le pas de chaque fleche ; nouveau test HeroesOfLiberty.Combat.BuildingShotCadence : (1) 16 s en portee : 10 fleches, calendrier premier + ceil(K*1625/pas) calcule dans le test, 10e fleche a 9 intervalles du 1er (a un pas pres, en ms de simulation) ; (2) pause de 20 s hors portee : 0 fleche pendant, puis premiere immediate et meme calendrier (aucune rafale) ; (3) deux tours : meme pas pour les premieres fleches, 2 par pas ensuite, 2 entrees ; (4) tours detruites : 2 entrees avant, 0 apres. Combat.BuildingShot : Released == 1 remplace par >= 1 et borne PV par fleche ; fondation = 0 entree, tour armee sans cible = 1 entree ; cas cible fragile / tour detruite en vol adaptes (options).

## RELECTURE_CPP
- FBuildingCooldown declare avant usage dans la classe (membre protege, utilise dans le corps de ReleaseBuildingArrows de la meme classe) ; GetBuildingCooldownCount() est dans la section publique et ne lit que .Num() (corps inline : classe complete).
- BuildingCooldownMs.Add(Step.Id) (Add(Key) construit la valeur par defaut) ; le pointeur Cooldown n'est utilise que dans son tour de boucle (un Add d'un autre batiment au tour suivant ne l'invalide pas).
- Suppression pendant iteration : It.RemoveCurrent() avec CreateIterator(), forme permise.
- Reste de BuildingsShot dans Source\ : aucun (Select-String).
- Tests : FTransformFragment et GetMutableTransform().SetLocation (modele HOLDeployButtonTests.cpp:940-946) ; HOLCombat::ToMilliseconds exporte (HOLCombatRules.h:84).

## Defauts vus dans 1122g (signales, non refaits)
- Aucun defaut visible dans ReleaseBuildingArrows / GetBuildingShotDamage. Remarque : l'appel de ReleaseBuildingArrows (HOLCombatProcessor.cpp ~l.1471) n'est pas garde par Fighting (hol.Combat.Enabled 0 laisse tirer les bâtiments) : a confirmer en T5/T7, non touche.

## NON VERIFIE
- Tout : rien compile ni lance. Hypotheses : ToMilliseconds(1.625f) == 1625 (arrondi), la premiere fleche part au pas 0 ou 1 (tolere : First <= 1), FTransformFragment de la victime est bien la position lue par le combat (Fighter.Location, l.1061), CurrentHealth = 100000 n'est pas ramene au max par un autre processeur, la 2e tour a (3000, 1500) ne chevauche pas la 1re ni un obstacle de navigation.

PROCESSUS_DETACHES:
aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn

ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE (docs voulus seulement : oui) :
 M Auto_Journey_File.md
?? Docs/Concept_Faction_Theatre_2026-11-18.md
?? Docs/HSR_Aha_Allegresse_Nihilux.md
?? Docs/Priorite_Socle_Factions_2026-11-16.md
?? Docs/Rapports/ARWEN-1122a.md
?? Docs/Rapports/ARWEN-1122b.md
?? Docs/Rapports/ARWEN-1122c.md
?? Docs/Rapports/ARWEN-1122d.md
?? Docs/Rapports/ARWEN-1122e.md
?? Docs/Rapports/ARWEN-1122f.md
?? Docs/Rapports/CELEBORN-1122a.md
?? Docs/Rapports/CELEBORN-1122b.md
?? Docs/Rapports/CELEBORN-1122c.md
?? Docs/Rapports/CELEBORN-1122d.md
?? Docs/Rapports/CELEBORN-1122e.md
?? Docs/Rapports/CELEBORN-1122f.md
?? Docs/Rapports/CELEBORN-1122g.md
?? Docs/Rapports/ELROND-1122a.md
?? Docs/Rapports/ELROND-1122b.md
?? Docs/Rapports/ELROND-1122c.md
?? Docs/Rapports/ELROND-1122d.md
?? Docs/Rapports/ELROND-1122e.md
?? Docs/Rapports/ELROND-1122f.md
?? Docs/Rapports/ELROND-1122g.md
?? Docs/Rapports/ELROND-1122h.md
?? Docs/Rapports/ELROND-1122i.md
?? Docs/Rapports/ETAPE0-1122.md
?? Docs/Rapports/FARAMIR-1122a.md
?? Docs/Rapports/FARAMIR-1122b.md
?? Docs/Rapports/FARAMIR-1122c.md
?? Docs/Rapports/FARAMIR-1122d.md
?? Docs/Rapports/GALADRIEL-1122a.md
?? Docs/Rapports/GALADRIEL-1122b.md
?? Docs/Rapports/GALADRIEL-1122c.md
?? Docs/Rapports/GALADRIEL-1122d.md
?? Docs/Rapports/GALADRIEL-1122e.md
?? Docs/Rapports/GLORFINDEL-1122a.md
?? Docs/Rapports/GLORFINDEL-1122b.md
?? Docs/Rapports/GLORFINDEL-1122c.md
?? Docs/Rapports/GLORFINDEL-1122d.md
?? Docs/Rapports/GLORFINDEL-1122e.md
?? Docs/Rapports/GLORFINDEL-1122f.md
?? Docs/Rapports/GLORFINDEL-1122g.md
?? Docs/Rapports/GLORFINDEL-1122h.md
?? Docs/Rapports/GLORFINDEL-1122i.md
?? Docs/Rapports/GLORFINDEL-1122j.md
?? Docs/Rapports/HALDIR-1122a.md
?? Docs/Rapports/RECETTE-1122a.md
?? Docs/Rapports/RECETTE-1122b.md
?? Docs/Rapports/SAURON-1122a.md
?? Docs/Rapports/SAURON-1122b.md
