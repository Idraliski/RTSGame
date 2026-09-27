#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RTSHUD.generated.h"

class URTSHUDWidget;
class UUserWidget;

/**
 * Draws the drag-selection box and puts the UMG HUD (WBP_HUD) on screen.
 * The HUD has an always-visible frame at the bottom centre that shows one panel at a time.
 * Selection logic itself lives in the player controller.
 */
UCLASS()
class RTSGAME_API ARTSHUD : public AHUD
{
	GENERATED_BODY()

public:
	ARTSHUD();

	virtual void DrawHUD() override;

	/** Swap the bottom-centre panel, e.g. to a building panel when a building is selected. */
	UFUNCTION(BlueprintCallable, Category = "RTS|HUD")
	UUserWidget* ShowBottomPanel(TSubclassOf<UUserWidget> PanelClass);

	/** Go back to the default panel: the selected units. */
	UFUNCTION(BlueprintCallable, Category = "RTS|HUD")
	void ShowSelectionPanel();

	UFUNCTION(BlueprintPure, Category = "RTS|HUD")
	URTSHUDWidget* GetHUDWidget() const { return HUDWidget; }

protected:
	virtual void BeginPlay() override;

	/** The whole on-screen HUD. Defaults to /Game/RTS/UI/WBP_HUD. */
	UPROPERTY(EditDefaultsOnly, Category = "RTS|HUD")
	TSoftClassPtr<URTSHUDWidget> HUDWidgetClass;

	/** Panel shown in the bottom frame by default. Defaults to /Game/RTS/UI/WBP_SelectionPanel. */
	UPROPERTY(EditDefaultsOnly, Category = "RTS|HUD")
	TSoftClassPtr<UUserWidget> SelectionPanelClass;

	UPROPERTY(Transient)
	TObjectPtr<URTSHUDWidget> HUDWidget;

	UPROPERTY(EditAnywhere, Category = "RTS|HUD")
	FLinearColor BoxFillColor = FLinearColor(0.f, 1.f, 0.f, 0.15f);

	UPROPERTY(EditAnywhere, Category = "RTS|HUD")
	FLinearColor BoxBorderColor = FLinearColor(0.f, 1.f, 0.f, 0.9f);
};
