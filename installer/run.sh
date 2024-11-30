#!/bin/sh
VENDOR=/system/vendor/unknown321/
BINARY=wampy

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

install() {
  log "installing ${BINARY}"
  mkdir -p ${VENDOR}/bin/
  cp ${BINARY} ${VENDOR}/bin/
  chmod 0744 ${VENDOR}/bin/${BINARY}

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
  chown root:root ${VENDOR}/lib/libMagick*-7.Q8HDRI.so

  cp libjpeg.so.62.4.0 ${VENDOR}/lib/
  chmod 0700 ${VENDOR}/lib/libjpeg.so.62.4.0
  chown root:root ${VENDOR}/lib/libjpeg.so.62.4.0
  ln -s ${VENDOR}/lib/libjpeg.so.62.4.0 ${VENDOR}/lib/libjpeg.so.62

  cp libprotobuf.so.32.0.12 ${VENDOR}/lib/
  chmod 0700 ${VENDOR}/lib/libprotobuf.so.32.0.12
  chown root:root ${VENDOR}/lib/libprotobuf.so.32.0.12
  ln -s ${VENDOR}/lib/libprotobuf.so.32.0.12 ${VENDOR}/lib/libprotobuf.so.32
  ln -s ${VENDOR}/lib/libprotobuf.so.32.0.12 /system/vendor/sony/lib/libprotobuf.so.32

  log "installing server"
  cp libqeglfs.so /system/vendor/sony/plugins/platforms/
  chown root:shell /system/vendor/sony/plugins/platforms/libqeglfs.so
  chmod 0755 /system/vendor/sony/plugins/platforms/libqeglfs.so

  log "installing winamp skin"
  mkdir -p ${VENDOR}/usr/share/skins/winamp/
  cp "base-2.91.wsz" ${VENDOR}/usr/share/skins/winamp/

  log "installing cassetes"
  mkdir -p ${VENDOR}/usr/share/skins/cassette/
  tar -C ${VENDOR}/usr/share/skins/cassette/ -xf cassette.tar
}

mount -t ext4 -o rw /emmc@android /system

install

sync
umount /system
