#include "stdafx.h"

#include "mainroutine.h"

int wmain(int argc, wchar_t *argv[])
{
    if (argc < 3) {
        fwprintf(stderr, L"Usage: %ls process-to-dim.exe priority-process-to-monitor.exe [level-to-lower-volume-to]\r\n", argv[0]);
        fwprintf(stderr, L"The level to lower the volume to when the priority process makes noise defaults to 10%% if not specified.\r\n");
        fwprintf(stderr, L"Example: %ls chrome.exe firefox.exe 10\r\n", argv[0]);
        return 1;
    }

    auto hr = S_OK;

    if (FAILED(hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
        fwprintf(stderr, L"Failed to initialize COM: %#010x\r\n", hr);
        return 2;
    }

    auto loweredVolume = 0.1f;
    if (argc >= 4) {
        loweredVolume = _wtoi(argv[3]) * 0.01f;
    }

    hr = MainRoutine(argv[1], argv[2], loweredVolume);

    CoUninitialize();

    return FAILED(hr) ? 1 : 0;
}
