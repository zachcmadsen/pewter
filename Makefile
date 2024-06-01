.PHONY: default
default: build

build/build.ninja:
	cmake -S . -B build

.PHONY: build
build: build/build.ninja
	cmake --build build

.PHONY: test
test: build
	ctest --test-dir build -j $$(nproc)

.PHONY: clean
clean:
	rm -rf build
