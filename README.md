 This tool allows full control of RGB LEDs on certain MSI boards. Unlike MSI's own Mystic Light app, there is no colour limitation (which is a software limitation, not a hardware one). It doesn't have any predefined style functions other than breathing mode and flashing mode (which are hardware implemented), but you can create almost any effect imaginable in software. 
 
 It currently should only work on Windows 10 x64, so long as VC Redist 2017 x64 is installed, but it could easily be ported to any other platform (most troublesome is Windows, since it doesn't allow direct access to the hardware without kernel drivers).
 
 **Many thanks to @nagisa for [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb) (a tool for controlling MSI MBs' RGB LEDs on Linux), which this tool is based on and which made it all the much easier to get going and figure out how the hardware worked.**
 
# Motherboard support
 This tool should work out-of-the-box with the following motherboards:
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
 
 **Important: See [License](#license) for information on warranty**.

# Confirmed supported motherboards
 - MSI B450I GAMING PLUS AC
 
 **There are other boards not supported by Mystic Light which reportedly work as well. One case is MSI B350M MORTAR ARCTIC. It may be that they're instead supported in MSI's Gaming App, which I did not look into. It is still possible to use this tool with any motherboard that isn't listed above by passing the --ignore-check flag to it, but do it at your own risk.**
 
 **Some boards which [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb) is reportedly working with and are part of the list above are as follows (and so should work with this tool as well):**
 - MSI H270M MORTAR ARTIC
 - MSI X470 GAMING PRO
 - MSI X470 GAMING PLUS
 - MSI Z270 SLI PLUS
 - MSI Z370M MORTAR
 
# Examples
## Hue wheel (ported from [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb))
![animation of hue wheel](https://thumbs.gfycat.com/CanineShorttermAdamsstaghornedbeetle-size_restricted.gif)

This example can be found in examples/hue_wheel.py. It requires you to install Python. In order to run the example, you need to set an environment variable in the command line beforehand pointing to MSIRGB's EXE, like this: `set MSIRGB_PATH=../x64/Release/MSIRGB.exe`
 
# License
 The code is licensed under the ISC license - the same one that [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb) uses. You're free to use, modify, redistribute and even use it in any commercial projects so long as you keep the copyright notice. **Be aware that this means I provide no warranty whatsoever should your motherboard malfunction**.
 
# How to use
 1. Download the [latest release](https://github.com/ixjf/MSIRGB/releases/latest).
 2. Install [VC Redist 2017 x64](https://aka.ms/vs/15/release/vc_redist.x64.exe).
 3. Run the command line as administrator (the tool requires such privileges to load a kernel driver needed to access the hardware).
 4. Run MSIRGB.exe with the command `MSIRGB.exe -h`. That should present all available commands. To see options for LED configuration, run `MSIRGB.exe config -h`. 
 
 You can: 
 - Disable the LEDs, by running `MSIRGB.exe disable`.
 - Change LED colour (there are 4 different values for colour, hardware implemented, which the chip loops through every x time, whose value is set by the -d option).
 - Set LED breathing mode enabled/disabled. (turns on/off smoothly)
 - Set LED flashing mode enabled/disabled. (turns on/off instantly)
 - Set step duration. (as I said above, the -d option)
 - Set channels inverted.
 - Enable some fade-in effect that I ported over from [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb) but which only works with certain boards. (There are other effects, but those also seem to work only on some boards)
 - Run any command with an unsupported motherboard, with `MSIRGB.exe --ignore-check <command>`. 
 
 Here's an example for changing LED config (sets colour to 0xffee11, flashing mode disabled, step duration set to max, breathing mode enabled): `MSIRGB.exe config 0xffee11 0xffee11 0xffee11 0xffee11 -f0 -d511 -b`
 
# How to build
 1. Install Visual Studio 2017 (any edition) with C++ desktop development tools. The project is currently set to use the Windows 10 SDK build 17663, but it should work with any other.
 2. Open the solution (MSIRGB.sln)
 3. Select debug/release target & build.
