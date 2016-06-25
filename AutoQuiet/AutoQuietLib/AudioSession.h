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
        static AudioSession^ GetFirstAudioSessionForProcess(System::Diagnostics::Process^ process)
        {
            CComPtr<IAudioSessionControl2> spSession;

            auto hr = S_OK;
            if (FAILED(hr = GetAudioSessionForProcessId(process->Id, &spSession))) {
                auto exceptionMsg = System::String::Format(L"Error getting audio session for PID = {0}: 0x{1:X8}", process->Id, hr);
                throw gcnew System::Exception(exceptionMsg);
            }

            if (spSession == nullptr) {
                auto exceptionMsg = System::String::Format(L"No audio sessions open for PID = {0}: try playing audio first", process->Id);
                throw gcnew System::Exception(exceptionMsg);
            }

            return gcnew AudioSession(spSession);
        }

        static bool TryGetFirstAudioSessionForProcess(System::Diagnostics::Process^ process, [System::Runtime::InteropServices::Out] AudioSession^% audioSession)
        {
            try
            {
                audioSession = GetFirstAudioSessionForProcess(process);
            }
            catch (System::Exception^)
            {
                return false;
            }

            return true;
        }

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

        event AudioSessionStateChangedEventHandler^ StateChanged;

        event AudioSessionDisconnectedEventHandler^ Disconnected;

    internal:
        AudioSession(IAudioSessionControl2 *pSession)
        {
            if (pSession == nullptr)
            {
                throw gcnew System::ArgumentNullException(L"pSession");
            }

            this->pSession = pSession;
            this->pSession->AddRef();

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
        }

    private:
        bool m_disposed = false;
        System::Diagnostics::Process^ m_process;

        IAudioSessionControl2 *pSession = nullptr;
        AudioSessionEventsSinkCallback *pEventsSinkCallback = nullptr;

        void InitializeProcessObject()
        {
            DWORD processId;
            if (FAILED(this->pSession->GetProcessId(&processId)))
            {
                throw gcnew System::Exception(L"Failed to get process ID from underlying IAudioSessionControl2");
            }

            this->m_process = System::Diagnostics::Process::GetProcessById(processId);
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
