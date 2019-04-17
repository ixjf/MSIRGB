 This program allows full control of the RGB LED controller on select MSI boards. Unlike MSI Mystic Light, there is no colour limitation (which is a software limitation, not a hardware one). Aside from being able to set colours and other hardware-implemented effects with a single click, it also allows you to create and run Lua scripts producing other effects, which are run every time the computer starts, so you don't have to worry about re-applying them.

 The program runs on Windows 10 (x64 only), so long as VC Redist 2017 x64 is installed and .NET support is enabled (which it is, by default).

 Download is available [here](#how-to-install). **Check if your motherboard is supported. [I provide no warranty should your motherboard malfunction.](#license)**
 
 **Many thanks to @nagisa for [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb) (a tool for controlling MSI MBs' RGB LEDs on Linux), which this tool is based on and which made it all the much easier to get going and figure out how the hardware worked.**

# Functionality
![MSIRGB](https://i.imgur.com/QJi2SJs.png)

**The program allows you to set up to 8 different colours for the LEDs to switch between.** The LED controller will switch between those colours in the order from 1-8 and back (from v1.1.0-beta2 onwards), with an interval between each switch, called 'step duration', and which can be changed. This function mirrors the hardware implementation, i.e. it merely provides you with a way to use the built-in functions of the MB's LED controller. **Scripts allow you to change between any number of colours.** There is a limit to how many colours the chip understands, which is (only) 4,096. Because of that, there is a limit to how smooth a colour transition can be (see the hue wheel effect, for example).

In the 'Effects' group, you're given the option to **change the step duration** (min 0, max 511, arbitrary time unit), and **enable/disable/adjust pulsing modes** - again, hardware implemented. Breathing mode is a smooth pulsing mode (gradually turning the LEDs brighter, and then less bright until they're off, then brighter again). Flashing mode is a 'sharp' pulsing mode, i.e., it'll turn the LEDs on and off instantly. There are 6 possible speeds, along with the option to keep the LEDs always on (flashing speed 'disabled').

**Clicking on Apply automatically turns the LEDs on if they're off.**

Along with those 'one-click-here-you-go' options, you also have the ability to **create and run scripts**. Scripts can bypass the hardware implemented functions and therefore should be much more powerful. There is one example available right now - the hue wheel - which cycles through blue, violet, 'pinkish' red and 'blueish' green. Scripts run automatically on Windows startup, so you don't have to reapply them once you turn one on. To disable any running script, simply press 'Apply' or 'Disable all lighting'.

The tool has been reported to affect all motherboard LEDs as well as the headers.

# Example scripts
## Hue wheel (ported from [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb))
![animation of hue wheel](https://thumbs.gfycat.com/CanineShorttermAdamsstaghornedbeetle-size_restricted.gif)

This example can be found in Scripts\Hue Wheel.lua. If you downloaded the latest release, all you have to do is run MSIRGB and it'll find it.

I welcome anyone to share scripts and open pull requests to add new ones here.

# Scripts
Learn more about scripting functionality and find the API reference in the [wiki](../../wiki/Scripts).

# How to install
 1. Check if your motherboard is supported [here](#motherboard-support). If it is, you may proceed. If it isn't, it's possible the program won't work with your motherboard. There are MSI motherboards which aren't listed but are supported, but **PLEASE do not attempt to use this program with a non-MSI board. It will DEFINITELY not work**.
 2. Install [VC Redist 2017 x64](https://aka.ms/vs/15/release/vc_redist.x64.exe).
 3. Download the [latest release](https://github.com/ixjf/MSIRGB/releases/latest).
 4. Unpack the archive and run MSIRGB.exe. It'll ask you for administrator privileges. This is required to access the hardware, and there is literally no way around it (Mystic Light does it as well, just not on startup).

# Motherboard support
 This tool **should** work with the following motherboards:
 - MSI B450I GAMING PLUS AC
 - MSI B350I PRO AC
 - MSI A320M GAMING PRO
 - MSI B350M GAMING PRO
 - MSI A320M GRENADE
 - MSI B450M PRO-VDH
 - MSI B450M BAZOOKA
 - MSI X470 GAMING PRO CARBON AC
 - MSI X470 GAMING M7 AC
 - MSI X470 GAMING PLUS
 - MSI X470 GAMING PRO
 - MSI Z370 OC GAMING
 - MSI Z370 GAMING PLUS
 - MSI Z370M MORTAR
 - MSI Z370 PC PRO
 - MSI Z370-A PRO
 - MSI Z370 GAMING PRO CARBON AC
 - MSI Z370 GAMING PRO AC
 - MSI Z270 SLI PLUS
 - MSI Z270 KRAIT GAMING
 - MSI Z270 GAMING PRO
 - MSI Z270 GAMING M7
 - MSI Z270 TOMAHAWK
 - MSI H270 TOMAHAWK ARTIC
 - MSI Z299M-A PRO
 - MSI X299 RAIDER
 - MSI X399 GAMING PRO CARBON AC
 - MSI X399 SLI PLUS

# Confirmed supported motherboards
 - MSI B450I GAMING PLUS AC
 
 **There are other boards not supported by MSI Mystic Light (at least the version I checked) which reportedly work as well. It may be that they're instead supported by MSI Gaming App, which I did not check. Motherboards which fit this criterion and are reportedly working are:**
 - MSI B350 TOMAHAWK
 - MSI B450 TOMAHAWK
 - MSI B350 GAMING PLUS
 - MSI B350 PC MATE
 - MSI A320M BAZOOKA
 - MSI B250M MORTAR
 - MSI B350M MORTAR ARTIC
 - MSI B350 KRAIT GAMING
 
 **Some boards which [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb) is reportedly working with and are part of the list above are as follows (and so should work with this tool as well):**
 - MSI H270M MORTAR ARTIC
 - MSI X470 GAMING PRO
 - MSI X470 GAMING PLUS
 - MSI Z270 SLI PLUS
 - MSI Z370M MORTAR
 
 **It is still possible to use this program with any motherboard that isn't listed above or in section [Motherboard support](#motherboard-support), but do it at your own risk.**

 # How to build
 1. Install Visual Studio 2017 (any edition) with C++ desktop development tools, C# and WPF support, and Blend for .NET. The project is currently set to use the Windows 10 SDK build 17663, but it should work with any other.
 2. Open the solution (MSIRGB.sln)
 3. Select debug/release target & build.
 
# License
 The code is licensed under the ISC license - the same one that [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb) uses. You're free to use, modify, redistribute and even use it in any commercial projects so long as you keep the copyright notice. **Be aware that this means I provide no warranty whatsoever should your motherboard malfunction**.
