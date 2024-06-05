build:
    cmake --build build

configure: clean
    cmake --preset dev

clean:
    rm -rf build
