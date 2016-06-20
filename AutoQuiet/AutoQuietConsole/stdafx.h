// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h> // Win32
#include <Objbase.h> // COM
#include <atlbase.h> // ATL
#include <Psapi.h> // Win32 Process functions
#include <Shlwapi.h> // Path functions

#include <mmdeviceapi.h> // MM interfaces/classes
#include <audiopolicy.h> // Audio session interfaces/classes
#include <endpointvolume.h> // IAudioMeterInformation

#include <cstdio>
#include <memory>

#include "errormacros.h"
