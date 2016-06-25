#pragma once

#include "AudioSession.h"
#include "AudioSessionNotificationSink.h"
#include "AudioSessionNotificationSinkCallback.h"

#include "AudioSessionNotificationSink.h"

namespace AutoQuietLib
{
    public ref class ProcessAudioWatcher
    {
    public:
        ProcessAudioWatcher(System::String^ processName)
            : m_processName(processName)
        {
            InitializeSessionManager();
            EnumerateExistingSessions();
            StartListeningForNewSessions();
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
            StopListeningForNewSessions();

            if (this->pSessionManager)
            {
                this->pSessionManager->Release();
                this->pSessionManager = nullptr;
            }

            if (this->pSessionNotification)
            {
                this->pSessionNotification->Release();
                this->pSessionNotification = nullptr;
            }

            if (this->pSessionNotificationCallback)
            {
                delete this->pSessionNotificationCallback;
                this->pSessionNotificationCallback = nullptr;
            }
        }

    private:
        bool m_disposed = false;
        System::String^ m_processName;

        System::Collections::Generic::List<AudioSession^>^ sessionList = gcnew System::Collections::Generic::List<AudioSession^>();

        IAudioSessionManager2* pSessionManager = nullptr;
        IAudioSessionNotification* pSessionNotification = nullptr;
        AudioSessionNotificationSinkCallback* pSessionNotificationCallback = nullptr;

        void OnNewAudioSession(AudioSession^ newSession)
        {
            System::Console::WriteLine(L"New session: PID = {0}, {1}", newSession->ProcessId, newSession->SessionIdentifier);
        }

        void InitializeSessionManager()
        {
            CComPtr<IAudioSessionManager2> spSessionManager;
            IF_FAIL_THROW(GetAudioSessionManager(&spSessionManager));

            this->pSessionManager = spSessionManager.Detach();;
        }

        void EnumerateExistingSessions()
        {
            CComPtr<IAudioSessionEnumerator> spEnumerator;
            IF_FAIL_THROW(this->pSessionManager->GetSessionEnumerator(&spEnumerator));

            auto count = 0;
            IF_FAIL_THROW(spEnumerator->GetCount(&count));
            for (auto i = 0; i < count; ++i)
            {
                CComPtr<IAudioSessionControl> spSessionControl;
                IF_FAIL_THROW(spEnumerator->GetSession(i, &spSessionControl));
                CComQIPtr<IAudioSessionControl2> spSessionControl2 = spSessionControl;

                auto session = gcnew AudioSession(spSessionControl2);
                System::Console::WriteLine(L"Existing session: PID = {0}, {1}", session->ProcessId, session->SessionIdentifier);
                this->sessionList->Add(session);
            }
        }

        void StartListeningForNewSessions()
        {
            this->pSessionNotificationCallback = new AudioSessionNotificationSinkCallback(gcnew NewAudioSessionDelegate(this, &ProcessAudioWatcher::OnNewAudioSession));

            CComPtr<IAudioSessionNotification> spNotificationSink;
            IF_FAIL_THROW(AudioSessionNotificationSink::Create(&spNotificationSink, this->pSessionNotificationCallback->GetCallback()));
            IF_FAIL_THROW(this->pSessionManager->RegisterSessionNotification(spNotificationSink));

            this->pSessionNotification = spNotificationSink.Detach();
        }

        void StopListeningForNewSessions()
        {
            if (this->pSessionManager != nullptr &&
                this->pSessionNotification != nullptr)
            {
                this->pSessionManager->UnregisterSessionNotification(this->pSessionNotification);
            }
        }
    };
}
