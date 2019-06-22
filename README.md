 This program allows to control the LED lighting (motherboard* and headers) on select MSI boards. Unlike MSI Mystic Light, there is no limitation to only 7 colours. Aside from providing a GUI for quickly experimenting with different effects, it also provides a scripting interface to create more advanced effects like [this one](#example-effects) and to auto-run these effects on Windows start-up.

 MSIRGB runs on Windows 10 (x64 only), so long as VC Redist 2017 x64 is installed and .NET support is enabled (which it is, by default). Older versions of Windows have not been tested.

 Download is available [here](#how-to-install). **Check if your motherboard is supported. [I provide no warranty should your motherboard malfunction.](#license)**
 
 This project began as a port of [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb) for Windows, but eventually evolved to have a more Windows user-friendly interface, both visually and in the creation of effects, but it is still heavily based on exposing the hardware-implemented LED functionality through which the user can create those effects.
 
\* It has been reported to affect all motherboard LEDs as well, but I do not guarantee it.
 
# How it works
![MSIRGB](media/main_window.PNG)

When you run MSIRGB, you will see this GUI. Here you can experiment with simple effects by changing hardware-implemented settings in your motherboard.

- **Colours**: You can choose to change between 8 different colours. The order by which the motherboard switches between them is from left to right as displayed. You can choose from 4096 different colours (unfortunately, this is a hardware limitation).

- **Step duration**: This is the interval of time between each change of colour. Possible values are 0 to 511, inclusive. The unit of time is unknown.

- **Breathing mode**: This is one of the two flashing modes supported by all the motherboards that MSIRGB targets. Breathing mode flashes from on to off and back again by gradually changing brightness. The speed of this effect cannot be changed.

- **Flashing mode**: This other flashing mode flashes from on to off and back again instantly, and the speed of the effect can be changed to different preset values.

- **Scripts**: Scripts allow you to leverage these hardware-implemented functions to create more advanced effects. While the functionality provided by the motherboard seems minimal, you can create some interesting effects with them. Currently MSIRGB has 4 example effects you can download and try: the [hue wheel effect](#example-effects), the strobe effect, the police lights effect, and the pumpkin effect. New effects are always welcome. Feel free to open a pull request if you'd like to contribute.

# Example effects
Hue Wheel

 <img src="/media/hue_wheel.gif?raw=true">

# Scripting interface (for creating effects)
Learn more about how to create scripts and find the Lua API reference in the [wiki](../../wiki/Scripts).

# How to install
 1. Check if your motherboard is supported [here](#motherboard-support). If it is, you may proceed. If it isn't, it's possible the program won't work with your motherboard. Other MSI motherboards may be supported despite not being listed. Non-MSI motherboards are not supported and MSIRGB will not run on those.
 2. Install [VC Redist 2017 x64](https://aka.ms/vs/15/release/vc_redist.x64.exe).
 3. Download the [latest release](https://github.com/ixjf/MSIRGB/releases/download/v2.2.1.2/MSIRGB-v2.2.1.2.7z).
 4. Download the [latest example effects](https://github.com/ixjf/MSIRGB/releases/download/scripts-v2.2.0/MSIRGB-Scripts.7z).
 5. Unpack the archive from 3. into any folder, then create a "Scripts" folder in that same directory and unpack the archive from 4. there, such that your directory structure is like this:
    - Scripts/
        - Hue Wheel.lua
        - ...
    - ...
    - MSIRGB.exe
 6. Run MSIRGB.exe. It'll ask you for administrator privileges. This is required to access the hardware.

# Motherboard support
 MSIRGB should **theoretically** work with the following motherboards:
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
 
 Other MSI motherboards may be supported. See the next section for the list of all motherboards reported to be working with MSIRGB.

# Reported working motherboards
 - MSI B450I GAMING PLUS AC
 - MSI Z270 GAMING M7
 - MSI X470 GAMING PRO
 
 **Certain motherboards are not supported by MSI Mystic Light and so are not on the list in section [Motherboard support](#motherboard-support) but some of them have been reported to be working with MSIRGB all the same. They are the following:**
 - MSI B350 TOMAHAWK
 - MSI B450 TOMAHAWK
 - MSI B350 GAMING PLUS
 - MSI B350 PC MATE
 - MSI A320M BAZOOKA
 - MSI B250M MORTAR
 - MSI B350M MORTAR ARTIC
 - MSI B350 KRAIT GAMING
 - MSI X370 KRAIT GAMING
 - MSI B350M MORTAR
 - MSI B350M PRO-VDH
 
 **Motherboards which [nagisa/msi-rgb](https://github.com/nagisa/msi-rgb) is reportedly working with and should work with MSIGB are as follows:**
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
