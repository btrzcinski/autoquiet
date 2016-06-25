#pragma once

#include "AudioSessionEventsSinkCallback.h"
#include "delegates.h"

#include "AudioSessionEventsSink.h"
#include "audiosessionenumeration.h"

namespace AutoQuietLib
{
    public ref class AudioSession
    {
    public:
        // Dispose
        ~AudioSession()
        {
            if (m_disposed) return;

            // Dispose of managed resources

            // Call Finalizer
            this->!AudioSession();
            m_disposed = true;
        }

        property System::Diagnostics::Process^ Process
        {
            System::Diagnostics::Process^ get()
            {
                return this->m_process;
            }
        }

        property System::String^ SessionIdentifier
        {
            System::String^ get()
            {
                CComHeapPtr<wchar_t> sessionIdentifier;
                if (FAILED(this->pSession->GetSessionIdentifier(&sessionIdentifier)))
                {
                    throw gcnew System::Exception(L"Failed to get session identifier from underlying IAudioSessionControl2");
                }

                return gcnew System::String(sessionIdentifier);
            }
        }

        property System::String^ SessionInstanceIdentifier
        {
            System::String^ get()
            {
                CComHeapPtr<wchar_t> sessionInstanceIdentifier;
                if (FAILED(this->pSession->GetSessionInstanceIdentifier(&sessionInstanceIdentifier)))
                {
                    throw gcnew System::Exception(L"Failed to get session instance identifier from underlying IAudioSessionControl2");
                }

                return gcnew System::String(sessionInstanceIdentifier);
            }
        }

        property AudioSessionState State
        {
            AudioSessionState get()
            {
                ::AudioSessionState state;
                if (FAILED(this->pSession->GetState(&state)))
                {
                    throw gcnew System::Exception(L"Failed to get state from IAudioSessionControl2");
                }

                return AudioSessionEnumExtensions::ConvertNativeAudioSessionStateToManaged(state);
            }
        }

        property float MasterVolume
        {
            float get()
            {
                float volume;
                if (FAILED(this->pVolume->GetMasterVolume(&volume)))
                {
                    throw gcnew System::Exception(L"Failed to get master volume from ISimpleAudioVolume");
                }

                return volume;
            }
            void set(float value)
            {
                if (FAILED(this->pVolume->SetMasterVolume(value, nullptr)))
                {
                    throw gcnew System::Exception(L"Failed to set master volume on ISimpleAudioVolume");
                }
            }
        }

        property float PeakMeterValue
        {
            float get()
            {
                float peakValue;
                if (FAILED(this->pMeterInformation->GetPeakValue(&peakValue)))
                {
                    throw gcnew System::Exception(L"Failed to get peak meter value from IAudioMeterInformation");
                }

                return peakValue;
            }
        }

        event AudioSessionStateChangedEventHandler^ StateChanged;

        event AudioSessionDisconnectedEventHandler^ Disconnected;

    internal:
        AudioSession(IAudioSessionControl2 *pSession)
        {
            InitializeSessionObjects(pSession);
            InitializeProcessObject();
            RegisterForEvents();
        }

    protected:
        // Finalizer
        !AudioSession()
        {
            UnregisterForEvents();

            if (this->pSession != nullptr)
            {
                this->pSession->Release();
                this->pSession = nullptr;
            }

            if (this->pVolume != nullptr)
            {
                this->pVolume->Release();
                this->pVolume = nullptr;
            }

            if (this->pMeterInformation != nullptr)
            {
                this->pMeterInformation->Release();
                this->pMeterInformation = nullptr;
            }
        }

    private:
        bool m_disposed = false;
        System::Diagnostics::Process^ m_process;

        IAudioSessionControl2* pSession = nullptr;
        ISimpleAudioVolume* pVolume = nullptr;
        IAudioMeterInformation* pMeterInformation = nullptr;
        AudioSessionEventsSinkCallback* pEventsSinkCallback = nullptr;

        void InitializeSessionObjects(IAudioSessionControl2* pSession)
        {
            if (pSession == nullptr)
            {
                throw gcnew System::ArgumentNullException(L"pSession");
            }

            this->pSession = pSession;
            this->pSession->AddRef();

            ISimpleAudioVolume *pVolume;
            IF_FAIL_THROW(this->pSession->QueryInterface(IID_PPV_ARGS(&pVolume)));
            this->pVolume = pVolume;

            IAudioMeterInformation *pMeterInformation;
            IF_FAIL_THROW(this->pSession->QueryInterface(IID_PPV_ARGS(&pMeterInformation)));
            this->pMeterInformation = pMeterInformation;
        }

        void InitializeProcessObject()
        {
            DWORD processId;
            if (FAILED(this->pSession->GetProcessId(&processId)))
            {
                throw gcnew System::Exception(L"Failed to get process ID from underlying IAudioSessionControl2");
            }

            try
            {
                this->m_process = System::Diagnostics::Process::GetProcessById(processId);
            }
            catch (...)
            {
                // Process may not be running anymore
                this->m_process = nullptr;
            }
        }

        void RegisterForEvents()
        {
            this->pEventsSinkCallback = new AudioSessionEventsSinkCallback(
                this->pSession,
                gcnew AudioSessionStateChangedEventHandler(this, &AudioSession::OnNewSessionState),
                gcnew AudioSessionDisconnectedEventHandler(this, &AudioSession::OnDisconnected));
            
            IF_FAIL_THROW(this->pEventsSinkCallback->Initialize());
        }

        void UnregisterForEvents()
        {
            if (this->pEventsSinkCallback != nullptr)
            {
                delete this->pEventsSinkCallback;
                this->pEventsSinkCallback = nullptr;
            }
        }

        void OnNewSessionState(AudioSession^ sender, AudioSessionState state)
        {
            this->StateChanged(this, state);
        }

        void OnDisconnected(AudioSession^ sender, AudioSessionDisconnectReason reason)
        {
            this->Disconnected(this, reason);
        }
    };
}
