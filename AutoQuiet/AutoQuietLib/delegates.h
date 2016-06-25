#pragma once

namespace AutoQuietLib
{
    ref class AudioSession;

    public enum class AudioSessionState
    {
        Inactive = 0,
        Active = 1,
        Expired = 2
    };

    public enum class AudioSessionDisconnectReason
    {
        DeviceRemoval = 0,
        ServerShutdown = (DeviceRemoval + 1),
        FormatChanged = (ServerShutdown + 1),
        SessionLogoff = (FormatChanged + 1),
        SessionDisconnected = (SessionLogoff + 1),
        ExclusiveModeOverride = (SessionDisconnected + 1)
    };

    ref class AudioSessionEnumExtensions abstract sealed
    {
    public:
        static AudioSessionState ConvertNativeAudioSessionStateToManaged(::AudioSessionState state)
        {
            AudioSessionState managedEnum = AudioSessionState::Inactive;

            switch (state)
            {
            case ::AudioSessionStateInactive:
                managedEnum = AudioSessionState::Inactive;
                break;
            case ::AudioSessionStateActive:
                managedEnum = AudioSessionState::Active;
                break;
            case ::AudioSessionStateExpired:
                managedEnum = AudioSessionState::Expired;
                break;
            default:
                _ASSERT(false);
            }

            return managedEnum;
        }

        static AudioSessionDisconnectReason ConvertNativeAudioSessionDisconnectReasonToManaged(::AudioSessionDisconnectReason reason)
        {
            auto managedEnum = AudioSessionDisconnectReason::DeviceRemoval;

            switch (reason)
            {
            case ::DisconnectReasonDeviceRemoval:
                managedEnum = AudioSessionDisconnectReason::DeviceRemoval;
                break;
            case ::DisconnectReasonServerShutdown:
                managedEnum = AudioSessionDisconnectReason::ServerShutdown;
                break;
            case ::DisconnectReasonFormatChanged:
                managedEnum = AudioSessionDisconnectReason::FormatChanged;
                break;
            case ::DisconnectReasonSessionLogoff:
                managedEnum = AudioSessionDisconnectReason::SessionLogoff;
                break;
            case ::DisconnectReasonSessionDisconnected:
                managedEnum = AudioSessionDisconnectReason::SessionDisconnected;
                break;
            case ::DisconnectReasonExclusiveModeOverride:
                managedEnum = AudioSessionDisconnectReason::ExclusiveModeOverride;
                break;
            default:
                _ASSERT(false);
            }

            return managedEnum;
        }
    };
    
    delegate void AudioSessionCreatedEventHandler(AudioSession^ newSession);

    public delegate void AudioSessionStateChangedEventHandler(AudioSession^ sender, AudioSessionState state);
    public delegate void AudioSessionDisconnectedEventHandler(AudioSession^ sender, AudioSessionDisconnectReason reason);
}
