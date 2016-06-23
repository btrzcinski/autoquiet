using AutoQuietLib;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

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
                LowerProcessVolumeWhenPriorityProcessMakesNoise(processToDim, priorityProcess, loweredVolume);
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"Error occurred: {ex.Message}");
            }
        }

        private static AudioSession GetFirstAudioSessionForProcessName(string processName)
        {
            var processCandidates = Process.GetProcessesByName(processName);
            if (processCandidates.Length == 0)
            {
                throw new Exception($"{processName} is not running; try starting it first.");
            }

            AudioSession processAudioSession = null;
            foreach (var candidate in processCandidates)
            {
                if (AudioSession.TryGetFirstAudioSessionForProcess(candidate, out processAudioSession))
                {
                    break;
                }
            }

            if (processAudioSession == null)
            {
                throw new Exception($"Can't find audio session for {processName}; try playing audio first.");
            }

            return processAudioSession;
        }

        private static void LowerProcessVolumeWhenPriorityProcessMakesNoise(string processToDimName, string priorityProcessName, float loweredVolume)
        {
            var processToDimSession = GetFirstAudioSessionForProcessName(processToDimName);
            var priorityProcessSession = GetFirstAudioSessionForProcessName(priorityProcessName);

            Console.WriteLine($"{processToDimName} PID = {processToDimSession.ProcessId}");
            Console.WriteLine($"{priorityProcessName} PID = {priorityProcessSession.ProcessId}");
        }
    }
}
