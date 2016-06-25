#pragma once

#include "AudioSession.h"
#include "AudioSessionEventsSink.h"
#include "delegates.h"

namespace AutoQuietLib
{
    class AudioSessionEventsSinkCallback
    {
    public:
        AudioSessionEventsSinkCallback(IAudioSessionControl2* pAudioSessionControl,
            AudioSessionStateChangedEventHandler^ stateDelegate,
            AudioSessionDisconnectedEventHandler^ disconnectedDelegate) :
            m_spAudioSessionControl(pAudioSessionControl),
            m_stateDelegate(stateDelegate),
            m_disconnectedDelegate(disconnectedDelegate)
        {
        }

        HRESULT Initialize()
        {
            if (m_spAudioSessionEvents != nullptr) return S_FALSE;

            IF_FAIL_RET_HR(AudioSessionEventsSink::Create(&m_spAudioSessionEvents, this->GetStateCallback(), this->GetDisconnectedCallback()));
            IF_FAIL_RET_HR(this->m_spAudioSessionControl->RegisterAudioSessionNotification(m_spAudioSessionEvents));

            return S_OK;
        }

        virtual ~AudioSessionEventsSinkCallback()
        {
            if (m_spAudioSessionEvents != nullptr)
            {
                this->m_spAudioSessionControl->UnregisterAudioSessionNotification(m_spAudioSessionEvents);
            }
        }

    private:
        gcroot<AudioSessionStateChangedEventHandler^> m_stateDelegate;
        gcroot<AudioSessionDisconnectedEventHandler^> m_disconnectedDelegate;
        CComPtr<IAudioSessionControl2> m_spAudioSessionControl;
        CComPtr<IAudioSessionEvents> m_spAudioSessionEvents;

        std::function<void(::AudioSessionState)> GetStateCallback()
        {
            return std::bind(&AudioSessionEventsSinkCallback::OnStateChanged, this, std::placeholders::_1);
        }

        void OnStateChanged(::AudioSessionState newState)
        {
            auto managedEnum = AudioSessionEnumExtensions::ConvertNativeAudioSessionStateToManaged(newState);

            try
            {
                this->m_stateDelegate->Invoke(nullptr, managedEnum);
            }
            catch (...)
            {
                // Throwing exceptions back to our native caller isn't good
            }
        }

        std::function<void(::AudioSessionDisconnectReason)> GetDisconnectedCallback()
        {
            return std::bind(&AudioSessionEventsSinkCallback::OnDisconnected, this, std::placeholders::_1);
        }

        void OnDisconnected(::AudioSessionDisconnectReason reason)
        {
            auto managedEnum = AudioSessionEnumExtensions::ConvertNativeAudioSessionDisconnectReasonToManaged(reason);

            try
            {
                this->m_disconnectedDelegate->Invoke(nullptr, managedEnum);
            }
            catch (...)
            {
                // Throwing exceptions back to our native caller isn't good
            }
        }
    };
}