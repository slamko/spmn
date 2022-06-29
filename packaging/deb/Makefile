build-image: 
	docker build -t deb-pkg-$(TARGET)-$(VERSION) -f ./packaging/deb/Dockerfile .

build-deb:
	@docker run -v $(shell pwd):/usr/src/$(TARGET) deb-pkg-$(TARGET)-$(VERSION)
