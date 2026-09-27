#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RTSPlayerController.h"
#include "RTSSelectionWidgets.generated.h"

class UImage;
class UPanelWidget;
class UTextBlock;

/**
 * One tile in the selection panel: an icon and a count for one unit type.
 * Layout lives in a Widget Blueprint (WBP_SelectionEntry) whose parent is this class;
 * the designer must contain widgets named exactly like the BindWidget properties below.
 */
UCLASS(Abstract)
class RTSGAME_API URTSSelectionEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Fill the tile from a group. Called by the panel whenever the selection changes. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Selection")
	void SetGroup(const FRTSSelectionGroup& InGroup);

protected:
	/** Hook for Blueprint-only extras (tooltips, colour by type...). Runs after the bound widgets are filled. */
	UFUNCTION(BlueprintImplementableEvent, Category = "RTS|Selection")
	void OnGroupSet(const FRTSSelectionGroup& InGroup);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountText;

	/** Optional: shows the unit type's name. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	/** Colour of the square shown when a unit type has no Icon yet. */
	UPROPERTY(EditAnywhere, Category = "RTS|Selection")
	FLinearColor NoIconColor = FLinearColor(0.15f, 0.45f, 0.2f, 1.f);

	UPROPERTY(BlueprintReadOnly, Category = "RTS|Selection")
	FRTSSelectionGroup Group;
};

/**
 * The box of selected units. Listens to ARTSPlayerController::OnSelectionChanged and
 * rebuilds one entry per unit type inside GroupContainer (any panel: HorizontalBox, WrapBox, UniformGridPanel...).
 * Created by ARTSHUD at BeginPlay.
 */
UCLASS(Abstract)
class RTSGAME_API URTSSelectionPanelWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Rebuild the entries from the controller's current selection. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Selection")
	void Refresh();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> GroupContainer;

	/** Widget spawned for each unit type (WBP_SelectionEntry). */
	UPROPERTY(EditAnywhere, Category = "RTS|Selection")
	TSubclassOf<URTSSelectionEntryWidget> EntryClass;

	/** Collapse the whole panel when nothing is selected. */
	UPROPERTY(EditAnywhere, Category = "RTS|Selection")
	bool bHideWhenEmpty = true;

private:
	TWeakObjectPtr<ARTSPlayerController> Controller;
};
