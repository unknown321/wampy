INDIR=../cassetteunpacker/res
OUTFILE=../installer/cassette.tar

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

cassette.tar.gz:
	find $(INDIR) \( -name "*.pkm" -o -name "*.txt" \) | sed 's|$(INDIR)/||g' |  tar -C $(INDIR) -czvf cassette.tar.gz -T -

build: $(reels_pkms) $(tapes_pkms) $(reel_atlas_config) cassette.tar.gz

clean:
	find $(INDIR) \( -name "*.pkm" -o -name "*.png" -o -name "*.pkm" -o -name atlas.txt \) -delete
	rm cassette.tar.gz

.DEFAULT_GOAL := build
.PHONY: clean build