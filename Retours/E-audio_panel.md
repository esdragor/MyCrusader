# [NE PAS JOUER] E-audio_panel : écoute TOUS les sons encore ouverts en UNE commande (Entrée = le son suivant), puis tu réponds une lettre par cue dans cette fiche

STATUT : NE PAS JOUER (LOGIQUE SEULEMENT) ; cause : même chaîne de sons que stone/metal, candidats jamais commités (seulement dans Arwen), pré-vol du 2026-11-08 sur un main périmé (diagnostic 3b94faf) ; redonner quand la copie choisie est recompilée et que preflight.ps1 audio_panel -Root <cette copie> -MaxSeconds 900 complet PASS
GESTE: interne      (les sons sont joués par la commande hol.Audio.Candidate du scénario, pas par un vrai clic ; seule la touche Entrée est la tienne)
SON: non entendu   (-nosound côté agent ; passe à « entendu » quand tu as écouté)
branches: heal-sleep   (le panneau, la commande `loud` et les candidats viennent du worktree Arwen ; candidats JAMAIS commités, décision B de l'utilisateur)

Pré-vol (2026-11-08 11:33, agent Arwen, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, main c158640) : `preflight.ps1 audio_panel -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen -MaxSeconds 900` PASS (129 s, 1 PNG, 96 sons joués, 0 « candidate missing »).
PNG OUVERTS et vus :
- audio_panel_start.png : l'écran du jeu (Plain, ton Centre principal, 5 Paysans) avec le panneau « Scénario audio_panel » : « Panneau d'écoute unique : 96 sons en 10 sections. Entrée = le son suivant… ».
NON VÉRIFIÉ : aucun son entendu par un agent (jamais possible) ; le gain « loud » (250 %) est joué par PlaySound2D à 2,5 : c'est le même gain que la table du jeu, mais pas la chaîne réelle d'un vrai clic sur un bâtiment ; le vote par LETTRE au clavier n'existe pas (il demanderait du C++, pas fait cette séance) : tu réponds ici.
Durée : plus que les 3 min de la règle 18, parce que tu as demandé TOUT en une commande : environ 20 min si tu écoutes les 96 sons (Entrée à chaque son) ; tu peux t'arrêter à tout moment et répondre pour les cues entendus.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_panel -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Réécouter un son : console (touche ²) puis `hol.Audio.Candidate <Cue> <lettre>` (ajouter `loud` pour le gain de 250 %).

Sections, dans l'ordre du panneau (le panneau affiche « [n/96, section k/10] <Cue> <lettre> (<fichier>) vient de jouer ») :

1. Élus, à 250 % puis à 100 % (6 sons) : Bois = F (s_wood_door), Sacré = H (s_sacred_book), Marché = handleCoins. Question : à 250 %, est-ce que ça CRAQUE / sature (écrêtage) ?
   Écrêtage à 250 % = _      (non / oui + quel(s) cue)
2. Pierre [Building.Stone] (candidats stone2, A à J) : Pierre = _
3. Métal [Building.Metal] (candidats metal2, A à J) : Métal = _
4. Alerte Attaque subie [Alert.UnderAttack] = _
5. Alerte Bâtiment fini [Alert.BuildingDone] = _
6. Alerte Technologie finie [Alert.TechDone] = _
7. Alerte Commerce [Alert.Trade] = _
8. Alerte Volcan [Alert.Volcano] = _
9. Alerte Diplomatie [Alert.Diplomacy] = _
10. Entraînement fini [Alert.TrainingDone] = _

Lettres (A à J, dans l'ordre joué) :
Building.Stone : A s_stone2_mine_deep, B s_stone2_mine_slow, C s_stone2_thud_low, D s_stone2_punch_deep, E s_stone2_timpani, F s_stone2_bass, G s_stone2_wood_heavy, H s_stone2_door_slow, I s_stone2_plank_low, J s_stone2_gong_dull
Building.Metal : A s_metal2_anvil_low, B s_metal2_plate_deep, C s_metal2_pot_deep, D s_metal2_pot_three, E s_metal2_tin_deep, F s_metal2_latch_deep, G s_metal2_click_deep, H s_metal2_mallet_soft, I s_metal2_gong_deep, J s_metal2_double
Alert.UnderAttack : A s_atk_drum_double, B s_atk_drum_deep, C s_atk_punch_double, D s_atk_wood_double, E s_atk_plate_double, F s_atk_mining_double, G s_atk_bass_double, H s_atk_gong_double, I s_atk_soft_double, J s_atk_metal_double
Alert.BuildingDone : A s_bdone_wood_tap, B s_bdone_plank, C s_bdone_door_close, D s_bdone_hammer, E s_bdone_mallet, F s_bdone_harp_low, G s_bdone_cello, H s_bdone_bass, I s_bdone_stone, J s_bdone_book
Alert.TechDone : A s_tech_book_flip, B s_tech_book_place, C s_tech_book_open, D s_tech_harp_low, E s_tech_harp_d4, F s_tech_cello, G s_tech_wood_soft, H s_tech_cloth, I s_tech_plate_light, J s_tech_knock_double
Alert.Trade : A s_trade_purse, B s_trade_purse_b, C s_trade_leather, D s_trade_drop, E s_trade_tin, F s_trade_wood_double, G s_trade_cloth_belt, H s_trade_soft, I s_trade_coins_double, J s_trade_generic
Alert.Volcano : A s_volc_timpani_deep, B s_volc_gong_low, C s_volc_bass, D s_volc_heavy_metal, E s_volc_punch, F s_volc_wood_rumble, G s_volc_plate, H s_volc_soft_boom, I s_volc_creak, J s_volc_mining
Alert.Diplomacy : A s_dipl_harp_low, B s_dipl_harp_arp, C s_dipl_cello, D s_dipl_bass, E s_dipl_book_open, F s_dipl_door, G s_dipl_cloth, H s_dipl_leather_double, I s_dipl_creak, J s_dipl_wood_double
Alert.TrainingDone : A s_train_leather_thud, B s_train_wood_knock, C s_train_plank_dull, D s_train_drum_soft, E s_train_bass_pluck, F s_train_mallet_dull, G s_train_stone_tap, H s_train_shield_muffled, I s_train_cloth_thump, J s_train_book_close

Réponse attendue : une lettre (ou plusieurs, ou « aucune, dix autres » + ce que tu aimes) par ligne `= _`, `OK`, `BUG: ...` ou `REGLAGE: ...`.
Les fiches séparées (E-audio_compare_buildings_*, E-audio_compare_alerts_*, E-audio_compare_training) restent jusqu'à ta validation de ce panneau.
