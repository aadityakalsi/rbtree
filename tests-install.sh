#!/usr/bin/env sh

dirnm=`dirname $0`
filnm=`basename $0`
exec=`cd $dirnm && pwd`/$filnm
root=`dirname $exec`

set -e

$root/build.sh
cd $root/tests/install
./test.sh
