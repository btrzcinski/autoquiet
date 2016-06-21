#include "stdafx.h"

#include "AudioSession.h"
using namespace AutoQuietLib;

#include "audiosessionenumeration.h"

AudioSession^ AudioSession::GetFirstAudioSessionForProcess(int processId)
{
    CComPtr<IAudioSessionControl2> spSession;
    
    auto hr = S_OK;
    if (FAILED(hr = GetAudioSessionForProcessId(processId, &spSession))) {
        auto exceptionMsg = System::String::Format(L"Error getting audio session for PID = {0}: 0x{1:X8}", processId, hr);
        throw gcnew System::Exception(exceptionMsg);
    }

    if (spSession == nullptr) {
        auto exceptionMsg = System::String::Format(L"No audio sessions open for PID = {0}: try playing audio first", processId);
        throw gcnew System::Exception(exceptionMsg);
    }

    return gcnew AudioSession(spSession);
}

bool AudioSession::TryGetFirstAudioSessionForProcess(int processId, AudioSession ^% audioSession)
{
    try
    {
        audioSession = GetFirstAudioSessionForProcess(processId);
    }
    catch (System::Exception^)
    {
        return false;
    }

    return true;
}

AudioSession::~AudioSession()
{
    if (this->pSession != nullptr)
    {
        this->pSession->Release();
        this->pSession = nullptr;
    }
}

int AudioSession::ProcessId::get()
{
    DWORD processId;
    if (FAILED(this->pSession->GetProcessId(&processId)))
    {
        throw gcnew System::Exception(L"Failed to get process ID from underlying IAudioSessionControl2");
    }

    return processId;
}

AudioSession::AudioSession(IAudioSessionControl2 *pSession)
{
    if (pSession == nullptr)
    {
        throw gcnew System::ArgumentNullException(L"pSession");
    }

    this->pSession = pSession;
    this->pSession->AddRef();
}
