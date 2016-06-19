#pragma once

#include "process.h"
#include "audiosession.h"
#include "actions.h"

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
