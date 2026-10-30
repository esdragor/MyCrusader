# Auto_Journey : ce qui attend l'utilisateur

Dernière mise à jour : 2026-10-30 21:45 (réponses de l'utilisateur enregistrées dans leurs chantiers).

**À faire par vous :** 1 essai prêt (`fog_ghosts`) · aucune question en attente (section 2) · **fusions : 3 branches attendent (section 3)** · 1 lot de docs à commiter dont `GDD.md`.

**Manière de faire (confirmée le 2026-10-30)**
- Vous répondez **en ligne**, sous chaque item, par `Reponse: ...`. Les sous-agents y ajoutent leurs lignes (essai prêt, question posée), jamais ailleurs.
- Une réponse **n'enclenche pas de travail** : on l'enregistre (journaux, GDD, `EtapeDefini.md`), on repose des questions si elles émergent, et la correction devient une ligne `[ ]` en tête de `Auto_Journey_File.md`, faite au prochain « commence ton Auto_Journey ».
- **`Reponse : A trancher` = vous voulez en discuter plus en profondeur** : rien n'est noté comme décidé, la question reste ouverte.
- Une question est une **vraie question pour vous** : contexte en 2 lignes simples, options A/B/C avec leurs conséquences en jeu, recommandation, fichier. Jamais une remarque pour soi-même.
- **Ce fichier est léger et sans historique.** Un item dont vous avez répondu et dont la réponse est **reportée dans son chantier** (*InProgress.md, GDD, file de travail) est **supprimé d'ici** ; de même un essai fait et reporté. Ce qui reste = ce qui attend encore quelque chose de vous.
- Un essai est listé **seulement s'il est testable** : scénario créé et démarré hors écran sans erreur, commande `play.ps1` en chemin absolu (PowerShell), gestes précis. Sinon : « scénario à créer » dans la file.

## 1. Essais à faire

Format de chaque essai : titre · commande · quoi regarder · état · ligne `Reponse:`.

### Essai `fog_ghosts` (brouillard : un bâtiment ennemi vu puis perdu de vue reste dessiné en gris, un jamais vu n'est pas dessiné)
- Commande (PowerShell) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 fog_ghosts`
- Le scénario envoie vos 4 Hommes d'armes à 20 m d'une maison ennemie située à 90 m à l'est (elle se dessine en rouge), puis les ramène : la maison reste dessinée, teintée de gris-bleu dans la zone explorée (grise), sans clic possible ; une seconde maison ennemie à 90 m à l'ouest, jamais vue, n'est pas dessinée. Vérifié hors écran : `hol.Buildings.Report` dit « drawn live » puis « drawn from memory » pour la maison vue et « drawn not at all » pour l'autre ; captures `fogghost_a` (rouge) et `fogghost_b` (grise).
- À regarder : (1) la maison grise est bien là quand vous revenez au centre, cliquer dessus ne fait rien ; (2) un bâtiment jamais vu n'apparaît pas ; (3) la teinte (B, 0,6, tranchée ; dites seulement si elle ne vous convient pas) : `hol.Fog.GhostTint 0` à `1` dans la console (²) puis renvoyer les soldats voir la maison et les ramener (la teinte ne change qu'à la reconstruction du bâtiment). Limites : en partie réseau, un client ne dessine pas encore de fantôme (l'acteur n'existe plus chez lui hors de la vue) ; les arbres coupés hors de vue ne sont pas encore traités.
- État : scénario démarré hors écran sans erreur, `scenario fog_ghosts ready` (2026-10-30 20:41). Non vérifié sur votre écran (rendu réel).
Reponse: 

Autres essais : `surplus` (la cible encerclée à 6 : le changement de cible de dd16203, inchangé) et `rank_slide` (glissement du rang, accepté) restent utilisables ; `fog_orders` et `tree_spacing` seront refaits après leur correction (lignes en tête des files de Faramir et de Haldir).

## 2. Questions

Aucune question en attente : toutes les réponses du 2026-10-30 sont enregistrées dans leur chantier (journaux `*InProgress.md`, GDD D581 à D585, lignes `[ ]` de `Auto_Journey_File.md`). Une vraie question sera posée ici si la correction des arbres met en jeu le passage des unités ou les budgets de D564.

## 3. Commits et fusions à valider

- **Fusions en attente (test 2, 2026-10-30 20:58)** : `hero-j8-close` (glissement du rang, 2080c26, tests verts) est **bloquée par le `GDD.md` non commité de la copie principale** (le fast-forward l'écraserait) : il faut commiter `GDD.md` (votre accord) ou le mettre à l'abri, puis `git merge --ff-only hero-j8-close`. `maps` (Plaine au budget standard, 91cf604) : chemins, `crossings.ps1` et `aiduel` sur la Plaine à rejouer avant. `net-playtest` (Technologies T1-T2, 0ad1d13) : recompiler et `-Parts tests` après le merge de `main`. Les autres branches sont dans `main` (cd1b64d).

- **À commiter dans la copie principale** (`C:\Users\baudo\Documents\MyCrusader`) : `GDD.md` (§ 7.2 aligné, D564-D575, prochain **D576** ; contient aussi D558-D563 d'autres sessions), `EtapeDefini.md`, les `*InProgress.md`, les `Auto_Journey*.md`, `FogInProgress.md`, `MapsInProgress.md`, `VolcanoInProgress.md`, `Tools\AutoJourney\status.ps1` (non suivis). Ils mêlent plusieurs sessions.
- **Fusions dans `main`** : les six branches (`ai-ia1`, `galadriel-tab`, `tiers`, `hero-j8-close`, `fog`, `maps`) sont fusionnées en avance rapide (2026-10-30 soir, `main` = f0c7b06), `quick_check` par défaut (530/530) et `-Parts victory` verts sur `main`. Rien d'ouvert ici. À noter : `maps` portait déjà M-Q10 / M-Q10b commité (0ae2591, plus de décor, herbe uniforme) quand elle a été fusionnée ; `net-playtest` (Glorfindel) n'est pas à fusionner.
