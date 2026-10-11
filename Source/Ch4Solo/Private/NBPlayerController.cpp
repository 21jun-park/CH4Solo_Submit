#include "NBPlayerController.h"
#include "NBGameMode.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"

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

void ANBPlayerController::ServerSendChatMessage_Implementation(
    const FString& Message
)
{
    if (Message.TrimStartAndEnd().IsEmpty())
    {
        return;
    }

    // 메시지 작성자의 이름을 붙임
    const FString PlayerName =
        PlayerState
        ? PlayerState->GetPlayerName()
        : TEXT("Player");

    const FString ChatMessage = FString::Printf(
        TEXT("%s: %s"),
        *PlayerName,
        *Message
    );

    // 서버에 연결된 모든 플레이어 컨트롤러에 전달
    for (FConstPlayerControllerIterator It =
        GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ANBPlayerController* Controller =
            Cast<ANBPlayerController>(It->Get());

        if (Controller)
        {
            Controller->ClientReceiveChatMessage(ChatMessage);
        }
    }
}

void ANBPlayerController::ClientReceiveChatMessage_Implementation(
    const FString& Message
)
{
    OnChatMessageReceived.Broadcast(Message);
}