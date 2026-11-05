# [A REPONDRE] Q-audio-a2b : les 79 sons importes entrent-ils dans git ?

Contexte : A2b a importe les 79 .ogg Kenney (CC0, hors depot) en `Content/Audio/Kenney/<pack>/SW_<nom>.uasset` : 1,89 Mo en tout. Le jeu les charge par chemin ; sans eux le journal dit `cue missing` et rien ne joue.

- A (recommande, defaut code : rien a changer dans le code) : commiter les 79 .uasset (1,89 Mo, comme les autres SW_ du depot, CC0 donc redistribuable, registre `Docs/assets_audio.csv` deja tenu). Un clone entend les sons tout de suite, le paquet les emporte.
- B : ne pas les commiter, les regenerer a chaque machine par `Tools\Editor\import_kenney_sounds.py` (20 s, demande les packs hors depot dans `MyCrusader_Audio\Kenney`). Depot plus leger, mais un worktree neuf est muet tant qu'on n'a pas lance l'import.
- Autre : precise.

Consequence visible : en B les worktrees sans import sont silencieux (log `cue missing`), en A non. Etat actuel : les .uasset sont dans le worktree Galadriel, NON commites (decision pendante).

Reponse: A (commiter les .uasset). 2026-11-05 20:38, utilisateur.
