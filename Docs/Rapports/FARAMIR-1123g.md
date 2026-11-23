APPELS_DECLARES: 24 (arret depasse : build en file)
HEURE_DEBUT/HEURE_FIN: 10:54 / 11:01
PREMIER_BUILD: 10:56 (lance, en attente/compilation au moment du rapport)   BUILD: NON COMPILE (build bg8zgc9rb toujours actif a la fin)   LAUNCHCHECK_DEBUT: non verifie
TEST_ROUGE: Zone.KillHonor rouge vu 10:50 (1123f, 2099/2100) + assertit l'effet : oui   TEST_ROUGE_SEPARE: oui (9442b74)
COMMITS: 1 (correctif + test adapte, NON COMPILE), merge_main_fait: non
TESTS: non lances ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune (GDD D859 tranche : +1 par ennemi tue a moins de 12 m d'un BATIMENT de l'Aube ; D76 : ennemis tues pres de ses batiments ; HOLDawnValues.as:34 deja conforme)
FAIT / RESTE: CreditKillHonor = CreditHonorPoints(World, Slot, HonorKillGain) ; AwardKillHonor parcourt StepBuildings (Box.ComputeSquaredDistanceToPoint, Camp, bGone) ; test Zone.KillHonor avec un MainCentre slot 0 a 10 m. RESTE : build, quick_check -Parts tests (Zone.* et Heal.*), merge main.
NON VERIFIE: compilation (FBox2D::ComputeSquaredDistanceToPoint avec FVector2D(FVector) ; StepBuildings rempli avant le premier Kill du pas), vert de Zone.KillHonor (le MainCentre doit etre un batiment non fondation dans le monde de test)
PROCESSUS_DETACHES: voir sortie du dernier geste
ANOMALIES_PROCESSUS: build attendu en file ~5 min (Haldir devant) ; non arrete
