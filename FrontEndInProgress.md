# Front-end in progress

**Journal de reprise du chantier du menu.** Reprise : **« continue FrontEndInProgress »**.

Chantier mené en parallèle de M2 (unités) et du moteur 5.8.3 avec cible Server (D231). Pour ne pas se marcher dessus :
- décisions numérotées **D240 et suivantes** (D237 à D239 restent à M2) ;
- décisions tenues **ici** pendant le plan, puis reportées au journal du GDD (§ 21) au moment de fusionner, pour ne pas
  éditer `GDD.md` en même temps que les autres sessions ;
- **pas de worktree** (choix de l'utilisateur, 2026-10-13) : le code se fait dans le dossier du projet et se committe
  sur `main`. Règles de cohabitation : ne compiler que si aucun éditeur ni compilation du projet ne tourne (DLL
  verrouillée) ; ne laisser **jamais** de code qui ne compile pas (même module que M2) ; committer par chemins explicites,
  jamais `git add -A` ; ne pas toucher aux réglages globaux qui changent l'environnement du stress test (GameMode global,
  client de viewport) sans le vérifier sur S1.

---

## Où on en est

| Étape | État |
|---|---|
| Plan (Q1 à Q9, D240 à D248) | **fait** (2026-10-13) |
| 1. Socle : plugin Common UI, module `HeroesOfLibertyUI` (client seulement), `Json` dans le module principal | **fait** (2026-10-13) |
| 2. Configuration de partie (`FHOLMatchConfig`, sous-système, GameMode, PlayerState) + premiers tests | **fait** (2026-10-13) : 7 tests verts |
| 3. Socle Common UI (viewport, entrées, styles, police, couches, carte `MainMenu`) | **fait** (2026-10-13) : capture conforme, S1/S2 complets, 7 tests verts, touches vérifiées |
| 4. Écrans (titre, Nouvelle partie, Réglages, confirmation de sortie) | **fait** (2026-10-13) : titre, confirmation de sortie, Nouvelle partie avec lancement, Réglages ; captures et touches vérifiées |
| 5. Localisation (français, anglais, japonais) | **fait** (2026-10-13) : 92 textes traduits en anglais et japonais (provisoires), bascule en direct vérifiée, captures dans les trois langues |
| 6. Vérification (tests d'automatisation, bout en bout en trois langues avec captures) | à faire |
| 7. Clôture (D240 à D247 au GDD, garde-fou du stress test, lien `OwnerPlayer`, commandes utiles) | à faire |

## Prochaine étape exacte

**Étape 6 — vérification** (D246) :
1. Tests d'automatisation à ajouter (module `HeroesOfLibertyUI` ou `HeroesOfLiberty`) : réglages enregistrés puis
   relus (`UHOLGameUserSettings`, volumes) ; toutes les clés de texte présentes et traduites dans les trois langues
   (lecture des archives ou des `.locres` de `Content/Localization/Game`) ; GameMode avec configuration lancée depuis
   l'écran (chemin de `hol.UI.NewGame.Launch`).
2. Script de bout en bout (`Tools/Tests/`) qui enchaîne les lancements de capture dans les trois langues (comme ci-dessous
   « Commandes utiles ») et produit une planche par langue ; liste courte de finition à juger par l'utilisateur.
3. **Toujours** lancer les fenêtres de test avec `-WinX=3940 -WinY=100 -saveddirsuffix=Tests` (voir « Commandes utiles »).

**Règle de tenue** (permanente) : mettre à jour ce fichier à **chaque étape** — tableau ci-dessus, prochaine étape
exacte, entrée de journal — avant de passer à la suivante.

## Commandes utiles

Voir `ImplementationInProgress.md` (« Commandes utiles ») : compilation `HeroesOfLibertyEditor`, lancement du stress
test. **Lancer l'éditeur depuis PowerShell**, jamais depuis Git Bash (conversion des chemins `/Game/...`).

```powershell
# Tests d'automatisation du projet, sans affichage (D246) ; résultat dans Saved/Logs/HeroesOfLiberty.log, lignes « Test Completed »
& "C:\UE\UE_5.8_AS_1\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\Users\baudo\Documents\MyCrusader\HeroesOfLiberty.uproject" `
  -ExecCmds="Automation RunTests HeroesOfLiberty" -TestExit="Automation Test Queue Empty" -unattended -nullrhi -nosplash -nosound -log

# Lancements de TEST : toujours -saveddirsuffix=Tests et la fenêtre sur le DEUXIÈME écran de l'utilisateur.
#  - -saveddirsuffix=Tests : dossier Saved_Tests/ à part (réglages, journaux, captures ; ignoré par git), les tests ne
#    touchent jamais le GameUserSettings.ini de l'utilisateur (relu par le stress test, D245) ;
#  - Saved_Tests/Config/WindowsEditor/GameUserSettings.ini = copie de Tools/Tests/GameUserSettings.ini (fenêtré
#    1600 x 900, DisplayIndex=1) : le moteur réapplique la résolution enregistrée ~15 s après le démarrage et recentre
#    la fenêtre sur l'écran enregistré, -WinX seul ne suffit pas ;
#  - -WinX/-WinY sont en coordonnées logiques (Windows à 150 %) : 3940 = deuxième écran.
# Capture de l'interface par le jeu lui-même (D246) : fenêtre de jeu seulement, jamais le bureau.
& "C:\UE\UE_5.8_AS_1\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\baudo\Documents\MyCrusader\HeroesOfLiberty.uproject" `
  -game -windowed -ResX=1600 -ResY=900 -WinX=3940 -WinY=100 -saveddirsuffix=Tests -nosplash -culture=fr `
  -ExecCmds="hol.UI.Capture title 5 quit"
# -> Saved_Tests/Screenshots/WindowsEditor/HOL/title.png ; journal Saved_Tests/Logs/HeroesOfLiberty.log
# Langue imposée au lancement : -culture=fr|en|ja

# Touches envoyées à l'interface comme par le clavier ou la manette (D246), sans piloter le bureau
#   -ExecCmds="hol.UI.Key Escape 4,hol.UI.Key Gamepad_FaceButton_Right 5,hol.UI.Capture keys_after_back 6.5,hol.UI.Key Enter 9"
#   (Enter et Gamepad_FaceButton_Bottom ouvrent la confirmation de sortie ; Left puis Enter quittent :
#   journal « LogHOLUI: Title screen: Quit »)
# Ouvrir un écran par le même chemin que son bouton (D246) : hol.UI.Open NewGame|Settings[:Onglet]|Quit [Délai]
#   (onglets : 0 Vidéo, 1 Son, 2 Commandes, 3 Langue ; un écran Réglages déjà ouvert est réutilisé)
# Avec -saveddirsuffix=Tests, les réglages changés par un test restent dans Saved_Tests.
# Lancer une partie comme le bouton « Lancer », éventuellement depuis un fichier JSON (format de -MatchConfig=) :
#   hol.UI.NewGame.Launch [Fichier] [Délai]  -> journal « LogHOLMatch: Match on BattleMap: given configuration »

# Localisation (D245) : traductions provisoires de Tools/Localization/fill_po.py écrites dans les .po (signale les
# clés inconnues, c.-à-d. les textes ajoutés depuis), puis collecte, import des .po, .locres, réexport des .po
#   python Tools/Localization/fill_po.py
& "C:\UE\UE_5.8_AS_1\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\Users\baudo\Documents\MyCrusader\HeroesOfLiberty.uproject" `
  -run=GatherText -config="C:\Users\baudo\Documents\MyCrusader\Config\Localization\Game.ini" -unattended -nop4 -nosplash

# Recréer les ressources d'entrée de l'interface (commandlet) et la carte MainMenu (éditeur complet)
#   UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script=<projet>\Tools\Editor\make_ui_input.py -unattended -nop4 -nosplash
#   UnrealEditor.exe <uproject> -nosplash -ExecCmds="py <projet>/Tools/Editor/create_main_menu_map.py"
# Classes de son et mélange des volumes (commandlet) : -run=pythonscript -script=<projet>\Tools\Editor\make_sound_classes.py

# Lancer une partie depuis une configuration JSON (comme un serveur dédié sans salon, D243)
#   ... HeroesOfLiberty.uproject /Game/Level/BattleMap -game -MatchConfig=C:\chemin\match.json
```

---

## Plan (à valider question par question)

| # | Question | État |
|---|---|---|
| Q1 | **Périmètre** du premier chantier menu. | **tranché (D240)** : coquille (titre, réglages, quitter) + écran « Nouvelle partie » en solo produisant une configuration de partie lue par le GameMode |
| Q2 | **Technologie d'interface** : UMG seul, Common UI, Slate. | **tranché (D241)** : Common UI (pile d'écrans activables, couches, routage des entrées) |
| Q3 | **Fabrication des écrans** : widgets Blueprint faits dans l'éditeur, arbres de widgets construits en C++, ou générés par script. | **tranché (D242)** : tout en C++ (arbres de widgets et styles), éléments en `BindWidgetOptional` pour une reprise en Blueprint écran par écran |
| Q4 | **Flux et transmission** : carte de menu dédiée, porteur de la configuration (instance de jeu, options d'URL, ligne de commande du serveur). | **tranché (D243)** : carte `MainMenu` dédiée ; `FHOLMatchConfig` portée par un sous-système d'instance de jeu, trois sources (menu, salon, JSON en ligne de commande), une seule lecture par le GameMode |
| Q5 | **Contenu de la configuration de partie** : champs retenus, ce qui agit déjà et ce qui est grisé. | **tranché (D244)** : seulement ce que le GDD a acté (carte, 8 emplacements avec état, faction, couleur, équipe, mode Standard et trois options), plus graine et version ; carte, emplacements occupés et couleur agissent, le reste grisé |
| Q6 | **Réglages** : lesquels, et où ils sont stockés. | **tranché (D245)** : vidéo et son actifs ; onglet Commandes (remappage D30, confort caméra) présent mais grisé jusqu'à M4 ; langues français (source), anglais et japonais, sélecteur actif |
| Q7 | **Vérification** : comment le chantier est testé (test d'automatisation, lancement de bout en bout). | **tranché (D246)** : tests d'automatisation sur la logique, lancement de bout en bout par script dans les trois langues avec captures lues par l'assistant |
| Q8 | **Module du menu** : module à part ou sous-dossier de `HeroesOfLiberty`. | **tranché (D247)** : module `HeroesOfLibertyUI` (interface), configuration et GameMode restent dans `HeroesOfLiberty` |
| Q9 | **Renommage du projet** (demandé avant de coder) : nom affiché, nom technique, ou tout. | **tranché (D248, au GDD)** : tout renommé en `HeroesOfLiberty`, préfixe `HOL`, variables `hol.*`, sauf le dossier et git ; fait le 2026-10-13 |

**Plan complet (D240 à D246).** Ordre d'implémentation proposé :
1. **Socle** : plugins `CommonUI` et `CommonInput` dans le `.uproject` ; dépendances du module (`UMG`, `Slate`,
   `SlateCore`, `CommonUI`, `CommonInput`, `Json`, `JsonUtilities`).
2. **Configuration de partie** (D243, D244) : `FHOLMatchConfig` (version, graine, carte, 8 emplacements, mode, options),
   `UHOLMatchSubsystem` (sources : menu, `-MatchConfig=` JSON), GameMode de partie qui la lit ou prend les valeurs par
   défaut, `AHOLPlayerState` avec index et couleur de joueur. **Dépendance** : l'octet `OwnerPlayer` des unités (D233) est
   posé par le chantier M2 ; ici on ne fait que l'index de joueur, le lien se fait à la fusion. Premiers tests (D246).
3. **Socle Common UI** (D241, D242) : client de viewport Common UI, données d'entrée, styles en C++, police composite
   (Roboto + sous-police japonaise : `DroidSansFallback.ttf` du moteur, Apache 2.0, déjà sur le disque — sinon Noto Sans
   JP), disposition racine à couches (menu, modale), carte `MainMenu` créée par script, GameMode et PlayerController
   de menu, `GameDefaultMap` → `MainMenu`.
4. **Écrans** : titre (Jouer, Réglages, Quitter), Nouvelle partie (D244 : actifs et grisés), Réglages (vidéo, son,
   commandes grisé, langue — D245), confirmation de sortie en modale.
5. **Localisation** (D245) : cible de localisation, collecte par script, `.po` anglais et japonais, compilation,
   bascule de langue en direct et enregistrée.
6. **Vérification** (D246) : tests d'automatisation complets, script de bout en bout en trois langues avec captures.
7. **Clôture** : D240 à D246 reportés au § 21 du GDD (et § 15, § 16.6) ; garde-fou du stress test
   (forcer et rapporter les réglages du menu) ; lien `OwnerPlayer` ↔ index de joueur si M2 l'a posé ; commandes utiles.

## Décisions du chantier (à reporter au § 21 du GDD)

| # | Date | Sujet | Décision |
|---|---|---|---|
| D240 | 2026-10-13 | Technique — Périmètre du premier chantier menu | **Coquille plus écran « Nouvelle partie » en solo** (choix de l'utilisateur, conforme à la recommandation) : écran titre (Jouer, Réglages, Quitter), réglages, et un écran de préparation façon AoE4 (carte, joueurs avec faction, couleur et équipe, mode et options de D204 et D205) qui remplit une **configuration de partie** lue par le GameMode au chargement. Ce qui n'existe pas encore dans le jeu (factions, modes) est affiché **grisé** ; seuls la carte et le nombre de joueurs agissent. La couleur des joueurs partage la palette côté client de D233. Motif : chantier indépendant des unités, testable tout de suite, et la configuration est le contrat que le **serveur dédié (D231)** recevra au lancement et que le salon de M2.5 remplira à plusieurs. Écartés : la coquille seule (« Jouer » en dur, préparation de partie et passage d'options à refaire) et le flux multijoueur complet d'emblée (dépend de M2.5 et du moteur avec cible Server, codé sans moyen de le tester). |
| D241 | 2026-10-13 | Technique — Technologie d'interface | **Common UI** (choix de l'utilisateur, conforme à la recommandation) : plugin d'Epic présent et stable en 5.8 (version 1.0, hors bêta), utilisé par Lyra. Il apporte la **pile d'écrans activables** avec retour arrière, les **couches** (menu, modale par-dessus), le **routage des entrées** entre interface et monde, des styles centralisés et des boutons communs. Motif : le menu seul ne le justifie pas, mais le HUD de D201 (clics partagés entre interface et carte), le menu pause et la fenêtre de défi de duel (D117) en ont besoin ; on évite de refaire la pile et le routage à M4. Coût assumé : mise en place initiale (client de viewport, données d'entrée, styles) et apprentissage, avec Lyra comme modèle. Écartés : **UMG seul** (pile d'écrans, routage des entrées et modales à écrire à la main, puis à refaire pour le HUD) et **Slate pur** (réservé aux outils de l'éditeur, beaucoup de code pour un HUD riche). |
| D242 | 2026-10-13 | Technique — Fabrication des écrans | **Tout en C++** (choix de l'utilisateur, conforme à la recommandation) : les écrans dérivent des classes de Common UI et **construisent leur arbre de widgets dans le code** ; les styles (boutons, textes) sont des classes C++ ; les rares ressources inévitables (police) sont importées par script d'éditeur, comme D227. Les éléments sont déclarés en **`BindWidgetOptional`** : si un widget Blueprint fournit un jour la mise en page d'un écran, elle est prise, sinon le code la construit — porte ouverte vers une finition graphique écran par écran, sans rien jeter. Motif : pas encore de direction artistique (écrans provisoires de toute façon), tout le chantier reste en texte, écrit, relu en diff et fusionné depuis le worktree par rebase. Coût assumé : pas d'éditeur visuel, mise en forme réglée en code (Live Coding), animations codées. Écartés : **widgets Blueprint faits à la main** (méthode d'Epic et de Lyra, mais travail manuel à chaque écran et fichiers binaires ni écrits ni relus par l'assistant) et **Blueprints générés depuis une description JSON** (outil d'éditeur en C++ à écrire et tenir : les propriétés de `UWidgetTree` ne sont pas exposées aux scripts). |
| D243 | 2026-10-13 | Technique — Transmission de la configuration de partie | **Carte de menu dédiée et sous-système d'instance de jeu** (choix de l'utilisateur, conforme à la recommandation) : une carte `MainMenu` (GameMode de menu) devient la carte de démarrage du jeu (`GameDefaultMap`) ; la carte d'ouverture de l'éditeur et le stress test ne changent pas. La configuration de partie est une **structure Unreal** (`FHOLMatchConfig`) portée par un `UGameInstanceSubsystem` : le menu la remplit puis ouvre la carte de bataille, le GameMode la lit au chargement ; elle survit au changement de carte. **Une seule lecture, trois sources** : le menu en solo, le salon de M2.5 (qui tournera sur le serveur et remplira le même sous-système côté serveur), et un fichier JSON passé en ligne de commande (`-MatchConfig=`) pour un serveur dédié lancé sans salon — gratuit, la structure étant sérialisable. Sans configuration, le GameMode prend des **valeurs par défaut** (le stress test et `BattleMap` continuent de tourner). Écartés : **tout dans l'URL de la carte** (chaîne de texte au format maison, échappement, aucun typage, illisible à 8 joueurs) et **un fichier écrit par le menu** (ne marche pas pour un serveur distant, passe par le disque même en solo). |
| D244 | 2026-10-13 | Technique — Contenu de la configuration de partie | **Seulement ce que le GDD a acté** (choix de l'utilisateur, conforme à la recommandation) : **carte** (liste) ; **8 emplacements**, chacun avec un **état** (fermé, humain, IA), une **faction** (les 6), une **couleur** (palette de D233) et une **équipe** (aucune, 1 à 4) — l'emplacement de départ est l'index de l'emplacement, les cartes étant symétriques (§ 14.4) ; **mode** Standard et trois **options** (Nomade, Régicide, Catastrophe, D204 et D205) ; deux champs techniques non affichés : une **graine aléatoire** (placement de Nomade, reproductibilité) et un **numéro de version** du format (JSON du serveur lisible quand des champs s'ajoutent). **Agissent dès maintenant** : la carte, le nombre d'emplacements occupés et la couleur (le GameMode crée les joueurs et leur donne l'octet `OwnerPlayer` de D233). **Grisés** (D240) : faction, équipe, IA, mode et options, présents dans la structure avec leur valeur par défaut. Motif : aucun champ inventé, forme du contrat stable dès maintenant ; d'autres réglages s'ajouteront un par un, par décision, grâce à la version. Écartés : **les réglages de lobby d'AoE4** (ressources de départ, population maximale, vitesse, carte révélée, positions aléatoires : aucun n'est décidé au GDD, chacun demanderait sa décision de design pour rester grisé des mois) et **le strict minimum** (contraire à D240, contrat qui changerait de forme à chaque étape). |
| D245 | 2026-10-13 | Technique — Réglages | **Vidéo, son, commandes et langue** (choix de l'utilisateur : option B, avec deux précisions). **Vidéo** : résolution, mode fenêtre, synchronisation verticale, limite d'i/s, qualité globale puis par groupe, échelle de rendu ; **son** : volumes général, musique, effets, voix (le général agit tout de suite, les trois autres passent par des classes de son créées par script et agissent dès qu'il y a des sons) ; enregistrés par une classe dérivée de `UGameUserSettings`. **Commandes** *(précision 1, option b, contre la recommandation)* : onglet présent mais **grisé jusqu'à M4** — remappage des actions de D30 (« tout est reconfigurable »), vitesse de la caméra et défilement au bord de l'écran ; les actions Enhanced Input seront déclarées à M4. **Langues** *(précision 2, nuance de l'utilisateur)* : **français** (langue source), **anglais** et **japonais**, « pour tester aussi un alphabet non latin » ; sélecteur actif, bascule en direct, choix enregistré ; tout texte affiché passe par la localisation d'Unreal (`FText`, `LOCTEXT`), collecte par script, traductions en fichiers `.po` relisibles ; la police doit couvrir le japonais (police composite avec une sous-police japonaise sous licence libre, importée par script, D242). Traductions anglaise et japonaise **provisoires**, écrites par l'assistant. Garde-fou du stress test : `GameUserSettings.ini` est relu par le stress test, qui devra **forcer et écrire dans son rapport** tout ce que le menu règle (en particulier l'échelle de rendu), modification faite à la fusion car le fichier appartient à l'autre chantier. Écartés : vidéo et son seuls (recommandation initiale), vidéo seule, onglet Commandes actif dès maintenant (recommandation de la précision 1), français seul avec sélecteur grisé. |
| D246 | 2026-10-13 | Technique — Vérification du chantier menu | **Tests automatiques sur la logique, captures par script pour l'aspect** (choix de l'utilisateur, conforme à la recommandation). **Tests d'automatisation Unreal**, en ligne de commande sans affichage : aller-retour JSON de la configuration et lecture d'une version antérieure du format ; GameMode **sans** configuration (valeurs par défaut, protège le stress test) ; GameMode **avec** configuration (nombre de joueurs, couleur, index de joueur) ; réglages enregistrés puis relus ; toutes les clés de texte présentes dans les trois langues. **Bout en bout par script** : des commandes console ouvrent chaque écran et lancent une partie depuis une configuration par le même chemin que le bouton « Lancer » ; le jeu est lancé en français, en anglais et en japonais et chaque écran est **capturé** ; l'assistant lit les captures (texte coupé, caractère manquant, chaîne restée en français) et les joint au compte rendu ; la finition se juge à l'œil par l'utilisateur sur une courte liste. Écartés : **vérification manuelle seule** (rien de rejouable, contrat JSON et GameMode sans configuration non protégés) et **pilotage de l'interface par clics simulés** (Automation Driver : fragile à chaque changement de mise en page, peu documenté, coût permanent pour des écrans provisoires, D242). Clôt le plan du chantier menu (D240 à D246). |
| D247 | 2026-10-13 | Technique — Module de l'interface | **Un module à part pour l'interface** (choix de l'utilisateur, conforme à la recommandation) : la configuration de partie, son sous-système, le GameMode et le PlayerState restent dans `HeroesOfLiberty` (le serveur en a besoin) ; Common UI, styles, écrans et leurs tests vont dans un module **`HeroesOfLibertyUI`**, qui dépend de `HeroesOfLiberty` et jamais l'inverse, et qui pourra être **exclu de la cible Server** dans le `.uproject` (le serveur dédié de D231 ne charge ni Common UI ni écrans). Motif : sans worktree, c'est l'isolation qui reste face au chantier M2 (même module sinon : même `Build.cs`, une erreur de compilation bloque l'autre session), et la séparation simulation / présentation du § 16.6 s'étend à l'interface. Écarté : un sous-dossier `UI/` de `HeroesOfLiberty` (Common UI chargé par le serveur, `Build.cs` partagé). |
| D249 | 2026-10-13 | Technique — Langue au premier lancement | **Langue de Windows** (choix de l'utilisateur, conforme à la recommandation) : tant que le joueur n'a rien choisi, le jeu prend la langue du système si elle est traduite (anglais, japonais), sinon le français, langue source (repli du moteur sur la culture native de la cible `Game`) ; le choix fait dans Réglages > Langue est enregistré et passe ensuite avant. Comportement standard d'Unreal. Conséquence constatée : sur le Windows anglais de l'utilisateur, le jeu démarre en anglais, et tant que la traduction anglaise n'existe pas, le sélecteur indique English sur des textes français (incohérence levée à l'étape 5). Écarté : toujours le français au premier lancement (un joueur anglophone ou japonais doit chercher sa langue). |

## Journal d'avancement

### 2026-10-13 — Plan du menu ouvert, plan complet (D240 à D246)

Point de départ constaté : aucun menu ; `GameDefaultMap` = `BattleMap`, `GameInstanceClass` par défaut du moteur,
`AHeroesOfLibertyGameModeBase` vide, Common UI non activé.
Vérifié dans `C:/UE/UE_5.8_AS_1` : `Engine/Plugins/Runtime/CommonUI` présent, `IsBetaVersion: false`. Les propriétés de
`UWidgetTree` (`RootWidget`, `AllWidgets`) ne sont **pas** exposées aux scripts : un script Python d'éditeur ne peut pas
construire la mise en page d'un widget Blueprint sans aide en C++ (utile pour Q3).
Le stress test se lance avec une carte explicite (`/Game/Maps/StressTest`) : changer `GameDefaultMap` ne le gêne pas, mais
`StressTest` et `BattleMap` prennent le GameMode global — le GameMode de partie devra tourner **sans configuration** (valeurs
par défaut), sans quoi le stress test casse.
Pas de worktree finalement (choix de l'utilisateur) ; Q8 ajoutée et tranchée (D247). L'utilisateur demande ensuite de
**renommer tout le projet avant de coder** : fait (D248, consigné directement au GDD car il concerne tous les chantiers).
Contenu suivi par git sans LFS (990 fichiers sous `Content`) : un worktree recopie ~3,3 Go. Le moteur fournit
`Engine/Content/Slate/Fonts/DroidSansFallback.ttf` (couvre le japonais) : candidat pour la sous-police japonaise.

### 2026-10-13 (suite) — Étape 1 faite : socle

- `HeroesOfLiberty.uproject` : plugin `CommonUI` activé (il contient `CommonInput`) ; module **`HeroesOfLibertyUI`**
  déclaré en `ClientOnly` (ni compilé ni chargé par une cible Server, D247). Module minimal : `Build.cs` (dépend de
  `HeroesOfLiberty`, `UMG`, `Slate`, `CommonUI`, `CommonInput`), catégorie de log `LogHOLUI`.
- `HeroesOfLiberty.Build.cs` : `Json`, `JsonUtilities` ajoutés (configuration de partie en JSON, étape 2).
- Compilation 80 s ; S2 court : les deux modules et Common UI se chargent, unités et rendu inchangés.
- **Erreur attendue jusqu'à l'étape 3** dans le journal : `LogUIActionRouter: Error: Using CommonUI without a
  CommonGameViewportClient derived game viewport client`. Elle disparaîtra avec le client de viewport Common UI (à
  vérifier alors sur S1, car c'est un réglage global qui touche aussi le stress test).
- Constat pour l'étape 2 : la palette de D233 est locale au processeur de rendu de M2, indexée par `OwnerPlayer`.

### 2026-10-13 (suite) — Étape 2 faite : configuration de partie

- `Match/HOLPlayerColors` : la palette de D233 sort du processeur de rendu de M2 (mêmes 8 couleurs, même indexation
  par `OwnerPlayer`, rendu inchangé) et gagne un nom localisable par couleur (textes source en français, D245).
- `Match/HOLMatchConfig` : `FHOLMatchConfig` (version 1, graine, carte, 8 emplacements état / faction / couleur /
  équipe, mode Standard, options Nomade, Régicide, Catastrophe), `MakeDefault()` (un humain en emplacement 0, couleurs
  distinctes), `Normalize()`, `FindFreeHumanSlot()`, JSON par `FJsonObjectConverter` (noms de champs sans casse,
  énumérations par leur nom) ; un fichier sans version est lu comme version 1, une version plus récente est refusée.
- `Match/HOLMatchSubsystem` (`UGameInstanceSubsystem`) : porte la configuration, lit `-MatchConfig=<fichier>`.
- `Match/HOLPlayerState` : index de joueur (= emplacement, futur `OwnerPlayer`) et index de couleur, répliqués.
- `AHeroesOfLibertyGameModeBase` : lit la configuration à `InitGame` (sinon défaut), donne à chaque humain qui se
  connecte le premier emplacement humain libre. Catégorie de log `LogHOLMatch`.
- **Tests** (`Tests/HOLMatchConfigTests.cpp`, `HeroesOfLiberty.Match.*`) : 7/7 verts — défaut, aller-retour JSON,
  fichier ancien, refus (version future, JSON cassé), emplacements humains, fichier lu par le sous-système, palette.
- **Bout en bout** : S2 court sans configuration → « default configuration », joueur en emplacement 0 ; avec
  `-MatchConfig=` (emplacement 0 en IA) → « given configuration », joueur en emplacement 1, couleur 4. Stress test
  inchangé dans les deux cas.

### 2026-10-13 (suite) — Étape 3 en cours : socle Common UI (interrompue avant les vérifications)

- **Entrées** (D241) : Common UI passe par **Enhanced Input** (`bEnableEnhancedInputSupport=True`), système que
  l'onglet Commandes remappera à M4 (D245). Ressources créées par `Tools/Editor/make_ui_input.py` (commandlet) :
  `/Game/UI/Input/IA_UI_Confirm` (Entrée, Espace, A de la manette), `IA_UI_Back` (Échap, B de la manette), `IMC_UI`.
  `UHOLUIInputData` (dérivée de `UCommonUIInputData`) les référence ; déclarée dans `DefaultGame.ini`.
  Le contexte `IMC_UI` est ajouté au joueur local par `AHOLMenuPlayerController`.
- **Client de viewport** : `GameViewportClientClassName=/Script/CommonUI.CommonGameViewportClient`
  (`DefaultEngine.ini`) ; l'erreur `LogUIActionRouter` de l'étape 1 a disparu. **S1/S2 pas encore revérifiés.**
- **Police** : la police `Roboto` du moteur (`/Engine/EngineFonts/Roboto`) a déjà `DroidSansFallback` en police de
  secours, qui couvre le japonais : rien à importer. À confirmer à l'étape 5 (affichage japonais réel).
- **Styles** en C++ (`Style/HOLUIStyles`) : textes titre, intertitre, corps, grisé ; bouton arrondi, accent au survol.
- **Widgets** construits en code si aucun Blueprint ne fournit de mise en page (D242) : `UHOLButton` (libellé),
  `UHOLRootLayout` (fond + couches `MenuLayer` et `ModalLayer`, `UCommonActivatableWidgetStack`), `UHOLScreen` (base
  des écrans, mode d'entrée menu, `BuildLayout()`), `UHOLTitleScreen` provisoire (titre + « Quitter »).
  Piège : `UCommonButtonBase::Initialize` enveloppe la racine de l'arbre dans son bouton interne juste après
  `NativeOnInitialized` ; l'arbre du bouton doit donc être construit là, avant `Super`.
- **Carte** `/Game/Maps/MainMenu` créée par `Tools/Editor/create_main_menu_map.py` (éditeur complet), surcharge de mode
  de jeu `AHOLMenuGameMode` (pas de pion, `AHOLMenuPlayerController`) ; `GameDefaultMap` → `MainMenu` (l'éditeur
  s'ouvre toujours sur `BattleMap`).
- **Capture** : commande `hol.UI.Capture <Nom> [Délai=3] [quit]` (`Dev/HOLUICapture.cpp`) : capture faite par le jeu
  (fenêtre de jeu et interface seulement) dans `Saved/Screenshots/WindowsEditor/HOL/`. Une première capture du bureau
  par PowerShell avait pris la mauvaise zone (mise à l'échelle de Windows) et une partie du bureau de l'utilisateur :
  supprimée, méthode abandonnée — **ne jamais capturer le bureau**.

### 2026-10-13 (suite) — Étape 3 faite : vérifications du socle Common UI

- **Capture** `title_step3.png` conforme : titre centré en couleur d'accent, bouton « Quitter » stylé, fond sombre.
  Défaut relevé : au clavier et à la souris, le bouton qui a le focus ne se distingue pas (à faire à l'étape 4).
- **Stress test** avec le client de viewport Common UI : S1 et S2 courts (2 000 unités, 3 s) tournent jusqu'au bout,
  rapport complet, 0 unité restante, plus d'erreur `LogUIActionRouter`. Chiffres non significatifs : sept `cl.exe` d'un
  autre projet occupaient le processeur (S1 3,32 ms et 115 i/s, S2 3,56 ms et 109 i/s, seuils en échec).
- **Tests d'automatisation** : 7/7 verts.
- **Touches** : nouvelle commande `hol.UI.Key <Touche> [Délai]` (`Dev/HOLUIKey.cpp`) qui envoie un appui à Slate (même
  chemin qu'une vraie touche : widget qui a le focus, client de viewport, routeur d'actions), relâché 0,1 s après.
  Échap et B de la manette ne font rien sur l'écran titre (écran racine, pas de retour arrière). **Entrée ne faisait
  rien** : Common UI a un réglage `CommonButtonAcceptKeyHandling` dont la valeur codée, `Ignore`, fait ignorer à un
  bouton qui a le focus les touches de validation de Slate (Entrée, Espace, A) ; Epic met `TriggerClick` dans les
  projets créés depuis la 5.6, le nôtre est plus ancien. Passé à **`TriggerClick`** (`DefaultGame.ini`) : Entrée et A
  activent « Quitter » (journal `Title screen: Quit`, sortie propre). `IA_UI_Confirm` reste l'action affichée comme
  icône de clic et routée pour la manette ; ce n'est pas elle qui clique au clavier.

### 2026-10-13 (suite) — Étape 4, partie 1 : focus, sélecteur, titre complet, confirmation de sortie

- **Focus visible** : un bouton qui reçoit le focus prend le style `UHOLButtonStyleFocused` (contour couleur
  d'accent, mêmes dimensions), son style revient quand il le perd (`HOLUIStyle::ShowFocus`). La largeur minimale des
  boutons est retirée du style (les largeurs viennent de la mise en page), sans quoi le changement de style changeait
  la taille. Le rectangle de focus bleu de Slate, dessiné après une navigation au clavier par-dessus le nôtre, est
  coupé : `RenderFocusRule=Never` (`DefaultEngine.ini`).
- **`UHOLRotator`** (dérivé de `UCommonRotator`) : « ‹ valeur › », Gauche / Droite pour changer, clic ou Entrée pour
  la valeur suivante, grisé par `SetIsEnabled(false)` ; servira à Nouvelle partie et Réglages.
- **`UHOLScreen`** : Retour (Échap, B) ferme l'écran par défaut (`bIsBackHandler`), sauf l'écran titre ; aides pour
  une ligne libellé + sélecteur, une rangée de boutons, une colonne de dialogue sur voile.
- **`UHOLConfirmDialog`** : question oui / non dans la couche modale (`bIsModal`), focus sur Non, Retour ferme.
- **Écran titre** : Jouer et Réglages (grisés jusqu'aux parties 2 et 3), Quitter ouvre « Quitter le jeu ? ».
- **`hol.UI.Open Quit`** (`Dev/HOLUIOpen.cpp`).
- **Vérifié par touches et captures** (`p1_*.png`) : focus visible, accents affichés (« Réglages »), Entrée ouvre la
  confirmation, Échap et B la ferment et rendent le focus à Quitter, Gauche / croix gauche puis Entrée / A quittent.

### 2026-10-13 (suite) — Étape 4, partie 2 : écran « Nouvelle partie » et lancement

- **`UHOLNewGameScreen`** (D244) : carte (liste en configuration, `[/Script/HeroesOfLibertyUI.HOLNewGameScreen]
  +Maps=` dans `DefaultGame.ini`, nom de l'asset en attendant de vrais noms), grille de 8 emplacements (joueur 1 =
  « Vous » ; état Fermé / Humain, l'IA est sautée ; faction et équipe grisées ; nuancier + couleur active pour les
  emplacements occupés), ligne mode Standard + Nomade, Régicide, Catastrophe grisés, note des grisés, Retour, Lancer.
  L'écran garde sa `FHOLMatchConfig` (celle du dernier lancement à l'ouverture) ; deux emplacements occupés n'ont
  jamais la même couleur (rotation qui saute les couleurs prises, couleur déplacée à l'ouverture d'un emplacement).
  Widgets d'emplacement retrouvés par leur nom (`SlotColor3`…) pour qu'une mise en page Blueprint puisse les fournir.
- **Lancer** : configuration normalisée, graine tirée de l'heure, `UHOLMatchSubsystem::SetConfig`, puis ouverture de
  la carte. Avant, le routeur de Common UI est mis en mode **jeu** (souris capturée), comme une carte ouverte
  directement : à la destruction des écrans, il efface sa configuration sans rendre la capture de la souris.
- « Jouer » activé sur l'écran titre ; `hol.UI.Open NewGame`, `hol.UI.NewGame.Launch [Fichier] [Délai]`.
- **Vérifié** (fenêtre sur le deuxième écran, à la demande de l'utilisateur) : Entrée sur Jouer ouvre l'écran (focus
  sur Lancer) ; lancement depuis un fichier (emplacements 0 humain couleur 3, 1 humain couleur 5, 2 IA) →
  `Match on BattleMap: given configuration, 3 occupied slots`, `holds slot 0 (colour 3)`, routeur `Menu → Game` ; la
  bataille s'affiche (`BattleMap` signale éclairage et navmesh à reconstruire, sans rapport avec le menu).
  Navigation : Haut depuis Lancer remonte la colonne Couleur, Tab passe d'une colonne à l'autre, ouvrir un emplacement
  lui donne la première couleur libre (captures `p2_*.png`).
- **Limite connue** (écrans provisoires, D242) : Gauche / Droite changent la valeur d'un sélecteur, donc ne font pas
  changer de colonne ; au clavier Tab le fait, à la manette chaque colonne active s'atteint par Haut depuis le bouton
  du dessous (Retour → Emplacement, Lancer → Couleur). À reprendre à la finition.

### 2026-10-13 (suite) — Étape 4, partie 3 : écran « Réglages » ; étape 4 faite

- **`UHOLGameUserSettings`** (module principal, `Settings/`, déclarée par `GameUserSettingsClassName` dans
  `DefaultEngine.ini`) : volumes général, musique, effets, voix enregistrés ; le général agit sur le périphérique audio
  (`SetTransientPrimaryVolume`), les trois autres par des surcharges de classe dans le mélange `SCM_Volumes`, poussé
  une fois. Ressources `/Game/Audio/SC_Music`, `SC_Effects`, `SC_Voice`, `SCM_Volumes` créées par
  `Tools/Editor/make_sound_classes.py`. Volumes appliqués à l'ouverture du menu et à chaque changement.
- **`UHOLSettingsScreen`** : onglets (boutons sélectionnables et basculables, sans quoi un onglet ne se désélectionne
  pas) et `UWidgetSwitcher`. **Vidéo** en deux colonnes : résolution (plein écran prises en charge, ≥ 1280 × 720),
  affichage, synchro verticale, images par seconde (30 à 240, illimitée), échelle de rendu (50 à 100 %) ; qualité
  globale (« Personnalisée » affichée quand les groupes diffèrent, jamais choisie) et 11 groupes. **Son** : 4 volumes
  par pas de 10 %. **Commandes** : grisé (changer les touches, vitesse de caméra, défilement au bord). **Langue** :
  Français, English, 日本語 (noms jamais traduits), `SetCurrentLanguage(…, enregistré)`.
- **Piège corrigé** : appliquer tous les réglages à chaque changement (`ApplySettings`) remettait la résolution et le
  mode d'affichage enregistrés (fenêtre de test passée en 2560 × 1440 sans bordure, possiblement sur l'écran principal
  de l'utilisateur). La résolution et le mode ne sont appliqués que quand ce sont eux qui changent ; le reste par
  `ApplyNonResolutionSettings` + `SaveSettings`.
- « Réglages » activé sur l'écran titre ; `hol.UI.Open Settings[:Onglet]`.
- **Vérifié** (captures `s_*.png`, fenêtre restée en 1600 × 900) : quatre onglets lisibles, un seul sélectionné ;
  volume général 100 → 90 % (journal `LogHOLSettings: Volumes: master 0.90`) et relu au redémarrage ; langue passée au
  japonais, enregistrée et relue au démarrage (`Overriding language … (ja)`, retombe sur l'anglais faute de
  traduction, étape 5) ; « 日本語 » affiché (la police de secours couvre le japonais) ; Échap revient au titre.
  S2 court : complet, 0 unité restante (3,62 ms, 115 i/s, processeur toujours chargé par un autre projet, même ordre
  que le matin) ; 7 tests verts. `GameUserSettings.ini` restauré après les essais.
- **Constat pour la clôture (garde-fou D245)** : avec la classe dérivée, les réglages vont dans la section
  `[/Script/HeroesOfLiberty.HOLGameUserSettings]` ; l'ancienne `[/Script/Engine.GameUserSettings]` n'est plus lue
  (réglages vidéo enregistrés remis à zéro une fois). `[ScalabilityGroups]` reste commune, et le stress test force sa
  résolution et sa qualité ; il devra aussi forcer et rapporter l'échelle de rendu et la synchro (étape 7).
- **Limite connue** : « Affichage » montre le mode enregistré, pas celui imposé par `-windowed` en ligne de commande.

### 2026-10-13 (suite) — Essai de l'utilisateur : fenêtre de test mal placée, langue ; étape 5 commencée

- **Session ouverte pour l'utilisateur** (menu complet). Retours : « on est bon » sauf deux points.
- **La fenêtre de test s'ouvrait au milieu de l'écran principal**, pour tous les essais lancés avec `-WinX=5900`
  depuis la demande de l'utilisateur. Cause : Unreal place la fenêtre en coordonnées **logiques** (Windows à 150 %
  sur les deux écrans : deuxième écran logique de x = 3840 à 5760), et une position hors de tout écran est recentrée
  sur l'écran principal. Vérifié en lisant le rectangle de la fenêtre (sans capture du bureau) : `-WinX=4000` → x
  physique 5988. Valeur corrigée : **`-WinX=3940 -WinY=100`**.
- **Réglages > Langue affichait English** alors que les textes sont en français : Windows est en anglais (`Using OS
  detected language (en-US)`), et sans données de localisation le moteur retombait sur `en` de toute façon. Décision
  **D249** (langue de Windows au premier lancement). La session de l'utilisateur avait enregistré `Language=en` dans
  `GameUserSettings.ini` : ligne retirée (rien d'autre touché), pour que la règle de D249 s'applique.
- **Cible de localisation `Game`** (`Config/Localization/Game.ini`) : collecte des `LOCTEXT` de `Source/` (tests
  exclus), manifeste, archives, import des `.po`, `.locres`, réexport des `.po` ; français natif, anglais et japonais.
  Premier passage : 92 textes, `Content/Localization/Game/{fr,en,ja}` (archives, `.po`, `.locres`) et
  `Game.locmeta`. Au démarrage : `No specific localization for 'en-US' exists, so 'en' will be used`.

### 2026-10-13 (suite) — Étape 5 faite : traductions ; fenêtres de test enfin sur le deuxième écran

- **Traductions** anglaise et japonaise **provisoires** des 92 textes, dans une table clé → (anglais, japonais) de
  `Tools/Localization/fill_po.py`, qui remplit les `msgstr` des `.po` et signale les clés inconnues ; `GatherText`
  importe, compile et réexporte (traductions conservées). Noms de factions inventés pour l'occasion (le GDD n'a que
  les identifiants `Faction.OrderOfDawn`…) : Order of the Dawn / 暁の騎士団, Black Legions / 黒の軍団, Children of the
  Dragon / 竜の子ら, Circle of Shadow / 影の結社, Heirs of Fire / 炎の継承者, Templars / テンプル騎士団 ; options en
  transcription façon AoE (ノマド, レジサイド, カタストロフィ).
- **Vérifié** : captures de chaque écran en anglais, japonais et français (`-culture=`), aucun texte resté en français,
  aucun caractère manquant (la police de secours du moteur couvre le japonais, sans gras) ; **bascule en direct**
  français → anglais depuis l'onglet Langue : onglets, libellés, sélecteurs et écran titre passent en anglais sans
  redémarrer. Débordements corrigés : libellés Vidéo 280 → 320, libellés d'options 130 → 170, sélecteur de mode
  170 → 200, « ボーダーレスウィンドウ » → « ボーダーレス ».
- **Fenêtre de test sur l'écran principal, deuxième fois** (signalé par l'utilisateur). Mesure de la position de la
  fenêtre dans le temps : créée sur le deuxième écran, puis **recentrée sur l'écran principal vers 15 s**. Le moteur
  réapplique la résolution enregistrée au démarrage (`ApplyResolutionSettings` → `FSceneViewport::ResizeFrame`) ; en
  mode fenêtré, toute demande recentre la fenêtre sur l'écran enregistré (`DisplayIndex`, le principal par défaut).
  Correction : les lancements de test passent par **`-saveddirsuffix=Tests`** (dossier `Saved_Tests/` ignoré par git)
  avec un `GameUserSettings.ini` qui désigne le deuxième écran (`DisplayIndex=1`, modèle suivi
  `Tools/Tests/GameUserSettings.ini`) ; vérifié : la fenêtre reste sur le deuxième écran pendant 25 s. Bénéfice
  annexe : les tests ne réécrivent plus le `GameUserSettings.ini` de l'utilisateur.
