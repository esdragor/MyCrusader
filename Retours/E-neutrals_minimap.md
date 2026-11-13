# [LOGIQUE SEULEMENT] E-neutrals_minimap : un camp neutre vu par mes soldats apparaît sur la minicarte en carré ROUGE, puis GRIS une fois nettoyé ; un camp jamais vu n'y est pas

STATUT : LOGIQUE SEULEMENT   (pré-vol FAIL sur un seul contrôle, « user's launch » : 294 fichiers audio non commités `Content/Audio/Candidates/` du worktree Arwen, sans lien avec cet essai ; à lister prêt quand ces fichiers sont commités ou exclus et le pré-vol rejoué PASS)
GESTE: interne      (hol.Units.Move ; l'essai montre un affichage, pas un geste)
SON: sans objet
branches: d751-root

D789 (2026-11-12) : rien au survol d'un camp sur la minicarte. Vérifié : aucun survol / infobulle dans `Script/Neutrals/` (le point n'a pas de survol).

Pré-vol (2026-11-13 12:47, agent ARWEN-1113b, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, d751-root 577b744) : `preflight.ps1 neutrals_minimap -Root <worktree>` : check PASS (53 s), 6 s d'attente explicite, 0 waitkey, 2 PNG ; FAIL (1) = fichiers non commités du worktree (voir STATUT). WARN : pas de `hol.HUD.Report`, pas de `hol.Test.Sanity`.
PNG OUVERTS et vus (minicarte agrandie x4 pour le contrôle) :
- neutrals_minimap_alive : au centre de la minicarte, dans le cadre de la caméra, un carré ROUGE (pixel 237,97,89) sous le point bleu d'un Homme d'armes ; rien vers le coin du camp lointain (9000, 9000).
- neutrals_minimap_cleared (après le correctif 577b744) : le même carré est GRIS, un point bleu par-dessus ; toujours rien pour le camp lointain. Avant le correctif, un anneau rouge d'alerte de combat recouvrait le carré gris à la capture (d'où l'attente de 6 s ajoutée).
NON VÉRIFIÉ : client en réseau (réplication OwnerOnly : `net` de ce jour rouge sur 15 parties, voir NeutralsInProgress.md) ; taille du carré à l'œil de l'utilisateur (le point bleu de mes unités en couvre une grande partie).

Lancer UNE commande (quand le statut passe à [À ESSAYER]) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 neutrals_minimap -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
