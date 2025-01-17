# https://registry.khronos.org/OpenGL/api/

IMAGE=wampy-builder
DOCKER=docker run -it --rm -v `pwd`:`pwd` -w `pwd` $(IMAGE)

ARM_CC=CC=/x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-gcc
ARM_CXX=CXX=/x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-g++
ARM_HOST=--host arm-unknown-linux-androideabi

ARM_OPTS=$(ARM_HOST) \
         $(ARM_CC) \
         $(ARM_CXX)

patch:
	-cd imgui && git apply ../imgui.patch
	-cd glfw && git apply ../glfw.patch

alsa-lib/src:
	make -C alsa-lib

ImageMagick/build-x86: IMAGE=nw-crosstool
ImageMagick/build-x86:
	$(DOCKER) bash -c "\
	cd ImageMagick && \
	PKG_CONFIG_PATH=../libjpeg-turbo/build-x86/release/lib/pkgconfig/ \
	CFLAGS=\" \
		-DMAGICKCORE_HDRI_ENABLE=1 \
		-DMAGICKCORE_CHANNEL_MASK_DEPTH=32 \
		-I../libjpeg-turbo/build-x86/release/include/ \
	\" \
	LDFLAGS=-L../libjpeg-turbo/build-x86/release/lib \
	./configure \
		--without-x \
		--without-zlib \
		--with-quantum-depth=8 \
		--prefix=$$(pwd)/ImageMagick/build-x86 \
		--enable-shared=yes\
		--enable-static=no \
		--disable-docs \
	&& \
	make && \
	make install"

ImageMagick/build-arm: IMAGE=nw-crosstool
ImageMagick/build-arm:
	$(DOCKER) bash -c "\
	cd ImageMagick && \
	CFLAGS=\" \
		-DMAGICKCORE_HDRI_ENABLE=1 \
		-DMAGICKCORE_CHANNEL_MASK_DEPTH=32 \
		-I../libjpeg-turbo/build-arm/release/include/ \
	\" \
	LDFLAGS=-L../libjpeg-turbo/build-arm/release/lib \
	./configure \
		--without-x \
		--without-zlib \
		--with-quantum-depth=8 \
		--prefix=$$(pwd)/ImageMagick/build-arm \
		--enable-shared=yes \
		--enable-static=no \
		--disable-docs \
		$(ARM_OPTS) && \
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
	$(DOCKER) bash -c "\
		cd $@ && \
		$(ARM_CC) $(ARM_CXX) cmake -DCMAKE_INSTALL_PREFIX=./release .. && \
		make && make install \
"

sqlite/build-arm:
	mkdir -p $@
	$(DOCKER) bash -c "\
		cd $@ && \
		../configure $(ARM_CC) $(ARM_CXX) --host arm-unknown-linux --prefix=`pwd`/$@/install  && \
		 make && make install && patchelf --set-soname libsqlite3.so `pwd`/$@/install/lib/libsqlite3.so \
	"

sqlite/build-x86:
	mkdir -p $@
	$(DOCKER) bash -c "\
		cd $@ && \
		../configure --prefix=`pwd`/$@/install  && \
		make && make install && patchelf --set-soname libsqlite3.so `pwd`/$@/install/lib/libsqlite3.so \
	"

libEGL.so: libMali_linux.so
	ln -s libMali_linux.so libEGL.so

libGLESv2.so: libMali_linux.so
	ln -s libMali_linux.so libGLESv2.so

libMali_linux.so:
	test -f $@

libjpeg: libjpeg-turbo/build-arm libjpeg-turbo/build-x86

magick: ImageMagick/build-x86 ImageMagick/build-arm

sqlite: sqlite/build-arm sqlite/build-x86

protobuf:
	$(DOCKER) bash -c "\
		make -f Makefile.protobuf \
"

llusbdac:
	$(MAKE)	-f Makefile.llusbdac

all: patch libEGL.so libGLESv2.so libjpeg magick protobuf llusbdac sqlite

clean:
	rm -rf libjpeg-turbo/build-*
	rm -rf ImageMagick/build-*
	rm -rf protobuf/build-*
	-make -C protobuf clean
	-make -C ImageMagick clean
	rm -rf sqlite/build-*


.DEFAULT_GOAL := all

.PHONY: libjpeg-turbo/build-arm libjpeg-turbo/build-x86 protobuf patch llusbdac sqlite
