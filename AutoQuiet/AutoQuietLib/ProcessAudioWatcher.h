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
        {
            this->m_processName = processName->ToLowerInvariant();
            this->m_sessionList = gcnew System::Collections::ObjectModel::ObservableCollection<AudioSession^>();
            this->m_readOnlySessionList = gcnew System::Collections::ObjectModel::ReadOnlyObservableCollection<AudioSession^>(this->m_sessionList);

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

        property System::String^ ProcessName
        {
            System::String^ get()
            {
                return this->m_processName;
            }
        }

        property System::Collections::ObjectModel::ReadOnlyObservableCollection<AudioSession^>^ SessionList
        {
            System::Collections::ObjectModel::ReadOnlyObservableCollection<AudioSession^>^ get()
            {
                return this->m_readOnlySessionList;
            }
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
        }

    private:
        bool m_disposed = false;
        System::String^ m_processName;

        System::Collections::ObjectModel::ObservableCollection<AudioSession^>^ m_sessionList;
        System::Collections::ObjectModel::ReadOnlyObservableCollection<AudioSession^>^ m_readOnlySessionList;

        IAudioSessionManager2* pSessionManager = nullptr;
        AudioSessionNotificationSinkCallback* pSessionNotificationCallback = nullptr;

        void OnNewAudioSession(AudioSession^ session)
        {
            if (session->Process != nullptr &&
                session->Process->ProcessName->ToLowerInvariant()->Equals(this->m_processName))
            {
                this->m_sessionList->Add(session);
            }
        }

        void InitializeSessionManager()
        {
            CComPtr<IAudioSessionManager2> spSessionManager;
            IF_FAIL_THROW(GetAudioSessionManager(&spSessionManager));

            this->pSessionManager = spSessionManager.Detach();
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
                if (session->Process != nullptr &&
                    session->Process->ProcessName->ToLowerInvariant()->Equals(this->m_processName))
                {
                    this->m_sessionList->Add(session);
                }
            }
        }

        void StartListeningForNewSessions()
        {
            this->pSessionNotificationCallback = new AudioSessionNotificationSinkCallback(this->pSessionManager,
                gcnew AudioSessionCreatedEventHandler(this, &ProcessAudioWatcher::OnNewAudioSession));

            IF_FAIL_THROW(this->pSessionNotificationCallback->Initialize());
        }

        void StopListeningForNewSessions()
        {
            if (this->pSessionNotificationCallback)
            {
                delete this->pSessionNotificationCallback;
                this->pSessionNotificationCallback = nullptr;
            }
        }
    };
}
