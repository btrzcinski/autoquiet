#pragma once

#include "ComRefCntBase.h"

class AudioSessionEventsSink : public ComRefCntBase<IAudioSessionEvents>
{
public:
    using StateCallback = std::function<void(AudioSessionState)>;

private:
    StateCallback stateCallback;

    AudioSessionEventsSink(StateCallback const& stateCallback)
    {
        this->stateCallback = stateCallback;
    }

public:
    static HRESULT STDMETHODCALLTYPE Create(IAudioSessionEvents** ppNewSink, StateCallback const& stateCallback)
    {
        if (ppNewSink == nullptr) return E_POINTER;

        *ppNewSink = static_cast<IAudioSessionEvents*>(new AudioSessionEventsSink(stateCallback));
        return S_OK;
    }

    // Inherited via IAudioSessionEvents
    virtual HRESULT STDMETHODCALLTYPE OnDisplayNameChanged(LPCWSTR NewDisplayName, LPCGUID EventContext) override
    {
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE OnIconPathChanged(LPCWSTR NewIconPath, LPCGUID EventContext) override
    {
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE OnSimpleVolumeChanged(float NewVolume, BOOL NewMute, LPCGUID EventContext) override
    {
        wprintf(L"New volume: %f, new mute: %s\r\n", NewVolume, (NewMute ? L"true" : L"false"));

        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE OnChannelVolumeChanged(DWORD ChannelCount, float NewChannelVolumeArray[], DWORD ChangedChannel, LPCGUID EventContext) override
    {
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE OnGroupingParamChanged(LPCGUID NewGroupingParam, LPCGUID EventContext) override
    {
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE OnStateChanged(AudioSessionState NewState) override
    {
        wchar_t *newStateStr = nullptr;
        switch (NewState) {
        case AudioSessionStateActive:
            newStateStr = L"active";
            break;
        case AudioSessionStateExpired:
            newStateStr = L"expired";
            break;
        case AudioSessionStateInactive:
            newStateStr = L"inactive";
            break;
        default:
            newStateStr = L"unknown";
        }

        wprintf(L"New state: %d (%ls)\r\n", NewState, newStateStr);
        this->stateCallback(NewState);

        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason) override
    {
        return S_OK;
    }
};
