#!/bin/sh
VENDOR=/system/vendor/unknown321/
BINARY=wampy
user=system
group=system

RM="/xbin/busybox rm"
GREP="/xbin/busybox grep"
CP="/xbin/busybox cp"
SED="/xbin/busybox sed"
CHMOD="/xbin/busybox chmod"
CHOWN="/xbin/busybox chown"

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

  ${RM} -f ${VENDOR}/lib/libshine.so.3

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

  log "removing skins, licenses, qr code, tunings, icons, localization files"
  ${RM} -rf ${VENDOR}/usr/share/${BINARY}/

  log "removing locales"
  ${RM} -rf /system/usr/lib/locale/

  log "removing modules"
  ${RM} -rf ${VENDOR}/modules/

  ${GREP} -q "libsound_service_fw.so" ${INITRD_UNPACKED}/init.hagoromo.rc
  if test $? -eq 0; then
    log "removing LD_PRELOAD entry for SoundServiceFw"
    ${SED} -i '/libsound_service_fw.so/d' ${INITRD_UNPACKED}/init.hagoromo.rc
  fi

  log "removing libsound_service_fw"
  ${RM} -f ${VENDOR}/lib/libsound_service_fw.so

  ${GREP} -q "libdmp_feature.so" ${INITRD_UNPACKED}/init.hagoromo.rc
  if test $? -eq 0; then
    log "removing LD_PRELOAD entry for PlayerService"
    ${SED} -i '/libdmp_feature.so/d' ${INITRD_UNPACKED}/init.hagoromo.rc
  fi

  log "removing libdmp_feature.so"
  ${RM} -f ${VENDOR}/lib/libdmp_feature.so

  test -f /system/vendor/sony/lib/libAudioAnalyzerService.so_vendor
  if busybox test $? -eq 0; then
    log "restoring AudioAnalyzerService from backup"
    ${CP} -fp /system/vendor/sony/lib/libAudioAnalyzerService.so_vendor /system/vendor/sony/lib/libAudioAnalyzerService.so
    ${CHMOD} 0755 /system/vendor/sony/lib/libAudioAnalyzerService.so
    ${CHOWN} root:shell /system/vendor/sony/lib/libAudioAnalyzerService.so
  fi

  test -f /system/vendor/sony/lib/libTunerPlayerService.so_vendor
  if busybox test $? -eq 0; then
    log "restoring libTunerPlayerService.so from backup"
    ${CP} -f /system/vendor/sony/lib/libTunerPlayerService.so_vendor /system/vendor/sony/lib/libTunerPlayerService.so
    ${CHMOD} 0755 /system/vendor/sony/lib/libTunerPlayerService.so
    ${CHOWN} root:shell /system/vendor/sony/lib/libTunerPlayerService.so
  fi

  test -f /system/vendor/sony/bin/HgrmMediaPlayerApp_vendor
  if busybox test $? -eq 0; then
    log "restoring HgrmMediaPlayerApp from backup"
    ${CP} -f /system/vendor/sony/bin/HgrmMediaPlayerApp_vendor /system/vendor/sony/bin/HgrmMediaPlayerApp
    ${CHMOD} 0755 /system/vendor/sony/bin/HgrmMediaPlayerApp
    ${CHOWN} root:shell /system/vendor/sony/bin/HgrmMediaPlayerApp
  fi
}

log "uninstaller for $(cat product_info)"

mount -t ext4 -o rw /emmc@android /system

uninstall

sync
umount /system
