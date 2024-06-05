.PHONY: default
default: build

build/build.ninja:
	cmake --preset dev

.PHONY: build
build: build/build.ninja
	cmake --build build

.PHONY: test
test: build
	ctest --test-dir build -j $$(nproc)

.PHONY: clean
clean:
	rm -rf build
