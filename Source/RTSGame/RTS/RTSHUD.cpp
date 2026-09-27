#include "RTSHUD.h"

#include "RTSHUDWidget.h"
#include "RTSPlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogRTSHUD, Log, All);

ARTSHUD::ARTSHUD()
{
	HUDWidgetClass = TSoftClassPtr<URTSHUDWidget>(FSoftObjectPath(TEXT("/Game/RTS/UI/WBP_HUD.WBP_HUD_C")));
	SelectionPanelClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(TEXT("/Game/RTS/UI/WBP_SelectionPanel.WBP_SelectionPanel_C")));
}

void ARTSHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	UClass* WidgetClass = HUDWidgetClass.LoadSynchronous();
	if (!WidgetClass)
	{
		UE_LOG(LogRTSHUD, Warning, TEXT("HUD widget %s not found, so no on-screen HUD."), *HUDWidgetClass.ToString());
		return;
	}

	HUDWidget = CreateWidget<URTSHUDWidget>(PC, WidgetClass);
	HUDWidget->AddToViewport();
	ShowSelectionPanel();
}

UUserWidget* ARTSHUD::ShowBottomPanel(TSubclassOf<UUserWidget> PanelClass)
{
	return HUDWidget ? HUDWidget->ShowBottomPanel(PanelClass) : nullptr;
}

void ARTSHUD::ShowSelectionPanel()
{
	if (UClass* PanelClass = SelectionPanelClass.LoadSynchronous())
	{
		ShowBottomPanel(PanelClass);
	}
	else
	{
		UE_LOG(LogRTSHUD, Warning, TEXT("Selection panel %s not found."), *SelectionPanelClass.ToString());
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
