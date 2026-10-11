#include "NBGameMode.h"
#include "NBPlayerState.h"
#include "NBPlayerController.h"

#include "GameFramework/GameStateBase.h"
#include "TimerManager.h"

ANBGameMode::ANBGameMode()
{
	PlayerStateClass = ANBPlayerState::StaticClass();
	PlayerControllerClass = ANBPlayerController::StaticClass();
}

void ANBGameMode::BeginPlay()
{
	Super::BeginPlay();

	GenerateRandomNumbers();

	// Test: Print the answer to the server Output Log
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Number Baseball Answer: %s"),
		*SecretNumber
	);
}

void ANBGameMode::GenerateRandomNumbers()
{
	TArray<int32> Numbers;

	for (int32 Number = 1; Number <= 9; ++Number)
	{
		Numbers.Add(Number);
	}

	SecretNumber.Empty();

	for (int32 Index = 0; Index < 3; ++Index)
	{
		const int32 RandomIndex =
			FMath::RandRange(0, Numbers.Num() - 1);

		SecretNumber.Append(
			FString::FromInt(Numbers[RandomIndex])
		);

		Numbers.RemoveAt(RandomIndex);
	}
}

bool ANBGameMode::ValidateInput(
	const FString& Input,
	FString& ErrorMessage
) const
{
	if (Input.Len() != 3)
	{
		ErrorMessage = TEXT("Please enter a 3-digit number.");
		return false;
	}

	for (const TCHAR Character : Input)
	{
		if (Character < TEXT('1') ||
			Character > TEXT('9'))
		{
			ErrorMessage =
				TEXT("Please enter digits between 1 and 9.");
			return false;
		}
	}

	if (Input[0] == Input[1] ||
		Input[0] == Input[2] ||
		Input[1] == Input[2])
	{
		ErrorMessage =
			TEXT("Please enter three different digits.");
		return false;
	}

	ErrorMessage.Empty();
	return true;
}

bool ANBGameMode::CheckAnswer(
	const FString& Input,
	FString& Result
) const
{
	FString ErrorMessage;

	if (!ValidateInput(Input, ErrorMessage))
	{
		Result = ErrorMessage;
		return false;
	}

	int32 Strike = 0;
	int32 Ball = 0;

	for (int32 Index = 0; Index < 3; ++Index)
	{
		if (Input[Index] == SecretNumber[Index])
		{
			++Strike;
		}
		else
		{
			for (int32 AnswerIndex = 0;
				AnswerIndex < 3;
				++AnswerIndex)
			{
				if (Input[Index] ==
					SecretNumber[AnswerIndex])
				{
					++Ball;
					break;
				}
			}
		}
	}

	if (Strike == 0 && Ball == 0)
	{
		Result = TEXT("OUT");
	}
	else
	{
		Result = FString::Printf(
			TEXT("%dS %dB"),
			Strike,
			Ball
		);
	}

	return true;
}

void ANBGameMode::ProcessGuess(
	ANBPlayerController* GuessingController,
	const FString& Guess
)
{
	if (bGameFinished || !GuessingController)
	{
		return;
	}

	ANBPlayerState* NBState =
		GuessingController->GetPlayerState<ANBPlayerState>();

	if (!NBState)
	{
		return;
	}

	// Prevent players from guessing after using all attempts
	if (NBState->GetCurrentAttempts() >=
		NBState->GetMaxAttempts())
	{
		GuessingController->ClientReceiveResult(
			TEXT("You have used all your attempts.")
		);
		return;
	}

	FString Result;

	// Invalid input does not consume an attempt
	if (!CheckAnswer(Guess, Result))
	{
		GuessingController->ClientReceiveResult(Result);
		return;
	}

	// Only valid input consumes an attempt
	NBState->AddAttempt();

	Result = FString::Printf(
		TEXT("%s (Attempts: %d/%d)"),
		*Result,
		NBState->GetCurrentAttempts(),
		NBState->GetMaxAttempts()
	);

	// Display the result to the player who submitted the guess
	GuessingController->ClientReceiveResult(Result);

	// Three strikes means an immediate win
	if (Guess == SecretNumber)
	{
		const FString PlayerName =
			GuessingController->PlayerState
			? GuessingController->PlayerState->GetPlayerName()
			: TEXT("Player");

		FinishGame(
			FString::Printf(
				TEXT("%s wins! The answer was %s."),
				*PlayerName,
				*SecretNumber
			)
		);

		return;
	}

	// Check whether all players have used all their attempts
	AGameStateBase* CurrentGameState = GameState;

	if (!CurrentGameState)
	{
		return;
	}

	bool bAllPlayersExhausted = true;
	int32 ValidPlayerCount = 0;

	for (APlayerState* PlayerState :
		CurrentGameState->PlayerArray)
	{
		ANBPlayerState* Player =
			Cast<ANBPlayerState>(PlayerState);

		if (!Player)
		{
			continue;
		}

		++ValidPlayerCount;

		if (Player->GetCurrentAttempts() <
			Player->GetMaxAttempts())
		{
			bAllPlayersExhausted = false;
			break;
		}
	}

	if (ValidPlayerCount >= 2 && bAllPlayersExhausted)
	{
		FinishGame(
			FString::Printf(
				TEXT("Draw! The answer was %s."),
				*SecretNumber
			)
		);
	}
}

void ANBGameMode::BroadcastResultToAll(
	const FString& Result
)
{
	AGameStateBase* CurrentGameState = GameState;

	if (!CurrentGameState)
	{
		return;
	}

	for (APlayerState* PlayerState :
		CurrentGameState->PlayerArray)
	{
		APlayerState* ValidPlayerState = PlayerState;

		if (!ValidPlayerState)
		{
			continue;
		}

		ANBPlayerController* Controller =
			Cast<ANBPlayerController>(
				ValidPlayerState->GetOwner()
			);

		if (Controller)
		{
			Controller->ClientReceiveResult(Result);
		}
	}
}

void ANBGameMode::FinishGame(const FString& Result)
{
	if (bGameFinished)
	{
		return;
	}

	bGameFinished = true;

	BroadcastResultToAll(
		Result + TEXT("\nThe game will restart in 3 seconds.")
	);

	GetWorldTimerManager().SetTimer(
		ResetGameTimerHandle,
		this,
		&ANBGameMode::ResetGame,
		3.0f,
		false
	);
}

void ANBGameMode::ResetGame()
{
	AGameStateBase* CurrentGameState = GameState;

	if (CurrentGameState)
	{
		for (APlayerState* PlayerState :
			CurrentGameState->PlayerArray)
		{
			ANBPlayerState* Player =
				Cast<ANBPlayerState>(PlayerState);

			if (Player)
			{
				Player->ResetAttempts();
			}
		}
	}

	GenerateRandomNumbers();
	bGameFinished = false;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Number Baseball Restarted. New Answer: %s"),
		*SecretNumber
	);

	BroadcastResultToAll(
		TEXT("A new game has started.")
	);
}

FString ANBGameMode::GetSecretNumber() const
{
	return SecretNumber;
}