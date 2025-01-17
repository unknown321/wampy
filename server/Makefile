PWD=$(shell pwd)
IMAGE=wampy-builder
QT_DIR=$(PWD)/qt
BUILD_DIR=$(PWD)/build
PLATFORM=armv5-unknown-linux-gnueabihf
SYSROOT=/x-tools/$(PLATFORM)/$(PLATFORM)/sysroot
#DEVICE_SERIAL ?= -s 10458B75388765
ADB=adb $(DEVICE_SERIAL) wait-for-device

GZ=Qt-5.3.2.tar.gz
URL=https://prodgpl.blob.core.windows.net/download/Audio/NW-A57/$(GZ)

$(GZ):
	wget $(URL)

$(QT_DIR)/configure:
	mkdir -p $(QT_DIR)
	tar -C $(QT_DIR) -xf $(GZ)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

prepare: libMali_linux.so $(GZ) $(QT_DIR)/configure $(BUILD_DIR)

clean:
	rm -rf build qt

veryclean: clean
	rm $(GZ)

build/plugins/platforms/libqeglfs.so: prepare
	sed -i 's/SECOND_PASS 1/SECOND_PASS 0/g' $(PLATFORM)/wampy.h
	docker run -it --rm \
		-v $(QT_DIR):/qt \
		-v ./$(PLATFORM):/qt/qtbase/mkspecs/devices/$(PLATFORM) \
		-v $(PWD)/../libs/libMali_linux.so:$(SYSROOT)/lib/libEGL.so \
		-v $(PWD)/../libs/libMali_linux.so:$(SYSROOT)/lib/libGLESv2.so \
		-v $(BUILD_DIR):/build \
		-v $(PWD)/../libs:/libs/ \
		-v $(PWD)/../libs/glad/include/KHR:/libs/KHR \
		-v $(BUILD_DIR):/build \
		-v $(PWD)/../libs/protobuf/:/protobuf/ \
		-v $(PWD)/../src/Version.h:/Version.h \
		-w /qt \
		$(IMAGE) /bin/bash -c "\
			rm -rf ./qtbase/src/plugins/platforms/eglfs/{.obj,.moc,.pch,.rcc,Makefile} ; \
			./configure \
				 -extprefix /build \
				 -device $(PLATFORM) \
				 -opensource \
				 -confirm-license \
				 -nomake examples \
				 -nomake tests \
				 -sysroot $(SYSROOT) \
				 -device-option CROSS_COMPILE=/x-tools/$(PLATFORM)/bin/$(PLATFORM)- \
				 -qt-zlib \
				 -qt-freetype \
				 -qpa eglfs \
				 -opengl es2 \
				 -eglfs \
				 -gui \
				 -widgets \
				 -evdev \
				 -skip qtquickcontrols \
				 -skip qtserialport \
				 -skip qtx11extras \
				 -skip qtgraphicaleffects \
				 -skip qtwebkit \
				 -skip qtsvg \
				 -skip qtwinextras \
				 -skip qtsensors \
				 -skip qtmacextras \
				 -skip qtenginio \
				 -skip qttranslations \
				 -skip qtandroidextras \
				 -skip qtmultimedia \
				 -skip qtactiveqt \
				 -skip qttools \
				 -skip qtwebsockets \
				 -skip qtlocation \
				 -skip qtimageformats \
				 -skip qtxmlpatterns \
				 -skip qtdoc \
				 -skip qtscript \
				 -skip qtquick1 \
				 -skip qtconnectivity \
				 -skip qtwebkit-examples \
				 -no-gif \
				 -no-libpng \
				 -no-libjpeg \
				 -no-openssl \
				 -no-glib \
				 -no-xinput2 \
				 -no-pulseaudio \
				 -no-alsa \
				 -no-nis \
				 -no-cups \
				 -no-iconv \
				 -no-icu \
				 -no-fontconfig \
				 -no-dbus \
				 -no-xcb \
				 -no-slog2 \
				 -no-pps \
				 -no-imf \
				 -no-pch \
				 -no-lgmon \
				 -release \
				 -force-debug-info && \
			make -j6 && \
			make install"

#/qt/qtbase/bin/qmake -o /qt/qtbase/src/plugins/platforms/eglfs/Makefile /qt/qtbase/src/plugins/platforms/eglfs/eglfs.pro

fast: libMali_linux.so
	sed -i 's/SECOND_PASS 0/SECOND_PASS 1/g' $(PLATFORM)/wampy.h
	docker run -it --rm \
		-v $(QT_DIR):/qt \
		-v ./$(PLATFORM):/qt/qtbase/mkspecs/devices/$(PLATFORM) \
		-v $(PWD)/../libs/libMali_linux.so:$(SYSROOT)/lib/libEGL.so \
		-v $(PWD)/../libs/libMali_linux.so:$(SYSROOT)/lib/libGLESv2.so \
		-v $(PWD)/../libs:/libs/ \
		-v $(PWD)/../libs/glad/include/KHR:/libs/KHR \
		-v $(BUILD_DIR):/build \
		-v $(PWD)/../libs/protobuf/:/protobuf/ \
		-v $(PWD)/../src/Version.h:/Version.h \
		-w /qt \
		$(IMAGE) /bin/bash -c "\
			rm -f ./qtbase/plugins/platforms/libqeglfs.so && \
			rm -f ./qtbase/src/plugins/platforms/eglfs/.obj/qeglfshooks_mt8590.o && \
			touch /qt/qtbase/mkspecs/devices/$(PLATFORM)/qmake.conf && \
			./qtbase/bin/moc /qt/qtbase/mkspecs/devices/$(PLATFORM)/Controller.moc -o /qt/qtbase/mkspecs/devices/$(PLATFORM)/moc_Controller.cpp && \
			./qtbase/bin/moc /qt/qtbase/mkspecs/devices/$(PLATFORM)/Provider.moc -o /qt/qtbase/mkspecs/devices/$(PLATFORM)/moc_Provider.cpp && \
			./qtbase/bin/moc /qt/qtbase/mkspecs/devices/$(PLATFORM)/server.moc -o /qt/qtbase/mkspecs/devices/$(PLATFORM)/moc_server.cpp && \
			cd /qt/qtbase/src/plugins/platforms/eglfs && \
			make EGL_LIB='-lQt5Widgets -L/qt/qtdeclarative/lib -lQt5Qml -lQt5Quick -lQt5Network -L/protobuf/build-arm/lib/ -lprotobuf' SECOND_PASS_CFLAGS='-DSECOND_PASS=1' \
		"
	readelf -a ./qt/qtbase/plugins/platforms/libqeglfs.so | grep -q libQt5Widgets

build: build/plugins/platforms/libqeglfs.so fast

release: build push

r: release
	$(ADB) shell logcat -c
	$(ADB) shell hagodaemons.sh restart

libMali_linux.so:
	test -f ../libs/libMali_linux.so

push:
	$(ADB) push $(PWD)/qt/qtbase/plugins/platforms/libqeglfs.so /system/vendor/sony/plugins/platforms/libqeglfs.so
	$(ADB) shell chown root:shell /system/vendor/sony/plugins/platforms/libqeglfs.so
	$(ADB) shell chmod 0755 /system/vendor/sony/plugins/platforms/libqeglfs.so

.DEFAULT_GOAL := build

.PHONY: build r push release fast clean veryclean prepare
