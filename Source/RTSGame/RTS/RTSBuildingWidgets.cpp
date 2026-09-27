#include "RTSBuildingWidgets.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "RTSBuilding.h"
#include "RTSPlayerController.h"
#include "RTSUnit.h"

#define LOCTEXT_NAMESPACE "RTSBuildingWidgets"

// ---------------------------------------------------------------- Production button

void URTSProductionButtonWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	ProduceButton->OnClicked.AddDynamic(this, &URTSProductionButtonWidget::OnProduceClicked);
}

void URTSProductionButtonWidget::SetUnit(ARTSBuilding* InBuilding, TSubclassOf<ARTSUnit> InUnitClass, const FText& InHotkey)
{
	Building = InBuilding;
	UnitClass = InUnitClass;

	// Name and icon live on the unit Blueprint's Class Defaults, so read them from its CDO.
	const ARTSUnit* UnitDefaults = InUnitClass ? GetDefault<ARTSUnit>(InUnitClass) : nullptr;
	UTexture2D* Icon = UnitDefaults ? UnitDefaults->GetUnitIcon() : nullptr;
	if (Icon)
	{
		IconImage->SetBrushFromTexture(Icon);
		IconImage->SetColorAndOpacity(FLinearColor::White);
	}
	else
	{
		IconImage->SetBrushFromTexture(nullptr);
		IconImage->SetColorAndOpacity(NoIconColor);
	}

	HotkeyText->SetText(InHotkey);
	if (NameText)
	{
		NameText->SetText(UnitDefaults ? UnitDefaults->GetUnitDisplayName() : FText::GetEmpty());
	}
}

void URTSProductionButtonWidget::OnProduceClicked()
{
	if (Building.IsValid())
	{
		Building->QueueUnit(UnitClass);
	}
}

// ---------------------------------------------------------------- Building panel

void URTSBuildingPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ButtonClass)
	{
		ButtonClass = LoadClass<URTSProductionButtonWidget>(nullptr, TEXT("/Game/RTS/UI/WBP_ProductionButton.WBP_ProductionButton_C"));
	}

	Controller = Cast<ARTSPlayerController>(GetOwningPlayer());
	if (Controller.IsValid())
	{
		Controller->OnSelectionChanged.AddUniqueDynamic(this, &URTSBuildingPanelWidget::Refresh);
	}
	Refresh();
}

void URTSBuildingPanelWidget::NativeDestruct()
{
	if (Controller.IsValid())
	{
		Controller->OnSelectionChanged.RemoveDynamic(this, &URTSBuildingPanelWidget::Refresh);
	}
	BindToBuilding(nullptr);
	Super::NativeDestruct();
}

void URTSBuildingPanelWidget::BindToBuilding(ARTSBuilding* NewBuilding)
{
	if (Building.IsValid())
	{
		Building->OnProductionQueueChanged.RemoveDynamic(this, &URTSBuildingPanelWidget::RefreshQueue);
	}
	Building = NewBuilding;
	if (NewBuilding)
	{
		NewBuilding->OnProductionQueueChanged.AddUniqueDynamic(this, &URTSBuildingPanelWidget::RefreshQueue);
	}
}

void URTSBuildingPanelWidget::Refresh()
{
	ARTSBuilding* NewBuilding = Controller.IsValid() ? Controller->GetSelectedBuilding() : nullptr;
	BindToBuilding(NewBuilding);

	ButtonContainer->ClearChildren();
	if (!NewBuilding)
	{
		if (BuildingNameText) { BuildingNameText->SetText(FText::GetEmpty()); }
		RefreshQueue();
		return;
	}

	if (BuildingNameText)
	{
		BuildingNameText->SetText(NewBuilding->GetBuildingDisplayName());
	}

	if (ButtonClass)
	{
		const TArray<TSubclassOf<ARTSUnit>> Units = NewBuilding->GetProducibleUnits();
		for (int32 SlotIndex = 0; SlotIndex < Units.Num(); ++SlotIndex)
		{
			URTSProductionButtonWidget* Button = CreateWidget<URTSProductionButtonWidget>(this, ButtonClass);
			Button->SetUnit(NewBuilding, Units[SlotIndex], Controller->GetProductionHotkeyText(SlotIndex));
			ButtonContainer->AddChild(Button);
		}
	}
	RefreshQueue();
}

void URTSBuildingPanelWidget::RefreshQueue()
{
	if (!QueueText)
	{
		return;
	}

	const TArray<TSubclassOf<ARTSUnit>> Queue = Building.IsValid() ? Building->GetProductionQueue() : TArray<TSubclassOf<ARTSUnit>>();
	if (Queue.Num() == 0)
	{
		QueueText->SetText(LOCTEXT("Idle", "Idle"));
		return;
	}

	QueueText->SetText(FText::Format(LOCTEXT("Training", "Training {0}  ({1} queued)"),
		GetDefault<ARTSUnit>(Queue[0])->GetUnitDisplayName(), Queue.Num()));
}

void URTSBuildingPanelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Progress changes every frame, so poll it rather than adding a per-frame event.
	if (ProductionProgress)
	{
		ProductionProgress->SetPercent(Building.IsValid() ? Building->GetProductionProgress() : 0.f);
	}
}

#undef LOCTEXT_NAMESPACE
