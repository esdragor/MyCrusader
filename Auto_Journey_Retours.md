# Auto_Journey : ce qui attend l'utilisateur

**À JOUR AU 2026-11-12 12:30 (CLOTURE-1112a, après la fusion groupée de 12:18).** Compteur : 20 fichiers dans `Retours\` = 2 BUG-*.md, 7 E-*.md (essais, tous `[NE PAS JOUER]`), 1 F-*.md (mesures), 8 Q-*.md (sans réponse, liste ci-dessous), `ESSAIS-A-JOUER.md`, `LISEZ-MOI.md` ; Q-play-hidden-wait répondue et archivée (3b94faf). La vraie liste vit dans ce dossier (une fiche par essai `E-*.md`, par question `Q-*.md`, par bug `BUG-*.md`) et `Retours\ESSAIS-A-JOUER.md` (prêts à donner : 0 ; binaire de la copie jouée périmé, sons candidats non commités).

**Questions du jour sans réponse (8, dans l'ordre de ce qu'elles débloquent ; chacune a son défaut codé, rien n'attend)** :
- `Q-secondary-centers.md` : coût et population du centre secondaire (D384), 2 questions ; défaut A : 300 bois + 300 pierre, population 10 ; débloque SC2-SC3 (chapitre en pause).
- `Q-specialization.md` : choix de spécialisation de palier, 2 questions ; défaut A : un déblocage à la fois, compte à rebours en pause quand le Forum est à terre ; débloque SP2.
- `Q-signals.md` : signaux sur la carte (D386), 2 questions ; défaut : un signal dure 5 s, un coéquipier éliminé peut encore en poser (B, comme sa discussion D357) ; débloque SG2.
- `Q-nomad.md` : option Nomade, 2 questions ; défaut A : groupes de 2 Paysans au plus, 40 cases au moins d'un groupe ennemi ; débloque N2.
- `Q-income-tooltip-rate.md` : infobulle de revenu « +N/min », 1 question ; défaut A : chaque Paysan compté à son taux à la source (branche net-playtest, non fusionnée).
- `Q-neutrals-camp-hover.md` : survol d'un point de camp sur la minicarte (N7), 1 question ; défaut A : aucun survol (branche d751-root, non fusionnée).
- `Q-resources-tree-size.md` : taille du peuplier (arbre C), 1 question ; défaut A : 7,5 m comme dans resources_compare (branche visual, non fusionnée).
- `Q-spear-grip.md` : prise de la lance du Lancier (bras gauche dans le torse), 1 question ; défaut A : prise gardée, rien à jouer. Ce fichier ne contient que la manière de faire ; bilans de séance et fusions : `Auto_Journey_Jour.md`. Anciens contenus : `Docs\Archive\Auto_Journey_Retours_jusqu_au_2026-11-09.md` (jusqu'au 2026-11-09) et `Docs\Archive\Auto_Journey_Retours_fusions_2026-11-10_archive_2026-11-11.md` (fusions du 2026-11-10).

**Manière de faire (confirmée le 2026-10-30)**
- Vous répondez **en ligne**, sous chaque item, par `Reponse: ...`. Les sous-agents y ajoutent leurs lignes (essai prêt, question posée), jamais ailleurs.
- **Vous répondez en balisant** (décidé le 2026-11-01) : `Reponse: OK`, `Reponse: BUG: ...`, `Reponse: REGLAGE: ...` ou `Reponse: A trancher`. Un `BUG:` ou un `REGLAGE:` devient une ligne `[ ]` de la file.
- Une réponse **n'enclenche pas de travail** : on l'enregistre (journaux, GDD, `EtapeDefini.md`), on repose des questions si elles émergent, et la correction devient une ligne `[ ]` en tête de `Auto_Journey_File.md`, faite au prochain « commence ton Auto_Journey ». **`Reponse : A trancher` = vous voulez en discuter plus en profondeur** : rien n'est noté comme décidé, la question reste ouverte.
- Une question est une **vraie question pour vous** : contexte en 2 lignes simples, options A/B/C avec leurs conséquences en jeu, recommandation, fichier. Jamais une remarque pour soi-même. **Questions rapides à comparer** : quand c'est possible, la question porte une commande `play.ps1 <nom>_compare` où toutes les options sont à l'écran (Entrée = option suivante).
- **Ce fichier est léger et sans historique.** Un item dont vous avez répondu et dont la réponse est **reportée dans son chantier** (*InProgress.md, GDD, file de travail) est **supprimé d'ici** ; de même un essai fait et reporté. Ce qui reste = ce qui attend encore quelque chose de vous.
- Un essai est listé **seulement s'il est testable** : `Tools\Scenarios\preflight.ps1` PASS, statut `VU À L'ÉCRAN` (sinon `LOGIQUE SEULEMENT`, jamais compté prêt), 3 min maximum, commande `play.ps1` en chemin absolu (PowerShell), gestes précis. Sinon : « scénario à créer » dans la file. Règles complètes : `Auto_Journey.md`, « Qualité des essais ».

**Fusions récentes** : fusion groupée du 2026-11-12 à 12:18 : main = 4851635, branches maps + fog (fff9792) + hero-j8-close (1d12401), sans C++, tests 1200/1200, RIEN POUSSÉ. Branches NON fusionnées faute de compilation (machine saturée par un autre projet) : d751-root (Arwen), tiers (Elrond), galadriel-tab, net-playtest (Glorfindel), visual (Sauron). Celles du 2026-11-11 sont dans `Docs\Archive\Auto_Journey_Retours_fusions_2026-11-11_archive_2026-11-12.md`.
