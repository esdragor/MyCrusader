# [REPONDUE B, APPLIQUEE 2026-11-09] Q-audio-select-gain : le volume x2,5 des sons de sélection (Bois, Sacré, Marché) écrête

Contexte : tu as demandé « volume x2 à x3 » pour Bois (F, s_wood_door) et Sacré (H, s_sacred_book), et le Marché joue handleCoins au même gain (250 % = `HOLAudio::BuildingSelectGainPercent`, `Script/Audio/HOLAudioTable.as`). Mesure faite le 2026-11-09 par `Tools\Audio\check_audio_headroom.py` (ffmpeg, vrais fichiers, aucune oreille) :

| Son | pic à 100 % | pic à x2,5 | niveau moyen à x2,5 | avec un limiteur à -1 dBFS (même gain) |
|---|---|---|---|---|
| Bois s_wood_door | -4,0 dBFS | +4,0 (écrête de 4 dB) | -11,1 dB | pic -1,0 ; moyen -14,4 (soit +4,7 dB au lieu de +8) |
| Sacré s_sacred_book | -4,0 dBFS | +4,0 (écrête de 4 dB) | -13,3 dB | pic -1,0 ; moyen -17,4 (+3,9 dB) |
| Marché handleCoins | 0,0 dBFS | +8,0 (écrête de 8 dB) | -20,8 dB | pic -1,0 ; moyen -26,4 (+2,4 dB) |

Au-delà de 0 dBFS le mixeur écrête (saturation), surtout le Marché. Écoute la section 1 du panneau `audio_panel` (élus à 250 % puis à 100 %) pour juger.

Options :
- A : garder x2,5 tel quel (défaut codé, rien ne change) ; tu juges à l'oreille si ça te gêne.
- B : fichiers « forts » refaits avec un limiteur à -1 dBFS (gain +8 dB puis limite), joués à 100 % : même plus de présence, sans saturation, mais gain réel plus petit (+2 à +5 dB selon le son) ; le Marché garde alors un son très tassé.
- C : gain par famille (Bois et Sacré x1,5 qui ne dépasse pas 0 dBFS, Marché x1,0) : propre, mais moins fort que demandé.
- Autre : dis le gain voulu.

Conséquence visible : A = comme aujourd'hui (possible grésillement, surtout le Marché) ; B = son plus plein et propre, un peu moins fort que x2,5 ; C = propre mais plus doux.
Recommandation : B pour Bois et Sacré, A pour le Marché le temps d'écouter (handleCoins est déjà au maximum).
Valeur par défaut codée : A (`BuildingSelectGainPercent = 250`, une ligne ; rien d'autre ne bouge).
Note : rien n'est branché en jeu pour Pierre et Métal (aucun son élu, dix autres candidats dans `E-audio_panel.md`).

Reponse (utilisateur, 2026-11-09 ~16:30) : B, et faire un scenario avant/apres. Tache confiee a Arwen-3 (worktree Arwen), fiche E-audio_select_gain a venir.

APPLIQUEE (Arwen, d751-root, 2026-11-09 16:4x, sans C++, pas dans main) : Bois, Sacre ET Marche en B (l'utilisateur n'a pas nuance pour le Marche). Fichiers refaits par `Tools\Audio\make_limited_selects.py` (+8 dB puis alimiter -1 dBFS) : s_wood_door_lim (pic -1,0 ; moyen -14,4 dB), s_sacred_book_lim (-1,0 ; -17,4), s_coins_select_lim (-1,0 ; -26,4), importes sous /Game/Audio/Soft, joues a 100 % (`BuildingSelectGainPercent = 100` ; l'ancien 250 reste dans `BuildingSelectOldGainPercent` pour le scenario seulement). Gain reel : Bois +4,7 dB, Sacre +3,9 dB, Marche +2,4 dB : handleCoins reste tres tasse (il etait deja au maximum). Controle : `check_audio_headroom.py --strict` (aucun son de selection au-dessus de -1 dBFS). Essai : `E-audio_select_gain.md` (scenario `audio_select_gain`, ancien / nouveau en alternance). Pour changer le niveau voulu : relancer `make_limited_selects.py` avec un autre gain (constante GAIN_DB) puis `import_audio_elected.py`.

Essai E-audio_select_gain joue par l'utilisateur 2026-11-09 17:00 : OK. Question close.
