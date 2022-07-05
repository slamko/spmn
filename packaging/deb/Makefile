
build-deb-base-img: 
	docker build -t deb-pkg-base -f ./packaging/deb/Dockerfile .

build-deb-bin-img:
	@docker build -t deb-pkg-bin-$(PKG_NAME) -f ./packaging/deb/bin/Dockerfile .

build-deb:
	@docker run -v $(shell pwd):/usr/src/$(TARGET) --rm --env PKG_FULL_NAME=$(PKG_NAME) deb-pkg-bin-$(PKG_NAME)

build-deb-src-img:
	@docker build -t deb-pkg-src-$(PKG_NAME) -f ./packaging/deb/src/Dockerfile .

build-src-deb:
	@docker run --rm -v $(shell pwd):/usr/src/$(TARGET) deb-pkg-src-$(PKG_NAME)

clean-deb:
	rm -f *.deb

clean-deb-src:
	$(RM) -r debian
