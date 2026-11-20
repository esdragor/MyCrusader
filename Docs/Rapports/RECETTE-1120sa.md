# RECETTE-1120sa (agent indépendant, 2026-11-20 23:49-23:57, worktree HeroesOfLiberty-Arwen, integ-36 = main 923d4af)

Aucun code modifié, aucun build, aucun commit. Un scénario temporaire (deploy_r1.*) a été créé dans le worktree puis supprimé.

## 1. deploy (D869b, ELROND 5cbbf40 + 77a229e)
- Preflight `deploy` : FAIL (9) : `timeout at step 77: waitlog D=Pack` + 6 captures du second volet (deploy_move_*, deploy_attack_*) jamais prises.
- Effet du correctif : PASS VU À L'ÉCRAN.
  - deploy_deploying.png : case D = « Montage… [#-------] » (1 dièse), engin pas encore monté. Journal 22:49.20.866 `HUD cells: ... D: Montage… [#-------]`.
  - deploy_packing.png : case D = « Démontage… [#-------] ». Journal 22:49.30.310.
  - deploy_packed_again : D redit Monter (journal 22:49.32.456 `D=Deploy`).
- 2e cycle : le FAIL est du SCÉNARIO (L18-f), pas du jeu. Journal 22:49.34.136 `phase 1 -> 2 (delay over)` puis 22:49.34.148 (12 ms plus tard) `HUD.Report` : `D=Mounting7`, `D: Montage… [#######-]` ; le scénario ne refait aucun `HUD.Report` avant `waitlog D=Pack` (ligne 76 de deploy.txt), donc timeout 60 s. Au 1er cycle un `wait 2` précède le rapport (ligne 32).
- Preuve que le jeu est bon : copie temporaire (deploy.txt lignes 1-72 + `waitlog Order: 1 units deploy`, `waitlog (delay over)`, `wait 2`, `HUD.Report`, `waitlog D=Pack`, capture) : check PASS, journal 22:53.44.487 `D=Pack`, `HUD cells: ... D: Démonter`, encore 22:53.45.523 ; PNG deploy_r1_mounted.png : case D « Démonter », engin monté (état Attaque).
- Correctif de scénario proposé (à faire par l'auteur) : insérer `wait 2` entre `waitlog (delay over)` (ligne 74) et `hol.HUD.Report` (ligne 75) de `Tools\Scenarios\deploy.txt`.
- Détail mineur non creusé : dans la copie, le démontage du cycle 1 est journalisé `(order 2, action 3, ..., walk 3)` alors que le run d'origine donnait `(order 3, action 3, ..., walk 0)` (même geste D) ; non investigué.
- Verdict : correctif PASS (vu) ; essai tel quel FAIL scénario ; pas de fiche E (preflight FAIL, règle 18).

## 2. chat_input (GLORFINDEL 728ee41)
- Preflight : PASS (34 s, 4 PNG ouverts).
- Correctif de focus : PASS. Journal : 2e Entrée `22:51.27.903 Key pressed: Enter (focus: GameEngine...)` puis `22:51.29.099 HUD chat: open Allies` ; 3e Entrée `22:51.31.597` (focus viewport) puis `22:51.33.148 chat: open Allies`. `grep 'focus: none'` = 0 occurrence. Les lignes `Key released: Enter (... ChatInputBox ...; down not handled` subsistent sur l'ouverture (le press est parti au viewport, le release au champ) : normal, le champ s'ouvre quand même.
- chat_input_shown.png : fil « Vous : (Alliés) salut les allies » ; chat_input_typed.png : texte « salut les allies » dans la boîte.
- DÉFAUTS visibles (hors correctif testé) :
  - chat_input_target.png : boîte blanche VIDE, aucune marque « (Privé) » ni nom du joueur 2 ; journal 22:51.33.148 `HUD chat: open Allies` alors que la note 2 attend Privé après Entrée + Tab Tab. Cycle attendu (HOLChatRules.as:182) : Alliés -> Tous -> Privé ; on obtient Alliés, ce qui correspond à `bHasPrivateTarget = false` (cycle Alliés -> Tous -> Alliés) bien que `HUD diplomacy: ... message to 1` soit journalisé. Cause probable : l'appelant C++ ne passe pas la cible privée au champ (HOLChatInput.as:49-55 reçoit bHasPrivateTarget) ; à confirmer. Le preflight dit OK car il ne vérifie pas la marque.
  - chat_input_typing / chat_input_target : la « marque de canal (Alliés) » annoncée par le .look n'est pas visible à gauche de la boîte (boîte blanche seule, 127 px) ; texte blanc sur blanc ou marque absente.
- Verdict : correctif focus PASS (vu) ; essai complet FAIL jeu (Privé / marque de canal, Script/UI/HOLChatInput.as + appelant C++ du champ) ; pas de fiche E.

## 3. virtuous_knight (SAURON 318837f)
- Preflight : PASS (25 s, 3 PNG ouverts).
- Correctif de sélection : PASS VU À L'ÉCRAN. Journal 22:52.14.481 `Select type VirtuousKnight: 1 units` / `Selection: VirtuousKnight #13`, panneau « Chevalier Vertueux », PV 155/155, anneau (cercle blanc ~8 m) visible autour de lui dans virtuous_knight_before.png et _hp.png. virtuous_knight_grid.png : case E « Chevalier Vertueux » présente mais GRISÉE (journal `E=VirtuousKnight greyed` avec 5200 de chaque ressource : à clarifier, non demandé).
- PROBLÈME DE SCÉNARIO : la note 3 / le .look disent « aucun ennemi ne l'a visé, aucun hits #<id du Chevalier> ». Le journal dit le contraire : `22:52.15.631 #15 Spearman hits #13 VirtuousKnight for 4, 151 HP left`, `22:52.15.732 #17 Spearman hits #13 VirtuousKnight for 4, 144.6 HP left`, et le Chevalier attaque (`#13 hits #15`, activité Attaquant). La baisse 155 -> 145 vient de coups DIRECTS, elle ne prouve pas la redirection D864/D865. Le preflight ne vérifie pas cette ligne.
- Verdict : sélection PASS (vu) ; preuve de redirection NON établie (FAIL scénario : ennemis qui ciblent le Chevalier, le geler avec `S` ou l'éloigner des ennemis) ; pas de fiche E.

## Fiches E écrites
Aucune (preflight deploy FAIL ; chat_input et virtuous_knight : notes du scénario démenties par le journal/PNG, les lister enverrait l'utilisateur sur un essai faux). REPONSES-*.md : aucun fichier présent dans Retours\ (sans objet).

## NON VÉRIFIÉ
Son (-nosound). Geste de Tab Tab sur un vrai clavier. Cause exacte du défaut Privé (code C++ non lu). Pourquoi `order 2 / walk 3` au démontage dans la copie deploy_r1. `quick_net` tournait dans le même worktree à 23:47-23:48 (netsig3) : pas de collision constatée.
