#include "stdafx.h"

#include "AudioSessionEventsSink.h"

HRESULT GetAudioSessionEnumerator(IAudioSessionEnumerator **ppSessionEnumerator)
{
    if (ppSessionEnumerator == nullptr) return E_POINTER;

    CComPtr<IMMDeviceEnumerator> spEnumerator;
    IF_FAIL_RET_HR(spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL));

    CComPtr<IMMDevice> spDefaultDevice;
    IF_FAIL_RET_HR(spEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &spDefaultDevice));

    CComPtr<IAudioSessionManager2> spSessionManager;
    IF_FAIL_RET_HR(spDefaultDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&spSessionManager));

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

HRESULT GetProcessIdForProcessName(const wchar_t *processName, DWORD *pid)
{
    if (processName == nullptr || pid == nullptr) return E_POINTER;

    DWORD bigPidArray[1024];
    DWORD bytesReturned;
    wchar_t imageFilePath[MAX_PATH + 1];

    if (0 == EnumProcesses(bigPidArray, sizeof(bigPidArray) * ARRAYSIZE(bigPidArray), &bytesReturned))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    auto processCount = bytesReturned / sizeof(DWORD);
    for (auto i = 0U; i < processCount; ++i) {
        CHandle procHandle(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, bigPidArray[i]));
        if (procHandle == NULL) {
            // This isn't really a condition worth worrying about; some processes we aren't allowed to open
            continue;
        }

        DWORD imageFilePathSize = ARRAYSIZE(imageFilePath);
        if (0 == QueryFullProcessImageName(procHandle, 0, imageFilePath, &imageFilePathSize)) {
            wprintf(L"Couldn't get image name for PID = %d\r\n", bigPidArray[i]);
            continue;
        }

        auto imageFileName = PathFindFileNameW(imageFilePath);
        if (0 == _wcsicmp(processName, imageFileName)) {
            *pid = bigPidArray[i];
            return S_OK;
        }
    }

    *pid = 0;
    return S_FALSE;
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

HRESULT LowerSessionVolumeWhenPrioritySessionBecomesActive(IAudioSessionControl2 *pSession,
    IAudioSessionControl2* pPrioritySession)
{
    auto hr = S_OK;

    // NOTE: This part (QI for ISimpleAudioVolume) is not supported by Microsoft and subject to breakage
    // in a future release of Windows. However, it does seem to work at least as of Windows 10.0.14361.
    CComPtr<ISimpleAudioVolume> spSessionVolume;
    if (FAILED(hr = pSession->QueryInterface(&spSessionVolume))) {
        fwprintf(stderr, L"Failed to QI for ISimpleAudioVolume from session control: %#010x\r\n", hr);
        return hr;
    }

    auto dimSessionWhenPrioritySessionIsActiveCallbackFn =
        [spSessionVolume](AudioSessionState newState)
    {
        if (newState == AudioSessionStateActive) {
            // Priority session is active, so dim session to 20%
            if (FAILED(spSessionVolume->SetMasterVolume(0.2f, nullptr))) {
                fwprintf(stderr, L"Failed to set session volume to 20%%\r\n");
            }
            else {
                wprintf(L"Set session volume to 20%%\r\n");
            }
        }
        else if (newState == AudioSessionStateInactive) {
            // Priority session is inactive, so set session back to 100%
            if (FAILED(spSessionVolume->SetMasterVolume(1.0f, nullptr))) {
                fwprintf(stderr, L"Failed to set session volume to 100%%\r\n");
            }
            else {
                wprintf(L"Set session volume to 100%%\r\n");
            }
        }
    };

    CComPtr<IAudioSessionEvents> spPrioritySessionEventSink;
    if (FAILED(hr = AudioSessionEventsSinkWithStateCallback::Create(&spPrioritySessionEventSink,
        dimSessionWhenPrioritySessionIsActiveCallbackFn))) {
        fwprintf(stderr, L"Failed to create new audio session events sink for priority session: %#010x\r\n", hr);
        return hr;
    }

    if (FAILED(hr = pPrioritySession->RegisterAudioSessionNotification(spPrioritySessionEventSink))) {
        fwprintf(stderr, L"Failed to register for audio session notifications for priority session: %#010x\r\n", hr);
        return hr;
    }

    wprintf(L"Press Enter to exit.\r\n");
    getchar();
    if (FAILED(hr = pPrioritySession->UnregisterAudioSessionNotification(spPrioritySessionEventSink))) {
        fwprintf(stderr, L"Failed to unregister for audio session notifications for priority session: %#010x\r\n", hr);
        return hr;
    }

    return hr;
}

// Assumes COM has been initialized.
HRESULT MainRoutine(const wchar_t *processName, const wchar_t *priorityProcessName)
{
    auto hr = S_OK;

    DWORD processPid, priorityProcessPid;
    if (FAILED(hr = GetProcessIdForProcessName(processName, &processPid))) {
        fwprintf(stderr, L"Failed to get %ls PID: %#010x\r\n", processName, hr);
        return hr;
    }

    if (processPid == 0) {
        fwprintf(stderr, L"%ls isn't running; start it first.\r\n", processName);
        return hr;
    }

    printf("%ls PID: %d\r\n", processName, processPid);

    if (FAILED(hr = GetProcessIdForProcessName(priorityProcessName, &priorityProcessPid))) {
        fwprintf(stderr, L"Failed to get %ls PID: %#010x\r\n", priorityProcessName, hr);
        return hr;
    }

    if (priorityProcessPid == 0) {
        fwprintf(stderr, L"%ls isn't running; start it first.\r\n", priorityProcessName);
        return hr;
    }

    wprintf(L"%ls PID: %d\r\n", priorityProcessName, priorityProcessPid);

    CComPtr<IAudioSessionControl2> spPriorityProcessSessionControl;
    if (FAILED(hr = GetAudioSessionForProcessId(priorityProcessPid, &spPriorityProcessSessionControl))) {
        fwprintf(stderr, L"Failed to get %ls (PID %d) audio session control: %#010x\r\n", priorityProcessName, priorityProcessPid, hr);
        return hr;
    }
    
    if (hr == S_FALSE) {
        fwprintf(stderr, L"There isn't any audio session associated with %ls. Try playing audio first.\r\n", priorityProcessName);
        return hr;
    }

    CComPtr<IAudioSessionControl2> spProcessSessionControl;
    if (FAILED(hr = GetAudioSessionForProcessId(processPid, &spProcessSessionControl))) {
        fwprintf(stderr, L"Failed to get %ls (PID %d) audio session control: %#010x\r\n", processName, processPid, hr);
        return hr;
    }
    
    if (hr == S_FALSE) {
        fwprintf(stderr, L"There isn't any audio session associated with %ls. Try playing audio first.\r\n", processName);
        return hr;
    }

    hr = LowerSessionVolumeWhenPrioritySessionBecomesActive(spProcessSessionControl, spPriorityProcessSessionControl);

    return hr;
}

int wmain(int argc, wchar_t *argv[])
{
    if (argc != 3) {
        fwprintf(stderr, L"Usage: %ls [process-to-dim.exe] [priority-process-to-monitor.exe]", argv[0]);
        return 1;
    }

    auto hr = S_OK;

    if (FAILED(hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
        fwprintf(stderr, L"Failed to initialize COM: %#010x\r\n", hr);
        return 2;
    }

    hr = MainRoutine(argv[1], argv[2]);

    CoUninitialize();

    return FAILED(hr) ? 1 : 0;
}
