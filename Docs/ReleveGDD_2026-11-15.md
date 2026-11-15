# Relevé des chapitres du GDD, état du 2026-11-15 12:2x (RELEVE-1115a, lecture seule)

Remplace la section « Chapitres du GDD non commencés (relevé NEUF du 2026-11-12 11:10) » de `Auto_Journey_Jour.md`.
Base : `main` = 339cebc ; journaux `*InProgress.md` de la copie principale ; `git merge-base --is-ancestor` pour « dans main » ;
Grep dans `Script\` et `Source\` pour « branché ou non ». Aucun design tranché ici.

## 1. Les dix lignes du relevé du 2026-11-12, état réel

| # | § et titre | État réel (2026-11-15) | Prochaine étape C++ ou question | Worktree conseillé |
|---|---|---|---|---|
| A1 | § 15 Signaux D386 | Règle pure + SG2 serveur DANS main (e65b3b7 : `UHOLSignalSubsystem`, RPC `ServerPostSignal` / `ClientReceiveSignal`, test `Signals.DefeatedSpectator`) ; seul geste = console `hol.Signal.Post` ; Q-signals tranchées | SG3 C++ : Alt + clic (roue), icône de mini-carte, ligne du fil ; scénario à geste réel | Haldir (maps) |
| A2 | § 13 Spécialisation de palier D07/D684 | Règle pure + SP2 serveur DANS main (1bce302 : `UHOLSpecializationSubsystem`, vue `FHOLSpecializationView`, branché à `HasCompleteForum`) ; seul geste = console `hol.Spec.Start` ; Q tranchées | SP3 C++ : bouton du Forum + barre ; reste aussi `hol.Cheat.Specialization`, 3 options des Templiers, net à 2 clients | Haldir (maps) ou Elrond (tiers) |
| A3 | § 14.1 / 14.3 Option Nomade | FAIT dans main pour le joueur (N0-N2, D797, D799 : départ dispersé, forum page II case C, fin du 1er forum répliquée) ; N3 IA règle pure DANS main (2c1eee5, `HOLAINomadRules.as`), **non branchée** (appelée par son seul test) | Branchement de N3 dans l'IA ; attend `Retours\Q-nomad-ia.md` (défaut A codé) ; essai `E-nomad_start` à jouer | Haldir (maps) |
| A4 | § 15 Canaux de discussion D385/D792 | Règle pure FAITE dans main 59e8f60 (`Script\Chat\HOLChatRules.as`) ; aucun C++ (Grep `HOLChat` vide dans `Source\`) ; Q-chat-channels tranchée (D792) | Transport C++ : RPC serveur + saisie Entrée + affichage | Glorfindel (net-playtest) |
| B1 | § 6 Garnison (F7 de Fortifications) D693/D793 | Lots 1, 2, 3a-3e, 4b DANS main (43156bd, ef624dd : entrer, sortir tous, abri, éjection, réseau, clic droit = garnir) | 4a bouton « Sortir tous » + liste : attend `Q-garrison-ui.md` ; `Q-garrison-soin.md` (défaut A codé) ; 4c IA (garnir une tour attaquée) ; 4d scénario impossible sans ligne `TowerWood` (F8) | Elrond (tiers) |
| B2 | § 9.4 XP d'exploration D05/D06 | Règle pure DANS main (cf77faf, `Script\Heroes\HOLExplorationXPRules.as`) ; non branchée (`OnCellExplored` appelé par son seul test) | Branchement C++ au brouillard (serveur) ; `Q-exploration-xp.md` (défaut C codé) | Faramir (fog) |
| B3 | § 8.2 Zones de furtivité D383 | Règle pure DANS main (d8bbe87 + registre 162b28d, `Script\Fog\HOLStealthRules.as`) ; non branchée (`IsHiddenFrom` appelé par ses seuls tests) ; aucune carte ne pose de zone | Branchement C++ à la vue par camp + filtre réseau ; `Q-stealth.md` (N = 800, cartes, aspect) | Faramir (fog), zones par Haldir |
| B4 | § 8.1 Reste de D33 (forêt, gué) | Couvert en forêt SUPPRIMÉ (D796 ; règle retirée, HALDIR-1114c) ; F4 forêt lente (cavalerie, siège) et F5 gué (défense réduite) : aucun code (Grep `Ford`, `ForestSlow` vide) | À ouvrir : règle pure F4/F5 (modèle du marais) ; « être en forêt » n'a plus de définition codée depuis D796 : question probable (définition, valeurs) | Haldir (maps) |
| B5 | § 7.3 Ordres Patrouille et Suivre | Règle pure Patrouille DANS main (3a636ce, `Script\Orders\HOLPatrolRules.as`) ; aucun `EHOLOrderKind::Patrol` ; Suivre et focus : rien | Attend `Q-patrol-follow.md` (case de grille, Suivre, focus ; aucun défaut codé) ; puis C++ de l'ordre | Glorfindel ou Celeborn (ordres) |
| B6 | § 7.3 / § 15 Raccourcis reconfigurables | Règle pure DANS main (61ec678, `Script\Input\HOLRebindRules.as`, table D30) ; non branchée ; onglet grisé (D245 « jusqu'à M4 ») | Attend `Q-rebind.md` (défaut A : grisé jusqu'à M4) ; puis C++ Enhanced Input + sauvegarde | Galadriel (galadriel-tab) |

## 2. Règle pure faite, branchement C++ à faire : premier test rouge et estimation

| Chantier | Premier test rouge C++ (assertit l'effet) | Estimation | Bloqué par |
|---|---|---|---|
| Signaux SG3 | Alt maintenu + clic sur le monde : l'allié reçoit le signal (`GetNumReceivedSignals` 1), l'ennemi 0 ; puis icône de mini-carte présente chez l'allié | 2 lots (saisie + roue ; icône + ligne du fil + scénario) | rien |
| Spécialisation SP3 | Clic sur la case de spécialisation du Forum au palier ouvert : la vue du joueur passe `PendingTier` à ce palier, l'option est appliquée au bout du délai | 1-2 lots (+ net à 2 clients) | rien |
| Discussion (A4) | `Chat.ServerRoute` (modèle `Signals.ServerBoard`) : message Alliés d'un siège reçu par l'allié, pas par l'ennemi ni un spectateur en chacun pour soi ; privé au seul destinataire | 2 lots (serveur ; saisie Entrée + affichage) | rien (harnais net = 2 clients seulement) |
| XP d'exploration (B2) | Une unité du camp 0 entre dans une tranche inexplorée : le héros du camp 0 gagne l'XP `Exploration` (valeur calculée par la règle) une seule fois, aussi héros mort ; rien au départ | 1-2 lots (crochet du brouillard serveur, liste des lieux) | défaut C codé, Q-exploration-xp ouverte |
| Furtivité (B3) | Unité dans une zone, observateur ennemi à N + 1 dans une case en vue : l'unité n'est ni visible ni répliquée à ce camp ; à N - 1 elle l'est | 2 lots (vue par camp ; filtre `UHOLFogNetSubsystem`) + zone posée par une carte ou un crochet de test | Q-stealth (Q2 cartes : aucune zone posée) |
| Patrouille (B5) | Ordre Patrouille A -> B sur le vrai `HOLUnitOrderProcessor` : arrivée en B puis repart vers A ; un ordre manuel l'arrête | 1 lot (ordre) + grille après réponse | Q-patrol-follow (case) |
| Raccourcis (B6) | Après réaffectation de `Grid.Cell0` à une touche libre, l'appui de cette touche déclenche la case 0 de `HOLCommandGrid`, l'ancienne touche plus rien | 2 lots (Enhanced Input ; sauvegarde + onglet) | Q-rebind / D245 (M4) |
| IA Nomade (N3) | Partie Nomade contre l'IA (`aiserver` ou test C++) : l'IA pose son premier forum au plus gros groupe dès que le stock couvre le prix, dans un temps borné | 1 lot | Q-nomad-ia (défaut A codé) |
| Garnison 4a / 4c | 4a : « Sortir tous » du panneau vide la tour ; 4c : l'IA garnit une tour attaquée | 1 lot chacun | 4a : Q-garrison-ui ; 4d : ligne `TowerWood` (F8) |

« Lot » = 30-45 min d'un agent C++ (un build compris). Estimations non mesurées.

## 3. Questions en attente de l'utilisateur (`Retours\Q-*.md`, toutes titrées [À RÉPONDRE])

| Fichier | Ce qu'elle débloque |
|---|---|
| Q-exploration-xp.md | Porteur et valeur de l'XP d'exploration (défaut C codé) avant le branchement B2 |
| Q-stealth.md | Distance « tout proche », cartes à zones, aspect (B3) ; sans réponse aucune carte n'a de zone |
| Q-patrol-follow.md | Case de grille de Patrouille / Suivre, comportement de Suivre et du focus (B5 C++ ; aucun défaut codé) |
| Q-rebind.md | Ouvrir l'onglet Commandes maintenant ou à M4, périmètre des raccourcis (B6 C++) |
| Q-nomad-ia.md | Où et quand l'IA Nomade fonde son forum (branchement N3 ; défaut A codé) |
| Q-garrison-ui.md | Bouton « Sortir tous », liste des occupants, clic droit dépôt + garnison (F7 4a) |
| Q-garrison-soin.md | Soin des unités en garnison (défaut A codé : non) |
| Q-sites-grisage.md | D'où l'écran Nouvelle partie sait qu'une carte a des sites (Sites sacrés S7 ; défaut A codé) |
| Q-rocks-proportion.md | Part de rochers à facettes, sol des autres scénarios (décor D802, régénération des cartes) |
| Q-arquebusier-modele.md | Modèle et poses de l'Arquebusier (l'utilisateur a demandé un GIF animé, pas de choix encore) |
| Q-audio-licences-tour5.md | Garder, remplacer ou retirer les 8 sons de cheval Red Library (Écurie) |
| Q-income-tooltip-taille.md | Taille du texte de l'infobulle de revenu |
| Q-march-member-detour.md | Garder la mémoire du détour de 8 pas dans un bloc en marche (défaut A codé) |
| Q-close-gate-l15-1.md | Deux cas FAIL de `session_gate -Phase Close` (outil ; défaut A, recommandation B) |

## 4. Chapitres ou § sans code ni journal que le relevé du 2026-11-12 n'avait pas vus

| # | § et titre | État réel | Prochaine étape | Worktree conseillé |
|---|---|---|---|---|
| C1 | § 15 Alertes mineures D202 (unité ou bâtiment attaqué ailleurs, recherche terminée, population pleine : fil, ping mini-carte, son court) | Alertes du héros (prêt à revenir, niveaux) et bannière majeure codées (`HOLForumMessages.h`) ; cues sonores `UnderAttack`, `TrainingDone` en script (`HOLAudioTable.as`) ; aucun ping de mini-carte (Grep `MinimapPing`, `AddPing` vide) ; aucun journal dédié (AudioInProgress : « reste le branchement des ordres/alertes ») | À ouvrir : relevé de ce qui manque dans le fil, puis règle pure « quelle alerte, à quel rythme » ; probablement 1 question (rythme, regroupement) | Galadriel (HUD) |
| C2 | § 5.3 Population pleine pour les unités sans file D391 (Squelettes, Renforts de l'Aube) | Aucun code ni journal (Grep `D391` vide dans `Script\`, `Source\`, journaux) | Ne pas ouvrir seul : à rattacher à Légions (Squelettes) et Aube (Renforts), qui ne sont pas codés | Elrond / Galadriel avec la faction |

Vérifiés couverts (pas de ligne à ouvrir) : § 9.2 aura de commandement (`HOLHeroAbilityRules`, `HOLCombatProcessor`), § 14.4 IA évolutive D329 (`HOLAITools.cpp` `hol.AI.SetEvolveTimes`, `EHOLAIDifficulty`), § 7.3 double-clic de sélection (`HOLUnitPicking`), § 14.4 déconnexion D71 (`DisconnectInProgress.md`), § 14.2 Snowball (principe sans règle). Le groupe C du relevé du 2026-11-12 reste valable.

**Après le prototype, ne se prennent pas** : § 13.4 à § 13.7, § 12.3, § 14.3 Battlefield / Domination / Reliques / Scénario, § 9.9, Diplomatie V2, moral de groupe à états.

## Ordre conseillé (sans question bloquante d'abord)

1. Signaux SG3 (Haldir) ; 2. Spécialisation SP3 (Haldir ou Elrond) ; 3. Discussion transport (Glorfindel) ; 4. Garnison 4c IA (Elrond) ;
5. XP d'exploration branchement (Faramir, défaut C) ; 6. IA Nomade branchement (Haldir, défaut A) ; 7. B4 F4/F5 règle pure (Haldir) ; 8. C1 alertes mineures (Galadriel) ;
puis, après réponse : Furtivité, Patrouille, Raccourcis, Garnison 4a.

## Non vérifié dans ce relevé

- C1 : la liste exacte des alertes mineures présentes dans le fil (seuls des Grep, pas de lecture de `HOLGameScreen.cpp`).
- § 9.2 « certaines formations disponibles dans la zone » ; § 8.3 contenu de carte (ruines, objectifs, ressources contestées) ; D203 fiche d'un héros adverse (supposée couverte par J8, test `net hero sheets`).
- Les worktrees n'ont pas été fouillés : un travail non fusionné (SG3, SP3, transport du chat) peut exister hors de `main` (`git log --all --grep SG3 / SP3` vide).
- Estimations en lots non mesurées ; aucune compilation, aucun test lancé.
