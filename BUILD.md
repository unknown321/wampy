## Building

### Prerequisites

- Linux with Docker
- stock firmware file for NW-A50, `NW_WM_FW.UPG`
- [abootimg](https://github.com/ggrandou/abootimg)
- [guestmount](https://libguestfs.org/)
- ImageMagick

On Debian/Ubuntu: `sudo apt-get install guestmount abootimg --no-install-recommends && sudo apt-get install imagemagick`

It is recommended to make a backup if you are going to modify init scripts, nvram and other stuff that may put your
device into a brick. See [BACKUP.md](./BACKUP.md).

#### Getting NW_WM_FW.UPG

Download [installer](https://walkman.update.sony.net/fw/pc/A50/J/NW-A50_V1_02.exe), launch it. Use your os' find utility
looking for `.UPG` files.

Linux, Wine:

```shell
find ~/.wine -name "*.UPG"
```

### Build

Put `NW_WM_FW.UPG` into `libs` directory.

Preparing:

```shell
git submodule update --recursive --init
make prepare docker deps
```

Building:

```shell
make release
```

It takes more than two hours on i3-7100U to build project from scratch.

See [server/README.md](./server/README.md) for server rebuild instructions.

### Debugging

Look for arm32 gdb builds online; use gdb-multiarch on desktop. Valgrind must be compiled by you with `armv7`
compiler (`wampy-builder` docker image uses `armv5`, won't compile). You'll also need `ld-2.23.so` with debug symbols,
which is built during `armv7` toolchain compilation. Afterwards you can drop ld to `/lib/` on device, it will be
replaced with stock version from initrd after reboot. Crosstool `armv7` config is [here](./crosstool.armv7.config).

Fast way to confirm memory leaks is to check `/var/log/memmon1.csv`. Values update every 5 minutes, file is flushed to
storage every 15 minutes.

Check dmesg and logcat, increase log levels with PST_LOG_* variables. `getprop` shows nothing in tmux, use plain bash.

Debug info is provided with each release. First, you need to unpack binary with upx: `upx -d wampy`. Then:

```shell
set solib-search-path ./libs/sysroot/lib/
set sysroot ./libs/sysroot/
file ./wampy
core core....
```

#### Debugging OpenGL

Modern RenderDoc fail on desktop; 1.2 works sometimes.