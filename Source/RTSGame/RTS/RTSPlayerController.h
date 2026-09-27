#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RTSPlayerController.generated.h"

class ARTSUnit;
class UInputAction;
class UInputMappingContext;
class UTexture2D;
struct FInputActionValue;

/** All selected units of one type (class), as shown by one entry in the selection panel. */
USTRUCT(BlueprintType)
struct FRTSSelectionGroup
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RTS|Selection")
	TSubclassOf<ARTSUnit> UnitClass;

	UPROPERTY(BlueprintReadOnly, Category = "RTS|Selection")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "RTS|Selection")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "RTS|Selection")
	int32 Count = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRTSSelectionChangedSignature);

/**
 * The brain of the player: reads mouse/keyboard, owns the current selection,
 * and issues orders to units. Also tells the camera pawn where to pan and zoom.
 *
 * Input goes through Enhanced Input: each verb is an InputAction asset, and the
 * mapping context IMC_RTS decides which keys trigger it (so rebinding means editing
 * that asset, not this code). Default bindings, created by Scripts/CreateRTSInput.py:
 *
 * IA_Select          Left mouse     click = select one unit, drag = box select
 * IA_AddToSelection  Shift          hold to add to the selection instead of replacing it
 * IA_Command         Right mouse    move selected units there, in a small grid formation
 * IA_Pan (2D)        Arrow keys     pan (pushing the mouse to the screen edge also pans)
 * IA_Zoom (1D)       Mouse wheel    zoom
 * IA_RotateHold      Middle mouse   hold to rotate the camera
 * IA_Rotate (2D)     Mouse XY       mouse movement, turns/tilts the camera while IA_RotateHold is held
 */
UCLASS()
class RTSGAME_API ARTSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARTSPlayerController();

	/** True while the left button is held and the mouse has moved past DragThreshold. The HUD uses this to draw the box. */
	bool IsDragSelecting() const;

	/** Screen-space corners of the drag box (unordered). */
	void GetSelectionRect(FVector2D& OutStart, FVector2D& OutEnd) const;

	/** The selection split by unit type, in the order each type was first selected. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Selection")
	TArray<FRTSSelectionGroup> GetSelectionGroups() const;

	/** Fires whenever units are added to or removed from the selection. UI listens to this instead of polling. */
	UPROPERTY(BlueprintAssignable, Category = "RTS|Selection")
	FRTSSelectionChangedSignature OnSelectionChanged;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

	// Soft references so the assets are loaded when play starts rather than when the class loads.
	// That way the defaults below work even if the assets were created after the editor opened.
	// Override them in a Blueprint child of this controller if you move or rename the assets.

	UPROPERTY(EditDefaultsOnly, Category = "RTS|Input")
	TSoftObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "RTS|Input")
	TSoftObjectPtr<UInputAction> SelectAction;

	UPROPERTY(EditDefaultsOnly, Category = "RTS|Input")
	TSoftObjectPtr<UInputAction> AddToSelectionAction;

	UPROPERTY(EditDefaultsOnly, Category = "RTS|Input")
	TSoftObjectPtr<UInputAction> CommandAction;

	UPROPERTY(EditDefaultsOnly, Category = "RTS|Input")
	TSoftObjectPtr<UInputAction> PanAction;

	UPROPERTY(EditDefaultsOnly, Category = "RTS|Input")
	TSoftObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(EditDefaultsOnly, Category = "RTS|Input")
	TSoftObjectPtr<UInputAction> RotateHoldAction;

	UPROPERTY(EditDefaultsOnly, Category = "RTS|Input")
	TSoftObjectPtr<UInputAction> RotateAction;

	UPROPERTY(EditAnywhere, Category = "RTS|Camera")
	bool bEdgeScroll = true;

	UPROPERTY(EditAnywhere, Category = "RTS|Camera")
	float EdgeScrollMargin = 15.f;

	/** Pixels the mouse must move before a click becomes a box select. */
	UPROPERTY(EditAnywhere, Category = "RTS|Selection")
	float DragThreshold = 8.f;

	/** Distance between units when a group is sent somewhere. */
	UPROPERTY(EditAnywhere, Category = "RTS|Commands")
	float FormationSpacing = 150.f;

private:
	void OnSelectPressed();
	void OnSelectReleased();
	void OnAddToSelectionPressed();
	void OnAddToSelectionReleased();
	void OnCommandPressed();
	void OnPan(const FInputActionValue& Value);
	void OnZoom(const FInputActionValue& Value);
	void OnRotateHoldPressed();
	void OnRotateHoldReleased();
	void OnRotate(const FInputActionValue& Value);

	void UpdateCameraPan();
	void SelectUnit(ARTSUnit* Unit);
	void SelectUnitsInRect(const FVector2D& CornerA, const FVector2D& CornerB);
	void ClearSelection();

	/** A selected unit was destroyed: drop it and tell the UI. */
	UFUNCTION()
	void OnSelectedUnitDestroyed(AActor* DestroyedActor);

	/** Weak pointers so a destroyed unit simply drops out of the selection. */
	TArray<TWeakObjectPtr<ARTSUnit>> SelectedUnits;

	bool bSelectHeld = false;
	bool bAddToSelectionHeld = false;
	bool bRotateHeld = false;
	/** Where the cursor was when rotation started; we pin it there so it can't drift into edge scrolling. */
	FVector2D RotateAnchor = FVector2D::ZeroVector;
	FVector2D DragStart = FVector2D::ZeroVector;
};
