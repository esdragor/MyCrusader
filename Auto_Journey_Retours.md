# Auto_Journey : ce qui attend l'utilisateur

Dernière mise à jour : 2026-10-30 21:45 (réponses de l'utilisateur enregistrées dans leurs chantiers).

**À faire par vous :** aucun essai prêt (`fog_ghosts` validé) · aucune question en attente (section 2) · **fusions : aucune en attente (section 3)** · docs de la copie principale commités (cfd73b0).

**Manière de faire (confirmée le 2026-10-30)**
- Vous répondez **en ligne**, sous chaque item, par `Reponse: ...`. Les sous-agents y ajoutent leurs lignes (essai prêt, question posée), jamais ailleurs.
- Une réponse **n'enclenche pas de travail** : on l'enregistre (journaux, GDD, `EtapeDefini.md`), on repose des questions si elles émergent, et la correction devient une ligne `[ ]` en tête de `Auto_Journey_File.md`, faite au prochain « commence ton Auto_Journey ».
- **`Reponse : A trancher` = vous voulez en discuter plus en profondeur** : rien n'est noté comme décidé, la question reste ouverte.
- Une question est une **vraie question pour vous** : contexte en 2 lignes simples, options A/B/C avec leurs conséquences en jeu, recommandation, fichier. Jamais une remarque pour soi-même.
- **Ce fichier est léger et sans historique.** Un item dont vous avez répondu et dont la réponse est **reportée dans son chantier** (*InProgress.md, GDD, file de travail) est **supprimé d'ici** ; de même un essai fait et reporté. Ce qui reste = ce qui attend encore quelque chose de vous.
- Un essai est listé **seulement s'il est testable** : scénario créé et démarré hors écran sans erreur, commande `play.ps1` en chemin absolu (PowerShell), gestes précis. Sinon : « scénario à créer » dans la file.

## 1. Essais à faire

Format de chaque essai : titre · commande · quoi regarder · état · ligne `Reponse:`.

Aucun essai prêt : `fog_ghosts` validé (« works ») et reporté dans `FogInProgress.md`. Prochains essais : `fog_orders` après le correctif du grésillement, `tree_spacing` après les bosquets compacts.

Autres essais : `surplus` (la cible encerclée à 6 : le changement de cible de dd16203, inchangé) et `rank_slide` (glissement du rang, accepté) restent utilisables ; `fog_orders` et `tree_spacing` seront refaits après leur correction (lignes en tête des files de Faramir et de Haldir).

## 2. Questions

Aucune question en attente : toutes les réponses du 2026-10-30 sont enregistrées dans leur chantier (journaux `*InProgress.md`, GDD D581 à D585, lignes `[ ]` de `Auto_Journey_File.md`). Une vraie question sera posée ici si la correction des arbres met en jeu le passage des unités ou les budgets de D564.

## 3. Commits et fusions à valider

- **Fusions faites (agent de fusion, 2026-10-30 22:0x)** : `hero-j8-close` (e00f7c0), `net-playtest` (96f2710) et `maps` (0346286) fusionnées en avance rapide dans `main` (= 0346286), docs commités avant (7aa15b4, cfd73b0 avec GDD D581-D585), builds PASS, `-Parts tests` verts (570/576/576), `combat` (hero-j8-close), `sizes`, `hinterland_paths` et `crossings` (100/100) PASS pour maps. **ROUGE préexistant, non causé par ces branches : `quick_check -Parts aiduel` échoue (« the two runs' step hashes are the same » = non-déterminisme entre les deux parties, sans erreur de partie) aussi sur `hero-j8-close` seule (état de main avant fusion) ; à instruire (IA/déterminisme), `maps` fusionnée malgré tout parce que l'échec ne vient pas d'elle. Rien d'ouvert ici.

- **Docs de la copie principale** : commités (7aa15b4 docs de séance, cfd73b0 `GDD.md`) ; ce qui a bougé depuis (`FogInProgress.md`, ce fichier) appartient aux autres sessions.
- **Fusions dans `main`** : les six branches (`ai-ia1`, `galadriel-tab`, `tiers`, `hero-j8-close`, `fog`, `maps`) sont fusionnées en avance rapide (2026-10-30 soir, `main` = f0c7b06), `quick_check` par défaut (530/530) et `-Parts victory` verts sur `main`. Rien d'ouvert ici. À noter : `maps` portait déjà M-Q10 / M-Q10b commité (0ae2591, plus de décor, herbe uniforme) quand elle a été fusionnée ; `net-playtest` (Glorfindel) n'est pas à fusionner.
