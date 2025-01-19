#!/bin/bash
utils=(addr2line c++filt gcc gcc-ranlib ld.bfd ranlib strip ar cpp gcc-4.8 gcov nm readelf as elfedit gcc-ar gprof objcopy size c++ g++ gcc-nm ld objdump strings)
for i in "${utils[@]}"; do
    ln -s /x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-$i /usr/bin/arm-eabi-$i
done

# fyi this kernel won't boot

sudo -u user cp -r llusbdac/kernel/arch/arm/mach-mt8590/bx8590m1_emmc/common/ llusbdac/kernel/arch/arm/mach-mt8590/BBDMP5_linux/
sed -i 's#obj-y   += imgsensor/src##g' llusbdac/kernel/drivers/misc/mediatek/Makefile
sed -i 's#obj-y   += flashlight/src##g' llusbdac/kernel/drivers/misc/mediatek/Makefile
sed -i 's/<lcm_drv_mt8127.h>/"lcm_drv_mt8127.h"/g' llusbdac/kernel/drivers/misc/mediatek/lcm/mt65xx_lcm_list.c
sed -i 's#<ion.h>#"../../drivers/staging/android/ion/ion.h"#g' llusbdac/kernel/include/linux/ion_drv.h
sed -i '2292s#!##' llusbdac/kernel/arch/arm/mach-mt8590/mt_clkmgr.c

sudo -u user make -C llusbdac/kernel -j4