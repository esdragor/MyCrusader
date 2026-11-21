# FARAMIR-1121sc

APPELS_DECLARES: 22 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 19:22 / 19:25 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun (tache = scenario) ; assertit l'effet : sans objet
COMMITS: 1 (2a11ec2, ~19:23, branche zone-effect), merge_main_fait: non (aucun merge demande)
TESTS: preflight talents_heal x2. Run 1 (apres ajout HUD.Report + Test.Sanity) : 4 FAIL = pas de mapSize dans talents_heal.json, fichier non commite, HUD overlap HeroPanel x FactionPanel (x2 : log + HUD rects) ; WARN frames 40. Run 2 (apres mapSize Small, commit, frames 40 -> wait 1, wait 8 avant le rapport HUD) : check PASS (37 s), sanity audit passed, FAIL (2) restants = HUD overlap HeroPanel x FactionPanel (log + HUD rects). TESTS_TAUTOLOGIQUES: 0
ESSAIS: talents_heal : preflight FAIL (2), 3 PNG produits, talents_heal_cell OUVERT (plus de « ++ » sur la fenetre Arbre de talents : vu), base et boosted non ouverts ; LOGIQUE SEULEMENT, pas de fiche E-talents_heal.md (preflight non PASS), aucune commande a donner.
QUESTIONS: aucune
FAIT / RESTE:
- talents_heal.txt : hol.HUD.Report 1 avant talents_heal_cell (apres wait 8 pour l'effacement de l'avis jaune), hol.Test.Sanity avant talents_heal_boosted, frames 40 -> wait 1. talents_heal.json : mapSize Small (D820). Commit 2a11ec2.
- Defaut restant (hors de mes fichiers) : le bloc du heros (avec la rangee d'icones de talents pris et « Talents pris (2) ») est allonge vers le haut et recouvre la jauge SacredLight du FactionPanel (visible sur talents_heal_cell : « 0 / 100, SacredLight cost 25 soon » sur le bloc du heros). Correctif = interface (Source/Script interdits ici). Note : ce n'est pas le scenario qui est faux, c'est un vrai recouvrement ; ne pas deplacer le HUD.Report pour le masquer.
- Auto_Journey_File.md : ligne talents_heal_cell mise a jour (contournement fait, FAIL preflight note) + ligne [ ] C++ ajoutee (HOLHealEffectSubsystem.cpp : vrai defaut de couche). EtapeDefini.md : ligne FARAMIR-1121sc ajoutee.
- RESTE : corriger le recouvrement HeroPanel x FactionPanel (ou le trancher : decaler la jauge de faction), relancer preflight talents_heal, ouvrir base et boosted, creer Retours\E-talents_heal.md ([À ESSAYER], branches: zone-effect, GESTE: reel (T puis clic case), SON: non entendu).
NON VERIFIE: que talents_heal_base et talents_heal_boosted montrent le « + » vert puis « ++ » dore (PNG non ouverts) ; que hol.Cheat.Wound 100 efface bien la marque (le « ++ » absent de la fenetre peut aussi venir de la camera, qui regarde un autre endroit avec mapSize Small).
PROCESSUS_DETACHES: voir sortie stray_processes lancee en dernier geste (aucun attendu : preflight termine)
ANOMALIES_PROCESSUS: aucune ; DIAGNOSTICS_AS: sans objet (aucun .as touche)
GIT_STATUS_PRINCIPALE: modifs docs voulues seulement (Auto_Journey_File.md, EtapeDefini.md) en plus de l'etat de depart : oui
