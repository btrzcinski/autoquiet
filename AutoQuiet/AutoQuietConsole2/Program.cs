using AutoQuietLib;
using System;
using System.Collections.Specialized;
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
            var loweredVolume = 0.1f;
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
                using (var watcher = new ProcessAudioWatcher(processToDim))
                {
                    Console.WriteLine($"Watching process: {processToDim}");
                    foreach (var existingSession in watcher.SessionList)
                    {
                        Console.WriteLine("Existing session: PID = {0}, {1}", existingSession.Process.Id, existingSession.Process.ProcessName);
                        existingSession.StateChanged += Session_StateChanged;
                        existingSession.Disconnected += Session_Disconnected;
                    }

                    var c = (INotifyCollectionChanged)watcher.SessionList;
                    c.CollectionChanged += SessionListChanged;

                    Console.WriteLine("Press a key to stop.");
                    Console.ReadKey(true);
                }
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"Error occurred: {ex.Message}");
            }
        }

        private static void Session_Disconnected(AudioSession sender, AudioSessionDisconnectReason reason)
        {
            Console.WriteLine("Session disconnected ({0}): PID = {1}, {2}", reason, sender.Process.Id, sender.Process.ProcessName);
        }

        private static void Session_StateChanged(AudioSession sender, AudioSessionState state)
        {
            Console.WriteLine("Session state changed ({0}): PID = {1}, {2}", state, sender.Process.Id, sender.Process.ProcessName);
        }

        private static void SessionListChanged(object sender, NotifyCollectionChangedEventArgs args)
        {
            if (args.Action == NotifyCollectionChangedAction.Add)
            {
                foreach (var newSession in args.NewItems.Cast<AudioSession>())
                {
                    Console.WriteLine("New session: PID = {0}, {1}", newSession.Process.Id, newSession.Process.ProcessName);
                    newSession.Disconnected += Session_Disconnected;
                    newSession.StateChanged += Session_StateChanged;
                }
            }
            else if (args.Action == NotifyCollectionChangedAction.Remove)
            {
                foreach (var oldSession in args.OldItems.Cast<AudioSession>())
                {
                    Console.WriteLine("Removed session: PID = {0}, {1}", oldSession.Process.Id, oldSession.Process.ProcessName);
                    oldSession.Disconnected -= Session_Disconnected;
                    oldSession.StateChanged -= Session_StateChanged;
                }
            }
        }
    }
}
