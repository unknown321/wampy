cp /tmp/config .config
cp -r arch/arm/mach-mt8590/bx8590m1_emmc/common/ arch/arm/mach-mt8590/BBDMP5_linux/

#drivers/misc/mediatek/lcm/mt65xx_lcm_list.c:18:28: fatal error: lcm_drv_mt8127.h: No such file or directory
# #include <lcm_drv_mt8127.h>

sed -i  's/<lcm_drv_mt8127.h>/"lcm_drv_mt8127.h"/g' drivers/misc/mediatek/lcm/mt65xx_lcm_list.c

# ld.bfd: read in flex scanner failed
sed -i 's#obj-y   += imgsensor/src##g' drivers/misc/mediatek/Makefile
sed -i 's#obj-y   += flashlight/src##g' drivers/misc/mediatek/Makefile

#include/linux/ion_drv.h:19:17: fatal error: ion.h: No such file or directory
 #include <ion.h>

sed -i 's#<ion.h>#"../../drivers/staging/android/ion/ion.h"#g' include/linux/ion_drv.h

sed -i '2292s#!##' arch/arm/mach-mt8590/mt_clkmgr.c
