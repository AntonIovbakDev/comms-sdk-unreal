// Copyright 2022 Dolby Laboratories

#include "DolbyIO.h"

#include "DolbyIO/Authenticator.h"
#include "DolbyIO/SdkAccess.h"

#include "Async/Async.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

void UDolbyIO::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CppSdk = MakeShared<DolbyIO::FSdkAccess>(*this);
	Authenticator = MakeShared<DolbyIO::FAuthenticator>(*this);
	GameInstance = GetGameInstance();

	OnTokenNeeded();

	if (GameInstance)
	{
		GameInstance->GetTimerManager().SetTimer(SpatialUpdateTimerHandle, this,
		                                         &UDolbyIO::UpdateViewPointUsingFirstPlayer, 0.03, true);
	}
}

void UDolbyIO::Deinitialize()
{
	CppSdk.Reset();
	Super::Deinitialize();
}

void UDolbyIO::SetToken(const FString& Token)
{
	CppSdk->SetToken(Token);
}
void UDolbyIO::SetTokenUsingKeyAndSecret(const FString& AppKey, const FString& AppSecret,
                                         int TokenExpirationTimeInSeconds)
{
	Authenticator->GetToken(AppKey, AppSecret, TokenExpirationTimeInSeconds);
}
void UDolbyIO::Connect(const FString& ConferenceName, const FString& UserName)
{
	CppSdk->Connect(ConferenceName, UserName);
}
void UDolbyIO::Disconnect()
{
	CppSdk->Disconnect();
}
void UDolbyIO::UpdateViewPoint(const FVector& Position, const FRotator& Rotation)
{
	if (SpatialUpdateTimerHandle.IsValid())
	{
		GameInstance->GetTimerManager().ClearTimer(SpatialUpdateTimerHandle);
	}

	CppSdk->UpdateViewPoint(Position, Rotation);
}
void UDolbyIO::MuteInput()
{
	CppSdk->MuteInput();
}
void UDolbyIO::UnmuteInput()
{
	CppSdk->UnmuteInput();
}
void UDolbyIO::MuteOutput()
{
	CppSdk->MuteOutput();
}
void UDolbyIO::UnmuteOutput()
{
	CppSdk->UnmuteOutput();
}
void UDolbyIO::GetAudioLevels()
{
	CppSdk->GetAudioLevels();
}

void UDolbyIO::UpdateViewPointUsingFirstPlayer()
{
	if (GameInstance)
	{
		if (const auto World = GameInstance->GetWorld())
		{
			if (const auto FirstPlayerController = World->GetFirstPlayerController())
			{
				FVector Position;
				FRotator Rotation;
				FirstPlayerController->GetActorEyesViewPoint(Position, Rotation);
				CppSdk->UpdateViewPoint(Position, Rotation);
			}
		}
	}
}

void UDolbyIO::OnTokenNeededEvent()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnTokenNeeded(); });
}
void UDolbyIO::OnInitializedEvent()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnInitialized(); });
}
void UDolbyIO::OnConnectedEvent()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnConnected(); });
}
void UDolbyIO::OnDisconnectedEvent()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnDisconnected(); });
}
void UDolbyIO::OnLocalParticipantChangedEvent(const DolbyIO::FParticipant& Participant)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnLocalParticipantChanged(Participant); });
}
void UDolbyIO::OnListOfRemoteParticipantsChangedEvent(const DolbyIO::FParticipants& Participants)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnListOfRemoteParticipantsChanged(Participants); });
}
void UDolbyIO::OnListOfActiveSpeakersChangedEvent(const DolbyIO::FParticipants& Speakers)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnListOfActiveSpeakersChanged(Speakers); });
}
void UDolbyIO::OnListOfAudioLevelsChangedEvent(const DolbyIO::FAudioLevels& Levels)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnListOfAudioLevelsChanged(Levels); });
}
