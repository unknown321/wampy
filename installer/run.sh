#!/bin/sh
VENDOR=/system/vendor/unknown321/
BINARY=wampy
user=system
group=system

CP="/xbin/busybox cp"
RM="/xbin/busybox rm"
MKDIR="/xbin/busybox mkdir"
CHMOD="/xbin/busybox chmod"
CHOWN="/xbin/busybox chown"
LN="/xbin/busybox ln"
GREP="/xbin/busybox grep"
TAR="/xbin/busybox tar"
SED="/xbin/busybox sed"

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

  log "badboots = $(${GREP} badboots /contents/wampy/config.ini)"
  log "resetting badboots to 0"
  ${SED} -i 's/badboots.*/badboots = 0/g' /contents/wampy/config.ini
  log "badboots = $(${GREP} badboots /contents/wampy/config.ini)"
}

install() {
  log "installing ${BINARY}"
  ${MKDIR} -p ${VENDOR}/bin/
  ${CP} ${BINARY} ${VENDOR}/bin/
  ${CHMOD} 0755 ${VENDOR}/bin/${BINARY}

  log "installing upgtool"
  ${CP} upgtool-linux-arm5 ${VENDOR}/bin/
  ${CHMOD} 0755 ${VENDOR}/bin/upgtool-linux-arm5

  log "installing ${BINARY} service"
  ${CP} "init.${BINARY}.rc" ${INITRD_UNPACKED}/
  ${CHMOD} 0600 "${INITRD_UNPACKED}/init.${BINARY}.rc"
  ${GREP} -q "init.${BINARY}.rc" "${INITRD_UNPACKED}/init.rc"
  if test $? -ne 0; then
    log "adding service"
    echo -e "import init.${BINARY}.rc\n$(cat ${INITRD_UNPACKED}/init.rc)" > "${INITRD_UNPACKED}/init.rc"
  fi

  log "installing libraries"
  ${MKDIR} -p ${VENDOR}/lib/
  ${CP} libMagick++-7.Q8HDRI.so ${VENDOR}/lib/
  ${CP} libMagickCore-7.Q8HDRI.so ${VENDOR}/lib/
  ${CP} libMagickWand-7.Q8HDRI.so ${VENDOR}/lib/
  ${CHMOD} 0700 ${VENDOR}/lib/libMagick*-7.Q8HDRI.so
  ${CHOWN} ${user}:${group} ${VENDOR}/lib/libMagick*-7.Q8HDRI.so

  ${CP} libjpeg.so.62.4.0 ${VENDOR}/lib/
  ${CHMOD} 0700 ${VENDOR}/lib/libjpeg.so.62.4.0
  ${CHOWN} ${user}:${group} ${VENDOR}/lib/libjpeg.so.62.4.0
  ${LN} -s ${VENDOR}/lib/libjpeg.so.62.4.0 ${VENDOR}/lib/libjpeg.so.62
  ${CHOWN} -h ${user}:${group} ${VENDOR}/lib/libjpeg.so.62

  ${CP} libprotobuf.so.32.0.12 ${VENDOR}/lib/
  ${CHMOD} 0700 ${VENDOR}/lib/libprotobuf.so.32.0.12
  ${CHOWN} ${user}:${group} ${VENDOR}/lib/libprotobuf.so.32.0.12
  ${LN} -s ${VENDOR}/lib/libprotobuf.so.32.0.12 ${VENDOR}/lib/libprotobuf.so.32
  ${LN} -s ${VENDOR}/lib/libprotobuf.so.32.0.12 /system/vendor/sony/lib/libprotobuf.so.32
  ${CHOWN} -h ${user}:${group} ${VENDOR}/lib/libprotobuf.so.32
  ${CHOWN} -h ${user}:${group} /system/vendor/sony/lib/libprotobuf.so.32

  log "installing server"
  test -f /system/vendor/sony/plugins/platforms/libqeglfs.so_vendor
  if test $? -ne 0; then
    # make sure this file is from SONY (not linked to libprotobuf)
    ${GREP} -q protobuf /system/vendor/sony/plugins/platforms/libqeglfs.so
    if test $? -eq 1; then
      log "backing up libqeglfs"
      ${CP} -p /system/vendor/sony/plugins/platforms/libqeglfs.so /system/vendor/sony/plugins/platforms/libqeglfs.so_vendor
    else
      log "libqeglfs is linked to libprotobuf, not backing up"
    fi
  else
    log "libqeglfs backup already exists"
  fi

  ${CP} libqeglfs.so /system/vendor/sony/plugins/platforms/
  ${CHOWN} root:shell /system/vendor/sony/plugins/platforms/libqeglfs.so
  ${CHMOD} 0755 /system/vendor/sony/plugins/platforms/libqeglfs.so

  log "installing winamp skin"
  ${MKDIR} -p ${VENDOR}/usr/share/${BINARY}/skins/winamp/
  ${CP} "base-2.91.wsz" ${VENDOR}/usr/share/${BINARY}/skins/winamp/

  log "wiping old cassettes"
  ${RM} -r ${VENDOR}/usr/share/${BINARY}/skins/cassette/

  log "installing cassettes"
  ${MKDIR} -p ${VENDOR}/usr/share/${BINARY}/skins/cassette/
  ${TAR} -C ${VENDOR}/usr/share/${BINARY}/skins/cassette/ -xf cassette.tar.gz

  log "wiping old digital clock"
  ${RM} -r ${VENDOR}/usr/share/${BINARY}/skins/digital_clock/

  log "installing digital clock"
  ${MKDIR} -p ${VENDOR}/usr/share/${BINARY}/skins/digital_clock/
  ${TAR} -C ${VENDOR}/usr/share/${BINARY}/skins/digital_clock/ -xf digital_clock.tar.gz

  log "installing licenses"
  ${MKDIR} -p ${VENDOR}/usr/share/${BINARY}/doc/
  ${CP} LICENSE ${VENDOR}/usr/share/${BINARY}/doc/
  ${CP} LICENSE_3rdparty ${VENDOR}/usr/share/${BINARY}/doc/

  log "installing qr code"
  ${CP} qr.bmp ${VENDOR}/usr/share/${BINARY}/
  ${CP} qrDonate.bmp ${VENDOR}/usr/share/${BINARY}/

  log "installing tunings"
  ${MKDIR} -p ${VENDOR}/usr/share/${BINARY}/sound_settings/
  ${TAR} -C ${VENDOR}/usr/share/${BINARY}/sound_settings/ -xf tunings.tar.gz

  log "installing llusbdac"
  ${MKDIR} -p ${VENDOR}/modules/
  ${CP} llusbdac.ko ${VENDOR}/modules/

  clearBadBoots
}

log "installing $(cat product_info)"

mount -t ext4 -o rw /emmc@android /system

install

sync
umount /system
