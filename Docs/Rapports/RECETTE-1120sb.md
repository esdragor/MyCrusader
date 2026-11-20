# RECETTE-1120sb (agent indépendant, 2026-11-21 00:12-00:17, worktree HeroesOfLiberty-Arwen, integ-37 = main f46dc2d)

Aucun code modifié, aucun build, aucun commit. Dépassement : ~31 appels (limite 30), pour écrire les fiches E et le rapport.
Les horodatages du journal UE sont en UTC-1 par rapport à Get-Date (23.12 = 00:12).

## 1. chat_input (GLORFINDEL 7b9f5b4, ancêtre de HEAD : oui)
- Preflight : PASS (28 s, 4 PNG ouverts). Code chargé : le journal affiche le nouveau format `HUD chat: open Allies (ally true, private target 1)`.
- Verdict : FAIL jeu. Après Entrée + Tab Tab :
  - `HUD chat: open Allies (ally true, private target 1)` (23.12.40.096) : la cible privée est maintenant bonne (1), mais le canal reste Alliés.
  - La ligne `ChatInput: Tab (open ...) ally ... chosen N -> channel X` est ABSENTE du journal (0 occurrence). Les deux Tab arrivent au champ (`Key pressed: Tab (focus: .../ChatInputBox/SEditableTextBox...)`, `down handled`) mais ni `HandleTab` par KeyPressed ni `OnUserMovedFocus` -> `HandleTab` (HOLChatInputScreen.as:210-219 et 332-335) ne s'exécute : Tab Slate perdu (consommé par la navigation de focus de la boîte ou envoyé au champ sans commit). Fichiers : Script/UI/HOLChatInputScreen.as (HandleTab, OnBoxCommitted), appelant C++ du KeyPressed (HOLScriptScreen.cpp ~1034 : ne transmet que Enter ?).
  - chat_input_target.png : le champ montre « Allies : » + boîte blanche vide ; ni « (Privé) » ni nom du joueur 2.
- Marque de canal : lisible (fond sombre, texte clair) dans chat_input_target.png et chat_input_typed.png (« Allies : salut les allies » dans la boîte). Mais écrite « Allies » (sans accent, anglais) et non « (Alliés) » comme dit le .look et comme le fil (« (Alliés) salut les allies ») : mark du champ non localisé, défaut mineur à signaler.
- 2e Entrée : rouvre le champ (journal 23.12.35.126 Enter puis 23.12.36.290 `HUD chat: open`) : PASS.
- Message privé envoyé par `/w Joueur 2 salut` (« (Privé) salut » au fil) : le chemin Privé par commande fonctionne, pas par Tab.
- Pas de fiche E.

## 2. virtuous_knight (SAURON a97f228, 3bdad5f)
- Preflight : PASS (21 s, 3 PNG ouverts). VU À L'ÉCRAN.
- Case E : active (journal `E=VirtuousKnight enabled`, PNG : même teinte que A et Z).
- Chevalier #13 sélectionné en garde (`Hold order ... 1 units`), même place sur before et hp ; anneau de 8 m visible.
- Journal : 0 ligne `hits #13` ; les coups visent l'allié #14 (`#15 Spearman hits #14 Spearman for 8`) ; PV du Chevalier 155/155 -> 148/155 (panneau) : baisse sans coup direct = redirection D864/D865 (cause alternative non trouvée au journal).
- Verdict : PASS (VU À L'ÉCRAN). GESTE: interne (commandes hol.*), SON: non entendu. Fiche E-virtuous_knight.md écrite.

## 3. deploy (ELROND 24c73b7)
- Preflight : PASS (43 s, 13 PNG ; plus de timeout du second cycle).
- Vus : deploy_deploying : case D « Montage… [#-------] » (1 dièse, engin pas monté) ; deploy_packing : « Démontage… [#-------] » ; deploy_packed_again : D redit « Monter » ; deploy_attack_hit : Maison 465/600.
- Journal : 23.14.15.430 `D: Montage… [#-------]`, 23.14.17.542 `D: Démonter`, 23.14.24.653 `D: Démontage… [#-------]`, 23.14.26.755 `D: Monter`, 2e cycle 23.14.30.382 `Démonter`, 23.14.32.487 `Monter`, 23.14.39.184 `Démonter`.
- Verdict : PASS (VU À L'ÉCRAN), confirme Elrond. E-deploy.md : branches et verdict mis à jour (GESTE: interne conservé).

## NON VÉRIFIÉ
Son (-nosound) ; vrai clavier pour Tab Tab ; cause exacte de la perte du Tab (C++ non lu) ; vrais gestes de souris (tous les essais utilisent hol.*) ; deploy_move_unmounting (libellé « Démontage… » non lu sur la capture, non rouvert).
