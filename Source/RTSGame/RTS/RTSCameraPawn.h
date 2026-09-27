#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RTSCameraPawn.generated.h"

class USceneComponent;
class USpringArmComponent;
class UCameraComponent;

/**
 * The "player" in an RTS is just a flying camera.
 * Root (moves across the map, yaw = which way we face) -> SpringArm (tilt + zoom distance) -> Camera.
 * The pawn doesn't read input itself; the player controller feeds it pan/zoom requests.
 */
UCLASS()
class RTSGAME_API ARTSCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ARTSCameraPawn();

	virtual void Tick(float DeltaSeconds) override;

	/** X = forward (up the screen), Y = right. Consumed once per frame. */
	void AddPanInput(const FVector2D& Direction);

	/** +1 zooms in one step, -1 zooms out one step. */
	void AddZoomInput(float Steps);

	/** Mouse movement while rotating: X turns the camera around (yaw), Y tilts it (pitch). */
	void AddRotateInput(const FVector2D& MouseDelta);

protected:
	UPROPERTY(VisibleAnywhere, Category = "RTS|Camera")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "RTS|Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "RTS|Camera")
	TObjectPtr<UCameraComponent> Camera;

	/** Pan speed as a fraction of the current zoom distance per second, so panning feels the same zoomed in or out. */
	UPROPERTY(EditAnywhere, Category = "RTS|Camera")
	float PanSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category = "RTS|Camera")
	float ZoomStep = 250.f;

	UPROPERTY(EditAnywhere, Category = "RTS|Camera")
	float MinZoom = 600.f;

	UPROPERTY(EditAnywhere, Category = "RTS|Camera")
	float MaxZoom = 4000.f;

	UPROPERTY(EditAnywhere, Category = "RTS|Camera")
	float ZoomInterpSpeed = 8.f;

	/** Degrees of rotation per unit of mouse movement. */
	UPROPERTY(EditAnywhere, Category = "RTS|Camera")
	float RotateSpeed = 1.0f;

	/** Tilt limits. -90 looks straight down; values nearer 0 look towards the horizon. */
	UPROPERTY(EditAnywhere, Category = "RTS|Camera")
	float MinPitch = -80.f;

	UPROPERTY(EditAnywhere, Category = "RTS|Camera")
	float MaxPitch = -25.f;

private:
	FVector2D PendingPan = FVector2D::ZeroVector;
	float TargetZoom = 1800.f;
};
