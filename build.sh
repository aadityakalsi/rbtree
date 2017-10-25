#!/usr/bin/env sh

dirnm=`dirname $0`
filnm=`basename $0`
exec=`cd $dirnm && pwd`/$filnm
root=`dirname $exec`

set -e

if [ -z "$BUILD_SHARED_LIBS" ]; then
    shared=0
else
    shared="$BUILD_SHARED_LIBS"
fi

if [ -z "$CMAKE_BUILD_TYPE" ]; then
    build=RelWithDebInfo
else
    build="$CMAKE_BUILD_TYPE"
fi

if [ -z "$GENERATOR" ]; then
    gen_arg=
else
    gen_arg="-G'$GENERATOR'"
fi

cmd="cmake -H$root -B$root/_build $gen_arg -DCMAKE_INSTALL_PREFIX=$root/_install -DCMAKE_BUILD_TYPE="$build" -DBUILD_SHARED_LIBS="$shared" -Wno-dev"
sh -c "$cmd"
env VERBOSE=1 cmake --build $root/_build --target tests --config "$build"
env VERBOSE=1 cmake --build $root/_build --target install --config "$build"
