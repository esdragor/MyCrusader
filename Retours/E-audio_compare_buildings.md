# [À ESSAYER] E-audio_compare_buildings : écoute le son de SELECTION d'un batiment, une famille apres l'autre : bois, pierre, sacre, metal, pieces (cuir, cloches, harpe, forge...) ; tu réponds par cue, la lettre que tu préfères

STATUT : LOGIQUE SEULEMENT   (aucune oreille n'a encore écouté : l'agent a lancé les scénarios sans son)
GESTE: interne      (les sons sont joués par la commande hol.Audio.Candidate du scénario, pas par un vrai clic ; seule la touche Entrée est la tienne)
SON: non entendu   (-nosound côté agent ; passe à « entendu » quand quelqu'un a écouté)
branches: heal-sleep   (le scénario et les candidats viennent du worktree Arwen ; candidats JAMAIS commités, décision B de l'utilisateur)

Réponses : remplace le _ par la lettre préférée (A à J), plusieurs lettres possibles, ou « aucune, dix autres » et dis ce que tu aimes.
Bois (Maison, camps, Moulin, Ferme) [Building.Wood] = _
Pierre (Centre principal, Caserne, Champ de tir, Ecurie, Atelier de siege) [Building.Stone] = _
Sacre (Chapelle, Sanctuaire) [Building.Sacred] = _
Metal (Forge) [Building.Metal] = _
Pieces (Marche) [Building.Coins] = _

Pré-vol (2026-11-07 10:45, agent ARWEN-14, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, branche heal-sleep, binaire du 09:00) : `preflight.ps1 audio_compare_buildings -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (79 s, 1 PNG, sanity audit passed, 50 lignes « candidate played Building. » sur 50, 0 « candidate missing »).
Licences vérifiées (Docs\assets_audio.csv) : Kenney rpg-audio et impact-sounds = CC0 (License.txt présents dans C:\Users\baudo\Documents\MyCrusader_Audio\Kenney), VSCO 2 CE = CC0 1.0 (LICENSE présent dans C:\Users\baudo\Documents\MyCrusader_Audio\VSCO) ; aucun nouveau pack, les 50 fichiers sont des candidats déjà importés.
PNG OUVERTS et vus :
- audio_compare_buildings_start : l'écran du jeu (forum, 5 Paysans, forêt, panneau du scénario avec les 3 notes : écoute le son de SELECTION d'un bâtiment, 50 sons, 5 cues, comment réécouter avec hol.Audio.Candidate Building.Wood C, réponds par cue) ; aucune fenêtre ne cache le panneau.
NON VÉRIFIÉ : SON non entendu (aucune oreille : que les 50 sons soient audibles, distincts, bien rangés par famille) ; l'avance par la touche Entrée en jeu réel (waitkey passés en -unattended) ; durée : 50 sons = environ 11 min, au-dessus des 3 min d'un essai (à écouter famille par famille : une lettre par famille suffit, la réponse peut être donnée en plusieurs fois) ; la correspondance famille/bâtiment vient de ta réponse Q3 = A (10:36), rien d'autre n'est tranché ; rien n'est branché dans le jeu (Select.Building reste muet).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_buildings -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 11 min (50 sons, Entrée = variante suivante ; le panneau dit « <Cue> <lettre> (<fichier>) vient de jouer »).
Réponse attendue : une lettre par cue, `OK`, `BUG: ...` ou `REGLAGE: ...`.
