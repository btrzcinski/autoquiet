using AutoQuietLib;
using AutoQuietLib.Extensions;
using System;
using System.Linq;
using System.Threading;

namespace AutoQuietConsole
{
    static class DimWhenPeakMeterRegisters
    {
        internal static void Run(string processToDim, string priorityProcess, float loweredVolume)
        {
            Console.WriteLine($"Using {nameof(DimWhenPeakMeterRegisters)} algorithm");

            try
            {
                using (var processToDimWatcher = new ProcessAudioWatcher(processToDim))
                {
                    Console.WriteLine($"Watching process to dim: {processToDim}");
                    using (var priorityProcessWatcher = new ProcessAudioWatcher(priorityProcess))
                    {
                        Console.WriteLine($"Watching priority process: {priorityProcess}");

                        // Signal timer now and every 500ms
                        var t = new Timer((state) => RecalculateProcessDimState(processToDimWatcher, priorityProcessWatcher, loweredVolume),
                            null, 0, 500);             

                        Console.WriteLine("Press a key to stop.");
                        Console.ReadKey(true);

                        // Stop timer
                        t.Change(Timeout.Infinite, Timeout.Infinite);

                        processToDimWatcher.SetAllSessionsToVolume(1.0f);
                    }
                }
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"Error occurred: {ex.Message}");
            }
        }

        private static bool isProcessDimmed = false;
        private static void RecalculateProcessDimState(ProcessAudioWatcher process, ProcessAudioWatcher priorityProcess,
            float loweredVolume)
        {
            bool shouldDim = priorityProcess.SessionList.Any(s => s.PeakMeterValue > 0.01f);

            if (shouldDim != isProcessDimmed)
            {
                isProcessDimmed = shouldDim;
                var volumeLevel = shouldDim ? loweredVolume : 1.0f;
                Console.WriteLine($"Setting {process.ProcessName} volume to {volumeLevel:P}");
                process.SetAllSessionsToVolume(volumeLevel);
            }
        }
    }
}
