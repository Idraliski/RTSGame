#include "RTSUnit.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

ARTSUnit::ARTSUnit()
{
	PrimaryActorTick.bCanEverTick = true;

	// Spawn a plain AIController for every unit, whether placed in the level or spawned at runtime.
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Face the way we walk instead of the way the controller points.
	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 640.f, 0.f);
	Movement->MaxWalkSpeed = 450.f;
	Movement->bUseRVOAvoidance = true; // simple local steering so units don't walk through each other
	Movement->AvoidanceConsiderationRadius = 300.f;

	PlaceholderBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderBody"));
	PlaceholderBody->SetupAttachment(GetCapsuleComponent());
	PlaceholderBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlaceholderBody->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.9f)); // roughly fills the default capsule

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		PlaceholderBody->SetStaticMesh(CylinderMesh.Object);
	}
}

void ARTSUnit::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Debug-draw a ring at the unit's feet while selected. Good enough for now;
	// swap for a decal later. (Debug drawing is stripped from Shipping builds.)
	if (bSelected)
	{
		const UCapsuleComponent* Capsule = GetCapsuleComponent();
		const FVector Feet = GetActorLocation() - FVector(0.f, 0.f, Capsule->GetScaledCapsuleHalfHeight() - 2.f);
		DrawDebugCircle(GetWorld(), Feet, Capsule->GetScaledCapsuleRadius() * 1.6f, 32, SelectionColor,
			false, -1.f, 0, 3.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);
	}
}

void ARTSUnit::CommandMoveTo(const FVector& Destination)
{
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		// AcceptanceRadius, bStopOnOverlap, bUsePathfinding, bProjectDestinationToNavigation
		AI->MoveToLocation(Destination, 30.f, true, true, true);
	}
}

FText ARTSUnit::GetUnitDisplayName() const
{
	if (!DisplayName.IsEmpty())
	{
		return DisplayName;
	}

	// Blueprint classes are named like "BP_Minion_C"; trim that down to "Minion".
	FString Name = GetClass()->GetName();
	Name.RemoveFromEnd(TEXT("_C"));
	Name.RemoveFromStart(TEXT("BP_"));
	return FText::FromString(Name);
}
