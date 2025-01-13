collect:
	python3 tunings.py

tunings.tar.gz: collect
	tar -czvf tunings.tar.gz master_volume master_volume_dsd tone_control

all: tunings.tar.gz

.DEFAULT_GOAL := all
