#pragma once

HRESULT GetAudioSessionManager(IAudioSessionManager2 **ppSessionManager)
{
    if (ppSessionManager == nullptr) return E_POINTER;

    CComPtr<IMMDeviceEnumerator> spEnumerator;
    IF_FAIL_RET_HR(spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL));

    CComPtr<IMMDevice> spDefaultDevice;
    IF_FAIL_RET_HR(spEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &spDefaultDevice));

    IF_FAIL_RET_HR(spDefaultDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)ppSessionManager));

    return S_OK;
}

HRESULT GetAudioSessionEnumerator(IAudioSessionEnumerator **ppSessionEnumerator)
{
    if (ppSessionEnumerator == nullptr) return E_POINTER;

    CComPtr<IAudioSessionManager2> spSessionManager;
    IF_FAIL_RET_HR(GetAudioSessionManager(&spSessionManager));

    IF_FAIL_RET_HR(spSessionManager->GetSessionEnumerator(ppSessionEnumerator));

    return S_OK;
}

HRESULT ListAudioSessionsOnPrimaryDevice()
{
    CComPtr<IAudioSessionEnumerator> spSessionEnumerator;
    IF_FAIL_RET_HR(GetAudioSessionEnumerator(&spSessionEnumerator));

    auto sessionCount = 0;
    IF_FAIL_RET_HR(spSessionEnumerator->GetCount(&sessionCount));

    for (auto i = 0; i < sessionCount; ++i) {
        CComPtr<IAudioSessionControl> spAudioSessionControl;
        IF_FAIL_RET_HR(spSessionEnumerator->GetSession(i, &spAudioSessionControl));
        CComPtr<IAudioSessionControl2> spAudioSessionControl2;
        IF_FAIL_RET_HR(spAudioSessionControl->QueryInterface(&spAudioSessionControl2));

        CComHeapPtr<wchar_t> displayName, iconPath;
        auto sessionState = AudioSessionStateInactive;

        IF_FAIL_RET_HR(spAudioSessionControl2->GetDisplayName(&displayName));
        IF_FAIL_RET_HR(spAudioSessionControl2->GetState(&sessionState));
        IF_FAIL_RET_HR(spAudioSessionControl2->GetIconPath(&iconPath));
        wprintf(L"Session %d (%d): %ls (%ls)\r\n", i, sessionState, (LPWSTR)displayName, (LPWSTR)iconPath);

        CComHeapPtr<wchar_t> sessionIdentifier, sessionInstanceIdentifier;
        DWORD processId;

        IF_FAIL_RET_HR(spAudioSessionControl2->GetSessionIdentifier(&sessionIdentifier));
        IF_FAIL_RET_HR(spAudioSessionControl2->GetSessionInstanceIdentifier(&sessionInstanceIdentifier));
        IF_FAIL_RET_HR(spAudioSessionControl2->GetProcessId(&processId));
        wprintf(L"\tID: '%ls'\r\n\tInstance ID: '%ls'\r\n\tPID: %d\r\n", (LPWSTR)sessionIdentifier, (LPWSTR)sessionInstanceIdentifier, processId);
    }

    return S_OK;
}

HRESULT GetAudioSessionForProcessId(DWORD processId, IAudioSessionControl2 **ppSessionControl)
{
    if (ppSessionControl == nullptr) return E_POINTER;

    CComPtr<IAudioSessionEnumerator> spSessionEnumerator;
    IF_FAIL_RET_HR(GetAudioSessionEnumerator(&spSessionEnumerator));

    auto sessionCount = 0;
    IF_FAIL_RET_HR(spSessionEnumerator->GetCount(&sessionCount));

    for (auto i = 0; i < sessionCount; ++i) {
        CComPtr<IAudioSessionControl> spAudioSessionControl;
        IF_FAIL_RET_HR(spSessionEnumerator->GetSession(i, &spAudioSessionControl));
        CComPtr<IAudioSessionControl2> spAudioSessionControl2;
        IF_FAIL_RET_HR(spAudioSessionControl->QueryInterface(&spAudioSessionControl2));

        DWORD sessionPid;
        IF_FAIL_RET_HR(spAudioSessionControl2->GetProcessId(&sessionPid));
        if (sessionPid == processId) {
            *ppSessionControl = spAudioSessionControl2.Detach();
            return S_OK;
        }
    }

    *ppSessionControl = nullptr;
    return S_FALSE;
}
