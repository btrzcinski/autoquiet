#include "stdafx.h"

#include "mainroutine.h"

int wmain(int argc, wchar_t *argv[])
{
    if (argc != 3) {
        fwprintf(stderr, L"Usage: %ls [process-to-dim.exe] [priority-process-to-monitor.exe]", argv[0]);
        return 1;
    }

    auto hr = S_OK;

    if (FAILED(hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
        fwprintf(stderr, L"Failed to initialize COM: %#010x\r\n", hr);
        return 2;
    }

    hr = MainRoutine(argv[1], argv[2]);

    CoUninitialize();

    return FAILED(hr) ? 1 : 0;
}
