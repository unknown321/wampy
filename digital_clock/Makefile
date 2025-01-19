IMAGE_DIGITAL_CLOCK ?= wampy-builder-digital-clock
DOCKER_DIGITAL_CLOCK=docker run -it --rm -v `pwd`:`pwd` -w `pwd` -u `id -u`:`id -g` $(IMAGE_DIGITAL_CLOCK)

prepare:
	cat Dockerfile | docker image build -t $(IMAGE_DIGITAL_CLOCK) -

docker: prepare

clean:
	rm -rf yellow gold green space_gray silver pink red_product blue_2012 pink_2012 purple blue digital_clock.tar.gz

sort:
	./sort.py

ipod_theme/body/229441876_0064.png:
	$(DOCKER_DIGITAL_CLOCK) bash -c "cd ipod_theme && ./01_firmware_unpack_7g && ./02_art_unpack.py"

yellow/0_big.jpg: ipod_theme/body/229441876_0064.png sort

digital_clock.tar.gz: yellow/0_big.jpg
	tar -czf digital_clock.tar.gz \
		yellow \
		gold \
		green \
		space_gray \
		silver \
		red_product \
		blue_2012 \
		pink_2012 \
		purple \
		pink \
		blue
	tar --list -f digital_clock.tar.gz blue/5_medium.jpg

run: digital_clock.tar.gz

.DEFAULT_GOAL := run
