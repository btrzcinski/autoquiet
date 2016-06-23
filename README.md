# autoquiet

## What does it do?

This app will lower the volume of an app if another app starts playing audio.

## Can I try it?

Sure, but you'll have to build it from source - there's no binary release yet.

## Usage

Usage: `AutoQuietConsole2.exe process-to-dim.exe priority-process-to-monitor.exe [level-to-lower-volume-to]`

The level to lower the volume to when the priority process makes noise defaults to 10% if not specified.

Example: `AutoQuietConsole2.exe chrome.exe firefox.exe 10`

## Requirements

To use:
* Windows 7 or above
* [Visual C++ 2015 32-bit Redistributable](https://download.microsoft.com/download/0/5/0/0504B211-6090-48B1-8DEE-3FF879C29968/vc_redist.x86.exe)
* [.NET Framework 4.5.2](http://www.microsoft.com/en-us/download/details.aspx?id=42643)

To develop:
* [Visual Studio 2015](https://www.visualstudio.com/downloads/download-visual-studio-vs) Community (or higher)
