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
    
    delegate void AudioSessionCreatedEventHandler(AudioSession^ newSession);

    public delegate void AudioSessionStateChangedEventHandler(AudioSession^ sender, AudioSessionState state);
    public delegate void AudioSessionDisconnectedEventHandler(AudioSession^ sender, AudioSessionDisconnectReason reason);
}
