# Enterprise
An enterprise management system.

## Getting Nuklear:
- Run `chmod +x ./scripts/setup.sh`
- Run `./scripts/setup.sh`

- Alternatively, manually create the third_party folder and run: 
`git clone https://github.com/Immediate-Mode-UI/Nuklear.git -C /third_party/`

## Compiling and running.
Ensure you have a C compiler like GCC installed, and libsdl2 setup.
- Run `make -j $(nproc)`
- Run `./bin/native/enterprise`'

## Compiling for web:
- Alternatively, run `make -j $(nproc) web`
- The resulting wasm and js files can be found in bin/web

## Third Party Libraries:
Thank you to all third party projects


[Simple DirectMedia Layer (SDL)](https://github.com/libsdl-org/SDL)

- [License (Permissive Zlib License)](third_party/licenses/SDL)

[Nuklear](https://github.com/Immediate-Mode-UI/Nuklear)

- [License (Permissive MIT License)](third_party/licenses/Nuklear)

[Emscripten](https://github.com/emscripten-core/emscripten)
- [License (Permissive MIT License)](third_party/licenses/emscripten)

[ProggyClean](https://github.com/bluescan/proggyfonts)
- [License (Permissive MIT License)](third_party/licenses/proggyclean)