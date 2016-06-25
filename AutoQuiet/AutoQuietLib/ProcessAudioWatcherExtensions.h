#pragma once

#include "ProcessAudioWatcher.h"

namespace AutoQuietLib
{
    namespace Extensions
    {
        using namespace System::Runtime::CompilerServices;

        [ExtensionAttribute]
        public ref class ProcessAudioWatcherExtensions abstract sealed
        {
        public:
            [ExtensionAttribute]
            static void SetAllSessionsToVolume(ProcessAudioWatcher^ watcher, float volume)
            {
                auto enumerator = watcher->SessionList->GetEnumerator();
                while (enumerator->MoveNext())
                {
                    enumerator->Current->MasterVolume = volume;
                }
            }
        };
    }
}
