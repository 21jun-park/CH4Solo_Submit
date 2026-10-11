#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NBPlayerState.generated.h"

UCLASS()
class CH4SOLO_API ANBPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ANBPlayerState();

	// 현재까지 사용한 시도 횟수
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Number Baseball")
	int32 CurrentAttempts = 0;

	// 최대 시도 횟수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Number Baseball")
	int32 MaxAttempts = 3;

	// 현재 시도 횟수 반환
	UFUNCTION(BlueprintPure, Category = "Number Baseball")
	int32 GetCurrentAttempts() const;

	// 최대 시도 횟수 반환
	UFUNCTION(BlueprintPure, Category = "Number Baseball")
	int32 GetMaxAttempts() const;

	// 시도 횟수 증가
	void AddAttempt();

	// 시도 횟수 초기화
	void ResetAttempts();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;
};
