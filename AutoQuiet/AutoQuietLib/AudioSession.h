#pragma once

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

        virtual ~AudioSession()
        {
            if (this->pSession != nullptr)
            {
                this->pSession->Release();
                this->pSession = nullptr;
            }
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
        }

    private:

        IAudioSessionControl2 *pSession;
    };
}
