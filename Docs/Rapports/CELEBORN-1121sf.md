# CELEBORN-1121sf : moral, la banniere plantee posee par le processeur ; le combat ne lit plus que le fragment

NON COMPILE (aucun build, aucun quick_check, consigne). Impact joueur : une unite alliee a portee d'une banniere plantee recoit le levier de la ligne Banner (donc les 15 % de l'Aura) depuis le fragment de moral, une seule fois, meme sous l'aura du heros ; le combat n'a plus de constante ni de branche IsInAura pour les degats recus.

APPELS_DECLARES: ~23 (arret a 25 vus ; premier Edit/ecriture au 17e appel : brief non tenu, lecture du point d'application et du harnais de test)
HEURE_DEBUT/HEURE_FIN: 20:17 / 20:20 (Get-Date PowerShell ; arret dur 20:31 respecte)
PREMIER_BUILD: sans build (NON COMPILE)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de compilation par l'agent)
TEST_ROUGE: f786eb0 (test seul, aucun bouchon : le processeur ne connait que les heros) ; rouge NON VU a l'execution ; assertit l'effet : oui (levier Taken du fragment d'un allie a 10 m de la banniere plantee par le vrai HOLHeroSubsystem::PlantBanner, attendu CALCULE = MakeEffectFromRow(Banner) pose sur un etat vide)
COMMITS: 2 (f786eb0 test, 129fa9a correctif), dernier a 20:20, merge_main_fait: non (branche integ-41 = main + levier Taken, DLL non compilee)
TESTS: aucun lance. TESTS_TAUTOLOGIQUES: 0 (attendu calcule par la regle, jamais recopie)

ROUGES ATTENDUS (nom par nom)
- Au tip f786eb0 : 1 rouge, HeroesOfLiberty.Morale.ProcessorLaysBannerAuraInRange (assertions : « the ally 10 m from the banner reads the Banner row's damage taken lever », « ... laid once », « the ally near the hero AND the banner carries one effect, not two » (Num == 1 : echoue aussi car le heros le couvre deja seul = 1 effet ? NON : l'allie a (800,800) est couvert par le heros, donc Num == 1 deja ; seule l'assertion « 10 m from the banner » + « laid once » est rouge). Les assertions « out of range », « enemy », « Peasant » passent deja.
- Au tip 129fa9a : 0 rouge attendu parmi HeroesOfLiberty.Morale.* et HeroesOfLiberty.HeroAbility*.

PARTIES A LANCER : `quick_check -Parts tests` apres `Tools\build.ps1 -LiveCoding=0 -Wait` sur C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn.

GARDES (relus) : HOLHeroAbilityMatchTests.cpp ne change PAS de sens (10.2 sous aura 12 m, 7.2 aura + cri, 10.2 sous la banniere a 10 m, hero mort : banniere reste, 45 s : banniere partie, 11 m des deux : un seul 10.2) ; MAIS son RunStep ne lancait que le processeur de combat : sans le processeur de moral, le combat ne voyant plus que le fragment, tout serait rouge. Changement de cablage seulement (commit 129fa9a, 4 lignes) : le processeur de moral tourne avant le combat dans RunStep (UE::Mass::Executor::Run(*Morale) puis (*Combat), meme ProcessingContext reutilise : a confirmer au build).

FAIT
- HOLMoraleProcessor.cpp : FAuraSource porte maintenant (lieu, camp, rayon carre, effet). Sources = heros vivants (ligne DawnAura) + bannieres debout (UHOLHeroSubsystem::GetBanner par slot, ligne Banner, rayon de la ligne, effet MakeEffectFromRow(Banner) = levier de DawnAura sous l'id DawnAura via Gives). Meme id depuis plusieurs sources : Apply rafraichit, jamais cumule. Bannieres debout meme si le heros est mort (J8c-Q3). Jamais un Peasant, camp du proprietaire seul (D705).
- HOLCombatProcessor.cpp GetDamageTakenPercent : branche IsInAura / AuraOutsideLever retiree ; Percent = HOLMorale::TakenPercent(Etat, CryPercent). IsInAura reste (soin, armure du talent Boucliers levees).
RESTE : le cri (CloseRanksMs) dans le processeur ; build + tests ; mesure a 2000 unites (D190) du second parcours (la boucle Sources x unites a maintenant 1 a 1+4 sources par unite, sans break).

RELECTURE_CPP (L20-2) : includes ajoutes : Engine/World.h, Match/HOLMatchConfig.h (FHOLMatchConfig::MaxSlots), Simulation/HOLHeroSubsystem.h (GetBanner const : oui) ; GetWorld() d'un processeur Mass (outer = sous-systeme de simulation dans le test ; en match : a confirmer, si nul pas de banniere posee et le test de match 10.2 sous banniere le dirait) ; FAuraSource aggregate init a 4 valeurs {FVector2D, uint8, double, FMoraleEffect} : ordre des champs respecte ; lambda 1 capture DawnRow, DawnRadiusSquared, &Dawn, &Sources ; HOLMorale::Apply(State, Effect) 2 args (attenuation par defaut 0) ; aucun switch ; Has / MakeEffectFromRow dans HOLCombatProcessor.cpp ne sont plus utilises (HOLMoraleRules.h / TableRows.h restent inclus, sans effet). Test : includes HOLHeroSubsystem.h et HOLHeroXPRules.h ajoutes ; Heroes->BindSteps() et SetXP(0, GetLevelXP(4)) puis un pas avant PlantBanner (comme le test de match) ; Spawn("Hero", 0, 0, 0) suppose que FindHeroUnit(0) le trouve (a confirmer : sinon « the banner planted 16 m from the hero » le dit).

NON VERIFIE : tout (compilation, rouge puis vert) ; que le heros du test soit bien le slot 0 pour PlantBanner ; que d'autres tests de combat sans processeur de moral comptaient sur IsInAura pour les degats recus (un test qui plantait une banniere ou plaçait un heros sans lancer le processeur de moral : chercher « IsInAura / aura » dans Tests\HOLCombat*Tests.cpp) ; que le ProcessingContext reutilise pour deux Run soit sans effet ; mesure D190.
QUESTIONS: aucune
ESSAIS: aucun
BUILDS_DEMANDES: 1 (lot C++ de la session principale, worktree Celeborn)   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs seulement (ce rapport, MoraleInProgress.md ; le reste = autres sessions) : oui. EtapeDefini.md NON mis a jour (budget d'appels) : a faire par la session principale.
