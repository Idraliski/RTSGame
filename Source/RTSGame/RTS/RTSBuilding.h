#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RTSBuilding.generated.h"

class ARTSUnit;
class UStaticMeshComponent;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRTSProductionQueueChangedSignature);

/**
 * A building that trains units. Units are queued, built one at a time (each unit class's
 * BuildTime), spawned at SpawnPoint, then sent to the rally point.
 *
 * Like units, a "building type" is a Blueprint child (BP_Barracks...): set DisplayName, Icon
 * and ProducibleUnits on its Class Defaults, and swap the placeholder cube for a real mesh.
 * Production is free for now; a resource cost check would go in QueueUnit.
 */
UCLASS()
class RTSGAME_API ARTSBuilding : public AActor
{
	GENERATED_BODY()

public:
	ARTSBuilding();

	virtual void Tick(float DeltaSeconds) override;

	void SetSelected(bool bNewSelected) { bSelected = bNewSelected; }
	bool IsSelected() const { return bSelected; }

	/** Name shown in the building panel. Falls back to the class name (BP_Barracks -> "Barracks"). */
	UFUNCTION(BlueprintPure, Category = "RTS|Building")
	FText GetBuildingDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "RTS|Building")
	UTexture2D* GetBuildingIcon() const { return Icon; }

	/** The unit types this building can train (one button each in the building panel). */
	UFUNCTION(BlueprintPure, Category = "RTS|Building")
	TArray<TSubclassOf<ARTSUnit>> GetProducibleUnits() const { return ProducibleUnits; }

	/** Adds a unit to the end of the queue. Fails if the class isn't producible here. The queue has no size limit. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Building")
	bool QueueUnit(TSubclassOf<ARTSUnit> UnitClass);

	/** Removes a queued unit. Index 0 is the one being built; cancelling it restarts the next one from zero. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Building")
	bool CancelQueuedUnit(int32 Index);

	/** Index 0 is in production, the rest are waiting. */
	UFUNCTION(BlueprintPure, Category = "RTS|Building")
	TArray<TSubclassOf<ARTSUnit>> GetProductionQueue() const { return ProductionQueue; }

	/** 0..1 progress of the unit at the front of the queue (0 when the queue is empty). Poll this for a progress bar. */
	UFUNCTION(BlueprintPure, Category = "RTS|Building")
	float GetProductionProgress() const;

	UFUNCTION(BlueprintCallable, Category = "RTS|Building")
	void SetRallyPoint(const FVector& NewRallyPoint) { RallyPoint = NewRallyPoint; }

	UFUNCTION(BlueprintPure, Category = "RTS|Building")
	FVector GetRallyPoint() const { return RallyPoint; }

	/** Fires when a unit is queued, cancelled or finished. UI listens to this to rebuild the queue icons. */
	UPROPERTY(BlueprintAssignable, Category = "RTS|Building")
	FRTSProductionQueueChangedSignature OnProductionQueueChanged;

protected:
	virtual void BeginPlay() override;

	/** A cube so the building is visible with zero assets. Replace the mesh in a Blueprint child. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Building")
	TObjectPtr<UStaticMeshComponent> BuildingMesh;

	/** Where new units appear. Keep it just outside the mesh, or units spawn stuck inside it. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Building")
	TObjectPtr<USceneComponent> SpawnPoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RTS|Building")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RTS|Building")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Building")
	TArray<TSubclassOf<ARTSUnit>> ProducibleUnits;

	/** Where new units walk to, relative to the building. Right-clicking with the building selected moves it. */
	UPROPERTY(EditAnywhere, Category = "RTS|Building", meta = (MakeEditWidget))
	FVector DefaultRallyOffset = FVector(600.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category = "RTS|Building")
	FColor SelectionColor = FColor::Green;

private:
	void SpawnFrontUnit();

	UPROPERTY(Transient)
	TArray<TSubclassOf<ARTSUnit>> ProductionQueue;

	float ProductionElapsed = 0.f;
	FVector RallyPoint = FVector::ZeroVector;
	bool bSelected = false;
};
