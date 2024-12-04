DOCKER=docker run -it --rm -v `pwd`:`pwd` -w `pwd` nw-crosstool
#DEVICE_SERIAL=-s 10458B75388765
ADB=adb $(DEVICE_SERIAL) wait-for-device
IMAGE=wampy-builder
DOCKER_BUILDER=docker run -it --rm -v `pwd`:`pwd` -w `pwd` $(IMAGE)
PRODUCT=wampy
VENDOR=/system/vendor/unknown321
TAPE_SOURCE_UPG=NW-A100_0003_V4_04_00_NW_WM_FW.UPG
INSTALL=install/arm
UPX=nw-installer/tools/upx/upx/upx
ECHO=/usr/bin/echo

prepare:
	$(MAKE) -C nw-installer/crosstool

docker:
	cat Dockerfile | docker image build -t $(IMAGE) -

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


cassetteunpacker/$(TAPE_SOURCE_UPG):
	test -f $@

cassetteunpacker/res: cassetteunpacker/$(TAPE_SOURCE_UPG)
	$(MAKE) -C cassetteunpacker docker
	$(DOCKER_BUILDER) $(MAKE) -C cassetteunpacker run

nw-installer/installer/userdata.tar: LICENSE_3rdparty qr.bmp
	$(MAKE) -C nw-installer prepare
	cp $(INSTALL)/bin/$(PRODUCT) installer/
	bash -c "cp $(INSTALL)/lib/libMagick{++,Core,Wand}-7.Q8HDRI.so installer/"
	cp $(INSTALL)/lib/libjpeg.so.62.4.0 installer/
	cp $(INSTALL)/lib/libprotobuf.so.32.0.12 installer/
	cp server/qt/qtbase/plugins/platforms/libqeglfs.so installer/
	$(UPX) -qqq --best installer/$(PRODUCT)
	$(DOCKER) /x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-strip installer/lib*
	cp base-2.91.wsz installer/
	tar -C cassetteunpacker/res -cf installer/cassette.tar \
		tape \
		reel
	cp LICENSE installer/
	cp LICENSE_3rdparty installer/
	cp qr.bmp installer/
	tar -C installer -cf nw-installer/installer/userdata.tar \
		init.wampy.rc \
		run.sh \
		libMagick++-7.Q8HDRI.so \
		libMagickCore-7.Q8HDRI.so \
		libMagickWand-7.Q8HDRI.so \
		libjpeg.so.62.4.0 \
		libprotobuf.so.32.0.12 \
		libqeglfs.so \
		base-2.91.wsz \
		cassette.tar \
		LICENSE \
		LICENSE_3rdparty \
		qr.bmp \
		wampy || rm -f nw-installer/installer/userdata.tar
	cat LICENSE LICENSE_3rdparty > nw-installer/installer/windows/LICENSE.txt.user

server:
	$(MAKE) -C server

release-clean:
	$(MAKE) -C nw-installer OUTFILE=$(PRODUCT).exe APPNAME=$(PRODUCT) clean

release: release-clean build-arm cassetteunpacker/res nw-installer/installer/userdata.tar
	$(MAKE) -C nw-installer OUTFILE=$(PRODUCT).exe APPNAME=$(PRODUCT)

profile:
	cd build && \
		timeout 5 $(PRODUCT) ; \
		gprof $(PRODUCT).out > $(PRODUCT).gprof && \
		gprof2dot < $(PRODUCT).gprof | dot -Tsvg -o output.svg && \
		firefox output.svg

profile-arm:
	-@adb pull /tmp/gmon.out
	$(DOCKER) /x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-gprof cmake-build-debug-docker/$(PRODUCT) gmon.out > gmon.prof
	#gprof2dot -s -w -e 1 -n 1 -z "WinampSkin::WinampSkin::Draw()" < gmon.prof | dot -Tsvg -o output.svg
	gprof2dot -s -w -e 1 -n 1 < gmon.prof | dot -Tsvg -o output.svg
	/usr/local/bin/firefox output.svg

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

# https://github.com/fukuchi/libqrencode
qr.bmp:
	@qrencode -o qr.png -m 1 -s 7 https://github.com/unknown321/$(PRODUCT)
	@convert qr.png -type palette qr.bmp
	@rm qr.png

.PHONY: build build-arm docker push profile profile-arm valgrind deps release release-clean LICENSE_3rdparty server
