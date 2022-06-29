#!/bin/sh 

make deb-installdirs
dpkg-deb --build spm-1.0_3/
