#!/bin/sh
VENDOR=/system/vendor/unknown321/
BINARY=wampy
user=system
group=system

RM="/xbin/busybox rm"
GREP="/xbin/busybox grep"
CP="/xbin/busybox cp"
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

uninstall() {
  log "removing ${BINARY}"
  ${RM} -f ${VENDOR}/bin/${BINARY}

  log "removing pstserver"
  ${RM} -f ${VENDOR}/bin/pstserver

  log "removing upgtool"
  ${RM} -f ${VENDOR}/bin/upgtool-linux-arm5

  log "uninstalling ${BINARY} service"
  ${GREP} -q "init.${BINARY}.rc" "${INITRD_UNPACKED}/init.rc"
  if test $? -eq 0; then
    log "removing service"
    ${SED} -i "/import init.${BINARY}.rc/d" ${INITRD_UNPACKED}/init.rc
  fi
  ${RM} -f ${INITRD_UNPACKED}/init.${BINARY}.rc

  log "removing libraries"
  ${RM} -f ${VENDOR}/lib/libMagick++-7.Q8HDRI.so
  ${RM} -f ${VENDOR}/lib/libMagickCore-7.Q8HDRI.so
  ${RM} -f ${VENDOR}/lib/libMagickWand-7.Q8HDRI.so

  ${RM} -f ${VENDOR}/lib/libjpeg.so.62.4.0
  ${RM} -f ${VENDOR}/lib/libjpeg.so.62

  log "uninstalling server"
  busybox test -f /system/vendor/sony/plugins/platforms/libqeglfs.so_vendor
  if busybox test $? -eq 0; then
    # make sure backup is from SONY (not linked to libprotobuf)
    ${GREP} -q protobuf /system/vendor/sony/plugins/platforms/libqeglfs.so_vendor
    if busybox test $? -eq 1; then
      log "restoring libqeglfs from backup"
      ${CP} -f /system/vendor/sony/plugins/platforms/libqeglfs.so_vendor /system/vendor/sony/plugins/platforms/libqeglfs.so
      chown root:shell /system/vendor/sony/plugins/platforms/libqeglfs.so
      chmod 0755 /system/vendor/sony/plugins/platforms/libqeglfs.so

      log "removing protobuf lib"
      ${RM} -f ${VENDOR}/lib/libprotobuf.so.32.0.12
      ${RM} -f ${VENDOR}/lib/libprotobuf.so.32
      ${RM} -f /system/vendor/sony/lib/libprotobuf.so.32
    else
      log "backup file is linked to libprotobuf, not restoring"
    fi
  else
    log "no valid libqeglfs backup found, leaving as is"
  fi

  log "removing skins, licenses, qr code, tunings"
  ${RM} -rf ${VENDOR}/usr/share/${BINARY}/

  log "removing modules"
  ${RM} -rf ${VENDOR}/modules/
}

log "uninstaller for $(cat product_info)"

mount -t ext4 -o rw /emmc@android /system

uninstall

sync
umount /system
