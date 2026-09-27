#include "RTSPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "RTSCameraPawn.h"
#include "RTSUnit.h"

DEFINE_LOG_CATEGORY_STATIC(LogRTSInput, Log, All);

namespace
{
	// Default asset locations. Scripts/CreateRTSInput.py creates the assets here.
	template <typename T>
	TSoftObjectPtr<T> RTSInputAsset(const TCHAR* Name)
	{
		return TSoftObjectPtr<T>(FSoftObjectPath(FString::Printf(TEXT("/Game/RTS/Input/%s.%s"), Name, Name)));
	}
}

ARTSPlayerController::ARTSPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;

	MappingContext = RTSInputAsset<UInputMappingContext>(TEXT("IMC_RTS"));
	SelectAction = RTSInputAsset<UInputAction>(TEXT("IA_Select"));
	AddToSelectionAction = RTSInputAsset<UInputAction>(TEXT("IA_AddToSelection"));
	CommandAction = RTSInputAsset<UInputAction>(TEXT("IA_Command"));
	PanAction = RTSInputAsset<UInputAction>(TEXT("IA_Pan"));
	ZoomAction = RTSInputAsset<UInputAction>(TEXT("IA_Zoom"));
	RotateHoldAction = RTSInputAsset<UInputAction>(TEXT("IA_RotateHold"));
	RotateAction = RTSInputAsset<UInputAction>(TEXT("IA_Rotate"));
}

void ARTSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Keep the cursor visible and inside the window so edge scrolling works.
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}

void ARTSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 1) Tell this player's Enhanced Input subsystem which key->action mappings are active.
	//    Games stack several contexts (e.g. a build-mode context on top of this one); priority decides who wins.
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (UInputMappingContext* Context = MappingContext.LoadSynchronous())
		{
			Subsystem->AddMappingContext(Context, 0);
		}
		else
		{
			UE_LOG(LogRTSInput, Error, TEXT("Mapping context %s not found. Run Scripts/CreateRTSInput.py."), *MappingContext.ToString());
		}
	}

	// 2) Bind what each action does. The controller only knows about actions, never about keys.
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
	if (!Input)
	{
		UE_LOG(LogRTSInput, Error, TEXT("InputComponent isn't an EnhancedInputComponent. Check Project Settings > Input > Default Input Component Class."));
		return;
	}

	// Started = the frame the input begins, Completed = the frame it ends,
	// Triggered = every frame it's active (what you want for held or analog input).
	if (UInputAction* Action = SelectAction.LoadSynchronous())
	{
		Input->BindAction(Action, ETriggerEvent::Started, this, &ARTSPlayerController::OnSelectPressed);
		Input->BindAction(Action, ETriggerEvent::Completed, this, &ARTSPlayerController::OnSelectReleased);
	}
	if (UInputAction* Action = AddToSelectionAction.LoadSynchronous())
	{
		Input->BindAction(Action, ETriggerEvent::Started, this, &ARTSPlayerController::OnAddToSelectionPressed);
		Input->BindAction(Action, ETriggerEvent::Completed, this, &ARTSPlayerController::OnAddToSelectionReleased);
	}
	if (UInputAction* Action = CommandAction.LoadSynchronous())
	{
		Input->BindAction(Action, ETriggerEvent::Started, this, &ARTSPlayerController::OnCommandPressed);
	}
	if (UInputAction* Action = PanAction.LoadSynchronous())
	{
		Input->BindAction(Action, ETriggerEvent::Triggered, this, &ARTSPlayerController::OnPan);
	}
	if (UInputAction* Action = ZoomAction.LoadSynchronous())
	{
		Input->BindAction(Action, ETriggerEvent::Triggered, this, &ARTSPlayerController::OnZoom);
	}
	if (UInputAction* Action = RotateHoldAction.LoadSynchronous())
	{
		Input->BindAction(Action, ETriggerEvent::Started, this, &ARTSPlayerController::OnRotateHoldPressed);
		Input->BindAction(Action, ETriggerEvent::Completed, this, &ARTSPlayerController::OnRotateHoldReleased);
	}
	if (UInputAction* Action = RotateAction.LoadSynchronous())
	{
		Input->BindAction(Action, ETriggerEvent::Triggered, this, &ARTSPlayerController::OnRotate);
	}
}

void ARTSPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	UpdateCameraPan();
}

// ---------------------------------------------------------------- Camera

void ARTSPlayerController::UpdateCameraPan()
{
	ARTSCameraPawn* CameraPawn = Cast<ARTSCameraPawn>(GetPawn());
	if (!CameraPawn)
	{
		return;
	}

	// Keyboard panning arrives through OnPan; edge scrolling isn't a key press, so it's polled here.
	FVector2D Dir = FVector2D::ZeroVector;
	if (bEdgeScroll && !bSelectHeld && !bRotateHeld)
	{
		float MouseX, MouseY;
		int32 ViewX, ViewY;
		GetViewportSize(ViewX, ViewY);
		if (GetMousePosition(MouseX, MouseY) && ViewX > 0 && ViewY > 0)
		{
			if (MouseX <= EdgeScrollMargin)         { Dir.Y -= 1.f; }
			if (MouseX >= ViewX - EdgeScrollMargin) { Dir.Y += 1.f; }
			if (MouseY <= EdgeScrollMargin)         { Dir.X += 1.f; }
			if (MouseY >= ViewY - EdgeScrollMargin) { Dir.X -= 1.f; }
		}
	}

	CameraPawn->AddPanInput(Dir);
}

void ARTSPlayerController::OnPan(const FInputActionValue& Value)
{
	// IA_Pan is a 2D axis: X = forward, Y = right. The mapping context's
	// Negate/Swizzle modifiers turn WASD into those directions.
	if (ARTSCameraPawn* CameraPawn = Cast<ARTSCameraPawn>(GetPawn()))
	{
		CameraPawn->AddPanInput(Value.Get<FVector2D>());
	}
}

void ARTSPlayerController::OnZoom(const FInputActionValue& Value)
{
	// Mouse wheel gives +1 per notch up, -1 per notch down.
	if (ARTSCameraPawn* CameraPawn = Cast<ARTSCameraPawn>(GetPawn()))
	{
		CameraPawn->AddZoomInput(Value.Get<float>());
	}
}

void ARTSPlayerController::OnRotateHoldPressed()
{
	float MouseX, MouseY;
	if (GetMousePosition(MouseX, MouseY))
	{
		bRotateHeld = true;
		RotateAnchor = FVector2D(MouseX, MouseY);
	}
}

void ARTSPlayerController::OnRotateHoldReleased()
{
	bRotateHeld = false;
}

void ARTSPlayerController::OnRotate(const FInputActionValue& Value)
{
	// IA_Rotate fires whenever the mouse moves; we only act on it while the middle button is held.
	if (!bRotateHeld)
	{
		return;
	}
	if (ARTSCameraPawn* CameraPawn = Cast<ARTSCameraPawn>(GetPawn()))
	{
		CameraPawn->AddRotateInput(Value.Get<FVector2D>());
	}
	// Mouse movement is read as raw deltas, so snapping the cursor back doesn't cancel the rotation.
	SetMouseLocation(FMath::RoundToInt(RotateAnchor.X), FMath::RoundToInt(RotateAnchor.Y));
}

// ---------------------------------------------------------------- Selection

bool ARTSPlayerController::IsDragSelecting() const
{
	if (!bSelectHeld)
	{
		return false;
	}
	float MouseX, MouseY;
	if (!GetMousePosition(MouseX, MouseY))
	{
		return false;
	}
	return FVector2D::Distance(DragStart, FVector2D(MouseX, MouseY)) > DragThreshold;
}

void ARTSPlayerController::GetSelectionRect(FVector2D& OutStart, FVector2D& OutEnd) const
{
	OutStart = DragStart;
	float MouseX, MouseY;
	OutEnd = GetMousePosition(MouseX, MouseY) ? FVector2D(MouseX, MouseY) : DragStart;
}

void ARTSPlayerController::OnAddToSelectionPressed()
{
	bAddToSelectionHeld = true;
}

void ARTSPlayerController::OnAddToSelectionReleased()
{
	bAddToSelectionHeld = false;
}

void ARTSPlayerController::OnSelectPressed()
{
	float MouseX, MouseY;
	if (GetMousePosition(MouseX, MouseY))
	{
		bSelectHeld = true;
		DragStart = FVector2D(MouseX, MouseY);
	}
}

void ARTSPlayerController::OnSelectReleased()
{
	if (!bSelectHeld)
	{
		return;
	}

	const bool bWasDrag = IsDragSelecting(); // must check before clearing bSelectHeld
	FVector2D Start, End;
	GetSelectionRect(Start, End);
	bSelectHeld = false;

	if (!bAddToSelectionHeld)
	{
		ClearSelection();
	}

	if (bWasDrag)
	{
		SelectUnitsInRect(Start, End);
	}
	else
	{
		// Trace on the Pawn channel: unit capsules block it (the default Pawn profile ignores Visibility).
		FHitResult Hit;
		if (GetHitResultUnderCursor(ECC_Pawn, false, Hit))
		{
			SelectUnit(Cast<ARTSUnit>(Hit.GetActor()));
		}
	}

	OnSelectionChanged.Broadcast();
}

void ARTSPlayerController::SelectUnitsInRect(const FVector2D& CornerA, const FVector2D& CornerB)
{
	const FVector2D Min(FMath::Min(CornerA.X, CornerB.X), FMath::Min(CornerA.Y, CornerB.Y));
	const FVector2D Max(FMath::Max(CornerA.X, CornerB.X), FMath::Max(CornerA.Y, CornerB.Y));

	// Project every unit to the screen and keep the ones inside the box.
	for (TActorIterator<ARTSUnit> It(GetWorld()); It; ++It)
	{
		FVector2D ScreenPos;
		if (ProjectWorldLocationToScreen(It->GetActorLocation(), ScreenPos)
			&& ScreenPos.X >= Min.X && ScreenPos.X <= Max.X
			&& ScreenPos.Y >= Min.Y && ScreenPos.Y <= Max.Y)
		{
			SelectUnit(*It);
		}
	}
}

void ARTSPlayerController::SelectUnit(ARTSUnit* Unit)
{
	if (Unit && !Unit->IsSelected())
	{
		Unit->SetSelected(true);
		Unit->OnDestroyed.AddUniqueDynamic(this, &ARTSPlayerController::OnSelectedUnitDestroyed);
		SelectedUnits.Add(Unit);
	}
}

void ARTSPlayerController::ClearSelection()
{
	for (const TWeakObjectPtr<ARTSUnit>& Unit : SelectedUnits)
	{
		if (Unit.IsValid())
		{
			Unit->SetSelected(false);
			Unit->OnDestroyed.RemoveDynamic(this, &ARTSPlayerController::OnSelectedUnitDestroyed);
		}
	}
	SelectedUnits.Reset();
}

void ARTSPlayerController::OnSelectedUnitDestroyed(AActor* DestroyedActor)
{
	SelectedUnits.RemoveAll([DestroyedActor](const TWeakObjectPtr<ARTSUnit>& Unit)
	{
		return !Unit.IsValid() || Unit.Get() == DestroyedActor;
	});
	OnSelectionChanged.Broadcast();
}

TArray<FRTSSelectionGroup> ARTSPlayerController::GetSelectionGroups() const
{
	TArray<FRTSSelectionGroup> Groups;
	for (const TWeakObjectPtr<ARTSUnit>& Unit : SelectedUnits)
	{
		if (!Unit.IsValid())
		{
			continue;
		}

		// One group per class. A linear search is fine: there are only ever a handful of unit types.
		FRTSSelectionGroup* Group = Groups.FindByPredicate([&Unit](const FRTSSelectionGroup& G) { return G.UnitClass == Unit->GetClass(); });
		if (!Group)
		{
			Group = &Groups.AddDefaulted_GetRef();
			Group->UnitClass = Unit->GetClass();
			Group->DisplayName = Unit->GetUnitDisplayName();
			Group->Icon = Unit->GetUnitIcon();
		}
		++Group->Count;
	}
	return Groups;
}

// ---------------------------------------------------------------- Commands

void ARTSPlayerController::OnCommandPressed()
{
	SelectedUnits.RemoveAll([](const TWeakObjectPtr<ARTSUnit>& Unit) { return !Unit.IsValid(); });
	if (SelectedUnits.Num() == 0)
	{
		return;
	}

	// Visibility channel ignores unit capsules, so this lands on the ground under the cursor.
	FHitResult Hit;
	if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		return;
	}

	// Spread the group over a square-ish grid centred on the click so they don't all fight for one spot.
	const int32 Num = SelectedUnits.Num();
	const int32 Cols = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Num)));
	const int32 Rows = FMath::DivideAndRoundUp(Num, Cols);

	for (int32 i = 0; i < Num; ++i)
	{
		const int32 Row = i / Cols;
		const int32 Col = i % Cols;
		const FVector Offset(
			(Row - (Rows - 1) * 0.5f) * FormationSpacing,
			(Col - (Cols - 1) * 0.5f) * FormationSpacing,
			0.f);
		SelectedUnits[i]->CommandMoveTo(Hit.Location + Offset);
	}
}
