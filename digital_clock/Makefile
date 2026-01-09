IMAGE_DIGITAL_CLOCK ?= wampy-builder-digital-clock
DOCKER_DIGITAL_CLOCK=docker run -it --rm -v `pwd`:`pwd` -w `pwd` -u `id -u`:`id -g` $(IMAGE_DIGITAL_CLOCK)

prepare:
	cat Dockerfile | docker image build -t $(IMAGE_DIGITAL_CLOCK) -
	wget -O master.zip https://github.com/unknown321/ipod_theme/archive/refs/heads/master.zip
	unzip -o master.zip
	mv ipod_theme-master ipod_theme

docker: prepare

clean:
	rm -rf yellow gold green space_gray silver pink red_product blue_2012 pink_2012 purple blue digital_clock.tar.gz

sort:
	./sort.py

ipod_theme/body/229441876_0064.png:
	$(DOCKER_DIGITAL_CLOCK) bash -c "cd ipod_theme && ./01_firmware_unpack_7g && ./02_art_unpack.py"

yellow/0_big.jpg: ipod_theme/body/229441876_0064.png sort

segments:
	# device colors, HgrmMediaPlayerApp 0x003dc700
	COLOR_BG=DDDDDD STYLE=-Italic STYLE_BOLD=-Italic $(MAKE) -f Makefile-segment segments-default
	COLOR_BG=FFD2B0 STYLE=-Italic STYLE_BOLD=-Italic $(MAKE) -f Makefile-segment segments-peach
	COLOR_BG=FF6757 STYLE=-Italic STYLE_BOLD=-Italic $(MAKE) -f Makefile-segment segments-red
	COLOR_BG=B1CFE5 STYLE=-Italic STYLE_BOLD=-Italic $(MAKE) -f Makefile-segment segments-blue
	COLOR_BG=AED1B3 STYLE=-Italic STYLE_BOLD=-Italic $(MAKE) -f Makefile-segment segments-green

	COLOR_BG=DDDDDD STYLE=-Regular $(MAKE) -f Makefile-segment segments-default-regular
	COLOR_BG=FFD2B0 STYLE=-Regular $(MAKE) -f Makefile-segment segments-peach-regular
	COLOR_BG=FF6757 STYLE=-Regular $(MAKE) -f Makefile-segment segments-red-regular
	COLOR_BG=B1CFE5 STYLE=-Regular $(MAKE) -f Makefile-segment segments-blue-regular
	COLOR_BG=AED1B3 STYLE=-Regular $(MAKE) -f Makefile-segment segments-green-regular

	COLOR_BG=ff8e00 STYLE=-Italic STYLE_BOLD=-Italic $(MAKE) -f Makefile-segment segments-amberlcd
	COLOR_BG=98ab46 STYLE=-Italic STYLE_BOLD=-Italic $(MAKE) -f Makefile-segment segments-greenlcd
	COLOR_BG=878988 STYLE=-Italic STYLE_BOLD=-Italic $(MAKE) -f Makefile-segment segments-greylcd

	COLOR_BG=ff8e00 STYLE=-Regular $(MAKE) -f Makefile-segment segments-amberlcd-regular
	COLOR_BG=98ab46 STYLE=-Regular $(MAKE) -f Makefile-segment segments-greenlcd-regular
	COLOR_BG=878988 STYLE=-Regular $(MAKE) -f Makefile-segment segments-greylcd-regular


digital_clock.tar.gz: yellow/0_big.jpg segments
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
		blue \
		segments-default \
		segments-peach \
		segments-red \
		segments-blue \
		segments-green \
		segments-default-regular \
		segments-peach-regular \
		segments-red-regular \
		segments-blue-regular \
		segments-green-regular \
		segments-amberlcd \
		segments-greenlcd \
		segments-greylcd \
		segments-amberlcd-regular \
		segments-greenlcd-regular \
		segments-greylcd-regular
	tar --list -f digital_clock.tar.gz blue/5_medium.jpg

run: digital_clock.tar.gz

.DEFAULT_GOAL := run
