#pragma once

#include <functional>

class AudioSessionEventsSinkWithStateCallback : public IAudioSessionEvents
{
public:
    using StateCallback = std::function<void(AudioSessionState)>;

private:
    int m_refCount = 1;
    StateCallback stateCallback;

    AudioSessionEventsSinkWithStateCallback(StateCallback const& stateCallback)
    {
        this->stateCallback = stateCallback;
    }

public:
    static HRESULT STDMETHODCALLTYPE Create(IAudioSessionEvents** ppNewSink, StateCallback const& stateCallback)
    {
        if (ppNewSink == nullptr) return E_POINTER;

        *ppNewSink = static_cast<IAudioSessionEvents*>(new AudioSessionEventsSinkWithStateCallback(stateCallback));
        return S_OK;
    }

    // Inherited via IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject) override
    {
        if (*ppvObject == nullptr) return E_POINTER;

        if (riid == __uuidof(IUnknown)) {
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else if (riid == __uuidof(IAudioSessionEvents)) {
            *ppvObject = static_cast<IAudioSessionEvents*>(this);
        }

        return E_NOINTERFACE;
    }
    virtual ULONG STDMETHODCALLTYPE AddRef(void) override
    {
        return ++m_refCount;
    }
    virtual ULONG STDMETHODCALLTYPE Release(void) override
    {
        auto newRefCount = --m_refCount;
        if (newRefCount == 0) {
            delete this;
        }
        return newRefCount;
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
        printf("New volume: %f, new mute: %s\r\n", NewVolume, (NewMute ? "true" : "false"));

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
        printf("New state: %d\r\n", NewState);
        this->stateCallback(NewState);

        return S_OK;
    }
    virtual HRESULT STDMETHODCALLTYPE OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason) override
    {
        return S_OK;
    }
};