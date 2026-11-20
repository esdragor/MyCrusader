# [REPONDU : A, GDD D874] Q-garrison-portrait : que montre un portrait de la garnison dans le cadre du batiment ?

Contexte : la ligne de garnison du cadre (HOLGarrisonFrame.as, compteur « 2 / 5 » + un bouton par occupant, clic = « cette unite sort ») doit dessiner chaque occupant ; la lecture client ne donne pour l'instant que l'identifiant d'unite (HOLBuilding::GetGarrisonOccupants).

- A : l'icone du type d'unite (comme la file de production / les puces), recommande. Demande une lecture client du type par identifiant (C++, lot groupe) ; la fiche d'unite n'existe pas encore pour toutes les unites (Placeholders.txt si l'icone manque).
- B : le nom du type en texte (« Lancier »), sans icone : aucun asset, un peu austere, meme lecture C++.
- C : l'identifiant brut (valeur codee en attendant, a ne pas livrer au joueur).
- Autre : ...

Valeur par defaut codee : C (texte = identifiant d'unite dans HOLGarrisonFrame::Refresh, une ligne a changer). Consequence en jeu : sans reponse le joueur voit des numeros sur les boutons de la garnison.

Reponse: A (utilisateur, 2026-11-20 ~15:5x, interface de choix) : le bouton d'un occupant montre l'icone du type d'unite (comme la file de production) ; icone manquante = ligne Placeholders.txt. Enregistre : GDD D874 ; ligne `[ ]` en tete de « Elrond » dans Auto_Journey_File.md (apres le lot C++ de Glorfindel).