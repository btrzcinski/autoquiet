#pragma once

namespace AutoQuietLib
{
    public ref class AudioSession
    {
    public:
        static AudioSession^ GetFirstAudioSessionForProcess(System::Diagnostics::Process^ process);
        static bool TryGetFirstAudioSessionForProcess(System::Diagnostics::Process^ process, [System::Runtime::InteropServices::Out] AudioSession^% audioSession);

        virtual ~AudioSession();

        property int ProcessId
        {
            int get();
        }

    private:
        AudioSession(IAudioSessionControl2 *pSession);

        IAudioSessionControl2 *pSession;
    };
}
