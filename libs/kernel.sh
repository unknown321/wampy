#!/bin/bash
makeLinks() {
    utils=(addr2line c++filt gcc gcc-ranlib ld.bfd ranlib strip ar cpp gcc-4.8 gcov nm readelf as elfedit gcc-ar gprof objcopy size c++ g++ gcc-nm ld objdump strings)
    for i in "${utils[@]}"; do
        ln -s /x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-$i /usr/bin/arm-eabi-$i
    done
}

makeLinks

sudo -u user  make -C llusbdac/kernel arch=arm cross_compile=arm-eabi- olddefconfig
sudo -u user  make -C llusbdac/kernel arch=arm cross_compile=arm-eabi- prepare
sudo -u user  make -C llusbdac/kernel arch=arm cross_compile=arm-eabi- scripts
