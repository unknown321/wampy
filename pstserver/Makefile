IMAGE=wampy-builder-clang
DOCKER=docker run -t --rm

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
			-mthumb \
			-target arm-linux-gnueabihf \
			--sysroot=/sysroot \
			-std=c++11 \
			-stdlib=libc++ \
			-ldl \
			-lrt \
			-lpthread \
			-lcxxrt \
			-lpstcore \
			-lDmpConfig \
			-lDmpFeature \
			-lEffectCtrlDmp \
			-lPlayerServiceClient \
			-lConfigurationService \
			-lTunerPlayerService \
			-I/usr/arm-linux-gnueabihf/include/ \
			-Wl,-rpath,. \
			-Wl,-rpath,/system/vendor/sony/lib \
			-o pstserver \
			main.cpp

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

test:
	$(DOCKER) \
		-v `pwd`:`pwd` \
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
			-g \
			-lpthread \
			-lcxxrt \
			-lpstcore \
			-lDmpConfig \
			-lDmpFeature \
			-lEffectCtrlDmp \
			-lPlayerServiceClient \
			-lConfigurationService \
			-lSoundServiceFw \
			-lTunerPlayerService \
			-I/usr/arm-linux-gnueabihf/include/ \
			-Wl,-rpath,. \
			-Wl,-rpath,/system/vendor/sony/lib \
			-o test \
			test.cpp
		adb push test /tmp

push:
	adb wait-for-device push pstserver /tmp
	adb wait-for-device push client /tmp

r: build client push

all: build

.DEFAULT_GOAL := all
.PHONY: client test