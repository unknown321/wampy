IMAGE=wampy-builder-clang
DOCKER=docker run -t --rm
NAME=dmp_feature

docker:
	cat Dockerfile | docker build --progress plain -t $(IMAGE) -

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
			-I/usr/arm-linux-gnueabihf/include/ \
			-Wl,-rpath,. \
			-Wl,-rpath,/system/vendor/sony/lib \
			-shared \
			-o lib$(NAME).so \
			dmp_feature.cpp

push:
	adb wait-for-device shell mount -o remount,rw /system
	adb wait-for-device push lib$(NAME).so /system/vendor/unknown321/lib/

r: build push

all: build

.DEFAULT_GOAL := all