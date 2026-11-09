# [À ESSAYER] E-audio_select_gain : écoute AVANT / APRÈS des sons de sélection (Bois, Sacré, Marché) : l'ancien x2,5 écrêté contre le nouveau limité à -1 dBFS

STATUT : LOGIQUE SEULEMENT   (aucune oreille n'a écouté : -nosound côté agent ; ne devient « prêt » qu'après ton écoute)
GESTE: interne      (les sons sont joués par la commande hol.Audio.SelectGain du scénario ; seule la touche Entrée est la tienne)
SON: non entendu   (préflight lancé sans son ; passe à « entendu » quand tu as écouté)
branches: d751-root   (fichiers limités, table, commande et scénario viennent du worktree Arwen ; pas dans main)

Contexte : réponse B à Q-audio-select-gain. Les trois sons sont refaits avec +8 dB puis un limiteur à -1 dBFS et joués à 100 % (au lieu de x2,5 qui écrêtait). Gain réel : Bois +4,7 dB, Sacré +3,9 dB, Marché +2,4 dB (handleCoins reste très tassé : il était déjà au maximum). Pour chaque famille : ANCIEN, NOUVEAU, ANCIEN, NOUVEAU ; l'écran dit le cue et la version qui viennent de jouer.

Pré-vol (2026-11-09 16:44, agent Arwen, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, main = ce36d77) : `preflight.ps1 audio_select_gain -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (54 s, 1 PNG, 12 lignes « select gain played » dont les 6 NEW lues de la vraie table (assets *_lim à 100), 0 « select gain missing »).
PNG OUVERTS et vus :
- audio_select_gain_start.png : l'écran du jeu (Plain, ton Centre principal, 5 Paysans) avec le panneau « Scénario audio_select_gain » : « AVANT / APRES : pour chaque famille (Bois, Sacre, Marche) l'ANCIEN son (x2,5, ecrete, sature) puis le NOUVEAU (limite a -1 dBFS, joue a 100 %)… ».
NON VÉRIFIÉ : aucun son entendu par un agent ; les textes d'attente par son (« [3/12] Bois [Select.BuildingWood] … ANCIEN … vient de jouer. Entrée : … ») ne sont pas capturés (le préflight passe les waitkey tout seul) ; le rendu réel dans le mixeur (Unreal peut avoir son propre limiteur) ; le niveau perçu : mesuré sur les fichiers seulement (pics -1,0 dBFS, moyennes -14,4 / -17,4 / -26,4 dB).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_select_gain -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min (12 sons, Entrée à chaque son). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: <famille, plus fort / moins fort / autre>` (le niveau se règle en relançant `Tools\Audio\make_limited_selects.py` avec un autre gain puis `Tools\Editor\import_audio_elected.py`).
