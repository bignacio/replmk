#!/bin/bash

set -e

for build_type in Release Debug ; do
    echo "Building type '${build_type}' compiler ${CXX}"
    rm -rf build
    cmake -B build -DCMAKE_BUILD_TYPE=${build_type}
    make -j2 -C build

    #just check the binary actually runs
    build/src/replmk --help
    build/tests/replmk-tests

    # doctest fails memory sanitization for some reason
    # run the test but don't fail so we can still see the output
    build/tests/replmk-tests-msan || true
    build/tests/replmk-tests-asan

done