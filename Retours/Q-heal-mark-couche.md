# [À RÉPONDRE] Marque de soin « + / ++ » vue par-dessus la fenêtre Arbre de talents

Contexte : HOLHealEffectSubsystem.cpp:40-41 dessine le trait (DrawDebugLine, SDPG_Foreground) et le texte (DrawDebugString, canvas du HUD). Les fenêtres sont du UMG ajouté par `Screen->AddToViewport()` (HOLGameHUD.cpp:79, ZOrder 0). Le canvas du HUD et la scène sont TOUJOURS sous Slate/UMG : aucun ZOrder ne peut y changer quelque chose. Si le « ++ » se voit « par-dessus », la fenêtre est translucide (la marque transparaît) ou le texte vient d'un autre dessin ; l'hypothèse de couche de la ligne `[ ] C++` d'Auto_Journey_File.md est donc fausse en l'état et non vérifiée (aucun lancement permis dans cet agent).

Options :
- A : ne rien changer au dessin ; rendre le fond de la fenêtre Arbre de talents opaque (réglage d'interface, script, sans build). Conséquence : plus de transparence sur la carte derrière la fenêtre.
- B : ne pas dessiner les marques (trait + texte) pendant qu'une fenêtre modale est ouverte (le script publie un booléen, le sous-système le lit ; C++ minimal). Conséquence : le soin n'est plus vu pendant la fenêtre ouverte.
- C : laisser tel quel, garder le contournement `hol.Cheat.Wound 100` du scénario.
- Autre.

Recommandation : A (cause probable = translucidité, correctif sans C++), après un PNG qui le confirme (talents_heal_cell sans le contournement).
Valeur par défaut codée : C (rien modifié).
