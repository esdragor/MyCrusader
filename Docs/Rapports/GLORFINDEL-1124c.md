# GLORFINDEL-1124c : F10, origine du « grand cercle pale » de faction_aim_free (lecture seule, sans C++)

VERDICT : VOULU, pas un defaut, pas un provisoire. Le grand cercle pale est le **cercle d'aura du heros** (cercle dore fin de 12 m autour du heros vivant, visible du proprietaire seul), decision D549 / J8c-Q4 (GDD.md ligne 2878 : « cercle dore de 12 m autour du heros vivant et de la banniere »). Ce n'est ni l'apercu de visee d'Arwen (HOLFactionAim*), ni un repere de selection, ni la portee de la Lumiere sacree.

PREUVES
- Dessin : `Source\HeroesOfLiberty\Player\HOLSelectionMarkers.cpp:803` `AddCircle(HeroPoint, HOLHeroAbility::AuraRadius, AuraRingWidth, AuraRingColor, Hero->Location.Z);` dans `AHOLSelectionMarkers::UpdateAuraRings()` (l.714), sans condition d'armement : le cercle est present des que le heros du joueur existe. Couleur `AuraRingColor(1.0, 0.82, 0.35)` (:87, or clair, d'ou le « pale » sur l'herbe), epaisseur 10 cm (:88). Rayon `HOLHeroAbility::AuraRadius = 1200.f` (`Source\HeroesOfLiberty\Simulation\HOLHeroAbilityRules.h:33`) = 12 m.
- Image : `Saved_Tests\Screenshots\WindowsEditor\HOL\faction_aim_free_ready.png` (worktree Arwen) : cercle centre sur le heros (pixel ~578,430), rayon ~180 px, present AVANT toute visee ; meme cercle sur `faction_aim_free_aim.png`, avec en plus le cercle VERT de 6 m de l'apercu de visee (celui-la est bien l'ajout d'Arwen, `Script\UI\HOLFactionAim.as:68-77`, PreviewOf).
- Coherence avec RECETTE-1123pc : « visible AVANT d'armer la visee et apres le lancer » = cercle toujours dessine (l.803), comme prevu par D549.

OBSERVATION (pas un defaut, pas de ligne [ ] dans Auto_Journey_File.md)
- Pendant la visee d'un pouvoir de faction, l'anneau de portee `ReachRingColor` (gris clair, :831, `GetFactionAimRangeCm()` = `HOLFactionAim::AimRangeCm` = 1200 cm, `Script\UI\HOLFactionAim.as:32`) a le MEME rayon que l'aura du heros (1200 cm) : les deux cercles se superposent exactement (un seul anneau visible sur le PNG aim). Le joueur ne voit donc pas la portee comme un cercle distinct de l'aura. Coincidence de valeurs (12 m / 12 m), pas un bug ; si l'utilisateur veut distinguer les deux, c'est un choix de design (question a lui), non traite ici.
- Placeholders.txt : aucune ligne a ajouter (cercle d'aura = fonction decidee, pas un provisoire).

APPELS_DECLARES: ~16 (arret a 25 vus ou 11:43)
HEURE_DEBUT/HEURE_FIN: 11:28:31 / 11:32:39 (Get-Date PowerShell)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build, tache en lecture)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (aucun code) ; TEST_ROUGE_SEPARE: sans objet
COMMITS: 0, merge_main_fait: non (aucun commit, worktree Glorfindel intact)
TESTS: aucun ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun liste ; PNG rouverts (ready, aim) a la place de ceux d'Arwen, lus seulement
QUESTIONS: aucune cree
FAIT / RESTE: fait = origine identifiee (fichier:ligne), verdict voulu. Reste = rien ; au besoin l'utilisateur tranche si l'anneau de portee doit differer de l'aura.
NON VERIFIE: le cercle d'aura n'a pas ete relance a l'ecran (lecture du code et des PNG d'Arwen seulement) ; le « pale » vient de la couleur or clair a 10 cm d'epaisseur sur herbe, lu pas mesure.
PROCESSUS_DETACHES: aucun (aucun processus lance)
ANOMALIES_PROCESSUS: aucune ; un hook rtk a fait refuser deux appels Bash par le classifieur (rejoues tels quels).
GIT_STATUS_PRINCIPALE: inchange hors docs voulus : oui (hors mon rapport, seulement des docs ; INTEG-65.txt est apparu pendant la seance, pas de moi)
