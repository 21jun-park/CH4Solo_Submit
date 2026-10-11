#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NBPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnNumberBaseballResult, const FString&, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnChatMessageReceived, const FString&, Message);

UCLASS()
class CH4SOLO_API ANBPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    // 클라이언트가 입력한 숫자를 서버에 전달
    UFUNCTION(Server, Reliable, BlueprintCallable)
    void ServerSubmitGuess(const FString& Guess);

    // 서버의 판정 결과를 해당 클라이언트에 전달
    UFUNCTION(Client, Reliable)
    void ClientReceiveResult(const FString& Result);

    UPROPERTY(BlueprintAssignable, Category = "Number Baseball")
    FOnNumberBaseballResult OnResultReceived;

    // 채팅 메시지를 서버에 전달
    UFUNCTION(Server, Reliable, BlueprintCallable)
    void ServerSendChatMessage(const FString& Message);

    // 채팅 메시지를 각 클라이언트에 전달
    UFUNCTION(Client, Reliable)
    void ClientReceiveChatMessage(const FString& Message);

    UPROPERTY(BlueprintAssignable, Category = "Chat")
    FOnChatMessageReceived OnChatMessageReceived;
};