 This program allows full control of the RGB LED controller on select MSI boards. Unlike MSI Mystic Light, there is no colour limitation (which is a software limitation, not a hardware one). Aside from being able to set colours and other hardware-implemented effects with a single click, it also allows you to create and run Lua scripts producing other effects, which are run every time the computer starts, so you don't have to worry about re-applying them.

 The program runs on Windows 10 (x64 only), so long as VC Redist 2017 x64 is installed and .NET support is enabled (which it is, by default).

 Download is available [here](#how-to-use). **Check if your motherboard is supported. See [License](#license) for information on warranty.**
 
 **Many thanks to @nagisa for [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb) (a tool for controlling MSI MBs' RGB LEDs on Linux), which this tool is based on and which made it all the much easier to get going and figure out how the hardware worked.**

# Functionality
![MSIRGB](https://i.imgur.com/x37qlj6.png)

**The program allows you to set up to 4 different colours for the LEDs to switch between.** The chip will go through these colours (seemingly) randomly with an interval between each that is called 'step duration' and which can also be changed. This function mirrors the hardware implementation, i.e. it merely provides you with a way to use the built-in functions of the MB's LED controller. **Scripts allow you to change between any number of colours with any step duration.**

In the 'Effects' group, you're given the option to **change the step duration** (min 0, max 511, arbitrary time unit), and **enable/disable/adjust pulsing modes** - again, hardware implemented. Breathing mode is a smooth pulsing mode (gradually turning the LEDs brighter, and then less bright until they're off, then brighter again). Flashing mode is a 'sharp' pulsing mode, i.e., it'll turn the LEDs on and off instantly. There are 6 possible speeds, along with the option to keep the LEDs always on (flashing speed 'disabled').

**Clicking on Apply automatically turns the LEDs on if they're off.**

Along with those 'one-click-here-you-go' options, you also have the ability to **create and run scripts**. Scripts can bypass the hardware implemented functions and therefore should be much more powerful. There is one example available right now - the hue wheel - which cycles through blue, violet, 'pinkish' red and 'blueish' green. Scripts run automatically on Windows startup, so you don't have to reapply them once you turn one on. To disable any running script, simply press 'Apply' or 'Disable all lighting'.

# Example scripts
## Hue wheel (ported from [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb))
![animation of hue wheel](https://thumbs.gfycat.com/CanineShorttermAdamsstaghornedbeetle-size_restricted.gif)

This example can be found in Scripts\Hue Wheel.lua. If you downloaded the latest release, all you have to do is run MSIRGB and it'll find it.

I welcome anyone to share scripts and open pull requests to add new ones here.

# Scripts
Scripts are written in Lua and allow you to create new effects. Lua is a very easy to learn, dynamically typed language, meaning writing and testing scripts will be pretty straightforward even for newbies.

Scripts are loaded from the 'Scripts' folder in the directory where you have saved MSIRGB. Any file with a .lua extension in that folder (subdirectories excluded) will be loaded. The name of the script that will appear on MSIRGB is the file name without extension. As a convention, you should use upper camel case and spaces between words for your file names, e.g. 'Hue Wheel', 'Single Colour', 'Edgy Rainbow'.

Scripts, when enabled, run on Windows startup, so you don't have to enable them every time.

It is recommended that scripts set all LED settings on startup because there are no guarantees on their state at that point.

I'm open to pull requests for new scripting functionality. I'll also be opening an issue regarding the ability to use 'require' and whether it's worth it.

## API
Scripts are loaded into a sandboxed Lua 5.2 environment with the following standard functions and packages:
 - assert
 - collectgarbage
 - error
 - print
 - select
 - type
 - tonumber
 - tostring
 - next
 - ipairs
 - pairs
 - string package
 - table package
 - math package
 - bit32 package
 - os time package

Additionally, there is one extension function to the os package:
```lua
os.sleep(number ms)
```
This function pauses the Lua thread for the number of milliseconds specified in 'ms'. 'ms' must be a positive number. Execution stops if an invalid value is passed.

### Lighting module
The 'Lighting' module provides the API to control the LEDs. Here follows documentation for each function:
#      
```lua
Lighting.SetAllEnabled(boolean enabled)
```
This function allows you to enable or disable the LEDs connected to the motherboard. As far as I know, this controls both motherboard and header LEDs.
##  
```lua
Lighting.SetColour(number index, number r, number g, number b)
```
This function sets the colour of one of the 4 built-in colours. 'index' can be a number between 1 and 4, inclusive. 'r', 'g', 'b' are the R, G, B values between 0 and 255. Execution stops if any of the arguments is invalid.
##  
```lua
number, number, number Lighting.GetColour(number index)
```
This function gets the current colour of one of the 4 built-in colours. 'index' can be a number between 1 and 4, inclusive. The function returns the R, G, B values in that order. Execution stops if 'index' is invalid.
##  
```lua
boolean Lighting.SetBreathingModeEnabled(boolean enabled)
```
This function sets the state of the breathing mode. Breathing mode and flashing mode are exclusive, and the latter will override the former. If flashing mode is enabled at the time of the call, this function will fail and return false, otherwise it returns true.
##  
```lua
boolean Lighting.IsBreathingModeEnabled()
```
This function returns whether breathing mode is currently enabled.
##  
```lua
boolean Lighting.SetFlashingSpeed(number flashingSpeed)
```
This function sets the flashing mode speed. Valid values for 'flashingSpeed' range from 0 to 6, inclusive. A value of '0' means that flashing mode will be disabled. A value of '1' means fastest flashing, while a value of '6' means slowest flashing. This function will disable breathing mode if it was previously enabled. Execution stops if 'flashingSpeed' is invalid.
##  
```lua
number Lighting.GetFlashingSpeed()
```
This function returns the current flashing mode speed. Speeds are as described above for 'SetFlashingSpeed'.
##  
```lua
Lighting.SetStepDuration(number stepDuration)
```
This function sets the value for step duration. Step duration is the interval of time between changes in any of the 4 chosen colours. It is in an arbitrary time unit. Valid values range from 0 to 511, inclusive. Execution stops if 'stepDuration' is invalid.
##  
```lua
number Lighting.GetStepDuration()
```
This function returns the current step duration.

## Debugging
In order to debug scripts, you can click on 'Open script log' in the 'Scripts' group to open the script log containing errors, warnings and other information from the script. The script can also call 'print' to print information to the log.

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
 
 **There are other boards not supported by Mystic Light which reportedly work as well. It may be that they're instead supported in MSI's Gaming App, which I did not look into. It is still possible to use this program with any motherboard that isn't listed above, but do it at your own risk. Motherboards which fit this criterion and are reportedly working are:**
 - MSI B350 TOMAHAWK
 - MSI B350M MORTAR ARTIC
 
 **Some boards which [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb) is reportedly working with and are part of the list above are as follows (and so should work with this tool as well):**
 - MSI H270M MORTAR ARTIC
 - MSI X470 GAMING PRO
 - MSI X470 GAMING PLUS
 - MSI Z270 SLI PLUS
 - MSI Z370M MORTAR

 # How to build
 1. Install Visual Studio 2017 (any edition) with C++ desktop development tools, C# WPF support, and Blend for .NET. The project is currently set to use the Windows 10 SDK build 17663, but it should work with any other.
 2. Open the solution (MSIRGB.sln)
 3. Select debug/release target & build.
 
# License
 The code is licensed under the ISC license - the same one that [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb) uses. You're free to use, modify, redistribute and even use it in any commercial projects so long as you keep the copyright notice. **Be aware that this means I provide no warranty whatsoever should your motherboard malfunction**.
