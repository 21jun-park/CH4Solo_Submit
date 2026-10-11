#include "NBPlayerState.h"
#include "Net/UnrealNetwork.h"

ANBPlayerState::ANBPlayerState()
{
	bReplicates = true;
}

int32 ANBPlayerState::GetCurrentAttempts() const
{
	return CurrentAttempts;
}

int32 ANBPlayerState::GetMaxAttempts() const
{
	return MaxAttempts;
}
void ANBPlayerState::AddAttempt()
{
	if (CurrentAttempts < MaxAttempts)
	{
		++CurrentAttempts;
	}
}

void ANBPlayerState::ResetAttempts()
{
	CurrentAttempts = 0;
}

void ANBPlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANBPlayerState, CurrentAttempts);
}

