#pragma once

#define IF_FAIL_RET_HR(exp) \
    { HRESULT _hr = (exp); if (FAILED(_hr)) return _hr; }

#ifdef __cplusplus_cli
#define IF_FAIL_THROW(exp) \
    { HRESULT _hr = (exp); if (FAILED(_hr)) throw gcnew System::Exception(); }
#endif
