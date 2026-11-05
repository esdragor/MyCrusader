# Audio — avancement

Chantier des sons du jeu, ouvert le 2026-11-03 (copie principale `main`, journal seul, aucun code). Reprend la ligne « Refaire tous les sons du jeu » d'`EtapeDefini.md` (demande de l'utilisateur, 2026-10-29). Chaque étape est notée ici dès qu'elle avance, et `EtapeDefini.md` est mis à jour en même temps.

## Reprise (« continue AudioInProgress »)

État au 2026-11-05 : A0, A1 faites ; **A2 (table événement vers son) faite en script dans le worktree Arwen, branche `ai-ia1`** (commit d570f06, pas dans `main`) : `Script/Audio/HOLAudioTable.as` + `HOLAudioTable_Test.as`, vérification des noms par `Tools/Audio/check_audio_table.ps1`, import prêt dans `Tools/Editor/import_kenney_sounds.py` (pas encore lancé). Les 334 `.ogg` Kenney (CC0) sont dans `C:\Users\baudo\Documents\MyCrusader_Audio\Kenney\<pack>\`, hors dépôt. Prochaine étape : **A2b** (importer les 79 `.ogg` de la table en `SW_<nom>` sous `/Game/Audio/Kenney/<pack>/` : vérifier que `.ogg` s'importe dans Unreal 5.8 Hazelight, sinon convertir en WAV ; puis jouer la table depuis le script : `HOLAudio::CueFor` + `Pick`/`PickNext` + `MayPlay` avec l'horloge de la présentation, branchée sur les clics de l'interface, les ordres et les alertes ; l'écoute et les choix de sons sont provisoires, à valider par l'utilisateur), puis A3. Règles du projet à suivre : test rouge d'abord, un max d'Angelscript (D483), `EtapeDefini.md` et ce journal mis à jour à chaque avancée, rien commité avant validation, pas de push.

## Point de départ (relevé du 2026-11-03)

- `Content/Audio/` : 8 fichiers seulement : classes `SC_Effects`, `SC_Music`, `SC_Voice`, mixage `SCM_Volumes`, et 4 sons du héros (`SW_HeroBanner`, `SW_HeroCry`, `SW_HeroLevelUp`, `SW_HeroTier`). Les fanfares du héros sont provisoires (VSCO 2 CE, CC0, `make_hero_fanfare.py`).
- Alerte : « un son du moteur, en 2D, provisoire » (D449).
- Le code ne joue des sons que dans `HOLSelectionMarkers.cpp`, `HOLBannerSubsystem.cpp`, `HOLHeroLibrary.cpp` et `HOLGameScreen.cpp`. Aucune musique, aucun son d'ordre, de combat, de bâtiment, de récolte ni d'interface relevé. À confirmer en jeu avec l'utilisateur (relevé par lecture du code, pas à l'écoute).

## Décisions de source (à valider par l'utilisateur)

Jeu vendu : licences simples, registre tenu, rien sans licence lisible.

| Usage | Source | Licence |
|---|---|---|
| Interface, ordres, alertes | Kenney : UI Audio (50), Interface Sounds (100), RPG Audio (50), Impact Sounds (130) | CC0 (affichée sur chaque page, lue le 2026-11-03) |
| Combat, ambiances, bâtiments | Sonniss GameAudioGDC (téléchargement par l'utilisateur) | libre de redevance, sans attribution, pas d'entraînement d'IA ; **texte officiel `sonniss.com/gdc-bundle-license` à lire avant livraison** |
| Compléments | Freesound filtré CC0 | CC0 ; jamais CC-BY-NC |
| Musique (prototype) | pistes CC0 ou ElevenLabs Music (Creator 22 $/mois) | ElevenLabs : sous 500 000 $ de revenu cumulé, déclaration Steam « Pre-Generated » ; **conditions du rapport `reports/`, non revérifiées** |
| Musique (sortie) | compositeur ou banque avec licence écrite | à décider |
| Voix des unités | plus tard | aucune voix IA ou clonée avant lecture des conditions |

Écarté : Pixabay (licence propre, sources divergentes), tout son sans licence claire (YouTube, extraits d'autres jeux).

Registre : `Docs/assets_audio.csv` (fichier, source, licence, lien, date de téléchargement, preuve de licence). Chaque pack garde son `License.txt` à côté de ses fichiers.

## Plan

- **A0 (fait)** : relevé de l'existant, choix des sources, journal et registre ouverts.
- **A1 (fait le 2026-11-03)** : les 4 packs Kenney téléchargés par l'utilisateur (la page refuse les scripts), vérifiés avant extraction (aucune entrée suspecte : que des `.ogg`, deux raccourcis `.url` jamais ouverts, un `License.txt` CC0 lu), extraits chacun dans `C:\Users\baudo\Documents\MyCrusader_Audio\Kenney\<pack>\` (hors dépôt) : UI Audio 52, Interface Sounds 100, RPG Audio 52, Impact Sounds 130 `.ogg`. Inscrits au registre avec le chemin de leur licence. **Format `.ogg` : l'import dans Unreal 5.8 Hazelight n'est pas vérifié, une conversion en WAV sera peut-être nécessaire.**
- **A2 (table faite le 2026-11-05, ai-ia1 d570f06)** : table « événement vers son » (interface, ordres, alertes), écrite et testée en script (D483, D484), sans effet sur la simulation ni sur le réseau : les sons sont de la présentation, jouée à la réception de l'état comme les autres retours visuels. 24 événements (10 interface, 5 ordres, 2 sélection, 7 alertes), 79 fichiers Kenney ; reste **A2b** : import des `.ogg` et branchement du jeu (voir Reprise).
- **A3** : plafonds de sons du combat (Sound Concurrency : nombre maximal par type, priorité à la caméra et aux ennemis), pour 2 000 unités. Mesure avant et après (PerformanceInProgress.md).
- **A4** : combat, bâtiments, récolte (Sonniss).
- **A5** : ambiances par biome (`BiomeInProgress.md`), musique.

## Ce que couvrent les packs Kenney (relevé des noms de fichiers, pas écouté)

- **Interface** (UI Audio + Interface Sounds) : clics, survol (`rollover`), `select`, `confirmation`, `error`, `open`/`close`, `back`, `toggle`, `scroll`, `tick`, `maximize`/`minimize`. Couvre les boutons, menus, erreurs (« ressource manquante »), confirmation d'ordre.
- **Combat de mêlée et coups** (Impact Sounds, RPG Audio) : `impactMetal`, `impactWood`, `impactPlate`, `impactPunch`, `impactSoft`, `impactBell_heavy` (lourd/moyen/léger, 5 variantes chacun) ; `knifeSlice`, `drawKnife`, `chop`.
- **Récolte et bâtiments** : `impactMining` (pierre, or), `chop` (bois), `impactPlank`, `doorOpen`/`doorClose`, `creak`, `metalLatch`.
- **Déplacement** : `footstep` sur herbe, neige, béton, bois, tapis (5 variantes chacun) ; utile aux biomes herbe et neige.
- **Divers** : tissu, ceinture, livres, pièces (`handleCoins` pour le Marché).
- **Manque** (à chercher dans Sonniss ou Freesound CC0) : tir d'arc et de carquois, flèches en vol et qui frappent, cliquetis d'épées et boucliers, cris et voix, chevaux, bâtiment qui s'effondre ou brûle, foule, volcan, ambiances, musique.

## Questions ouvertes

- **AU-Q1** — Source de la musique du prototype : pistes CC0 (gratuit, qualité variable) ou ElevenLabs Music (22 $/mois, déclaration Steam) ?
- **AU-Q2** — Worktree pour A2 (code et contenu Unreal) : à choisir parmi ceux marqués libres dans leur `IN_USE.txt`, l'utilisateur donne le nom (règle : jamais de worktree créé sans lui demander).
- **AU-Q3** — Le bundle Sonniss (plusieurs dizaines de Go) : où le ranger sur le disque, et quelle année ?

## Réponses du 2026-11-04

- **AU-Q1 = A** : musique du prototype en pistes CC0 (D676). **AU-Q2** : A2 se fait dans le worktree **Arwen** (HeroesOfLiberty-Arwen, libre, main 532e638 ; à marquer PRIS dans son IN_USE.txt au départ). **AU-Q3 = A** : Sonniss récupéré hors dépôt, dans `MyCrusader_Audio\Sonniss\<année>` (D676).

## Journal

- 2026-11-03 : chantier ouvert. Relevé de `Content/Audio/` et du code. Kenney : licences CC0 lues sur les 4 pages ; téléchargement automatique refusé (HTTP 403 à `curl`, bouton JavaScript), donc à la main. Aucun fichier de jeu, aucun code touché, rien commité.
- 2026-11-05 (Arwen, `ai-ia1`) : **A2, la table.** AU-Q2 était déjà répondue (Arwen), aucune question nouvelle. `HOLAudioTable_Test.as` écrit d'abord : 9 tests, ROUGE vu (« Identifier 'FHOLAudioCue' is not a data type », `scripts compiled` KO, `tests ran (0)`), puis `HOLAudioTable.as` : `FHOLAudioCue` (événement, nom « Groupe.Nom », pack, fichiers, volume %, hauteur min/max %, délai en ms, priorité), `HOLAudioEvent::*` (0-23), `HOLAudioPriority::{Ui, Order, Alert}`, `HOLAudio::{Count, IsValid, CueFor, GroupOf, Pick, PickNext, SoundPath, MayPlay}`. Entiers seulement (pas de hasard ni d'horloge dedans : graine et millisecondes données par l'appelant). Piège : un script ne peut pas avoir de variable globale non constante (« Mutable global variables are not supported ») : la table est reconstruite à chaque appel (24 petites structures). Chemin d'asset : `/Game/Audio/Kenney/<pack>/SW_<fichier>.SW_<fichier>`. Vert : 9/9 des tests `Audio`. `Tools/Audio/check_audio_table.ps1` lit les lignes `Add(Cues, ...)` et vérifie sur disque que chaque pack et chacun des 79 fichiers existe et que le pack est au registre (PASS, rouge vu sur un faux nom). `Tools/Editor/import_kenney_sounds.py` (même lecture de la table, classe de son `SC_Effects`) écrit, vu seulement en simulation hors éditeur (79 chemins résolus), **jamais lancé dans l'éditeur**. Les fichiers sont choisis d'après leurs noms, **pas écoutés**.
