#pragma once

#include "ComRefCntBase.h"

class AudioSessionNotificationSink : public ComRefCntBase<IAudioSessionNotification>
{
public:
    using SessionCreatedCallback = std::function<void(IAudioSessionControl*)>;

private:
    SessionCreatedCallback sessionCreatedCallback;

    AudioSessionNotificationSink(SessionCreatedCallback const& sessionCreatedCallback)
    {
        this->sessionCreatedCallback = sessionCreatedCallback;
    }

public:
    static HRESULT STDMETHODCALLTYPE Create(IAudioSessionNotification** ppNewSink, SessionCreatedCallback const& sessionCreatedCallback)
    {
        if (ppNewSink == nullptr) return E_POINTER;

        *ppNewSink = static_cast<IAudioSessionNotification*>(new AudioSessionNotificationSink(sessionCreatedCallback));
        return S_OK;
    }

    // Inherited via IAudioSessionNotification
    virtual HRESULT STDMETHODCALLTYPE OnSessionCreated(IAudioSessionControl* NewSession) override
    {
        this->sessionCreatedCallback(NewSession);

        return S_OK;
    }
};
