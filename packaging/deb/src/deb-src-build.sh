#!/bin/sh

# arg $1 pkgname_ver

dh_make -p "$1" --createorig
debuild
