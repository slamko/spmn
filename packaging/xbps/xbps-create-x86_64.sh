#!/bin/sh

# arg $1 - target directory

xbps-create --architecture x86_64 \
			--pkgver "$1" \
			--dependencies "git>=2.9.5 xdg-utils>=1.0.0 libbsd>=0.2.1" \
			--homepage "https://github.com/slamko/spmn" \
			--desc "Suckless Patch Manager" \
			--compression=xz \
			--license GPLv3 \
			--maintainer "Viacheslav <https://github.com/slamko>" \
			--long-desc "Command line tool for easy patch managing for suckless software." \
			"$1"
			

