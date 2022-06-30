build-base-img: 
	docker build -t deb-pkg-base -f ./packaging/deb/Dockerfile .

build-deb:
	@docker build 
	@docker run -v $(shell pwd):/usr/src/$(TARGET) deb-pkg-bin-$(TARGET)-$(VERSION)



clean-deb:
	rm -f *.deb
