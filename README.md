wampy
=====

Alternative frontend for NW-A50 series Walkman player.

<img src="images/winamp.png" width="49%"> <img src="images/cassette.png" width="49%">

## Features:

- Incomplete Winamp 2 skin support
- Custom cassette skins
- On-the-fly skin change
- Default player enhancements (add clock and increase cover art size), see below

<img src="images/hagoromo-fix.png" width="70%">

## Install

#### Windows

Download exe from [releases](./releases), run and follow instructions.

Device will restart twice.

#### Linux:

##### Without adb (regular install):

- copy corresponding `NW_WM_FW.UPG` to root directory on device (the one with MUSIC directory)
- use [scsitool](https://www.rockbox.org/wiki/SonyNWDestTool.html)
    - `scsitool list_devices`
    - choose your device, I use `/dev/sg4`
    - `scsitool -d -s nw-a50 /dev/sg4 do_fw_upgrade` (may require root)
- device reboots, upgrades a little, reboots again and upgrades again (fully)

##### With adb:

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

### Mac:

No native installer.

See Linux section. You'll have to build `scsitool` yourself, good luck!

## Build

See [BUILD.md](./BUILD.md)