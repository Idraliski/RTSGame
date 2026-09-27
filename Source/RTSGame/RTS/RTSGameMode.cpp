#include "RTSGameMode.h"

#include "RTSCameraPawn.h"
#include "RTSHUD.h"
#include "RTSPlayerController.h"

ARTSGameMode::ARTSGameMode()
{
	DefaultPawnClass = ARTSCameraPawn::StaticClass();
	PlayerControllerClass = ARTSPlayerController::StaticClass();
	HUDClass = ARTSHUD::StaticClass();
}
