#pragma once

#include "AudioSession.h"
#include "AudioSessionNotificationSink.h"
#include "delegates.h"

namespace AutoQuietLib
{
    class AudioSessionNotificationSinkCallback
    {
    public:
        AudioSessionNotificationSinkCallback(IAudioSessionManager2 *pAudioSessionManager,
            AudioSessionCreatedEventHandler^ delegate) :
            m_spSessionManager(pAudioSessionManager),
            m_delegate(delegate)
        {
        }

        HRESULT Initialize()
        {
            if (m_spSessionNotification != nullptr) return S_FALSE;

            IF_FAIL_RET_HR(AudioSessionNotificationSink::Create(&m_spSessionNotification, this->GetCallback()));
            IF_FAIL_RET_HR(this->m_spSessionManager->RegisterSessionNotification(m_spSessionNotification));

            return S_OK;
        }

        virtual ~AudioSessionNotificationSinkCallback()
        {
            if (m_spSessionNotification != nullptr)
            {
                m_spSessionManager->UnregisterSessionNotification(m_spSessionNotification);
            }
        }

    private:
        gcroot<AudioSessionCreatedEventHandler^> m_delegate;
        CComPtr<IAudioSessionManager2> m_spSessionManager;
        CComPtr<IAudioSessionNotification> m_spSessionNotification;

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
    };
}
