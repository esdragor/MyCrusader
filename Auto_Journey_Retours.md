# Auto_Journey : ce qui attend l'utilisateur

Dernière mise à jour : 2026-10-30 19:00 (réponses de l'utilisateur enregistrées par Auto_Journey).

**À faire par vous :** 0 essai prêt · questions : épaisseur des lignes d'ordre (brouillard, en attente de la correction du bug), M-Q10c arbres (à juger en jeu) · fusions : toutes faites (section 3) · 1 lot de docs à commiter.

**Manière de faire (confirmée le 2026-10-30)**
- Vous répondez **en ligne**, sous chaque item, par `Reponse: ...`. Les sous-agents y ajoutent leurs lignes (essai prêt, question posée), jamais ailleurs.
- Une réponse **n'enclenche pas de travail** : on l'enregistre (journaux, GDD, `EtapeDefini.md`), on repose des questions si elles émergent, et la correction devient une ligne `[ ]` en tête de `Auto_Journey_File.md`, faite au prochain « commence ton Auto_Journey ».
- **`Reponse : A trancher` = vous voulez en discuter plus en profondeur** : rien n'est noté comme décidé, la question reste ouverte.
- Une question est une **vraie question pour vous** : contexte en 2 lignes simples, options A/B/C avec leurs conséquences en jeu, recommandation, fichier. Jamais une remarque pour soi-même.
- **Ce fichier est léger et sans historique.** Un item dont vous avez répondu et dont la réponse est **reportée dans son chantier** (*InProgress.md, GDD, file de travail) est **supprimé d'ici** ; de même un essai fait et reporté. Ce qui reste = ce qui attend encore quelque chose de vous.
- Un essai est listé **seulement s'il est testable** : scénario créé et démarré hors écran sans erreur, commande `play.ps1` en chemin absolu (PowerShell), gestes précis. Sinon : « scénario à créer » dans la file.

## 1. Essais à faire

Format de chaque essai : titre · commande · quoi regarder · état · ligne `Reponse:`.

Aucun essai prêt pour l'instant : `tiers`, `tab`, `surplus` et `fog` ont été faits et reportés dans leurs chantiers (2026-10-30). Prochains essais : `fog_orders` à refaire après la correction du bug du brouillard ; `surplus` après le glissement du rang (D577).

## 2. Questions

### Brouillard
- **Question à vous : les lignes d'ordre sont-elles assez épaisses sous le brouillard ?** Les lignes d'ordre font 6,25 cm de large (réduites deux fois à vos essais) : à la caméra du jeu elles font 1 à 2 pixels. Sous le brouillard elles sortent maintenant du noir, mais restent fines. Options : A) garder 6,25 cm ; B) 12,5 cm seulement quand le brouillard est actif ; C) 12,5 cm partout. Recommandation : A tant que vous ne dites pas que c'est encore trop discret (réglable à chaud : `hol.Orders.LineWidth`). Fichiers : `FogInProgress.md`, `Source/HeroesOfLiberty/Player/HOLSelectionMarkers.cpp`.
(Essai impossible tant que le bug du brouillard n'est pas corrigé, voir la file de Faramir : la question sera reposée avec un essai `fog_orders` refait.)
Reponse:

### Cartes
- **M-Q10c : jusqu'où resserrer les arbres ?** (ne se tranche que dans le jeu, pas sur une capture)
  Contexte : les troncs d'un bosquet sont à 1,6 m les uns des autres (avant 2,2 m) ; une unité ne passe plus entre deux arbres d'un paquet. Vous ne pouvez pas lire une capture : il faut le voir dans le jeu. Un scénario ou essai in-game (arbres à 1,4 / 1,6 / 1,9 m côte à côte) est en file pour Haldir ; **la question reste en attente jusqu'à cet essai**.
  - A. Garder 1,6 m. En jeu : bosquets denses, bien lisibles.
  - B. Resserrer encore (1,4 m) : bosquets presque pleins, plus de place pour d'autres bosquets, Immense pourrait atteindre 450 arbres.
  - C. Revenir à 1,9 m : plus aéré, mais Immense reste à 360 et Normal / Grande ont moins de bosquets.
  Recommandation : A, à confirmer en jeu. Fichier : `Tools/Editor/maps/common.py` (`GROVE_SPACING`).
Reponse:

## 3. Commits et fusions à valider

- **À commiter dans la copie principale** (`C:\Users\baudo\Documents\MyCrusader`) : `GDD.md` (§ 7.2 aligné, D564-D575, prochain **D576** ; contient aussi D558-D563 d'autres sessions), `EtapeDefini.md`, les `*InProgress.md`, les `Auto_Journey*.md`, `FogInProgress.md`, `MapsInProgress.md`, `VolcanoInProgress.md`, `Tools\AutoJourney\status.ps1` (non suivis). Ils mêlent plusieurs sessions.
- **Fusions dans `main`** : les six branches (`ai-ia1`, `galadriel-tab`, `tiers`, `hero-j8-close`, `fog`, `maps`) sont fusionnées en avance rapide (2026-10-30 soir, `main` = f0c7b06), `quick_check` par défaut (530/530) et `-Parts victory` verts sur `main`. Rien d'ouvert ici. À noter : `maps` portait déjà M-Q10 / M-Q10b commité (0ae2591, plus de décor, herbe uniforme) quand elle a été fusionnée ; `net-playtest` (Glorfindel) n'est pas à fusionner.
