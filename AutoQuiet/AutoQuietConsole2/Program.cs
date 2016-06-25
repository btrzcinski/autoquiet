using AutoQuietLib;
using AutoQuietLib.Extensions;
using System;
using System.Collections.Specialized;
using System.Diagnostics;
using System.Linq;
using System.Reflection;

namespace AutoQuietConsole2
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length < 2)
            {
                var assemblyName = Assembly.GetExecutingAssembly().GetName().Name;
                Console.WriteLine($"Usage: {assemblyName} process-to-dim priority-process-to-monitor [level-to-lower-volume-to]");
                Console.WriteLine("The level to lower the volume to when the priority process makes noise defaults to 10% if not specified.");
                Console.WriteLine($"Example: {assemblyName} chrome firefox 10");

                return;
            }

            var processToDim = args[0];
            var priorityProcess = args[1];
            loweredVolume = 0.1f;
            if (args.Length > 2)
            {
                try
                {
                    loweredVolume = int.Parse(args[2]) * 0.01f;
                    if (loweredVolume < 0.0f || loweredVolume > 1.0f)
                    {
                        throw new ArgumentOutOfRangeException(nameof(loweredVolume));
                    }
                }
                catch
                {
                    Console.Error.WriteLine($"'{args[2]}' is not a valid volume level; specify a number from 0-100.");
                    return;
                }
            }

            try
            {
                using (processToDimWatcher = new ProcessAudioWatcher(processToDim))
                {
                    Console.WriteLine($"Watching process to dim: {processToDim}");
                    using (priorityProcessWatcher = new ProcessAudioWatcher(priorityProcess))
                    {
                        Console.WriteLine($"Watching priority process: {priorityProcess}");
                        foreach (var existingSession in priorityProcessWatcher.SessionList)
                        {
                            existingSession.StateChanged += PriorityProcessSession_StateChanged;
                            existingSession.Disconnected += PriorityProcessSession_Disconnected;
                        }                        

                        var c = (INotifyCollectionChanged)priorityProcessWatcher.SessionList;
                        c.CollectionChanged += SessionListChanged;

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
            if (priorityProcess.SessionList.Any(s => s.State == AudioSessionState.Active))
            {
                process.SetAllSessionsToVolume(loweredVolume);
            }
            else
            {
                process.SetAllSessionsToVolume(1.0f);
            }
        }

        private static void PriorityProcessSession_Disconnected(AudioSession sender, AudioSessionDisconnectReason reason)
        {
            RecalculateProcessDimState(processToDimWatcher, priorityProcessWatcher);
        }

        private static void PriorityProcessSession_StateChanged(AudioSession sender, AudioSessionState state)
        {
            RecalculateProcessDimState(processToDimWatcher, priorityProcessWatcher);
        }

        private static void SessionListChanged(object sender, NotifyCollectionChangedEventArgs args)
        {
            if (args.Action == NotifyCollectionChangedAction.Add)
            {
                foreach (var newSession in args.NewItems.Cast<AudioSession>())
                {
                    newSession.Disconnected += PriorityProcessSession_Disconnected;
                    newSession.StateChanged += PriorityProcessSession_StateChanged;
                }
            }
            else if (args.Action == NotifyCollectionChangedAction.Remove)
            {
                foreach (var oldSession in args.OldItems.Cast<AudioSession>())
                {
                    oldSession.Disconnected -= PriorityProcessSession_Disconnected;
                    oldSession.StateChanged -= PriorityProcessSession_StateChanged;
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
