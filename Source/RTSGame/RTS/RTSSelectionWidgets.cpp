#include "RTSSelectionWidgets.h"

#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

// ---------------------------------------------------------------- Entry

void URTSSelectionEntryWidget::SetGroup(const FRTSSelectionGroup& InGroup)
{
	Group = InGroup;

	// Keep the brush's size and draw settings from the designer; only swap the texture.
	FSlateBrush Brush = IconImage->GetBrush();
	Brush.SetResourceObject(Group.Icon);
	IconImage->SetBrush(Brush);
	// A brush with no texture draws as a solid square, so tint it to act as a placeholder portrait.
	IconImage->SetColorAndOpacity(Group.Icon ? FLinearColor::White : NoIconColor);

	CountText->SetText(FText::AsNumber(Group.Count));

	if (NameText)
	{
		NameText->SetText(Group.DisplayName);
	}

	OnGroupSet(Group);
}

// ---------------------------------------------------------------- Panel

void URTSSelectionPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Controller = Cast<ARTSPlayerController>(GetOwningPlayer());
	if (Controller.IsValid())
	{
		Controller->OnSelectionChanged.AddUniqueDynamic(this, &URTSSelectionPanelWidget::Refresh);
	}
	Refresh();
}

void URTSSelectionPanelWidget::NativeDestruct()
{
	if (Controller.IsValid())
	{
		Controller->OnSelectionChanged.RemoveDynamic(this, &URTSSelectionPanelWidget::Refresh);
	}
	Super::NativeDestruct();
}

void URTSSelectionPanelWidget::Refresh()
{
	const TArray<FRTSSelectionGroup> Groups = Controller.IsValid() ? Controller->GetSelectionGroups() : TArray<FRTSSelectionGroup>();

	// Selections change a few times a second at most, so rebuilding the handful of entries is simplest.
	GroupContainer->ClearChildren();
	if (EntryClass)
	{
		for (const FRTSSelectionGroup& G : Groups)
		{
			URTSSelectionEntryWidget* Entry = CreateWidget<URTSSelectionEntryWidget>(this, EntryClass);
			GroupContainer->AddChild(Entry);
			Entry->SetGroup(G);
		}
	}

	// HitTestInvisible: the panel is display-only for now, so clicks on it still reach the world underneath.
	// Switch to SelfHitTestInvisible once entries become clickable (e.g. click a type to select only those).
	SetVisibility(Groups.Num() > 0 || !bHideWhenEmpty ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
