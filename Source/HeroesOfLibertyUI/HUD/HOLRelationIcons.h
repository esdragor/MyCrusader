#pragma once

#include "CoreMinimal.h"

enum class EHOLPlayerRelation : uint8;

/**
 * E4-1: the icon before another player's name in the players' table: a green shield for an ally, red crossed swords
 * for an enemy, nothing for the viewer's own row or a spectator's. The textures are made by
 * Tools/Editor/make_ui_icons.py and cooked with /Game/UI/Icons (Config/DefaultGame.ini).
 */
namespace HOLRelationIcons
{
	inline constexpr const TCHAR* AllyPath = TEXT("/Game/UI/Icons/T_Icon_Ally.T_Icon_Ally");
	inline constexpr const TCHAR* EnemyPath = TEXT("/Game/UI/Icons/T_Icon_Enemy.T_Icon_Enemy");

	/** The object path of the relation's icon, nullptr when it has none (Self, None). */
	HEROESOFLIBERTYUI_API const TCHAR* GetIconPath(EHOLPlayerRelation Relation);
}
