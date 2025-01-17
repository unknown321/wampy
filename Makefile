DOCKER=docker run -it --rm -v `pwd`:`pwd` -w `pwd` nw-crosstool
#DEVICE_SERIAL=-s 10458B75388765
ADB=adb $(DEVICE_SERIAL) wait-for-device
IMAGE=wampy-builder
IMAGE_DIGITAL_CLOCK=$(IMAGE)-digital-clock
DOCKER_BUILDER=docker run -it --rm -v `pwd`:`pwd` -w `pwd` $(IMAGE)
DOCKER_DIGITAL_CLOCK=docker run -it --rm -v `pwd`:`pwd` -w `pwd` -u `id -u`:`id -g` $(IMAGE_DIGITAL_CLOCK)
PRODUCT=wampy
VENDOR=/system/vendor/unknown321
INSTALL=install/arm
UPX=nw-installer/tools/upx/upx/upx
ECHO=/usr/bin/echo

prepare:
	$(MAKE) -C nw-installer/crosstool

docker: docker_digital_clock
	cat Dockerfile | docker image build -t $(IMAGE) -

docker_digital_clock:
	$(MAKE) -C digital_clock docker

build:
	mkdir -p build && \
	cd build && \
	cmake -DDESKTOP=1 ../  && \
	make && \
	make install && \
	./$(PRODUCT)

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

push:
	timeout 1s $(ADB) shell echo "device connected"
	$(ADB) push install/arm/bin/$(PRODUCT) $(VENDOR)/bin/
	$(ADB) push install/arm/lib/libMagickCore-7.Q8HDRI.so $(VENDOR)/lib/
	$(ADB) push install/arm/lib/libMagickWand-7.Q8HDRI.so $(VENDOR)/lib/
	$(ADB) push install/arm/lib/libMagick++-7.Q8HDRI.so $(VENDOR)/lib/
	$(ADB) push install/arm/lib/libjpeg.so.62.4.0 $(VENDOR)/lib/
	$(ADB) shell ln -s  $(VENDOR)/lib/libjpeg.so.62.4.0 $(VENDOR)/lib/libjpeg.so.62
	$(ADB) push base-2.91.wsz $(VENDOR)/usr/share/skins/winamp/base-2.91.wsz
	$(MAKE) -C server push

nw-installer/installer/userdata.tar.gz: LICENSE_3rdparty qr.bmp qrDonate.bmp
	$(MAKE) -C nw-installer prepare
	cp $(INSTALL)/bin/$(PRODUCT) installer/
	bash -c "cp $(INSTALL)/lib/libMagick{++,Core,Wand}-7.Q8HDRI.so installer/"
	cp $(INSTALL)/lib/libjpeg.so.62.4.0 installer/
	cp $(INSTALL)/lib/libprotobuf.so.32.0.12 installer/
	cp server/qt/qtbase/plugins/platforms/libqeglfs.so installer/
	cp nw-installer/tools/upgtool/upgtool-linux-arm5 installer/
	$(UPX) -qqq --best installer/$(PRODUCT)
	$(UPX) -qqq --best installer/upgtool-linux-arm5
	$(DOCKER) /x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-strip installer/lib*
	cp base-2.91.wsz installer/
	$(MAKE) -C cassette
	cp cassette/cassette.tar.gz installer/
	$(MAKE) -C digital_clock
	cp digital_clock/digital_clock.tar.gz installer/
	$(MAKE) -C tunings
	cp tunings/tunings.tar.gz installer/
	cp libs/llusbdac/llusbdac/llusbdac.ko installer/
	cp LICENSE installer/
	cp LICENSE_3rdparty installer/
	cp qr.bmp installer/
	cp qrDonate.bmp installer/
	echo -n "$(PRODUCT), version " > installer/product_info
	grep VERSION src/Version.h | cut -f 3,4,5 -d " " | sed 's/"//g' >> installer/product_info
	tar -C installer -cf nw-installer/installer/userdata.tar.gz \
		init.wampy.rc \
		run.sh \
		libMagick++-7.Q8HDRI.so \
		libMagickCore-7.Q8HDRI.so \
		libMagickWand-7.Q8HDRI.so \
		libjpeg.so.62.4.0 \
		libprotobuf.so.32.0.12 \
		libqeglfs.so \
		base-2.91.wsz \
		cassette.tar.gz \
		digital_clock.tar.gz \
		tunings.tar.gz \
		llusbdac.ko \
		upgtool-linux-arm5 \
		LICENSE \
		LICENSE_3rdparty \
		qr.bmp \
		qrDonate.bmp \
		product_info \
		wampy || rm -f nw-installer/installer/userdata.tar.gz
	cat LICENSE LICENSE_3rdparty > nw-installer/installer/windows/LICENSE.txt.user

userdata: nw-installer/installer/userdata.tar.gz

server:
	$(MAKE) -C server

nw-installer/installer/userdata.uninstaller.tar.gz:
	$(MAKE) -C nw-installer prepare
	cat LICENSE LICENSE_3rdparty > nw-installer/installer/windows/LICENSE.txt.user
	echo -n "$(PRODUCT), version " > uninstaller/product_info
	grep VERSION src/Version.h | cut -f 3,4,5 -d " " | sed 's/"//g' >> uninstaller/product_info
	tar -C uninstaller -cf nw-installer/installer/userdata.uninstaller.tar.gz \
		product_info \
		run.sh

release-clean:
	$(MAKE) -C nw-installer OUTFILE=$(PRODUCT).exe APPNAME=$(PRODUCT) clean
	-rm -rf release

release: release-clean build-arm server nw-installer/installer/userdata.tar.gz nw-installer/installer/userdata.uninstaller.tar.gz
	# first, build and move uninstaller upgs
	$(MAKE) -C nw-installer OUTFILE=$(PRODUCT).uninstaller.exe APPNAME=$(PRODUCT)-uninstaller A40=0 A30=0 USERDATA_FILENAME=userdata.uninstaller.tar.gz build
	mkdir -p release/uninstaller
	cd nw-installer/installer/nw-a50/ && tar -czvf ../../../release/uninstaller/nw-a50.uninstaller.tar.gz NW_WM_FW.UPG && rm NW_WM_FW.UPG
	cd nw-installer/installer/walkmanOne/ && tar -czvf ../../../release/uninstaller/walkmanOne.uninstaller.tar.gz NW_WM_FW.UPG && rm NW_WM_FW.UPG
	# second, build installer upgs
	$(MAKE) -C nw-installer OUTFILE=$(PRODUCT).exe APPNAME=$(PRODUCT) A40=0 A30=0 build
	# next, build installer (with uninstaller included)
	$(MAKE) -C nw-installer OUTFILE=$(PRODUCT).exe APPNAME=$(PRODUCT) A40=0 A30=0 win
	# finally, move installer upg and exe files
	mkdir -p release/installer/
	cd nw-installer/installer/nw-a50/ && tar -czvf ../../../release/installer/nw-a50.tar.gz NW_WM_FW.UPG
	cd nw-installer/installer/walkmanOne/ && tar -czvf ../../../release/installer/walkmanOne.tar.gz NW_WM_FW.UPG
	mv nw-installer/installer/windows/$(PRODUCT).exe release/installer/$(PRODUCT).$(shell date --iso).$(shell git log -1 --format=%h).exe

# see also: `perf record` && `perf report`
profile:
	cd cmake-build-default && \
		timeout 10 ./$(PRODUCT) ; \
		gprof $(PRODUCT) gmon.out > $(PRODUCT).gprof && \
		gprof2dot < $(PRODUCT).gprof | dot -Tsvg -o output.svg && \
		firefox output.svg

profile-arm:
	-@adb pull /tmp/gmon.out
	$(DOCKER) /x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-gprof cmake-build-debug-docker/$(PRODUCT) gmon.out > gmon.prof
	#gprof2dot -s -w -e 1 -n 1 -z "WinampSkin::WinampSkin::Draw()" < gmon.prof | dot -Tsvg -o output.svg
	gprof2dot -s -w -e 1 -n 1 < gmon.prof | dot -Tsvg -o output.svg
	firefox output.svg

valgrind:
	cd build && valgrind --leak-check=full --read-var-info=yes --read-inline-info=yes --gen-suppressions=yes --suppressions=../suppressions.valgrind -s  ./$(PRODUCT)

LICENSE_3rdparty:
	@$(ECHO) -e "\n***\nprotobuf:\n" > $@
	@cat libs/protobuf/LICENSE >> $@
	@$(ECHO) -e "\n***\nzlib:\n" >> $@
	@cat libs/zlib/LICENSE >> $@
	@$(ECHO) -e "\n***\nmINI:\n" >> $@
	@cat libs/mINI/LICENSE >> $@
	@$(ECHO) -e "\n***\nImageMagick:\n" >> $@
	@cat libs/ImageMagick/LICENSE >> $@
	@$(ECHO) -e "\n***\nMagick++:\n" >> $@
	@cat libs/ImageMagick/Magick++/LICENSE >> $@

	@$(ECHO) -e "\n***\nglfw:\n" >> $@
	@cat libs/glfw/LICENSE.md >> $@
	@$(ECHO) -e "\n***\nlibjpeg-turbo:\n" >> $@
	@cat libs/libjpeg-turbo/LICENSE.md >> $@

	@$(ECHO) -e "\n***\nglm:\n" >> $@
	@cat libs/glm/copying.txt >> $@

	@$(ECHO) -e "\n***\nDear ImGui:\n" >> $@
	@cat libs/imgui/LICENSE.txt >> $@

	@$(ECHO) -e "\n***\nLumixEngine:\n" >> $@
	@cat libs/LumixEngine_LICENSE.txt >> $@

	@$(ECHO) -e "\n***\nLLUSBDAC:\n" >> $@
	@cat libs/llusbdac/LICENSE >> $@

# https://github.com/fukuchi/libqrencode
qr.bmp:
	@qrencode -o qr.png -m 1 -s 7 https://github.com/unknown321/$(PRODUCT)
	@convert qr.png -type palette qr.bmp
	@rm qr.png

qrDonate.bmp:
	@qrencode -o qrDonate.png -m 1 -s 7 https://boosty.to/unknown321/donate
	@convert qrDonate.png -type palette qrDonate.bmp
	@rm qrDonate.png

fastinstall:
	cd release/installer/
	tar -C release/installer -xf release/installer/nw-a50.tar.gz
	adb push release/installer/NW_WM_FW.UPG /contents
	adb shell sync
	adb shell nvpflag fup 0x70555766
	adb shell reboot

.PHONY: build build-arm docker docker_digital_clock push profile profile-arm valgrind deps release release-clean LICENSE_3rdparty server userdata fastinstall
