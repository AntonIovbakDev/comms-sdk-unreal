// Copyright 2023 Dolby Laboratories

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "DolbyIOConnectionMode.h"
#include "DolbyIOLogLevel.h"
#include "DolbyIOParticipantInfo.h"
#include "DolbyIOScreenshareSource.h"
#include "DolbyIOSpatialAudioStyle.h"
#include "DolbyIOVideoTrack.h"

#include <memory>

#include "Engine/EngineTypes.h"

#include "DolbyIOSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSubsystemOnTokenNeededDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSubsystemOnInitializedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnConnectedDelegate, const FString&, LocalParticipantID,
                                             const FString&, ConferenceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSubsystemOnDisconnectedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnParticipantAddedDelegate, const EDolbyIOParticipantStatus,
                                             Status, const FDolbyIOParticipantInfo&, ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnParticipantUpdatedDelegate, const EDolbyIOParticipantStatus,
                                             Status, const FDolbyIOParticipantInfo&, ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnVideoTrackAddedDelegate, const FDolbyIOVideoTrack&, VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnVideoTrackRemovedDelegate, const FDolbyIOVideoTrack&,
                                            VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnVideoEnabledDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnVideoDisabledDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnScreenshareStartedDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnScreenshareStoppedDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnActiveSpeakersChangedDelegate, const TArray<FString>&,
                                            ActiveSpeakers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnAudioLevelsChangedDelegate, const TArray<FString>&,
                                             ActiveSpeakers, const TArray<float>&, AudioLevels);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnScreenshareSourcesReceivedDelegate,
                                            const TArray<FDolbyIOScreenshareSource>&, Sources);

namespace dolbyio::comms
{
	enum class conference_status;
	class refresh_token;
	class sdk;
}

namespace DolbyIO
{
	class FVideoFrameHandler;
	class FVideoSink;
}

UCLASS(DisplayName = "Dolby.io Subsystem")
class DOLBYIO_API UDolbyIOSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Initializes or refreshes the client access token. Initializes the plugin unless already initialized.
	 *
	 * Successful initialization triggers the On Initialized event.
	 *
	 * For quick testing, you can manually obtain a token from the Dolby.io dashboard (https://dashboard.dolby.io) and
	 * paste it directly into the node or use the Get Dolby.io Token function.
	 *
	 * @param Token - The client access token.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetToken(const FString& Token);

	/** Connects to a conference.
	 *
	 * Triggers On Connected if successful.
	 *
	 * @param ConferenceName - The conference name. Must not be empty.
	 * @param UserName - The name of the participant.
	 * @param ExternalID - The external unique identifier that the customer's application can add to the participant
	 * while opening a session. If a participant uses the same external ID in conferences, the participant's ID also
	 * remains the same across all sessions.
	 * @param AvatarURL - The URL of the participant's avatar.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void Connect(const FString& ConferenceName = "unreal", const FString& UserName = "", const FString& ExternalID = "",
	             const FString& AvatarURL = "", EDolbyIOConnectionMode ConnectionMode = EDolbyIOConnectionMode::Active,
	             EDolbyIOSpatialAudioStyle SpatialAudioStyle = EDolbyIOSpatialAudioStyle::Shared);

	/** Connects to a demo conference.
	 *
	 * The demo automatically brings in 3 invisible bots into the conference as a quick way to validate the connection
	 * to the service with audio functionality. The bots are placed at point {0, 0, 0}.
	 *
	 * Triggers On Connected if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void DemoConference();

	/** Disconnects from the current conference.
	 *
	 * Triggers On Disconnected when complete.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void Disconnect();

	/** Sets the spatial environment scale.
	 *
	 * The larger the scale, the longer the distance at which the spatial audio
	 * attenuates. To get the best experience, the scale should be set separately for each level. The default value of
	 * "1.0" means that audio will fall completely silent at a distance of 10000 units (10000 cm/100 m).
	 *
	 * @param SpatialEnvironmentScale - The scale as a floating point number.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetSpatialEnvironmentScale(float SpatialEnvironmentScale = 1.0f);

	/** Mutes audio input. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteInput();

	/** Unmutes audio input. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteInput();

	/** Mutes audio output. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteOutput();

	/** Unmutes audio output. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteOutput();

	/** Mutes a given participant for the local user. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteParticipant(const FString& ParticipantID);

	/** Unmutes a given participant for the local user. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteParticipant(const FString& ParticipantID);

	/** Enables video streaming from the primary webcam.
	 *
	 * Triggers On Video Enabled if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void EnableVideo();

	/** Disables video streaming from the primary webcam.
	 *
	 * Triggers On Video Disabled if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void DisableVideo();

	/** Binds a dynamic material instance to hold the frames of the given video track. The plugin will update the
	 * material's texture parameter named "DolbyIO Frame" with the necessary data, therefore the material should have
	 * such a parameter to be usable. Automatically unbinds the material from all other tracks, but it is possible to
	 * bind multiple materials to the same track. Has no effect if the track does not exist at the moment the function
	 * is called, therefore it should usually be called as a response to the "On Video Track Added" event.
	 *
	 * @param Material - The dynamic material instance to bind.
	 * @param VideoTrackID - The ID of the video track.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void BindMaterial(UMaterialInstanceDynamic* Material, const FString& VideoTrackID);

	/** Unbinds a dynamic material instance to no longer hold the video frames of the given video track. The plugin will
	 * no longer update the material's texture parameter named "DolbyIO Frame" with the necessary data.
	 *
	 * @param Material - The dynamic material instance to unbind.
	 * @param VideoTrackID - The ID of the video track.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnbindMaterial(UMaterialInstanceDynamic* Material, const FString& VideoTrackID);

	/** Gets the texture to which video from a given track is being rendered.
	 *
	 * @param VideoTrackID - The ID of the video track.
	 * @return The texture holding the video tracks's frame or NULL if no such texture exists.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	class UTexture2D* GetTexture(const FString& VideoTrackID);

	/** Gets a list of all possible screen sharing sources. These can be entire screens or specific application windows.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetScreenshareSources();

	/** Starts screen sharing using a given source and content type.
	 *
	 * Triggers On Screenshare Started if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void StartScreenshare(const FDolbyIOScreenshareSource& Source,
	                      EDolbyIOScreenshareContentType ContentType = EDolbyIOScreenshareContentType::Unspecified);

	/** Stops screen sharing.
	 *
	 * Triggers On Screenshare Stopped if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void StopScreenshare();

	/** Changes the screenshare content type if already sharing screen. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void ChangeScreenshareContentType(EDolbyIOScreenshareContentType ContentType);

	/** Updates the location of the listener for spatial audio purposes.
	 *
	 * Calling this function even once disables the default behavior, which is to automatically use the location of the
	 * first player controller.
	 *
	 * @param Location - The location of the listener.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLocalPlayerLocation(const FVector& Location);

	/** Updates the rotation of the listener for spatial audio purposes.
	 *
	 * Calling this function even once disables the default behavior, which is to automatically use the rotation of the
	 * first player controller.
	 *
	 * @param Rotation - The rotation of the listener.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLocalPlayerRotation(const FRotator& Rotation);

	/** Sets what and how to log in the Dolby.io C++ SDK.
	 *
	 * This function should be called before the first call to Set Token if the user needs logs about the plugin's
	 * operation. Calling more than once has no effect.
	 *
	 * @param SdkLogLevel - Log level for SDK logs. Default is INFO.
	 * @param MediaLogLevel - Log level for Media Engine logs. It is recommended to keep the Media Engine log level
	 * at OFF, ERROR or WARNING to avoid spam and only enable more detailed logs when necessary. Default is OFF.
	 * @param LogDirectory - Directory to which the logs should be saved. The application must have write access to
	 * the directory or it must be able to create such a directory. Providing a valid directory implies starting logging
	 * to a timestamped file. Providing no value or an empty string has no effect. Default is an empty string.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLogSettings(EDolbyIOLogLevel SdkLogLevel = EDolbyIOLogLevel::INFO,
	                    EDolbyIOLogLevel MediaLogLevel = EDolbyIOLogLevel::OFF, const FString& LogDirectory = "");

	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnTokenNeededDelegate OnTokenNeeded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnInitializedDelegate OnInitialized;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnConnectedDelegate OnConnected;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnDisconnectedDelegate OnDisconnected;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnParticipantAddedDelegate OnParticipantAdded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnParticipantUpdatedDelegate OnParticipantUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnVideoTrackAddedDelegate OnVideoTrackAdded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnVideoTrackRemovedDelegate OnVideoTrackRemoved;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnVideoEnabledDelegate OnVideoEnabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnVideoDisabledDelegate OnVideoDisabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnScreenshareStartedDelegate OnScreenshareStarted;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnScreenshareStoppedDelegate OnScreenshareStopped;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnActiveSpeakersChangedDelegate OnActiveSpeakersChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnAudioLevelsChangedDelegate OnAudioLevelsChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnScreenshareSourcesReceivedDelegate OnScreenshareSourcesReceived;

private:
	void Initialize(FSubsystemCollectionBase&) override;
	void Deinitialize() override;

	bool CanConnect() const;
	bool IsConnected() const;
	bool IsConnectedAsActive() const;
	bool IsSpatialAudio() const;

	void Initialize(const FString& Token);
	void UpdateStatus(dolbyio::comms::conference_status);

	void SetSpatialEnvironment();

	void ToggleInputMute();
	void ToggleOutputMute();

	void SetLocationUsingFirstPlayer();
	void SetLocalPlayerLocationImpl(const FVector& Location);
	void SetRotationUsingFirstPlayer();
	void SetLocalPlayerRotationImpl(const FRotator& Rotation);

	template <class TDelegate, class... TArgs> void BroadcastEvent(TDelegate&, TArgs&&...);

	dolbyio::comms::conference_status ConferenceStatus;
	FString LocalParticipantID;
	FString ConferenceID;
	EDolbyIOConnectionMode ConnectionMode;
	EDolbyIOSpatialAudioStyle SpatialAudioStyle;

	TMap<FString, std::shared_ptr<DolbyIO::FVideoSink>> VideoSinks;
	std::shared_ptr<DolbyIO::FVideoFrameHandler> LocalCameraFrameHandler;
	std::shared_ptr<DolbyIO::FVideoFrameHandler> LocalScreenshareFrameHandler;
	TSharedPtr<dolbyio::comms::sdk> Sdk;
	TSharedPtr<dolbyio::comms::refresh_token> RefreshTokenCb;

	float SpatialEnvironmentScale = 1.0f;

	bool bIsInputMuted = false;
	bool bIsOutputMuted = false;
	bool bIsVideoEnabled = false;

	FTimerHandle LocationTimerHandle;
	FTimerHandle RotationTimerHandle;

	class FErrorHandler final
	{
	public:
		FErrorHandler(UDolbyIOSubsystem& DolbyIOSubsystem, int Line);

		void operator()(std::exception_ptr&& ExcPtr) const;
		void HandleError() const;

	private:
		void HandleError(TFunction<void()> Callee) const;
		void LogException(const FString& Type, const FString& What) const;

		UDolbyIOSubsystem& DolbyIOSubsystem;
		int Line;
	};
};
