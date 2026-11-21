# Auto_Journey : ce qui attend l'utilisateur

**ÉTAT AU 2026-11-21 19:0x : 1 [À ESSAYER] dans Retours\ (E-deploy.md, déjà joué : sa réponse BUG est reportée en D877, la fiche attend l'archivage par le nettoyage de Retours\), 0 [À RÉPONDRE], 0 question ouverte, 1 BUG en file (deploy = D877, Auto_Journey_File.md).** E-audio_panel est [NE PAS JOUER], E-playtest_hero_army est LOGIQUE SEULEMENT, 3 BUG-* gardés. TowerWood A et centre secondaire A (D878, D879) répondus et enregistrés. main = 6fe9d84 (local, rien poussé).

Ce fichier ne contient que la manière de faire ; bilans de séance et fusions : `Auto_Journey_Jour.md`. Anciens contenus : `Docs\Archive\Auto_Journey_Retours_jusqu_au_2026-11-09.md` (jusqu'au 2026-11-09), `Docs\Archive\Auto_Journey_Retours_fusions_2026-11-10_archive_2026-11-11.md` (fusions du 2026-11-10) et `Docs\Archive\Auto_Journey_Retours_2026-11-13_matin_archive_2026-11-13.md` (en-tête de 10:2x, fusion du 2026-11-12, décisions en attente à 14:05) et `Docs\Archive\Auto_Journey_Retours_2026-11-15_archive_2026-11-16.md` (en-tête « Précédent (2026-11-15) ») et `Docs\Archive\Auto_Journey_Retours_2026-11-16_apres-midi_archive_2026-11-16.md` (en-tête de la séance interactive du 2026-11-16 après-midi) et `Docs\Archive\Auto_Journey_Retours_2026-11-21_matin_archive_2026-11-21.md` (en-tête de 10:5x, item BUG deploy reporté en D877, état de main de 10:5x).

**Manière de faire (confirmée le 2026-10-30)**
- Vous répondez **en ligne**, sous chaque item, par `Reponse: ...`. Les sous-agents y ajoutent leurs lignes (essai prêt, question posée), jamais ailleurs.
- **Vous répondez en balisant** (décidé le 2026-11-01) : `Reponse: OK`, `Reponse: BUG: ...`, `Reponse: REGLAGE: ...` ou `Reponse: A trancher`. Un `BUG:` ou un `REGLAGE:` devient une ligne `[ ]` de la file.
- Une réponse **n'enclenche pas de travail** : on l'enregistre (journaux, GDD, `EtapeDefini.md`), on repose des questions si elles émergent, et la correction devient une ligne `[ ]` en tête de `Auto_Journey_File.md`, faite au prochain « commence ton Auto_Journey ». **`Reponse : A trancher` = vous voulez en discuter plus en profondeur** : rien n'est noté comme décidé, la question reste ouverte.
- Une question est une **vraie question pour vous** : contexte en 2 lignes simples, options A/B/C avec leurs conséquences en jeu, recommandation, fichier. Jamais une remarque pour soi-même. **Questions rapides à comparer** : quand c'est possible, la question porte une commande `play.ps1 <nom>_compare` où toutes les options sont à l'écran (Entrée = option suivante).
- **Ce fichier est léger et sans historique.** Un item dont vous avez répondu et dont la réponse est **reportée dans son chantier** (*InProgress.md, GDD, file de travail) est **supprimé d'ici** ; de même un essai fait et reporté. Ce qui reste = ce qui attend encore quelque chose de vous.
- Un essai est listé **seulement s'il est testable** : `Tools\Scenarios\preflight.ps1` PASS, statut `VU À L'ÉCRAN` (sinon `LOGIQUE SEULEMENT`, jamais compté prêt), 3 min maximum, commande `play.ps1` en chemin absolu (PowerShell), gestes précis. Sinon : « scénario à créer » dans la file. Règles complètes : `Auto_Journey.md`, « Qualité des essais ».

**Historique** (état du 2026-11-13 21:4x, fusions récentes, essais joués et suites du 2026-11-14) : archivé tel quel le 2026-11-15 dans `Docs\Archive\Auto_Journey_Retours_historique_2026-11-14_archive_2026-11-15.md`.

## 1. Essais à jouer
Aucun essai en attente de réponse : le BUG deploy (réponse reportée en D877, ligne `[ ]` en tête de la section Elrond d'Auto_Journey_File.md) est archivé dans `Docs\Archive\Auto_Journey_Retours_2026-11-21_matin_archive_2026-11-21.md`. Prochain essai : le scénario deploy corrigé, quand D877 sera fait.

## 3. État de main
- main = 6fe9d84 (local, rien poussé ; clôture du 2026-11-21 matin : integ-41 dans main, intégré a987f58, docs ensuite).
