INWEBM = $(wildcard *.webm)
INBMP=$(wildcard *.bmp)
OUTPNG=$(subst bmp,png,$(INBMP))

%.bmp:
	ffmpeg -i $(subst _001.bmp,.webm,$@) -r 5 $(subst _001.bmp,_%03d.bmp,$@)

%.png: %.bmp
	convert $< -scale 800x  -crop 800x480+0+0 $@

atlas.pkm: atlas.png
	etc1tool atlas.png

atlas.png:
	montage -mode concatenate  -tile 5x *_0*.png atlas.png	

atlas.txt:
	python3 ./config.py

bmp: $(subst .webm,_001.bmp,$(INWEBM))

png: $(OUTPNG)

default: 
	$(MAKE) bmp
	-rm *_003.bmp *_020.bmp
	$(MAKE) png
	$(MAKE) atlas.pkm
	$(MAKE) atlas.txt

clean:
	rm *.png *.pkm *.bmp *.txt

push:
	adb shell mkdir /contents/wampy/skins/cassette/reel/test/
	adb push atlas.pkm /contents/wampy/skins/cassette/reel/test/
	adb push config.txt /contents/wampy/skins/cassette/reel/test/
	adb push atlas.txt /contents/wampy/skins/cassette/reel/test/

.DEFAULT_GOAL := default
