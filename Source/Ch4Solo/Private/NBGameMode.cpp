2. NBGameMode.cpp 전체 교체

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

	// 테스트용: 서버 Output Log에 정답 출력
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
		ErrorMessage = TEXT("3자리 숫자를 입력해주세요.");
		return false;
	}

	for (const TCHAR Character : Input)
	{
		if (Character < TEXT('1') ||
			Character > TEXT('9'))
		{
			ErrorMessage =
				TEXT("1~9 사이의 숫자를 입력해주세요.");
			return false;
		}
	}

	if (Input[0] == Input[1] ||
		Input[0] == Input[2] ||
		Input[1] == Input[2])
	{
		ErrorMessage =
			TEXT("중복되지 않은 숫자를 입력해주세요.");
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

	// 기회를 모두 사용한 플레이어는 추가 입력 불가
	if (NBState->GetCurrentAttempts() >=
		NBState->GetMaxAttempts())
	{
		GuessingController->ClientReceiveResult(
			TEXT("기회를 모두 사용했습니다.")
		);
		return;
	}

	FString Result;

	// 잘못된 입력이면 기회를 소모하지 않음
	if (!CheckAnswer(Guess, Result))
	{
		GuessingController->ClientReceiveResult(Result);
		return;
	}

	// 유효한 입력만 기회 소모
	NBState->AddAttempt();

	Result = FString::Printf(
		TEXT("%s (시도: %d/%d)"),
		*Result,
		NBState->GetCurrentAttempts(),
		NBState->GetMaxAttempts()
	);

	// 먼저 입력한 플레이어에게 판정 결과 표시
	GuessingController->ClientReceiveResult(Result);

	// 3S면 즉시 승리
	if (Guess == SecretNumber)
	{
		const FString PlayerName =
			GuessingController->PlayerState
			? GuessingController->PlayerState->GetPlayerName()
			: TEXT("플레이어");

		FinishGame(
			FString::Printf(
				TEXT("%s 승리! 정답: %s"),
				*PlayerName,
				*SecretNumber
			)
		);

		return;
	}

	// 모든 플레이어가 기회를 소진했는지 확인
	AGameStateBase* CurrentGameState = GetGameState();

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

	if (ValidPlayerCount > 0 && bAllPlayersExhausted)
	{
		FinishGame(
			FString::Printf(
				TEXT("무승부! 정답은 %s 입니다."),
				*SecretNumber
			)
		);
	}
}

void ANBGameMode::BroadcastResultToAll(
	const FString& Result
)
{
	AGameStateBase* CurrentGameState = GetGameState();

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
		Result + TEXT("\n3초 후 게임이 재시작됩니다.")
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
	AGameStateBase* CurrentGameState = GetGameState();

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
		TEXT("새 게임이 시작되었습니다.")
	);
}

FString ANBGameMode::GetSecretNumber() const
{
	return SecretNumber;
}