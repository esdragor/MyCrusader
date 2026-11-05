# [À ESSAYER] E-neutral_color : un camp neutre est GRIS, jamais de la couleur d'un joueur

STATUT : VU À L'ÉCRAN (cause du « défaut commun » des neutres du 2026-10-31 : rien à reprocher à la couleur dans ce build).

Pré-vol (2026-11-05 20:43, agent Elrond-sans-C++, worktree HeroesOfLiberty-Elrond = branche tiers, main b39fafd + 88b0813, SANS build) : `preflight.ps1 neutral_color -Root ...Elrond` PASS (check 21 s, 8 s de jeu, HUD sans recouvrement, audit Sanity OK). Corrections de scénario faites : `hol.Camera.Zoom 4` au lieu de -4 (les lanciers faisaient ~10 px), `hol.HUD.Report`, `hol.Sanity.Watch 1` après le chargement de la carte, `hol.Test.Sanity` (avant : deux WARN).
PNG OUVERTS et vus :
- neutralcol_a : les 3 Lanciers du camp sont GRIS (pas bleus), lisibles (lances, boucliers) ; mon bâtiment hors cadre ; aucun combat.
- neutralcol_b : mes Paysans sont BLEUS (ma couleur), le camp gris est à 30 m hors cadre.
NON VÉRIFIÉ : la couleur sous le brouillard (voir scénario neutrals_fog, non refait) ; la couleur des neutres dans la mini-carte (points non lus).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 neutral_color -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : 15 s, rien à faire. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
