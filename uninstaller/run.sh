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

uninstall() {
  log "removing ${BINARY}"
  busybox rm -f ${VENDOR}/bin/${BINARY}

  log "removing upgtool"
  busybox rm -f ${VENDOR}/bin/upgtool-linux-arm5

  log "uninstalling ${BINARY} service"
  grep -q "init.${BINARY}.rc" "${INITRD_UNPACKED}/init.rc"
  if test $? -eq 0; then
    log "removing service"
    busybox sed -i "/import init.${BINARY}.rc/d" ${INITRD_UNPACKED}/init.rc
  fi
  busybox rm -f ${INITRD_UNPACKED}/init.${BINARY}.rc

  log "removing libraries"
  busybox rm -f ${VENDOR}/lib/libMagick++-7.Q8HDRI.so
  busybox rm -f ${VENDOR}/lib/libMagickCore-7.Q8HDRI.so
  busybox rm -f ${VENDOR}/lib/libMagickWand-7.Q8HDRI.so

  busybox rm -f ${VENDOR}/lib/libjpeg.so.62.4.0
  busybox rm -f ${VENDOR}/lib/libjpeg.so.62

  log "uninstalling server"
  busybox test -f /system/vendor/sony/plugins/platforms/libqeglfs.so_vendor
  if busybox test $? -eq 0; then
    # make sure backup is from SONY (not linked to libprotobuf)
    busybox grep -q protobuf /system/vendor/sony/plugins/platforms/libqeglfs.so_vendor
    if busybox test $? -eq 1; then
      log "restoring libqeglfs from backup"
      busybox cp -f /system/vendor/sony/plugins/platforms/libqeglfs.so_vendor /system/vendor/sony/plugins/platforms/libqeglfs.so
      chown root:shell /system/vendor/sony/plugins/platforms/libqeglfs.so
      chmod 0755 /system/vendor/sony/plugins/platforms/libqeglfs.so

      log "removing protobuf lib"
      busybox rm -f ${VENDOR}/lib/libprotobuf.so.32.0.12
      busybox rm -f ${VENDOR}/lib/libprotobuf.so.32
      busybox rm -f /system/vendor/sony/lib/libprotobuf.so.32
    else
      log "backup file is linked to libprotobuf, not restoring"
    fi
  else
    log "no valid libqeglfs backup found, leaving as is"
  fi

  log "removing skins, licenses, qr code, tunings"
  busybox rm -rf ${VENDOR}/usr/share/${BINARY}/

  log "removing modules"
  busybox rm -rf ${VENDOR}/modules/
}

log "uninstaller for $(cat product_info)"

mount -t ext4 -o rw /emmc@android /system

uninstall

sync
umount /system
