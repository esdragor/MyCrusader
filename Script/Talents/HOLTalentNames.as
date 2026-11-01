// The one table of the talents' names and effects (T5; GDD 13.2, D111, D195): every place that shows a talent takes its name from here,
// and the names are those of Tools/Data/Talents.csv (HOLTalentNames_Test.as compares the two, 24 by 24). Families: 0 Guardian, 1 Captain,
// 2 Champion; rows 2 to 9. French is the source (GatherText); English and Japanese are in Tools/Localization/fill_po.py, keys
// "HOLTalentNames,Name<row><family>" and "HOLTalentNames,Effect<row><family>". Literals only in the NSLOCTEXT calls. GENERATED ONCE by a
// script, edit by hand afterwards.
namespace HOLTalentNames
{
	FText Name(int Row, int Family)
	{
		if (Row == 2 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Name20", "Moisson bénie");
		if (Row == 2 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Name21", "Boucliers levés");
		if (Row == 2 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Name22", "Lame bénie");
		if (Row == 3 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Name30", "Vigie de l'Aube");
		if (Row == 3 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Name31", "Mains du guérisseur");
		if (Row == 3 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Name32", "Endurance du croisé");
		if (Row == 4 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Name40", "Étendard du rempart");
		if (Row == 4 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Name41", "Pas de l'Aube");
		if (Row == 4 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Name42", "Cri du défi");
		if (Row == 5 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Name50", "Remparts de lumière");
		if (Row == 5 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Name51", "Grâce abondante");
		if (Row == 5 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Name52", "Contre parfait");
		if (Row == 6 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Name60", "Double étendard");
		if (Row == 6 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Name61", "Frères d'armes");
		if (Row == 6 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Name62", "Escorte du champion");
		if (Row == 7 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Name70", "Bannière bénie");
		if (Row == 7 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Name71", "Charge disciplinée");
		if (Row == 7 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Name72", "Fer de lance");
		if (Row == 8 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Name80", "Rempart éternel");
		if (Row == 8 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Name81", "Ordre serré");
		if (Row == 8 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Name82", "Sentence publique");
		if (Row == 9 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Name90", "Jugement des assiégeants");
		if (Row == 9 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Name91", "Renforts aguerris");
		if (Row == 9 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Name92", "Verdict des armes");
		return FText();
	}

	FText Effect(int Row, int Family)
	{
		if (Row == 2 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Effect20", "Rayon du Sanctuaire +25 %");
		if (Row == 2 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Effect21", "+1 armure aux unités dans l'Aura de l'Aube");
		if (Row == 2 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Effect22", "Hors duel, environ 1 attaque de mêlée sur 4 contre le Paladin est parée");
		if (Row == 3 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Effect30", "Tours : +15 % de vision et +10 % de portée");
		if (Row == 3 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Effect31", "Moines dans l'aura : soins +20 %");
		if (Row == 3 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Effect32", "Paladin : +10 % de PV, régénération hors combat doublée");
		if (Row == 4 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Effect40", "Murs et tours dans la zone de la Bannière : plus d'armure, tours plus rapides");
		if (Row == 4 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Effect41", "Sous Serrez les rangs, la formation avance à 50 % de sa vitesse au lieu de rester sur place, sans perdre sa réduction de dégâts");
		if (Row == 4 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Effect42", "Lancer un défi donne 1 charge de Ferveur (~15 s) aux alliés proches, quelle que soit la réponse");
		if (Row == 5 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Effect50", "Le Rempart béni protège aussi les segments et tours voisins");
		if (Row == 5 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Effect51", "Lumière sacrée : une seconde charge");
		if (Row == 5 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Effect52", "Une riposte sur une attaque forte annoncée étourdit ~0,5 s et compte double pour l'Épreuve");
		if (Row == 6 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Effect60", "Deux Bannières en même temps");
		if (Row == 6 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Effect61", "Défi du Chevalier (passif) : dans l'aura, redirige deux fois plus de dégâts");
		if (Row == 6 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Effect62", "Renforts de l'Aube : les Chevaliers arrivent à côté du Paladin");
		if (Row == 7 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Effect70", "Hors combat, les alliés dans la zone de la Bannière régénèrent lentement");
		if (Row == 7 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Effect71", "Les unités qui ont suivi la charge subissent ~−15 % de dégâts pendant ~8 s après l'impact");
		if (Row == 7 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Effect72", "Impact de la charge du Paladin : +30 % de dégâts");
		if (Row == 8 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Effect80", "Rempart béni : durée +30 %");
		if (Row == 8 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Effect81", "Serrez les rangs : recharge −20 %");
		if (Row == 8 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Effect82", "Chaque Faute consommée rapporte un peu d'Honneur");
		if (Row == 9 && Family == 0)
			return NSLOCTEXT("HOLTalentNames", "Effect90", "Jugement : +50 % de dégâts aux engins de siège");
		if (Row == 9 && Family == 1)
			return NSLOCTEXT("HOLTalentNames", "Effect91", "Renforts de l'Aube : +1 Chevalier Vertueux dans l'escouade");
		if (Row == 9 && Family == 2)
			return NSLOCTEXT("HOLTalentNames", "Effect92", "Jugement lancé dans les ~30 s d'un duel gagné : ~50 % d'Honneur en moins");
		return FText();
	}

	// Said of a talent the prototype cannot show yet (no Monk, no wall bonus...): it is taken, nothing is hidden (D194).
	FText NoEffectYet()
	{
		return NSLOCTEXT("HOLTalentNames", "NoEffectYet", "pas encore actif dans ce prototype");
	}
}
