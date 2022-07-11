# Maintainer: Viacheslav Chepelyk-Kozhin <vaceslavkozin619@gmail.com>
pkgname=spmn
pkgver=1.0_3
pkgrel=1
pkgdesc="Suckless Package Manager"
arch=('x86_64')
url="https://github.com/slamko/spmn"
license=('GPL')
depends=('glibc' 'libbsd' 'xdg-utils' 'git')
makedepends=('git')
source=("$pkgname-$pkgver.tar.gz::https://github.com/slamko/spmn/archive/refs/tags/v1.0_3.tar.gz")
md5sums=('SKIP')

prepare() {
	mkdir "$pkgname-$pkgver"
	cp -r ../* "$pkgname-$pkgver"
	cd "$pkgname-$pkgver1"
	git clone "https://github.com/slamko/zic.git"
}

build() {
	cd "$pkgname-$pkgver"
	make
}

package() {
	cd "$pkgname-$pkgver"
	make DESTDIR="$pkgdir/" install
}
