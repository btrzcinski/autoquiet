#pragma once

template<typename T>
class ComRefCntBase : public T
{
public:
    ComRefCntBase()
        : m_refCount(1)
    {
    }

    // Inherited via IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject) override
    {
        if (ppvObject == nullptr) return E_POINTER;

        if (riid == __uuidof(IUnknown)) {
            AddRef();
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else if (riid == __uuidof(T)) {
            AddRef();
            *ppvObject = static_cast<T*>(this);
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

protected:
    virtual ~ComRefCntBase()
    {
    }

private:
    LONG m_refCount;
};