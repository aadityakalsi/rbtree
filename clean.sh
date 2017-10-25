#!/usr/bin/env sh
dirnm=`dirname $0`
filnm=`basename $0`
exec=`cd $dirnm && pwd`/$filnm
root=`dirname $exec`

set -e

rm -fr $root/_build $root/_install $root/tests/install/_source $root/tests/install/_build $root/tests/install/testLog.txt
