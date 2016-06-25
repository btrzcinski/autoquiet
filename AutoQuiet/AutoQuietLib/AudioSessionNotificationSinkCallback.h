#pragma once

#include "AudioSession.h"

namespace AutoQuietLib
{
    delegate void NewAudioSessionDelegate(AudioSession^ newSession);

    class AudioSessionNotificationSinkCallback
    {
    public:
        AudioSessionNotificationSinkCallback(NewAudioSessionDelegate^ delegate)
            : m_delegate(delegate)
        {
        }

        std::function<void(IAudioSessionControl*)> GetCallback()
        {
            return std::bind(&AudioSessionNotificationSinkCallback::OnNewAudioSession, this, std::placeholders::_1);
        }

        void OnNewAudioSession(IAudioSessionControl* pSessionControl)
        {
            CComQIPtr<IAudioSessionControl2> spSessionControl2 = pSessionControl;
            try
            {
                auto newAudioSession = gcnew AudioSession(spSessionControl2);
                this->m_delegate->Invoke(newAudioSession);
            }
            catch (...)
            {
                // Throwing exceptions back to our native caller isn't good
            }
        }

    private:
        gcroot<NewAudioSessionDelegate^> m_delegate;
    };
}
