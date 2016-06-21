#pragma once

namespace AutoQuietLib
{
    public ref class AudioSession
    {
    public:
        static AudioSession^ GetFirstAudioSessionForProcess(int processId);

        virtual ~AudioSession();

    private:
        AudioSession(IAudioSessionControl2 *pSession);

        IAudioSessionControl2 *pSession;
    };
}
