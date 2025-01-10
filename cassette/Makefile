INDIR=./cassetteunpacker/res
OUTFILE=cassette.tar.gz

# digital clock image has imagemagick, python3 and etc1tool
IMAGE_CASSETTE ?= wampy-builder-digital-clock
DOCKER_CASSETTE=docker run -it --rm -v `pwd`:`pwd` -w `pwd` -u `id -u`:`id -g` $(IMAGE_CASSETTE)

# e2tools is present in wampy-builder image
IMAGE ?= wampy-builder
DOCKER_BUILDER ?= docker run -it --rm -v `pwd`:`pwd` -w `pwd` $(IMAGE)

TAPE_SOURCE_UPG=NW-A100_0003_V4_04_00_NW_WM_FW.UPG
TAPE_SOURCE_UPG_URL=https://info.update.sony.net/PA001/NW-A100Series_0003/contents/0013/$(TAPE_SOURCE_UPG)

cassetteunpacker/$(TAPE_SOURCE_UPG):
	curl --output-dir cassetteunpacker -O $(TAPE_SOURCE_UPG_URL)

cassetteunpacker/res: cassetteunpacker/$(TAPE_SOURCE_UPG)
	$(MAKE) -C cassetteunpacker docker
	$(DOCKER_BUILDER) $(MAKE) -C cassetteunpacker run

reels = chf metal_master other
tapes = ahf bhf chf duad jhf metal metal_master ucx ucx_s

%/atlas.pkm:
	montage -mode concatenate -tile 4x $(@D)/*.jpg $(@D)/atlas.png
	etc1tool $(@D)/atlas.png

%/tape.pkm:
	convert $(@D)/*.jpg $(@D)/tape.png
	etc1tool $(@D)/tape.png

%/atlas.txt:
	echo $@
	python3 atlas_config.py $(@D)

reels_pkms = $(addsuffix /atlas.pkm, $(addprefix $(INDIR)/reel/,$(reels)))
tapes_pkms = $(addsuffix /tape.pkm, $(addprefix $(INDIR)/tape/,$(tapes)))
reel_atlas_config = $(addsuffix /atlas.txt, $(addprefix $(INDIR)/reel/,$(reels)))

$(OUTFILE):
	find $(INDIR) \( -name "*.pkm" -o -name "*.txt" \) | sed 's|$(INDIR)/||g' |  tar -C $(INDIR) -czvf $(OUTFILE) -T -

build:  $(reels_pkms) $(tapes_pkms) $(reel_atlas_config) $(OUTFILE)

clean:
	find $(INDIR) \( -name "*.pkm" -o -name "*.png" -o -name "*.pkm" -o -name atlas.txt \) -delete
	rm -f $(OUTFILE)

run: cassetteunpacker/res
	$(DOCKER_CASSETTE) make build

.DEFAULT_GOAL := run
.PHONY: clean build run