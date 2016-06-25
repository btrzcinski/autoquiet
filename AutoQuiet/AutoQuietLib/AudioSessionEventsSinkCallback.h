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
            AudioSessionState managedEnum = AudioSessionState::Inactive;

            switch (newState)
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
            AudioSessionDisconnectReason managedEnum = AudioSessionDisconnectReason::DeviceRemoval;

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