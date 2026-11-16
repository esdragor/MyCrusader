# ARWEN-1116c : make_audio_compare.py pose le bâtiment (check_audio_compare vert)

APPELS_DECLARES: 27
HEURE_DEBUT/HEURE_FIN: 15:53 / 16:00 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: aucun build (sans C++)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: vu à 15:55 : `check_audio_compare: FAIL (7)` (3 lignes neuves « the generator does not place the Barracks / ArcheryRange / Stable » + 4 « not the generator's output ») ; test et correctif ensemble dans 981d31f (règle L15-5) ; assertit l'effet : oui (sortie RÉELLE de make_audio_compare.py dans un dossier temporaire : une ligne `hol.Cheat.SpawnBuilding <classe> 0 X Y`, la ligne `waitlog Cheat: spawned <classe> of slot 0` et `hol.Camera.FocusNear X Y` aux MÊMES coordonnées)
COMMITS: 3 + merge, dernier à 15:59 (981d31f test + générateur + audio_compare_buildings.txt ; 01dbfb8 fin de fichier de barracks.look ; 045a45a AudioBuildingsInProgress ; merge main bf25d37) ; merge_main_fait: oui
TESTS: check_audio_compare rouge FAIL (7) 15:55 -> PASS 15:57, PASS encore après merge main 16:00 (19 scénarios, 15 lignes de cue) ; quick_check non lancé (aucun C++, aucun .as touché) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun lancé (consigne : l'utilisateur joue les 3 essais audio depuis Arwen ; aucun jeu, aucun preflight) ; fiches Retours\E-audio_compare_buildings_*.md NON touchées ; HOLAudioTable.as NON touché
QUESTIONS: aucune
FAIT / RESTE:
- Tools\Audio\make_audio_compare.py : table BUILDING (scénario -> classe, phrase « <bâtiment> que tu ecoutes, fini(e) et a toi, pose(e) ») ; pour ces scénarios le générateur écrit lui-même le commentaire, `hol.Cheat.SpawnBuilding <classe> 0 -3800 4600`, `waitlog Cheat: spawned <classe> of slot 0`, `hol.Camera.FocusNear -3800 4600` (sinon `FocusNear -900 300` comme avant). Table TEXT : textes du tour 6 (titre, ligne source, notes intro/réponse/fin, ligne du .look) ; clé absente = texte général. Caserne : « what » passé du tour 5 au tour 6.
- Tools\Audio\check_audio_compare.ps1 : assertion neuve (scénario d'un seul bâtiment Barracks/Range/Stable = pose + waitlog + caméra aux mêmes X Y) ; comparaison insensible aux fins de ligne (git sort les scénarios en CRLF, .gitattributes eol=crlf, le générateur écrit LF : la copie de travail CRLF de barracks.txt échouait même avec un contenu égal).
- Régénération des 3 scénarios joués (`--only`), comparaison fins de ligne normalisées : `barracks/range/stable .txt` IDENTIQUE, `.json` IDENTIQUE, `range/stable .look` IDENTIQUE ; seul `barracks.look` change, et seulement sa fin de fichier. `git diff` complet après régénération :
```
diff --git a/Tools/Scenarios/audio_compare_buildings_barracks.look b/Tools/Scenarios/audio_compare_buildings_barracks.look
@@ -1,2 +1,2 @@
 # <nom de capture> | ce que l'utilisateur doit voir
-audio_compare_buildings_barracks_start | OBJET : la Caserne au centre de l'ecran (posee par hol.Cheat.SpawnBuilding, pas le Forum) ; ... Entree = variante suivante
\ No newline at end of file
+audio_compare_buildings_barracks_start | OBJET : la Caserne au centre de l'ecran (posee par hol.Cheat.SpawnBuilding, pas le Forum) ; ... Entree = variante suivante
```
  (aucune ligne de diff pour les .txt : le jeu lit exactement les mêmes pas)
- Tools\Scenarios\audio_compare_buildings.txt (« tout », non compté, non joué) régénéré : il citait encore les tiges du tour 5 (s_voice5_*, 80 sons) -> tiges du tour 6 (s_barr6_*, s_shot6_*, 70 sons).
- File : aucune ligne sans C++ restante pour Arwen (furtivité = C++, D641 = C++) ; ligne audio mise à jour (RESTE : élection des lettres dans HOLAudioTable.as, session principale).
- Branche d751-root prête (rien poussé, rien fusionné dans main).
NON VÉRIFIÉ: aucun jeu ni preflight lancé (l'utilisateur joue) : le pose du bâtiment n'est vérifié que par le texte généré, identique à celui que le preflight de 1116b a vu à l'écran ; quick_check non lancé ; la régénération en place a eu lieu à 15:58 pendant le jeu de l'utilisateur (contenu identique, seule la fin de ligne de barracks.txt passe CRLF -> LF dans la copie de travail).
PROCESSUS_DETACHES: `stray_processes.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` (SANS -Stop) : « aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Arwen »
ANOMALIES_PROCESSUS: (1) le dernier geste `stray_processes.ps1 -Stop` a été REFUSÉ par le classifieur : à raison, il aurait arrêté l'éditeur dans lequel l'utilisateur joue depuis Arwen ; relancé sans -Stop (liste seulement). La règle 20 devrait dire « sans -Stop quand l'utilisateur joue depuis ton worktree ». (2) ATTENTION : le `git merge main` final (bf25d37) a apporté du C++ (HOLGameScreen, HOLScriptScreen, HOLPopulationAlertThreadTests : 8 fichiers Source) : le binaire d'Arwen est désormais plus ancien que ses sources ; la partie en cours n'est pas touchée, mais un prochain play.ps1 / preflight depuis Arwen peut être refusé par LaunchCheck (binaire périmé) : build d'Arwen à faire par la session principale après les essais de l'utilisateur (je n'ai pas compilé : tâche sans C++).
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, M Auto_Journey_Jour.md, M EtapeDefini.md, ?? Docs/Priorite_Socle_Factions_2026-11-16.md, ?? Docs/Rapports/ ; inchangée hors docs voulus : oui (Auto_Journey_File.md et EtapeDefini.md = mes lignes ; le reste ne vient pas de moi)
