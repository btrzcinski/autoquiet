#pragma once

#include "ComRefCntBase.h"

class AudioSessionEventsSink : public ComRefCntBase<IAudioSessionEvents>
{
public:
    using StateCallback = std::function<void(AudioSessionState)>;
    using DisconnectedCallback = std::function<void(AudioSessionDisconnectReason)>;

private:
    StateCallback stateCallback;
    DisconnectedCallback disconnectedCallback;

    AudioSessionEventsSink(StateCallback const& stateCallback, DisconnectedCallback const& disconnectedCallback)
    {
        this->stateCallback = stateCallback;
        this->disconnectedCallback = disconnectedCallback;
    }

public:
    static HRESULT STDMETHODCALLTYPE Create(IAudioSessionEvents** ppNewSink, StateCallback const& stateCallback,
        DisconnectedCallback const& disconnectedCallback)
    {
        if (ppNewSink == nullptr) return E_POINTER;

        *ppNewSink = static_cast<IAudioSessionEvents*>(new AudioSessionEventsSink(stateCallback, disconnectedCallback));
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
        this->stateCallback(NewState);

        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason) override
    {
        this->disconnectedCallback(DisconnectReason);

        return S_OK;
    }
};
