#include "RTSHUDWidget.h"

#include "Components/PanelWidget.h"

UUserWidget* URTSHUDWidget::ShowBottomPanel(TSubclassOf<UUserWidget> PanelClass)
{
	if (!PanelClass)
	{
		return nullptr;
	}

	TObjectPtr<UUserWidget>& Panel = PanelCache.FindOrAdd(PanelClass);
	if (!Panel)
	{
		Panel = CreateWidget<UUserWidget>(this, PanelClass);
	}

	if (Panel != CurrentPanel)
	{
		PanelHost->ClearChildren();
		PanelHost->AddChild(Panel);
		CurrentPanel = Panel;
	}
	return Panel;
}
