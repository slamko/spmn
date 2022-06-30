
build-base-img: 
	docker build -t deb-pkg-base -f ./packaging/deb/Dockerfile .

build-bin-img:
	@docker build -t deb-pkg-bin-$(TARGET)-$(VERSION) -f ./packaging/deb/bin/Dockerfile

build-deb:
	@docker run -v $(shell pwd):/usr/src/$(TARGET) deb-pkg-bin-$(TARGET)-$(VERSION)

build-src-img:
	@docker build -t deb-pkg-src-$(TARGET)-$(VERSION) -f ./packaging/deb/src/Dockerfile

build-src-deb:
	@docker run -v $(shell pwd):/usr/src/$(TARGET) deb-pkg-src-$(TARGET)-$(VERSION)

clean-deb:
	rm -f *.deb

clean-deb-src:
	$(RM) -r debian
