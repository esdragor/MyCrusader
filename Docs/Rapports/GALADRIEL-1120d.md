# GALADRIEL-1120d : 3b (i)(ii), vue HOLFactionSlot dans l'ecran de jeu

NON COMPILE : aucun build, aucun quick_check (consigne). Rouge non vu a l'execution.

APPELS_DECLARES: 24 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 09:50 / 09:56
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 1660464 (Test_FactionSlotFromGameStateValue + stub qui renvoie une vue vide) ; assertit l'effet : oui (jauge ecrite dans un vrai AHOLGameState de monde de test, relue par FromGameState). Rouge attendu a l'execution, NON VU.
COMMITS: 3 (specialization-trial, au-dessus de 8fe8b75), dernier 09:55, merge_main_fait: non
  - 1660464 Test + stub (HOLFactionSlot.as, HOLFactionSlot_Test.as)
  - a651d0c FactionOfText + FromGameState (correctif (i))
  - 0917eb8 (ii) widgets + RefreshFactionSlot + ligne hol.HUD.Report (HOLGameScreenScript.as, +64 lignes)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (le texte attendu est calcule par la chaine Make + Gain, ecrit dans l'etat puis relu)
ESSAIS: aucun (pas de scenario faction_slot)
QUESTIONS: aucune

FAIT
- (i) `HOLFactionSlot::FromGameState(State, Slot, FactionText, bHasSpecButton, bTierOpen)` : valeur `State.GetFactionResource(Slot)` tenue dans [0, HOLDawnValues::HonorCap()], lignes `HOLDawnPowers::Rows()` si la faction a une jauge, recharges 0, Tier 0 ; spectateur, pas d'etat ou texte inconnu = "slot empty". `FactionOfText` : faction deduite du texte de `GetSlotFactionText` (radicaux ASCII Aube / gions / Dragon / Ombre / Feu / Templiers).
- (ii) `BuildFactionSlot` (appele apres BuildHeroBlock) + `RefreshFactionSlot` (appele dans Refresh() entre RefreshHero et RefreshDuel) : panneau coin bas gauche + `FactionSlotRise` 150 px, texte "30 / 100", barre, une ligne par pouvoir (Describe de la boite) ; ligne "faction slot: <Describe>" dans `Describe()` pour hol.HUD.Report.
- Placeholders.txt : ligne ajoutee (look provisoire).

RESTE / pour un scenario `faction_slot` (pre-vol session principale)
1. Compiler les scripts (un build/chargement) : le code (ii) n'a jamais ete analyse par le compilateur AS (`Gameplay::GetGameState()`, `FMargin.Bottom +=`, `SizedBar`, `FactionOfText(..., EHOLFaction&)`) ; voir le rouge puis le vert de Test_FactionSlotFromGameStateValue.
2. Scenario `faction_slot` : partie Aube, `hol.Cheat.Honor 30` (CELEBORN-1120b), `waitlog`, puis `hol.HUD.Report` doit contenir "faction slot: slot gauge 30 / 100 powers SacredLight cost 25 ready" (ou "short" selon `StateOf`), PNG du coin bas gauche : verifier que le slot ne chevauche pas le bloc du heros (150 px provisoire) ; rejouer avec un slot Legions : aucun panneau.
3. `bHasSpecializationButton` = false partout : NON TROUVE de widget C++ de bouton de specialisation dans `Source\HeroesOfLibertyUI\HUD` (seulement la commande de grille `Specialize` / `CancelSpecialization`). La coexistence D201/D823 n'est pas tranchee ici.
4. Recharges : 0 en attendant `PowerCooldownEndMs` (ELROND-1120c) ; Tier 0.
5. C++ a signaler (lot groupe) : un accesseur exposant `EHOLFaction` (ou la cle) du slot au script ; aujourd'hui la faction passe par le texte francais affiche, fragile si la table de textes change. TYPES_A_EXPOSER: accesseur faction par slot. BUILDS_DEMANDES: 1 (chargement des scripts).
6. Pouvoirs "boites" : une seule ligne de texte par pouvoir dans un TextBlock, pas de UBorder par boite (look non decide).

NON VERIFIE: tout le code (aucune compilation) ; le contenu exact de `GetSlotFactionText` pour les 6 factions (lu dans le .cpp seulement) ; la position visuelle ; la presence d'un bouton de specialisation permanent ; que `Gameplay::GetGameState()` rend l'etat dans un widget.

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: le 1er appel d'ecriture a utilise des chemins relatifs avec [IO.File] : le repertoire du processus etait la copie principale, donc stub + test y ont ete ecrits (2 fichiers .as modifies dans MyCrusader) alors que git du worktree voyait "nothing to commit". Signale par la session principale ; contenu reporte dans le worktree (1660464), puis `git -C MyCrusader checkout --` des 2 fichiers : `git status --short -- Script Source Tools` de la copie principale est vide. Aucune autre ecriture de code dans la copie principale. Je n'ai pas verifie IN_USE.txt (non mis a jour).
GIT_STATUS_PRINCIPALE: hors docs voulus, inchange (Script, Source, Tools propres) : oui. Docs modifies par moi : AubeInProgress.md, EtapeDefini.md, Auto_Journey_File.md, Placeholders.txt (lignes ajoutees en fin de fichier via -Append, a replacer a la main).
