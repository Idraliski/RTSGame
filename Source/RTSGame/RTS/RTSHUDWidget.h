#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RTSHUDWidget.generated.h"

class UPanelWidget;

/**
 * The on-screen HUD (WBP_HUD). Owns an always-visible frame at the bottom centre
 * that shows exactly one panel at a time: the selected-units panel by default,
 * later a building panel when a building is selected, and so on.
 *
 * Panels are plain UserWidgets. Swap them with ShowBottomPanel (or ARTSHUD::ShowBottomPanel).
 */
UCLASS(Abstract)
class RTSGAME_API URTSHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Put a panel of this class in the bottom frame, replacing the current one.
	 * Each class is created once and reused, so a panel keeps its state while hidden.
	 * Panels get NativeConstruct/NativeDestruct each time they are shown/hidden.
	 */
	UFUNCTION(BlueprintCallable, Category = "RTS|HUD")
	UUserWidget* ShowBottomPanel(TSubclassOf<UUserWidget> PanelClass);

	UFUNCTION(BlueprintPure, Category = "RTS|HUD")
	UUserWidget* GetBottomPanel() const { return CurrentPanel; }

protected:
	/** The frame's content area. Any panel type works; a Border (one child) is simplest. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> PanelHost;

private:
	UPROPERTY(Transient)
	TMap<TSubclassOf<UUserWidget>, TObjectPtr<UUserWidget>> PanelCache;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> CurrentPanel;
};
