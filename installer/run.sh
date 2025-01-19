#!/bin/sh
VENDOR=/system/vendor/unknown321/
BINARY=wampy
user=system
group=system

log()
{
        oldIFS=$IFS
        IFS="
"
        for line in $(echo "${1}"); do
                echo "$(date) ${line}" >> $LOG_FILE
        done
        IFS=$oldIFS
}

clearBadBoots() {
  if ! test -f /contents/wampy/config.ini; then
    log "no previous config found"
    return
  fi

  log "badboots = $(busybox grep badboots /contents/wampy/config.ini)"
  log "resetting badboots to 0"
  busybox sed -i 's/badboots.*/badboots = 0/g' /contents/wampy/config.ini
  log "badboots = $(busybox grep badboots /contents/wampy/config.ini)"
}

install() {
  log "installing ${BINARY}"
  mkdir -p ${VENDOR}/bin/
  cp ${BINARY} ${VENDOR}/bin/
  chmod 0755 ${VENDOR}/bin/${BINARY}

  log "installing upgtool"
  cp upgtool-linux-arm5 ${VENDOR}/bin/
  chmod 0755 ${VENDOR}/bin/upgtool-linux-arm5

  log "installing ${BINARY} service"
  cp "init.${BINARY}.rc" ${INITRD_UNPACKED}/
  chmod 0600 "${INITRD_UNPACKED}/init.${BINARY}.rc"
  grep -q "init.${BINARY}.rc" "${INITRD_UNPACKED}/init.rc"
  if test $? -ne 0; then
    log "adding service"
    echo -e "import init.${BINARY}.rc\n$(cat ${INITRD_UNPACKED}/init.rc)" > "${INITRD_UNPACKED}/init.rc"
  fi

  log "installing libraries"
  mkdir -p ${VENDOR}/lib/
  cp libMagick++-7.Q8HDRI.so ${VENDOR}/lib/
  cp libMagickCore-7.Q8HDRI.so ${VENDOR}/lib/
  cp libMagickWand-7.Q8HDRI.so ${VENDOR}/lib/
  chmod 0700 ${VENDOR}/lib/libMagick*-7.Q8HDRI.so
  chown ${user}:${group} ${VENDOR}/lib/libMagick*-7.Q8HDRI.so

  cp libjpeg.so.62.4.0 ${VENDOR}/lib/
  chmod 0700 ${VENDOR}/lib/libjpeg.so.62.4.0
  chown ${user}:${group} ${VENDOR}/lib/libjpeg.so.62.4.0
  ln -s ${VENDOR}/lib/libjpeg.so.62.4.0 ${VENDOR}/lib/libjpeg.so.62
  busybox chown -h ${user}:${group} ${VENDOR}/lib/libjpeg.so.62

  cp libprotobuf.so.32.0.12 ${VENDOR}/lib/
  chmod 0700 ${VENDOR}/lib/libprotobuf.so.32.0.12
  chown ${user}:${group} ${VENDOR}/lib/libprotobuf.so.32.0.12
  ln -s ${VENDOR}/lib/libprotobuf.so.32.0.12 ${VENDOR}/lib/libprotobuf.so.32
  ln -s ${VENDOR}/lib/libprotobuf.so.32.0.12 /system/vendor/sony/lib/libprotobuf.so.32
  busybox chown -h ${user}:${group} ${VENDOR}/lib/libprotobuf.so.32
  busybox chown -h ${user}:${group} /system/vendor/sony/lib/libprotobuf.so.32

  log "installing server"
  test -f /system/vendor/sony/plugins/platforms/libqeglfs.so_vendor
  if test $? -ne 0; then
    # make sure this file is from SONY (not linked to libprotobuf)
    busybox grep -q protobuf /system/vendor/sony/plugins/platforms/libqeglfs.so
    if test $? -eq 1; then
      log "backing up libqeglfs"
      busybox cp -p /system/vendor/sony/plugins/platforms/libqeglfs.so /system/vendor/sony/plugins/platforms/libqeglfs.so_vendor
    else
      log "libqeglfs is linked to libprotobuf, not backing up"
    fi
  else
    log "libqeglfs backup already exists"
  fi

  cp libqeglfs.so /system/vendor/sony/plugins/platforms/
  chown root:shell /system/vendor/sony/plugins/platforms/libqeglfs.so
  chmod 0755 /system/vendor/sony/plugins/platforms/libqeglfs.so

  log "installing winamp skin"
  mkdir -p ${VENDOR}/usr/share/${BINARY}/skins/winamp/
  cp "base-2.91.wsz" ${VENDOR}/usr/share/${BINARY}/skins/winamp/

  log "wiping old cassettes"
  rm -r ${VENDOR}/usr/share/${BINARY}/skins/cassette/

  log "installing cassettes"
  mkdir -p ${VENDOR}/usr/share/${BINARY}/skins/cassette/
  busybox tar -C ${VENDOR}/usr/share/${BINARY}/skins/cassette/ -xf cassette.tar.gz

  log "wiping old digital clock"
  rm -r ${VENDOR}/usr/share/${BINARY}/skins/digital_clock/

  log "installing digital clock"
  mkdir -p ${VENDOR}/usr/share/${BINARY}/skins/digital_clock/
  busybox tar -C ${VENDOR}/usr/share/${BINARY}/skins/digital_clock/ -xf digital_clock.tar.gz

  log "installing licenses"
  mkdir -p ${VENDOR}/usr/share/${BINARY}/doc/
  cp LICENSE ${VENDOR}/usr/share/${BINARY}/doc/
  cp LICENSE_3rdparty ${VENDOR}/usr/share/${BINARY}/doc/

  log "installing qr code"
  cp qr.bmp ${VENDOR}/usr/share/${BINARY}/
  cp qrDonate.bmp ${VENDOR}/usr/share/${BINARY}/

  log "installing tunings"
  mkdir -p ${VENDOR}/usr/share/${BINARY}/sound_settings/
  busybox tar -C ${VENDOR}/usr/share/${BINARY}/sound_settings/ -xf tunings.tar.gz

  log "installing llusbdac"
  mkdir -p ${VENDOR}/modules/
  cp llusbdac.ko ${VENDOR}/modules/

  clearBadBoots
}

log "installing $(cat product_info)"

mount -t ext4 -o rw /emmc@android /system

install

sync
umount /system
