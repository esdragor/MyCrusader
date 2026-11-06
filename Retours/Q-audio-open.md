# [À RÉPONDRE] Q-audio-open : ta réponse sur l'essai audio_compare dit « UI.Open F » puis « UI.Open A »

Contexte : tu as répondu « UI.Click H, pas de son quand tu hover, UI.Open F, UI.Open A, Ui.Back B ... ». Il n'existe qu'UN cue Ui.Open (10 variantes) dans audio_compare, audio_compare_orders et audio_compare_alerts. Dans la liste de l'essai, l'ordre est Click, Hover, Open, **Close**, Back, Error, Confirm...; ta 2e ligne « Open A » tombe exactement à la place de **Close**.

Options :
- **A** : Ui.Open = F (bookFlip2, page tournée) ET Ui.Close = A (bookClose). Probable faute de frappe : la 2e ligne était Close.
- **B** : Ui.Open = A (bookOpen) : la 2e ligne remplace la 1re ; Close reste à choisir.
- **C** : Ui.Open = F, Close non choisi (reste les sons actuels « vaisseau spatial » jusqu'à ton prochain essai).
- **Autre** : dis-moi la lettre pour Open et pour Close.

Conséquences visibles : seul le son d'ouverture (arbre de talents, panneau Diplomatie) et de fermeture change ; rien d'autre.

Recommandation : **A** (lecture de ta liste dans l'ordre de l'essai).

Valeur par défaut codée (consigne reçue : ne pas deviner) : **C** = Open F, Close inchangé. Pour passer à A : dans `Script/Audio/HOLAudioTable.as`, ligne `Ui.Close`, remplacer le pack et les fichiers par `"rpg-audio", "bookClose"` (puis importer `bookClose`, une commande : `Tools/Editor/import_kenney_sounds.py`).
