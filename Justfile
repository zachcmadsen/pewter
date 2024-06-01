build:
    cmake --build build

configure:
    cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

clean:
    rm -rf build
