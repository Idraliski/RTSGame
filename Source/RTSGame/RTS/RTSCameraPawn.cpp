#include "RTSCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"

ARTSCameraPawn::ARTSCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// The camera never turns with the controller; we look at the map from a fixed angle.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Root);
	SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f)); // tilt down 60 degrees, facing +X
	SpringArm->TargetArmLength = TargetZoom;
	SpringArm->bDoCollisionTest = false; // don't let trees/buildings pull the camera in
	SpringArm->bEnableCameraLag = true;  // slight smoothing while panning
	SpringArm->CameraLagSpeed = 10.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

void ARTSCameraPawn::AddPanInput(const FVector2D& Direction)
{
	PendingPan += Direction;
}

void ARTSCameraPawn::AddZoomInput(float Steps)
{
	TargetZoom = FMath::Clamp(TargetZoom - Steps * ZoomStep, MinZoom, MaxZoom);
}

void ARTSCameraPawn::AddRotateInput(const FVector2D& MouseDelta)
{
	// Yaw turns the whole pawn, so panning directions turn with it.
	AddActorWorldRotation(FRotator(0.f, MouseDelta.X * RotateSpeed, 0.f));

	// Pitch only tilts the arm, so "forward" for panning stays flat on the ground.
	FRotator ArmRotation = SpringArm->GetRelativeRotation();
	ArmRotation.Pitch = FMath::Clamp(ArmRotation.Pitch + MouseDelta.Y * RotateSpeed, MinPitch, MaxPitch);
	SpringArm->SetRelativeRotation(ArmRotation);
}

void ARTSCameraPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Pan input is relative to the screen, so turn it by the camera's yaw to get a world direction.
	if (!PendingPan.IsNearlyZero())
	{
		const FVector2D Dir = PendingPan.GetSafeNormal();
		const FVector WorldDir = FRotator(0.f, GetActorRotation().Yaw, 0.f).RotateVector(FVector(Dir.X, Dir.Y, 0.f));
		const float Speed = PanSpeed * SpringArm->TargetArmLength;
		AddActorWorldOffset(WorldDir * Speed * DeltaSeconds);
	}
	PendingPan = FVector2D::ZeroVector;

	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetZoom, DeltaSeconds, ZoomInterpSpeed);
}
