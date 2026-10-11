#include "NBPlayerController.h"
#include "NBGameMode.h"
#include "Engine/World.h"

void ANBPlayerController::ServerSubmitGuess_Implementation(
	const FString& Guess
)
{
	ANBGameMode* GameMode =
		GetWorld()->GetAuthGameMode<ANBGameMode>();

	if (!GameMode)
	{
		return;
	}

	GameMode->ProcessGuess(this, Guess);
}

void ANBPlayerController::ClientReceiveResult_Implementation(
	const FString& Result
)
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Controller: %s] 판정 결과: %s"),
		*GetName(),
		*Result
	);

	OnResultReceived.Broadcast(Result);
}