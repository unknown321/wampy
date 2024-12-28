## Installing wampy

<!-- TOC -->

* [Installing wampy](#installing-wampy)
  * [Windows](#windows)
  * [Linux](#linux)
    * [Without adb](#without-adb)
    * [With adb](#with-adb)
  * [Mac](#mac)
* [Uninstalling](#uninstalling)

<!-- TOC -->

### Windows

Download exe from [releases](https://github.com/unknown321/wampy/releases), run and follow instructions.

Device will restart twice.

### Linux

#### Without adb

- copy corresponding `NW_WM_FW.UPG` to root directory on device (the one with MUSIC directory)
- use [scsitool](https://www.rockbox.org/wiki/SonyNWDestTool.html):
  - `scsitool list_devices`
  - choose your device, I use `/dev/sg4`
  - `scsitool -d -s nw-a50 /dev/sg4 do_fw_upgrade` (may require root)
- device reboots, upgrades a little, reboots again and upgrades again (fully)

#### With adb

If your player has adb on, there is no need for scsitool.

Turn USB Mass storage **OFF**.

Copy `NW_WM_FW.UPG` to root directory on device (the one with MUSIC directory):

```shell
adb push NW_WM_FW.UPG /contents/
```

Run on your computer:

```shell
adb shell nvpflag fup 0x70555766
adb shell reboot
```

Device reboots, upgrades a little, reboots again and upgrades again (fully).

### Mac

No native installer.

See Linux section. You'll have to build `scsitool` yourself, good luck!

## Uninstalling

Download `*.uninstaller.*` file for your platform and firmware
from [releases](https://github.com/unknown321/wampy/releases), follow instructions from `install` section above.