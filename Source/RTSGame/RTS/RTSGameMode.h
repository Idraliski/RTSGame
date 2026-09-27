#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RTSGameMode.generated.h"

/** Wires the pieces together: which pawn the player gets, which controller drives it, which HUD draws. */
UCLASS()
class RTSGAME_API ARTSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARTSGameMode();
};
