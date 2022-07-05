build-xbps: installdirs
	./packaging/xbps/xbps-create.sh $(PKG_NAME) "x86_64"
	./packaging/xbps/xbps-create.sh $(PKG_NAME) "amd64"

clean-xbps:
	$(RM) *.xbps
	$(RM) *repodata

