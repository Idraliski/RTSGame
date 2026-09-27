#include "RTSBuilding.h"

#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "RTSUnit.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogRTSBuilding, Log, All);

ARTSBuilding::ARTSBuilding()
{
	PrimaryActorTick.bCanEverTick = true;

	// A plain root at ground level, so the actor's pivot sits on the ground and its scale stays 1.
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// The mesh's collision is what the cursor trace hits and what carves the NavMesh.
	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	BuildingMesh->SetupAttachment(Root);
	BuildingMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	BuildingMesh->SetCanEverAffectNavigation(true);

	// The engine cube is 100 units wide with its pivot in the centre, so this is an 8m x 6m x 4m block resting on the ground.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		BuildingMesh->SetStaticMesh(CubeMesh.Object);
	}
	BuildingMesh->SetRelativeScale3D(FVector(8.f, 6.f, 4.f));
	BuildingMesh->SetRelativeLocation(FVector(0.f, 0.f, 200.f));

	// Just past the front face (x = 400), high enough for a unit's capsule to clear the ground.
	SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint"));
	SpawnPoint->SetupAttachment(Root);
	SpawnPoint->SetRelativeLocation(FVector(520.f, 0.f, 100.f));
}

void ARTSBuilding::BeginPlay()
{
	Super::BeginPlay();
	RallyPoint = GetActorTransform().TransformPosition(DefaultRallyOffset);
}

FText ARTSBuilding::GetBuildingDisplayName() const
{
	if (!DisplayName.IsEmpty())
	{
		return DisplayName;
	}

	FString Name = GetClass()->GetName();
	Name.RemoveFromEnd(TEXT("_C"));
	Name.RemoveFromStart(TEXT("BP_"));
	return FText::FromString(Name);
}

// ---------------------------------------------------------------- Queue

bool ARTSBuilding::QueueUnit(TSubclassOf<ARTSUnit> UnitClass)
{
	if (!UnitClass || !ProducibleUnits.Contains(UnitClass))
	{
		return false;
	}

	// Units are free for now. When resources exist, check and spend the cost here.
	ProductionQueue.Add(UnitClass);
	OnProductionQueueChanged.Broadcast();
	return true;
}

bool ARTSBuilding::CancelQueuedUnit(int32 Index)
{
	if (!ProductionQueue.IsValidIndex(Index))
	{
		return false;
	}

	ProductionQueue.RemoveAt(Index);
	if (Index == 0)
	{
		ProductionElapsed = 0.f;
	}
	OnProductionQueueChanged.Broadcast();
	return true;
}

float ARTSBuilding::GetProductionProgress() const
{
	if (ProductionQueue.Num() == 0)
	{
		return 0.f;
	}
	const float BuildTime = GetDefault<ARTSUnit>(ProductionQueue[0])->GetBuildTime();
	return BuildTime > 0.f ? FMath::Clamp(ProductionElapsed / BuildTime, 0.f, 1.f) : 1.f;
}

void ARTSBuilding::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ProductionQueue.Num() > 0)
	{
		ProductionElapsed += DeltaSeconds;
		if (ProductionElapsed >= GetDefault<ARTSUnit>(ProductionQueue[0])->GetBuildTime())
		{
			SpawnFrontUnit();
		}
	}

	if (bSelected)
	{
		// Outline the footprint and draw a flag at the rally point. Debug drawing, like the unit selection ring.
		const FBox Bounds = BuildingMesh->Bounds.GetBox();
		const FVector Centre = Bounds.GetCenter();
		const FVector Extent = Bounds.GetExtent();
		const FVector Ground(Centre.X, Centre.Y, Bounds.Min.Z + 5.f);
		DrawDebugBox(GetWorld(), Ground, FVector(Extent.X + 30.f, Extent.Y + 30.f, 1.f), SelectionColor, false, -1.f, 0, 4.f);

		DrawDebugLine(GetWorld(), SpawnPoint->GetComponentLocation(), RallyPoint, SelectionColor, false, -1.f, 0, 2.f);
		DrawDebugLine(GetWorld(), RallyPoint, RallyPoint + FVector(0.f, 0.f, 200.f), SelectionColor, false, -1.f, 0, 4.f);
		DrawDebugSolidBox(GetWorld(), RallyPoint + FVector(0.f, 30.f, 175.f), FVector(2.f, 30.f, 20.f), SelectionColor);
	}
}

void ARTSBuilding::SpawnFrontUnit()
{
	const TSubclassOf<ARTSUnit> UnitClass = ProductionQueue[0];

	FActorSpawnParameters Params;
	Params.Owner = this;
	// Nudge the unit out of the way if something is standing on the spawn point, but always spawn it.
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	const FRotator Facing = (RallyPoint - SpawnPoint->GetComponentLocation()).GetSafeNormal2D().Rotation();
	ARTSUnit* Unit = GetWorld()->SpawnActor<ARTSUnit>(UnitClass, SpawnPoint->GetComponentLocation(), Facing, Params);
	if (!Unit)
	{
		// Keep it at the front and try again next tick rather than silently eating the unit.
		UE_LOG(LogRTSBuilding, Warning, TEXT("%s couldn't spawn %s"), *GetName(), *GetNameSafe(UnitClass));
		return;
	}

	if (!Unit->GetController())
	{
		Unit->SpawnDefaultController();
	}
	Unit->CommandMoveTo(RallyPoint);

	ProductionQueue.RemoveAt(0);
	ProductionElapsed = 0.f;
	OnProductionQueueChanged.Broadcast();
}
