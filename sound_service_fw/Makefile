IMAGE=wampy-builder-clang
DOCKER=docker run -t --rm

docker:
	cat Dockerfile | docker build --progress plain -t $(IMAGE) -

client:
	$(DOCKER) \
		-v `pwd`/..:`pwd`/.. \
		-v `pwd`/../libs/sysroot/:/sysroot \
		-w `pwd` \
		$(IMAGE) \
			clang++ \
			-v \
			-mthumb \
			-target arm-linux-gnueabihf \
			--sysroot=/sysroot \
			-std=c++11 \
			-stdlib=libc++ \
			-ldl \
			-lrt \
			-lpthread \
			-lcxxrt \
			-I/usr/arm-linux-gnueabihf/include/ \
			-Wl,-rpath,. \
			-Wl,-rpath,/system/vendor/sony/lib \
			-o client \
			client.cpp

build:
	$(DOCKER) \
		-v `pwd`:`pwd` \
		-v `pwd`/../libs/sysroot/:/sysroot \
		-w `pwd` \
		$(IMAGE) \
			clang++ \
			-v \
			-target arm-linux-gnueabihf \
			--sysroot=/sysroot \
			-std=c++11 \
			-stdlib=libc++ \
			-fpic \
			-ldl \
			-lrt \
			-lpthread \
			-lcxxrt \
			-lSoundServiceFw \
			-I/usr/arm-linux-gnueabihf/include/ \
			-Wl,-rpath,. \
			-Wl,-rpath,/system/vendor/sony/lib \
			-shared \
			-o libsound_service_fw.so \
			sound_service_fw.cpp

push:
	adb wait-for-device shell mount -o remount,rw /system
	adb wait-for-device push libsound_service_fw.so /system/vendor/unknown321/lib/
	adb wait-for-device push client /tmp

r: build client push

all: build

.DEFAULT_GOAL := all
.PHONY: client