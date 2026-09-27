#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RTSUnit.generated.h"

class UStaticMeshComponent;
class UTexture2D;

/**
 * A unit is a Character driven by an AIController.
 * Character gives us a capsule + CharacterMovement (walking, gravity, turning);
 * the AIController gives us pathfinding on the NavMesh via MoveToLocation.
 */
UCLASS()
class RTSGAME_API ARTSUnit : public ACharacter
{
	GENERATED_BODY()

public:
	ARTSUnit();

	virtual void Tick(float DeltaSeconds) override;

	void SetSelected(bool bNewSelected) { bSelected = bNewSelected; }
	bool IsSelected() const { return bSelected; }

	/** Ask this unit's AI controller to path to Destination. */
	void CommandMoveTo(const FVector& Destination);

	/** Name shown in the selection panel. Falls back to the class name (BP_Minion -> "Minion") when DisplayName is empty. */
	FText GetUnitDisplayName() const;

	UTexture2D* GetUnitIcon() const { return Icon; }

protected:
	/** A cylinder so the unit is visible with zero assets. Hide it once you give a Blueprint child a real skeletal mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Unit")
	TObjectPtr<UStaticMeshComponent> PlaceholderBody;

	UPROPERTY(EditAnywhere, Category = "RTS|Unit")
	FColor SelectionColor = FColor::Green;

	// A "unit type" is simply a class: make one Blueprint child of RTSUnit per type
	// (BP_Minion, BP_Archer...) and fill these in on its Class Defaults.
	// The selection panel groups the selected units by class.

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RTS|Unit")
	FText DisplayName;

	/** Portrait for the selection panel. Leave empty to show a plain coloured square. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RTS|Unit")
	TObjectPtr<UTexture2D> Icon;

private:
	bool bSelected = false;
};
