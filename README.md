# MiniVitaTV

This plugin fakes your device as a PS TV and allows you to connect up to 4 DS3/DS4 controllers to your PS Vita, and play local multiplayer games, by using the exact same driver as on the PS TV. Yes, the PS Vita screen is rather small, but I think it's kinda cute to play on it with friends while travelling or so.  
Note that this plugin is still in beta stage and the known issues will be fixed when I find time.  
**Only official DS3/DS4 controllers are currently supported!**

## Changelog beta 4
- Fixed issue where physical buttons could not be used. Thanks to cuevavirus.

## Changelog beta 3
- Fixed compatibility with h-encore 2.0.

## Changelog beta 2
- Added support for L2/R2 buttons and rumble.

## Installation

1. Download [minivitatv.skprx](https://github.com/TheOfficialFloW/MiniVitaTV/releases/download/v0.4/minivitatv.skprx) and [ds3.skprx](https://github.com/TheOfficialFloW/MiniVitaTV/releases/download/v0.4/ds3.skprx), and copy them to `ux0:tai`.

2. Add these lines to taiHEN config.txt at `ux0:tai/config.txt`:

   ```
   *KERNEL
   ux0:tai/minivitatv.skprx
   ux0:tai/ds3.skprx
   ```

3. Reboot your device and relaunch HENkaku.

If you're using enso you should boot with L trigger hold to skip plugins in case you want to remove this plugin. Remember it's a beta.

## Pairing a DS4 controller

#### Using it for the first time:

1. Go to Settings → Devices → Bluetooth Devices
2. Press SHARE+PS on the DS4 for about 3-4 seconds, until the lightbar blinks very quickly
3. The DS4 will then connect and be paired (don't press over it when it appears)

#### Using it once paired (see above):

1. Just press the PS button and it will connect to the Vita

## Pairing a DS3 controller

#### Using it for the first time:

1. Download [this](http://dancingpixelstudios.com/sixaxis-controller/sixaxispairtool/) tool (or [this](https://help.ubuntu.com/community/Sixaxis?action=AttachFile&do=get&target=sixpair.c) other one if you want to compile it yourself)
2. Connect your DS3 to the PC and open the tool
3. Introduce the Vita's **MAC address plus 1** to the tool (Settings → System → System information)

#### Using it once paired (see above):
1. Just press the PS button and it will connect to the Vita

## Known issues

- Pressing the touchpad on DS4 crashes Adrenaline

## Credits

Thanks to xerpi for his work on ds3vita/ds4vita and the pairing guide.

Thanks to cuevavirus for working Vita buttons.
