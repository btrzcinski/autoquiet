#include "stdafx.h"

#include "AudioSession.h"
using namespace AutoQuietLib;

#include "audiosessionenumeration.h"

AudioSession^ AudioSession::GetFirstAudioSessionForProcess(int processId)
{
    throw gcnew System::NotImplementedException();
}

AutoQuietLib::AudioSession::~AudioSession()
{
    if (this->pSession != nullptr)
    {
        this->pSession->Release();
        this->pSession = nullptr;
    }
}

AutoQuietLib::AudioSession::AudioSession(IAudioSessionControl2 *pSession)
{
    if (pSession == nullptr)
    {
        throw gcnew System::ArgumentNullException(L"pSession");
    }

    this->pSession = pSession;
    this->pSession->AddRef();
}
