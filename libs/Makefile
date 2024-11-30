# https://registry.khronos.org/OpenGL/api/

DOCKER=docker run -it --rm -v `pwd`:`pwd` -w `pwd` wampy-builder

ARM_CC=CC=/x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-gcc
ARM_CXX=CXX=/x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-g++
ARM_HOST=--host arm-unknown-linux-androideabi

ARM_OPTS=$(ARM_HOST) \
         $(ARM_CC) \
         $(ARM_CXX)

patch:
	cd imgui && git apply ../imgui.patch
	cd glfw && git apply ../glfw.patch

alsa-lib/src:
	make -C alsa-lib

ImageMagick/build-x86:
	$(DOCKER) bash -c "\
	cd ImageMagick && \
	./configure \
		--without-x \
		--without-zlib \
		--with-quantum-depth=8 \
		--prefix=$$(pwd)/ImageMagick/build-x86 \
		--enable-shared=yes\
		--enable-static=no \
		--disable-docs \
		CFLAGS=\" \
			-DMAGICKCORE_HDRI_ENABLE=1 \
			-DMAGICKCORE_CHANNEL_MASK_DEPTH=32 \
			-I../libjpeg-turbo/build-x86/release/include/ \
		\" \
		LDFLAGS=\"-L../libjpeg-turbo/build-x86/release/lib \" && \
	make && \
	make install"

ImageMagick/build-arm:
	$(DOCKER) bash -c "\
	cd ImageMagick && \
	./configure \
		--without-x \
		--without-zlib \
		--with-quantum-depth=8 \
		--prefix=$$(pwd)/ImageMagick/build-arm \
		--enable-shared=yes \
		--enable-static=no \
		--disable-docs \
		$(ARM_OPTS) \
		CFLAGS=\" \
			-DMAGICKCORE_HDRI_ENABLE=1 \
			-DMAGICKCORE_CHANNEL_MASK_DEPTH=32 \
			-I../libjpeg-turbo/build-arm/release/include/ \
		\" \
		LDFLAGS=-L../libjpeg-turbo/build-arm/release/lib && \
	make && \
	make install"


libjpeg-turbo/build-x86:
	mkdir -p $@
	$(DOCKER) bash -c "\
		cd $@ && \
		cmake -DCMAKE_INSTALL_PREFIX=./release .. && \
		make && make install \
"

libjpeg-turbo/build-arm:
	mkdir -p $@
	$(DOCKERCMAKE) bash -c "\
		cd $@ && \
		$(ARM_CC) $(ARM_CXX) cmake -DCMAKE_INSTALL_PREFIX=./release .. && \
		make && make install \
"

libEGL.so: libMali_linux.so
	ln -s libMali_linux.so libEGL.so

libGLESv2.so: libMali_linux.so
	ln -s libMali_linux.so libGLESv2.so

libMali_linux.so:
	test -f $@

libjpeg: libjpeg-turbo/build-arm libjpeg-turbo/build-x86

magick: ImageMagick/build-x86 ImageMagick/build-arm

protobuf:
	$(DOCKERCMAKE) bash -c "\
		make -f Makefile.protobuf \
"

all: patch libEGL.so libGLESv2.so magick libjpeg protobuf

clean:
	rm -rf libjpeg-turbo/build-*
	rm -rf ImageMagick/build-*
	rm -rf protobuf/build-*
	make -C protobuf clean
	make -C ImageMagick clean


.DEFAULT_GOAL := all

.PHONY: libjpeg-turbo/build-arm libjpeg-turbo/build-x86 protobuf patch
