TAR=shine-3.1.1.tar.gz
URL=https://github.com/toots/shine/releases/download/3.1.1/$(TAR)

PREFIX ?= $(shell pwd)/build-arm

$(TAR):
	wget $(URL)

src/shine-3.1.1/README.md: $(TAR)
	test -d src || mkdir src
	tar -C src -xf $(TAR)
	touch $@

clean:
	rm -rf $(TAR) src/ build-* LICENSE

$(PREFIX)/lib/libshine.so: src/shine-3.1.1/README.md LICENSE
	cd src/shine-3.1.1 && \
	make distclean ; \
	./configure --prefix=$(PREFIX) $(FLAGS) && \
	make && make install

LICENSE:
	cp src/shine-3.1.1/COPYING LICENSE

build: $(PREFIX)/lib/libshine.so

.DEFAULT_GOAL := build
.PHONY: build
