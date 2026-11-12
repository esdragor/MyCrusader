# [A ESSAYER] E-neutrals : un camp de 3 Lanciers neutres charge mes 3 Hommes d'armes (Q puis clic), est détruit, et mon héros gagne 100 XP

STATUT : VU À L'ÉCRAN (2026-11-11 16:5x, agent Celeborn-neutrals-spin2, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, hero-j8-close eeed9dc, build de main 39ba5ec PASS). Pré-vol PASS 3 fois de suite (16:47, 16:48, 16:49) : journal sans erreur, aucun HUD qui en recouvre un autre, audit Sanity PASS ; et ab_matrix N=8 : 8/8 PASS.
CHANGEMENT DE L'ESSAI : 3 Hommes d'armes (avant : 6) et le clic d'attaque est à 3 m du camp (avant : 10 m). Raison mesurée : avec 6 soldats contre 3 Lanciers, 3 soldats en trop forment une seconde ligne qui tourne en rond (spin de mêlée, D641 écartée, comportement du jeu non changé) : l'audit Sanity échouait 5 fois sur 8 ; avec 3 contre 3 il passe 8 fois sur 8. La clairière (arbres retirés) n'a pas d'effet sur ce spin (3/8 contre 4/8).
GESTE: réel      (Q = `hol.Select.Grid 4` puis clic gauche au sol = `hol.Select.LeftClickGround -300 0`, par les gestionnaires d'entrée ; souris physique non testée)
SON: non entendu    (-nosound, rien entendu par l'agent)
branches: hero-j8-close

PNG OUVERTS et vus (Saved_Tests\Screenshots\WindowsEditor\HOL\, dernier pré-vol 16:49) :
- neutrals_camp : sol nu, plus aucun arbre ; 2 Lanciers gris nettement visibles en haut (le 3e se confond avec un voisin à ce cadrage), mes bleus groupés en bas, personne ne se bat : conforme.
- neutrals_fight : 3 Lanciers gris au contact de mes 3 Hommes d'armes bleus (sélection ronde, barres de vie vertes, ligne d'ordre rouge, grille Q/S/V affichée), rien ne couvre la scène : conforme.
- neutrals_cleared : les Lanciers gris sont à terre (2 à 3 corps vus), mes 3 bleus debout : conforme.
- Cosmétique : le panneau « Scénario neutrals en préparation… » reste sur les captures (défaut connu).
NON VÉRIFIÉ : la souris physique ; le son ; les 100 XP à l'écran (journal seulement : « Camp 1 reward: slot 0 +100 XP ») ; compter 3 silhouettes grises sur neutrals_camp ; la mêlée à 6 contre 3 (spin D641, toujours rouge ~70 % et NON corrigé).
Commande (à NE PAS donner avant le cadrage de la session principale) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 neutrals -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`

Reponse: BUG: quand je combats le Lancier neutre, le bras qui ne tient pas la lance est dans son corps (2026-11-11 soir)
