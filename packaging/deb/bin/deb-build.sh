#!/bin/sh 

make deb-installdirs
dpkg-deb --build "$PKG_FULL_NAME"/
make clean-pkgdirs
