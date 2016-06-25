#pragma once

#include "audiosessionenumeration.h"
#include "errormacros.h"

#include "AudioSessionNotificationSink.h"

namespace AutoQuietLib
{
    public ref class ProcessAudioWatcher
    {
    public:
        ProcessAudioWatcher(System::String^ processName)
            : m_disposed(false)
            , m_processName(processName)
        {
            StartListeningForSessions();
        }

        // Dispose
        ~ProcessAudioWatcher()
        {
            if (m_disposed) return;

            // Dispose of managed resources

            // Call Finalizer
            this->!ProcessAudioWatcher();
            m_disposed = true;
        }

    protected:
        // Finalizer
        !ProcessAudioWatcher()
        {
            // Release unmanaged resources
            StopListeningForSessions();
        }

    private:
        bool m_disposed;
        System::String^ m_processName;



        void StartListeningForSessions()
        {
            CComPtr<IAudioSessionManager2> spSessionManager;
            IF_FAIL_THROW(GetAudioSessionManager(&spSessionManager));


        }
        void StopListeningForSessions()
        {

        }
    };
}
