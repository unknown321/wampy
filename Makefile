DOCKER=docker run -it --rm -v `pwd`:`pwd` -w `pwd` toolchain
#DEVICE_SERIAL=-s 10458B75388765
ADB=adb $(DEVICE_SERIAL) wait-for-device
IMAGE=wampy-builder
NAME=wampy

TAPE_SOURCE_UPG=NW-A100_0003_V4_04_00_NW_WM_FW.UPG

docker:
	cat Dockerfile | docker image build -t $(IMAGE) -

build:
	mkdir -p build && \
	cd build && \
	cmake -DDESKTOP=1 ../  && \
	make && \
	make install && \
	./$(NAME)

build-arm:
	docker run -it --rm \
		-v `pwd`:`pwd` -w `pwd` \
		$(IMAGE) bash -c " \
			mkdir -p cmake-build-debug-docker && \
			cd cmake-build-debug-docker && \
			CC=/x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-gcc \
			CXX=/x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-g++ \
			cmake .. && \
			make && \
			make install"

deps:
	make -C libs

VENDOR=/system/vendor/unknown321
push:
	timeout 1s $(ADB) shell echo "device connected"
	$(ADB) push install/arm/bin/$(NAME) $(VENDOR)/bin/
	$(ADB) push install/arm/lib/libMagickCore-7.Q8HDRI.so $(VENDOR)/lib/
	$(ADB) push install/arm/lib/libMagickWand-7.Q8HDRI.so $(VENDOR)/lib/
	$(ADB) push install/arm/lib/libMagick++-7.Q8HDRI.so $(VENDOR)/lib/
	$(ADB) push install/arm/lib/libjpeg.so.62.4.0 $(VENDOR)/lib/
	$(ADB) shell ln -s  $(VENDOR)/lib/libjpeg.so.62.4.0 $(VENDOR)/lib/libjpeg.so.62
	$(ADB) push skins/base-2.91.wsz $(VENDOR)/usr/share/skins/winamp/base-2.91.wsz
	$(MAKE) -C server push
#	cp /media/ssd/dev/imgui/libs/qt/qt/qtbase/plugins/platforms/libqeglfs.so /media/ssd/walkman/NW-A50/toolchain/sysroot/system/vendor/sony/plugins/platforms/libqeglfs.so
#	cp /media/ssd/dev/imgui/libs/qt/qt/qtbase/lib/libQt5Core.so.5.3.2 /media/ssd/walkman/NW-A50/toolchain/sysroot/system/vendor/sony/lib/libQt5Core.so.5
#	$(ADB) push libs/qt/build/plugins/platforms/libqeglfs.so /system/vendor/sony/plugins/platforms/libqeglfs.so

image:
	/tmp/magick convert /tmp/MAIN.BMP -compress None BMP3:/tmp/out.bmp

profile:
	cd build && \
		timeout 5 $(NAME) ; \
		gprof $(NAME).out > $(NAME).gprof && \
		gprof2dot < $(NAME).gprof | dot -Tsvg -o output.svg && \
		firefox output.svg

profile-arm:
	-@adb pull /tmp/gmon.out
	$(DOCKER) /x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-gprof cmake-build-debug-docker/$(NAME) gmon.out > gmon.prof
	#gprof2dot -s -w -e 1 -n 1 -z "WinampSkin::WinampSkin::Draw()" < gmon.prof | dot -Tsvg -o output.svg
	gprof2dot -s -w -e 1 -n 1 < gmon.prof | dot -Tsvg -o output.svg
	/usr/local/bin/firefox output.svg

cassetteunpacker/$(TAPE_SOURCE_UPG):
	test -f $@

cassetteunpacker/res: cassetteunpacker/$(TAPE_SOURCE_UPG)
	$(MAKE) -C cassetteunpacker docker run

valgrind:
	cd build && valgrind --leak-check=full --read-var-info=yes --read-inline-info=yes --gen-suppressions=yes --suppressions=../suppressions.valgrind -s  ./$(NAME)


.PHONY: build push profile prepare imagemagick
