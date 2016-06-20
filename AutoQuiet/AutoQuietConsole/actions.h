#pragma once

#include "AudioSessionEventsSink.h"

// Period is expressed in milliseconds
HRESULT PrintPeakMeterValueOnInterval(IAudioSessionControl2 *pSession, UINT period)
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

    wprintf(L"Press Ctrl-C to exit.\r\n");

    while (1)
    {
        if (WaitForSingleObject(timerHandle, INFINITE) != WAIT_OBJECT_0) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            fwprintf(stderr, L"WaitForSingleObject failed: %#010x\r\n", hr);
            break;
        }

        auto peakValue = 0.0f;
        if (FAILED(hr = spMeterInformation->GetPeakValue(&peakValue))) {
            fwprintf(stderr, L"Failed to get peak value from meter information: %#010x\r\n", hr);
            break;
        }

        wprintf(L"\rPeak value: %f", peakValue);
    }
    wprintf(L"\r\n");

    CancelWaitableTimer(timerHandle);

    return hr;
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
