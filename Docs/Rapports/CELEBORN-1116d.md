APPELS_DECLARES: 21 (vus par moi)
HEURE_DEBUT/HEURE_FIN: 16:38 / 16:44
PREMIER_BUILD: sans C++   BUILD: aucun (D830)   LAUNCHCHECK_DEBUT: 16:38, sortie vide (binaire de HeroesOfLiberty-Galadriel à jour après merge main 36314b8)
TEST_ROUGE: vu a 16:41 : scénario nomad_start modifié (Z puis E), préflight FAIL « Test sequence: timeout at step 43: waitlog Construction: slot 0 placed MainCentre foundation at » ; puis UN commit vert 84f2b30 (réglage + scénario). Assertit l'effet : oui (fondation du Forum réellement posée par vraie touche E + vrai clic, journal + PNG).
COMMITS: 2 (84f2b30 correctif + scénario, 5c1fc95 merge main), dernier à 16:43, merge_main_fait: oui
TESTS: preflight nomad_start rouge 16:41 (85 s, FAIL timeout étape 43), vert 16:43 (PASS 50 s, 4 PNG) ; aucun quick_check (aucun C++, aucun .as touché) ; TESTS_TAUTOLOGIQUES: 0 (la case est lue par le vrai jeu : touche Slate E -> grille -> fantôme -> clic -> « placed MainCentre foundation »)
ESSAIS: nomad_start : preflight PASS 16:43, 4 PNG ouverts (wide : aucun bâtiment, unités dispersées ; grid : page II, E « Centre principal » en clair, C vide ; forum : fondation fil de fer, Bois 200 Pierre 200 ; site : chantier, « 2 en construction · 3 en marche »), VU À L'ÉCRAN ; ligne « Rejoué après D831 84f2b30 » ajoutée à Retours\E-nomad_start.md (Reponse gardée) ; commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 nomad_start -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
QUESTIONS: aucune
FAIT / RESTE:
- Où : la case n'est ni dans un CSV ni en Angelscript : c'est le cvar C++ hol.Nomad.ForumCell (Source\HeroesOfLiberty\Buildings\HOLBuildingTypes.cpp:61, défaut 10 = C), lu par UHOLBuildingSettings::GetBuildableTypesForSlot (l. 72-82).
- Correctif SANS C++ : Config\DefaultEngine.ini [SystemSettings] hol.Nomad.ForumCell=2 (E ; libre en page II : Marché A, Atelier Q, Chapelle S, Écurie D, Forge W). Journal : « CVar [[hol.Nomad.ForumCell:2]] deferred » puis appliqué au chargement du module.
- Scénario nomad_start.txt / .look : Z puis E (vraies touches), textes « puis E ».
- RESTE (C++ ou migration, D830, écrit dans la ligne D831 de la file) : aligner le défaut 10 -> 2 et le commentaire de HOLBuildingTypes.cpp:60-62, ou porter GetBuildableTypesForSlot en Angelscript.
- Branche galadriel-tab PRÊTE (non fusionnée dans main).
NON VÉRIFIÉ: rejeu par un autre agent sur le build de main fusionné (règle 19) ; partie en réseau (client : le cvar vient du même DefaultEngine.ini, non essayé) ; kit empaqueté (DefaultEngine.ini embarqué, non vérifié) ; l'infobulle de la case E non lue ; tests C++ HOLCommandGridTests utilisent la case 10 en dur (indépendants du cvar, non relancés).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune ; le préflight rouge garde -AllowUncommitted (fichiers du scénario non commités à ce moment) ; la fondation se pose en (-15200, -3600) contre (-14800, -3600) à la fiche précédente (même clic, binaire différent de celui de Celeborn ; non cherché).
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, M Retours/E-nomad_start.md (voulus) ; ?? Docs/Priorite_Socle_Factions_2026-11-16.md, ?? Docs/Rapports/, ?? Retours/Q-alert_population_visible.md (déjà là au départ) : inchangée hors docs voulus : oui
