## Building

### Prerequisites

- Linux with Docker
- `libMali_linux.so`

#### libMali_linux.so

You can get `libMali_linux.so` from firmware file or device with adb on.

##### adb:

```shell
adb pull /system/lib/libMali_linux.so .
```

##### Firmware upgrade file:

You'll need [upgtool](https://www.rockbox.org/wiki/SonyNWUPGTool#Getting_the_tool), firmware upgrade
file (`NW_WM_FW.UPG`) and be able to mount ext4 filesystem.

Linux, NW-A50:

```shell
$ mkdir fw
$ upgtool_64-v3 -m nw-a50 -e -o ./fw/ ./NW_WM_FW.UPG -z 6
$ mkdir tmpmount
$ sudo mount -t ext4 -o loop ./fw/6.bin tmpmount
$ cp tmpmount/lib/libMali_linux.so ./libs
$ sudo umount tmpmount
$ rm -rf tmpmount fw
```

### Build

Preparing:

```shell
git submodule update --recursive --init
make prepare docker deps server
```

Building:

```shell
make release
```

It takes 97 minutes on i3-7100U to build project from scratch (without downloading Qt and firmware).

See [server/README.md](./server/README.md) for server rebuild instructions.
