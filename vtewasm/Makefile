IMAGE=emscripten/emsdk:4.0.3

build:
	docker run --rm \
	-v $$(pwd)/../:/src \
	-w /src/vtewasm \
	-u $$(id -u):$$(id -g) \
       $(IMAGE) bash -c "make -f Makefile.emscripten"

clean:
	docker run --rm \
	-v $$(pwd)/../:/src \
	-w /src/vtewasm \
	-u $$(id -u):$$(id -g) \
      $(IMAGE) bash -c "make -f Makefile.emscripten clean"

serve:
	$(MAKE) -f Makefile.emscripten serve

.DEFAULT_GOAL := build

.PHONY: build serve clean