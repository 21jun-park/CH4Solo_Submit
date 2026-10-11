#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NBGameMode.generated.h"

class ANBPlayerController;

UCLASS()
class CH4SOLO_API ANBGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANBGameMode();

protected:
	virtual void BeginPlay() override;

private:
	// 서버가 보관하는 정답 숫자
	FString SecretNumber;

	// 게임 종료 및 리셋 대기 상태
	bool bGameFinished = false;

	FTimerHandle ResetGameTimerHandle;

	// 게임 시작 시 정답 생성
	void GenerateRandomNumbers();

	// 모든 플레이어에게 게임 결과 전달
	void BroadcastResultToAll(const FString& Result);

	// 승리 또는 무승부 처리
	void FinishGame(const FString& Result);

	// 게임 데이터 초기화 후 재시작
	void ResetGame();

public:
	// 입력값이 규칙에 맞는지 검사
	bool ValidateInput(
		const FString& Input,
		FString& ErrorMessage
	) const;

	// 입력값과 정답을 비교해 결과 반환
	bool CheckAnswer(
		const FString& Input,
		FString& Result
	) const;

	// 서버에서 플레이어의 입력 처리
	void ProcessGuess(
		ANBPlayerController* GuessingController,
		const FString& Guess
	);

	// 현재 정답 반환 (테스트용)
	FString GetSecretNumber() const;
};