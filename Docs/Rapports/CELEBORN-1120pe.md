NON COMPILE (aucun build, aucun quick_check, aucun editeur : C++ et script jamais compiles)

APPELS_DECLARES: ~22 (arret a 25 vus ; 16:20 non atteint)
HEURE_DEBUT/HEURE_FIN: 16:05 / 16:09 (Get-Date)
PREMIER_BUILD: sans build (NON COMPILE)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build demande)
TEST_ROUGE: 96a2479 (Script\Tests\HOLMoraleLibrary_Test.as, 3 tests + bouchon HOLMoraleLibrary.cpp qui compile et ne fait rien) ; rouge NON VU a l'execution ; assertit l'effet : oui (Total lu apres deux Apply = un seul effet de 25, Total retombe a 0 apres Tick, plafond lu de NativeBonusCap, effet d'une ligne de table compare champ par champ a HOLMoraleTable::Effect)
COMMITS: 3 sur duel-rewards (96a2479 bouchon + tests rouges, 37a91ed corps + habillage, 7077407 journal), dernier 16:09, merge_main_fait: oui (deja a jour)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ : HOLMoraleRules + HOLMoraleLibrary ; Tools\build.ps1 -LiveCoding=0 -Wait) ; puis quick_check -Parts tests
TESTS_SCRIPT_AJOUTES: 3 (Test_MoraleLibraryApplyRefreshTotalTick, Test_MoraleLibraryCapsAndImmunity, Test_MoraleTableCppRowsAgreeWithCsv)
TYPES_A_EXPOSER: FHOLMoraleEffectData, FHOLMoraleStateData (USTRUCT dans HOLMoraleLibrary.h) ; UHOLMoraleLibrary (19 fonctions Native*)
FAIT / RESTE:
 - FAIT : Source\HeroesOfLiberty\Simulation\HOLMoraleLibrary.h/.cpp : pont sans regle (ToRule / ToData / WriteBack, puis appel de HOLMorale::*). Etat en UPARAM(ref) (un script ne passe pas de int32&), codes Apply* retournes.
 - FAIT : Script\Morale\HOLMoraleRules.as reduit a un habillage (HOLMorale::Apply, Total, Multiplier, Tick, ... appellent HOLMorale::Native*). DOUBLON SUPPRIME : oui (plus de Strongest, de plafonds, de boucles ; seules restent les constantes de contrat Lever* / Apply* et Count = Effects.Num()).
 - Signatures : types renommes par sed dans HOLMoraleTable.as, HOLMoraleBlock.as, Tools\Data\make_effects.py (--check : ok), et les tests Rules / Table / Block / Recipients / SocleLibraries / UnitAbilityRules : HOLMorale::FHOLMoraleEffect -> FHOLMoraleEffectData, HOLMorale::FHOLMoraleState -> FHOLMoraleStateData, HOLMorale::AttenuationCap -> HOLMorale::NativeAttenuationCap(). Aucun test supprime.
 - RESTE : ApplyEffectAround (non fait, temps) ; sous-etape 3 (fragment, en-tete central) non commencee.
NON VERIFIE: rien compile. (1) L'espace de noms script d'UHOLMoraleLibrary est SUPPOSE `HOLMorale` (le plugin retire U et Library : cas de HOLConstructionRead::TryPay) ; si le script plugin et le namespace script `HOLMorale` du meme nom entrent en conflit, renommer la classe ou l'espace (le prefixe Native evite deja le conflit de noms de fonctions). (2) UPARAM(ref) sur un USTRUCT contenant un TArray : modele HOLConstructionReadLibrary.h:127. (3) `"row " + Id` (string + int) dans le test de table. (4) Les constantes Lever* / Apply* du script sont une COPIE des constantes C++ (contrat d'interface, pas de regle) ; aucun test ne verifie leur accord (a ajouter si besoin). (5) Un etat script de plus de 4 effets est tronque a 4 au passage dans la regle (Apply refuse deja un 5e). (6) Tous les points du rapport CELEBORN-1120pd (HOLMoraleTableRows.h jamais compile, ApplyNoRoom=4, etc.). (7) Diagnostics Angelscript non relus (pas d'editeur) : DIAGNOSTICS_AS non verifie.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune (premier Edit refuse « not read » sur HOLMoraleRules.as, lu avec Read limit 1 : EDITS_REFUSES: 1)
GIT_STATUS_PRINCIPALE: EtapeDefini.md et Auto_Journey_File.md modifies par moi (une ligne chacun, CELEBORN-1120pe) ; reste preexistant : oui
