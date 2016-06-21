#pragma once

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
