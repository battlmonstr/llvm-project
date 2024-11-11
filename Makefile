build:
	cmake --build build_reldeb -j6 --target clang-tidy

test:
	LIT_FILTER=use-prefix-increment cmake --build build_reldeb -j6 --target check-clang-tools

configure:
	cmake -S llvm -B build_reldeb -G Ninja -DCMAKE_MAKE_PROGRAM=/Applications/CLion.app/Contents/bin/ninja/mac/aarch64/ninja -DLLVM_ENABLE_PROJECTS='clang;clang-tools-extra' -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLLVM_ENABLE_ASSERTIONS=OFF -DLLVM_PARALLEL_COMPILE_JOBS=6 -DLLVM_PARALLEL_LINK_JOBS=6 -DLLVM_PARALLEL_TABLEGEN_JOBS=6

configure-xcode:
	cmake -S llvm -B build_xc -G Xcode -DLLVM_ENABLE_PROJECTS='clang;clang-tools-extra' -DCMAKE_BUILD_TYPE=Debug
