#!/bin/sh

# arg $1 - target directory
# arg $2 - architecture

xbps-create --architecture "$2" \
			--pkgver spm-1.0_3 \
			--dependencies "git>=2.9.5 xdg-utils>=1.0.0 libbsd>=0.2.1" \
			--homepage "https://github.com/slamko/spm" \
			--desc "Suckless Patch Manager" \
			--compression=xz \
			--license GPLv3 \
			--maintainer "Viacheslav <https://github.com/slamko>" \
			--long-desc "Command line tool for easy patch managing for suckless software." \
			"$1"
			

