#pragma once

#include "AudioSessionEventsSinkCallback.h"

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

        property int ProcessId
        {
            int get()
            {
                DWORD processId;
                if (FAILED(this->pSession->GetProcessId(&processId)))
                {
                    throw gcnew System::Exception(L"Failed to get process ID from underlying IAudioSessionControl2");
                }

                return processId;
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

    internal:
        AudioSession(IAudioSessionControl2 *pSession)
        {
            if (pSession == nullptr)
            {
                throw gcnew System::ArgumentNullException(L"pSession");
            }

            this->pSession = pSession;
            this->pSession->AddRef();

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

            if (this->pEvents != nullptr)
            {
                this->pEvents->Release();
                this->pEvents = nullptr;
            }

            if (this->pEventsSinkCallback != nullptr)
            {
                delete this->pEventsSinkCallback;
                this->pEventsSinkCallback = nullptr;
            }
        }

    private:
        bool m_disposed = false;
        IAudioSessionControl2 *pSession = nullptr;

        IAudioSessionEvents *pEvents = nullptr;
        AudioSessionEventsSinkCallback *pEventsSinkCallback = nullptr;

        void RegisterForEvents()
        {
            this->pEventsSinkCallback = new AudioSessionEventsSinkCallback(
                gcnew AudioStateChangedDelegate(this, &AudioSession::OnNewSessionState),
                gcnew AudioSessionDisconnectedDelegate(this, &AudioSession::OnDisconnected));
            
            CComPtr<IAudioSessionEvents> spEvents;
            IF_FAIL_THROW(AudioSessionEventsSink::Create(&spEvents, this->pEventsSinkCallback->GetStateCallback(),
                this->pEventsSinkCallback->GetDisconnectedCallback()));
            IF_FAIL_THROW(this->pSession->RegisterAudioSessionNotification(spEvents));

            this->pEvents = spEvents.Detach();
        }

        void UnregisterForEvents()
        {
            if (this->pSession != nullptr &&
                this->pEvents != nullptr)
            {
                this->pSession->UnregisterAudioSessionNotification(this->pEvents);
            }
        }

        void OnNewSessionState(AudioSessionState state)
        {
            System::Console::WriteLine(L"Session state changed to {0}: PID = {1}, {2}", state.ToString(), this->ProcessId, this->SessionIdentifier);
        }

        void OnDisconnected(AudioSessionDisconnectReason reason)
        {
            System::Console::WriteLine(L"Session disconnected ({0}): PID = {1}, {2}", reason.ToString(), this->ProcessId, this->SessionIdentifier);
        }
    };
}
