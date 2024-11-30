TAR=alsa-lib-1.0.29.tar.gz
URL=https://prodgpl.blob.core.windows.net/download/Audio/NW-A57/$(TAR)

$(TAR):
	wget $(URL)

src/INSTALL: $(TAR)
	mkdir -p src
	tar -C src -xf $(TAR)

clean:
	rm -rf $(TAR) src/

.DEFAULT_GOAL := src/INSTALL
