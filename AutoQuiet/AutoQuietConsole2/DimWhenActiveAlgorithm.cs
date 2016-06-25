using AutoQuietLib;
using AutoQuietLib.Extensions;
using System;
using System.Collections.Specialized;
using System.Diagnostics;
using System.Linq;

namespace AutoQuietConsole2
{
    static class DimWhenActiveAlgorithm
    {
        internal static void Run(string processToDim, string priorityProcess, float loweredVolume)
        {
            DimWhenActiveAlgorithm.loweredVolume = loweredVolume;

            try
            {
                using (processToDimWatcher = new ProcessAudioWatcher(processToDim))
                {
                    var c = (INotifyCollectionChanged)processToDimWatcher.SessionList;
                    c.CollectionChanged += SessionListChanged;

                    Console.WriteLine($"Watching process to dim: {processToDim}");
                    using (priorityProcessWatcher = new ProcessAudioWatcher(priorityProcess))
                    {
                        Console.WriteLine($"Watching priority process: {priorityProcess}");
                        foreach (var existingSession in priorityProcessWatcher.SessionList)
                        {
                            existingSession.StateChanged += Session_StateChanged;
                            existingSession.Disconnected += Session_Disconnected;
                        }

                        var c2 = (INotifyCollectionChanged)priorityProcessWatcher.SessionList;
                        c2.CollectionChanged += SessionListChanged;

                        // If an existing session is already playing, dim our process
                        RecalculateProcessDimState(processToDimWatcher, priorityProcessWatcher);

                        Console.WriteLine("Press a key to stop.");
                        Console.ReadKey(true);

                        processToDimWatcher.SetAllSessionsToVolume(1.0f);
                    }
                }
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"Error occurred: {ex.Message}");
            }
        }

        private static float loweredVolume = 0.1f;
        private static ProcessAudioWatcher priorityProcessWatcher = null;
        private static ProcessAudioWatcher processToDimWatcher = null;

        private static void RecalculateProcessDimState(ProcessAudioWatcher process, ProcessAudioWatcher priorityProcess)
        {
            var volumeLevel = 1.0f;
            if (priorityProcess.SessionList.Any(s => s.State == AudioSessionState.Active))
            {
                volumeLevel = loweredVolume;
            }

            Console.WriteLine($"Setting {process.ProcessName} volume to {volumeLevel:P}");
            process.SetAllSessionsToVolume(volumeLevel);
        }

        private static void Session_Disconnected(AudioSession sender, AudioSessionDisconnectReason reason)
        {
            RecalculateProcessDimState(processToDimWatcher, priorityProcessWatcher);
        }

        private static void Session_StateChanged(AudioSession sender, AudioSessionState state)
        {
            RecalculateProcessDimState(processToDimWatcher, priorityProcessWatcher);
        }

        private static void SessionListChanged(object sender, NotifyCollectionChangedEventArgs args)
        {
            if (args.Action == NotifyCollectionChangedAction.Add)
            {
                foreach (var newSession in args.NewItems.Cast<AudioSession>())
                {
                    newSession.Disconnected += Session_Disconnected;
                    newSession.StateChanged += Session_StateChanged;
                }
            }
            else if (args.Action == NotifyCollectionChangedAction.Remove)
            {
                foreach (var oldSession in args.OldItems.Cast<AudioSession>())
                {
                    oldSession.Disconnected -= Session_Disconnected;
                    oldSession.StateChanged -= Session_StateChanged;
                }
            }
            else
            {
                Debug.Assert(false);
            }

            RecalculateProcessDimState(processToDimWatcher, priorityProcessWatcher);
        }
    }
}
