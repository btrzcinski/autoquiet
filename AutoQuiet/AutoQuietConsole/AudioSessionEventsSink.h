#pragma once

#include <functional>

class AudioSessionEventsSinkWithStateCallback : public IAudioSessionEvents
{
public:
    using StateCallback = std::function<void(AudioSessionState)>;

private:
    LONG m_refCount;
    StateCallback stateCallback;

    AudioSessionEventsSinkWithStateCallback(StateCallback const& stateCallback)
        : m_refCount(1)
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
        if (ppvObject == nullptr) return E_POINTER;

        if (riid == __uuidof(IUnknown)) {
            AddRef();
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else if (riid == __uuidof(IAudioSessionEvents)) {
            AddRef();
            *ppvObject = static_cast<IAudioSessionEvents*>(this);
        }

        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    virtual ULONG STDMETHODCALLTYPE AddRef(void) override
    {
        return InterlockedIncrement(&m_refCount);
    }
    virtual ULONG STDMETHODCALLTYPE Release(void) override
    {
        auto newRefCount = InterlockedDecrement(&m_refCount);
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
