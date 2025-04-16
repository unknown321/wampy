## Backing up your device

### Why?

It is **always** nice to have a backup file in case installation goes horribly wrong (very unlikely). Don't forget, this
software is provided as is without any warranty.

### Backing up

Install version [1.63](https://github.com/bkerler/mtkclient/releases/tag/1.63) of https://github.com/bkerler/mtkclient.

> [!NOTE]
> Make sure your operating system got the required `udev` rules/device permissions - mtkclient includes useful resources for this

> [!TIP]
> Perform factory reset on device to reduce backup image size
>
> Songs won't compress as good as empty space. You may skip it if you don't care
> about hard drive space.


```shell
mtk rf <path to backup file>
```

Command waits for device, repeatedly printing:
```
...........

Port - Hint:

Power off the phone before connecting.
For brom mode, press and hold vol up, vol dwn, or all hw buttons and connect usb.
For preloader mode, don't press any hw button and connect usb.
If it is already connected and on, hold power for 10 seconds to reset.

```

1. Take out the SD card
2. Turn device off
3. Hold `volume down` + `play` buttons
4. Insert USB cable.

Which shoud result in:
```
....Port - Device detected :)

```

Dumps whole device to file (~20 minutes for 16Gb model)

Output file will be as big as your device's capacity (16 GB). You can compress it to about 800 MB using zip/rar/whatever
and decompress when needed.

### Restoring

Make sure you use **w**f command, which writes to device.

```shell
mtk wf <path to backup file>
```

### Example `mtkclient` backup output

<details>
<summary>Output of command mtk rf a50-sonyone.backup</summary>

```
....Port - Device detected :)
Preloader - 	CPU:			MT8590/MT7683/MT8521/MT7623()
Preloader - 	HW version:		0x0
Preloader - 	WDT:			0x10007000
Preloader - 	Uart:			0x11002000
Preloader - 	Brom payload addr:	0x100a00
Preloader - 	DA payload addr:	0x201000
Preloader - 	Var1:			0xa
Preloader - Disabling Watchdog...
Preloader - HW code:			0x8590
Preloader - Target config:		0x0
Preloader - 	SBC enabled:		False
Preloader - 	SLA enabled:		False
Preloader - 	DAA enabled:		False
Preloader - 	SWJTAG enabled:		False
Preloader - 	EPP_PARAM at 0x600 after EMMC_BOOT/SDMMC_BOOT:	False
Preloader - 	Root cert required:	False
Preloader - 	Mem read auth:		False
Preloader - 	Mem write auth:		False
Preloader - 	Cmd 0xC8 blocked:	False
Preloader - Get Target info
Preloader - 	HW subcode:		0x8a00
Preloader - 	HW Ver:			0xcb01
Preloader - 	SW Ver:			0x102
DA_handler - Device is unprotected.
DA_handler - Device is in Preloader-Mode :(
DALegacy - Uploading legacy da...
DALegacy - Uploading legacy stage 1 from MTK_AllInOne_DA_5.2228.bin
legacyext
legacyext - [LIB]: Legacy address check not patched.
legacyext
legacyext - [LIB]: Legacy DA2 CMD F0 not patched.
Preloader - Jumping to 0x200000
Preloader - Jumping to 0x200000: ok.
DALegacy - Got loader sync !
DALegacy - Reading nand info
DALegacy - Reading emmc info
DALegacy - ACK: 040291
DALegacy - Setting stage 2 config ...
DALegacy - Uploading stage 2...
DALegacy - Successfully uploaded stage 2
DALegacy - Connected to stage2
DALegacy - m_int_sram_ret = 0x0
m_int_sram_size = 0x40000
m_ext_ram_ret = 0x0
m_ext_ram_type = 0x2
m_ext_ram_chip_select = 0x0
m_int_sram_ret = 0x0
m_ext_ram_size = 0x20000000
randomid = 0xFA7B5F0AE9D55212ABBAD92E4DFDA3B

m_emmc_ret = 0x0
m_emmc_boot1_size = 0x400000
m_emmc_boot2_size = 0x400000
m_emmc_rpmb_size = 0x1000000
m_emmc_gp_size[0] = 0x0
m_emmc_gp_size[1] = 0x0
m_emmc_gp_size[2] = 0x0
m_emmc_gp_size[3] = 0x0
m_emmc_ua_size = 0x3ab400000
m_emmc_cid = 4xyz150xyz14a685dc187ad3e011051
m_emmc_fwver = 0100000000000000

Dumping sector 0 with flash size 0x3ab400000 as a50-sonyone.backup.
Progress: |██████████████████████████████████████████████████| 100.0% Read (Sector 0x1D5A000 of 0x1D5A000, ) 1.72 MB/s.72 MB/sB/sMB/s
Dumped sector 0 with flash size 0x3ab400000 as a50-sonyone.backup.

mtkclient on  v1.63 [?] via 🐍 v3.13.2 (venv) took 2h25m39s 
```

</details>

### Issues

* Double check that you see the MTxyz Preloader

```
kernel: usb 1-2.1.4: new high-speed USB device number 40 using xhci_hcd
kernel: usb 1-2.1.4: New USB device found, idVendor=0e8d, idProduct=2000, bcdDevice= 1.00
kernel: usb 1-2.1.4: New USB device strings: Mfr=1, Product=2, SerialNumber=0
kernel: usb 1-2.1.4: Product: MT65xx Preloader
kernel: usb 1-2.1.4: Manufacturer: MediaTek
kernel: cdc_acm 1-2.1.4:1.0: Zero length descriptor references
kernel: cdc_acm 1-2.1.4:1.0: probe with driver cdc_acm failed with error -22
kernel: cdc_acm 1-2.1.4:1.1: ttyACM0: USB ACM device
```

* udev rules / permissions

* [There are reports on this method not working for NW-A40][mtkclienterror] and NW-A50 using 
version 2.0.x of mtkclient with
```
DeviceClass - [LIB]: USB Overflow
```


[mtkclienterror]: https://github.com/unknown321/wampy/issues/1#issuecomment-2599157714
<details>
<summary>Full mtkclient error message failing to unpack bytes</summary>

```
....Port - Device detected :)
Preloader - 	CPU:			MT8590/MT7683/MT8521/MT7623()
Preloader - 	HW version:		0x0
Preloader - 	WDT:			0x10007000
Preloader - 	Uart:			0x11002000
Preloader - 	Brom payload addr:	0x100a00
Preloader - 	DA payload addr:	0x201000
Preloader - 	Var1:			0xa
Preloader - Disabling Watchdog...
Preloader - HW code:			0x8590
Preloader - Target config:		0x0
Preloader - 	SBC enabled:		False
Preloader - 	SLA enabled:		False
Preloader - 	DAA enabled:		False
Preloader - 	SWJTAG enabled:		False
Preloader - 	EPP_PARAM at 0x600 after EMMC_BOOT/SDMMC_BOOT:	False
Preloader - 	Root cert required:	False
Preloader - 	Mem read auth:		False
Preloader - 	Mem write auth:		False
Preloader - 	Cmd 0xC8 blocked:	False
Preloader - Get Target info
Preloader - 	HW subcode:		0x8a00
Preloader - 	HW Ver:			0xcb01
Preloader - 	SW Ver:			0x102
DaHandler - Device is unprotected.
DaHandler - Device is in Preloader-Mode.
DALegacy - Uploading legacy da...
DALegacy - Uploading legacy stage 1 from MTK_DA_V5.bin
LegacyExt
LegacyExt - [LIB]: Legacy address check not patched.
LegacyExt
LegacyExt - [LIB]: Legacy DA2 CMD F0 not patched.
Preloader - Jumping to 0x200000
Preloader - Jumping to 0x200000: ok.
DALegacy - Got loader sync !
DALegacy - Reading nand info
DALegacy - Reading emmc info
DALegacy - ACK: 040291
DALegacy - Setting stage 2 config ...
DALegacy - Uploading stage 2...
DALegacy - Successfully uploaded stage 2
DeviceClass
DeviceClass - [LIB]: USB Overflow
Traceback (most recent call last):
  File "mtkclient/mtk.py", line 1021, in <module>
    main()
    ~~~~^^
  File "mtkclient/mtk.py", line 1017, in main
    mtk = Main(args).run(parser)
  File "mtkclient/mtkclient/Library/mtk_main.py", line 682, in run
    mtk = da_handler.configure_da(mtk, preloader)
  File "mtkclient/mtkclient/Library/DA/mtk_da_handler.py", line 162, in configure_da
    if not mtk.daloader.upload_da(preloader=preloader):
           ~~~~~~~~~~~~~~~~~~~~~~^^^^^^^^^^^^^^^^^^^^^
  File "mtkclient/mtkclient/Library/DA/mtk_daloader.py", line 297, in upload_da
    return self.da.upload_da()
           ~~~~~~~~~~~~~~~~~^^
  File "mtkclient/mtkclient/Library/DA/legacy/dalegacy_lib.py", line 763, in upload_da
    if self.upload_da1():
       ~~~~~~~~~~~~~~~^^
  File "mtkclient/mtkclient/Library/DA/legacy/dalegacy_lib.py", line 621, in upload_da1
    if self.read_flash_info():
       ~~~~~~~~~~~~~~~~~~~~^^
  File "mtkclient/mtkclient/Library/DA/legacy/dalegacy_lib.py", line 532, in read_flash_info
    pi = PassInfo(self.usbread(0xA))
  File "mtkclient/mtkclient/Library/DA/legacy/dalegacy_lib.py", line 38, in __init__
    self.m_download_status = sh.dword(True)
                             ~~~~~~~~^^^^^^
  File "mtkclient/mtkclient/Library/utils.py", line 248, in dword
    dat = unpack(e + "I", self.data[self.pos:self.pos + 4])[0]
          ~~~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
struct.error: unpack requires a buffer of 4 bytes
```

</details>
