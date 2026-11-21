# CELEBORN-1121se : moral, le combat lit le levier « degats recus » (Taken)

NON COMPILE (aucun build, aucun quick_check, consigne). Impact joueur : une unite sous l'Aura de l'Aube recoit 15 % de degats en moins d'une meme frappe, lus sur la ligne DawnAura de Effects.csv et sur le fragment de moral (plus seulement sur la constante du combat) ; le cri s'ajoute dessous le plafond de 50 %.

APPELS_DECLARES: ~18 (arret a 25 vus ; premier Edit apres le 10e appel : brief non tenu d'un appel, lecture du point d'application)
HEURE_DEBUT/HEURE_FIN: 19:44 / ~19:50 (Get-Date PowerShell ; arret dur 20:00 respecte)
PREMIER_BUILD: sans build (NON COMPILE)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de compilation par l'agent)
TEST_ROUGE: 72430f9 (stub HOLMorale::TakenPercent = 100 + test) ; rouge NON VU a l'execution ; assertit l'effet : oui (meme frappe 200 : degats sous aura = degats nus x (100 - Row->Taken)/100, Row lue de Find(DawnAura) ; et parite sur 4 cas aura/cri avec l'ancienne regle HOLHeroAbility::GetDamageTakenPercent)
COMMITS: 4 (72430f9 test + stub, ac4e79e lecture du levier, af83140 declaration anticipee, cba257a banniere sans double compte), dernier a ~19:46, merge_main_fait: non (DLL non compilee)
TESTS: aucun lance. TESTS_TAUTOLOGIQUES: 0 (attendu calcule par la ligne du CSV et par la regle GetReducedDamage / GetDamageTakenPercent, jamais recopie)

ROUGES ATTENDUS : au tip 72430f9 : 1 rouge, HeroesOfLiberty.Morale.CombatReadsTheTakenLever (assertions « the same strike does the row's percent less under the aura » et les 2 « aura 1, cry 0/1 : the lever reads as the old rule » ; « a bare unit takes the whole strike » et « aura 0 » passent deja avec le stub 100 sauf le cas cri). Au tip cba257a : 0 rouge attendu parmi les HeroesOfLiberty.Morale.* ; les tests de match (HOLHeroAbilityMatchTests : 10.2 sous aura, 7.2 aura + cri, 10.2 sous la banniere, un seul compte aux deux sources) doivent rester verts, c'est le point a surveiller.

ESSAIS: aucun
QUESTIONS: aucune (pas de design tranche)
BUILDS_DEMANDES: 0 de plus (lot C++ de la session principale)   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun

FAIT / RESTE:
- FAIT : point unique = UHOLCombatProcessor::GetDamageTakenPercent (HOLCombatProcessor.cpp ~:767), appele par la frappe de melee (:3164), la fleche (:1466) et l'eclat (:1532). Il lit maintenant HOLMorale::TakenPercent(Etat, Autre) : 100 - min(Total(Taken) + Autre, BonusCap(Taken)). Valeur de l'aura = la ligne DawnAura du CSV via le fragment ; aucune constante recopiee.
- FFighter.Morale (const FHOLMoraleFragment*, requete Optional ReadOnly) ; declaration anticipee dans HOLCombatProcessor.h.
- Pas de double compte : l'aura posee par le processeur (fragment) ET l'ancienne IsInAura ne s'additionnent jamais. Transition : IsInAura (banniere plantee, pas encore dans le processeur) compte seulement tant que le fragment ne porte pas DawnAura, au pourcentage de la ligne (MakeEffectFromRow(DawnAura).Taken). Le cri reste a part (CloseRanksMs, CloseRanksReductionPercent).
- RESTE : migrer la Banniere et le cri dans le processeur, puis supprimer la branche IsInAura de GetDamageTakenPercent (IsInAura reste pour le soin et l'armure du talent) ; infliges (Dealt) et cadence (Rate) pas lus par le combat ; mesure a 2000 unites (D190).
RELECTURE_CPP (L20-2) : includes : HOLMoraleRules.h ajoute au .cpp (apporte HOLMoraleTableRows.h pour HOLMoraleDawnAura) ; struct FHOLMoraleFragment declaree en avant dans le .h (membre pointeur const seulement) ; Has(MoraleState, int32) existe dans HOLMoraleRules.h ; Total / BonusCap definis avant TakenPercent dans le .cpp (meme namespace, TakenPercent place avant Multiplier, apres Total : oui) ; static const FHOLMoraleState NoMorale (aggregate a valeurs par defaut, init par defaut const : un const sans initialiseur sur un type a membres par defaut est valide) : a confirmer au build ; GetFragmentView sur un fragment Optional absent donne une vue vide (Morales.IsEmpty() teste) : oui ; aucun switch ; test : lambda aucune, variables Strike float.
NON VERIFIE: tout (compilation, rouge puis vert) ; que les tests de match d'aura / banniere restent verts (le chemin a change) ; que l'ordre processeur moral / combat soit sans effet (il ne l'est pas necessairement : le moral recalcule ClearAuras + aura dans le meme Execute, donc le combat voit un fragment coherent a chaque pas, mais un combat avant le moral lirait l'aura du pas precedent : a verifier dans les ExecuteAfter/Before) ; que le combat de nos tests de combat sans processeur de moral (aura absente du fragment) retombe bien sur IsInAura ; la parite 60 % (aura + cri) du test.
PROCESSUS_DETACHES: aucun (aucun processus lance)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs seulement (ce rapport, MoraleInProgress.md, EtapeDefini.md par moi ; le reste = autres sessions) : oui.
