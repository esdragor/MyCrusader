#include "Dev/HOLUIMouse.h"

#include "Widgets/SWidget.h"

// Red step (L14-5): the mouse does nothing yet.
namespace UE::HeroesOfLiberty::UI::Mouse
{
	void MoveTo(const FVector2D& Position)
	{
	}

	void Click(const FVector2D& Position)
	{
	}

	void RestorePlatformCursor()
	{
	}

	FVector2D GetCentre(const SWidget& Widget)
	{
		return FVector2D(Widget.GetTickSpaceGeometry().GetAbsolutePositionAtCoordinates(FVector2f(0.5f, 0.5f)));
	}
}
