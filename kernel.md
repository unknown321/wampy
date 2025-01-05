Kernel versions:

Stock:

| model  | version                                                                                                      |
|--------|--------------------------------------------------------------------------------------------------------------|
| A50    | Linux version 3.10.26 (slave@azslave5q) (gcc version 4.8 (GCC) ) #1 SMP PREEMPT Wed Jul 31 16:22:50 JST 2019 |
| A40    | Linux version 3.10.26 (slave@azslave50) (gcc version 4.8 (GCC) ) #1 SMP PREEMPT Fri Oct 20 16:00:34 JST 2017 |
| A30    | Linux version 3.10.26 (slave@rairyu03) (gcc version 4.8 (GCC) ) #1 SMP PREEMPT Mon Mar 6 21:54:41 JST 2017   |
| ZX-300 | Linux version 3.10.26 (slave@azslave5h) (gcc version 4.8 (GCC) ) #1 SMP PREEMPT Wed Jul 31 16:24:44 JST 2019 |
| WM1A   | Linux version 3.10.26 (slave@azslave6n) (gcc version 4.8 (GCC) ) #1 SMP PREEMPT Wed Jul 31 16:25:40 JST 2019 |

Walkman One

| model  | version                                                                                                      |
|--------|--------------------------------------------------------------------------------------------------------------|
| A50    | Linux version 3.10.26 (slave@azslave5q) (gcc version 4.8 (GCC) ) #1 SMP PREEMPT Wed Jul 31 16:22:50 JST 2019 |
| A40    | Linux version 3.10.26 (slave@azslave5h) (gcc version 4.8 (GCC) ) #1 SMP PREEMPT Wed Jul 31 16:24:44 JST 2019 |
| A30    | Linux version 3.10.26 (slave@azslave5h) (gcc version 4.8 (GCC) ) #1 SMP PREEMPT Wed Jul 31 16:24:44 JST 2019 |
| ZX-300 | Linux version 3.10.26 (slave@azslave5h) (gcc version 4.8 (GCC) ) #1 SMP PREEMPT Wed Jul 31 16:24:44 JST 2019 |
| WM1A   | Linux version 3.10.26 (slave@azslave6n) (gcc version 4.8 (GCC) ) #1 SMP PREEMPT Wed Jul 31 16:25:40 JST 2019 |

A40 and A30 use ZX-300 kernel.

## compiling kernel

```
cp /tmp/config .config
cp -r arch/arm/mach-mt8590/bx8590m1_emmc/common/ arch/arm/mach-mt8590/BBDMP5_linux/

#drivers/misc/mediatek/lcm/mt65xx_lcm_list.c:18:28: fatal error: lcm_drv_mt8127.h: No such file or directory

# #include <lcm_drv_mt8127.h>

sed -i  's/<lcm_drv_mt8127.h>/"lcm_drv_mt8127.h"/g' drivers/misc/mediatek/lcm/mt65xx_lcm_list.c

# ld.bfd: read in flex scanner failed

sed -i 's#obj-y += imgsensor/src##g' drivers/misc/mediatek/Makefile
sed -i 's#obj-y += flashlight/src##g' drivers/misc/mediatek/Makefile

#include/linux/ion_drv.h:19:17: fatal error: ion.h: No such file or directory
#include <ion.h>

sed -i 's#<ion.h>#"../../drivers/staging/android/ion/ion.h"#g' include/linux/ion_drv.h

sed -i '2292s#!##' arch/arm/mach-mt8590/mt_clkmgr.c
```