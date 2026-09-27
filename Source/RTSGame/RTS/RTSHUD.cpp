#include "RTSHUD.h"

#include "RTSPlayerController.h"
#include "RTSSelectionWidgets.h"

DEFINE_LOG_CATEGORY_STATIC(LogRTSHUD, Log, All);

ARTSHUD::ARTSHUD()
{
	SelectionPanelClass = TSoftClassPtr<URTSSelectionPanelWidget>(FSoftObjectPath(TEXT("/Game/RTS/UI/WBP_SelectionPanel.WBP_SelectionPanel_C")));
}

void ARTSHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	if (UClass* PanelClass = SelectionPanelClass.LoadSynchronous())
	{
		SelectionPanel = CreateWidget<URTSSelectionPanelWidget>(PC, PanelClass);
		SelectionPanel->AddToViewport();
	}
	else
	{
		UE_LOG(LogRTSHUD, Warning, TEXT("Selection panel %s not found, so no selection UI."), *SelectionPanelClass.ToString());
	}
}

void ARTSHUD::DrawHUD()
{
	Super::DrawHUD();

	const ARTSPlayerController* PC = Cast<ARTSPlayerController>(GetOwningPlayerController());
	if (!PC || !PC->IsDragSelecting())
	{
		return;
	}

	FVector2D Start, End;
	PC->GetSelectionRect(Start, End);

	const float X0 = static_cast<float>(FMath::Min(Start.X, End.X));
	const float Y0 = static_cast<float>(FMath::Min(Start.Y, End.Y));
	const float X1 = static_cast<float>(FMath::Max(Start.X, End.X));
	const float Y1 = static_cast<float>(FMath::Max(Start.Y, End.Y));

	DrawRect(BoxFillColor, X0, Y0, X1 - X0, Y1 - Y0);
	DrawLine(X0, Y0, X1, Y0, BoxBorderColor, 1.5f);
	DrawLine(X1, Y0, X1, Y1, BoxBorderColor, 1.5f);
	DrawLine(X1, Y1, X0, Y1, BoxBorderColor, 1.5f);
	DrawLine(X0, Y1, X0, Y0, BoxBorderColor, 1.5f);
}
