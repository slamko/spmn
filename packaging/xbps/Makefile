build-xbps: installdirs
	./packaging/xbps/xbps-create.sh $(TARGET)-$(VERSION) "x86_64"
	./packaging/xbps/xbps-create.sh $(TARGET)-$(VERSION) "amd64"

clean-xbps:
	$(RM) *.xbps
	$(RM) *repodata

