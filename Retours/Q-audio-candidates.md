# [A REPONDRE] Q-audio-candidates : les 145 sons candidats entrent-ils dans git ?

Contexte : pour remplacer les sons « console de vaisseau spatial » de l'interface, 145 sons candidats (Kenney rpg-audio / impact-sounds + VSCO 2 CE, tous CC0) sont importes en `Content/Audio/Candidates/<kenney|vsco>/SW_<nom>.uasset` : 6,2 Mo. Ils ne servent qu'a l'ecoute ; une fois tes lettres choisies, seuls les elus passent dans la table.

- A : commiter les 145 .uasset (6,2 Mo, historique a vie). Un worktree neuf entend les candidats tout de suite.
- B (recommande, defaut code : rien n'est commite) : ne pas les commiter ; `python Tools\Audio\make_ui_candidates.py` (telecharge les echantillons VSCO, 30 s) puis l'import editeur `Tools\Editor\import_audio_candidates.py` (30 s) les regenerent dans n'importe quel worktree. Les elus seront commites apres ton choix (comme D736).
- Autre : precise.

Consequence visible : en B, les scenarios `audio_compare*` ne jouent rien (log `candidate missing`) dans un worktree sans import ; ils marchent dans Arwen (`HeroesOfLiberty-Arwen`) ou ils sont deja importes. En A, partout.
Attention : tant que tu n'as pas choisi, la table du jeu (`HOLAudioTable.as`) garde ses sons actuels (Kenney interface-sounds) : je ne choisis pas a ta place.
