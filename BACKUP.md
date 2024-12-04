## Backing up your device

### Why?

It is **always** nice to have a backup file in case installation goes horribly wrong (very unlikely). Don't forget, this
software is provided as is without any warranty.

### Backing up

Install latest version of https://github.com/bkerler/mtkclient.

Perform factory reset on device, take sd card out.

Turn device off. Hold `volume down` + `play` buttons and insert USB cable.

Dump whole device to file (~20 minutes):

```shell
mtk rf <path to backup file>
```

Output file will be as big as your device's capacity (16GB). You can compress it to about 800 MB using zip/rar/whatever
and decompress when needed.

Factory reset is needed because your songs won't compress as good as empty space.

### Restoring

Make sure you use **w**f, which writes to device.

```shell
mtk wf <path to backup file>
```