#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RTSHUD.generated.h"

class URTSSelectionPanelWidget;

/**
 * Draws the drag-selection box and puts the UMG selection panel on screen.
 * Selection logic itself lives in the player controller.
 */
UCLASS()
class RTSGAME_API ARTSHUD : public AHUD
{
	GENERATED_BODY()

public:
	ARTSHUD();

	virtual void DrawHUD() override;

protected:
	virtual void BeginPlay() override;

	/** Widget Blueprint for the selected-units box. Defaults to /Game/RTS/UI/WBP_SelectionPanel. */
	UPROPERTY(EditDefaultsOnly, Category = "RTS|HUD")
	TSoftClassPtr<URTSSelectionPanelWidget> SelectionPanelClass;

	UPROPERTY(Transient)
	TObjectPtr<URTSSelectionPanelWidget> SelectionPanel;

	UPROPERTY(EditAnywhere, Category = "RTS|HUD")
	FLinearColor BoxFillColor = FLinearColor(0.f, 1.f, 0.f, 0.15f);

	UPROPERTY(EditAnywhere, Category = "RTS|HUD")
	FLinearColor BoxBorderColor = FLinearColor(0.f, 1.f, 0.f, 0.9f);
};
