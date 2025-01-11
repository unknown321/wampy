IMAGE_DIGITAL_CLOCK ?= wampy-builder-digital-clock
DOCKER_DIGITAL_CLOCK=docker run -it --rm -v `pwd`:`pwd` -w `pwd` -u `id -u`:`id -g` $(IMAGE_DIGITAL_CLOCK)

prepare:
	cat Dockerfile | docker image build -t $(IMAGE_DIGITAL_CLOCK) -

docker: prepare

colors = yellow gold green space_gray silver pink red_product blue_2012 pink_2012 purple blue
colors_pkms = $(addsuffix /atlas.pkm, $(colors))
colors_config = $(addsuffix /atlas.txt, $(colors))

%/atlas.pkm:
	montage -mode concatenate -tile 2x $(@D)/*_big.png PNG32:$(@D)/big_atlas.png
	montage -mode concatenate -tile 5x $(@D)/*_medium.png PNG32:$(@D)/medium_atlas.png
	montage -mode concatenate -tile 5x $(@D)/*_small.png PNG32:$(@D)/small_atlas.png
	montage -mode concatenate -tile 5x $(@D)/day_*.png PNG32:$(@D)/day_atlas.png
	montage -mode concatenate -tile 4x $(@D)/dot.png $(@D)/minus.png $(@D)/shoe.png $(@D)/colon.png PNG32:$(@D)/misc_atlas.png
	montage -mode concatenate -tile 1x $(@D)/*_atlas.png PNG32:$(@D)/atlas.png
	etc1tool $(@D)/atlas.png

%/atlas.txt:
	echo $@

clean:
	rm -rf $(colors) digital_clock.tar.gz

sort:
	./sort.py

ipod_theme/body/229441876_0064.png:
	$(DOCKER_DIGITAL_CLOCK) bash -c "cd ipod_theme && ./01_firmware_unpack_7g && ./02_art_unpack.py"

yellow/0_big.png: ipod_theme/body/229441876_0064.png sort

digital_clock.tar.gz: yellow/0_big.png $(colors_pkms) $(colors_config)
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

run: digital_clock.tar.gz

.DEFAULT_GOAL := run
