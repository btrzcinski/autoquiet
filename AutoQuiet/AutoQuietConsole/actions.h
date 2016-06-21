#pragma once

#include "AudioSessionEventsSink.h"

HRESULT MonitorPeakMeterValueAndPerformActionUntilSignaled(IAudioSessionControl2 *pSession, UINT period,
    std::function<void(float)> actionFn, HANDLE cancelHandle)
{
    auto hr = S_OK;

    CComPtr<IAudioMeterInformation> spMeterInformation;
    if (FAILED(hr = pSession->QueryInterface(&spMeterInformation))) {
        fwprintf(stderr, L"Failed to QI for IAudioMeterInformation from session control: %#010x\r\n", hr);
        return hr;
    }

    CHandle timerHandle(CreateWaitableTimerW(nullptr, FALSE, nullptr));
    if (timerHandle == nullptr) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        fwprintf(stderr, L"Failed to create timer object: %#010x\r\n", hr);
        return hr;
    }

    LARGE_INTEGER timerDueTime;
    // interval is in ms (10 ^ -3); timerDueTime is in 100-ns (10 ^ -7)
    timerDueTime.QuadPart = -10000LL * period;

    if (0 == SetWaitableTimer(timerHandle, &timerDueTime, period, nullptr, nullptr, FALSE)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        fwprintf(stderr, L"Failed to setup timer object: %#010x\r\n", hr);
        return hr;
    }

    auto lastPeakValue = 0.0f, peakValue = 0.0f;
    HANDLE waitHandles[2] = { timerHandle, cancelHandle };
    while (1)
    {
        auto waitRet = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);
        if (waitRet != WAIT_OBJECT_0 && waitRet != (WAIT_OBJECT_0 + 1))
        {
            hr = E_FAIL;
            fwprintf(stderr, L"WaitForMultipleObjects failed\r\n");
            break;
        }

        // Check if we were cancelled
        if (waitRet == (WAIT_OBJECT_0 + 1)) break;

        if (FAILED(hr = spMeterInformation->GetPeakValue(&peakValue))) {
            fwprintf(stderr, L"Failed to get peak value from meter information: %#010x\r\n", hr);
            break;
        }

        if (lastPeakValue == peakValue) continue;

        lastPeakValue = peakValue;
        actionFn(peakValue);
    }

    CancelWaitableTimer(timerHandle);

    return hr;
}

HRESULT MonitorPeakMeterValueAndPerformActionUntilEnterIsPressed(IAudioSessionControl2 *pSession, UINT period,
    std::function<void(float)> actionFn)
{
    auto hr = S_OK;

    auto cancelHandle = CHandle(CreateEventW(nullptr, TRUE, FALSE, nullptr));
    if (cancelHandle == nullptr) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        fwprintf(stderr, L"Failed to create cancel handle for MonitorPeakMeterValueAndPerformActionUntilSignaled: %#010x\r\n", hr);
        return hr;
    }

    struct ThreadParams {
        IAudioSessionControl2 *pSession;
        UINT period;
        std::function<void(float)> actionFn;
        HANDLE cancelHandle;
    };

    auto threadProc = [](LPVOID params) -> DWORD
    {
        auto threadParams = static_cast<ThreadParams*>(params);
        return SUCCEEDED(MonitorPeakMeterValueAndPerformActionUntilSignaled(
            threadParams->pSession,
            threadParams->period,
            threadParams->actionFn,
            threadParams->cancelHandle));
    };

    ThreadParams params;
    params.pSession = pSession;
    params.period = 500;
    params.actionFn = actionFn;
    params.cancelHandle = cancelHandle;

    auto threadHandle = CreateThread(nullptr, 0, threadProc, &params, 0, nullptr);
    if (threadHandle == nullptr) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        fwprintf(stderr, L"Failed to create thread for MonitorPeakMeterValueAndPerformActionUntilSignaled: %#010x\r\n", hr);
        return hr;
    }

    wprintf(L"Press Enter to exit.\r\n");
    getchar();

    // Cancel the operation and wait for the thread to exit.
    if (0 == SetEvent(cancelHandle)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        fwprintf(stderr, L"Failed to signal cancel handle for thread: %#010x\r\n", hr);
        return hr;
    }

    if (WaitForSingleObject(threadHandle, INFINITE) != WAIT_OBJECT_0) {
        fwprintf(stderr, L"Failed to wait for thread to terminate\r\n");
        return E_FAIL;
    }

    wprintf(L"\r\n");

    return hr;
}

// Period is expressed in milliseconds
HRESULT PrintPeakMeterValueOnInterval(IAudioSessionControl2 *pSession, UINT period)
{
    auto peakValueChangedActionFn = [](float peakValue) {
        wprintf(L"\rPeak value: %f", peakValue);
    };

    return MonitorPeakMeterValueAndPerformActionUntilEnterIsPressed(pSession, period, peakValueChangedActionFn);
}

HRESULT LowerSessionVolumeWhenPrioritySessionMakesNoise(IAudioSessionControl2 *pProcessSessionControl,
    IAudioSessionControl2 *pPriorityProcessSessionControl)
{
    auto hr = S_OK;

    CComPtr<ISimpleAudioVolume> spSessionVolume;
    if (FAILED(hr = pProcessSessionControl->QueryInterface(&spSessionVolume))) {
        fwprintf(stderr, L"Failed to QI for ISimpleAudioVolume from session control: %#010x\r\n", hr);
        return hr;
    }

    auto dimSessionWhenPrioritySessionMakesNoiseCallbackFn =
        [spSessionVolume](float newPrioritySessionPeakMeterValue)
    {
        static bool isPrioritySessionActive = false;

        if (newPrioritySessionPeakMeterValue > 0.0f) {
            if (isPrioritySessionActive) {
                // We already know it's active
                return;
            }
            
            // Priority session is active, so dim session to 20%
            isPrioritySessionActive = true;

            if (FAILED(spSessionVolume->SetMasterVolume(0.2f, nullptr))) {
                fwprintf(stderr, L"Failed to set session volume to 20%%\r\n");
            }
            else {
                wprintf(L"Set session volume to 20%%\r\n");
            }
        }
        else {
            // Priority session is inactive, so set session back to 100%
            isPrioritySessionActive = false;

            if (FAILED(spSessionVolume->SetMasterVolume(1.0f, nullptr))) {
                fwprintf(stderr, L"Failed to set session volume to 100%%\r\n");
            }
            else {
                wprintf(L"Set session volume to 100%%\r\n");
            }
        }
    };
    
    return MonitorPeakMeterValueAndPerformActionUntilEnterIsPressed(pPriorityProcessSessionControl, 500,
        dimSessionWhenPrioritySessionMakesNoiseCallbackFn);
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
