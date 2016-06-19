#pragma once

#include "AudioSessionEventsSink.h"

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
