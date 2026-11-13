# [À ESSAYER] E-legions_roster_gesture : les noms des Legions (FR, EN, JA) apparaissent quand on clique les batiments et les unites, l'Aube garde ses noms

## PRE-VOL 2026-11-13 10:53-10:55 (FARAMIR-1113, sans C++, worktree Faramir = main 2503fed + merge 25f5094, dll fraiche) : VU A L'ECRAN
preflight.ps1 legions_roster_gesture -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir : PASS (103 s ; sequence 188 etapes « done in 26.4 s », ready : l'etape 41 « jamais venu » ne se produit plus ; section 6 lancement utilisateur OK ; WARN : pas de ligne Sanity). Les 9 PNG du .look ouverts (3 planches) et vus :
- barracks : Caserne / Vous / 1500, grille A Lancier squelette, Z Garde noire, E Guerrier damne (le defaut « Damned Warrior » en FR du 2026-11-11 est CORRIGE)
- blackguard : Garde noire 155/155, attaque 12, armures 4/4/2, 7 m/s ; rider : Cavalier funeste 120/120, 8,8 m/s
- aube_unit : Lancier rouge (IA Normal 2) 80/80 : nom d'Aube ; aube_barracks : Caserne rouge sans grille
- en_barracks : Barracks / You, Skeleton Spearman, Black Guard, Damned Warrior ; en_ram : Bone Ram 400/400, « Building attack 60 » (en anglais maintenant)
- ja_barracks : 兵舎 / あなた, スケルトン槍兵, 黒衛兵, 呪われた戦士 ; ja_ram : 骨の破城槌 400/400
GESTE: interne (hol.Select.* ; l'objet est l'AFFICHAGE des noms, l'utilisateur clique lui-meme) ; SON: sans objet ; branches: aucune
Lancer UNE commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 legions_roster_gesture -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
Duree : environ 1 min. Reponse attendue : OK, BUG: ... ou REGLAGE: ...
NON VERIFIE : vrai clic de souris (gestes internes) ; traduction japonaise a relire ; les sections ci-dessous sont l'historique.

STATUT : NE PAS JOUER (LOGIQUE SEULEMENT) ; cause : binaire de la copie jouée périmé (diagnostic 3b94faf) ; redonner quand la copie choisie est recompilée et que preflight.ps1 legions_roster_gesture -Root <cette copie> complet PASS. Historique, plus valable : vu à l'écran le 2026-11-11 (rejoué par l'intégrateur sur main fusionné : 9 PNG ouverts en 3 planches ; Caserne des Légions : grille Lancier squelette (A) / Garde noire (Z) ; bloc Garde noire et Cavalier funeste avec leurs stats ; Lancier d'Aube rouge nommé Lancier ; Caserne d'Aube sans grille Légions ; EN : Skeleton Spearman, Black Guard, Bone Ram ; JA : 兵舎, スケルトン槍兵, 黒衛兵, 骨の破城槌)
GESTE: interne   (hol.Select.ClickGround / Grid / ClickUnit = commandes hol.*, pas de vraie souris ; l'objet de l'essai est l'AFFICHAGE des noms)
SON: sans objet
branches: aucune

Pre-vol (2026-11-11 13:56, agent Glorfindel-26 sans C++, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main = merge de af56ff8) : `preflight.ps1 legions_roster_gesture -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` PASS (43 s, 9 PNG ; pan_exposure_test.ps1 PASS avant). WARN : pas de ligne Sanity.
PNG OUVERTS et vus :
- legions_gesture_barracks : Caserne bleue selectionnee, bloc du bas "Caserne / Vous / 1500", grille A "Lancier squelette", Z "Garde noire"
- legions_gesture_blackguard : bloc "Garde noire" (PV 155, attaque 12, armures 4/4/2, vitesse 7 m/s), pop 5/70
- legions_gesture_rider : bloc "Cavalier funeste" (PV 120, attaque 9, vitesse 8,8 m/s), unite grise selectionnee
- legions_gesture_aube_unit : unite rouge (IA Normal 2) "Lancier" PV 80 : nom d'Aube, pas Lancier squelette
- legions_gesture_aube_barracks : Caserne rouge "Caserne / IA Normal 2" sans aucune grille
- legions_gesture_en_barracks : "Barracks", grille "Skeleton Spearman" (A) et "Black Guard" (Z)
- legions_gesture_en_ram : "Bone Ram" 400/400, vitesse 2,2 m/s
- legions_gesture_ja_barracks : barre et bloc en japonais, grille "スケルトン槍兵" (A) et "黒衛兵" (Z)
- legions_gesture_ja_ram : bloc "骨の破城槌" 400/400
NON VERIFIE : un vrai clic de souris et le son (gestes internes, -RenderOffScreen) ; la grille de la Caserne rouge est absente parce qu'elle est ennemie (le point prouve donc surtout le nom, pas l'absence de noms Legions dans la grille) ; en EN le bloc du Belier garde "Attaque batiments 60" en francais (non juge : hors de l'essai, a signaler) ; la traduction japonaise reste une proposition a relire.

Lancer UNE commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 legions_roster_gesture -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir  (rejoue sur le build de main 521adcb fusionne, worktree Faramir ; sans -Hidden = fenetre, reservee a l'utilisateur ; un agent ajoute -Hidden)
Duree : environ 1 min. Reponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

## Integrateur Faramir 2026-11-11 15:09-15:16, main 521adcb, binaire 15:03 (session_gate -Phase Essay PASS)
Défaut constaté : la 3e case de la grille de la Caserne affiche « Damned Warrior » (E) en anglais dans l'interface FR et JA (non traduit) ; le bandeau du scénario reste en français en EN et JA. session_gate -Phase Essay dit « aucune branche citée » alors que la ligne est « branches: aucune » (gabarit : aucune est permis) : le contrôle du gate est trop strict. Durée : environ 1 min de jeu.

Reponse: le scenario n'a jamais demarre chez l'utilisateur (erreur etape 41 « jamais venu »), 2026-11-11 soir

## STATUT 2026-11-12 (FARAMIR-1112a) : LOGIQUE SEULEMENT, NE PAS JOUER
Cause (FARAMIR-1112a, 2026-11-12) : la commande donnee visait Faramir ; Faramir a avance par avance rapide sur main a 18:01 le 2026-11-11 (C++ de 17:01-17:17 : rage L4, Monter/Demonter) sans recompiler (dll 16:02) : scripts plus neufs que le binaire, l'etape 41 (waitlog HUD selection name: Garde noire) ne vient jamais. Deduit de la chronologie (journal du jeu de l'utilisateur ecrase), rejeu Faramir en cours a la fin de l'agent. binaire de main (copie principale, dll 2026-11-11 11:06) PERIME : 9 commits C++ depuis (rage L4, Monter/Demonter, camps) ; au lancement Angelscript: Error (Namespace 'HOLAIRageOrder' doesn't exist, UHOLCampSubsystem::GetNumCamps) et le jeu reste a l'image 0 (rejoue par FARAMIR-1112a, play.ps1 -Hidden depuis C:\Users\baudo). Arwen (dll 11-11 13:43) et Faramir (dll 11-11 16:02) sont aussi plus vieux que leur dernier commit C++ : recompiler la copie choisie avant de donner la commande.
Commande a redonner seulement apres : fog fusionnee dans main, copie choisie recompilee, puis preflight.ps1 <nom> -Root <cette copie> PASS (le preflight verifie maintenant le binaire, le joueur nomme et les fichiers non commites).

## ETAT 2026-11-13 (nettoyage Retours) : toujours NE PAS JOUER
Fait depuis : fog fusionnee dans main (b689e5a via maps, 4851635) et preflight durci (42e5e41) dans main. Reste : recompiler la copie choisie, puis preflight.ps1 legions_roster_gesture -Root <cette copie> PASS ; defaut « Damned Warrior » non traduit (integrateur Faramir 2026-11-11) a verifier au rejeu. NON VERIFIE : etat des binaires ce jour.
