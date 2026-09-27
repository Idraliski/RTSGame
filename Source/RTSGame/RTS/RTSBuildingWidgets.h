#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RTSBuildingWidgets.generated.h"

class ARTSBuilding;
class ARTSPlayerController;
class ARTSUnit;
class UButton;
class UImage;
class UPanelWidget;
class UProgressBar;
class UTextBlock;

/**
 * One "train this unit" button in the building panel: the unit's icon, with its hotkey written on top.
 * Layout lives in WBP_ProductionButton; its designer must contain widgets named like the BindWidget properties.
 */
UCLASS(Abstract)
class RTSGAME_API URTSProductionButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Called by the panel whenever the selected building changes. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Building")
	void SetUnit(ARTSBuilding* InBuilding, TSubclassOf<ARTSUnit> InUnitClass, const FText& InHotkey);

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void OnProduceClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ProduceButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	/** The key that also trains this unit, e.g. "A". */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HotkeyText;

	/** Optional: the unit's name under the icon. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	/** Colour of the square shown when a unit type has no Icon yet. */
	UPROPERTY(EditAnywhere, Category = "RTS|Building")
	FLinearColor NoIconColor = FLinearColor(0.15f, 0.3f, 0.55f, 1.f);

	UPROPERTY(BlueprintReadOnly, Category = "RTS|Building")
	TWeakObjectPtr<ARTSBuilding> Building;

	UPROPERTY(BlueprintReadOnly, Category = "RTS|Building")
	TSubclassOf<ARTSUnit> UnitClass;
};

/**
 * Bottom-centre panel shown instead of the units panel while a building is selected
 * (the player controller swaps it in through ARTSHUD::ShowBottomPanel).
 * Shows the building's name, one production button per unit type, and the queue.
 */
UCLASS(Abstract)
class RTSGAME_API URTSBuildingPanelWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Rebuild everything from the controller's selected building. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Building")
	void Refresh();

	/** Only the queue line changes when a unit is queued or finished. */
	UFUNCTION()
	void RefreshQueue();

	/** Holds the production buttons (HorizontalBox, WrapBox, UniformGridPanel...). */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> ButtonContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BuildingNameText;

	/** e.g. "Training MinionMelee  (3 queued)". */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QueueText;

	/** Progress of the unit currently in training. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ProductionProgress;

	/** Widget spawned per producible unit. Falls back to /Game/RTS/UI/WBP_ProductionButton when empty. */
	UPROPERTY(EditAnywhere, Category = "RTS|Building")
	TSubclassOf<URTSProductionButtonWidget> ButtonClass;

private:
	void BindToBuilding(ARTSBuilding* NewBuilding);

	TWeakObjectPtr<ARTSPlayerController> Controller;
	TWeakObjectPtr<ARTSBuilding> Building;
};
