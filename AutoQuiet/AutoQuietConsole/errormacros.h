#pragma once

#define IF_FAIL_RET_HR(exp) \
    { HRESULT _hr = (exp); if (FAILED(_hr)) return _hr; }
