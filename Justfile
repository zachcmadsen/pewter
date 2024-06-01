build:
    cmake --build build

configure:
    cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DFLTK_BUILD_FLTK_OPTIONS=OFF -DFLTK_BUILD_FLUID=OFF -DFLTK_BUILD_FORMS=OFF -DFLTK_BUILD_GL=OFF

clean:
    rm -rf build
