#include "HUD/HOLRelationIcons.h"

#include "Match/HOLForumRules.h"

const TCHAR* HOLRelationIcons::GetIconPath(const EHOLPlayerRelation Relation)
{
	switch (Relation)
	{
	case EHOLPlayerRelation::Ally:
		return AllyPath;
	case EHOLPlayerRelation::Enemy:
		return EnemyPath;
	default:
		return nullptr;
	}
}
